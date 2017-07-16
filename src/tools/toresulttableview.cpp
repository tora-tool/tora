
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

#include "tools/toresulttableview.h"

#include "widgets/toresultmodel.h"
#include "core/toeventquery.h"
#include "core/utils.h"
#include "core/toconfiguration.h"
#include "core/toconnection.h"
#include "editor/tomodeleditor.h"
#include "core/tomainwindow.h"
#include "widgets/toresultlistformat.h"
#include "core/tolistviewformatter.h"
#include "core/tolistviewformatterfactory.h"
#include "core/tolistviewformatteridentifier.h"
#include "widgets/toworkingwidget.h"
#include "core/toglobalconfiguration.h"
#include "core/todatabaseconfig.h"
#include "core/tocontextmenu.h"

#include <QtCore/QSize>
#include <QtCore/QTimer>
#include <QtCore/QtDebug>
#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QProgressDialog>

QMap<QString, toResultTableView*> toResultTableView::Registry;

toResultTableView::toResultTableView(QWidget * parent)
    : QTableView(parent)
    , toResult()
    , toEditWidget()
    , Model(NULL)
{
    toEditWidget::FlagSet.Save = true;
    toEditWidget::FlagSet.Copy = true;
    toEditWidget::FlagSet.Search = true;
    toEditWidget::FlagSet.SelectAll = true;

    setObjectName("toResultTableView");
    setup(true, false, false);
    Registry.insert(Utils::ptr2str(this).c_str(), this);
}

toResultTableView::toResultTableView(bool readable,
                                     bool numberColumn,
                                     QWidget *parent,
                                     const char *name,
                                     bool editable)
    : QTableView(parent)
    , toResult()
    , toEditWidget()
    , Model(NULL)
{
    toEditWidget::FlagSet.Save = true;
    toEditWidget::FlagSet.Copy = true;
    toEditWidget::FlagSet.Search = true;
    toEditWidget::FlagSet.SelectAll = true;

    if (name)
        setObjectName(name);
    setup(readable, numberColumn, editable);

    Registry.insert(Utils::ptr2str(this).c_str(), this);
}

void toResultTableView::setup(bool readable, bool numberColumn, bool editable)
{
    Statistics      = NULL;
    ReadAll         = false;
    Filter          = NULL;
    VisibleColumns  = 0;
    ReadableColumns = readable;
    NumberColumn    = numberColumn;
    ColumnsResized  = false;
    Ready           = false;
    Finished        = false;

    Working = new toWorkingWidget(this);
    connect(Working, SIGNAL(stop()), this, SLOT(slotStop()));
    Working->hide(); // hide by default

    displayAct   = new QAction(tr("&Display in editor..."), this);
    refreshAct   = new QAction(tr("&Refresh"), this);
    leftAct      = new QAction(tr("&Left"), this);
    centerAct    = new QAction(tr("&Center"), this);
    rightAct     = new QAction(tr("&Right"), this);
    copyAct      = new QAction(tr("&Copy"), this);
    copyFormatAct = new QAction(tr("Copy in &format..."), this);
    copyTransAct = new QAction(tr("Copy &transposed"), this);
    selectAllAct = new QAction(tr("Select &all"), this);
    exportAct    = new QAction(tr("E&xport to file..."), this);
    rowCountAct  = new QAction(tr("C&ount Rows"), this);
    readAllAct   = new QAction(tr("&Read All"), this);

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setAlternatingRowColors(true);

    connect(horizontalHeader(),
            SIGNAL(sectionResized(int, int, int)),
            this,
            SLOT(slotColumnWasResized(int, int, int)));
    connect(this,
            SIGNAL(doubleClicked(const QModelIndex &)),
            this,
            SLOT(slotHandleDoubleClick(const QModelIndex &)));

    setDragEnabled(true);
    setDropIndicatorShown(true);

    // this is my dirty hack to prevent fetchMore being called on the
    // model during a horizontal scroll.
    // this would be one great way to fix it, but as of this time it's
    // prioritized low and not scheduled for a fix:
    // https://bugreports.qt-project.org/browse/QTBUG-9988
    disconnect(QTableView::horizontalScrollBar(),
               SIGNAL(valueChanged(int)),
               this,
               SLOT(horizontalScrollbarValueChanged(int)));
}


