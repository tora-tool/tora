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

#include "connection/toqmysqlconnection.h"
#include "connection/toqsqlconnection.h"
#include "connection/toqmysqlquery.h"
#include "core/tosql.h"

#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

static toSQL SQLVersion("toQSqlConnection:Version",
                        "SHOW VARIABLES LIKE 'version'",
                        "Show version of database, "
                        "last value of first return record of result is used.",
                        "3.0",
                        "QMYSQL");

static toSQL SQLListDatabases("toQSqlConnection:ListDatabases",
                              "show databases where `Database` = :f1<quote> or :f1<quote> = ''",
                              "List the available databases for a mysql connection",
                              "3.0",
                              "QMYSQL");

static toSQL SQLListObjectsDatabase("toQSqlConnection:ListObjectsDatabase",
                                    "show table status from :f1<noquote> where `Name` = :f2<quote> or :f2<quote> = ''",
                                    "Get the available tables for a specific database (MySQL specific, won't work for anything else)",
                                    "3.0",
                                    "QMYSQL");

static toSQL SQLListObjects("toQSqlConnection:ListObjects",
                            "show tables",
                            "Get the available tables for a connection",
                            "3.0",
                            "QMYSQL");

static toSQL SQLConnectionID("toQSqlConnection:ConnectionID",
                             "SELECT connection_id()",
                             "Get a connection ID for a session",
                             "3.23",
                             "QMYSQL");

toQMySqlConnectionImpl::toQMySqlConnectionImpl(toConnection &conn)
	: toQSqlConnectionImpl(conn)
{
}

toConnectionSub *toQMySqlConnectionImpl::createConnection(void)
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
			QString t = toQMySqlConnectionSub::ErrorString(db.lastError());
			QSqlDatabase::removeDatabase(dbName);
			throw t;
		}

		toQMySqlConnectionSub *ret = new toQMySqlConnectionSub(parentConnection(), db, dbName);

		// Try to figure out the connection ID for canceling
		try
		{
			QString sql = SQLConnectionID(parentConnection());
			QSqlQuery query = db.exec(sql);
			if (query.next())
				ret->ConnectionID = query.value(0).toString();
		}
		catch (...)
		{
			TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
		}
		return ret;
}

void toQMySqlConnectionImpl::closeConnection(toConnectionSub *)
{

}

void toQMySqlConnectionSub::throwError(const QString &sql)
{
   LockingPtr<QSqlDatabase> ptr(Connection, Lock);
   throw ErrorString(ptr->lastError(), sql);
}

void toQMySqlConnectionSub::commit(void)
{
    LockingPtr<QSqlDatabase> ptr(Connection, Lock);
    if(!ptr->commit() && HasTransactions)
    {
        ptr.unlock();
        throwError(QString::fromLatin1("COMMIT"));
    }
}

void toQMySqlConnectionSub::rollback(void)
{
    LockingPtr<QSqlDatabase> ptr(Connection, Lock);
    if(!ptr->rollback() && HasTransactions)
    {
        ptr.unlock();
        throwError(QString::fromLatin1("ROLLBACK"));
    }
}

QString toQMySqlConnectionSub::version()
{
    QString ret;
    try
    {
        LockingPtr<QSqlDatabase> ptr(Connection, Lock);

        QSqlQuery query = ptr->exec(toSQL::sql(SQLVersion, ParentConnection));
        if (query.next())
        {
            if (query.isValid())
            {
                QSqlRecord record = query.record();
                QVariant val = query.value(record.count() - 1);
                ret = val.toString().toLatin1();
            }
        }
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
    return ret;
}

queryImpl* toQMySqlConnectionSub::createQuery(toQuery *query)
{
	return new mysqlQuery(query, this);
}

QString toQMySqlConnectionSub::ErrorString(const QSqlError &err, const QString &sql)
{
    QString ret;
    if (err.databaseText().isEmpty())
    {
        if (err.driverText().isEmpty())
            ret = QString::fromLatin1("Unknown error");
        else
            ret = err.driverText();
    }
    else
        ret = err.databaseText();
    if (!sql.isEmpty())
        ret += QString::fromLatin1("\n\n") + sql;
    return ret;
}
