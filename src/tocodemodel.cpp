
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

// List all code objects and its status (IN/VALID)
// it takes PACKAGE BODY for every PACKAGE as
// a IN/VALID indicator too (for invalid icons).
static toSQL SQLListObjects("toCodeModel:ListObjects",
                             "SELECT a.object_name,\n"
                             "       a.object_type,\n"
                             "       (CASE\n"
                             "           WHEN a.object_type = 'PACKAGE'\n"
                             "                AND a.status = 'VALID'\n"
                             "                AND (select count(1)\n"
                             "                        from sys.all_objects b\n"
                             "                        where a.object_name = b.object_name\n"
                             "                            and a.owner = b.owner\n"
                             "                            and b.object_type = 'PACKAGE BODY'\n"
                             "                            and b.status != 'VALID'\n"
                             "                    ) > 0\n"
                             "                THEN 'INVALID'\n"
                             "           ELSE a.status\n"
                             "        END\n"
                             "       ) as Status\n"
                             "  FROM sys.all_objects a\n"
                             "  WHERE\n"
                             "       a.object_type IN ( 'FUNCTION',\n"
                             "                          'PACKAGE',\n"
                             "                          'PROCEDURE',\n"
                             "                          'TYPE' )\n"
                             "       AND a.owner = :owner<char[50]>\n"
                             "ORDER BY a.object_name\n",
                             "Get list of code objects",
                             "0901",
                             "Oracle");

static toSQL SQLListObjects8("toCodeModel:ListObjects",
                             "SELECT a.object_name,\n"
                             "       a.object_type,\n"
                             "       DECODE (a.object_type,\n"
                             "         'PACKAGE', (SELECT DECODE (ao.status,\n"
                             "           'VALID', (SELECT ao2.status \n"
                             "                       FROM sys.all_objects ao2\n"
                             "                      WHERE ao2.object_name = a.object_name\n"
                             "                        AND ao2.owner = a.owner \n"
                             "                        AND ao2.object_type='PACKAGE BODY'),\n"
                             "           ao.status )\n"
                             " FROM sys.all_objects ao\n"
                             " WHERE ao.object_name = a.object_name\n"
                             " AND ao.owner = a.owner AND ao.object_type=a.object_type ),\n"
                             " a.status ) AS Status\n"
                             "  FROM sys.all_objects a\n"
                             "  WHERE\n"
                             "       a.object_type IN ( 'FUNCTION',\n"
                             "                          'PACKAGE',\n"
                             "                          'PROCEDURE',\n"
                             "                          'TYPE' )\n"
                             "       AND a.owner = :owner<char[50]>\n"
                             "ORDER BY a.object_name\n",
                             "",
                             "0801",
                             "Oracle");

static toSQL SQLListObjectsPgSQL("toCodeModel:ListObjects",
                             "SELECT p.proname AS Object_Name,\n"
                             "  CASE WHEN p.prorettype = 0 THEN 'PROCEDURE'\n"
                             "       ELSE 'FUNCTION'\n"
                             "   END AS Object_Type,\n"
                             "  'VALID' as Status\n"
                             "FROM pg_proc p LEFT OUTER JOIN pg_namespace n ON p.pronamespace=n.oid\n"
                             "WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                             "ORDER BY Object_Name",
                             "",
                             "7.1",
                             "PostgreSQL");

static toSQL SQLListObjectsMySQL("toCodeModel:ListObjects",
                                 "select r.routine_name object_name,\n"
                                 "       r.routine_type objec_type,\n"
                                 "       'VALID' status\n"
                                 "  from information_schema.routines r\n"
                                 " where r.routine_schema = :owner<char[50]>\n",
                                 "",
                                 "5.0",
                                 "MySQL");

static toSQL SQLListPackage("toCodeModel:ListPackage",
                            "SELECT \n"
                            "       DECODE(a.object_type, 'PACKAGE', 'SPEC', 'BODY'),\n"
                            "       a.status\n"
                            "  FROM sys.all_objects a\n"
                            " WHERE\n"
                            "       a.object_name = :name<char[50]>\n"
                            "   AND a.owner = :owner<char[50]>\n"
                            "ORDER BY a.object_name",
                            "Get package spec and body status");



toCodeModelItem::toCodeModelItem(
                                    toCodeModelItem *parent,
                                    const QString & display,
                                    const QString & type,
                                    const QString & status)
{
    parentItem = parent;
    itemDisplay = display;
    itemType = type;
    itemStatus = status;
    if (parent)
        parent->appendChild(this);
}

toCodeModelItem::~toCodeModelItem()
{
    qDeleteAll(childItems);
}

void toCodeModelItem::appendChild(toCodeModelItem *item)
{
    childItems.append(item);
}

toCodeModelItem *toCodeModelItem::child(int row)
{
    return childItems.value(row);
}

int toCodeModelItem::childCount() const
{
    return childItems.count();
}

int toCodeModelItem::columnCount() const
{
    return 1;//itemData.count();
}

QString toCodeModelItem::display() const
{
    return itemDisplay;
}

QString toCodeModelItem::type() const
{
    return itemType;
}

QString toCodeModelItem::status() const
{
    return itemStatus;
}

void toCodeModelItem::setStatus(const QString & s)
{
    itemStatus = s;
}

toCodeModelItem *toCodeModelItem::parent()
{
    return parentItem;
}

int toCodeModelItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<toCodeModelItem*>(this));

    return 0;
}


