
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

#ifndef __QSQL_PROVIDER__
#define __QSQL_PROVIDER__

#include "core/toconnectionprovider.h"
#include "core/toconnectiontraits.h"
#include "connection/absfact.h"

#define QSQL_FINDER     "QSQL"
#define MYSQL_PROVIDER "MySQL (QSql)"
#define PGSQL_PROVIDER "PgSQL (QSql)"
#define ODBC_PROVIDER "ODBC (QSql)"
#define QT_MYSQL_DRIVER "QMYSQL"
#define QT_PGSQL_DRIVER "QPSQL"
#define QT_ODBC_DRIVER  "QODBC"

class toQSqlProvider : public  toConnectionProvider
{
    public:
        toQSqlProvider(toConnectionProviderFinder::ConnectionProvirerParams const& p);

        /** see: @ref toConnectionProvider::initialize() */
        bool initialize() override;

        /** see: @ref toConnectionProvider::name() */
        QString const& name() const = 0;

        /** see: @ref toConnectionProvider::hosts() */
        QList<QString> hosts() const override;

        /** see: @ref toConnectionProvider::databases() */
        QList<QString> databases(const QString &host, const QString &user, const QString &pwd) const override;

        /** see: @ref toConnectionProvider::options() */
        QList<QString> options() const override;

        /** see: @ref toConnectionProvider::configurationTab() */
        QWidget *configurationTab(QWidget *parent) override;

        /** see: @ref toConnection */
        toConnection::connectionImpl* createConnectionImpl(toConnection&) override;

        /** see: @ref toConnection
         * TODO used only by toQODBCProvider (implement this in toQODBCProvider)
         */
        toConnectionTraits* createConnectionTrait(void);
};

class toQSqlTraits: public toConnectionTraits
{
    public:
        /** Return a string representation to address an object.
        * @param name The name to be quoted.
        * @param quoteLowercase Enclose in quotes when identifier has lowercase letters
        * @return String addressing table.
        */
        virtual QString quote(const QString &name) const
        {
            return name;
        }

        /** Perform the opposite of @ref quote.
        * @param name The name to be un-quoted.
        * @return String addressing table.
        */
        virtual QString unQuote(const QString &name) const
        {
            return name;
        }

        virtual QString schemaSwitchSQL(QString const&) const
        {
            return "";
        }

        /** Check if connection provider supports table level comments.
         *  @return bool return true if database supports table level comments
         *  See toSql: toResultCols:TableComment
         */
        virtual bool hasTableComments() const
        {
            return false;
        }

        virtual bool hasAsyncBreak() const
        {
            return false;
        }
};

#endif
