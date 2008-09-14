
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
