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

#include <QStringList>

#include "toscripttreeitem.h"


toScriptTreeItem::toScriptTreeItem(
                                    toScriptTreeItem *parent,
                                    const QString &data,
                                    const QString & type,
                                    const QString & schema)
{
    parentItem = parent;
    itemData = data;
    itemType = type;
    itemSchema = schema;
}

toScriptTreeItem::~toScriptTreeItem()
{
    qDeleteAll(childItems);
}

void toScriptTreeItem::appendChild(toScriptTreeItem *item)
{
    childItems.append(item);
}

toScriptTreeItem *toScriptTreeItem::child(int row)
{
    return childItems.value(row);
}

int toScriptTreeItem::childCount() const
{
    return childItems.count();
}

int toScriptTreeItem::columnCount() const
{
    return 1;//itemData.count();
}

QString toScriptTreeItem::data() const
{
    return itemData;
}

QString toScriptTreeItem::type() const
{
    return itemType;
}

QString toScriptTreeItem::schema() const
{
    return itemSchema;
}

toScriptTreeItem *toScriptTreeItem::parent()
{
    return parentItem;
}

int toScriptTreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<toScriptTreeItem*>(this));

    return 0;
}
