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

#include "toquery.h"
#include "toconnection.h"
#include "toconnectionpool.h"
#include "tosql.h"
#include "utils.h"
#include <QApplication>

toQuery::toQuery(toConnection &conn,
                 const toSQL &sql,
                 const QString &arg1,
                 const QString &arg2,
                 const QString &arg3,
                 const QString &arg4,
                 const QString &arg5,
                 const QString &arg6,
                 const QString &arg7,
                 const QString &arg8,
                 const QString &arg9)
    : Connection(QPointer<toConnection>(&conn)),
      ConnectionSub(conn.pooledConnection()),
      SQL(sql(*Connection).toAscii())
{
    Mode = Normal;
    showBusy = true;
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    if (numArgs > 0)
        Params.insert(Params.end(), arg1);
    if (numArgs > 1)
        Params.insert(Params.end(), arg2);
    if (numArgs > 2)
        Params.insert(Params.end(), arg3);
    if (numArgs > 3)
        Params.insert(Params.end(), arg4);
    if (numArgs > 4)
        Params.insert(Params.end(), arg5);
    if (numArgs > 5)
        Params.insert(Params.end(), arg6);
    if (numArgs > 6)
        Params.insert(Params.end(), arg7);
    if (numArgs > 7)
        Params.insert(Params.end(), arg8);
    if (numArgs > 8)
        Params.insert(Params.end(), arg9);

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn,
                 const QString &sql,
                 const QString &arg1,
                 const QString &arg2,
                 const QString &arg3,
                 const QString &arg4,
                 const QString &arg5,
                 const QString &arg6,
                 const QString &arg7,
                 const QString &arg8,
                 const QString &arg9)
    : Connection(QPointer<toConnection>(&conn)),
      ConnectionSub(conn.pooledConnection()),
      SQL(sql)
{
    Mode = Normal;
    showBusy = true;
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    if (numArgs > 0)
        Params.insert(Params.end(), arg1);
    if (numArgs > 1)
        Params.insert(Params.end(), arg2);
    if (numArgs > 2)
        Params.insert(Params.end(), arg3);
    if (numArgs > 3)
        Params.insert(Params.end(), arg4);
    if (numArgs > 4)
        Params.insert(Params.end(), arg5);
    if (numArgs > 5)
        Params.insert(Params.end(), arg6);
    if (numArgs > 6)
        Params.insert(Params.end(), arg7);
    if (numArgs > 7)
        Params.insert(Params.end(), arg8);
    if (numArgs > 8)
        Params.insert(Params.end(), arg9);

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, const toSQL &sql, const toQList &params)
    : Connection(QPointer<toConnection>(&conn)),
      ConnectionSub(conn.pooledConnection()),
      Params(params),
      SQL(sql(conn).toAscii())
{
    Mode = Normal;
    showBusy = true;
    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

// for testing sub
toQuery::toQuery(toConnection &conn,
                 toConnectionSub *sub,
                 const QString &sql,
                 const toQList &params)
    : Connection(QPointer<toConnection>(&conn)),
      ConnectionSub(sub),
      Params(params),
      SQL(sql)
{
    Mode = Test;
    showBusy = true;
    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, const QString &sql, const toQList &params)
    : Connection(QPointer<toConnection>(&conn)),
      ConnectionSub(conn.pooledConnection()),
      Params(params),
      SQL(sql)
{
    Mode = Normal;
    showBusy = true;
    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn,
                 queryMode mode,
                 const toSQL &sql,
                 const toQList &params)
    : Connection(QPointer<toConnection>(&conn)),
      Params(params),
      SQL(sql(conn).toAscii())
{
    Mode = mode;
    showBusy = true;

    ConnectionSub = conn.pooledConnection();

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn,
                 queryMode mode,
                 const QString &sql,
                 const toQList &params)
    : Connection(QPointer<toConnection>(&conn)),
      Params(params),
      SQL(sql)
{
    Mode = mode;
    showBusy = true;

    ConnectionSub = conn.pooledConnection();

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, queryMode mode)
    : Connection(QPointer<toConnection>(&conn))
{
    Mode = mode;
    showBusy = true;

    ConnectionSub = conn.pooledConnection();

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        throw;
    }
    ConnectionSub->setQuery(this);
}

void toQuery::execute(const toSQL &sql, const toQList &params)
{
    toBusy busy;
    SQL = sql(*Connection);
    Params = params;
    Query->execute();
    this->connectionSub()->setLastUsed();
}

void toQuery::execute(const QString &sql, const toQList &params)
{
    toBusy busy;
    SQL = sql;
    Params = params;
    Query->execute();
    this->connectionSub()->setLastUsed();
}

void toQuery::execute(const toSQL &sql)
{
    std::list<toQValue> params;
    params.clear();
    execute(sql, params);
    this->connectionSub()->setLastUsed();
}

void toQuery::execute(const toSQL &sql, const QString &param)
{
    std::list<toQValue> params;
    params.clear();
    params.insert(params.end(), param);
    execute(sql, params);
    this->connectionSub()->setLastUsed();
}

void toQuery::execute(const toSQL &sql, const QString &param1, const QString &param2, const QString &param3)
{
    std::list<toQValue> params;
    params.clear();
    params.insert(params.end(), param1);
    params.insert(params.end(), param2);
    params.insert(params.end(), param3);
    execute(sql, params);
    this->connectionSub()->setLastUsed();
}

toQuery::~toQuery()
{
    toBusy busy;
    if (Query)
        delete Query;
    try
    {
        if (ConnectionSub->query() == this)
            ConnectionSub->setQuery(NULL);
        if (Mode != Test && Connection && Connection->ConnectionPool)
            Connection->ConnectionPool->release(ConnectionSub);
    }
    catch (...) {}
}

bool toQuery::eof(void)
{
    return Query->eof();
}

toQList toQuery::readQuery(toConnection &conn, const toSQL &sql, toQList &params)
{
    toBusy busy;
    toQuery query(conn, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQuery(toConnection &conn, const QString &sql, toQList &params)
{
    toBusy busy;
    toQuery query(conn, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQuery(const QString &sql, toQList &params)
{
    toBusy busy(showBusy);
    SQL = sql;
    Params = params;
    Query->execute();
    toQList ret;
    while (!eof())
        ret.insert(ret.end(), readValue());
    return ret;
}

toQList toQuery::readQuery(toConnection &conn, const toSQL &sql,
                           const QString &arg1, const QString &arg2,
                           const QString &arg3, const QString &arg4,
                           const QString &arg5, const QString &arg6,
                           const QString &arg7, const QString &arg8,
                           const QString &arg9)
{
    toBusy busy;
    toQuery query(conn, sql, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQuery(toConnection &conn, const QString &sql,
                           const QString &arg1, const QString &arg2,
                           const QString &arg3, const QString &arg4,
                           const QString &arg5, const QString &arg6,
                           const QString &arg7, const QString &arg8,
                           const QString &arg9)
{
    toBusy busy;
    toQuery query(conn, sql, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQValue toQuery::readValue(void)
{
    if (!Connection)
        return toQValue(0);

    if (Connection->Abort)
        throw qApp->translate("toQuery", "Query aborted");
    return Query->readValue();
}

void toQuery::cancel(void)
{
    Query->cancel();
}
