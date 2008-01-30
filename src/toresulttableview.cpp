/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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

#include "config.h"
#include "toresulttableview.h"
#include "toresultmodel.h"
#include "toconf.h"
#include "utils.h"
#include "toconfiguration.h"
#include "toconnection.h"
#include "tomemoeditor.h"
#include "tomain.h"

#include <QClipboard>
#include <QScrollBar>
#include <QItemDelegate>
#include <QSize>
#include <QFont>
#include <QFontMetrics>


/**
 * This is a simple class for providing sensible size hints to the
 * view. This will probably be extended to support editing and moved
 * to it's own class.
 *
 */
class toResultTableViewDelegate : public QItemDelegate {
    static const int maxWidth = 200; // the maximum size to grow a column

public:
    toResultTableViewDelegate(QObject *parent = 0)
        : QItemDelegate(parent) {
    }


    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const {
        QSize size = QItemDelegate::sizeHint(option, index);
        if(size.width() > maxWidth)
            size.setWidth(maxWidth);

        return size;
    }
};


toResultTableView::toResultTableView(bool readable,
                                     bool numberColumn,
                                     QWidget *parent,
                                     const char *name,
                                     bool editable)
    : QTableView(parent),
      toResult(),
      Model(NULL) {

    if(name)
        setObjectName(name);

    Statistics      = NULL;
    Menu            = NULL;
    Editable        = editable;
    ReadAll         = false;
    Filter          = false;
    ReadableColumns = readable;
    NumberColumn    = numberColumn;
    ColumnsResized  = false;

    toResultTableViewDelegate *del = new toResultTableViewDelegate(this);
    setItemDelegate(del);

    createActions();

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(displayMenu(const QPoint &)));
    connect(horizontalHeader(),
            SIGNAL(sectionResized(int, int, int)),
            this,
            SLOT(columnWasResized(int, int, int)));
}


toResultTableView::~toResultTableView() {
    if(Model)
        delete Model;
    Model = NULL;
}


void toResultTableView::query(const QString &sql, const toQList &param) {
    setSQLParams(sql, param);

    toNoBlockQuery *query = NULL;
    try {
        if(running())
            Model->stop();
        delete Model;
        Model = NULL;

        query = new toNoBlockQuery(connection(),
                                   toQuery::Long,
                                   sql,
                                   param,
                                   Statistics);
        query->eof();           // checks for error and throws

        Model = new toResultModel(query,
                                  this,
                                  Editable,
                                  ReadableColumns);
        setModel(Model);

        connect(Model, SIGNAL(done()), this, SLOT(handleDone()));
        connect(Model, SIGNAL(modelReset()), this, SLOT(handleReset()));
        connect(Model,
                SIGNAL(firstResult(const QString &,
                                   const toConnection::exception &,
                                   bool)),
                this,
                SLOT(handleFirst(const QString &,
                                 const toConnection::exception &,
                                 bool)));

        setSortingEnabled(true);
    }
    catch(const toConnection::exception &str) {
        emit firstResult(toResult::sql(), str, true);
        emit done();
    }
    catch(const QString &str) {
        emit firstResult(toResult::sql(), str, true);
        emit done();
    }

    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(QFontMetrics(QFont()).height() + 4);

    horizontalHeader()->setHighlightSections(false);

    // when a new model is created the column sizes are lost
    ColumnsResized = false;
}


void toResultTableView::createActions() {
    displayAct   = new QAction(tr("&Display in editor..."), this);
    refreshAct   = new QAction(tr("&Refresh"), this);
    leftAct      = new QAction(tr("&Left"), this);
    centerAct    = new QAction(tr("&Center"), this);
    rightAct     = new QAction(tr("&Right"), this);
    copyAct      = new QAction(tr("&Copy field"), this);
    copySelAct   = new QAction(tr("Copy &selection"), this);
    copyHeadAct  = new QAction(tr("Copy selection with &header"), this);
    copyTransAct = new QAction(tr("Copy &transposed"), this);
    selectAllAct = new QAction(tr("Select &all"), this);
    exportAct    = new QAction(tr("E&xport to file..."), this);
    editAct      = new QAction(tr("&Edit SQL..."), this);
    readAllAct   = new QAction(tr("&Read All"), this);
}


