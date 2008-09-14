
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

#include "tosql.h"
#include "toconnection.h"
#include "tomain.h"
#include "toqvalue.h"
#include "utils.h"
#include "tosecuritytreemodel.h"

/*static toSQL SQLObjectPrivs("toSecurity:ObjectPrivs",
                            "SELECT DECODE(:type<char[100]>,'FUNCTION','EXECUTE',\n"
                            "          'LIBRARY','EXECUTE',\n"
                            "          'PACKAGE','EXECUTE',\n"
                            "          'PROCEDURE','EXECUTE',\n"
                            "          'SEQUENCE','ALTER,SELECT',\n"
                            "          'TABLE','ALTER,DELETE,INDEX,INSERT,REFERENCES,SELECT,UPDATE',\n"
                            "          'TYPE','EXECUTE',\n"
                            "          'VIEW','DELETE,SELECT,INSERT,UPDATE',\n"
                            "          'OPERATOR','EXECUTE',\n"
                            "          'DIRECTORY','READ',\n"
                            "          NULL) FROM sys.DUAL",
                            "Takes a type as parameter and return ',' separated list of privileges");*/
static toSQL SQLObjectPrivs("toSecurity:ObjectPrivs",
                            "SELECT 'FUNCTION','EXECUTE' from dual\n"
                            "    UNION ALL\n"
                            "SELECT 'LIBRARY','EXECUTE' from dual\n"
                            "    UNION ALL\n"
                            "SELECT 'PACKAGE','EXECUTE' from dual\n"
                            "    UNION ALL\n"
                            "SELECT 'PROCEDURE','EXECUTE' from dual\n"
                            "    UNION ALL\n"
                            "SELECT 'SEQUENCE','ALTER,SELECT' from dual\n"
                            "    UNION ALL\n"
                            "SELECT 'TABLE','ALTER,DELETE,INDEX,INSERT,REFERENCES,SELECT,UPDATE' from dual\n"
                            "    UNION ALL\n"
                            "SELECT 'TYPE','EXECUTE' from dual\n"
                            "    UNION ALL\n"
                            "SELECT 'VIEW','DELETE,SELECT,INSERT,UPDATE' from dual\n"
                            "    UNION ALL\n"
                            "SELECT 'OPERATOR','EXECUTE' from dual\n"
                            "    UNION ALL\n"
                            "SELECT 'DIRECTORY','READ' from dual",
                            "Returns mapping of TYPE-supported acces right");

static toSQL SQLObjectGrant("toSecurity:ObjectGrant",
                            "SELECT owner,\n"
                            "       table_name,\n"
                            "       privilege,\n"
                            "       grantable,\n"
                            "       hierarchy\n"
                            "  FROM sys.dba_tab_privs\n"
                            " WHERE grantee = :f1<char[100]>",
                            "Get the privilege on objects for a user or role, "
                            "must have same columns and binds");


toSecurityTreeModel::toSecurityTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new toSecurityTreeItem();

    // read common object type vs its privs mapping
    toQuery typelst(toCurrentConnection(this), SQLObjectPrivs);
    QString key;
    QStringList value;
    while (!typelst.eof())
    {
        key = typelst.readValueNull();
        value = typelst.readValueNull().toString().split(",");
        m_SQLObjectPrivs[key] = value;
    }
}

toSecurityTreeModel::~toSecurityTreeModel()
{
    if (rootItem)
        delete rootItem;
}

int toSecurityTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<toSecurityTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant toSecurityTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    toSecurityTreeItem *item = static_cast<toSecurityTreeItem*>(index.internalPointer());
    if (!item)
        return QVariant();

    if (role == Qt::CheckStateRole && item->isCheckable())
        return item->state();

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(/*index.column()*/);
}

bool toSecurityTreeModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (!index.isValid())
        return false;

    toSecurityTreeItem *item = static_cast<toSecurityTreeItem*>(index.internalPointer());
    if (!item)
        return false;
    if (role == Qt::CheckStateRole)
    {
        Qt::CheckState state = (Qt::CheckState)value.toInt();
        item->setState(state);
        item->setChanged(true);
        // check parent if its child ("with admin opt") is currently checked
        if (item->parent()->isCheckable() && state != Qt::Unchecked)
        {
            item->parent()->setState(state);
            item->parent()->setChanged(true);
        }
        // uncheck "admin opt" child when its parent gets unchecked
        if (item->childCount() == 2 && state == Qt::Unchecked && item->child(0)->isCheckable())
        {
            item->child(0)->setState(state);
            item->child(1)->setState(state);
        }
        return true;
    }
    return false;
}

Qt::ItemFlags toSecurityTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    toSecurityTreeItem *item = static_cast<toSecurityTreeItem*>(index.internalPointer());
    if (item)
        return Qt::ItemIsEnabled | /*Qt::ItemIsSelectable |*/ Qt::ItemIsUserCheckable;

    return Qt::ItemIsEnabled /*| Qt::ItemIsSelectable*/;
}

QVariant toSecurityTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return tr("Objects");

    return QVariant();
}

