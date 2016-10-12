
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

#pragma once

#include "core/toqvalue.h"
#include "core/tosql.h"
#include "core/toresult.h"
#include "core/utils.h"
#include "core/toconnection.h"
#include "widgets/toresultmodel.h"
#include "core/toeditwidget.h"

#include <QtCore/QAbstractTableModel>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLabel>
#include <QtCore/QList>
#include <QMenu>
#include <QtCore/QModelIndex>
#include <QtCore/QObject>
#include <QPushButton>
#include <QTableView>

class toResultStats;
class toViewFilter;
class toTableViewIterator;
class toWorkingWidget;
class toExportSettings;
class toSearchReplace;

class toResultTableView : public QTableView, public toResult, public toEditWidget
{
        Q_OBJECT;
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

        /**
         * Reimplemented to create query and new model
         */
        void query(const QString &sql, toQueryParams const& param) override;

        void querySub(QSharedPointer<toConnectionSubLoan> &con, const QString &sql, toQueryParams const& param);

        /** Clear result widget */
        void clearData() override;

        /**
         * Set statistics widget.
         * @param stats Statistics widget.
         */
        void setStatistics(toResultStats *stats)
        {
            Statistics = stats;
        }

        /**
         * override parent to return toResultModel pointer
         */
        toResultModel* model(void) const
        {
            return Model;
        }

        /**
         * True if query is running
         */
        bool running(void);


        /**
         * Enable or disable vertical header
         */
        void setNumberColumn(bool b)
        {
            NumberColumn = b;
            verticalHeader()->setVisible(b);
        }

        /**
         * True by default
         */
        bool canHandle(const toConnection &) override
        {
            return true;
        }

        /**
         * apply Filter to row visibility
         */
        void applyFilter(void);

        /**
         * Sets the model for the view to present.
         *
         * Must be set before query()
         */
        virtual void setModel(toResultModel *model);
	void setModel(QAbstractItemModel *model) override;

        /**
         * Should view read all data.
         */
        virtual void setReadAll(bool b)
        {
            ReadAll = b;
        }

        /**
         * Convenience function to determine if the row indicated by index
         * is selected. Disregards column information.
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
         * Export list as a string.
         */
        QString exportAsText(toExportSettings settings);
        // ----- overrides toEditWidget
        /**
         * Perform a save on this widget.
         *
         * @param askfile Ask filename even if default filename is available.
         */
        bool editSave(bool askfile) override;

        /**
         * Print this widgets contents.
         */
        void editPrint(void) override;

        /**
         * Perform copy.
         */
        void editCopy(void) override;

        /**
         * Select all contents.
         */
	void editSelectAll(void) override;

        // These methods do not make any sense for readonly "editor" like data table display
	bool editOpen(const QString&) override { return false; }
	void editUndo() override {}
	void editRedo() override {}
	void editCut()  override {}
	void editPaste() override {}
	void editReadAll() override {}
	bool searchNext() override { return false; };
        void searchReplace() override {};
	QString editText() override { return ""; }

        /** Fill in result from the cache rather than executing actual query on database.
         */
        bool queryFromCache(const QString &owner, const QString &type);

        static QMap<QString, toResultTableView*> Registry;

    public slots:
        /**
         * Set a filter to this list.
         *
         * @param filter The new filter or NULL if no filter is to be used.
         */
        void setFilter(toViewFilter *filter);

        /**
         * Resizes all columns based on the size hints of the delegate
         * used to render each item in the columns.
         *
         * overridden from parent
         */
        void slotResizeColumnsToContents(void);

        /**
         * Connected to horizontal header so we know when a column was
         * resized. Prevents resizeColumnsToContents from further
         * modifying column widths.
         */
        void slotColumnWasResized(int, int, int);

        /**
         * Overrides toResult to keep column sizes and declare a slot
         */
        void refresh(void) override;

        /**
         * Stop running query.
         */
        void slotStop(void)
        {
            if (Model)
                Model->stop();
        }

    signals:

