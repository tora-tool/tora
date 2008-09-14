
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#ifndef TORESULTTABLEVIEW_H
#define TORESULTTABLEVIEW_H

#include "config.h"
#include "toqvalue.h"
#include "tosql.h"
#include "toresult.h"
#include "toconnection.h"
#include "toresultmodel.h"
#include "toeditwidget.h"

#include <QObject>
#include <QAbstractTableModel>
#include <QTableView>
#include <QModelIndex>
#include <QList>
#include <QHeaderView>
#include <QMenu>
#include <QLabel>
#include <QPushButton>

class toResultStats;
class toViewFilter;
class toTableViewIterator;
class toWorkingWidget;


class toResultTableView : public QTableView,
            public toResult,
            public toEditWidget
{
    Q_OBJECT;

    // Widget to store query statistics to.
    toResultStats *Statistics;

    // if edit delegate should be created
    bool Editable;

    // if all records should be read
    bool ReadAll;

    // if column headers should be modified to be readable
    bool ReadableColumns;

    // if vertical header should be displayed
    bool NumberColumn;

    // if user resized columns
    bool ColumnsResized;

    // filter object if set
    toViewFilter *Filter;

    // superimposed until model is ready
    toWorkingWidget *Working;

    // set true when model has signaled
    bool Ready;


    /**
     * context menu items. may be null.
     *
     */
    QMenu   *Menu;
    QAction *displayAct;
    QAction *refreshAct;
    QAction *leftAct;
    QAction *rightAct;
    QAction *centerAct;
    QAction *copyAct;
    QAction *copySelAct;
    QAction *copyHeadAct;
    QAction *copyTransAct;
    QAction *selectAllAct;
    QAction *exportAct;
    QAction *editAct;
    QAction *rowCountAct;
    QAction *readAllAct;

    void createActions(void);

    // use Filter to hide rows
    void applyFilter(void);

protected slots:
    void displayMenu(const QPoint &pos);
    void menuCallback(QAction *action);
    void handleDone(void);
    void handleReset(void);
    void handleFirst(const toConnection::exception &res,
                     bool error);
    void handleDoubleClick(const QModelIndex &);
    // override parent
    virtual void selectionChanged(const QItemSelection &selected,
                                  const QItemSelection &deselected);

    // apply column rules, numbercolumn, readable columns
    void applyColumnRules(void);

protected:
    // pointer to model
    QPointer<toResultModel> Model;

    /**
     * overridden from parent.
     *
     * Returns the size hint for the given row's height or -1 if there
     * is no model.
     *
     */
    virtual int sizeHintForRow(int row) const;


    /**
     * overrides parent to overlay working working message until model
     * is ready.
     *
     */
    virtual void paintEvent(QPaintEvent *event);

signals:

    /**
     * Called before the menu is displayed so that you can add items
     * to it before it is shown.
     *
     * @param menu Pointer to the menu about to be shown.
     */
    void displayMenu(QMenu *menu);


    /**
     * Emitted when table's selection changes.
     *
     */
    void selectionChanged(void);

public:

    typedef toTableViewIterator iterator;


    /**
     * Creates a new tableview.
     *
     * @param readable use readable headers
     * @param numberColumn display vertical header
     * @param parent qobject parent
     * @param name sets objectName property
     * @param editable data is editable. you must setModel() with something
     * able to modify data.
     */
    toResultTableView(bool readable,
                      bool numberColumn,
                      QWidget *parent,
                      const char *name = 0,
                      bool editable = false);
    virtual ~toResultTableView(void);


    /**
     * Reimplemented to create query and new model.
     *
     */
    virtual void query(const QString &sql, const toQList &param);


    /**
     * Reimplemented to create query and new model.
     *
     */
    void query(const QString &sql)
    {
        toQList p;
        query(sql, p);
    }


    /**
     * Set statistics widget.
     * @param stats Statistics widget.
     */
    void setStatistics(toResultStats *stats)
    {
        Statistics = stats;
    }


    /**
     * override parent to return toResultModel pointer.
     *
     */
    toResultModel* model(void) const
    {
        return Model;
    }


    /**
     * True if query is running.
     *
     */
    bool running(void);


    /**
     * Enable or disable vertical header
     *
     */
    void setNumberColumn(bool b)
    {
        NumberColumn = b;
        verticalHeader()->setVisible(b);
    }


    /**
     * True by default
     *
     */
    virtual bool canHandle(toConnection &)
    {
        return true;
    }


    /**
     * Set a filter to this list.
     *
     * @param filter The new filter or NULL if no filter is to be used.
     */
    void setFilter(toViewFilter *filter)
    {
        Filter = filter;
    }


    /**
     * Sets the model for the view to present.
     *
     * Must be set before query()
     */
    virtual void setModel(toResultModel *model);


    /**
     * Should view read all data.
     *
     */
    virtual void setReadAll(bool b)
    {
        ReadAll = b;
    }


    /**
     * Convenience function to determine if the row indicated by index
     * is selected. Disregards column information.
     *
     */
    virtual bool isRowSelected(QModelIndex index);


    /**
     * Convenience function to return the first selected index. Useful
     * for tableviews that can only have one selected row.
     *
     * @param col returns an index with column col
     */
    virtual QModelIndex selectedIndex(int col = 1);


    /**
     * Returns 1 if number column is displayed or 0 if not.
     *
     */
    virtual int numberColumn(void)
    {
        if (NumberColumn)
            return 1;
        return 0;
    }


    /**
     * Helper function to display a toResultListFormat.
     *
     * @param separator
     * @param delimiter
     */
    virtual int exportType(QString &separator, QString &delimiter);


    /**
     * Export list as a string.
     *
     * @param includeHeader Include header.
     * @param onlySelection Only include selection.
     * @param type Format of exported list.
     * @param separator Separator for CSV format.
     * @param delimiter Delimiter for CSV format.
     */
    QString exportAsText(bool includeHeader,
                         bool onlySelection,
                         int type,
                         QString &separator,
                         QString &delimiter);


    // ---------------------------------------- overrides toEditWidget

    /**
     * Perform a save on this widget.
     *
     * @param askfile Ask filename even if default filename is available.
     */
    virtual bool editSave(bool askfile);


    /**
     * Print this widgets contents.
     *
     */
    virtual void editPrint(void);


    /**
     * Perform copy.
     *
     */
    virtual void editCopy(void);


    /**
     * Select all contents.
     *
     */
    virtual void editSelectAll(void);



public slots:
    /**
     * Resizes all columns based on the size hints of the delegate
     * used to render each item in the columns.
     *
     * overridden from parent
     */
    void resizeColumnsToContents(void);


    /**
     * Connected to horizontal header so we know when a column was
     * resized. Prevents resizeColumnsToContents from further
     * modifying column widths.
     *
     */
    void columnWasResized(int, int, int);


    /**
     * Overrides toResult to keep column sizes and declare a slot.
     *
     */
    virtual void refresh(void);


    /**
     * Stop running query.
     *
     */
    void stop(void)
    {
        if (Model)
            Model->stop();
    }

signals:

    /**
     * Emitted when the first result is available.
     *
     * @param sql SQL that was run.
     * @param res String describing result.
     * @param error Error has occurred.
     */
    void firstResult(const QString &sql,
                     const toConnection::exception &res,
                     bool error);


    /**
     * Emitted when query is finished.
     *
     */
    void done(void);
};



