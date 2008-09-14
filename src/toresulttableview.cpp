
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

#include "config.h"
#include "toresulttableview.h"
#include "toresultmodel.h"
#include "toeventquery.h"
#include "toconf.h"
#include "utils.h"
#include "toconfiguration.h"
#include "toconnection.h"
#include "tomodeleditor.h"
#include "tomain.h"
#include "toresultlistformat.h"
#include "tolistviewformatter.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toworkingwidget.h"

#include <QClipboard>
#include <QScrollBar>
#include <QItemDelegate>
#include <QSize>
#include <QFont>
#include <QFontMetrics>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>


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


toResultTableView::toResultTableView(bool readable,
                                     bool numberColumn,
                                     QWidget *parent,
                                     const char *name,
                                     bool editable)
        : QTableView(parent),
        toResult(),
        toEditWidget(false,       // open
                     true,        // save
                     true,        // print
                     false,       // undo
                     false,       // redo
                     false,       // cut
                     true,        // copy
                     false,       // past
                     true,        // search
                     true,        // selectall
                     false),      // readall
        Model(NULL)
{

    if (name)
        setObjectName(name);

    Statistics      = NULL;
    Menu            = NULL;
    Editable        = editable;
    ReadAll         = false;
    Filter          = false;
    ReadableColumns = readable;
    NumberColumn    = numberColumn;
    ColumnsResized  = false;
    Ready           = false;

    Working = new toWorkingWidget(this);
    connect(Working, SIGNAL(stop()), this, SLOT(stop()));
    Working->hide(); // hide by default

    toResultTableViewDelegate *del = new toResultTableViewDelegate(this);
    setItemDelegate(del);

    createActions();

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    setAlternatingRowColors(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(displayMenu(const QPoint &)));
    connect(horizontalHeader(),
            SIGNAL(sectionResized(int, int, int)),
            this,
            SLOT(columnWasResized(int, int, int)));
    connect(this,
            SIGNAL(doubleClicked(const QModelIndex &)),
            this,
            SLOT(handleDoubleClick(const QModelIndex &)));

    setDragEnabled(true);
    if(Editable)
        viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);

    if(Editable)
        setDragDropMode(QAbstractItemView::DragDrop);

    // this is my dirty hack to prevent fetchMore being called on the
    // model during a horizontal scroll.

    // this would be one great way to fix it, but as of this time it's
    // prioritized low and not scheduled for a fix:
    // http://trolltech.no/developer/task-tracker/index_html?method=entry&id=177663
    disconnect(QTableView::horizontalScrollBar(),
               SIGNAL(valueChanged(int)),
               this,
               SLOT(horizontalScrollbarValueChanged(int)));
}


toResultTableView::~toResultTableView()
{
    if (Model)
        delete Model;
    Model = NULL;
}


void toResultTableView::query(const QString &sql, const toQList &param)
{
    setSQLParams(sql, param);

    toEventQuery *query = NULL;
    try
    {
        if (Model && running())
            Model->stop();
        if(Model)
        {
            delete Model;
            Model = NULL;
        }

        readAllAct->setEnabled(true);
        Ready = false;
        Working->setText(tr("Please wait..."));
        Working->hide();

        // sets visible true but won't show if parent is hidden
        QTimer t(this);
        t.singleShot(300, Working, SLOT(forceShow()));

        query = new toEventQuery(connection(),
                                 toQuery::Long,
                                 sql,
                                 param,
                                 Statistics);

        Model = new toResultModel(query,
                                  this,
                                  Editable,
                                  ReadableColumns);
        setModel(Model);

        connect(Model, SIGNAL(done()), this, SLOT(handleDone()));
        connect(Model, SIGNAL(modelReset()), this, SLOT(handleReset()));
        connect(Model,
                SIGNAL(firstResult(const toConnection::exception &, bool)),
                this,
                SLOT(handleFirst(const toConnection::exception &, bool)));

        setSortingEnabled(true);
    }
    catch (const toConnection::exception &str)
    {
        emit firstResult(toResult::sql(), str, true);
        emit done();
        toStatusMessage(str);
    }
    catch (const QString &str)
    {
        emit firstResult(toResult::sql(), str, true);
        emit done();
        toStatusMessage(str);
    }

    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(QFontMetrics(QFont()).height() + 4);

    horizontalHeader()->setHighlightSections(false);

    // when a new model is created the column sizes are lost
    ColumnsResized = false;
}


void toResultTableView::createActions()
{
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
    rowCountAct  = new QAction(tr("C&ount Rows"), this);
    readAllAct   = new QAction(tr("&Read All"), this);
}


void toResultTableView::applyFilter()
{
    if (!Filter)
        return;

    Filter->startingQuery();

    for (int row = 0; row < Model->rowCount(); row++)
    {
        if (!Filter->check(Model, row))
            hideRow(row);
    }
}


int toResultTableView::sizeHintForRow(int row) const
{
    return 5;
}


void toResultTableView::paintEvent(QPaintEvent *event)
{
    if(!Ready) {
        Working->setGeometry(this->viewport()->frameGeometry());
        Working->show();
        event->ignore();
    }
    else {
        Working->hide();
        QTableView::paintEvent(event);
    }
}


