
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "core/toeventquery.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/toeventqueryworker.h"
//#include "widgets/toresultstats.h"
#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "core/toconnectionsubloan.h"
#include "core/toconnectiontraits.h"

toEventQuery::toEventQuery(QObject *parent
                           , toConnection &conn
                           , QString const& sql
                           , toQueryParams const& param
                           , FETCH_MODE mode
                           , QList<QString> const &init
                           //, toResultStats *stats
                          )
    : QObject(parent)
    , SQL(sql)
    , Param(param)
    , ColumnCount(0)
    , Processed(0L)
    //, Statistics(stats)
    , Thread(NULL)
    , Worker(NULL)
    , Started(false)
    , WorkDone(false)
    , Connection(new toConnectionSubLoan(conn))
    , CancelCondition(new toEventQuery::WaitConditionWithMutex())
    , Mode(mode)
{
    /* BIG FAT WARNING QThread's parent must be  NULL, so it is not disposed when toEventQuery is deleted.
     * Theoretically (and practically) it can be freed after ~toEventQuery is processed
     */
    Thread = new BGThread(NULL);
    Thread->setObjectName("toEventQuery");
    TLOG(7, toDecorator, __HERE__) << "toEventQuery created" << std::endl;
}

toEventQuery::toEventQuery(QObject *parent
                           , QSharedPointer<toConnectionSubLoan> &conn
                           , QString const& sql
                           , toQueryParams const& param
                           , FETCH_MODE mode
                           , QList<QString> const &init
                           //, toResultStats *stats
                          )
    : QObject(parent)
    , SQL(sql)
    , Param(param)
    , ColumnCount(0)
    , Processed(0L)
    //, Statistics(stats)
    , Thread(NULL)
    , Worker(NULL)
    , Started(false)
    , WorkDone(false)
    , Connection(conn)
    , CancelCondition(new toEventQuery::WaitConditionWithMutex())
    , Mode(mode)
{
    /* BIG FAT WARNING QThread's parent must be  NULL, so it is not disposed when toEventQuery is deleted.
     * Theoretically (and practically) it can be freed after ~toEventQuery is processed
     */
    Thread = new BGThread(NULL);
    Thread->setObjectName("toEventQuery");
    TLOG(7, toDecorator, __HERE__) << "toEventQuery created" << std::endl;
}

toEventQuery::~toEventQuery()
{
    TLOG(7, toDecorator, __HERE__) << "~toEventQuery a" << std::endl;
    // we request bg thread to stop, but do not really wait for thread finish
    stop();
}

void toEventQuery::start()
{
    if ( Worker || Started || WorkDone )
        throw tr("toEventQuery::start - can not restart already stared query");

    Worker = new toEventQueryWorker(this, Connection, CancelCondition, SQL, Param);
    Worker->moveToThread(Thread);
    Thread->Slave = Worker;

    // Connect to Worker's API
    connect(Worker, SIGNAL(headers(toQColumnDescriptionList &, int)),      //  BG -> main
            this, SLOT(slotDesc(toQColumnDescriptionList &, int)));

    connect(Worker, SIGNAL(data(const ValuesList &)),                      //  BG -> main
            this, SLOT(slotData(const ValuesList &)));

    connect(Worker, SIGNAL(error(const toConnection::exception &)),        //  BG -> main
            this, SLOT(slotError(const toConnection::exception &)));

    connect(Worker, SIGNAL(workDone()),                                    //  BG -> main
            this, SLOT(slotFinished()));

    connect(Worker, SIGNAL(rowsProcessed(unsigned long)),                  //  BG -> main
            this, SLOT(slotRowsProcessed(unsigned long)));

    connect(this,   SIGNAL(dataRequested()),  Worker, SLOT(slotRead()));   // main -> BG

    connect(this,   SIGNAL(consumed()),       Worker, SLOT(slotRead()));   // main -> BG

    // Connect to QThread's API
    //  error handling
    connect(Worker, SIGNAL(error(toConnection::exception const &))         //  BG -> main
            , this, SLOT(slotError(toConnection::exception const &)));
    //  initization
    connect(Thread, SIGNAL(started()),        Worker, SLOT(init()));       // main -> BG
    connect(Worker, SIGNAL(started()),        this,   SLOT(slotStarted()));// BG   -> main
    //  finish
    connect(Worker, SIGNAL(finished()),       Thread, SLOT(quit()));          // BG -> BG?? (quit event loop)
    connect(Worker, SIGNAL(finished()),       Worker, SLOT(deleteLater()));   // BG -> BG
    connect(Thread, SIGNAL(finished()),       Thread, SLOT(deleteLater()));   // BG -> main
    connect(Thread, SIGNAL(destroyed()),      this,   SLOT(slotThreadEnd())); // main -> main
    connect(this,   SIGNAL(stopRequested()),  Worker, SLOT(slotStop()));      // main -> BG

    TLOG(7, toDecorator, __HERE__) << "toEventQuery start" << std::endl;
    // finally start the thread
    Thread->start();
}

