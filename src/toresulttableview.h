
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

#include <QAbstractTableModel>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QModelIndex>
#include <QObject>
#include <QPushButton>
#include <QTableView>

class toResultStats;
class toViewFilter;
class toTableViewIterator;
class toWorkingWidget;
class toExportSettings;


/**
 * This is a simple class for providing sensible size hints to the
 * view.
 *
 */
class toResultTableViewDelegate : public QItemDelegate
{
    static const int maxWidth = 200; // the maximum size to grow a column

public:
    toResultTableViewDelegate(QObject *parent = 0)
        : QItemDelegate(parent)
    {
    }


    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
    {
        QSize size = QItemDelegate::sizeHint(option, index);
        if (size.width() > maxWidth)
            size.setWidth(maxWidth);

        return size;
    }
};


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

    // number of visible columns
    int VisibleColumns;

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

    // set true when model has finished.
    // helps work around determining when query.eof has been reached.
    bool Finished;

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
    QAction *copyFormatAct;
    QAction *copyTransAct;
    QAction *selectAllAct;
    QAction *exportAct;
    QAction *editAct;
    QAction *rowCountAct;
    QAction *readAllAct;

    void createActions(void);

    /*! \brief Common setup function called from constructors
    */
    void setup(bool readable, bool numberColumn, bool editable);


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

    QString Owner;
    QString Table;

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


    /**
     * Overrides QWidget to resize columns when applicable.
     *
     */
    virtual void resizeEvent(QResizeEvent *event);

    /*! Catch special keyboard shortcuts.
        Copy, etc.
    */
    virtual void keyPressEvent(QKeyEvent * event);

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
    /*! \brief Constructor provided for Qt designer. See setup()
    */
    toResultTableView(QWidget * parent = 0);
    virtual ~toResultTableView(void);

    virtual bool searchNext(const QString & text);
    virtual bool searchPrevious(const QString & text);
    virtual void searchReplace(const QString & text) {};
    virtual void searchReplaceAll(const QString & text) {};
    virtual bool searchCanReplace(bool all)
    {
        return false;
    };

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
    void setFilter(toViewFilter *filter);


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
     * Export list as a string.
     *
     */
    QString exportAsText(toExportSettings settings);


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


    /**
     * apply Filter to row visibility
     *
     */
    void applyFilter(void);


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
