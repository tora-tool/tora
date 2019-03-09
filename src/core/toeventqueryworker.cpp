
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

#include "core/toeventqueryworker.h"
#include "core/toeventquery.h"
#include "core/utils.h"
#include "core/tologger.h"
//stat #include "widgets/toresultstats.h"
#include "core/totool.h"
#include "core/toconfiguration.h"
#include "core/toquery.h"
#include "core/toqvalue.h"
#include "core/todatabaseconfig.h"
#include "core/toconnectiontraits.h"

#include "widgets/toworkspace.h"
#include "widgets/totoolwidget.h"

#include <QApplication>
#include <QtCore/QMutexLocker>
#include <QtCore/QTimer>

/* It is not allowed to throw an exception from event slot.
 * So let's catch all the possible errors in slot handlers
 */
#define CATCH_ALL                                   \
    catch(const toConnection::exception &str) {     \
        TLOG(7,toDecorator,__HERE__) << "What: " << str << std::endl;\
        emit error(str);                            \
        Stopped=true;                               \
        close();                                    \
    }                                               \
    catch(const QString &str) {                     \
        TLOG(7,toDecorator,__HERE__) << "What: " << str << std::endl;\
        emit error(str);                            \
        Stopped=true;                               \
        close();                                    \
    }                                               \
    catch(std::exception const &e) {                \
        TLOG(7,toDecorator,__HERE__) << "what: " << e.what() << std::endl;\
        emit error(QString(e.what()));              \
        Stopped=true;                               \
        close();                                    \
    }                                               \
    catch(...) {                                    \
        TLOG(7,toDecorator,__HERE__) << "	Ignored exception." << std::endl; \
        emit error(tr("Unknown exception."));       \
        Stopped=true;                               \
        close();                                    \
    }

void toEventQueryWorker::toQueryPriv::init()
{
    try
    {
        // Try to switch the current db schema
        if (m_ConnectionSubLoan.SchemaInitialized == false && !m_ConnectionSubLoan.Schema.isEmpty())
        {
            QString sql = m_ConnectionSubLoan.ParentConnection.getTraits().schemaSwitchSQL(m_ConnectionSubLoan.Schema);
            if (!sql.isEmpty())
            {
                m_Query = m_ConnectionSubLoan->createQuery(this);
                m_ConnectionSubLoan->setQuery(this);
                m_Query->execute(sql);
                delete m_Query;
            }
            m_ConnectionSubLoan.SchemaInitialized = true;
            m_ConnectionSubLoan->setSchema(m_ConnectionSubLoan.Schema); // assign value in toConnectionSub from toConnectionSubLoan
        }

        // Try to switch the current NLS_SETTINGS (not implemented yet)
        if ( m_ConnectionSubLoan->isInitialized() == false)
        {
            Q_FOREACH(QString sql, m_ConnectionSubLoan.initStrings())
            {
                if (sql.isEmpty())
                    continue;
                m_Query = m_ConnectionSubLoan->createQuery(this);
                m_ConnectionSubLoan->setQuery(this);
                m_Query->execute(sql);
                delete m_Query;
            }
            m_ConnectionSubLoan->setInitialized(true);
        }
#if defined(TORA_EXPERIMENTAL) && 0
// This breaks Mysql, PostgreSQL, ODBC
        {
            toToolWidget *widget = toWorkSpaceSingle::Instance().currentTool();
            QString title = widget ? widget->windowTitle() : "no window";
            m_Query = m_ConnectionSubLoan->createQuery(this);
            m_ConnectionSubLoan->setQuery(this);
            m_Query->execute(QString("begin DBMS_APPLICATION_INFO.SET_MODULE('%1','%2'); end;").arg(title).arg(this->m_SQLName));
            delete m_Query;
        }
#endif
        m_Query = m_ConnectionSubLoan->createQuery(this);
        m_ConnectionSubLoan->setQuery(this);
        m_Query->execute();
    }
    catch (...)
    {
        if (m_Query)
            delete m_Query;
        m_ConnectionSubLoan->setQuery(NULL);
        m_Query = NULL;
        throw;
    }
}

toEventQueryWorker::toEventQueryWorker(toEventQuery *c
                                       , QSharedPointer<toConnectionSubLoan> &conn
                                       , QSharedPointer<toEventQuery::WaitConditionWithMutex> &wait
                                       , QString &sql
                                       , toQueryParams &params)
    : Consumer(c)
    , SQL(sql)
    , Params(params)
    , Connection(conn)
    , CancelCondition(wait)
    , ColumnCount(0)
    , Stopped(false)
    , Closed(false)
    , Query(*Connection, SQL, Params)
{
    TLOG(7, toDecorator, __HERE__) << "toEventQueryWorker created" << std::endl;
    connect(this, SIGNAL(readRequested()), this, SLOT(slotRead()));
    Query.moveToThread(c->Thread);
}

toEventQueryWorker::~toEventQueryWorker()
{
    TLOG(7, toDecorator, __HERE__) << "~toEventQueryWorker" << std::endl;
}

void toEventQueryWorker::init()
{
    TLOG(7, toDecorator, __HERE__) << "toEventQueryWorker init a" << std::endl;
    try
    {
        Query.init();
        emit started();
        toQColumnDescriptionList desc = Query.describe();
        ColumnCount = Query.columns();
        //QThread::sleep(5); // to simulate slow query
        emit headers(desc, ColumnCount);

        if (Query.eof())
        {
            // emit empty result
            // ValuesList values;
            // emit data(values);
            Stopped = true;
            close();
        }
    }
    CATCH_ALL;
    TLOG(7, toDecorator, __HERE__) << "toEventQueryWorker init b" << std::endl;
}

void toEventQueryWorker::slotStop()
{
    TLOG(7, toDecorator, __HERE__) << "toEventQueryWorker syncStop" << std::endl;
    Stopped = true;
    CancelCondition->WaitCondition.wakeAll();
    close();
}

void toEventQueryWorker::close()
{
    TLOG(7, toDecorator, __HERE__) << "toEventQueryWorker close a" << std::endl;
    if (Closed)
        return;

    unsigned long p = Query.rowsProcessed();
    if (p > 0)
        emit rowsProcessed(p);

    emit workDone();

    Closed = true;
    emit finished();
    TLOG(7, toDecorator, __HERE__) << "toEventQueryWorker close b" << std::endl;
}

void toEventQueryWorker::slotRead()
{
    try
    {
        //TLOG(7, toDecorator, __HERE__) << "toEventQueryWorker slot read" << std::endl;
        if (Query.eof() || ColumnCount == 0 || Stopped)
        {
            Stopped = true;
            close();
            return;
        }

        unsigned maxRead = toConfigurationNewSingle::Instance().option(ToConfiguration::Database::InitialFetchInt).toInt();
        ValuesList values;
        for (unsigned row = 0; row < maxRead; row++)
        {
            for (unsigned i = 0; i < ColumnCount && !Query.eof(); i++)
                values.append(Query.readValue());
        }

        if (values.size() > 0)
        {
            //QThread::sleep(5); // to simulate slow query
            emit data(values);    // must not access after this line
        }

        if (Query.eof())
        {
            Stopped = true;
            close();
        }
    }
    CATCH_ALL
}
