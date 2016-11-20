
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

#include "core/utils.h"
#include "core/tologger.h"
#include "connection/toqodbcprovider.h"
#include "connection/toqsqlconnection.h"

#define QT_DRIVER_NAME "QODBC"

#include <QtCore/QSettings>

QString toQODBCProvider::m_name = QT_DRIVER_NAME;
QString toQODBCProvider::m_displayName = ODBC_PROVIDER;

toQODBCProvider::toQODBCProvider(toConnectionProviderFinder::ConnectionProvirerParams const& p)
    : toQSqlProvider(p)
{}

toConnection::connectionImpl* toQODBCProvider::createConnectionImpl(toConnection &conn)
{
    return new toQSqlConnectionImpl(conn);
}

QList<QString> toQODBCProvider::databases(const QString &host, const QString &user, const QString &pwd) const
{
    QList<QString> ret;
#ifdef Q_OS_WIN32
    {
        QSettings settings("HKEY_CURRENT_USER\\Software\\ODBC\\ODBC.INI", QSettings::NativeFormat);
        foreach(QString key, settings.childGroups())
        {
            QString s = key;
            QString t = settings.value(key + '/' + "Driver").toString();

            QFileInfo driver(t);
            bool valid  = Utils::toLibrary::isValidLibrary(driver); // TODO this one thows, the others don't
            if (valid)
            {
                ret << key;
                TLOG(5, toNoDecorator, __HERE__) << "Users DSN: " << key << '(' << t << ')'<< " passed."<< std::endl;
            } else {
                TLOG(5, toNoDecorator, __HERE__) << "Users DSN: " << key << '(' << t << ')'<< " failed."<< std::endl;
            }
        }
    }

    {
        QSettings settings("HKEY_LOCAL_MACHINE\\Software\\ODBC\\ODBC.INI", QSettings::NativeFormat);
        foreach(QString key, settings.childGroups())
        {
            QString s = key;
            QString t = settings.value(key + '/' + "Driver").toString();

            QFileInfo driver(t);
            bool valid  = Utils::toLibrary::isValidLibrary(driver); // TODO this one thows, the others don't
            if (valid)
            {
                ret << key;
                TLOG(5, toNoDecorator, __HERE__) << "Machine DSN: " << key << '(' << t << ')'<< " passed."<< std::endl;
            } else {
                TLOG(5, toNoDecorator, __HERE__) << "Machine DSN: " << key << '(' << t << ')'<< " failed."<< std::endl;
            }
        }
    }

#else
    QString envODBC = getenv("ODBCINI");
    QFileInfo odbcINI(envODBC);

    if (envODBC.isEmpty() || !odbcINI.exists() || !odbcINI.isReadable())
        odbcINI = QFileInfo("/etc/odbc.ini");

    QSettings settings(odbcINI.absoluteFilePath(), QSettings::IniFormat);
    foreach(QString key, settings.childGroups())
    {
        QString s = key;
        ret << key;
    }
#endif
    return ret;
}

/*
bool toQODBCProvider::initialize()
{
	return true;
}

QList<QString> toQODBCProvider::options()
{
	QList<QString> ret;
	ret << "TBD options";
	return ret;
}

QWidget* toQODBCProvider::configurationTab(QWidget *parent)
{
	return NULL;
}

toConnectionTraits* toQODBCProvider::createConnectionTrait(void)
{
	return NULL;
}
*/
