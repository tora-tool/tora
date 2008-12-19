
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconf.h"
#include "toeventquery.h"
#include "toeventquerytask.h"
#include "toresultstats.h"
#include "totool.h"
#include "torunnable.h"


toQValue toEventQuery::readValueNull() {
    if(Values.isEmpty())
        throw tr("Read past end of query");

    if((Values.size() == 15 || Values.size() == 1) && !eof()) {
        if(Task)
            Task->read();           // getting close to end, request read
    }

    return Values.takeFirst();
}


toQValue toEventQuery::readValue() {
    return toNull(readValueNull());
}


toEventQuery::toEventQuery(toConnection &conn,
                           const QString &sql,
                           const toQList &param,
                           toResultStats *stats)
    : SQL(sql),
      Param(param),
      Statistics(stats),
      Task(0)
{

    Connection = &conn;
    Processed  = 0;
    Columns    = 0;
    TaskDone   = false;
}

toEventQuery::toEventQuery(toConnection &conn,
                           toQuery::queryMode mode,
                           const QString &sql,
                           const toQList &param,
                           toResultStats *stats)
    : SQL(sql),
      Param(param),
      Statistics(stats),
      Task(0) {

    Connection = &conn;
    Processed  = 0;
    Columns    = 0;
    TaskDone   = false;
}


void toEventQuery::start() {
    Task = new toEventQueryTask(this, *Connection, SQL, Param, Statistics);
    Task->ThreadAlive.lock();

    qRegisterMetaType<toQDescList>("toQDescList&");
    qRegisterMetaType<ValuesList>("ValuesList&");

    connect(Task,
            SIGNAL(headers(toQDescList &, int)),
            this,
            SLOT(taskDesc(toQDescList &, int)),
            Qt::QueuedConnection);

    connect(Task,
            SIGNAL(data(ValuesList &)),
            this,
            SLOT(taskData(ValuesList &)),
            Qt::QueuedConnection);

    connect(Task,
            SIGNAL(error(const toConnection::exception &)),
            this,
            SLOT(taskError(const toConnection::exception &)),
            Qt::QueuedConnection);

    connect(Task,
            SIGNAL(done()),
            this,
            SLOT(taskFinished()),
            Qt::QueuedConnection);

    connect(Task,
            SIGNAL(rowsProcessed(int)),
            this,
            SLOT(taskRowsProcessed(int)),
            Qt::QueuedConnection);

    (new toRunnableThread(Task))->start();
}


toEventQuery::~toEventQuery() {
    try {
        if(Task) {
            disconnect(Task, 0, 0, 0);
            if(Task->thread())
                Task->thread()->exit();
            Task->ThreadAlive.unlock();
        }
    }
    catch(...) {
        ; // ignored
    }
}


bool toEventQuery::eof(void) const {
    if(hasMore())
        return false;
    return TaskDone;
}


void toEventQuery::stop(void) {
    if(!TaskDone && Task)
        Task->close();
}


void toEventQuery::taskData(ValuesList &values) {
    Values += values;
    emit dataAvailable();

    try {
        if(Statistics)
            Statistics->refreshStats(false);
    }
    catch(...) {
        // ignored
    }
}


void toEventQuery::taskDesc(toQDescList &desc, int column) {
    Description = desc;
    Columns = column;
    emit descriptionAvailable();
}


void toEventQuery::taskError(const toConnection::exception &msg) {
    emit error(msg);
}


void toEventQuery::taskFinished() {
    TaskDone = true;
    emit done();
}


void toEventQuery::taskRowsProcessed(int rows) {
    Processed = rows;
}