toResultTableView::~toResultTableView()
{
    if (Model && running())
        Model->stop();
    freeModel();
    Registry.remove(Utils::ptr2str(this).c_str());
}

void toResultTableView::query(const QString &sql, toQueryParams const& param)
{
    setSqlAndParams(sql, param);

    TLOG(7, toDecorator, __HERE__) << "Query from toResultTableView::query :" << sql << std::endl;
    try
    {
        if (Model && running())
            Model->stop();
        freeModel();

        readAllAct->setEnabled(true);
        Ready = false;
        Finished = false;
        Working->setText(tr("Please wait..."));
        Working->hide();

        // sets visible true but won't show if parent is hidden
        QTimer::singleShot(300, Working, SLOT(forceShow()));

        toEventQuery *query = new toEventQuery(this
                                               , connection()
                                               , sql
                                               , param
                                               , toEventQuery::READ_FIRST
                                               //, Statistics
                                              );

        toResultModel *model = allocModel(query);
        setModel(model);

        connect(Model, SIGNAL(done()), this, SLOT(slotHandleDone()));
        connect(Model, SIGNAL(modelReset()), this, SLOT(slotHandleReset()));
        connect(Model,
                SIGNAL(firstResult(const toConnection::exception &, bool)),
                this,
                SLOT(slotHandleFirst(const toConnection::exception &, bool)));
        setSortingEnabled(true);
        query->start();
    }
    catch (const toConnection::exception &str)
    {
        emit firstResult(toResult::sql(), str, true);
        emit done();
        slotHandleDone();
        Utils::toStatusMessage(str);
    }
    catch (const QString &str)
    {
        emit firstResult(toResult::sql(), str, true);
        emit done();
        slotHandleDone();
        Utils::toStatusMessage(str);
    }

    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(QFontMetrics(QFont()).height() + 4);

    horizontalHeader()->setHighlightSections(false);

    // when a new model is created the column sizes are lost
    ColumnsResized = false;
}

void toResultTableView::querySub(QSharedPointer<toConnectionSubLoan> &con, const QString &sql, toQueryParams const& param)
{
    setSqlAndParams(sql, param);

    TLOG(7, toDecorator, __HERE__) << "Query from toResultTableView::querySub :" << sql << std::endl;
    try
    {
        if (Model && running())
            Model->stop();
        freeModel();

        readAllAct->setEnabled(true);
        Ready = false;
        Finished = false;
        Working->setText(tr("Please wait..."));
        Working->hide();

        // sets visible true but won't show if parent is hidden
        QTimer t(this);
        t.singleShot(300, Working, SLOT(forceShow()));

        toEventQuery *query = new toEventQuery(this
                                               , con
                                               , sql
                                               , param
                                               , toEventQuery::READ_FIRST
                                               //, Statistics
                                              );

        toResultModel *model = allocModel(query);
        setModel(model);

        connect(Model, SIGNAL(done()), this, SLOT(slotHandleDone()));
        connect(Model, SIGNAL(modelReset()), this, SLOT(slotHandleReset()));
        connect(Model,
                SIGNAL(firstResult(const toConnection::exception &, bool)),
                this,
                SLOT(slotHandleFirst(const toConnection::exception &, bool)));
        setSortingEnabled(true);
        query->start();
    }
    catch (const toConnection::exception &str)
    {
        emit firstResult(toResult::sql(), str, true);
        emit done();
        slotHandleDone();
        Utils::toStatusMessage(str);
    }
    catch (const QString &str)
    {
        emit firstResult(toResult::sql(), str, true);
        emit done();
        slotHandleDone();
        Utils::toStatusMessage(str);
    }

    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(QFontMetrics(QFont()).height() + 4);

    horizontalHeader()->setHighlightSections(false);

    // when a new model is created the column sizes are lost
    ColumnsResized = false;
}

