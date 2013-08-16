
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#ifdef HAVE_DB2

#include "todb2connection.h"
#include "db2connectionsub.h"
#include "db2query.h"
#include "db2provider.h"

#include <QtDebug>

DB2Connection::DB2Connection(toConnection *conn)
    : toConnection::connectionImpl(conn),
      Connection(conn)
{
}

void DB2Connection::commit(toConnectionSub * sub)
{
    qDebug() << "conn commit";
    DB2ConnectionSub * db2 = toDB2ConnectionSub(sub);
    if (!db2)
        throw "DB2 internal error: 01 Cannot convert common sub to DB2";
    else
        db2->commit();
}

void DB2Connection::rollback(toConnectionSub * sub)
{
    qDebug() << "conn rollback";
    DB2ConnectionSub * db2 = toDB2ConnectionSub(sub);
    if (!db2)
        throw "DB2 internal error: 02 Cannot convert common sub to DB2";
    else
        db2->rollback();
}

toConnectionSub * DB2Connection::createConnection(void)
{
    return new DB2ConnectionSub(connection());
}

void DB2Connection::closeConnection(toConnectionSub * sub)
{
    qDebug() << "conn close";
    delete sub;
}

QString DB2Connection::version(toConnectionSub * sub)
{
    DB2ConnectionSub * db2 = toDB2ConnectionSub(sub);
    if (!db2)
        throw "DB2 internal error: 03 Cannot convert common sub to DB2";
    return db2 ? db2->version() : "n/a";
}

toQuery::queryImpl * DB2Connection::createQuery(toQuery *query, toConnectionSub * sub)
{
    qDebug() << "conn createQuery";
    return new DB2Query(query, sub);
}

void DB2Connection::execute(toConnectionSub *conn, const QString &sql, toQList &params)
{
    qDebug() << "conn execute";
}


DB2ConnectionSub * DB2Connection::toDB2ConnectionSub(toConnectionSub *sub)
{
    DB2ConnectionSub * conn = dynamic_cast<DB2ConnectionSub*>(sub);
    if (!conn)
        throw QString::fromLatin1("Internal error, not a DB2 sub connection");
    return conn;
}

void DB2Connection::handleError(const QString & msg, SQLHANDLE handle, SQLSMALLINT handleType)
{
    SQLCHAR     sqlState[ SQL_SQLSTATE_SIZE + 1 ];
    SQLCHAR     msgText[ SQL_MAX_MESSAGE_LENGTH + 1 ];
    SQLSMALLINT msgTextOutLen = 0;
    SQLINTEGER  nativeErr;
    SQLRETURN   rc;

    rc = SQLGetDiagRec(
             handleType,
             handle,
             (SQLSMALLINT)1,     /* RecNumber */
             sqlState,
             &nativeErr,
             msgText,
             sizeof(msgText),
             &msgTextOutLen);

    QString err("DB2: %1 - %2 - %3: %4");
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
        throw err.arg(msg).arg((char*)sqlState).arg(nativeErr).arg((char*)msgText);
    else if (rc == SQL_INVALID_HANDLE)
        throw err.arg(msg).arg("E").arg("I").arg("invalid handle");
    else if (rc == SQL_ERROR)
        throw err.arg(msg).arg("E").arg("I").arg("SQL internal error");
    else
        throw err.arg(msg).arg("I").arg("?").arg("Unknown error when calling SQLGetDiagRec");
}



static DB2Provider db2Provider;

#endif
