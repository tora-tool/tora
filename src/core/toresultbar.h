/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

#ifndef TORESULTBAR_H
#define TORESULTBAR_H

#include "core/tobarchart.h"
#include "core/toresult.h"

#include <QtGui/QMenu>

#include <time.h>
#include <list>


class toEventQuery;
class toSQL;

/** Display the result of a query in a piechart. The first column of the query should
 * contain the x value and the rest of the columns should be values of the diagram. The
 * legend is the column name. Connects to the tool timer for updates automatically.
 */

class toResultBar : public toBarChart, public toResult
{
private:
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
    toEventQuery *Query;
    unsigned int Columns;

    void query(const QString &sql, toQueryParams const& param, bool first);

public:
    /** Create widget.
     * @param parent Parent of list.
     * @param name Name of widget.
     */
    toResultBar(QWidget *parent, const char *name = NULL);

    /** Destroy chart
     */
    ~toResultBar();

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

    /** Reimplemented abstract method
     */
    virtual void query(const QString &sql, toQueryParams const& param)
    {
        query(sql, param, true);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void clear(void)
    {
        LastStamp = 0;
        LastValues.clear();
        toBarChart::clear();
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
    virtual bool canHandle(const toConnection &)
    {
        return true;
    }

public slots:
    /** Reimplemented for internal reasons.
     */
    virtual void refresh(void)
    {
        query(sql(), params(), false);
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
    void queryDone(void);
};

#endif
