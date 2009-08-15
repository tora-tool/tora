
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

#ifndef TOTREEWIDGET_H
#define TOTREEWIDGET_H

#include "config.h"
#include "totreewidget.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QList>
#include <QVariant>

class QPainter;
class QColorGroup;

class toTreeWidgetItem;


class toTreeWidget : public QTreeWidget
{
    Q_OBJECT;

public:
    // from q3listview
    enum SelectionMode { Single, Multi, Extended, NoSelection };
    enum ResizeMode { NoColumn, AllColumns, LastColumn };
    enum WidthMode { Manual, Maximum };

private:
    // for compatibility, use old qlistview enum.
    ResizeMode columnResize;

public:

    toTreeWidget(QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);


    /**
     * Returns the first (top) child of this item, or 0 if this item
     * has no children.
     *
     * Note that the children are not guaranteed to be sorted
     * properly. Q3ListView and Q3ListViewItem try to postpone or
     * avoid sorting to the greatest degree possible, in order to keep
     * the user interface snappy.
     *
     */
    virtual toTreeWidgetItem* firstChild(void) const;


    /**
     * Returns the current item, or 0 if there isn't one.
     *
     */
    virtual toTreeWidgetItem* currentItem(void) const;


    /**
     * Returns the selected item if the list view is in Single
     * selection mode and an item is selected.
     *
     */
    virtual toTreeWidgetItem* selectedItem(void) const;


    /**
     * Returns the list view item at viewPos. Note that viewPos is in
     * the viewport()'s coordinate system, not in the list view's own,
     * much larger, coordinate system.
     *
     */
    virtual toTreeWidgetItem* itemAt(const QPoint &viewPos) const;


    /**
     * Sets the list view to be sorted by column column in ascending
     * order if ascending is true or descending order if it is false.
     *
     * If column is -1, sorting is disabled and the user cannot sort
     * columns by clicking on the column headers. If column is larger
     * than the number of columns the user must click on a column
     * header to sort the list view.
     *
     */
    void setSorting(int column, bool ascending = true);


    /**
     * This property holds the number of columns in this list view.
     *
     */
    const int columns(void);


    /**
     * Adds a width pixels wide column with the column header label to
     * the list view, and returns the index of the new column.
     *
     * All columns apart from the first one are inserted to the right
     * of the existing ones.
     *
     * If width is negative, the new column's WidthMode is set to
     * Maximum instead of Manual.
     *
     */
    int addColumn(const QString &label, int width = -1);


    /**
     * This property holds the list view's selection mode.
     *
     * The mode can be Single (the default), Extended, Multi or
     * NoSelection.
     *
     */
    void setSelectionMode(SelectionMode mode);


    virtual int selectionMode(void) const;


    // overrode parent namespace. this is a qt4 function.
    void setSelectionMode(QAbstractItemView::SelectionMode mode)
    {
        QTreeWidget::setSelectionMode(mode);
    }


    /**
     * Removes item i from the list view; i must be a top-level item.
     *
     * Removes item from this object's list of children and causes an
     * update of the screen display. The item is not deleted. You
     * should not normally need to call this function because
     * Q3ListViewItem::~Q3ListViewItem() calls it.
     *
     * The normal way to delete an item is to use delete.

     * If a taken item is part of a selection in Single selection
     * mode, it is unselected and selectionChanged() is emitted. If a
     * taken item is part of a selection in Multi or Extended
     * selection mode, it remains selected.
     *
     */
    void takeItem(toTreeWidgetItem *it);


    /**
     * Sets column column's alignment to align. The alignment is
     * ultimately passed to Q3ListViewItem::paintCell() for each item
     * in the list view. For horizontally aligned text with
     * Qt::AlignLeft or Qt::AlignHCenter the ellipsis (...) will be to
     * the right, for Qt::AlignRight the ellipsis will be to the left.
     *
     */
    virtual void setColumnAlignment(int column, int align);


    /**
     * This property holds the advisory item margin that list items
     * may use.
     *
     * The item margin defaults to one pixel and is the margin between
     * the item's edges and the area where it draws its
     * contents. Q3ListViewItem::paintFocus() draws in the margin.
     *
     */
    int itemMargin(void) const;


