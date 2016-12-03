
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

#include "core/toconnectionmodel.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"
#include "core/utils.h"
#include "core/tooracleconst.h"

#include <QtCore/QSettings>

toConnectionModel::toConnectionModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void toConnectionModel::setupData(QMap<int, toConnectionOptions> list)
{
    beginResetModel();
    m_data.clear();
    m_data = list;
    endResetModel();
}


void toConnectionModel::readConfig()
{
    beginResetModel();
    m_data.clear();

    QSettings Settings;
    Settings.beginGroup("connections");
    Settings.beginGroup("history");
    for (int pos = 0; pos < Settings.childGroups().count(); ++pos)
    {
        Settings.beginGroup(QString::number(pos)); // history\## entry in TOra.conf
        if (!Settings.contains("provider"))
        {
            Settings.endGroup();
            break;
        }

        QString provider = Settings.value("provider", "").toString();
        QString host     = Settings.value("host", "").toString();
        QString database = Settings.value("database", "").toString();
        QString username = Settings.value("username", "").toString();
        QString password = Utils::toUnobfuscate(Settings.value("password", "").toString());
        QString schema   = Settings.value("schema", "").toString();
        QString color    = Settings.value("color", "").toString();

        if (provider == ORACLE_TNSCLIENT)
            host = "";

        Settings.beginGroup("options");
        QSet<QString> options;
        QStringList keys = Settings.allKeys();
        foreach(QString const & s, keys)
        {
            if (Settings.value(s, false).toBool())
                options.insert(s);
        }
        Settings.endGroup();

        toConnectionOptions opt(
            provider,
            host,
            database,
            username,
            password,
            schema,
            color,
            Settings.value("port", 0).toInt(),
            options);
        m_data[pos] = opt;
        Settings.endGroup();
    }
    Settings.endGroup(); // history section

    endResetModel();
}

void toConnectionModel::append(toConnectionOptions conn)
{
    // find latest id (max+1)
    QList<int> keys = m_data.keys();
    qSort(keys);
    int max = 0;
    if (!keys.empty())
        max = keys.last() + 1;

    beginResetModel();
    m_data[max] = conn;
    endResetModel();
}

bool toConnectionModel::removeRow(int row, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), row, row + 1);
    QModelIndex ix = index(row, 0);
    int key = data(ix, Qt::DisplayRole).toInt();
    bool ret = m_data.remove(key);
    endRemoveRows();

    return ret;
}

int toConnectionModel::findConnection(toConnectionOptions const &conn) const
{
    return m_data.key(conn, -1);
}

void toConnectionModel::saveConnection(toConnectionOptions const &opt)
{
    int pos = findConnection(opt);

    QSettings Settings;
    Settings.beginGroup("connections");
    Settings.beginGroup("history");
    Settings.beginGroup(QString::number(pos));

    Settings.setValue("provider", opt.provider);
    Settings.setValue("username", opt.username);
    if (toConfigurationNewSingle::Instance().option(ToConfiguration::Global::SavePasswordBool).toBool())
    {
        Settings.setValue("password", Utils::toObfuscate(opt.password));
    }
    Settings.setValue("host",     opt.host);
    Settings.setValue("port",     opt.port);
    Settings.setValue("database", opt.database);
    Settings.setValue("schema",   opt.schema);
    Settings.setValue("color",    opt.color);

    Settings.remove("options");
    Settings.beginGroup("options");
    Q_FOREACH(QString s, opt.options)
    {
        Settings.setValue(s, true);
    }
    Settings.endGroup(); // options

    Settings.endGroup(); // history/##pos
    Settings.endGroup(); // history
    Settings.endGroup(); // connections section

}

QVariant toConnectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return section + 1;
    switch (section)
    {
        case 0 :
            return "#";
        case 1 :
            return "Provider";
        case 2 :
            return "Host";
        case 3 :
            return "Database";
        case 4 :
            return "Username";
        case 5 :
            return "Schema";
    }
    return "oops!";
}

QVariant toConnectionModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();


    toConnectionOptions opt = m_data[index.row()];
    switch (role)
    {
        case Qt::DisplayRole:
            switch (index.column())
            {
                case 0 :
                    return index.row();
                case 1 :
                    return opt.provider;
                case 2 :
                    return opt.host;
                case 3 :
                    return opt.database;
                case 4 :
                    return opt.username;
                case 5 :
                    return opt.schema;
                default :
                    return "oops!";
            }
            break;
        case Qt::DecorationRole:
            if (index.column() == 1 && toConfigurationNewSingle::Instance().option(ToConfiguration::Global::ColorizedConnectionsBool).toBool())
                return Utils::connectionColorPixmap(opt.color);
            break;
    }
    return QVariant();
}

Qt::ItemFlags toConnectionModel::flags(const QModelIndex & index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    toConnectionOptions opt = m_data[index.row()];
    if (opt.enabled)
    	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    else
    	return Qt::NoItemFlags;
}

void toConnectionModel::disableConnection(int ix)
{
    toConnectionOptions opt = m_data[ix];
    opt.enabled = false;
    m_data[ix] = opt;
}