QModelIndex toSecurityTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    toSecurityTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<toSecurityTreeItem*>(parent.internalPointer());

    toSecurityTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex toSecurityTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    toSecurityTreeItem *childItem = static_cast<toSecurityTreeItem*>(index.internalPointer());
    toSecurityTreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int toSecurityTreeModel::rowCount(const QModelIndex &parent) const
{
    toSecurityTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<toSecurityTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void toSecurityTreeModel::setupModelData(const QString & user)
{
    // clear old contents
    m_objectList.clear();
    delete rootItem;
    rootItem = new toSecurityTreeItem();

    // a map to remember schema vs its root item
    QMap<QString, toSecurityTreeItem*> ownerMap;
    // key owner; value <typename, its item>
    QMap<toSecurityTreeItem*,QMap<QString,toSecurityTreeItem*> > typeMap;

    // reading current grants status
    QMap<QString, QMap<QString, QMap<QString, QString> > > privs;
    QMap<QString, QMap<QString, QMap<QString, QString> > > hierarchyPrivs;
    toQuery grant(toCurrentConnection(this), SQLObjectGrant, user);
    QString yes = "YES";
    QString admstr = "ADMIN";
    QString normalstr = "normal";
    while (!grant.eof())
    {
        QString owner(grant.readValue());
        QString object(grant.readValue());
        QString priv(grant.readValue());
        QString admin(grant.readValue());
        QString hierarchy(grant.readValue());

        ((privs[owner])[object])[priv] = (admin == yes ? admstr : normalstr);
        ((hierarchyPrivs[owner])[object])[priv] = (admin == yes ? admstr : normalstr);
    }

    QString type;
    QString owner;
    QString name;

    std::list<toConnection::objectName> &objectList = toCurrentConnection(this).objects(true);
    for (std::list<toConnection::objectName>::iterator i = objectList.begin();
        i != objectList.end();
        i++)
    {
        type = (*i).Type;
        owner = (*i).Owner;
        name = (*i).Name;

        // no granting user's own objects for himself
        if (owner == user)
            continue;
        if (!ownerMap.contains(owner))
        {
            toSecurityTreeItem *i = new toSecurityTreeItem(rootItem, owner);
            ownerMap[owner] = i;
            rootItem->appendChild(i);
        }
        toSecurityTreeItem * ownerItem = ownerMap[owner];
        if (!typeMap[ownerItem].contains(type))
        {
            toSecurityTreeItem *i = new toSecurityTreeItem(ownerItem, type);
            typeMap[ownerItem][type] = i;
            ownerItem->appendChild(i);
        }

        toSecurityTreeItem * objectItem = new toSecurityTreeItem(typeMap[ownerItem][type], name);
        typeMap[ownerItem][type]->appendChild(objectItem);

        toSecurityTreeItem * grantItem;
        foreach(QString right, m_SQLObjectPrivs[type])
        {
            grantItem = new toSecurityTreeItem(objectItem, right, name, type, owner);
            objectItem->appendChild(grantItem);
            m_objectList.append(grantItem);
            grantItem->setCheckable(true);

            toSecurityTreeItem * admin = new toSecurityTreeItem(grantItem, "WITH GRANT OPTION");
            grantItem->appendChild(admin);
            admin->setCheckable(true);
            if (privs.contains(owner) && privs[owner].contains(name)
                && privs[owner][name].contains(right))
            {
                grantItem->setState(Qt::Checked);
                if (((privs[owner])[name])[right] == admstr)
                    admin->setState(Qt::Checked);
            }

            if (right != "SELECT")
                continue;
            // hierarchy option is only for SELECTs
            toSecurityTreeItem * hierarchyOption = new toSecurityTreeItem(grantItem, "WITH HIERARCHY OPTION");
            grantItem->appendChild(hierarchyOption);
            hierarchyOption->setCheckable(true);
            if (hierarchyPrivs.contains(owner) && hierarchyPrivs[owner].contains(name))
            {
//                 grantItem->setState(Qt::Checked);
                if (((hierarchyPrivs[owner])[name])[right] == admstr)
                    hierarchyOption->setState(Qt::Checked);
            }
        }
    }

    reset();
}

void toSecurityTreeModel::sql(const QString &user, std::list<QString> &sqlLst)
{
    QString grant("GRANT %1 ON %2 TO %3");
    QString revoke("REVOKE %1 ON %2 FROM %3");

    foreach (toSecurityTreeItem *i, m_objectList)
    {
        if (!i->changed())
            continue;

        QString addons;
        if (i->state() == Qt::Checked)
        {
            addons = "";
            for (int j = 0; j < i->childCount(); ++j)
            {
                if (i->child(j)->isCheckable() && i->child(j)->state() == Qt::Checked)
                    addons += "\n" + i->child(j)->data();
            }
            sqlLst.push_back(grant.arg(i->data()).arg(i->name()).arg(user) + addons);
        }
        else
            sqlLst.push_back(revoke.arg(i->data()).arg(i->name()).arg(user));
    }
}
