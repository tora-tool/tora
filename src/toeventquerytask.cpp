/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toconf.h"
#include "toeventquerytask.h"
#include "toresultstats.h"
#include "totool.h"
#include "toresultstats.h"

#include <QApplication>


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
    : QThread(0),               // don't set or thread will delete
                                // with parent and possibly crash
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

            // begin thread's event loop
            exec();
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
        delete Query;
        Query = 0;
    }
    catch(...) {
        // ignored
    }

    ThreadAlive.lock();
    QCoreApplication::postEvent(this, new FinishedEvent(this));
}


toEventQueryTask::~toEventQueryTask() {
}


void toEventQueryTask::customEvent(QEvent *event) {
    FinishedEvent *e = dynamic_cast<FinishedEvent *>(event);
    if(e) {
        QThread *t = e->thread();
        if(t) {
            t->exit();
            t->wait();
            delete t;
        }
    }
}


void toEventQueryTask::close() {
    try {
        if(isRunning())
            emit done();

        disconnect(this, 0, 0, 0);
        if(Query && !Closed)
            Query->cancel();
    }
    catch(...) {
        // noop
    }

    Closed = true;

    // exit thread event loop. safe to call before event loop starts.
    exit();
}


void toEventQueryTask::read(bool all) {
    if(isRunning())
        emit readRequested(all);
}


void toEventQueryTask::pread(bool all) {
    if(!isRunning())
        return;
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
