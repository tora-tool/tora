
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

#include "config.h"

#include "tosql.h"
#include "toconnection.h"
#include "tomain.h"
#include "toqvalue.h"
#include "utils.h"
#include "tocodemodel.h"
#include "toeventquery.h"

static toSQL SQLListObjects("toCodeModel:ListObjects",
                            "SELECT a.object_name,\n"
                            "       a.object_type\n"
                            "  FROM sys.all_objects a,\n"
                            "       sys.all_tab_comments b\n"
                            " WHERE a.owner = b.owner ( + )\n"
                            "   AND a.object_name = b.table_name ( + )\n"
                            "   AND a.object_type = b.table_type ( + )\n"
                            "   AND a.object_type IN ( 'FUNCTION',\n"
                            "                          'PACKAGE',\n"
                            "                          'PROCEDURE',\n"
                            "                          'TYPE' )\n"
                            "   AND a.owner = :owner<char[50]>\n"
                            "ORDER BY a.object_name",
                            "Get list of code objects");

toCodeModel::toCodeModel(QObject *parent) : QAbstractItemModel(parent), query(0)
{
    rootItem    = new QTreeWidgetItem();
    packageItem = new QTreeWidgetItem(rootItem);
    procItem    = new QTreeWidgetItem(rootItem);
    typeItem    = new QTreeWidgetItem(rootItem);

    rootItem->setText(0, tr("Code"));
    packageItem->setText(0, tr("Package"));
    procItem->setText(0, tr("Procedure"));
    typeItem->setText(0, tr("Type"));
}

toCodeModel::~toCodeModel()
{
    if(rootItem)
        delete rootItem;
    if(query)
        cleanup();
}

int toCodeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<QTreeWidgetItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant toCodeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::ToolTipRole)
        return static_cast<QTreeWidgetItem*>(index.internalPointer())->text(index.column());

    if (role != Qt::DisplayRole)
        return QVariant();

    QTreeWidgetItem *item = static_cast<QTreeWidgetItem*>(index.internalPointer());
    if (!item)
        return QVariant();

    return item->text(index.column());
}

Qt::ItemFlags toCodeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant toCodeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return tr("Objects");

    return QVariant();
}

QModelIndex toCodeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    QTreeWidgetItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<QTreeWidgetItem*>(parent.internalPointer());

    QTreeWidgetItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex toCodeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    QTreeWidgetItem *childItem = static_cast<QTreeWidgetItem*>(index.internalPointer());
    QTreeWidgetItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();
    if (parentItem == packageItem)
        return createIndex(1, 0, parentItem);
    if (parentItem == procItem)
        return createIndex(2, 0, parentItem);
    if (parentItem == typeItem)
        return createIndex(3, 0, parentItem);

    return QModelIndex();
}

int toCodeModel::rowCount(const QModelIndex &parent) const
{
    QTreeWidgetItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<QTreeWidgetItem*>(parent.internalPointer());

    return parentItem->childCount();
}


void toCodeModel::queryError(const toConnection::exception &err) {
    toStatusMessage(err);
    cleanup();
}


void toCodeModel::refresh(toConnection &conn, const QString &owner)
{
    delete rootItem;
    rootItem = new QTreeWidgetItem();

    packageItem = new QTreeWidgetItem(rootItem);
    procItem = new QTreeWidgetItem(rootItem);
    typeItem = new QTreeWidgetItem(rootItem);

    rootItem->setText(0, tr("Code"));
    packageItem->setText(0, tr("Package"));
    procItem->setText(0, tr("Procedure"));
    typeItem->setText(0, tr("Type"));

    toQList param;
    param.push_back(owner);

    try {
        query = new toEventQuery(conn,
                                 toQuery::Long,
                                 toSQL::sql(SQLListObjects, conn),
                                 param);

        qRegisterMetaType<toConnection::exception>("toConnection::exception");

        connect(query,
                SIGNAL(dataAvailable()),
                this,
                SLOT(readData()));
        connect(query,
                SIGNAL(error(const toConnection::exception &)),
                this,
                SLOT(queryError(const toConnection::exception &)));
        connect(query,
                SIGNAL(done()),
                this,
                SLOT(readData()));

        query->start();
    }
    TOCATCH;
}

void toCodeModel::cleanup()
{
    if (query)
    {
        disconnect(query, 0, this, 0);

        query->stop();
        delete query;
    }

    query = 0;
}

void toCodeModel::readData()
{
    if (!query)
    {
        cleanup();
        return;
    }

    while(query->hasMore()) {
        QString cname = query->readValueNull().toString();
        QString ctype = query->readValueNull().toString();

        QTreeWidgetItem *item = 0;
        if(ctype == QString("FUNCTION"))
            item = new QTreeWidgetItem(procItem);
        else if(ctype == QString("PACKAGE"))
            item = new QTreeWidgetItem(packageItem);
        else if(ctype == QString("PROCEDURE"))
            item = new QTreeWidgetItem(procItem);
        else if(ctype == QString("TYPE"))
            item = new QTreeWidgetItem(typeItem);

        item->setText(0, cname);
    }

    reset();

    if(!query->hasMore() && query->eof())
    {
        cleanup();
        return;
    }

    query->readAll();
}
