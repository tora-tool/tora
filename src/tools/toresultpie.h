
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTPIE_H
#define TORESULTPIE_H

#include "tools/topiechart.h"
#include "core/toresult.h"

#include <list>

#include <QtCore/QRegExp>

class toEventQuery;
class toSQL;

/** Display the result of a query in a piechart. The first column of the query should
 * contain the value and the second should contain an optional label.
 */
class toResultPie : public toPieChart, public toResult
{
    Q_OBJECT;
public:
    /** Create widget.
     * @param parent Parent of list.
     * @param name Name of widget.
     */
    toResultPie(QWidget *parent, const char *name = NULL);

    /** Reimplemented for internal reasons.
     */
    virtual void query(const QString &sql, const toQueryParams &params);
    virtual bool canHandle(const toConnection &)
    {
        return true;
    }

    /** Stop automatic updating from tool timer.
     */
    void stop();
    /** Start automatic updating from tool timer.
     */
    void start();

    /** Indicate that the first column should be the label.
     */
    void setLabelFirst(bool first)
    {
        LabelFirst = first;
    }
    /** Check label first status.
     */
    bool labelFirst(void)
    {
        return LabelFirst;
    }
    /** Set a filter on which columns to add based on label.
     * @param filter A regexp which the label must match.
     * @param valueFilter A regexp which the value must match.
     */
    void setFilter(const QRegExp &filter, const QRegExp &valueFilter)
    {
        Filter = filter;
        ValueFilter = valueFilter;
    }
    /** Get the current filter.
     */
    const QRegExp &filter(void)
    {
        return Filter;
    }
    /** Get the current value filter.
     */
    const QRegExp &valueFilter(void)
    {
        return ValueFilter;
    }

    // Why are these needed?
#if 0
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

public slots:
    /** Reimplemented for internal reasons.
     */
    virtual void refresh(void)
    {
        toResult::refresh();
    }
    /** Reimplemented for internal reasons.
     */
    virtual void executeWithParams(const QString &Param1)
    {
        toResult::executeWithParams(Param1);
    }
    /** Reimplemented For internal reasons.
     */
    virtual void executeWithParams(const QString &Param1, const QString &Param2)
    {
        toResult::executeWithParams(Param1, Param2);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void executeWithParams(const QString &Param1, const QString &Param2, const QString &Param3)
    {
        toResult::executeWithParams(Param1, Param2, Param3);
    }
#endif

private slots:
    void poll(void);
    void queryDone(void);

private:
    toEventQuery *Query;
    std::list<QString> Labels;
    std::list<double> Values;
    int Columns;
    bool Started;
    bool LabelFirst;
    QRegExp Filter;
    QRegExp ValueFilter;
};

#endif
