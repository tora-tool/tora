
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

#include "core/tocodemodel.h"

#include "core/tosql.h"
#include "core/toconnection.h"
#include "core/tomainwindow.h"
#include "core/toqvalue.h"
#include "core/utils.h"
#include "core/toeventquery.h"

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
                                 "0701",
                                 "QPSQL");

static toSQL SQLListObjectsMySQL("toCodeModel:ListObjects",
                                 "select r.routine_name object_name,\n"
                                 "       r.routine_type objec_type,\n"
                                 "       'VALID' status\n"
                                 "  from information_schema.routines r\n"
                                 " where r.routine_schema = :owner<char[50]>\n",
                                 "",
                                 "0500",
                                 "QMYSQL");

static toSQL SQLListObjectsTeradata(
    "toCodeModel:ListObjects",
    "SELECT TABLENAME,\n"
    "       CASE WHEN TABLEKIND = 'P' THEN 'PROCEDURE' ELSE 'MACRO' END AS object_type,\n"
    "       'VALID' AS Status\n"
    "  FROM dbc.TABLES\n"
    " WHERE databasename = trim ( :f1<char[101]> )\n"
    "   AND tablekind IN ( 'P', 'M' )\n"
    " ORDER BY 1",
    "",
    "",
    "Teradata");

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


toCodeModel::toCodeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , rootItem(NULL)
    , packageItem(NULL)
    , procItem(NULL)
    , funcItem(NULL)
    , macroItem(NULL)
    , typeItem(NULL)
    , query(0)
{
    rootItem    = new toCodeModelItem(0, "Code");
}

toCodeModel::~toCodeModel()
{
    if (rootItem)
        delete rootItem;
    if (query)
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
                || item->type() == "PACKAGE BODY"
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

        if (item->type() == "MACRO"
                || item == macroItem)
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


void toCodeModel::queryError(const toConnection::exception &err)
{
    Utils::toStatusMessage(err);
    cleanup();
}


void toCodeModel::refresh(toConnection &conn, const QString &owner)
{
    m_owner = owner;

    // Create an empty code object tree displayed on the right hand-side
    // Individual items are fetched later using SQLListObjects query
    // Note: MySQL does not support packages and types therefore these
    // branches are not included for MySQL connections.
    beginResetModel();
    delete rootItem;
    rootItem = new toCodeModelItem(0, "Code");
    if (!conn.providerIs("QMYSQL") && !conn.providerIs("Teradata"))
        packageItem = new toCodeModelItem(rootItem, tr("Package"));
    procItem = new toCodeModelItem(rootItem, tr("Procedure"));
    if (!conn.providerIs("Teradata"))
        funcItem = new toCodeModelItem(rootItem, tr("Function"));
    if (conn.providerIs("Teradata"))
        macroItem = new toCodeModelItem(rootItem, tr("Macro"));
    if (!conn.providerIs("QMYSQL") && !conn.providerIs("Teradata"))
        typeItem = new toCodeModelItem(rootItem, tr("Type"));
    endResetModel();

    toQueryParams param;
    param << m_owner;

    cleanup(); // stop/delete query object (if exists)

    try
    {
        query = new toEventQuery(this
                                 , conn
                                 , toSQL::sql(SQLListObjects, conn)
                                 , param
                                 , toEventQuery::READ_FIRST); // really?

        auto c1 = connect(query, &toEventQuery::dataAvailable, this, &toCodeModel::receiveData);
        auto c2 = connect(query, &toEventQuery::error, this, [=](toEventQuery*, const toConnection::exception &e){ queryError(e); });
        auto c3 = connect(query, &toEventQuery::done, this, [=](toEventQuery *, unsigned long) { cleanup();});

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

        toConnectionSubLoan c(toConnection::currentConnection(this));
        toQuery query(c, SQLListPackage, toQueryParams() << item->display() << m_owner);
        QString ctype;
        QString cstatus;

        emit layoutAboutToBeChanged();
        while (!query.eof())
        {
            ctype = (QString)query.readValue();
            cstatus = (QString)query.readValue();

            new toCodeModelItem(item, ctype, ctype, cstatus);
            // "inherit" child status for parent if it's required
            if ((ctype == "SPEC" || ctype == "BODY" || ctype == "PACKAGE" || ctype == "PACKAGE BODY")
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
        // TODO - here we directly call QObject's slot
        query->stop();
        delete query;
    }

    query = 0;
}

void toCodeModel::receiveData(toEventQuery*)
{
    if (!query)
    {
        cleanup();
        return;
    }

    while (query->hasMore())
    {
        QString cname = (QString)query->readValue();
        QString ctype = (QString)query->readValue();
        QString cstatus = (QString)query->readValue();

        toCodeModelItem *item = 0;
        if (ctype == QString("PACKAGE"))
            item = packageItem;
        else if (ctype == QString("PROCEDURE"))
            item = procItem;
        else if (ctype == QString("FUNCTION"))
            item = funcItem;
        else if (ctype == QString("MACRO"))
            item = macroItem;
        else if (ctype == QString("TYPE"))
            item = typeItem;

        new toCodeModelItem(item, cname, ctype, cstatus);
    }

    beginResetModel();
    endResetModel();
    emit dataReady();

    if (!query->hasMore() && query->eof())
    {
        cleanup();
        return;
    }

    query->setFetchMode(toEventQuery::READ_ALL);
}
