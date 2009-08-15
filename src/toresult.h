
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

#ifndef TORESULT_H
#define TORESULT_H

#include "config.h"
#include "toqvalue.h"
#include "tosql.h"

#include <list>

#include <qobject.h>

class QTabWidget;
class toConnection;
class toResult;
class toTimer;

/** Used for getting slots into toResult.
 * @internal
 */

class toResultObject : public QObject
{
    Q_OBJECT

    toResult *Result;
public:
    toResultObject(toResult *result)
            : Result(result)
    { }
public slots:
    /** Called when connection is changed. Be sure to call the parent if you reimplement this.
     */
    virtual void connectionChanged(void);
    /** Called when timer times out
     */
    virtual void setup(void);
};

/** Abstract baseclass of widgets that can perform queries. Usefull because you can execute
 * the query without knowing how it's UI is presented.
 */

class toResult
{
    toResultObject Slots;
    bool Handled;
    QTabWidget *Tabs;

    QWidget *TabWidget;

    QString SQL;
    bool ForceRefresh;
    bool QueryReady;
    toQList Params;
    bool FromSQL;
    QString Name;
    bool DisableTab;

protected:
    /** Set parameters of last query.
     */
    void setParams(const toQList &par)
    {
        Params = par;
        QueryReady = true;
    }
    /** Set SQL and parameters and return false if query shouldn't be reexecuted. It is
     * important that all descendants call this function in the beginning of the implementation
     * of the query function to determine if the query should be reexecuted or not.
     */
    bool setSQLParams(const QString &sql, const toQList &par);
public:
    /** Get the current connection from the closest tool.
     * @return Reference to connection.
     */
    toConnection &connection(void);
    /** Get the timer associated with the closest tool.
     * @return Pointer to tool timer.
     */
    toTimer *timer(void);

    toResult(void);
    virtual ~toResult()
    { }

    /** Erase last parameters
     */
    virtual void clearParams(void)
    {
        Params.clear();
        QueryReady = false;
    }
    /** Re execute last query
     */
    virtual void refresh(void);
    /** Perform a query.
     * @param sql Execute an SQL statement.
     * @param params Parameters needed as input to execute statement.
     */
    virtual void query(const QString &sql, const toQList &params) = 0;
    /** Reexecute with changed parameters.
     * @param Param1 First parameter.
     */
    virtual void changeParams(const QString &Param1);
    /** Reexecute with changed parameters.
     * @param Param1 First parameter.
     * @param Param1 Second parameter.
     */
    virtual void changeParams(const QString &Param1, const QString &Param2);
    /** Reexecute with changed parameters.
     * @param Param1 First parameter.
     * @param Param2 Second parameter.
     * @param Param3 Third parameter.
     */
    virtual void changeParams(const QString &Param1, const QString &Param2, const QString &Param3);
    /** Reimplemented for internal reasons.
     */
    virtual bool canHandle(toConnection &)
    {
        return false;
    }
    /** Get last parameters used.
     * @return Don't modify the list returned.
     */
    toQList &params(void)
    {
        return Params;
    }
    /** Check if this result is handled by the current connection
     */
    bool handled(void)
    {
        return Handled;
    }
    /** Indicate that this result can not be handled even though indicated otherwise
     */
    void setHandle(bool);
    /** Indicate what tabbed widget to disable page if not immediate parent if this
     * result is not handled.
     */
    void setTabWidget(QTabWidget *tab)
    {
        Tabs = tab;
    }

    /** disable hiding of the tab this result object lives in.
     *
     */
    void setDisableTab(bool en)
    {
        DisableTab = en;
    }

    /** Set the SQL statement of this list
     * @param sql String containing statement.
     */
    void setSQL(const QString &sql)
    {
        SQL = sql;
    }
    /** Set the SQL statement of this list. This will also affect @ref Name.
     * @param sql SQL containing statement.
     */
    void setSQL(const toSQL &sql);
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const QString &sql);
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const toSQL &sql);
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const toSQL &sql, toQList &par);
    /** Get SQL to execute
     */
    QString sql(void)
    {
        return SQL;
    }

    /** Get SQL name of list.
     */
    virtual QString sqlName(void)
    {
        return Name;
    }
    /** Set SQL name of list.
     */
    virtual void setSQLName(const QString &name)
    {
        Name = name;
    }

    friend class toResultObject;
protected:
    /** Called when connection is changed. Be sure to call the parent if you reimplement this.
     */
    virtual void connectionChanged(void);
private:
    void changeHandle(void);
};

#endif