        /**
         * Emitted when table's selection changes
         */
        void selectionChanged(void);

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
         */
        void done(void);

        /**
         * Inform upstream controller about the new Model
         */
        void modelChanged(toResultModel*);

    protected slots:
        void slotMenuCallback(QAction *action);
        void slotHandleDone(void);
        void slotHandleReset(void);
        void slotHandleFirst(const toConnection::exception &res,
                             bool error);
        virtual void slotHandleDoubleClick(const QModelIndex &);
        // override parent
        void selectionChanged(const QItemSelection &selected,
			      const QItemSelection &deselected) override;

        // apply column rules, numbercolumn, readable columns
        virtual void slotApplyColumnRules(void);

    protected:
        void contextMenuEvent(QContextMenuEvent *e) override;

        /*! \brief Common setup function called from constructors
        */
        void setup(bool readable, bool numberColumn, bool editable);

        /**
         * overridden from parent.
         *
         * Controls maximum size (height) of a given row.
         */
        int sizeHintForRow(int row) const override;

        /**
         * overridden from parent.
         *
         * Controls a maximum size (width) of a given column.
         */
        int sizeHintForColumn(int col) const override;

        /**
         * overrides parent to overlay working working message until model
         * is ready
         */
	void paintEvent(QPaintEvent *event) override;

        /**
         * Overrides QWidget to resize columns when applicable.
         */
        void resizeEvent(QResizeEvent *event) override;

        /*! Catch special keyboard shortcuts.
            Copy, etc.
        */
        void keyPressEvent(QKeyEvent * event) override;

        virtual toResultModel* allocModel(toEventQuery *);
        void freeModel();

        /*! A guessed amount of "visible" rows used as initial fetch size in the model
         *
         *  QAbstractItemView call canFetchMore/fetchMore only when scroll bar moves. This should satisfy that
         *  the first chunk of rows received from toEventQueryWorker will exceed number of visible rows in the view
         */
        int visibleRows() const;

        // pointer to model
        QPointer<toResultModel> Model;

        // Widget to store query statistics to.
        toResultStats *Statistics;

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
         * context menu items. may be null
         */
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
        QAction *rowCountAct;
        QAction *readAllAct;
};



/**
 * A simple iterator to walk the rows of a QTableView.
 *
 */
class toTableViewIterator
{
    public:
        /**
         * Create an iterator starting at 0 on a view
         */
        toTableViewIterator(toResultTableView *view);

        /**
         * Create a copy of other
         */
        toTableViewIterator(toTableViewIterator &other);

        /**
         * Get a QModelIndex for this row. May be invalid if you've gone
         * too far, use QModelIndex.isValid()
         */
        QModelIndex operator*() const;

        /**
         * Increment by one
         */
        toTableViewIterator& operator++();

        /**
         * Postfix operator
         */
        const toTableViewIterator operator++(int);

        /**
         * Go forward by n
         */
        toTableViewIterator& operator+=(int n);

        /**
         * Decrement by one
         */
        toTableViewIterator& operator--();

        /**
         * Postfix operator
         */
        const toTableViewIterator operator--(int);

        /**
         * Go back by n
         */
        toTableViewIterator& operator-=(int n);

        /**
         * Make a copy of it and return a reference
         */
        toTableViewIterator& operator=(const toTableViewIterator &it);
    private:
        int                         _row;
        QPointer<toResultTableView> _view;
        QModelIndex                 _index;

        friend class toTableViewIteratorItem;

        // update _index
        void updateIndex(void);

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

#ifdef TORA3_SESSION
        /**
         * Export data to a map.
         *
         * @param data A map that can be used to recreate the data of a chart.
         * @param prefix Prefix to add to the map.
         */
        virtual void exportData(std::map<QString, QString> &data, const QString &prefix)
        {
        }


        /**
         * Import data
         *
         * @param data Data to read from a map.
         * @param prefix Prefix to read data from.
         */
        virtual void importData(std::map<QString, QString> &data, const QString &prefix)
        {
        }
#endif
};
