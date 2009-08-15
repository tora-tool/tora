
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
#include "totreewidget.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>


toTreeWidget::toTreeWidget(QWidget *parent,
                           const char *name,
                           Qt::WFlags f)
        : QTreeWidget(parent)
{
    if (name)
        setObjectName(name);

    connect(this,
            SIGNAL(itemSelectionChanged()),
            this,
            SLOT(handleSelectionChange()));

    connect(this,
            SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            this,
            SLOT(handleDoubleClick(QTreeWidgetItem *, int)));

    connect(this,
            SIGNAL(itemClicked(QTreeWidgetItem *, int)),
            this,
            SLOT(handleClick(QTreeWidgetItem *, int)));

    connect(this,
            SIGNAL(itemExpanded(QTreeWidgetItem *)),
            this,
            SLOT(handleExpanded(QTreeWidgetItem *)));

    connect(this,
            SIGNAL(itemCollapsed(QTreeWidgetItem *)),
            this,
            SLOT(handleCollapsed(QTreeWidgetItem *)));

    connect(this,
            SIGNAL(itemActivated(QTreeWidgetItem *, int)),
            this,
            SLOT(handleActivated(QTreeWidgetItem *, int)));

    columnResize = NoColumn;      // q3listview default
}


toTreeWidgetItem* toTreeWidget::firstChild(void) const
{
    QTreeWidgetItem *it = QTreeWidget::topLevelItem(0);
    if (it)
        return dynamic_cast<toTreeWidgetItem *>(it);
    return NULL;
}


toTreeWidgetItem* toTreeWidget::currentItem(void) const
{
    QTreeWidgetItem *it = QTreeWidget::currentItem();
    if (it)
        return dynamic_cast<toTreeWidgetItem *>(it);
    return NULL;
}


toTreeWidgetItem* toTreeWidget::selectedItem(void) const
{
    QList<QTreeWidgetItem *> items = QTreeWidget::selectedItems();

    // just return the first item. widgets requiring multiple
    // selection support should be modified to use selectedItems()
    if (items.size() > 0)
        return dynamic_cast<toTreeWidgetItem *>(items.at(0));
    return NULL;
}


toTreeWidgetItem* toTreeWidget::itemAt(const QPoint &viewPos) const
{
    QTreeWidgetItem *it = QTreeWidget::itemAt(viewPos);
    if (it)
        return dynamic_cast<toTreeWidgetItem *>(it);
    return NULL;
}


void toTreeWidget::setSorting(int column, bool ascending)
{
    if (column < 0)
    {
        QTreeWidget::setSortingEnabled(false);
        return;
    }

    QTreeWidget::setSortingEnabled(true);
    QTreeWidget::sortByColumn(column,
                              ascending ?
                              Qt::AscendingOrder :
                              Qt::DescendingOrder);
}


const int toTreeWidget::columns(void)
{
    return QTreeWidget::columnCount();
}


int toTreeWidget::addColumn(const QString &label, int width)
{
    QTreeWidgetItem *head = QTreeWidget::headerItem();
    if (!head)
        return -1;

    int index = head->columnCount();

    // dirty, dirty hack.

    // QTreeWidget appears to always have at least one column.  you're
    // meant to add columns by using setHeaderLabels() but this is a
    // lot of porting.

    // check if we have just 1 column and it's name is "1" and then
    // change the name.
    if (index == 1 && head->text(0) == "1")
        index = 0;              // don't add

    head->setText(index, label);
    return index;
}


void toTreeWidget::setSelectionMode(SelectionMode mode)
{
    switch (mode)
    {
    case Multi:
        QTreeWidget::setSelectionMode(QAbstractItemView::MultiSelection);
        break;

    case Extended:
        QTreeWidget::setSelectionMode(QAbstractItemView::ExtendedSelection);
        break;

    case NoSelection:
        QTreeWidget::setSelectionMode(QAbstractItemView::NoSelection);
        break;

    default:
        QTreeWidget::setSelectionMode(QAbstractItemView::SingleSelection);
    }
}


int toTreeWidget::selectionMode() const
{
    switch (QTreeWidget::selectionMode())
    {
    case QAbstractItemView::MultiSelection:
        return Multi;

    case QAbstractItemView::ExtendedSelection:
        return Extended;

    case QAbstractItemView::NoSelection:
        return NoSelection;

    case QAbstractItemView::SingleSelection:
        return Single;

    case QAbstractItemView::ContiguousSelection:
        return Extended;

    default:
        return Single;
    }
}


