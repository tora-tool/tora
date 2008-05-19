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
#include "toconnectionpool.h"
#include "tosql.h"

#include <QMutexLocker>
#include <QTimer>
#include <QCoreApplication>


static const int TEST_MSEC = 5000;


toConnectionPoolTest::toConnectionPoolTest(toConnectionPool *pool)
    : QThread(0) {
    Pool = pool;
}


void toConnectionPoolTest::run() {
    QTimer::singleShot(TEST_MSEC, this, SLOT(test()));
    exec();
}


void toConnectionPoolTest::test() {
    for(int i = 0; Pool && i < Pool->Pool.size(); i++) {
        toConnectionPool::PooledState state = Pool->test(i);
        if(state == toConnectionPool::Busy)
            continue;

        if(state == toConnectionPool::Broken)
            Pool->fix(i);
    }

    QTimer::singleShot(TEST_MSEC, this, SLOT(test()));
}


toConnectionPoolExec::toConnectionPoolExec(toConnectionPool *pool, Action act) {
    action = act;
    Pool   = pool;
}


toConnectionPoolExec::toConnectionPoolExec(toConnectionPool *pool,
                                           Action act,
                                           const QString &sql,
                                           toQList &params) {
    action = act;
    Pool   = pool;
    Sql    = sql;
    Params = params;
}


void toConnectionPoolExec::run() {
    for(int mem = 0; mem < Pool->Pool.size(); mem++) {
        toConnectionSub *sub = Pool->steal(mem);
        if(!sub)
            continue;

        try {
            switch(action) {
            case Commit:
                Pool->Connection->commit(sub);
                break;

            case Rollback:
                Pool->Connection->rollback(sub);
                break;

            case Cancel:
                sub->cancel();
                // send it back to the pool
                Pool->release(sub);
                break;

            case Execute: {
                toQuery q(*(Pool->Connection), sub, Sql, Params);
                break;
            }

            } // switch
        }
        TOCATCH; // show errors to user
    }

    // thread deletes self
    QCoreApplication::postEvent(this, new ExecFinished(this));
}


void toConnectionPoolExec::customEvent(QEvent *event) {
    ExecFinished *e = dynamic_cast<ExecFinished *>(event);
    if(e) {
        QThread *t = e->thread();
        if(t) {
            t->exit();
            t->wait();
            delete t;
        }
    }
}


toConnectionPool::toConnectionPool(toConnection *conn) : QObject(conn) {
    Connection = conn;
    QMutexLocker lock(&PoolLock);

    for(int i = 0; i < PreferredSize; i++) {
        PooledSub *psub = new PooledSub;
        try {
            psub->Sub = Connection->addConnection();
            psub->State = Free;
        }
        catch(...) {
            psub->State = Broken;
        }

        Pool.append(psub);
    }

    TestThread = new toConnectionPoolTest(this);
    TestThread->start();
}


toConnectionPool::~toConnectionPool() {
    TestThread->exit();
    TestThread->wait();
    delete TestThread;

    toConnection *conn = Connection;
    Connection = 0;

    QMutexLocker lock(&PoolLock);

    for(int mem = 0; mem < Pool.size(); mem++) {
        PooledSub *psub = Pool[mem];
        try {
            psub->Sub->cancel();
        }
        catch(...) {
        }

        conn->closeConnection(psub->Sub);
    }

    while(!Pool.isEmpty())
        delete Pool.takeFirst();
}


void toConnectionPool::fix(int member) {
    if(!Connection)
        return;

    QMutexLocker lock(&PoolLock);
    PooledSub *psub = Pool[member];
    psub->State = Broken;
    lock.unlock();

    Connection->closeConnection(psub->Sub);
    try {
        psub->Sub = Connection->addConnection();
        psub->State = Free;
    }
    catch(...) {
        psub->State = Broken;
    }
}


toConnectionPool::PooledState toConnectionPool::test(int member) {
    if(!Connection)
        return Broken;

    QMutexLocker lock(&PoolLock);
    PooledSub *psub = Pool[member];
    if(psub->State != Free)
        return psub->State;

    psub->State = Busy;
    lock.unlock();

    psub->State = test(psub);
    return psub->State;
}


toConnectionPool::PooledState toConnectionPool::test(PooledSub *sub) {
    PooledState state = Free;
    try {
        toQList params;
        toQuery q(*Connection,
                  sub->Sub,
                  toSQL::string("Global:Now", *Connection),
                  params);
    }
    catch(...) {
        state = Broken;
    }

    return state;
}


toConnectionSub* toConnectionPool::steal(int member) {
    QMutexLocker lock(&PoolLock);
    return Pool[member]->Sub;
}


toConnectionSub* toConnectionPool::borrow() {
    QMutexLocker lock(&PoolLock);

    try {
        for(int mem = 0; mem < Pool.size(); mem++) {
            PooledSub *psub = Pool[mem];

            if(psub->State == Free) {
                psub->State = Busy;
                lock.unlock();

                PooledState state = test(psub);
                if(state == Free)
                    return psub->Sub;
                else
                    psub->State = state;

                lock.relock();

                // be careful adding code after this, as the pool size
                // might have changed since lock was last acquired.
            }
        }
    }
    catch(...) {
        throw;
    }

    PooledSub *psub = new PooledSub(Connection->addConnection(), Busy);
    Pool.append(psub);
    return psub->Sub;
}


void toConnectionPool::remove(int member) {
    QMutexLocker lock(&PoolLock);

    PooledSub *psub = Pool.takeAt(member);
    Connection->closeConnection(psub->Sub);
    delete psub;
}


void toConnectionPool::release(toConnectionSub *sub) {
    QMutexLocker lock(&PoolLock);

    for(int mem = 0; mem < Pool.size(); mem++) {
        PooledSub *psub = Pool[mem];
        if(psub->Sub == sub) {

            // if needscommit is false, we can eliminate extra
            // connections here.
            if(Pool.size() > PreferredSize &&
               Connection &&
               !Connection->needCommit()) {

                lock.unlock();
                remove(mem);
            }
            else
                psub->State = Free;

            return;
        }
    }

    if(Connection)
        Connection->closeConnection(sub);
}


void toConnectionPool::commit() {
    (new toConnectionPoolExec(this, toConnectionPoolExec::Commit))->start();
}


void toConnectionPool::rollback() {
    (new toConnectionPoolExec(this, toConnectionPoolExec::Rollback))->start();
}


void toConnectionPool::cancelAll(bool wait) {
    toConnectionPoolExec *ex = new toConnectionPoolExec(
        this,
        toConnectionPoolExec::Cancel);
    printf("cancel all %i\n", wait);
    if(!wait)
        ex->start();
    else
        ex->run();
}


void toConnectionPool::executeAll(const QString &sql, toQList &params) {
    (new toConnectionPoolExec(this, toConnectionPoolExec::Execute, sql, params))->start();
}
