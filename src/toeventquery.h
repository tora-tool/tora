/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOEVENTQUERY_H
#define TOEVENTQUERY_H

#include "config.h"
#include "toconnection.h"
#include "toeventquerytask.h"

#include <QObject>
#include <QPointer>

class toResultStats;


/**
 * Run a query in the background without blocking. This class should
 * always be in the main thread, it uses toEventQueryTask to actually
 * run the sql.
 *
 */
class toEventQuery : public QObject {
    Q_OBJECT;

private:

    ValuesList Values;

    // SQL to execute.
    QString SQL;

    // Bind parameters
    toQList Param;

    // Number of rows processed.
    int Processed;

    // Statistics to be used if any.
    QPointer<toResultStats> Statistics;

    // Description of result
    toQDescList Description;

    // Number of columns in Description
    int Columns;

    // QThread instance that will do actual reading
    QPointer<toEventQueryTask> Task;

    // true when task finishes. this set from a slot called by a
    // queued message. the problem is that sometimes the thread can
    // finish before it's messages have all been processed, making
    // Task->isRunning() useless for detecting if there's data
    // waiting.
    volatile bool TaskDone;

    // connection for this query
    toConnection *Connection;

public:

    /** 
     * Create a new query.
     *
     * @param conn Connection to run on.
     * @param sql SQL to execute.
     * @param param Parameters to pass to query.
     * @param statistics Optional statistics widget to update with values from query.
     */
    toEventQuery(toConnection &conn,
                 const QString &sql,
                 const toQList &param,
                 toResultStats *statistics = NULL);


    /**
     * Create a new query.
     *
     * @param conn Connection to run on.
     * @param mode Query mode to execute query in.
     * @param sql SQL to execute.
     * @param param Parameters to pass to query.
     * @param statistics Optional statistics widget to update with
     * values from query.
     */
    toEventQuery(toConnection &conn,
                   toQuery::queryMode mode,
                   const QString &sql,
                   const toQList &param,
                   toResultStats *statistics = NULL);

    /**
     * Undefined copy contructor. Don't copy me.
     *
     */
    toEventQuery(toEventQuery &other);


    virtual ~toEventQuery();


    /**
     * Start the query. Must be called prior to any other function.
     *
     */
    void start(void);


    /**
     * Get description of columns.
     *
     * @return Description of columns list.
     */
    inline toQDescList describe(void) const {
        return Description;
    }


    /**
     * Get column count
     *
     */
    inline int columns(void) const {
        return Columns;
    }


    /**
     * Read the next value from the query.
     *
     * @return The next available value.
     */
    toQValue readValue(void);


    /**
     * Read the next value from the query. Don't send NULL as string.
     *
     * @return The next available value.
     */
    toQValue readValueNull(void);


    /**
     * Get the number of rows processed.
     *
     * @return Number of rows processed.
     */
    inline int rowsProcessed(void) const {
        return Processed;
    }


    /**
     * Check if at end of query.
     *
     * @return True if query is done.
     */
    bool eof(void) const;


    /**
     * return query's sql command
     *
     */
    inline const QString sql(void) const {
        return SQL;
    }


    /**
     * Returns true if more data is available for readValue()
     *
     */
    bool hasMore(void) const {
        return !Values.isEmpty();
    }


private slots:

    // handle tasks's data() signal. emits dataAvailable()
    void taskData(ValuesList &values);

    // handle tasks's headers() signal emits descriptionAvailable()
    void taskDesc(toQDescList &desc, int columns);

    // handle tasks's error() signal
    void taskError(const toConnection::exception &msg);

    // handle task finished
    void taskFinished(void);

public slots:

    /**
     * Stop reading query
     *
     */
    void stop(void);


    /**
     * Read all data
     *
     */
    void readAll(void) {
        if(Task)
            Task->read(true);
    }


signals:


    /**
     * Emitted when header descriptions are available
     *
     */
    void descriptionAvailable();


    /**
     * Emitted when data has been read.
     *
     * @param rows Number of rows to be read
     */
    void dataAvailable();


    /**
     * Emitted with error string
     *
     */
    void error(const toConnection::exception &);


    /**
     * Emitted when done
     *
     */
    void done();
};

#endif
