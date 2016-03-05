
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

#include "connection/toqsqlprovider.h"
#include "connection/toqsqlconnection.h"

#include <QtCore/QSettings>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

toQSqlProvider::toQSqlProvider(toConnectionProviderFinder::ConnectionProvirerParams const& p)
    : toConnectionProvider(p)
{}

bool toQSqlProvider::initialize()
{
    return true;
}

QList<QString> toQSqlProvider::hosts() const
{
    return QList<QString>{};
}

QList<QString> toQSqlProvider::databases(const QString &host, const QString &user, const QString &pwd) const
{
    QList<QString> ret;
#ifdef Q_OS_WIN32
    QSettings settings("HKEY_CURRENT_USER\\Software\\ODBC\\ODBC.INI\\ODBC Data Sources", QSettings::NativeFormat);
    foreach(QString key, settings.childGroups())
    {
        QString s = key;
    }

    foreach(QString key, settings.childKeys())
    {
        QString s = key;
        QString t = settings.value(key).toString();
        ret << key;
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

QList<QString> toQSqlProvider::options() const
{
    QList<QString> ret;
    ret << "TBD options";
    return ret;
}

QWidget* toQSqlProvider::configurationTab(QWidget *parent)
{
    return NULL;
}

toConnection::connectionImpl* toQSqlProvider::createConnectionImpl(toConnection &conn)
{
    return new toQSqlConnectionImpl(conn);
}

toConnectionTraits* toQSqlProvider::createConnectionTrait(void)
{
    static toQSqlTraits *t = new toQSqlTraits();
    return t;
}
