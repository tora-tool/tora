/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

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

