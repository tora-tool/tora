
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

#ifndef TORESULTVIEW_H
#define TORESULTVIEW_H

#include "config.h"
#include "toeditwidget.h"
#include "toresult.h"

#include "totreewidget.h"

#include <QString>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>

#include <map>
#include <algorithm>

class toTreeWidgetItem;
class TOPrinter;
class toListTip;
class toQuery;
class toResultCols;
class toResultView;
class toSQL;
class toSearchReplace;
class toExportSettings;

/** Baseclass for filters to apply to the @ref toResultView to filter
 * out rows that you don't want to add as items to the list.
 */
class toResultFilter
{
public:
    toResultFilter() { }
    virtual ~toResultFilter() { }
    virtual void startingQuery(void) { }
    /** This function can inspect the item to be added and decide if it is
     * valid for adding or not.
     * @param item Item to inspect.
     * @return If false is returned the item isn't added.
     */
    virtual bool check(const toTreeWidgetItem *item) = 0;
    /** Create a copy of this filter.
     * @return A newly created copy of this filter.
     */
    virtual toResultFilter *clone(void) = 0;
    /** Export data to a map.
     * @param data A map that can be used to recreate the data of a chart.
     * @param prefix Prefix to add to the map.
     */
    virtual void exportData(std::map<QString, QString> &data,
                            const QString &prefix);
    /** Import data
     * @param data Data to read from a map.
     * @param prefix Prefix to read data from.
     */
    virtual void importData(std::map<QString, QString> &data,
                            const QString &prefix);
};


/** An item to display in a toListView or toResultView. They differ
 * from normal QListViewItems in that they can have a tooltip and
 * actually contain more text than is displayed in the cell of the
 * listview.
 */
class toResultViewItem : public toTreeWidgetItem
{
    struct keyData
    {
        QString Data;
        QString KeyAsc;
        QString KeyDesc;
        int Width;
        enum { String, Number } Type;
    };

    int      ColumnCount;
    keyData *ColumnData;

    QString firstText(int col) const;

protected:
    virtual int realWidth(const QFontMetrics &fm,
                          const toTreeWidget *top,
                          int column,
                          const QString &txt) const;
public:
    /** Create a new item.
     * @param parent Parent list view.
     * @param after Insert after this item.
     * @param buffer String to set as first column
     */
    toResultViewItem(toTreeWidget *parent,
                     toTreeWidgetItem *after,
                     const QString &buf = QString::null)
            : toTreeWidgetItem(parent, after, QString::null)
    {
        ColumnData = NULL;
        ColumnCount = 0;
        if (!buf.isNull())
            setText(0, buf);
    }
    /** Create a new item.
     * @param parent Parent to this item.
     * @param after Insert after this item.
     * @param buffer String to set as first column
     */
    toResultViewItem(toTreeWidgetItem *parent,
                     toTreeWidgetItem *after,
                     const QString &buf = QString::null)
            : toTreeWidgetItem(parent, after, QString::null)
    {
        ColumnData = NULL;
        ColumnCount = 0;
        if (!buf.isNull())
            setText(0, buf);
    }
    /** Reimplemented for internal reasons.
     */
    virtual ~toResultViewItem()
    {
        delete[] ColumnData;
    }
    /** Reimplemented for internal reasons.
     */
    virtual void setText(int col, const QString &txt);
    /** Set from database.
     */
    virtual void setText(int col, const toQValue &val);
    /** Reimplemented for internal reasons.
     */
    virtual void paintCell(QPainter * p,
                           const QColorGroup & cg,
                           int column,
                           int width,
                           int align);
    /** Reimplemented for internal reasons.
     */
    virtual QString text(int col) const;
    /** String to sort the data on. This is reimplemented so that
     * numbers are sorted as numbers and not as strings.
     * @param col Column
     * @param asc Wether to sort ascending or not.
     */
    virtual QString key(int col, bool asc) const
    {
        if (col >= ColumnCount)
            return QString::null;
        return asc ? ColumnData[col].KeyAsc : ColumnData[col].KeyDesc;
    }
    /** Reimplemented for internal reasons.
     */
    virtual int width(const QFontMetrics &, const toTreeWidget *, int col) const
    {
        if (col >= ColumnCount)
            return 0;
        return ColumnData[col].Width;
    }
    /** Get all text for this item. This is used for copying, drag &
     * drop and memo editing etc.
     * @param col Column.
     * @return All of the text.
     */
    virtual QString allText(int col) const
    {
        return toTreeWidgetItem::text(col);
    }
    /** Get the text to be displayed as tooltip for this item.
     * @param col Column.
     * @return The text to display as tooltip.
     */
    virtual QString tooltip(int col) const
    {
        return allText(col);
    }
};


