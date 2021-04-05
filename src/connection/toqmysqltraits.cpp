
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

#include "connection/toqmysqltraits.h"
#include "core/toconnectionsubloan.h"
#include "core/toquery.h"

QString toQMySqlTraits::quote(const QString &name) const
{
    return name;
}

QString toQMySqlTraits::unQuote(const QString &name) const
{
    if (name.at(0).toLatin1() == '`' && name.at(name.length() - 1).toLatin1() == '`')
        return name.left(name.length() - 1).right(name.length() - 2);
    else
        return name;
}

QString toQMySqlTraits::schemaSwitchSQL(const QString &schema) const
{
    static const QString USE_DATABASE("USE `%1`");
    return USE_DATABASE.arg(schema);
}

QList<QString> toQMySqlTraits::primaryKeys(toConnection &conn, toCache::ObjectRef const&obj) const
{
    toCache::CacheEntry const* e = conn.getCache().findEntry(obj);
    if (!e || e->type != toCache::TABLE )
        return QList<QString>();

    // Cache is used because the function is called twice for the same table
    static QString cached_table_schema;
    static QString cached_table_name;
    static QList<QString> cached_pk;

    if (obj.owner() == cached_table_schema && obj.name() == cached_table_name)
        return cached_pk;

    cached_table_schema = obj.owner();
    cached_table_name = obj.name();
    cached_pk.clear();

    toConnectionSubLoan loan(conn);
    toQuery QueryR(loan,
        "SELECT column_name"
        "  FROM information_schema.key_column_usage"
        " WHERE table_schema = :f1<char[101]>"
        "   AND table_name = :f2<char[101]>"
        "   AND constraint_name = 'PRIMARY'"
        " ORDER BY ordinal_position",
        toQueryParams() << obj.owner() << obj.name());
    while (!QueryR.eof())
        cached_pk << QueryR.readValue();

    return cached_pk;
}