toResultModel* toResultTableView::allocModel(toEventQuery *query)
{
    toResultModel *retval = new toResultModel(query, this, ReadableColumns);
    retval->setInitialRows(visibleRows());
    return retval;
}

void toResultTableView::freeModel()
{
    if (Model)
    {
        delete Model;
        Model = NULL;
        emit modelChanged(NULL);
    }
}

int toResultTableView::visibleRows() const
{
    // TODO this is ugly hack and needs some validation
    //this->visibleRegion();
    int h = sizeHintForRow(0);
    QAbstractItemDelegate *d = itemDelegate();
    QSize s = d->sizeHint(QStyleOptionViewItem(), QModelIndex());
    int hh = height();
    int hhh = s.height();
    int rows = hh / s.height() + 1;
    if (rows < toConfigurationNewSingle::Instance().option(ToConfiguration::Database::InitialFetchInt).toInt())
        return toConfigurationNewSingle::Instance().option(ToConfiguration::Database::InitialFetchInt).toInt();
    return rows;
}

void toResultTableView::clearData()
{
    // Note that destroying data model effectively "clears" QTableView
    if (Model && running())
        Model->stop();
    freeModel();
} // clearData

void toResultTableView::applyFilter()
{
    if (!Filter || !Model)
        return;

    Filter->startingQuery();

    setUpdatesEnabled(false);
    for (int row = 0; row < Model->rowCount(); row++)
    {
        if (!Filter->check(Model, row))
            hideRow(row);
        else
            showRow(row);
    }
    setUpdatesEnabled(true);
}


/* Controls height of all table views in TOra. Will use standart Qt function to
   calculate a row height and will control that it is not larger than a predefined
   size. Note: this height is only used in QTableView when resizeRowsToContents
   is called. */
int toResultTableView::sizeHintForRow(int row) const
{
    int s;

    s = QTableView::sizeHintForRow(row);
    if (s > 60) s = 60; // TODO: This should probably be moved to configuration file
    return s;
} // sizeHintForRow

/* Controls width of all table views in TOra. Will use standart Qt function to
   calculate a columns width and will control that it is not larger than a predefined
   size. Note: this height is only used in QTableView when resizeColumnsToContents
   is called. Column width is also adjusted when calculating width of column headers! */
int toResultTableView::sizeHintForColumn(int col) const
{
    int s;

    s = QTableView::sizeHintForColumn(col);
    if (s > 200) s = 200; // TODO: This should probably be moved to configuration file
    return s;
} // sizeHintForColumn

void toResultTableView::paintEvent(QPaintEvent *event)
{
    if (!Ready)
    {
        Working->setGeometry(this->viewport()->frameGeometry());
        Working->show();
        event->ignore();
    }
    else
    {
        Working->hide();
        QTableView::paintEvent(event);
    }
}


void toResultTableView::resizeEvent(QResizeEvent *event)
{
    if (ReadableColumns && VisibleColumns == 1)
        setColumnWidth(1, viewport()->width());
    QTableView::resizeEvent(event);

    if (!Ready)
    {
        Working->setGeometry(this->viewport()->frameGeometry());
        Working->repaint();
    }
}


void toResultTableView::keyPressEvent(QKeyEvent * event)
{
    if (event->matches(QKeySequence::Copy))
    {
        editCopy();
        return;
    }
    QTableView::keyPressEvent(event);
}


void toResultTableView::slotApplyColumnRules()
{
    // TODO: call after every model reset
    // connect(Model, SIGNAL(modelReset()), this, SLOT(slotApplyColumnRules()));
    if (!NumberColumn)
        hideColumn(0);

    if (ReadableColumns)
    {
        VisibleColumns = 0;
        // loop through columns and hide anything starting with a ' '
        for (int col = 1; col < model()->columnCount(); col++)
        {
            if (model()->headerData(
                        col,
                        Qt::Horizontal,
                        Qt::DisplayRole).toString().startsWith(" "))
            {
                hideColumn(col);
            }
            else
                VisibleColumns++;
        }
    }

    // hiding columns sends signal sectionResized
    ColumnsResized = false;

    slotResizeColumnsToContents();
    if (toConfigurationNewSingle::Instance().option(ToConfiguration::Global::MultiLineResultsBool).toBool())
        resizeRowsToContents();

    if (ReadableColumns && VisibleColumns == 1)
        setColumnWidth(1, viewport()->width());
}


