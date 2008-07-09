/*****
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003-2005 Quest Software, Inc
 * Portions Copyright (C) 2005 Other Contributors
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 *      You may link this product with any GPL'd Qt library such as Qt/Free
 *
 * All trademarks belong to their respective owners.
 *
 *****/

#include "connectionmodel.h"


namespace MigrateTool
{

ConnectionModel::ConnectionModel()
{
    m_data.clear();
}

void ConnectionModel::setupData(QList<toConnectionOptions> list)
{
    m_data.clear();
    m_data = list;
    reset();
}

QVariant ConnectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return section + 1;
    switch (section)
    {
        case 0 : return "Provider";
        case 1 : return "Host";
        case 2 : return "Database";
        case 3 : return "Username";
    }
    return "oops!";
}

QVariant ConnectionModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        toConnectionOptions opt = m_data.at(index.row());
        switch (index.column())
        {
            case 0 : return opt.provider;
            case 1 : return opt.host;
            case 2 : return opt.database;
            case 3 : return opt.username;
            default : return "oops!";
        }
    }
    return QVariant();
}

Qt::ItemFlags ConnectionModel::flags(const QModelIndex & index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

}; // namespace