toCodeModel::toCodeModel(QObject *parent) : QAbstractItemModel(parent), query(0)
{
    rootItem    = new toCodeModelItem(0, "Code");
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
        return static_cast<toCodeModelItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant toCodeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::ToolTipRole)
        return static_cast<toCodeModelItem*>(index.internalPointer())->display();

    toCodeModelItem *item = static_cast<toCodeModelItem*>(index.internalPointer());
    if (!item)
        return QVariant();

    if (role == Qt::FontRole && item->type() == "NULL")
    {
        QFont f;
        f.setBold(true);
        return f;
    }

    if (role == Qt::DecorationRole)
    {
        if (item->type() == "PACKAGE"
            || item->type() == "SPEC"
            || item->type() == "BODY"
            || item == packageItem)
        {
            if (item->status() == "VALID")
                return QPixmap(":/icons/package.png");
            else
                return QPixmap(":/icons/package-invalid.png");
        }
        if (item->type() == "PROCEDURE"
            || item == procItem)
        {
            if (item->status() == "VALID")
                return QPixmap(":/icons/procedure.png");
            else
                return QPixmap(":/icons/procedure-invalid.png");
        }
        if (item->type() == "FUNCTION"
            || item == funcItem)
        {
            if (item->status() == "VALID")
                return QPixmap(":/icons/function.png");
            else
                return QPixmap(":/icons/function-invalid.png");
        }
        if (item->type() == "TYPE"
            || item == typeItem)
        {
            if (item->status() == "VALID")
                return QPixmap(":/icons/type.png");
            else
                return QPixmap(":/icons/type-invalid.png");
        }
        return QVariant();
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->display();
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
        return tr("Code");

    return QVariant();
}

QModelIndex toCodeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    toCodeModelItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<toCodeModelItem*>(parent.internalPointer());

    toCodeModelItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex toCodeModel::parent(const QModelIndex &index) const
{
//     if (!index.isValid())
//         return QModelIndex();
// 
//     toCodeModelItem *childItem = static_cast<toCodeModelItem*>(index.internalPointer());
//     toCodeModelItem *parentItem = childItem->parent();
// 
//     if (parentItem == rootItem)
//         return QModelIndex();
//     if (parentItem == packageItem)
//         return createIndex(1, 0, parentItem);
//     if (parentItem == procItem)
//         return createIndex(2, 0, parentItem);
//     if (parentItem == funcItem)
//         return createIndex(3, 0, parentItem);
//     if (parentItem == typeItem)
//         return createIndex(4, 0, parentItem);
// 
//     return QModelIndex();
    if (!index.isValid())
        return QModelIndex();

    toCodeModelItem *childItem = static_cast<toCodeModelItem*>(index.internalPointer());
    toCodeModelItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int toCodeModel::rowCount(const QModelIndex &parent) const
{
    toCodeModelItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<toCodeModelItem*>(parent.internalPointer());

    return parentItem->childCount();
}


void toCodeModel::queryError(const toConnection::exception &err) {
    toStatusMessage(err);
    cleanup();
}


void toCodeModel::refresh(toConnection &conn, const QString &owner)
{
    m_owner = owner;

    // Create an empty code object tree displayed on the right hand-side
    // Individual items are fetched later using SQLListObjects query
    // Note: MySQL does not support packages and types therefore these
    // branches are not included for MySQL connections.
    delete rootItem;
    rootItem    = new toCodeModelItem(0, "Code");
    if (!toIsMySQL(conn))
        packageItem = new toCodeModelItem(rootItem, tr("Package"));
    procItem    = new toCodeModelItem(rootItem, tr("Procedure"));
    funcItem    = new toCodeModelItem(rootItem, tr("Function"));
    if (!toIsMySQL(conn))
        typeItem    = new toCodeModelItem(rootItem, tr("Type"));

    toQList param;
    param.push_back(m_owner);

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

void toCodeModel::addChildContent(const QModelIndex & index)
{
    toCodeModelItem * item = static_cast<toCodeModelItem*>(index.internalPointer());
    if (!item)
        return;

    QString itemType(item->type());
    if (itemType == "PACKAGE")
    {
        // don't re-read it until refresh action...
        if (item->childCount() > 0)
            return;

        toQuery query(toCurrentConnection(this), SQLListPackage, item->display(), m_owner);
        QString ctype;
        QString cstatus;

        emit layoutAboutToBeChanged();
        while (!query.eof())
        {
            ctype = query.readValueNull();
            cstatus = query.readValueNull();

            new toCodeModelItem(item, ctype, ctype, cstatus);
            // "inherit" child status for parent if it's required
            if ((ctype == "SPEC" || ctype == "BODY")
                && cstatus != "VALID")
            {
                item->setStatus(cstatus);
            }
        }
        emit layoutChanged();
        return;
    } // end of packages
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

    while(query->hasMore())
    {
        QString cname = query->readValueNull().toString();
        QString ctype = query->readValueNull().toString();
        QString cstatus = query->readValueNull().toString();

        toCodeModelItem *item = 0;
        if(ctype == QString("PACKAGE"))
            item = packageItem;
        else if(ctype == QString("PROCEDURE"))
            item = procItem;
        else if (ctype == QString("FUNCTION"))
            item = funcItem;
        else if(ctype == QString("TYPE"))
            item = typeItem;

        new toCodeModelItem(item, cname, ctype, cstatus);
    }

    reset();
    emit dataReady();

    if(!query->hasMore() && query->eof())
    {
        cleanup();
        return;
    }

    query->readAll();
}