void toResultTableView::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *popup = new QMenu(this);

    // Handle parent widget's context menu fields
    toContextMenuHandler::traverse(this, popup);

    popup->addAction(displayAct);
    popup->addAction(refreshAct);

    QMenu *just = new QMenu(tr("A&lignment"), this);
    just->addAction(leftAct);
    just->addAction(centerAct);
    just->addAction(rightAct);
    connect(just,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(slotMenuCallback(QAction *)));
    popup->addAction(just->menuAction());

    popup->addSeparator();

    popup->addAction(copyAct);
    popup->addAction(copyFormatAct);

    popup->addSeparator();

    popup->addAction(selectAllAct);

    popup->addSeparator();

    popup->addAction(exportAct);

    popup->addSeparator();

    popup->addAction(rowCountAct);
    popup->addAction(readAllAct);

    connect(popup,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(slotMenuCallback(QAction *)));

    // Display and "run" the menu
    e->accept();
    popup->exec(e->globalPos());
    delete popup;
}


void toResultTableView::slotMenuCallback(QAction *action)
{
    QModelIndex index = currentIndex();
    if (!index.isValid())
        return;

    if (action == displayAct)
    {
        QVariant data = model()->data(index, Qt::EditRole);
#if TORA3_MEMOEDITOR
        toModelEditor *ed = new toModelEditor(this, model(), index);
        ed->exec();
#endif
    }
    else if (action == leftAct)
        Model->setAlignment(index.column(), Qt::AlignLeft);
    else if (action == rightAct)
        Model->setAlignment(index.column(), Qt::AlignRight);
    else if (action == centerAct)
        Model->setAlignment(index.column(), Qt::AlignHCenter);
    else if (action == copyAct)
        editCopy();
    else if (action == selectAllAct)
        editSelectAll();
    else if (action == readAllAct || action == rowCountAct)
    {
        Model->readAll();

        if (action == rowCountAct)
        {
            int count = Model->rowCount();
            TOMessageBox::information(
                this,
                tr("Row Count"),
                tr("%1 row%2.").arg(count).arg(count > 1 ? "s" : 0));
        }
    }
    else if (action == refreshAct)
        refresh();
    else if (action == exportAct)
        editSave(false);
    else if (action == copyFormatAct)
    {
        toResultListFormat exp(this, toResultListFormat::TypeCopy);
        if (!exp.exec())
            return;
        QString t = exportAsText(exp.exportSettings());
        QClipboard *clip = qApp->clipboard();
        clip->setText(t);
    }
}


void toResultTableView::slotHandleDone(void)
{
    readAllAct->setEnabled(false);

    applyFilter();
    Ready = true;
    Finished = true;
    Working->hide();
    emit done();
}


void toResultTableView::slotHandleReset(void)
{
    if (ReadAll)
        Model->readAll();
}


void toResultTableView::slotHandleFirst(const toConnection::exception &res,
                                        bool error)
{
    slotApplyColumnRules();
    Ready = true;
    Working->hide();
    emit firstResult(sql(), res, error);
}


void toResultTableView::slotHandleDoubleClick(const QModelIndex &index)
{
#if TORA3_MEMOEDITOR
    toModelEditor *ed = new toModelEditor(this, model(), index);
    ed->exec();
#endif
}


void toResultTableView::selectionChanged(const QItemSelection &selected,
        const QItemSelection &deselected)
{
    QTableView::selectionChanged(selected, deselected);
    emit selectionChanged();
}


