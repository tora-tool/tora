/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-20010 Numerous Other Contributors
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

#ifndef TOQUERY_H
#define TOQUERY_H

#include "tocache.h"
#include "toqueryimpl.h"
#include "toqvalue.h"
#include "toconnection.h"

class toConnection;
class toConnectionSub;
class toSQL;

/** This class is used to perform a query on a database connection.
 */
class toQuery : public QObject
{
    Q_OBJECT;

public:
    /** Represent different modes to run a query in.
     */
    enum queryMode
    {
        /** Run the query normally on the main connection of the @ref toConnection object.
         */
        Normal,
        /** Run the query normally on the main backgrround connection of the
         * @ref toConnection object. This can be the same as the main connection depending
         * on settings.
         */
        Background,
        /** Run the query in a separate connection for long running queries.
         */
        Long,
        /** Run the query on all non occupied connections of the @ref toConnection object.
         */
        All,
        /** For internal use, doesn't close resources
         */
        Test
    };

private:
    QPointer<toConnection> Connection;
    toConnectionSub *ConnectionSub;
    std::list<toQValue> Params;
    QString SQL;
    queryMode Mode;
    bool showBusy; // does a "busy" indicator has to be shown while query is running (default - true)

    queryImpl *Query;
    toQuery(const toQuery &);
public:
    /** Create a normal query.
     * @param conn Connection to create query on.
     * @param sql SQL to run.
     * @param params Parameters to pass to query.
     */
    toQuery(toConnection &conn,
            const toSQL &sql,
            const std::list<toQValue> &params);

    /** Create a normal query.
     * @param conn Connection to create query on.
     * @param sql SQL to run.
     * @param params Parameters to pass to query.
     */
    toQuery(toConnection &conn,
            const QString &sql,
            const std::list<toQValue> &params);

    /** Create a normal query.
     * @param conn Connection to create query on.
     * @param sql SQL to run.
     * @param arg1 Arguments to pass to query.
     */
    toQuery(toConnection &conn,
            const toSQL &sql,
            const QString &arg1 = QString::null,
            const QString &arg2 = QString::null,
            const QString &arg3 = QString::null,
            const QString &arg4 = QString::null,
            const QString &arg5 = QString::null,
            const QString &arg6 = QString::null,
            const QString &arg7 = QString::null,
            const QString &arg8 = QString::null,
            const QString &arg9 = QString::null);
    /** Create a normal query.
     * @param conn Connection to create query on.
     * @param sql SQL to run.
     * @param arg1 Arguments to pass to query.
     */
    toQuery(toConnection &conn,
            const QString &sql,
            const QString &arg1 = QString::null,
            const QString &arg2 = QString::null,
            const QString &arg3 = QString::null,
            const QString &arg4 = QString::null,
            const QString &arg5 = QString::null,
            const QString &arg6 = QString::null,
            const QString &arg7 = QString::null,
            const QString &arg8 = QString::null,
            const QString &arg9 = QString::null);

    /** Create a query.
     * @param conn Connection to create query on.
     * @param mode Mode to run query in.
     * @param sql SQL to run.
     * @param params Arguments to pass to query.
     */
    toQuery(toConnection &conn,
            queryMode mode,
            const toSQL &sql,
            const std::list<toQValue> &params);

    /** Internal use. Create a query object to test a
     * toConnectionSub. Does not close sub.
     *
     * @param conn Connection to create query on.
     * @param mode Mode to run query in.
     * @param sql SQL to run.
     * @param params Arguments to pass to query.
     */
    toQuery(toConnection &conn,
            toConnectionSub *sub,
            const QString &sql,
            const std::list<toQValue> &params);

    /** Create a query.
     * @param conn Connection to create query on.
     * @param mode Mode to run query in.
     * @param sql SQL to run.
     * @param params Arguments to pass to query.
     */
    toQuery(toConnection &conn,
            queryMode mode,
            const QString &sql,
            const std::list<toQValue> &params);

    /** Create a query. Don't runn any SQL using it yet. Observe though that the @ref
     * toConnectionSub object is assigned here so you know that all queries run using this
     * query object will run on the same actual connection to the database (Unless mode is All off
     * course).
     * @param conn Connection to create query for.
     * @param mode Mode to execute queries in.
     */
    toQuery(toConnection &conn, queryMode mode = Normal);
    /** Destroy query.
     */
    virtual ~toQuery();

    /** Execute an SQL statement using this query.
     * @param sql SQL to run.
     * @param params Parameters to pass to query.
     */
    void execute(const toSQL &sql, const std::list<toQValue> &params);
    /** Execute an SQL statement using this query.
     * @param sql SQL to run.
     * @param params Parameters to pass to query.
     */
    void execute(const QString &sql, const std::list<toQValue> &params);