/**
 * A simple iterator to walk the rows of a QTableView.
 *
 */
class toTableViewIterator
{
    int                         _row;
    QPointer<toResultTableView> _view;
    QModelIndex                 _index;

    friend class toTableViewIteratorItem;

    // update _index
    void updateIndex(void);

public:

    /**
     * Create an iterator starting at 0 on a view
     *
     */
    toTableViewIterator(toResultTableView *view);


    /**
     * Create a copy of other
     *
     */
    toTableViewIterator(toTableViewIterator &other);


    /**
     * Get a QModelIndex for this row. May be invalid if you've gone
     * too far, use QModelIndex.isValid()
     *
     */
    QModelIndex operator*() const;


    /**
     * Increment by one
     *
     */
    toTableViewIterator& operator++();


    /**
     * Postfix operator
     *
     */
    const toTableViewIterator operator++(int);


    /**
     * Go forward by n
     *
     */
    toTableViewIterator& operator+=(int n);


    /**
     * Decrement by one
     *
     */
    toTableViewIterator& operator--();


    /**
     * Postfix operator
     *
     */
    const toTableViewIterator operator--(int);


    /**
     * Go back by n
     *
     */
    toTableViewIterator& operator-=(int n);


    /**
     * Make a copy of it and return a reference
     *
     */
    toTableViewIterator& operator=(const toTableViewIterator &it);
};


/**
 * Baseclass for filters to apply to the view to hide out rows that
 * you don't want.
 *
 */
class toViewFilter
{

public:
    toViewFilter()
    {
    }

    virtual ~toViewFilter()
    {
    }

    virtual void startingQuery(void)
    {
    }

    /**
     *  This function can inspect the item to be added and decide if
     *  it is valid for adding or not.
     *
     * @param model toResultModel for fetching data.
     * @param row to inspect.
     * @return If false is returned the item isn't added.
     */
    virtual bool check(const toResultModel *model, const int row) = 0;


    /**
     * Create a copy of this filter.
     *
     * @return A newly created copy of this filter.
     */
    virtual toViewFilter *clone(void) = 0;


    /**
     * Export data to a map.
     *
     * @param data A map that can be used to recreate the data of a chart.
     * @param prefix Prefix to add to the map.
     */
    virtual void exportData(std::map<QString, QString> &data,
                            const QString &prefix)
    {
    }


    /**
     * Import data
     *
     * @param data Data to read from a map.
     * @param prefix Prefix to read data from.
     */
    virtual void importData(std::map<QString, QString> &data,
                            const QString &prefix)
    {
    }
};


#endif
