
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

#pragma once

#include "core/tora_export.h"
#include "core/tocache.h"
#include "core/toqvalue.h"
#include "core/toqueryimpl.h"
#include "core/toconnection.h"
#include "core/toconnectionsubloan.h"

class toConnection;
class toConnectionSub;
class toConnectionSubLoan;
class toSQL;
class queryImpl;

#include <QtCore/QPointer>

/** This class is used to perform a query on a database connection.
 *  Runs synchronously in foreground thread
 */
class toQueryAbstr : public QObject
{
        Q_OBJECT;
        friend class queryImpl;

    public:
        struct HeaderDesc
        {
            QString           name;        /* column name */
            QString           name_orig;   /* if this starts with space column if hidden */
            QString           datatype;    /* data type */
            bool              hidden;
        };

        typedef QList<toQValue> Row;
        typedef QList<Row> RowList;
        struct HeaderList: public QList<HeaderDesc>
        {
            // return column index from header name
            int indexOf(QString const& name) const
            {
                for (int i = 0; i < size(); i++)
                {
                    if (this->at(i).name_orig == name)
                        return i;
                }
                for (int i = 0; i < size(); i++)
                {
                    if (this->at(i).name_orig.contains(name))
                        return i;
                }
                return -1;
            }
        };

        /** Create a normal query.
         * @param conn Connection to create query on.
         * @param sql SQL to run.
         * @param params Parameters to pass to query.
         */
        toQueryAbstr(toConnectionSubLoan &conn, const toSQL &sql, toQueryParams const &params);

        /** Create a normal query.
         * @param conn Connection to create query on.
         * @param sql SQL to run.
         * @param params Parameters to pass to query.
         */
        //toQuery(toConnection &conn, const QString &sql, toQueryParams const &params);
        toQueryAbstr(toConnectionSubLoan &conn, const QString &sql, toQueryParams const &params);

        /** Destroy query.
         */
        virtual ~toQueryAbstr();

        // GETTERS

        /** Connection object of this object. */
        inline toConnection const& connection(void)
        {
            return m_ConnectionSubLoan.ParentConnection;
        }

        /** Actual database connection that this query is currently using. */
        inline toConnectionSubLoan& connectionSub(void)
        {
            return m_ConnectionSubLoan;
        }

        inline toConnectionSub* connectionSubPtr(void)
        {
            return m_ConnectionSubLoan.ConnectionSub;
        }

        /** Parameters of the current query. */
        inline toQueryParams & params(void)
        {
            return m_Params;
        }

        /** SQL to run. Observe that this string is in UTF8 format. */
        inline QString const& sql(void) const
        {
            return m_SQL;
        }

        /** Read a value from the query.
         * @return Value read.
         */
        toQValue readValue(void);

        /** Check if end of query is reached.
         * @return True if end of query is reached.
         */
        bool eof(void);

        /** Get the number of rows processed by the query. */
        inline unsigned long rowsProcessed(void)
        {
            if (m_Query)
                return m_Query->rowsProcessed();
            else
                return m_rowsProcessed;
        }

        /** Get a list of descriptions for the columns. This function is relatively slow. */
        toQColumnDescriptionList describe(void);

        /** Get the number of columns in the resultset of the query.*/
        inline unsigned columns(void) const
        {
            return m_Query->columns();
        }

    protected:
        toConnectionSub* sub()
        {
            return m_ConnectionSubLoan.ConnectionSub;
        }

        virtual void init() = 0;

        toConnectionSubLoan& m_ConnectionSubLoan;
        toQueryParams m_Params;
        QString m_SQL;
        QString m_SQLName;
        bool m_eof;
        unsigned long m_rowsProcessed;

        queryImpl *m_Query;
        toQueryAbstr(const toQuery &);
};

class toQuery : public toQueryAbstr
{
public:
	toQuery(toConnectionSubLoan &conn, const toSQL &sql, toQueryParams const& params)
		: toQueryAbstr(conn, sql, params)
	{
	    init();
	}

	toQuery(toConnectionSubLoan &conn, QString const& sql, toQueryParams const& params)
		: toQueryAbstr(conn, sql, params)
	{
		init();
	}

    /** Execute a query and return all the values returned by it.
     * @param conn Connection to run query on.
     * @param sql SQL to run.
     * @param params Parameters to pass to query.
     * @return A list of @ref toQValue(s) read from the query.
     */
    static std::list<toQValue> readQuery(toConnection &conn, const toSQL &sql, toQueryParams const &params);

    /** Execute a query and return all the values returned by it.
     * @param conn Connection to run query on.
     * @param sql SQL to run.
     * @param params Parameters to pass to query.
     * @return A list of @ref toQValues:s read from the query.
     */
    static std::list<toQValue> readQuery(toConnection &conn, const QString &sql, toQueryParams const &params);

protected:
    void init() override;
private:
    /** This class contains a reference onto loaned connection
     * therefore an instance of toQuery it MUST not live longer
     * than instance of toConnectionSubLoan
     *
     * An instance of toQuery can be allocated on stack only.
     */
    void *operator new(size_t);
    void *operator new[](size_t);
};

Q_DECLARE_METATYPE(toQueryAbstr::HeaderDesc);