/** This item expands the height to commodate all lines in the input
 * buffer.
 */
class toResultViewMLine : public toResultViewItem
{
private:
    /** Number of lines in the largest row.
     */
    int Lines;

protected:
    virtual int realWidth(const QFontMetrics &fm,
                          const toTreeWidget *top,
                          int column,
                          const QString &txt) const;

public:
    /** Create a new item.
     * @param parent Parent list view.
     * @param after Insert after this item.
     * @param buffer String to set as first column
     */
    toResultViewMLine(toTreeWidget *parent,
                      toTreeWidgetItem *after,
                      const QString &buf = QString::null)
            : toResultViewItem(parent, after, QString::null)
    {
        Lines = 1;
        if (!buf.isNull())
            setText(0, buf);
    }
    /** Create a new item.
     * @param parent Parent to this item.
     * @param after Insert after this item.
     * @param buffer String to set as first column
     */
    toResultViewMLine(toTreeWidgetItem *parent,
                      toTreeWidgetItem *after,
                      const QString &buf = QString::null)
            : toResultViewItem(parent, after, QString::null)
    {
        Lines = 1;
        if (!buf.isNull())
            setText(0, buf);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void setText(int, const QString &);
    /** Set from database.
     */
    virtual void setText(int col, const toQValue &val);
    /** Reimplemented for internal reasons.
     */
    virtual void setup(void);
    /** Reimplemented for internal reasons.
     */
    virtual QString text(int col) const
    {
        return toResultViewItem::allText(col);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void paintCell(QPainter *pnt,
                           const QColorGroup & cg,
                           int column,
                           int width,
                           int alignment);
};


/** An item to display in a toListView or toResultView. They differ
 * from normal QListViewItems in that they can have a tooltip and
 * actually contain more text than is displayed in the cell of the
 * listview.
 */
class toResultViewCheck : public toTreeWidgetCheck
{
    struct keyData
    {
        QString Data;
        QString KeyAsc;
        QString KeyDesc;
        int Width;
        enum { String, Number } Type;
    };

    int      ColumnCount;
    keyData *ColumnData;

protected:
    virtual int realWidth(const QFontMetrics &fm,
                          const toTreeWidget *top,
                          int column,
                          const QString &txt) const;

    QString firstText(int col) const;

public:
    /** Create a new item.
     * @param parent Parent list view.
     * @param text Text of first column.
     * @param type Type of check on this item.
     */
    toResultViewCheck(toTreeWidget *parent,
                      const QString &text,
                      toTreeWidgetCheck::Type type = Controller)
            : toTreeWidgetCheck(parent, QString::null, type)
    {
        ColumnData = NULL;
        ColumnCount = 0;
        if (!text.isNull())
            setText(0, text);
    }
    /** Create a new item.
     * @param parent Parent item.
     * @param text Text of first column.
     * @param type Type of check on this item.
     */
    toResultViewCheck(toTreeWidgetItem *parent,
                      const QString &text,
                      toTreeWidgetCheck::Type type = Controller)
            : toTreeWidgetCheck(parent, QString::null, type)
    {
        ColumnData = NULL;
        ColumnCount = 0;
        if (!text.isNull())
            setText(0, text);
    }
    /** Create a new item.
     * @param parent Parent list view.
     * @param after After last item.
     * @param text Text of first column.
     * @param type Type of check on this item.
     */
    toResultViewCheck(toTreeWidget *parent,
                      toTreeWidgetItem *after,
                      const QString &text,
                      toTreeWidgetCheck::Type type = Controller);
    /** Create a new item.
     * @param parent Parent item.
     * @param after After last item.
     * @param text Text of first column.
     * @param type Type of check on this item.
     */
    toResultViewCheck(toTreeWidgetItem *parent,
                      toTreeWidgetItem *after,
                      const QString &text,
                      toTreeWidgetCheck::Type type = Controller);
    /** Reimplemented for internal reasons.
     */
    virtual ~toResultViewCheck()
    {
        delete[] ColumnData;
    }
    /** Reimplemented for internal reasons.
     */
    virtual void setText(int col, const QString &txt);
    /** Set from database.
     */
    virtual void setText(int col, const toQValue &val);
    /** Reimplemented for internal reasons.
     */
    virtual void paintCell(QPainter * p,
                           const QColorGroup & cg,
                           int column,
                           int width,
                           int align);
    /** Reimplemented for internal reasons.
     */
    virtual QString text(int col) const;
    /** String to sort the data on. This is reimplemented so that
     * numbers are sorted as numbers and not as strings.
     * @param col Column
     * @param asc Wether to sort ascending or not.
     */
    /** String to sort the data on. This is reimplemented so that
     * numbers are sorted as numbers and not as strings.
     * @param col Column
     * @param asc Wether to sort ascending or not.
     */
    virtual QString key(int col, bool asc) const
    {
        if (col >= ColumnCount)
            return QString::null;
        return asc ? ColumnData[col].KeyAsc : ColumnData[col].KeyDesc;
    }
    /** Reimplemented for internal reasons.
     */
    virtual int width(const QFontMetrics &,
                      const toTreeWidget *,
                      int col) const
    {
        if (col >= ColumnCount)
            return 0;
        return ColumnData[col].Width;
    }
    /** Get all text for this item. This is used for copying, drag &
     * drop and memo editing etc.
     * @param col Column.
     * @return All of the text.
     */
    virtual QString allText(int col) const
    {
        return text(col);
    }
    /** Get the text to be displayed as tooltip for this item.
     * @param col Column.
     * @return The text to display as tooltip.
     */
    virtual QString tooltip(int col) const
    {
        return allText(col);
    }
};

/** This item expands the height to commodate all lines in the input buffer.
 */
class toResultViewMLCheck : public toResultViewCheck
{
private:
    /** Number of lines in the largest row.
     */
    int Lines;
protected:
    virtual int realWidth(const QFontMetrics &fm,
                          const toTreeWidget *top,
                          int column,
                          const QString &txt) const;
public:
    /** Create a new item.
     * @param parent Parent list view.
     * @param text Text of first column.
     * @param type Type of check on this item.
     */
    toResultViewMLCheck(toTreeWidget *parent,
                        const QString &text,
                        toTreeWidgetCheck::Type type = Controller)
            : toResultViewCheck(parent, QString::null, type)
    {
        Lines = 1;
        if (!text.isNull())
            setText(0, text);
    }
    /** Create a new item.
     * @param parent Parent item.
     * @param text Text of first column.
     * @param type Type of check on this item.
     */
    toResultViewMLCheck(toTreeWidgetItem *parent,
                        const QString &text,
                        toTreeWidgetCheck::Type type = Controller)
            : toResultViewCheck(parent, QString::null, type)
    {
        Lines = 1;
        if (!text.isNull())
            setText(0, text);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void setup(void);
    /** Reimplemented for internal reasons.
     */
    virtual void setText(int, const QString &);
    /** Set from database.
     */
    virtual void setText(int col, const toQValue &val);
    /** Reimplemented for internal reasons.
     */
    virtual QString text(int col) const
    {
        // ladies and gentlemen, welcome to the Infinite Recursion.
        // And now... KABOOM!
//         return toResultViewCheck::allText(col);
        return toResultViewCheck::text(col);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void paintCell(QPainter *pnt, const QColorGroup & cg, int column, int width, int alignment);
};

/**
 * The TOra implementation of a listview which offers a few extra
 * goodies to the baseclass.  First of all tooltip which can display
 * contents that doesn't fit in the list, printing, integration into
 * toMain with Edit menu etc, drag & drop, export as file, display
 * item as memo and context menu.
 */
class toListView : public toTreeWidget, public toEditWidget
{
    Q_OBJECT;

    /**
     * Name of this list, used primarily when printing. Also used to
     * be able to edit
     *
     * SQL displayed in @ref toResultView.
     */
    QString Name;
    /** Used to display tip on fields.
     */
    toListTip *AllTip;
    /** Item selected when popup menu displayed.
     */
    toTreeWidgetItem *MenuItem;
    /** Column of item selected when popup menu displayed.
     */
    int MenuColumn;
    /** Last move, used to determine if drag has started.
     */
    QPoint LastMove;

    /** Reimplemented for internal reasons.
     */
    virtual void contentsMouseDoubleClickEvent(QMouseEvent *e);
    /** Reimplemented for internal reasons.
     */
    virtual void contentsMousePressEvent(QMouseEvent *e);
    /** Reimplemented for internal reasons.
     */
    virtual void contentsMouseReleaseEvent(QMouseEvent *e);
    /** Reimplemented for internal reasons.
     */
    virtual void contentsMouseMoveEvent(QMouseEvent *e);

#if 0
    /** Used to print one page of the list.
     * @param printer Printer to print to.
     * @param painter Painter to print page to.
     * @param top Item at top of page.
     * @param column Column to start printing at. Will be changed to where you are when done.
     * @param level The indentation level of the top item.
     * @param pageNo Page number.
     * @param paint If just testing to determine how many pages are needed set this to false.
     * @return The next item to print to (Pass as top to this function).
     */
    virtual toTreeWidgetItem *printPage(
        TOPrinter *printer,
        QPainter *painter,
        toTreeWidgetItem *top,
        int &column,
        int &level,
        int pageNo,
        bool paint = true);
#endif

    QString owner;
    QString objectName;

protected:
    /**
     * context menu items. may be null.
     *
     */
    QMenu   *Menu;
    QAction *displayAct;
    QAction *leftAct;
    QAction *rightAct;
    QAction *centerAct;
    QAction *copyAct;
    QAction *copyFormatAct;
    QAction *copyTransAct;
    QAction *selectAllAct;
    QAction *exportAct;
    QAction *editAct;

public:

    /** Create new list view.
     * @param parent Parent of list.
     * @param name Name of list.
     * @param f Widget flags.
     */
    toListView(QWidget *parent, const char *name = NULL, Qt::WFlags f = 0);
    virtual ~toListView();

    /** Get SQL name of list.
     */
    virtual QString sqlName(void)
    {
        return Name;
    }
    /** Set SQL name of list.
     */
    virtual void setSQLName(const QString &name)
    {
        Name = name;
    }

    /** Set owner
     * introduced to get type information for fields
     */
    virtual void setOwner(QString const & tOwner)
    {
        owner = tOwner;
    }


    /** Set object name
     * introduced to get type information for fields
     */
    virtual void setObjectName(QString const & tObjectName)
    {
        objectName = tObjectName;
    }

    /** Get owner
     * introduced to get type information for fields
     */
    virtual QString getOwner()
    {
        return owner;
    }


    /** Get object name
     * introduced to get type information for fields
     */
    virtual QString getObjectName()
    {
        return objectName;
    }

    /** Get the whole text for the item and column selected when menu
     * was poped up.
     */
    QString menuText(void);

    /** Print this list
     */
    virtual void editPrint(void);
    /** Reimplemented for internal reasons.
     */
    virtual void focusInEvent(QFocusEvent *e);
    /** The string to be displayed in the middle of the footer when printing.
     * @return String to be placed in middle.
     */
    virtual QString middleString()
    {
        return QString::null;
    }
    /** Adds option to add menues to the popup menu before it is displayed.
     * @param menu Menu to add entries to.
     */
    virtual void addMenues(QMenu *menu);
    /** Export list as a string.
     */
    virtual QString exportAsText(toExportSettings settings);
    /** Export list as file.
     */
    virtual bool editSave(bool ask);

    /** Select all contents.
     */
    virtual void editSelectAll(void)
    {
        selectAll(true);
    }

    /** Search for next entry
     * @return True if found, should select the found text.
     */
    virtual bool searchNext(const QString & text);
    virtual bool searchPrevious(const QString & text);
    virtual void searchReplace(const QString & text) {};
    virtual void searchReplaceAll(const QString & text) {};
    virtual bool searchCanReplace(bool all) { return false; };

    /** Export data to a map.
     * @param data A map that can be used to recreate the data of a chart.
     * @param prefix Prefix to add to the map.
     */
    virtual void exportData(std::map<QString, QString> &data,
                            const QString &prefix);
    /** Import data
     * @param data Data to read from a map.
     * @param prefix Prefix to read data from.
     */
    virtual void importData(std::map<QString, QString> &data,
                            const QString &prefix);
    /** Create transposed copy of list
     * @return Pointer to newly allocated transposed listview.
     */
    virtual toListView *copyTransposed(void);
signals:
    /** Called before the menu is displayed so that you can add items
     * to it before it is shown.
     *
     * @param menu Pointer to the menu about to be shown.
     */
    void displayMenu(QMenu *menu);
public slots:
    /** set the popup menu --> see displayMenu()
     * @param item Item to display.
     */
    virtual void setDisplayMenu(QMenu *item);
    /** Display the menu at the given point and column.
     * @param item Item to display.
     * @param pnt Point to display menu at.
     * @param col Column to display menu for.
     */
    virtual void displayMenu(const QPoint &);
    /** Display memo of selected menu column
     */
    virtual void displayMemo(void);
protected slots:
    /** Callback when menu is selected. If you override this make sure you
     * call the parents function when you have parsed your entries.
     * @param id ID of the menu item selected.
     */
    virtual void menuCallback(QAction *);
};

/**
 * This class defines a list which displays the result of a query.
 *
 * One special thing to know about this class is that columns at the
 * end in which the description start with a '-' characters are not
 * displayed.
 */

class toResultView : public toListView, public toResult
{
    Q_OBJECT;

    int  SortColumn;
    bool SortAscending;
    bool SortConnected;

    /** Reimplemented for internal reasons.
     */
    virtual void keyPressEvent(QKeyEvent * e);

protected:
    /**
     * Context Menu action to read all records
     *
     */
    QAction *ReadAllAct;

    /** Connection to execute statement on.
     */
    toQuery *Query;
    /** Last added item.
     */
    toTreeWidgetItem *LastItem;

    /** Number of rows in list.
     */
    int RowNumber;
    /** If column names are to be made more readable.
     */
    bool ReadableColumns;
    /** Wether to display first number column or not.
     */
    bool NumberColumn;
    /** If all the available data should be read at once.
     */
    bool ReadAll;
    /** Input filter if any.
     */
    toResultFilter *Filter;

    /** Setup the list.
     * @param readable Wether to display first number column or not.
     * @param dispCol Wether to display first number column or not.
     */
    void setup(bool readable, bool dispCol);

    /** Check if end of query is detected yet or not.
     */
    virtual bool eof(void);

public:
    /** Create list.
     * @param readable Indicate if columns are to be made more
     * readable. This means that the descriptions are capitalised and
     * '_' are converted to ' '.
     * @param numCol If number column is to be displayed.
     * @param parent Parent of list.
     * @param name Name of widget.
     * @param f Widget flags.
     */
    toResultView(bool readable,
                 bool numCol,
                 QWidget *parent,
                 const char *name = NULL,
                 Qt::WFlags f = 0);

    /** Create list. The columns are not readable and the number
     * column is displayed.
     * @param parent Parent of list.
     * @param name Name of widget.
     * @param f Widget flags.
     */
    toResultView(QWidget *parent, const char *name = NULL, Qt::WFlags f = 0);
    ~toResultView();

    /** Set the read all flag.
     * @param all New value of flag.
     */
    void setReadAll(bool all)
    {
        ReadAll = all;
    }

    /** Get read all flag
     * @return Value of read all flag.
     */
    virtual void editReadAll(void);

    /** Get the number of columns in query.
     * @return Columns in query.
     */
    int queryColumns() const;

    /** Get the query used to execute this.
     */
    toQuery *query()
    {
        return Query;
    }

    /** Set a filter to this list.
     * @param filter The new filter or NULL if no filter is to be used.
     */
    void setFilter(toResultFilter *filter)
    {
        Filter = filter;
    }
    /** Get the current filter.
     * @return Current filter or NULL if no filter.
     */
    toResultFilter *filter(void)
    {
        return Filter;
    }

    /** Get number column flag.
     * @return Wether or not the numbercolumn is displayed.
     */
    bool numberColumn() const
    {
        return NumberColumn;
    }
    /** Set number column flag. Don't change this while a query is
     * running. Observe that not all descendants of this class support
     * changing this on the fly. The base
     * class and @ref toResultTableView does though.
     * @param val New value of number column.
     */
    void setNumberColumn(bool val)
    {
        NumberColumn = val;
    }

    /** Get readable column flag.
     * @return Wether or not the readable column names.
     */
    bool readableColumn() const
    {
        return ReadableColumns;
    }
    /** Set readable column flag.
     */
    void setReadableColumns(bool val)
    {
        ReadableColumns = val;
    }

    /** Create a new item in this list. Can be used if a special kind
     * of item is wanted in the list. The rest of the columns will be
     * filled with setText.
     * @param last Where to insert the item.
     * @param str String to set first column to.
     * @return Allocated item.
     */
    virtual toTreeWidgetItem *createItem(toTreeWidgetItem *last,
                                         const QString &str);

    /** Reimplemented for internal reasons.
     */
    virtual void query(const QString &sql, const toQList &param);

    /** Get SQL name of list.
     */
    virtual QString sqlName(void)
    {
        return toListView::sqlName();
    }
    /** Set SQL name of list.
     */
    virtual void setSQLName(const QString &name)
    {
        toListView::setSQLName(name);
    }

    // Why are these needed?
#if 1
    /** Set the SQL statement of this list
     * @param sql String containing statement.
     */
    void setSQL(const QString &sql)
    {
        toResult::setSQL(sql);
    }
    /** Set the SQL statement of this list. This will also affect @ref Name.
     * @param sql SQL containing statement.
     */
    void setSQL(const toSQL &sql)
    {
        toResult::setSQL(sql);
    }
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const QString &sql)
    {
        toResult::query(sql);
    }
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const toSQL &sql)
    {
        toResult::query(sql);
    }
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const toSQL &sql, toQList &par)
    {
        toResult::query(sql, par);
    }
#endif

    /** Reimplemented for internal reasons.
     */
    virtual void editPrint(void)
    {
        editReadAll();
        toListView::editPrint();
    }
    /** Reimplemented for internal reasons.
     */
    virtual QString middleString();

    /** Reimplemented for internal reasons.
     */
    virtual void addMenues(QMenu *);
    /** Reimplemented for internal reasons.
     */
    virtual void setSorting(int col, bool asc = true);
    /** Reimplemented for internal reasons.
     */
    virtual int sortColumn() const
    {
        return SortColumn;
    }

public slots:
    /** Reimplemented for internal reasons.
     */
    virtual void refresh(void);
    /** Reimplemented for internal reasons.
     */
    virtual void changeParams(const QString &Param1)
    {
        toResult::changeParams(Param1);
    }
    /** Reimplemented For internal reasons.
     */
    virtual void changeParams(const QString &Param1,
                              const QString &Param2)
    {
        toResult::changeParams(Param1, Param2);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void changeParams(const QString &Param1,
                              const QString &Param2,
                              const QString &Param3)
    {
        toResult::changeParams(Param1, Param2, Param3);
    }
    /** Try to add an item to the list if available.
     */
    virtual void addItem(void);
    /** Handle any connection by default
     */
    virtual bool canHandle(toConnection &)
    {
        return true;
    }
private slots:
    void headingClicked(int col);
    void checkHeading(void);
protected slots:
    /** Reimplemented for internal reasons.
     */
    virtual void menuCallback(QAction *);
};

#endif
