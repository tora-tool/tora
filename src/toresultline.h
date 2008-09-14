/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTLINE_H
#define TORESULTLINE_H

#include "config.h"
#include "tobackground.h"
#include "tolinechart.h"
#include "toresult.h"

#include <time.h>

#include <list>

class QMenu;
class toNoBlockQuery;
class toSQL;


/** Display the result of a query in a piechart. The first column of the query should
 * contain the x value and the rest of the columns should be values of the diagram. The
 * legend is the column name. Connects to the tool timer for updates automatically.
 */

class toResultLine : public toLineChart, public toResult
{
    Q_OBJECT
    /** Display flow in change per second instead of actual values.
     */
    bool Flow;
    bool Started;
    /** Timestamp of last fetch.
     */
    time_t LastStamp;
    /** Last read values.
     */
    std::list<double> LastValues;
    bool First;
    toNoBlockQuery *Query;
    toBackground Poll;
    unsigned int Columns;
    void query(const QString &sql, const toQList &param, bool first);
public:
    /** Create widget.
     * @param parent Parent of list.
     * @param name Name of widget.
     */
    toResultLine(QWidget *parent, const char *name = NULL);
    /** Destroy chart
     */
    ~toResultLine();

    /** Stop automatic updating from tool timer.
     */
    void stop();
    /** Start automatic updating from tool timer.
     */
    void start();

    /** Display actual values or flow/s.
     * @param on Display flow or absolute values.
     */
    void setFlow(bool on)
    {
        Flow = on;
    }
    /** Return if flow is displayed.
     * @return If flow is used.
     */
    bool flow(void)
    {
        return Flow;
    }

    /** Reimplemented for internal reasons.
     */
    virtual void query(const QString &sql, const toQList &param)
    {
        query(sql, param, true);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void clear(void)
    {
        LastStamp = 0;
        LastValues.clear();
        toLineChart::clear();
    }
    /** Transform valueset. Make it possible to perform more complex transformation.
     * called directly before adding the valueset to the chart. After flow transformation.
     * Default is passthrough.
     * @param input The untransformed valueset.
     * @return The valueset actually added to the chart.
     */
    virtual std::list<double> transform(std::list<double> &input);
    /** Handle any connection
     */
    virtual bool canHandle(toConnection &)
    {
        return true;
    }

    // Why are these needed?
#if 1
    /** Set the SQL statement of this list
     * @param sql String containing statement.
     */
    void setSQL(const QString &sql)
    {
        toResult::setSQL(sql);
    }
    /** Set the SQL statement of this list. This will also affect @ref Name.
     * @param sql SQL containing statement.
     */
    void setSQL(const toSQL &sql)
    {
        toResult::setSQL(sql);
    }
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const QString &sql)
    {
        toResult::query(sql);
    }
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const toSQL &sql)
    {
        toResult::query(sql);
    }
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const toSQL &sql, toQList &par)
    {
        toResult::query(sql, par);
    }
#endif
public slots:
    /** Reimplemented for internal reasons.
     */
    virtual void refresh(void)
    {
        query(sql(), params(), false);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void changeParams(const QString &Param1)
    {
        toResult::changeParams(Param1);
    }
    /** Reimplemented For internal reasons.
     */
    virtual void changeParams(const QString &Param1, const QString &Param2)
    {
        toResult::changeParams(Param1, Param2);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void changeParams(const QString &Param1, const QString &Param2, const QString &Param3)
    {
        toResult::changeParams(Param1, Param2, Param3);
    }
protected slots:
    /** Reimplemented for internal reasons.
     */
    virtual void connectionChanged(void);
    /** Reimplemented for internal reasons.
     */
    virtual void addMenues(QMenu *);
private slots:
    void poll(void);
    void editSQL(void);
};

#endif
