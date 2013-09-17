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

#include "core/toconnectionprovider.h"
#include "core/tologger.h"
#include "connection/toqsqlprovider.h"
#include "connection/toqpsqlprovider.h"
#include "connection/toqmysqlprovider.h"
#include "connection/toqodbcprovider.h"
#include "core/toconfiguration.h"

#include <QtCore/QDir>
#include <QtSql/QSqlDatabase>

class toQSqlFinder : public  toConnectionProviderFinder
{
public:
    inline toQSqlFinder(unsigned int i) : toConnectionProviderFinder(i) {};

    virtual QString name() const
    {
        return QString::fromAscii(QSQL_FINDER);
    };

    /** Return list of possible client locations
     */
    virtual QList<ConnectionProvirerParams> find();

    /**
       Load connection providers library
    */
    virtual void load(ConnectionProvirerParams const&);

};

QList<toConnectionProviderFinder::ConnectionProvirerParams>  toQSqlFinder::find()
{
	QList<ConnectionProvirerParams> retval;
#ifdef Q_OS_WIN
	QString mysqlHome(toConfigurationSingle::Instance().mysqlHome());
	QDir mysqlHomeDir(mysqlHome);
	if(!mysqlHome.isEmpty() && mysqlHomeDir.exists())
		QCoreApplication::addLibraryPath (mysqlHome);

	QString pgsqlHome(toConfigurationSingle::Instance().pgsqlHome());
	QDir pgsqlHomeDir(pgsqlHome);
	if(!pgsqlHome.isEmpty() && pgsqlHomeDir.exists())
	{
		QCoreApplication::addLibraryPath (pgsqlHome + QDir::separator() + "bin");  // libssleay32.dll
		QCoreApplication::addLibraryPath (pgsqlHome + QDir::separator() + "lib");  // libpg.dll
	}
#endif

	QStringList drivers = QSqlDatabase::drivers();

	Q_FOREACH(QString driver, drivers)
	{
		TLOG(5, toNoDecorator, __HERE__) << "QT supported driver:'" << driver << "'" << std::endl;
	}

	if(drivers.contains(QT_MYSQL_DRIVER))
	{
		ConnectionProvirerParams mysql;
		TLOG(5, toNoDecorator, __HERE__) << "Tora supports:'" QT_MYSQL_DRIVER "'" << std::endl;
		//mysql.insert("VERSION", version);
		mysql.insert("KEY", name());
		mysql.insert("PROVIDER", QT_MYSQL_DRIVER);
		retval.append(mysql);
	}
	if(drivers.contains(QT_PGSQL_DRIVER))
	{
		ConnectionProvirerParams psql;
		TLOG(5, toNoDecorator, __HERE__) << "Tora Supports:'" QT_PGSQL_DRIVER "'" << std::endl;
		//psql.insert("VERSION", version);
		psql.insert("KEY", name());
		psql.insert("PROVIDER", QT_PGSQL_DRIVER);
		retval.append(psql);
	}
	if(drivers.contains(QT_ODBC_DRIVER))
	{
		ConnectionProvirerParams odbc;
		TLOG(5, toNoDecorator, __HERE__) << "Tora Supports:'" QT_ODBC_DRIVER "'" << std::endl;
		//odbc.insert("VERSION", version);
		odbc.insert("KEY", name());
		odbc.insert("PROVIDER", QT_ODBC_DRIVER);
		retval.append(odbc);
	} 
	return retval;
}

/** QSql based drivers have no plugins in Tora. So there is no dlopen here.
 *  We just extend ConnectionProvirerFactory keys if needed.
 */
void toQSqlFinder::load(ConnectionProvirerParams const &provider)
{
	QString providerName = provider.value("PROVIDER").toString();
	if(providerName == QT_MYSQL_DRIVER)
		ConnectionProvirerFactory::Instance().registerInFactory<toQMySqlProvider>(QT_MYSQL_DRIVER);
	else if(providerName == QT_PGSQL_DRIVER)
		ConnectionProvirerFactory::Instance().registerInFactory<toQPSqlProvider>(QT_PGSQL_DRIVER);
	else if(providerName == QT_ODBC_DRIVER)
		ConnectionProvirerFactory::Instance().registerInFactory<toQODBCProvider>(QT_ODBC_DRIVER);
	else
		throw QString("Unknown provider to load: %1").arg(providerName);
	TLOG(5, toNoDecorator, __HERE__) << "QT " << providerName << " driver \"loaded\"" << std::endl;
}

Util::RegisterInFactory<toQSqlFinder, ConnectionProviderFinderFactory> regToQSqlFind(QSQL_FINDER);