void toEventQuery::setFetchMode(FETCH_MODE m)
{
    if (Mode == READ_FIRST && m == READ_ALL)
        emit dataRequested();
    Mode = m;
}

toQColumnDescriptionList const& toEventQuery::describe(void) const
{
    return Description;
}

unsigned toEventQuery::columnCount(void) const
{
    return ColumnCount;
}

unsigned long toEventQuery::rowsProcessed(void) const
{
    return Processed;
}

unsigned int toEventQuery::rowsAvaiable() const
{
    if (ColumnCount == 0)
        return 0;

    return Values.size() / ColumnCount;
}

/* Call stask for this function:
 * Async thread toEventQueryWorker emits signal data()
 * the rest is processed in the main thread
 * the slot taskData receives data (runs within the main thread context)
 * taskData emits signal dataAvailable
 * toEventQuery parent(toResulModel for example) receives signal
 * and then calls toEventQeury::readValue
 */
toQValue toEventQuery::readValue()
{
    if (Values.isEmpty())
        throw tr("Read past end of query");

    if ((Values.size() == ColumnCount) && !eof())
        emit dataRequested();

    return Values.takeFirst();
}

bool toEventQuery::eof(void) const
{
    if (hasMore())
        return false;
    return WorkDone;
}

QString const& toEventQuery::sql(void) const
{
    return SQL;
}

bool toEventQuery::hasMore(void) const
{
    return !Values.isEmpty();
}

void toEventQuery::stop(void)
{
    TLOG(7, toDecorator, __HERE__) << "toEventQuery stop a" << std::endl;
    if (WorkDone)
        return;

    if (Thread && Thread->isRunning())
    {
        Utils::toBusy busy;
        TLOG(7, toDecorator, __HERE__) << "toEventQuery stop Thread is running" << std::endl;
        CancelCondition->Mutex.lock();
        emit stopRequested();
        bool succeeded = CancelCondition->WaitCondition.wait(&CancelCondition->Mutex, 1000); // possible wait for 1 sec
        CancelCondition->Mutex.unlock();
        if (!succeeded)
        {
            TLOG(7, toDecorator, __HERE__) << "toEventQuery stop bg did not respond" << std::endl;
            if (Connection->ParentConnection.getTraits().hasAsyncBreak())
                try
                {
                    (*Connection)->cancel();
                }
            TOCATCH;
        }
        //Thread->wait();
    }
    WorkDone = true;
}

// private slots

void toEventQuery::slotStarted()
{
    Started = true;
    emit dataRequested();             // request 1st chunk of rows
}

// warning: values is a reference only bg thread's stack
void toEventQuery::slotData(const ValuesList &values)
{
    TLOG(7, toDecorator, __HERE__) << "toEventQuery slot data" << std::endl;
    Values << values;

    if (Mode == READ_ALL)
        emit consumed();

    // TODO: this signal can also be emitted asynchronically
    // from QTime - once per second
    emit dataAvailable(this);
}

void toEventQuery::slotDesc(toQColumnDescriptionList &desc, int columns)
{
    TLOG(7, toDecorator, __HERE__) << "toEventQuery slot desc" << std::endl;
    Started = true;
    Description = desc;
    Q_ASSERT_X(columns >= 0 , qPrintable(__QHERE__), " invalid number of columns for a result");
    ColumnCount = columns;
    emit descriptionAvailable(this);
}

void toEventQuery::slotError(const toConnection::exception &msg)
{
    TLOG(7, toDecorator, __HERE__) << "toEventQuery slot error" << std::endl;
    emit error(this, msg);
}

void toEventQuery::slotFinished()
{
    TLOG(7, toDecorator, __HERE__) << "toEventQuery slot finish" << std::endl;
    WorkDone = true;
    disconnect(SIGNAL(consumed()));
    disconnect(SIGNAL(dataRequested()));
    emit done(this, Processed);
}

void toEventQuery::slotRowsProcessed(unsigned long rows)
{
    TLOG(7, toDecorator, __HERE__) << "toEventQuery slot rows" << std::endl;
    Processed = rows;
}

void toEventQuery::slotThreadEnd()
{
    TLOG(7, toDecorator, __HERE__) << "toEventQuery thread end" << std::endl;
    Thread = NULL;
    Worker = NULL;
}