void toTreeWidget::takeItem(toTreeWidgetItem *it)
{
    int pos = QTreeWidget::indexOfTopLevelItem(it);
    if (pos < 0)
        return;                 // not found

    QTreeWidget::takeTopLevelItem(pos);
}


void toTreeWidget::setColumnAlignment(int column, int align)
{
    ;                           // stub
}


int toTreeWidget::itemMargin() const
{
    return 1;                   // stub
}


void toTreeWidget::setTreeStepSize(int i)
{
    ;                           // stub
}


int toTreeWidget::childCount(void) const
{
    return QTreeWidget::topLevelItemCount();
}


void toTreeWidget::resizeColumnsToContents(void)
{
    for (int col = 0; col < QTreeWidget::columnCount(); col++)
        QTreeWidget::resizeColumnToContents(col);
}


void toTreeWidget::setResizeMode(ResizeMode m)
{
    columnResize = m;

    if (columnResize == NoColumn)
        return;

    if (columnResize == AllColumns)
        resizeColumnsToContents();

    // qtreewidget will by default allow the last column the most
    // space.
    if (columnResize == LastColumn)
        resizeColumnsToContents();
}


toTreeWidget::ResizeMode toTreeWidget::resizeMode() const
{
    return columnResize;
}


void toTreeWidget::setColumnWidthMode(int column, toTreeWidget::WidthMode)
{
    ;                           // stub
}


void toTreeWidget::sort(void)
{
    return;                     // stub
}


int toTreeWidget::visibleHeight(void)
{
    return 300;                 // stub
}


void toTreeWidget::setSelected(toTreeWidgetItem* item, bool selected)
{
    item->setSelected(selected);
}


void toTreeWidget::ensureItemVisible(const toTreeWidgetItem *tree)
{
    QTreeWidget::scrollToItem(tree);
}


void toTreeWidget::setOpen(bool o)
{
    int count = toTreeWidget::topLevelItemCount();
    for (int i = 0; i < count; i++)
        QTreeWidget::topLevelItem(i)->setExpanded(o);
}


void toTreeWidget::setOpen(toTreeWidgetItem *item, bool o)
{
    item->setExpanded(o);
}


void toTreeWidget::repaintItem(const QTreeWidgetItem *it) const
{
    return;                     // stub
}


void toTreeWidget::setShowSortIndicator(bool show)
{
    return;                     // stub
}


QString toTreeWidget::columnText(int column) const
{
    return QTreeWidget::headerItem()->text(column);
}


QModelIndex toTreeWidget::indexFromItem(QTreeWidgetItem *item,
                                        int column) const
{
    return QTreeWidget::indexFromItem(item, column);
}


QTreeWidgetItem* toTreeWidget::itemFromIndex(const QModelIndex &index) const
{
    return QTreeWidget::itemFromIndex(index);
}


void toTreeWidget::selectAll(bool s)
{
    if (!s)
        QTreeWidget::clearSelection();
    else
        QTreeWidget::selectAll();
}


void toTreeWidget::updateContents()
{
    return;                     // stub
}


void toTreeWidget::handleSelectionChange()
{
    QList<QTreeWidgetItem *> s = QTreeWidget::selectedItems();
    if (s.size() > 0)
        emit selectionChanged(dynamic_cast<toTreeWidgetItem *>(s.at(0)));

    emit selectionChanged();
}


void toTreeWidget::handleDoubleClick(QTreeWidgetItem *item, int column)
{
    if (!item || column < 0)
        return;
    emit doubleClicked(dynamic_cast<toTreeWidgetItem *>(item));
}


void toTreeWidget::handleClick(QTreeWidgetItem *item, int column)
{
    if (!item || column < 0)
        return;
    emit clicked(dynamic_cast<toTreeWidgetItem *>(item));
}


void toTreeWidget::handleExpanded(QTreeWidgetItem *item)
{
    if (item)
        emit(expanded(dynamic_cast<toTreeWidgetItem *>(item)));
}


void toTreeWidget::handleCollapsed(QTreeWidgetItem *item)
{
    if (item)
        emit(collapsed(dynamic_cast<toTreeWidgetItem *>(item)));
}


void toTreeWidget::handleActivated(QTreeWidgetItem *item, int column)
{
    if (item)
        emit(returnPressed(dynamic_cast<toTreeWidgetItem *>(item)));
}