    /**
     * This property holds the number of pixels a child is offset from
     * its parent.
     *
     * The default is 20 pixels.
     *
     * Of course, this property is only meaningful for hierarchical
     * list views.
     *
     */
    virtual void setTreeStepSize(int);


    /**
     * Returns how many children this item has. The count only
     * includes the item's immediate children.
     *
     */
    virtual int childCount(void) const;


    /**
     * This property holds whether all, none or the only the last
     * column should be resized.
     *
     * Specifies whether all, none or only the last column should be
     * resized to fit the full width of the list view. The values for
     * this property can be one of: NoColumn (the default), AllColumns
     * or LastColumn.
     *
     * Warning: Setting the resize mode should be done after all
     * necessary columns have been added to the list view, otherwise
     * the behavior is undefined.
     *
     */
    virtual void setResizeMode(ResizeMode m);


    /**
     * This property holds whether all, none or the only the last
     * column should be resized.
     *
     */
    ResizeMode resizeMode() const;


    /**
     * Resizes all columns to their contents. QTreeWidget provides
     * resizeColumntoContents(int) for specific columns.
     *
     */
    virtual void resizeColumnsToContents(void);


    /**
     * Sets column c's width mode to mode. The default depends on the
     * original width argument to addColumn().
     *
     */
    virtual void setColumnWidthMode(int column, WidthMode);


    /**
     * Sorts the list view using the last sorting configuration (sort
     * column and ascending/descending).
     *
     */
    virtual void sort(void);


    /**
     * This property holds the vertical amount of the content that is
     * visible.
     *
     */
    virtual int visibleHeight(void);


    /**
     * If selected is true the item is selected; otherwise it is
     * unselected.
     *
     * If the list view is in Single selection mode and selected is
     * true, the currently selected item is unselected and item is
     * made current. Unlike Q3ListViewItem::setSelected(), this
     * function updates the list view as necessary and emits the
     * selectionChanged() signals.
     *
     */
    virtual void setSelected(toTreeWidgetItem* item, bool selected);


    /**
     * Ensures that item i is visible, scrolling the list view
     * vertically if necessary and opening (expanding) any parent
     * items if this is required to show the item.
     *
     */
    virtual void ensureItemVisible(const toTreeWidgetItem *tree);


    /**
     * Opens or closes an item, i.e. shows or hides an item's
     * children.
     *
     */
    virtual void setOpen(bool o);


    /**
     * Sets item to be open if open is true and item is expandable,
     * and to be closed if open is false. Repaints accordingly.
     *
     */
    virtual void setOpen(toTreeWidgetItem *item, bool o);


    /**
     * Repaints this item on the screen if it is currently visible.
     *
     */
    void repaintItem(const QTreeWidgetItem *it) const;


    /**
     * This property holds whether the list view header should display
     * a sort indicator.
     *
     * If this property is true, an arrow is drawn in the header of
     * the list view to indicate the sort order of the list view
     * contents. The arrow will be drawn in the correct column and
     * will point up or down, depending on the current sort
     * direction. The default is false (don't show an indicator).
     *
     */
    virtual void setShowSortIndicator(bool show);


    /**
     * Returns the text of column c.
     *
     */
    QString columnText(int column) const;


    /**
     * Returns the QModelIndex assocated with the given item in the
     * given column.
     *
     * Not virtual, but if you call the wrong one, it's protected.
     *
     */
    QModelIndex indexFromItem(QTreeWidgetItem *item, int column = 0) const;


    /**
     * Returns a pointer to the QTreeWidgetItem assocated with the given index.
     *
     * Not virtual, but if you call the wrong one, it's protected.
     *
     */
    QTreeWidgetItem* itemFromIndex(const QModelIndex &index) const;


signals:
    /**
     * This signal is emitted whenever the set of selected items has
     * changed (normally before the screen update). It is available
     * both in Single selection and Multi selection mode but is most
     * useful in Multi selection mode.
     *
     */
    void selectionChanged(void);


    /**
     * This signal is emitted whenever the set of selected items has
     * changed (normally before the screen update). It is available
     * both in Single selection and Multi selection mode but is most
     * useful in Multi selection mode.
     *
     */
    void selectionChanged(toTreeWidgetItem *);


    /**
     * This signal is emitted whenever an item is double-clicked. It's
     * emitted on the second button press, not the second button
     * release. item is the list view item on which the user did the
     * double-click.
     *
     */
    void doubleClicked(toTreeWidgetItem *);


