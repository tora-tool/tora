
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
#include "toeventquerytask.h"
#include "toresultstats.h"
#include "totool.h"
#include "toresultstats.h"
#include "toconfiguration.h"

#include <QApplication>
#include <QMutexLocker>
#include <QTimer>


static const int FIREWALL_TIMEOUT = 240000;


#define CATCH_ALL                                   \
    catch(const toConnection::exception &str) {     \
        if(!Closed) {                               \
            emit error(str);                        \
            close();                                \
        }                                           \
    }                                               \
    catch(const QString &str) {                     \
        if(!Closed) {                               \
            emit error(str);                        \
            close();                                \
        }                                           \
    }                                               \
    catch(...) {                                    \
        if(!Closed) {                               \
            emit error(tr("Unknown exception"));    \
            close();                                \
        }                                           \
    }


toEventQueryTask::toEventQueryTask(QObject *parent,
                                   toConnection &conn,
                                   const QString &sql,
                                   const toQList &param,
                                   toResultStats *stats)
    : toRunnable(),
      SQL(sql),
      Params(param),
      Statistics(stats) {
    Query      = 0;
    Columns    = 0;
    Connection = &conn;
    Closed     = false;
    setObjectName("toEventQueryTask");
}


void toEventQueryTask::run(void) {
    try {
        Query = new toQuery(*Connection);
        Query->execute(SQL, Params);

        connect(this,
                SIGNAL(readRequested(bool)),
                this,
                SLOT(pread(bool)),
                Qt::QueuedConnection);

        toQDescList desc = Query->describe();
        Columns = Query->columns();
        emit headers(desc, Columns);

        if(Query->eof()) {
            // emit empty result
            ValuesList values;
            emit data(values);
        }
        else {
            read();

            if(toConfigurationSingle::Instance().firewallMode())
                QTimer::singleShot(FIREWALL_TIMEOUT, this, SLOT(timeout()));

            // begin thread's event loop
            thread()->exec();
        }
    }
    CATCH_ALL;

    close();

    try {
        if(Statistics)
            Statistics->changeSession(*Query);
    }
    catch(...) {
        // ignored
    }

    try {
        QMutexLocker lock(&CloseLock);
        Closed = true;
        delete Query;
        Query = 0;
    }
    catch(...) {
        // ignored
    }

    ThreadAlive.lock();
}


toEventQueryTask::~toEventQueryTask() {
}


void toEventQueryTask::close() {
    try {
        emit done();

        disconnect(this, 0, 0, 0);

        QMutexLocker lock(&CloseLock);
        if(Query && !Closed)
            Query->cancel();
    }
    catch(...) {
        // noop
    }

    Closed = true;

    // exit thread event loop. safe to call before event loop starts.
    if(thread())
        thread()->exit();
}


void toEventQueryTask::read(bool all) {
    emit readRequested(all);
}


void toEventQueryTask::pread(bool all) {
    if(!Query || Columns < 1) {
        close();
        return;
    }

    int maxRead = toConfigurationSingle::Instance().maxNumber();

    try {
        do {
            ValuesList values;

            for(int row = 0; row < maxRead; row++) {
                for(int i = 0; i < Columns && !Query->eof(); i++)
                    values.append(Query->readValueNull());
            }

            if(values.size() > 0)
                emit data(values);    // must not access after this line
        } while(all && !Query->eof());
    }
    CATCH_ALL;

    try {
        if(!Query || Query->eof())
            close();
    }
    CATCH_ALL;
}


void toEventQueryTask::timeout()
{
    if(Closed)
        return;

    pread(false);

    // check again in case config changes
    if(toConfigurationSingle::Instance().firewallMode())
        QTimer::singleShot(FIREWALL_TIMEOUT, this, SLOT(timeout()));
}
