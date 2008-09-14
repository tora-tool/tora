/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOEVENTQUERYTASK_H
#define TOEVENTQUERYTASK_H

#include "config.h"
#include "toconnection.h"
#include "tothread.h"
#include "torunnable.h"

#include <QThread>
#include <QPointer>
#include <QMetaType>
#include <QEvent>
#include <QMutex>

typedef QList<toQValue> ValuesList;
Q_DECLARE_METATYPE(ValuesList);

class toResultStats;


/**
 * Threaded class used by toEventQuery. Internal to toEventQuery only.
 *
 * All signals and calls to slots should be created with
 * Qt::QueuedConnection except to read() and stop()
 *
 */
class toEventQueryTask : public toRunnable {
    Q_OBJECT;

    // the real query object
    QPointer<toQuery> Query;

    // sql and bind parameters
    QString SQL;
    toQList Params;

    int Columns;

    // Statistics to be used if any.
    QPointer<toResultStats> Statistics;

    toConnection *Connection;

    // object was closed
    volatile bool Closed;

    // can't synchronize on query or cancel wouldn't work, but need to
    // protect access during close() which may be called from the main
    // thread
    QMutex CloseLock;


public:

    // keeps thread from exiting until toEventQuery exits. this
    // prevents many race conditions and the caller can always assume
    // task is alive.
    QMutex ThreadAlive;

    toEventQueryTask(QObject *parent,
                     toConnection &conn,
                     const QString &sql,
                     const toQList &param,
                     toResultStats *stats = NULL);
    virtual ~toEventQueryTask();

protected:


    /**
     * Overrides toRunnable::run. Create toRunnableThread to start
     *
     */
    virtual void run(void);


private slots:
    void pread(bool all);
    void timeout(void);


public slots:
    /**
     * Queues a read request. Emits readRequested()
     *
     */
    void read(bool all = false);

    /**
     * Closes this class. Good idea to call this.
     *
     */
    void close(void);


signals:
    // must be careful when defining signals. passing by reference
    // will share memory between threads.


    /**
     * A read was requested
     *
     */
    void readRequested(bool all);


    /**
     * Error message
     *
     */
    void error(const toConnection::exception &msg);


    /**
     * Result headers
     *
     * @param desc list of header values
     * @param columns number of columns in result
     */
    void headers(toQDescList &desc, int columns);


    /**
     * Data read from query
     *
     */
    void data(ValuesList &values);


    /**
     * Emitted when sql query is done
     *
     */
    void done();
};

#endif