    /** Execute an SQL statement with no parameters using this query.
     * @param sql SQL to run.
     */
    void execute(const toSQL &sql);

    /** Execute an SQL statement using this query with String as parameter
     * @param sql SQL to run.
     * @param param Parameter to pass to query
     */
    void execute(const toSQL &sql, const QString &param);

    /** Execute an SQL statement using this query with 3 Strings as parameters
     * @param sql SQL to run.
     * @param param1 1st parameter
     * @param param2 2nd parameter
     * @param param3 3rd parameter
     */
    void execute(const toSQL &sql, const QString &param1, const QString &param2, const QString &param3);

    /** Connection object of this object.
     */
    toConnection &connection(void)
    {
        return *Connection;
    }
    /** Actual database connection that this query is currently using.
     */
    toConnectionSub *connectionSub(void)
    {
        return ConnectionSub;
    }
    /** Parameters of the current query.
     */
    std::list<toQValue> &params(void)
    {
        return Params;
    }
    /** SQL to run. Observe that this string is in UTF8 format.
     */
    QString sql(void)
    {
        return SQL;
    }
    /** Get the mode this query is executed in.
     */
    toQuery::queryMode mode(void) const
    {
        return Mode;
    }

    /** Read a value from the query.
     * @return Value read.
     */
    toQValue readValue(void);
    /** Check if end of query is reached.
     * @return True if end of query is reached.
     */
    bool eof(void);

    /** Get the number of rows processed by the query.
     */
    int rowsProcessed(void)
    {
        return Query->rowsProcessed();
    }
    /** Get a list of descriptions for the columns. This function is relatively slow.
     */
    std::list<toQDescribe> describe(void)
    {
        return Query->describe();
    }
    /** Get the number of columns in the resultset of the query.
     */
    int columns(void)
    {
        return Query->columns();
    }

    /** Execute a query and return all the values returned by it.
     * @param conn Connection to run query on.
     * @param sql SQL to run.
     * @param params Parameters to pass to query.
     * @return A list of @ref toQValues:s read from the query.
     */
    static std::list<toQValue> readQuery(toConnection &conn,
                                         const toSQL &sql,
                                         std::list<toQValue> &params);
    /** Execute a query and return all the values returned by it.
     * @param conn Connection to run query on.
     * @param sql SQL to run.
     * @param params Parameters to pass to query.
     * @return A list of @ref toQValues:s read from the query.
     */
    static std::list<toQValue> readQuery(toConnection &conn,
                                         const QString &sql,
                                         std::list<toQValue> &params);

    /** Execute a query using this oracle session and return all values.
      * @param sql SQL to run.
      * @param params Parameters to pass to query.
      * @return A list of @ref toQValues:s read from query
      */
    std::list<toQValue> readQuery(const QString &sql,
                                  std::list<toQValue> &params);

    /** Execute a query and return all the values returned by it.
     * @param conn Connection to run query on.
     * @param sql SQL to run.
     * @param arg1 Parameters to pass to query.
     * @return A list of @ref toQValues:s read from the query.
     */
    static std::list<toQValue> readQuery(toConnection &conn, const toSQL &sql,
                                         const QString &arg1 = QString::null, const QString &arg2 = QString::null,
                                         const QString &arg3 = QString::null, const QString &arg4 = QString::null,
                                         const QString &arg5 = QString::null, const QString &arg6 = QString::null,
                                         const QString &arg7 = QString::null, const QString &arg8 = QString::null,
                                         const QString &arg9 = QString::null);
    /** Execute a query and return all the values returned by it.
     * @param conn Connection to run query on.
     * @param sql SQL to run.
     * @param arg1 Parameters to pass to query.
     * @return A list of @ref toQValues:s read from the query.
     */
    static std::list<toQValue> readQuery(toConnection &conn, const QString &sql,
                                         const QString &arg1 = QString::null, const QString &arg2 = QString::null,
                                         const QString &arg3 = QString::null, const QString &arg4 = QString::null,
                                         const QString &arg5 = QString::null, const QString &arg6 = QString::null,
                                         const QString &arg7 = QString::null, const QString &arg8 = QString::null,
                                         const QString &arg9 = QString::null);

    /** Cancel the current execution of a query.
     */
    void cancel(void);

    /** Specify if busy cursor must be displayed while a query is running
     */
    void setShowBusy(bool busy)
    {
        showBusy = busy;
    }
};

#endif