void toResultTableView::applyFilter(void) {
    if(!Filter)
        return;

    Filter->startingQuery();

    for(int row = 0; row < Model->rowCount(); row++) {
        if(!Filter->check(Model, row))
            hideRow(row);
    }
}


int toResultTableView::sizeHintForRow(int row) const {
    return 5;
}


void toResultTableView::applyColumnRules(void) {
    if(!NumberColumn)
        hideColumn(0);

    int visible = 0;
    if(ReadableColumns) {
        // loop through columns and hide anything starting with a ' '
        for(int col = 1; col < model()->columnCount(); col++) {
            if(model()->headerData(
                   col,
                   Qt::Horizontal,
                   Qt::DisplayRole).toString().startsWith(" ")) {
                hideColumn(col);
            }
            else
                visible++;
        }
    }

    resizeColumnsToContents();

    if(visible == 1 && ReadableColumns)
        setColumnWidth(1, maximumViewportSize().width());
}


void toResultTableView::displayMenu(const QPoint &pos) {
    if(!Menu) {
        Menu = new QMenu(this);
        Menu->addAction(displayAct);
        Menu->addAction(refreshAct);

        QMenu *just = new QMenu(tr("A&lignment"), this);
        just->addAction(leftAct);
        just->addAction(centerAct);
        just->addAction(rightAct);
        connect(just,
                SIGNAL(triggered(QAction *)),
                this,
                SLOT(menuCallback(QAction *)));
        Menu->addAction(just->menuAction());

        Menu->addSeparator();

        Menu->addAction(copyAct);
        Menu->addAction(copySelAct);
        Menu->addAction(copyHeadAct);
        Menu->addAction(copyTransAct);

        Menu->addSeparator();

        Menu->addAction(selectAllAct);

        Menu->addSeparator();

        Menu->addAction(exportAct);

        Menu->addSeparator();

        Menu->addAction(editAct);

        Menu->addSeparator();

        Menu->addAction(readAllAct);

        connect(Menu,
                SIGNAL(triggered(QAction *)),
                this,
                SLOT(menuCallback(QAction *)));

        emit displayMenu(Menu);
    }

    Menu->exec(QCursor::pos());
}


void toResultTableView::menuCallback(QAction *action) {
    QModelIndex index = currentIndex();
    if(!index.isValid())
        return;

    QVariant data = model()->data(index, Qt::EditRole);

    if(action == displayAct) {
        toMemoEditor *ed = new toMemoEditor(this, data.toString());
        ed->exec();
    }
    else if(action == leftAct)
        Model->setAlignment(index.column(), Qt::AlignLeft);
    else if(action == rightAct)
        Model->setAlignment(index.column(), Qt::AlignRight);
    else if(action == centerAct)
        Model->setAlignment(index.column(), Qt::AlignHCenter);
    else if(action == copyAct) {
        QClipboard *clip = qApp->clipboard();

        if(data.canConvert<QString>())
            clip->setText(data.toString());
    }
    else if(action == selectAllAct) {
        selectAll();
    }
    else if(action == editAct) {
        toMainWidget()->editSQL(sqlName());
    }
    else if(action == readAllAct) {
        QModelIndex index;
        while(Model->canFetchMore(index))
            Model->fetchMore(index);
    }
    else if(action == refreshAct)
        refresh();
}


void toResultTableView::handleDone(void) {
    readAllAct->setEnabled(false);

    applyFilter();
    emit done();
}


void toResultTableView::handleReset(void) {
    if(ReadAll) {
        QModelIndex index;
        while(Model->canFetchMore(index))
            Model->fetchMore(index);
    }

    applyColumnRules();
}


