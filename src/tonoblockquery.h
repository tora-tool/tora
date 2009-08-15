
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

#ifndef TONOBLOCKQUERY_H
#define TONOBLOCKQUERY_H

#include "config.h"
#include "toconnection.h"
#include "tothread.h"

#include <QObject>
#include <time.h>

class toResultStats;

/** This is class to be able to run a query in the background without
 * blocking until a response is available from OCI.
 */

class toNoBlockQuery : public QObject
{
    Q_OBJECT;

private:
    /** A task to implement running the query.
     */
class queryTask : public toTask
    {
        toNoBlockQuery &Parent;
    public:
        queryTask(toNoBlockQuery &parent)
                : Parent(parent)
        { }
        virtual void run(void);
    };
    friend class queryTask;

    /** This semaphore indicates wether the query is still running.
     */
    toSemaphore Running;
    /** This semaphore indicates wether the child thread should
     * continue reading values.
     */
    toSemaphore Continue;
    /** Lock for all this stuff
     */
    toLock Lock;
    /** Current location that values are being read.
     */
    toQList::iterator CurrentValue;
    /** Values read by the task. This can be changed without holding @ref Lock.
     */
    toQList ReadingValues;
    /** Values ready to be read by client.
     */
    toQList Values;
    /** Indicator if at end of query.
     */
    bool EOQ;
    /** Indicator if to quit reading from query.
     */
    bool Quit;
    /** SQL to execute.
     */
    QString SQL;
    /** Error string if error occurs.
     */
    toConnection::exception Error;
    /** Number of rows processed.
     */
    int Processed;
    /** Parameters to pass to query before execution.
     */
    toQList Param;
    /** Statistics to be used if any.
     */
    QPointer<toResultStats> Statistics;
    /** Description of result
     */
    toQDescList Description;
    /** When query is executed
     */
    time_t Started;
    /** Query used to run query
     */
    toQuery *Query;
    /** Throw error if any.
     */
    void checkError();
    /** Stop reading query
     */
    void stop();
public:
    /** Create a new query.
     * @param conn Connection to run on.
     * @param sql SQL to execute.
     * @param param Parameters to pass to query.
     * @param statistics Optional statistics widget to update with values from query.
     */
    toNoBlockQuery(toConnection &conn,
                   const QString &sql,
                   const toQList &param,
                   toResultStats *statistics = NULL);
    /** Create a new query.
     * @param conn Connection to run on.
     * @param mode Query mode to execute query in.
     * @param sql SQL to execute.
     * @param param Parameters to pass to query.
     * @param statistics Optional statistics widget to update with values from query.
     */
    toNoBlockQuery(toConnection &conn,
                   toQuery::queryMode mode,
                   const QString &sql,
                   const toQList &param,
                   toResultStats *statistics = NULL);
    virtual ~toNoBlockQuery();

    /** Poll if any result is available.
     * @return True if at least one row is available.
     */
    bool poll(void);

    /** Get description of columns.
     * @return Description of columns list. Don't modify this list.
     */
    toQDescList &describe(void);

    /** Read the next value from the query.
     * @return The next available value.
     */
    toQValue readValue(void);
    /** Read the next value from the query. Don't send NULL as string.
     * @return The next available value.
     */
    toQValue readValueNull(void);

    /** Get the number of rows processed.
     * @return Number of rows processed.
     */
    int rowsProcessed(void);

    /** Check if at end of query.
     * @return True if query is done.
     */
    bool eof(void);

    /**
     * return query's sql command
     *
     */
    const QString sql(void)
    {
        return SQL;
    }
};

#endif