    /**
     * This signal is emitted whenever the user clicks (mouse pressed
     * and mouse released) in the list view. item is the clicked list
     * view item, or 0 if the user didn't click on an item.
     *
     */
    void clicked(toTreeWidgetItem *item);


    /**
     * This signal is emitted when item has been expanded, i.e. when
     * the children of item are shown.
     *
     */
    void expanded(toTreeWidgetItem *);


    /**
     * This signal is emitted when the item has been collapsed,
     * i.e. when the children of item are hidden.
     *
     */
    void collapsed(toTreeWidgetItem *);


    /**
     * This signal is emitted when Enter or Return is pressed. The
     * item parameter is the currentItem().
     *
     */
    void returnPressed(toTreeWidgetItem *);

public slots:

    /**
     * If select is true, all the items get selected; otherwise all
     * the items get unselected. This only works in the selection
     * modes Multi and Extended. In Single and NoSelection mode the
     * selection of the current item is just set to select.
     *
     */
    virtual void selectAll(bool);


    /**
     * Updates the sizes of the viewport, header, scroll bars and so
     * on.
     *
     */
    virtual void updateContents(void);


    /**
     * emits selectionChanged
     *
     */
    virtual void handleSelectionChange(void);


    /**
     * emits doubleClicked
     *
     */
    virtual void handleDoubleClick(QTreeWidgetItem *item, int column);


    /**
     * emits clicked
     *
     */
    virtual void handleClick(QTreeWidgetItem *item, int column);


    /**
     * emits expanded(toTreeWigetItem)
     *
     */
    virtual void handleExpanded(QTreeWidgetItem *item);


    /**
     * emits returnPressed
     *
     */
    virtual void handleActivated(QTreeWidgetItem *item, int column);


    /**
     * emits collapsed(toTreeWigetItem)
     *
     */
    virtual void handleCollapsed(QTreeWidgetItem *item);


    /**
     * qt4:
     * Removes all items in the view. This will also remove all
     * selections. The table dimentions stay the same.
     *
     * this version also removes column descriptions.
     *
     */
    void clear(void);
};


class toTreeWidgetItem : public QTreeWidgetItem
{

public:

    /**
     * Constructs a new list view item that is a child of parent and
     * first in the parent's list of children.
     *
     */
    toTreeWidgetItem(QTreeWidget *parent);


    /**
     * Constructs an empty list view item that is a child of parent
     * and is after item after in the parent's list of children. Since
     * parent is a Q3ListView the item will be a top-level item.
     *
     */
    toTreeWidgetItem(QTreeWidget *parent, toTreeWidgetItem *after);


    /**
     * Constructs a new top-level list view item in the Q3ListView
     * parent, with up to eight constant strings, label1, label2,
     * label3, label4, label5, label6, label7 and label8 defining its
     * columns' contents.
     *
     */
    toTreeWidgetItem(toTreeWidget *parent,
                     const QString &label0,
                     const QString &label1 = QString(),
                     const QString &label2 = QString(),
                     const QString &label3 = QString(),
                     const QString &label4 = QString(),
                     const QString &label5 = QString(),
                     const QString &label6 = QString(),
                     const QString &label7 = QString());


    /**
     * Constructs a new list view item as a child of the
     * Q3ListViewItem parent with up to eight constant strings,
     * label1, label2, label3, label4, label5, label6, label7 and
     * label8 as columns' contents.
     *
     */
    toTreeWidgetItem(toTreeWidget *parent,
                     toTreeWidgetItem *after,
                     const QString &label0,
                     const QString &label1 = QString(),
                     const QString &label2 = QString(),
                     const QString &label3 = QString(),
                     const QString &label4 = QString(),
                     const QString &label5 = QString(),
                     const QString &label6 = QString(),
                     const QString &label7 = QString());


    /**
     * Constructs a new list view item as a child of the
     * Q3ListViewItem parent. It is inserted after item after and may
     * contain up to eight strings, label1, label2, label3, label4,
     * label5, label6, label7 and label8 as column entries.
     *
     */
    toTreeWidgetItem(toTreeWidgetItem *parent,
                     toTreeWidgetItem *after,
                     const QString &label0,
                     const QString &label1 = QString(),
                     const QString &label2 = QString(),
                     const QString &label3 = QString(),
                     const QString &label4 = QString(),
                     const QString &label5 = QString(),
                     const QString &label6 = QString(),
                     const QString &label7 = QString());


