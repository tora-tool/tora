
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

#include "tosql.h"
#include "toconnection.h"
#include "tomain.h"
#include "toqvalue.h"
#include "utils.h"
#include "toscripttreeitem.h"
#include "toscripttreemodel.h"


toScriptTreeModel::toScriptTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new toScriptTreeItem();
}

toScriptTreeModel::~toScriptTreeModel()
{
    if (rootItem)
        delete rootItem;
}

int toScriptTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<toScriptTreeItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant toScriptTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    toScriptTreeItem *item = static_cast<toScriptTreeItem*>(index.internalPointer());
    if (!item)
        return QVariant();

    return item->data(/*index.column()*/);
}

Qt::ItemFlags toScriptTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant toScriptTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return tr("Objects");//rootItem->data(/*section*/);

    return QVariant();
}

QModelIndex toScriptTreeModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    toScriptTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<toScriptTreeItem*>(parent.internalPointer());

    toScriptTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex toScriptTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    toScriptTreeItem *childItem = static_cast<toScriptTreeItem*>(index.internalPointer());
    toScriptTreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int toScriptTreeModel::rowCount(const QModelIndex &parent) const
{
    toScriptTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<toScriptTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

// static toSQL SQLObjectListMySQL("toScriptTreeModel:ExtractObject",
//                                 "TOAD 1,0,0 SHOW DATABASES",
//                                 "Extract objects available to extract from the database, "
//                                 "should have same columns",
//                                 "3.23",
//                                 "MySQL");

static toSQL SQLObjectList("toScriptTreeModel:ExtractObject",
                           "SELECT *\n"
                           "  FROM (SELECT 'system environment', 'TABLESPACE', tablespace_name\n"
                           "   FROM sys.dba_tablespaces\n"
                           " UNION\n"
                           " SELECT 'system environment', 'ROLE', role\n"
                           "   FROM sys.dba_roles\n"
                           " UNION\n"
                           // public is handled inside the statement now the
                           //" SELECT 'PUBLIC',NULL,NULL\n"
                           //"   FROM dual\n"
// synonyms are handled in SQLUserObjectList finally
//                            " SELECT 'PUBLIC', 'SYNONYM', synonym_name\n"
//                            "   FROM sys.all_synonyms WHERE owner = 'PUBLIC'\n"
//                            " UNION\n"
                           " SELECT 'system environment', 'USER', username\n"
                           "   FROM sys.all_users)\n"
                           "  ORDER BY 1,2,3",
                           "Objects for user with sys privs.",
                           "0801");

static toSQL SQLUserObjectList("toScriptTreeModel:UserExtractObject",
                               "SELECT owner,object_type,object_name\n"
                               "  FROM sys.all_objects\n"
                               " WHERE object_type IN ('VIEW','TABLE','TYPE','SEQUENCE','PACKAGE',\n"
                               "                'PACKAGE BODY','FUNCTION','PROCEDURE', 'SYNONYM')\n"
                               " ORDER BY 1,2,3",
                               "Extract objects available to extract from the database if you "
                               "don't have admin access, should have same columns");

// static toSQL SQLPublicSynonymList("toScriptTreeModel:PublicSynonyms",
//                                   "SELECT owner, 'SYNONYM', synonym_name\n"
//                                   "  FROM sys.all_synonyms WHERE owner = 'PUBLIC'\n"
//                                   " ORDER BY 1, 2, 3",
//                                   "Extract all public synonyms from database");

// static toSQL SQLUserObjectsMySQL("toScriptTreeModel:UserObjects",
//                                  "SHOW TABLES FROM :own<noquote>",
//                                  "Get the objects available for a user, must have same columns and binds",
//                                  "3.23",
//                                  "MySQL");

static toSQL SQLUserObjects("toScriptTreeModel:UserObjects",
                            "SELECT *\n"
                            "  FROM (SELECT owner, 'DATABASE LINK',db_link\n"
                            "          FROM sys.all_db_links\n"
                            "         WHERE owner = :own<char[101]>\n"
                            "        UNION\n"
                            "        SELECT owner, object_type,object_name\n"
                            "          FROM sys.all_objects\n"
                            "         WHERE object_type IN ('VIEW','TYPE','SEQUENCE','PACKAGE',\n"
                            "                               'PACKAGE BODY','FUNCTION','PROCEDURE',\n"
                            "                               'TRIGGER', 'SYNONYM')\n"
                            "           AND owner = :own<char[101]>\n"
                            "         UNION\n"
                            "        SELECT owner, 'TABLE',table_name\n"
                            "          FROM sys.all_tables\n"
                            "         WHERE temporary != 'Y' AND secondary = 'N' AND iot_name IS NULL\n"
                            "           AND owner = :own<char[101]>\n"
                            "        UNION\n"
                            "        SELECT owner, 'MATERIALIZED TABLE',mview_name AS object\n"
                            "          FROM sys.all_mviews\n"
                            "         WHERE owner = :own<char[101]>)\n"
                            " ORDER BY 1, 2, 3",
                            "List objects for common user.");

static toSQL SQLUserObjects7("toScriptTreeModel:UserObjects",
                             "SELECT *\n"
                             "  FROM (SELECT owner, 'DATABASE LINK',db_link\n"
                             "          FROM sys.all_db_links\n"
                             "         WHERE owner = :own<char[101]>\n"
                             "        UNION\n"
                             "        SELECT owner, object_type,object_name\n"
                             "          FROM sys.all_objects\n"
                             "         WHERE object_type IN ('VIEW','TYPE','SEQUENCE','PACKAGE',\n"
                             "                               'PACKAGE BODY','FUNCTION','PROCEDURE','TRIGGER')\n"
                             "           AND owner = :own<char[101]>\n"
                             "         UNION\n"
                             "        SELECT owner, 'TABLE',table_name\n"
                             "          FROM sys.all_tables\n"
                             "         WHERE temporary != 'Y' AND secondary = 'N'\n"
                             "           AND owner = :own<char[101]>\n"
                             "        UNION\n"
                             "        SELECT owner, 'MATERIALIZED TABLE', mview_name AS object\n"
                             "          FROM sys.all_mviews\n"
                             "         WHERE owner = :own<char[101]>)\n"
                             " ORDER BY 1, 2, 3",
                             "",
                             "0703");

void toScriptTreeModel::setupModelData(const QString & connId, const QString & schema)
{
//     qDebug() << "void toScriptTreeModel::setupModelData(const QString & schema)" << schema;
    // clear old contents
    delete rootItem;
    rootItem = new toScriptTreeItem();

    QString topLevel("");
    QString secondLevel("");
    toScriptTreeItem * currentSchemaItem = 0;
    toScriptTreeItem * currentTypeItem = 0;

    toConnection &conn = toMainWidget()->connection(connId);
    toQList object;
    if (schema.isNull())
    {
        try
        {
            object = toQuery::readQueryNull(conn, SQLObjectList);
        }
        catch (...)
        {
//             qDebug() << "SQLObjectList call failed. Running 'common user' stmt.";
            qDebug("SQLObjectList call failed. Running 'common user' stmt.");
        }
        toQList object1 = toQuery::readQueryNull(conn, SQLUserObjectList);
        object.splice(object.end(), object1);
    }
    else
    {
        object = toQuery::readQueryNull(conn, SQLUserObjects, schema);
    }

    QString c1;
    QString c2;
    QString c3;
    while (object.size() > 0)
    {
        c1 = toShift(object);
        c2 = toShift(object);
        c3 = toShift(object);
//         qDebug() << c1 << c2 << c3;

        // new schema in hierarchy
        if (c1 != topLevel)
        {
// 			qDebug() << "SCHEMA switch " << topLevel << c1;
            currentSchemaItem = new toScriptTreeItem(rootItem, c1);
            rootItem->appendChild(currentSchemaItem);
            secondLevel = ""; // reset it
        }
        // new type in hierarchy
        if (c2 != secondLevel)
        {
// 			qDebug() << "TYPE switch " << secondLevel << c2;
            currentTypeItem = new toScriptTreeItem(currentSchemaItem, c2);
            currentSchemaItem->appendChild(currentTypeItem);
        }
        // real objects
        {
            toScriptTreeItem * item = new toScriptTreeItem(currentTypeItem, c3, c2, c1);
            currentTypeItem->appendChild(item);
        }
        topLevel = c1;
        secondLevel = c2;
    }

    reset();
//     qDebug() << "void toScriptTreeModel::setupModelData(const QString & schema)" << "end";
}
