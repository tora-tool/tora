/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOCONNECTIONPOOL_H
#define TOCONNECTIONPOOL_H

#include "toconnection.h"
#include "torunnable.h"

#include <QMutex>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QWaitCondition>
#include <QThread>
#include <QEvent>
#include <QTimer>

class toConnectionPool;

/**
 * Tests connections in pool
 *
 */
class toConnectionPoolTest : public QThread {
    Q_OBJECT;

    QPointer<toConnectionPool> Pool;
    QTimer *timer;

public:
    toConnectionPoolTest(toConnectionPool *pool);

    /**
     * Overrides. Call start() to execute thread.
     *
     */
    virtual void run(void);

private slots:
    // execute tests
    void test(void);
};


/**
 * Executes sql on pool connections
 *
 */
class toConnectionPoolExec : public toRunnable {
    Q_OBJECT;

public:
    enum Action {
        Commit,
        Rollback,
        Cancel,
        Execute
    };

private:
    QPointer<toConnectionPool> Pool;
    Action action;

    // for Execute
    toQList Params;
    QString Sql;


public:
    toConnectionPoolExec(toConnectionPool *pool, Action act);

    toConnectionPoolExec(toConnectionPool *pool,
                         Action act,
                         const QString &sql,
                         toQList &params);


    /**
     * Overrides. Call start() to execute thread.
     *
     */
    virtual void run(void);
};


class toConnectionPool : public QObject {
    Q_OBJECT;

    friend class toConnectionPoolTest;
    friend class toConnectionPoolExec;

    toConnectionPoolTest *TestThread;

    // for future configuration
    static const int PreferredSize = 3;

    enum PooledState {
        Busy,
        Free,
        Broken
    };

    class PooledSub {
    public:
        toConnectionSub *Sub;
        PooledState State;

        // need this for template container
        PooledSub() {
            Sub = 0;
            State = Broken;
        }

        PooledSub(toConnectionSub *sub, PooledState state = Free) {
            Sub = sub;
            State = state;
        }
    };


    // the pool
    typedef QList<PooledSub *> SubList;
    volatile SubList Pool;

    // lock for the pool.
    QMutex PoolLock;

    // toConnection instance this class is a member of.  will be used
    // to create new connections when needed.
    QPointer<toConnection> Connection;

    // this is used internally to test
    PooledState test(PooledSub *sub);
    PooledState test(int member);
    int size(void);

    // get a specific connection.
    toConnectionSub* steal(int member);

    // create new connection at position
    void fix(int member);

public:
    toConnectionPool(toConnection *conn);
    ~toConnectionPool();


    /**
     * Get a connection from pool.
     *
     * This method is thread safe
     */
    toConnectionSub* borrow(void);



    /**
     * Release connection back to pool
     *
     * This method is thread safe
     */
    void release(toConnectionSub *sub);


    /**
     * Commit connections that are not busy
     *
     */
    void commit(bool wait = false);


    /**
     * Rollback connections that are not busy
     *
     */
    void rollback(bool wait = false);


    /**
     * Cancels all running queries
     *
     */
    void cancelAll(bool wait = false);


    /**
     * Execute sql on all connections
     *
     */
    void executeAll(const QString &sql, toQList &params);
};


/**
 * Encapsulates returning toConnectionSub to pool for easier exception
 * proof code.
 *
 */
class PoolPtr {
    QPointer<toConnectionPool> Pool;
    toConnectionSub *Sub;

public:
    PoolPtr(toConnectionPool *pool) {
        Pool = pool;
        Sub = Pool->borrow();
    }


    toConnectionSub* operator*() {
        return Sub;
    }


    ~PoolPtr() {
        if(Pool)
            Pool->release(Sub);
    }
};

#endif