    /**
     * Constructs a new list view item as a child of the
     * Q3ListViewItem parent with up to eight constant strings,
     * label1, label2, label3, label4, label5, label6, label7 and
     * label8 as columns' contents.
     *
     */
    toTreeWidgetItem(toTreeWidgetItem *parent,
                     const QString &label0,
                     const QString &label1 = QString(),
                     const QString &label2 = QString(),
                     const QString &label3 = QString(),
                     const QString &label4 = QString(),
                     const QString &label5 = QString(),
                     const QString &label6 = QString(),
                     const QString &label7 = QString());


    /**
     * Returns the sibling item below this item, or 0 if there is no
     * sibling item after this item.
     *
     * this is horribly broken. use QTreeWidgetItemIterator
     * instead. provided to keep code compiling.
     *
     */
    virtual toTreeWidgetItem* nextSibling(void) const;


    /**
     * Returns the first (top) child of this item, or 0 if this item
     * has no children.
     *
     */
    virtual toTreeWidgetItem* firstChild(void) const;


    /**
     * Returns the parent of this item, or 0 if this item has no
     * parent.
     *
     */
    virtual toTreeWidgetItem* parent(void) const;


    /**
     * Returns a pointer to the list view containing this item.
     *
     */
    virtual toTreeWidget* listView(void) const;


    /**
     * Returns a pointer to the item immediately above this item on
     * the screen. This is usually the item's closest older sibling,
     * but it may also be its parent or its next older sibling's
     * youngest child, or something else if anyoftheabove->height()
     * returns 0. Returns 0 if there is no item immediately above this
     * item.
     *
     * This function assumes that all parents of this item are open
     * (i.e. that this item is visible, or can be made visible by
     * scrolling).
     *
     * This function might be relatively slow because of the tree
     * traversions needed to find the correct item.
     *
     */
    virtual toTreeWidgetItem* itemAbove(void) const;


    /**
     * Returns a pointer to the item immediately below this item on
     * the screen. This is usually the item's eldest child, but it may
     * also be its next younger sibling, its parent's next younger
     * sibling, grandparent's, etc., or something else if
     * anyoftheabove->height() returns 0. Returns 0 if there is no
     * item immediately below this item.
     *
     * This function assumes that all parents of this item are open
     * (i.e. that this item is visible or can be made visible by
     * scrolling).
     *
     */
    virtual toTreeWidgetItem* itemBelow(void) const;


    // child widgets expect this
    virtual void setup(void)
    {
        ;
    }


    /**
     * Returns the number of pixels of width required to draw column c
     * of list view lv, using the metrics fm without cropping. The
     * list view containing this item may use this information
     * depending on the Q3ListView::WidthMode settings for the column.
     *
     * The default implementation returns the width of the bounding
     * rectangle of the text of column c.
     *
     */
    virtual int width(const QFontMetrics &fm,
                      const toTreeWidget *lv,
                      int c) const;


    /**
     * Returns the height of this item in pixels. This does not
     * include the height of any children; totalHeight() returns that.
     *
     */
    virtual int height(void);


    // couldn't find any documentation on this
    void paintCell(QPainter *,
                   const QColorGroup & cg,
                   int column,
                   int width,
                   int alignment)
    {
        ;                       /* stub todo */
    }


    /**
     * Returns true if this list view item has children and they are
     * not explicitly hidden; otherwise returns false.
     *
     */
    bool isOpen(void) const;


    /**
     * Opens or closes an item, i.e. shows or hides an item's
     * children.
     *
     * If o is true all child items are shown initially. The user can
     * hide them by clicking the - icon to the left of the item. If o
     * is false, the children of this item are initially hidden. The
     * user can show them by clicking the + icon to the left of the
     * item.
     *
     */
    virtual void setOpen(bool o);


    /**
     * Sets the pixmap in column column to pm, if pm is non-null and
     * different from the current pixmap, and if column is
     * non-negative.
     *
     */
    virtual void setPixmap(int col, const QPixmap &pix);


    /**
     * Sets this items to be selectable if enable is true (the
     * default) or not to be selectable if enable is false.
     *
     * The user is not able to select a non-selectable item using
     * either the keyboard or the mouse. The application programmer
     * still can though, e.g. using setSelected().
     *
     */
    virtual void setSelectable(bool enable);


