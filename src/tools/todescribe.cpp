
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

#include "tools/todescribe.h"

#include "core/utils.h"
#include "core/toconnection.h"
#include <QtCore/QSettings>
#include <QtGui/QHideEvent>
#include <QMessageBox>
#include <QtCore/QList>

#include "tools/tobrowserbasewidget.h"

toDescribe::toDescribe(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);

    QSettings s;
    s.beginGroup("toDescribe");
    restoreGeometry(s.value("geometry", QByteArray()).toByteArray());
    s.endGroup();
}

void toDescribe::hideEvent(QHideEvent * event)
{
    QSettings s;
    s.beginGroup("toDescribe");
    s.setValue("geometry", saveGeometry());
    s.endGroup();
    event->accept();
}

void toDescribe::changeParams(const toCache::ObjectRef &obj)
{
    if (widget)
    {
        delete widget;
        widget = NULL;
    }
    QString schema(obj.first), object(obj.second);

    toCache::CacheEntryType objectType = toCache::NO_TYPE;
    toCache::CacheEntry const* cacheEntry; // was object found in cache?
    bool addedNewObject = false; // did we actually add new objects to cache during rereading?

    {
        Utils::toBusy busy;
        // Get a cached list of objects and then search in it without querying
        // the database in order to improve performance
        cacheEntry = toConnection::currentConnection(this).getCache().findEntry(obj);
        if ( cacheEntry)
        {
            // Possibly toCache did call toConnectionSub::resolve and object name was re-resolved (was synonym)
            schema = cacheEntry->name.first;
            object = cacheEntry->name.second;
            objectType = cacheEntry->type;
        }
        else
            objectType = toCache::TABLE;
        // TODO emit signal to request object description (if it was not found)
        // if ( !cacheEntry) // only check if object was not found in already cached list
        // {
        // // If object was not found in cache, then try updating the cache
        // //conn addedNewObject = toConnection::currentConnection(this).rereadObject(owner, object);
        // }
    }

    widget = toBrowserWidgetFactory::Instance().createPtr(objectType, this);
    if (widget == NULL)
    {
        QString objectTypeStr;
        try
        {
            objectTypeStr = toCache::cacheEntryTypeToString(objectType);
        }
        catch (...)
        {
            objectTypeStr = "UNKNOWN";
        }

        QMessageBox::information(this, "Describe",
                                 tr("Object %1.%2 (%3) cannot be described").arg(obj.first, obj.second).arg(objectTypeStr));
        return;
    }

    layout()->addWidget(widget);
    widget->changeParams(schema, object);
    show();
}