void toTreeWidget::clear(void)
{
    QTreeWidget::clear();
}


// -------------------------------------------------- item

toTreeWidgetItem::toTreeWidgetItem(QTreeWidget *parent)
        : QTreeWidgetItem(parent, QTreeWidgetItem::Type)
{
}


toTreeWidgetItem::toTreeWidgetItem(QTreeWidget *parent, toTreeWidgetItem *after)
        : QTreeWidgetItem(parent, after, QTreeWidgetItem::Type)
{
}


toTreeWidgetItem::toTreeWidgetItem(toTreeWidget *parent,
                                   const QString &label0,
                                   const QString &label1,
                                   const QString &label2,
                                   const QString &label3,
                                   const QString &label4,
                                   const QString &label5,
                                   const QString &label6,
                                   const QString &label7)
        : QTreeWidgetItem(parent, QTreeWidgetItem::Type)
{
    if (!label0.isNull())
        setText(0, label0);
    if (!label1.isNull())
        setText(1, label1);
    if (!label2.isNull())
        setText(2, label2);
    if (!label3.isNull())
        setText(3, label3);
    if (!label4.isNull())
        setText(4, label4);
    if (!label5.isNull())
        setText(5, label5);
    if (!label6.isNull())
        setText(6, label6);
    if (!label7.isNull())
        setText(7, label7);
}


toTreeWidgetItem::toTreeWidgetItem(toTreeWidget *parent,
                                   toTreeWidgetItem *after,
                                   const QString &label0,
                                   const QString &label1,
                                   const QString &label2,
                                   const QString &label3,
                                   const QString &label4,
                                   const QString &label5,
                                   const QString &label6,
                                   const QString &label7)
        : QTreeWidgetItem(parent, after, QTreeWidgetItem::Type)
{
    if (!label0.isNull())
        setText(0, label0);
    if (!label1.isNull())
        setText(1, label1);
    if (!label2.isNull())
        setText(2, label2);
    if (!label3.isNull())
        setText(3, label3);
    if (!label4.isNull())
        setText(4, label4);
    if (!label5.isNull())
        setText(5, label5);
    if (!label6.isNull())
        setText(6, label6);
    if (!label7.isNull())
        setText(7, label7);
}


toTreeWidgetItem::toTreeWidgetItem(toTreeWidgetItem *parent,
                                   toTreeWidgetItem *after,
                                   const QString &label0,
                                   const QString &label1,
                                   const QString &label2,
                                   const QString &label3,
                                   const QString &label4,
                                   const QString &label5,
                                   const QString &label6,
                                   const QString &label7)
        : QTreeWidgetItem(parent, after, QTreeWidgetItem::Type)
{
    if (!label0.isNull())
        setText(0, label0);
    if (!label1.isNull())
        setText(1, label1);
    if (!label2.isNull())
        setText(2, label2);
    if (!label3.isNull())
        setText(3, label3);
    if (!label4.isNull())
        setText(4, label4);
    if (!label5.isNull())
        setText(5, label5);
    if (!label6.isNull())
        setText(6, label6);
    if (!label7.isNull())
        setText(7, label7);
}


toTreeWidgetItem::toTreeWidgetItem(toTreeWidgetItem *parent,
                                   const QString &label0,
                                   const QString &label1,
                                   const QString &label2,
                                   const QString &label3,
                                   const QString &label4,
                                   const QString &label5,
                                   const QString &label6,
                                   const QString &label7)
        : QTreeWidgetItem(parent, QTreeWidgetItem::Type)
{
    if (!label0.isNull())
        setText(0, label0);
    if (!label1.isNull())
        setText(1, label1);
    if (!label2.isNull())
        setText(2, label2);
    if (!label3.isNull())
        setText(3, label3);
    if (!label4.isNull())
        setText(4, label4);
    if (!label5.isNull())
        setText(5, label5);
    if (!label6.isNull())
        setText(6, label6);
    if (!label7.isNull())
        setText(7, label7);
}


toTreeWidgetItem* toTreeWidgetItem::nextSibling() const
{
    toTreeWidget *tree = dynamic_cast<toTreeWidget *>(
                             QTreeWidgetItem::treeWidget());
    if (!tree)
        return 0;

    QModelIndex index = tree->indexFromItem(const_cast<toTreeWidgetItem *>(this));
    QModelIndex sibling = index.sibling(index.row() + 1, index.column());

    if (sibling.isValid())
        return dynamic_cast<toTreeWidgetItem *>(tree->itemFromIndex(sibling));

    return 0;
}