void toResultTableView::handleFirst(const QString &sql,
                                    const toConnection::exception &res,
                                    bool error) {
    emit firstResult(sql, res, error);
}


void toResultTableView::selectionChanged(const QItemSelection &selected,
                                         const QItemSelection &deselected) {
    QTableView::selectionChanged(selected, deselected);
    emit selectionChanged();
}


bool toResultTableView::running(void) {
    if(!Model)
        return false;
    QModelIndex index = currentIndex();
    return Model->canFetchMore(index);
}


void toResultTableView::setModel(toResultModel *model) {
//     if(running())
//         throw tr("Cannot change model while query is running.");
    Model = QPointer<toResultModel>(model);
    QTableView::setModel(model);
}


bool toResultTableView::isRowSelected(QModelIndex index) {
    QModelIndexList sel = selectedIndexes();
    for(QModelIndexList::iterator it = sel.begin(); it != sel.end(); it++) {
        if((*it).row() == index.row())
            return true;
    }

    return false;
}


QModelIndex toResultTableView::selectedIndex(int col) {
    // should only have one anyhow. just take first.
    QModelIndexList sel = selectedIndexes();
    if(sel.size() < 1)
        return QModelIndex();
    return model()->index(sel[0].row(), col);
}


void toResultTableView::resizeColumnsToContents() {
    if(!ColumnsResized)
        QTableView::resizeColumnsToContents();
}


void toResultTableView::columnWasResized(int, int, int) {
    ColumnsResized = true;
}


void toResultTableView::refresh() {
    // todo. save column sizes. horizontalHeader()->restoreState(st);
    // didn't work.
    toResult::refresh();
}



// ---------------------------------------- iterator

/**
 * Create an iterator starting at 0 on a view
 *
 */
toTableViewIterator::toTableViewIterator(toResultTableView *view) {
    _row  = 0;
    _view = view;
    updateIndex();
}


/**
 * Create a copy of other
 *
 */
toTableViewIterator::toTableViewIterator(toTableViewIterator &other) {
    _row  = other._row;
    _view = other._view;
    updateIndex();
}


void toTableViewIterator::updateIndex(void) {
    if(_row < _view->model()->rowCount())
        _index = _view->model()->index(_row, 1); // skip numbercolumn
    else
        _index = QModelIndex();
}


/**
 * Get a QModelIndex for this row. May be invalid if you've gone
 * too far. Check with QModelIndex.isValid()
 *
 */
QModelIndex toTableViewIterator::operator*() const {
    return _index;
}


/**
 * Increment by one
 *
 */
toTableViewIterator& toTableViewIterator::operator++() {
    _row++;
    updateIndex();
    return *this;
}


/**
 * Postfix operator
 *
 */
const toTableViewIterator toTableViewIterator::operator++(int) {
    toTableViewIterator tmp(*this);
    ++(*this);
    updateIndex();
    return tmp;
}


/**
 * Go forward by n
 *
 */
toTableViewIterator& toTableViewIterator::operator+=(int n) {
    _row += n;
    updateIndex();
    return *this;
}


/**
 * Decrement by one
 *
 */
toTableViewIterator& toTableViewIterator::operator--() {
    _row--;
    updateIndex();
    return *this;
}


/**
 * Postfix operator
 *
 */
const toTableViewIterator toTableViewIterator::operator--(int) {
    toTableViewIterator tmp(*this);
    --(*this);
    updateIndex();
    return tmp;
}


/**
 * Go back by n
 *
 */
toTableViewIterator& toTableViewIterator::operator-=(int n) {
    _row -= n;
    updateIndex();
    return *this;
}


/**
 * Make a copy of it and return a reference
 *
 */
toTableViewIterator& toTableViewIterator::operator=(const toTableViewIterator &it) {
    _row  = it._row;
    _view = it._view;
    updateIndex();
    return *this;
}
