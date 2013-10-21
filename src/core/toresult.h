
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

#ifndef TORESULT_H
#define TORESULT_H


#include "core/toqvalue.h"
#include "core/toquery.h"
#include "core/tosql.h"

#include <list>

#include <QtCore/QObject>

class QTabWidget;
class toConnection;
class toResult;
class toTimer;

/** Used for getting slots into toResult.
 * @internal
 *
 * abstract class
 */
class toResultObject : public QObject
{
    Q_OBJECT;

public:
    toResultObject(toResult *result)
        : Result(result)
    { }
public slots:
    /** Called when connection is changed. Be sure to call the parent if you reimplement this.
     */
    virtual void slotConnectionChanged(void);

    /** Called when timer times out
     */
    virtual void setup(void);

private:
    toResult *Result;
};

/** Abstract baseclass of widgets that can perform queries. Useful because you can execute
 * the query without knowing how it's UI is presented.
 */
class toResult
{
    friend class toResultObject;
private:
    /** Perform a query - abstract method, must be re-implemented by subclasses
     * @param sql Execute an SQL statement.
     * @param params Parameters needed as input to execute statement.
     */
    virtual void query(const QString &sql, toQueryParams const& params) = 0;
public:
    toResult(void);

    virtual ~toResult() { }

    /** Erase last parameters
     */
    void clearParams(void);

    /** Re execute last query
     */
    virtual void refresh(void);

    /** Reexecute with changed parameters.
     * @param list of query parameters
     */
    virtual void refreshWithParams(toQueryParams const& params);

    /** Clear result pane. When used in Schema Browser this method will be called when
        nothing is selected in object list.
     */
    virtual void clearData() {}

    /** To be reimplemented for internal reasons. (returns false by default)
     */
    virtual bool canHandle(const toConnection &);

    /** Get last parameters used.
     * @return Don't modify the list returned.
     */
    toQueryParams const& params(void);

    /** Check if this result is handled by the current connection
     */
    bool handled(void);

    /** Indicate that this result can not be handled even though indicated otherwise
     */
    void setHandle(bool);

    /** disable hiding of the tab this result object lives in.
     *
     */
    void setDisableTab(bool en);

    /** Set the SQL statement of this list
     * @param sql String containing statement.
     */
    void setSQL(const QString &sql);

    /** Set the SQL statement of this list. This will also affect @ref Name.
     * @param sql SQL containing statement.
     */
    void setSQL(const toSQL &sql);

    /** Get SQL to execute
     */
    QString sql(void);

    void removeSQL();

    /** Get SQL name of list.
     */
    QString sqlName(void);

    /** Set SQL name of list.
     */
    void setSQLName(const QString &name);

protected:

    /** Get the timer associated with the closest tool.
     * @return Pointer to tool timer.
     * NOTE: can be used only in subclasses who also inherit from QWidget
     */
    toTimer *timer(void);

    // Used only in toResultTableViewEdit
    std::list<QString> PriKeys;

    /** Called when connection is changed. Be sure to call the parent if you reimplement this.
     */
    virtual void connectionChanged(void);

    /** Set parameters of last query.
     */
    void setParams(toQueryParams const& par);

    /** Set SQL and parameters and return false if query shouldn't be reexecuted. It is
     * important that all descendants call this function in the beginning of the implementation
     * of the query function to determine if the query should be reexecuted or not.
     *
     * So far used in:
     * toresultview.cpp
     * toresulttableview.cpp
     * toresultbar.cpp
     * toresultcombo.cpp
     * toresultplan.cpp
     * toresultitem.cpp
     * toresultextract.cpp
     * toresultfield.cpp
     * toresultdepend.cpp
     * toresultstorage.cpp
     */
    bool setSqlAndParams(const QString &sql, toQueryParams const& par);

    /** Get the current connection from the closest tool.
     * @return Reference to connection.
     * NOTE: can be used only in subclasses who also inherit from QWidget
     */
    toConnection &connection(void);

private:
    void changeHandle(void);

    toResultObject Slots;
    QString SQL;
    // Is set to true, if connection was changed
    // This value is returned from setSqlAndParams
    bool NeedsRefresh;
    bool QueryReady;
    toQueryParams Params;
    bool FromSQL;
    QString Name;

    /** is set to true and toResult fails for some reason, the whole tool's Tab is disabled */
    bool IsCriticalTab;

    /** set to true, if something fails
     * for example the current connection does not support the current toSql query
     */
    bool Handled;
};

#endif
