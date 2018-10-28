
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

#include "core/toconnectionsubloan.h"
#include "core/toconnectionsub.h"
#include "core/toconnection.h"
#include "core/toquery.h"
#include "core/tosql.h"

toConnectionSubLoan::toConnectionSubLoan(toConnection &con)
    : ParentConnection(con)
    , SchemaInitialized(false)
    , ConnectionSub(con.borrowSub())
{}

toConnectionSubLoan::toConnectionSubLoan(toConnection &con, QString const & schema)
    : ParentConnection(con)
    , SchemaInitialized(false)
    , Schema(schema)
    , ConnectionSub(con.borrowSub())
{
    Q_ASSERT_X(!schema.isEmpty(), qPrintable(__QHERE__), "schema is empty");
    SchemaInitialized = ConnectionSub->schema() == schema;
}

/** This special kind of constructor is used by @ref toQuery while testing the connections*/
toConnectionSubLoan::toConnectionSubLoan(toConnection &con, int*)
    : ParentConnection(con)
    , SchemaInitialized(false)
    , ConnectionSub(NULL)
{}

toConnectionSubLoan::~toConnectionSubLoan()
{
    if (ConnectionSub)
    {
        const_cast<toConnection&>(ParentConnection).putBackSub(ConnectionSub);
    }
}

void toConnectionSubLoan::execute(QString const &SQL)
{
    toQuery query(*this, SQL, toQueryParams());
    query.eof();
}

void toConnectionSubLoan::execute(toSQL const &SQL)
{
    toQuery query(*this, SQL, toQueryParams());
    query.eof();
}

void toConnectionSubLoan::setInit(const QString &key, const QString &sql)
{
    InitStrings.insert(key, sql);
}

void toConnectionSubLoan::delInit(const QString &key)
{
    InitStrings.remove(key);
}

QList<QString> toConnectionSubLoan::initStrings() const
{
    return InitStrings.values();
}