bool toResultTableView::running(void)
{
    if (!Model)
        return false;
    QModelIndex index = currentIndex();
    return Model->canFetchMore(index);
}

void toResultTableView::setFilter(toViewFilter *filter)
{
    // Do not delete the filter, it's parent widget's responsibility
    Filter = filter;
}


void toResultTableView::setModel(toResultModel *model)
{
//     if(running())
//         throw tr("Cannot change model while query is running.");
    Model = QPointer<toResultModel>(model);
    QTableView::setModel(model);
    // After data model is set we need to connect to it's signal dataChanged. This signal
    // will be emitted after sorting on column and we need to resize Row's again then
    // because height of rows do not "move" together with their rows when sorting.
    if (toConfigurationNewSingle::Instance().option(ToConfiguration::Global::MultiLineResultsBool).toBool())
        connect(model,
                SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                this,
                SLOT(resizeRowsToContents()));
    emit modelChanged(model);
}

void toResultTableView::setModel(QAbstractItemModel *model)
{
    Q_ASSERT_X(qobject_cast<toResultModel*>(model), qPrintable(__QHERE__), "Invaid QAbstractItemModel subclass");
    QTableView::setModel(model);
}

bool toResultTableView::isRowSelected(QModelIndex index)
{
    QModelIndexList sel = selectedIndexes();
    for (QModelIndexList::iterator it = sel.begin(); it != sel.end(); it++)
    {
        if ((*it).row() == index.row())
            return true;
    }

    return false;
}


QModelIndex toResultTableView::selectedIndex(int col)
{
    // should only have one anyhow. just take first.
    QModelIndexList sel = selectedIndexes();
    if (sel.size() < 1)
        return QModelIndex();
    return model()->index(sel[0].row(), col);
}


QString toResultTableView::exportAsText(toExportSettings settings)
{
    if (settings.requireSelection())
        settings.selected = selectedIndexes();

    if (settings.rowsExport == toExportSettings::RowsAll)
    {
        QProgressDialog progress("Fetching All Data...", "Abort", 0, 2, parentWidget());
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

        // prevent scrolling in table view
        this->clearSelection();
        this->setEnabled(false);

        while (!Finished)
        {
            qApp->processEvents();

            if (progress.wasCanceled())
                break;
            if (Model->canFetchMore(currentIndex()))
                Model->fetchMore(currentIndex());
            progress.setValue(progress.value() == 0 ? 1 : 0);
        }

        this->setEnabled(true);
        progress.setValue(2);
    }

    std::unique_ptr<toListViewFormatter> pFormatter(toListViewFormatterFactory::Instance().CreateObject(settings.type));
    // TODO WTF? Owner and Table are now defined in the sub-class toResultTableViewEdit
    //    settings.owner = Owner;
    //    settings.objectName = Table;
    return pFormatter->getFormattedString(settings, model());
}

// ---------------------------------------- overrides toEditWidget

bool toResultTableView::editSave(bool askfile)
{
    try
    {
        toResultListFormat exp(this, toResultListFormat::TypeExport);
        if (!exp.exec())
            return false;

        toExportSettings settings = exp.exportSettings();

        QString filename = Utils::toSaveFilename(QString::null, settings.extension, this);
        if (filename.isEmpty())
            return false;

        return Utils::toWriteFile(filename, exportAsText(settings));
    }
    TOCATCH;

    return false;
}

void toResultTableView::editCopy()
{
    QClipboard *clip = qApp->clipboard();
    QMimeData *md = new QMimeData();
    // if there's a selection, then export as text to clipboard
    QModelIndexList sel = selectedIndexes();
    if (sel.size() > 1)
    {
        toExportSettings settings = toResultListFormat::plaintextCopySettings();
        settings.selected = sel;
        md->setText(exportAsText(settings));
        md->setData("application/x-tora", QByteArray(Utils::ptr2str(this).c_str())); // store pointer to self in clipboard see tobindvar.cpp insertFromMimeData
#ifdef Q_OS_WIN32
        std::unique_ptr<toListViewFormatter> pFormatter(toListViewFormatterFactory::Instance().CreateObject(toListViewFormatterIdentifier::XLSX));
        md->setData("XML Spreadsheet", pFormatter->getFormattedString(settings, model()).toUtf8());
#endif
        clip->setMimeData(md, QClipboard::Clipboard);
    }
    else
    {
        QModelIndex index = currentIndex();
        QVariant data = model()->data(index, Qt::EditRole);
        if (data.canConvert<QString>())
            clip->setText(data.toString());
    }
}


