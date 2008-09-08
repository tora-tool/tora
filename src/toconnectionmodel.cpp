/*****
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003-2008 Quest Software, Inc
 * Portions Copyright (C) 2008 Other Contributors
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

#include "toconnectionmodel.h"


toConnectionModel::toConnectionModel()
{
    m_data.clear();
}

void toConnectionModel::setupData(QMap<int,toConnectionOptions> list)
{
    m_data.clear();
    m_data = list;
    reset();
}

void toConnectionModel::append(int ix, toConnectionOptions conn)
{
    m_data[ix] = conn;
    reset();
}

// bool toConnectionModel::removeRow(int row, const QModelIndex & parent)
// {
// //     beginRemoveRows(QModelIndex(), row, row + 1);
//     QModelIndex ix = index(row, 0);
//     int key = data(ix, Qt::DisplayRole).toInt();
//     bool ret = m_data.remove(key);
// //     endRemoveRows();
//     return ret;
// }

QVariant toConnectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return section + 1;
    switch (section)
    {
        case 0 : return "#";
        case 1 : return "Provider";
        case 2 : return "Host";
        case 3 : return "Database";
        case 4 : return "Username";
        case 5 : return "Schema";
    }
    return "oops!";
}

QVariant toConnectionModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        toConnectionOptions opt = m_data[index.row()];
        switch (index.column())
        {
            case 0 : return index.row();
            case 1 : return opt.provider;
            case 2 : return opt.host;
            case 3 : return opt.database;
            case 4 : return opt.username;
            case 5 : return opt.schema;
            default : return "oops!";
        }
    }
    return QVariant();
}

Qt::ItemFlags toConnectionModel::flags(const QModelIndex & index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

