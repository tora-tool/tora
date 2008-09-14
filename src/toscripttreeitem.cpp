/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

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
