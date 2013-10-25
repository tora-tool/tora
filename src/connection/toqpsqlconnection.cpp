
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

queryImpl* toQPSqlConnectionSub::createQuery(toQuery *query)
{
	return new psqlQuery(query, this);
}
