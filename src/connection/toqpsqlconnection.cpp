
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

#include "connection/toqpsqlconnection.h"
#include "connection/toqpsqlquery.h"
#include "core/tosql.h"

#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>

#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
#include <libpq-fe.h>
#endif

static toSQL SQLListObjectsDatabase("toConnection:ListObjectsInDatabase",
                                    "select n.nspname, count(o.oid)                    "
                                    "  from pg_namespace n                             "
                                    "  left join pg_class o on n.oid=o.relnamespace    "
                                    " group by 1                                       "
                                    " order by count(o.oid)>0, 1                       ",
                                    "",
                                    "0700",
                                    "QPSQL");

static toSQL SQLConnectioId("toQSqlConnection:ConnectionID",
                            "select pg_backend_pid()                           ",
                            "",
                            "",
                            "QPSQL");


static toSQL SQLVersionPgSQL("toQSqlConnection:Version",
                             "SHOW SERVER_VERSION",
                             "", // "Show version of the database"
                             "",
                             "QPSQL");

static toSQL SQLColumnComments("toQSqlConnection:ColumnComments",
                               "select a.attname,b.description\n"
                               "from\n"
                               "  pg_attribute a,\n"
                               "  pg_description b,\n"
                               "  pg_class c LEFT OUTER JOIN pg_namespace n ON c.relowner=n.oid\n"
                               "where\n"
                               "  a.oid=b.objoid\n"
                               "  and c.oid=a.attrelid\n"
                               "  and (u.nspname = :owner OR u.usesysid IS NULL)\n"
                               "  and c.relname=:table",
                               "Get the available comments on columns of a table, "
                               "must have same binds and columns",
                               "0701",
                               "QPSQL");

static toSQL SQLColumnComments72("toQSqlConnection:ColumnComments",
                                 "select a.attname,b.description\n"
                                 "from\n"
                                 "  pg_attribute a,\n"
                                 "  pg_description b,\n"
                                 "  pg_class c LEFT OUTER JOIN pg_namespace n ON c.relowner=n.oid\n"
                                 "where\n"
                                 "  a.attnum=b.objsubid\n"
                                 "  and b.objoid=a.attrelid\n"
                                 "  and c.oid=a.attrelid\n"
                                 "  and (n.nspname = :owner OR u.usesysid IS NULL)\n"
                                 "  and c.relname=:table",
                                 "",
                                 "0702",
                                 "QPSQL");

// Opening a second connection has drawbacks and can fail to
// successfully cancel queries if there is any problem getting a
// second connection, causing crashes.
static toSQL SQLCancelPg("toQSqlConnection:Cancel",
                         "SELECT pg_cancel_backend(:pid)",
                         "",
                         "0800",
                         "QPSQL");

toQPSqlConnectionImpl::toQPSqlConnectionImpl(toConnection &conn)
    : toQSqlConnectionImpl(conn)
{
}

toConnectionSub *toQPSqlConnectionImpl::createConnection(void)
{
    // TODO shouldn't be this method reenteant?
    static QAtomicInt ID_COUNTER(0);
    int ID = ID_COUNTER.fetchAndAddAcquire(1);

    QString dbName = QString::number(ID);
    QSqlDatabase db = QSqlDatabase::addDatabase(parentConnection().provider(), dbName);
    db.setDatabaseName(parentConnection().database());
    QString host = parentConnection().host();
    int pos = host.indexOf(QString(":"));
    if (pos < 0)
        db.setHostName(host);
    else
    {
        db.setHostName(host.mid(0, pos));
        db.setPort(host.mid(pos + 1).toInt());
    }

    QString opt;

    QSet<QString> options = parentConnection().options();
    if (options.find("Compress") != options.end())
        opt += ";CLIENT_COMPRESS";
    if (options.find("Ignore Space") != options.end())
        opt += ";CLIENT_IGNORE_SPACE";
    if (options.find("No Schema") != options.end())
        opt += ";CLIENT_NO_SCHEMA";
    if (options.find("SSL") != options.end())
        opt += ";CLIENT_SSL";

    if (!opt.isEmpty())
        db.setConnectOptions(opt.mid(1)); // Strip first ; character

    db.open(parentConnection().user(), parentConnection().password());
    if (!db.isOpen())
    {
        QString t = toQPSqlConnectionSub::ErrorString(db.lastError());
        QSqlDatabase::removeDatabase(dbName);
        throw t;
    }

    toQPSqlConnectionSub *ret = new toQPSqlConnectionSub(parentConnection(), db, dbName);
    return ret;
}

void toQPSqlConnectionImpl::closeConnection(toConnectionSub *)
{

}

QString toQPSqlConnectionSub::version()
{
    int ver = nativeVersion();
    QString retval;
    if (ver)
        retval = QString::number(ver);
    else
        retval = super::version();
    QStringList v = retval.split(".");
    if (v.size() >= 2)
    	retval = QString("%1%2").arg(v[0], 2, '0').arg(v[1], 2, '0');
    else
    	retval = QString("0000"); // PostgreSQL version up to 7.4
    TLOG(5, toDecorator, __HERE__) << "QPSQL Connection version: " << retval << std::endl;
    return retval;

}

toQueryParams toQPSqlConnectionSub::sessionId()
{
    int ver = nativeSessionId();
    if (ver)
        return toQueryParams() << QString::number(ver);
    else
        return super::sessionId();
}

queryImpl* toQPSqlConnectionSub::createQuery(toQueryAbstr *query)
{
    return new psqlQuery(query, this);
}

int toQPSqlConnectionSub::nativeVersion()
{
    QVariant v = Connection.driver()->handle();
    if (v.isValid() && v.typeName() == QString("PGconn*"))
    {
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
        PGconn *handle = *static_cast<PGconn **>(v.data());
        if (handle)
            return PQserverVersion(handle);
#endif
    }
    return 0;
}

int toQPSqlConnectionSub::nativeSessionId()
{
    QVariant v = Connection.driver()->handle();
    if (v.isValid() && v.typeName() == QString("PGconn*"))
    {
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
        PGconn *handle = *static_cast<PGconn **>(v.data());
        if (handle)
            return PQbackendPID(handle);
#endif
    }
    return 0;
}

void toQPSqlConnectionSub::nativeCancel()
{
    QVariant v = Connection.driver()->handle();
    if (v.isValid() && v.typeName() == QString("PGconn*"))
    {
#ifdef LIBPQ_DECL_CANCEL
        PGconn *handle = *static_cast<PGconn **>(v.data());
        if (!handle)
            return;

        PGcancel *cancel = PQgetCancel(handle);
        if (!cancel)
            return;

        char *errbuf = new char[1024];
        PQcancel(cancel, errbuf, 1024);
        PQfreeCancel(cancel);
        delete[] errbuf;
#endif
    }
}