void toResultTableView::editSelectAll()
{
    selectAll();
}


void toResultTableView::slotResizeColumnsToContents()
{
    if (!ColumnsResized)
        QTableView::resizeColumnsToContents();
}


void toResultTableView::slotColumnWasResized(int, int, int)
{
    ColumnsResized = true;
    // After resizing columns it could happen that different amount of vertical
    // space is required to display all information therefore we resize Rows.
    if (toConfigurationNewSingle::Instance().option(ToConfiguration::Global::MultiLineResultsBool).toBool())
        resizeRowsToContents();
}


void toResultTableView::refresh()
{
    // todo. save column sizes. horizontalHeader()->restoreState(st);
    // didn't work.
    toResult::refresh();
}

bool toResultTableView::queryFromCache(const QString &owner, const QString &objectType)
{
    if (Model && running())
        Model->stop();
    freeModel();
    //TODO: Pass pri keys

    toResultModel* model = new toResultModel(owner, objectType, this, ReadableColumns);
    setModel(model);

    this->sortByColumn(0, Qt::AscendingOrder);
    setSortingEnabled(true);

    // when a new model is created the column sizes are lost
    slotApplyColumnRules();
    Ready = true;
    return true;
}

// ---------------------------------------- iterator

/**
 * Create an iterator starting at 0 on a view
 *
 */
toTableViewIterator::toTableViewIterator(toResultTableView *view)
{
    _row  = 0;
    _view = view;
    updateIndex();
}


/**
 * Create a copy of other
 *
 */
toTableViewIterator::toTableViewIterator(toTableViewIterator &other)
{
    _row  = other._row;
    _view = other._view;
    updateIndex();
}


void toTableViewIterator::updateIndex(void)
{
    if (_view && _view->model() && _row < _view->model()->rowCount())
        _index = _view->model()->index(_row, 1); // skip numbercolumn
    else
        _index = QModelIndex();
}


/**
 * Get a QModelIndex for this row. May be invalid if you've gone
 * too far. Check with QModelIndex.isValid()
 *
 */
QModelIndex toTableViewIterator::operator*() const
{
    return _index;
}


/**
 * Increment by one
 *
 */
toTableViewIterator& toTableViewIterator::operator++()
{
    _row++;
    updateIndex();
    return *this;
}


/**
 * Postfix operator
 *
 */
const toTableViewIterator toTableViewIterator::operator++(int)
{
    toTableViewIterator tmp(*this);
    ++(*this);
    updateIndex();
    return tmp;
}


/**
 * Go forward by n
 *
 */
toTableViewIterator& toTableViewIterator::operator+=(int n)
{
    _row += n;
    updateIndex();
    return *this;
}


/**
 * Decrement by one
 *
 */
toTableViewIterator& toTableViewIterator::operator--()
{
    _row--;
    updateIndex();
    return *this;
}


/**
 * Postfix operator
 *
 */
const toTableViewIterator toTableViewIterator::operator--(int)
{
    toTableViewIterator tmp(*this);
    --(*this);
    updateIndex();
    return tmp;
}


/**
 * Go back by n
 *
 */
toTableViewIterator& toTableViewIterator::operator-=(int n)
{
    _row -= n;
    updateIndex();
    return *this;
}


/**
 * Make a copy of it and return a reference
 *
 */
toTableViewIterator& toTableViewIterator::operator=(const toTableViewIterator & it)
{
    _row  = it._row;
    _view = it._view;
    updateIndex();
    return *this;
}
