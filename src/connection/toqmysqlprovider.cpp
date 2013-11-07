
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

#include "connection/toqmysqlprovider.h"
#include "connection/toqmysqlsetting.h"
#include "connection/toqmysqltraits.h"
#include "connection/toqsqlconnection.h"
#include "connection/toqsqlprovider.h"
#include "connection/toqmysqlconnection.h"

#define QT_DRIVER_NAME "QMYSQL"

QString toQMySqlProvider::m_name = QT_DRIVER_NAME;
QString toQMySqlProvider::m_displayName = MYSQL_PROVIDER;

toQMySqlProvider::toQMySqlProvider(toConnectionProviderFinder::ConnectionProvirerParams const& p)
	: toQSqlProvider(p)
{}

bool toQMySqlProvider::initialize()
{
	return true;
}

QList<QString> toQMySqlProvider::hosts()
{
    QList<QString> ret = QList<QString>() << QString::null << "localhost";
    return ret;
}

QList<QString> toQMySqlProvider::databases(const QString &host, const QString &user, const QString &pwd)
{
	QList<QString> ret = QList<QString>() << "...";
	return ret;
}

QList<QString> toQMySqlProvider::options()
{
	QList<QString> ret = QList<QString>()
			<< "*SSL"
			<< "*Compress"
            << "-"
            << "Ignore Space"
            << "No Schema";

	return ret;
}

QWidget* toQMySqlProvider::configurationTab(QWidget *parent)
{
	return new toQMySqlSetting(parent);
}

toConnection::connectionImpl* toQMySqlProvider::createConnectionImpl(toConnection &conn)
{
	return new toQMySqlConnectionImpl(conn);
}

toConnectionTraits* toQMySqlProvider::createConnectionTrait(void)
{
	static toQMySqlTraits *t = new toQMySqlTraits();
	return t;
}