toTreeWidgetItem* toTreeWidgetItem::firstChild() const
{
    if (QTreeWidgetItem::childCount() > 0)
        return dynamic_cast<toTreeWidgetItem *>(QTreeWidgetItem::child(0));
    return 0;
}


toTreeWidgetItem* toTreeWidgetItem::parent() const
{
    QTreeWidgetItem *p = QTreeWidgetItem::parent();
    if (p)
        return dynamic_cast<toTreeWidgetItem *>(p);
    return 0;
}


toTreeWidget* toTreeWidgetItem::listView() const
{
    return dynamic_cast<toTreeWidget *>(QTreeWidgetItem::treeWidget());
}


toTreeWidgetItem* toTreeWidgetItem::itemAbove() const
{
    return parent();
}


toTreeWidgetItem* toTreeWidgetItem::itemBelow() const
{
    return firstChild();
}


int toTreeWidgetItem::width(const QFontMetrics &fm,
                            const toTreeWidget *lv,
                            int c) const
{
    return 10;                  // stub
}


int toTreeWidgetItem::height()
{
    return 10;                  // stub
}


bool toTreeWidgetItem::isOpen(void) const
{
    return QTreeWidgetItem::isExpanded();
}


void toTreeWidgetItem::setOpen(bool o)
{
    return QTreeWidgetItem::setExpanded(o);
}


void toTreeWidgetItem::setPixmap(int col, const QPixmap &pix)
{
    QTreeWidgetItem::setIcon(col, QIcon(pix));
}


void toTreeWidgetItem::setSelectable(bool enable)
{
    // xor out the selectable flag
    Qt::ItemFlags fl = QTreeWidgetItem::flags() ^ Qt::ItemIsSelectable;
    QTreeWidgetItem::setFlags(fl);
}


void toTreeWidgetItem::moveItem(toTreeWidgetItem *after)
{
    return;                     // todo
}


void toTreeWidgetItem::setEnabled(bool b)
{
    Qt::ItemFlags fl = QTreeWidgetItem::flags();
    if (!b)
    {
        // xor out the enabled flag
        fl = fl ^ Qt::ItemIsEnabled;
    }
    else
    {
        // add enabled
        fl = fl | Qt::ItemIsEnabled;
    }

    QTreeWidgetItem::setFlags(fl);
}


bool toTreeWidgetItem::isEnabled() const
{
    return QTreeWidgetItem::flags() & Qt::ItemIsEnabled;
}


void toTreeWidgetItem::setExpandable(bool o)
{
    if (!o)
    {
        toTreeWidgetItem::setChildIndicatorPolicy(
            QTreeWidgetItem::DontShowIndicator);
    }
    else
    {
        toTreeWidgetItem::setChildIndicatorPolicy(
            QTreeWidgetItem::ShowIndicator);
    }
}


int toTreeWidgetItem::depth() const
{
    QTreeWidgetItem *parent;
    int d = 0;
    while ((parent = QTreeWidgetItem::parent()) != NULL)
        d++;

    return d;
}


// -------------------------------------------------- check item

toTreeWidgetCheck::toTreeWidgetCheck(toTreeWidget *parent,
                                     const QString &text,
                                     Type)
        : toTreeWidgetItem(parent, text)
{
    setOn(false);
}


toTreeWidgetCheck::toTreeWidgetCheck(toTreeWidgetItem *parent,
                                     const QString &text,
                                     Type)
        : toTreeWidgetItem(parent, text)
{
    setOn(false);
}


toTreeWidgetCheck::toTreeWidgetCheck(toTreeWidget *parent,
                                     toTreeWidgetItem *after,
                                     const QString &text,
                                     Type)
        : toTreeWidgetItem(parent, after, text)
{
    setOn(false);
}


toTreeWidgetCheck::toTreeWidgetCheck(toTreeWidgetItem *parent,
                                     toTreeWidgetItem *after,
                                     const QString &text,
                                     Type)
        : toTreeWidgetItem(parent, after, text)
{
    setOn(false);
}


bool toTreeWidgetCheck::isOn() const
{
    return checkState(0) == Qt::Checked;
}


void toTreeWidgetCheck::setOn(bool b)
{
    setCheckState(0, b ? Qt::Checked : Qt::Unchecked);
}