    /**
     * Move the item to be after item after, which must be one of the
     * item's siblings. To move an item in the hierarchy, use
     * takeItem() and insertItem().
     *
     */
    virtual void moveItem(toTreeWidgetItem *after);


    /**
     * If b is true the item is enabled; otherwise it is
     * disabled. Disabled items are drawn differently
     * (e.g. grayed-out) and are not accessible by the user.
     *
     */
    virtual void setEnabled(bool b);


    /**
     * Returns true if this item is enabled; otherwise returns false.
     *
     */
    bool isEnabled(void) const;


    /**
     * Sets this item to be expandable even if it has no children if
     * enable is true, and to be expandable only if it has children if
     * enable is false (the default).
     *
     * The dirview example uses this in the canonical fashion. It
     * checks whether the directory is empty in setup() and calls
     * setExpandable(true) if not; in setOpen() it reads the contents
     * of the directory and inserts items accordingly. This strategy
     * means that dirview can display the entire file system without
     * reading very much at startup.
     *
     */
    virtual void setExpandable(bool o);


    /**
     * Returns the depth of this item.
     *
     * (thanks qt)
     */
    int depth() const;

protected:
    /**
     * Sets this item's height to height pixels. This implicitly
     * changes totalHeight(), too.
     *
     * Note that a font change causes this height to be overwritten
     * unless you reimplement setup().
     *
     * For best results in Windows style we suggest using an even
     * number of pixels.
     *
     */
    virtual void setHeight(int height)
    {
        ;                       /* stub */
    }
};


class toTreeWidgetCheck : public toTreeWidgetItem
{

public:
    enum Type { RadioButton,
                CheckBox,
                Controller,
                RadioButtonController = Controller,
                CheckBoxController
              };

    enum ToggleState { Off, NoChange, On };


    /**
     * Constructs a checkable item with parent parent, text text and
     * of type tt. Note that tt must not be RadioButton. Radio buttons
     * must be children of a RadioButtonController.
     *
     */
    toTreeWidgetCheck(toTreeWidget *parent,
                      const QString &text,
                      Type = RadioButtonController);


    /**
     * Constructs a checkable item with parent parent, text text and
     * of type tt. Note that a RadioButton must be the child of a
     * RadioButtonController, otherwise it will not toggle.
     *
     */
    toTreeWidgetCheck(toTreeWidgetItem *parent,
                      const QString &text,
                      Type = RadioButtonController);


    /**
     * Constructs a checkable item with parent parent, which is after
     * after in the parent's list of children, and with text text and
     * of type tt. Note that a RadioButton must be the child of a
     * RadioButtonController, otherwise it will not toggle.
     *
     */
    toTreeWidgetCheck(toTreeWidget *parent,
                      toTreeWidgetItem *after,
                      const QString &text,
                      Type = RadioButtonController);


    /**
     * Constructs a checkable item with parent parent, which is after
     * after in the parent's list of children, with text text and of
     * type tt. Note that this item must not be a RadioButton. Radio
     * buttons must be children of a RadioButtonController.
     *
     */
    toTreeWidgetCheck(toTreeWidgetItem *parent,
                      toTreeWidgetItem *after,
                      const QString &text,
                      Type = RadioButtonController);


    // -------------------------------------------------- stubs for checkbox

    /**
     * Returns true if the item is toggled on; otherwise returns
     * false.
     *
     */
    virtual bool isOn(void) const;


    /**
     * Sets the button on if b is true, otherwise sets it
     * off. Maintains radio button exclusivity.
     *
     */
    virtual void setOn(bool);
};


class toTreeWidgetItemIterator : public QTreeWidgetItemIterator
{
public:
    toTreeWidgetItemIterator(toTreeWidget *widget, IteratorFlags flags = All)
            : QTreeWidgetItemIterator(widget, flags)
    {
    }


    toTreeWidgetItemIterator(toTreeWidgetItem *widget, IteratorFlags flags = All)
            : QTreeWidgetItemIterator(widget, flags)
    {
    }


    toTreeWidgetItem* operator*() const
    {
        return dynamic_cast<toTreeWidgetItem *>(QTreeWidgetItemIterator::operator*());
    }
};

#endif