void toResultTableView::applyColumnRules(void)
{
    if (!NumberColumn)
        hideColumn(0);

    int visible = 0;
    if (ReadableColumns)
    {
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
                visible++;
        }
    }

    resizeColumnsToContents();

    if (visible == 1 && ReadableColumns)
        setColumnWidth(1, viewport()->width());
}


void toResultTableView::displayMenu(const QPoint &pos)
{
    if (!Menu)
    {
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
        // not implemented
        // Menu->addAction(copyTransAct);

        Menu->addSeparator();

        Menu->addAction(selectAllAct);

        Menu->addSeparator();

        Menu->addAction(exportAct);

        Menu->addSeparator();

        Menu->addAction(editAct);

        Menu->addSeparator();

        Menu->addAction(rowCountAct);
        Menu->addAction(readAllAct);

        connect(Menu,
                SIGNAL(triggered(QAction *)),
                this,
                SLOT(menuCallback(QAction *)));

        emit displayMenu(Menu);
    }

    Menu->exec(QCursor::pos());
}


void toResultTableView::menuCallback(QAction *action)
{
    QModelIndex index = currentIndex();
    if (!index.isValid())
        return;

    if (action == displayAct)
    {
        QVariant data = model()->data(index, Qt::EditRole);
        toModelEditor *ed = new toModelEditor(this, model(), index);
        ed->exec();
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
    else if (action == editAct)
        toMainWidget()->editSQL(sqlName());
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
    else if (action == copySelAct || action == copyHeadAct)
    {
        QString sep, del;
        int type = exportType(sep, del);

        if(type > -1)
        {
            QString t = exportAsText(action == copyHeadAct,
                                     true,
                                     type,         // as text
                                     sep,
                                     del);
            QClipboard *clip = qApp->clipboard();
            clip->setText(t);
        }
    }
}


void toResultTableView::handleDone(void)
{
    readAllAct->setEnabled(false);

    applyFilter();
    Ready = true;
    Working->hide();
    emit done();
}


void toResultTableView::handleReset(void)
{
    if (ReadAll)
        Model->readAll();
}


void toResultTableView::handleFirst(const toConnection::exception &res,
                                    bool error)
{
    applyColumnRules();
    Ready = true;
    Working->hide();
    emit firstResult(sql(), res, error);
}


void toResultTableView::handleDoubleClick(const QModelIndex &index)
{
    if (Editable)
        return;

    QVariant data = model()->data(index, Qt::EditRole);
    toModelEditor *ed = new toModelEditor(this, model(), index);
    ed->exec();
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


void toResultTableView::setModel(toResultModel *model)
{
//     if(running())
//         throw tr("Cannot change model while query is running.");
    Model = QPointer<toResultModel>(model);
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


int toResultTableView::exportType(QString &separator, QString &delimiter)
{
    toResultListFormat format(this, NULL);
    if (!format.exec())
        return -1;

    format.saveDefault();

    separator = format.Separator->text();
    delimiter = format.Delimiter->text();

    return format.Format->currentIndex();
}


QString toResultTableView::exportAsText(bool includeHeader,
                                        bool onlySelection,
                                        int type,
                                        QString &separator,
                                        QString &delimiter)
{
    QString result;

    if (type < 0)
        type = exportType(separator, delimiter);
    if (type < 0)
        return QString::null;

    toExportSettings settings(includeHeader,
                              onlySelection,
                              type,
                              separator,
                              delimiter);
    if(onlySelection)
        settings.selected = selectedIndexes();

    std::auto_ptr<toListViewFormatter> pFormatter(
        toListViewFormatterFactory::Instance().CreateObject(type));
    result = pFormatter->getFormattedString(settings, model());

    return result;
}


// ---------------------------------------- overrides toEditWidget

bool toResultTableView::editSave(bool askfile)
{
    try
    {
        QString delimiter;
        QString separator;
        int type = exportType(separator, delimiter);

        QString nam;
        switch (type)
        {
        case - 1:
            return false;
        default:
            nam = "*.txt";
            break;
        case 2:
            nam = "*.csv";
            break;
        case 3:
            nam = "*.html";
            break;
        case 4:
            nam = "*.sql";
            break;
        }

        QString filename = toSaveFilename(QString::null, nam, this);
        if (filename.isEmpty())
            return false;

        return toWriteFile(filename, exportAsText(true,
                           false,
                           type,
                           separator,
                           delimiter));
    }
    TOCATCH;

    return false;
}


void toResultTableView::editPrint()
{
}


void toResultTableView::editCopy()
{
    QClipboard *clip = qApp->clipboard();

    // if there's a selection, then export as text to clipboard
    QModelIndexList sel = selectedIndexes();
    if(sel.size() > 1)
    {
        QString sep, del;
        QString t = exportAsText(true,
                                 true,
                                 0,         // as text
                                 sep,
                                 del);
        clip->setText(t);
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


void toResultTableView::resizeColumnsToContents()
{
    if (!ColumnsResized)
        QTableView::resizeColumnsToContents();
}


void toResultTableView::columnWasResized(int, int, int)
{
    ColumnsResized = true;
}


void toResultTableView::refresh()
{
    // todo. save column sizes. horizontalHeader()->restoreState(st);
    // didn't work.
    toResult::refresh();
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
