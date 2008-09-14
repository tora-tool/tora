/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include <QStringList>

#include "tosecuritytreeitem.h"


toSecurityTreeItem::toSecurityTreeItem(
                                    toSecurityTreeItem *parent,
                                    const QString & data,
                                    const QString & name,
                                    const QString & type,
                                    const QString & schema)
    : m_changed(false),
      m_checkable(false),
      m_checked(Qt::Unchecked)
{
    parentItem = parent;
    itemData = data;
    itemName = name;
    itemType = type;
    itemSchema = schema;
}

toSecurityTreeItem::~toSecurityTreeItem()
{
    qDeleteAll(childItems);
}

void toSecurityTreeItem::appendChild(toSecurityTreeItem *item)
{
    childItems.append(item);
}

toSecurityTreeItem *toSecurityTreeItem::child(int row)
{
    return childItems.value(row);
}

int toSecurityTreeItem::childCount() const
{
    return childItems.count();
}

int toSecurityTreeItem::columnCount() const
{
    return 1;//itemData.count();
}

QString toSecurityTreeItem::data() const
{
    return itemData;
}

QString toSecurityTreeItem::name() const
{
    return itemName;
}

QString toSecurityTreeItem::type() const
{
    return itemType;
}

QString toSecurityTreeItem::schema() const
{
    return itemSchema;
}

toSecurityTreeItem *toSecurityTreeItem::parent()
{
    return parentItem;
}

int toSecurityTreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<toSecurityTreeItem*>(this));

    return 0;
}
