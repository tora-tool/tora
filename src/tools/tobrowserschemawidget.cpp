
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

#include "tools/tobrowserschemawidget.h"
#include "core/tocodemodel.h"
#include "core/utils.h"

#include <QtCore/QCoreApplication>

toBrowserSchemaTableView::toBrowserSchemaTableView(QWidget * parent, const QString &type)
    : toResultTableView(true, false, parent),
      toBrowserSchemaBase()
{
    ObjectType = type;
    ForceRequery = false;
    if (!type.isEmpty())
        connect(this, SIGNAL(done()), this, SLOT(updateCache()));
}

QString toBrowserSchemaTableView::objectName()
{
    return selectedIndex(1).data(Qt::EditRole).toString();
}

void toBrowserSchemaTableView::refreshWithParams(const QString & schema, const QString & filter)
{
    // If all objects of a specific type are to be displayed, try displaying them from the cache
    // This cannot work for filtered objects yet as we do not want to re-impelement the "like" operator.
    if (filter == "%" &&
            !ObjectType.isEmpty() &&
            !ForceRequery
            /* && toConnection::currentConnection(this).cacheAvailable(false)*/)
    {
        /* NOTE: that directories are not owned by any individual schema. Therefore they are
         * always stored under schema "SYS" in Oracle data dictionary. This is why in case
         * of directories we're not specifying schema name when checking the cache.
         */
        QString sch;
        if (ObjectType == "DIRECTORY")
            sch = "%";
        else
            sch = schema;

        if (toConnection::currentConnection(this).getCache().findEntry(toCache::ObjectRef(sch, ObjectType, ""))) // search for entry of type TORA_SCHEMA_LIST
        {
            this->queryFromCache(sch, ObjectType);
            return;
        }
    }

    ForceRequery = false;
    Schema = schema;
    toResultTableView::refreshWithParams(toQueryParams() << schema << filter);
}

void toBrowserSchemaTableView::updateCache(void)
{
	// if toEventQuery creation thrown an exception, Model == NULL
	if(this->Model == NULL)
		return;

    // Update list of objects
    QList<toCache::CacheEntry*> rows;
    toCache::CacheEntry *obj;
    // TODO: Check that result model rows are NOT sorted in descending order as that would break updating of cache!!!
    toQuery::RowList modelRows = this->Model->getRawData();
    for (QList<toQuery::Row>::iterator i = modelRows.begin(); i != modelRows.end(); i++)
    {
        obj = toCache::createCacheEntry(Schema, (QString)(*i)[1], ObjectType, "");
        if (obj != NULL) // Some objects (like DBLINKs are not held in the toCache => obj == NULL
            rows.append(obj);
    }
    // NOTE: Oracle directories do not belong to any particular schema.
    //       Therefore they are saved as belonging to SYS schema.
    if (ObjectType == "DIRECTORY")
        Schema = "SYS";

    toConnection::currentConnection(this).getCache().upsertSchemaEntries(Schema, ObjectType, rows);

    // Update information when list of this type of objects in this schema was updated
    // NOTE: Type is placed in the name field in order not to
    // mix this meta-information with actual list of objects.
    toConnection::currentConnection(this).getCache().upsertEntry(new toCache::CacheEntry(Schema, ObjectType, toCache::TORA_SCHEMA_LIST, ""));
} // updateCache

toBrowserSchemaCodeBrowser::toBrowserSchemaCodeBrowser(QWidget * parent)
    : QTreeView(parent)
    , toBrowserSchemaBase()
    , m_filter(NULL)
{
    m_model = new toCodeModel(this);
    setModel(m_model);

    setAlternatingRowColors(true);

    connect(this, SIGNAL(clicked(const QModelIndex &)),
            m_model, SLOT(addChildContent(const QModelIndex &)));
    connect(m_model, SIGNAL(dataReady()), this, SLOT(expandAll()));
}

QString toBrowserSchemaCodeBrowser::objectName()
{
    toCodeModelItem *item = static_cast<toCodeModelItem*>(currentIndex().internalPointer());

    if (!item || item->type() == "NULL")
        return "";

    if (item->type() == "PACKAGE"
            || item->type() == "FUNCTION"
            || item->type() == "MACRO"
            || item->type() == "PROCEDURE"
            || item->type() == "TYPE")
    {
        return item->display();
    }

    if (item->type() == "SPEC" || item->type() == "BODY")
        return item->parent()->display();

    return "";
}

QString toBrowserSchemaCodeBrowser::objectType()
{
    toCodeModelItem *item = static_cast<toCodeModelItem*>(currentIndex().internalPointer());
    if (item && item->parent())
    {
        return item->type();
    }
    return "";
}

void toBrowserSchemaCodeBrowser::refreshWithParams(const QString & schema, const QString & filter)
{
    // WARNING: QCoreApplication::processEvents() is mandatory here
    //          to prevent random crashing on parent-tab widget switching
    //          in toBrowser. Dunno why...
    QCoreApplication::processEvents();
    m_model->refresh(toConnection::currentConnection(this), schema);
}

void toBrowserSchemaCodeBrowser::setFilter(toViewFilter *filter)
{
	m_filter  = filter;
};
