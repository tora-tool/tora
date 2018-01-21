
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

#include "result/toresulttabledata.h"
#include "core/utils.h"
#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "widgets/toresultmodeledit.h"
#include "core/toconnectiontraits.h"
#include "editor/toscintilla.h"
#include "ui_toresultcontentfilterui.h"

#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QtGui/QCloseEvent>

#include "result/toresultdatasingle.h"
#include "tools/toresulttableviewedit.h"
#include "icons/refresh.xpm"
#include "icons/addrecord.xpm"
#include "icons/canceledit.xpm"
#include "icons/duplicaterecord.xpm"
#include "icons/filter.xpm"
#include "icons/forward.xpm"
#include "icons/next.xpm"
#include "icons/nofilter.xpm"
#include "icons/previous.xpm"
#include "icons/rewind.xpm"
#include "icons/filesave.xpm"
#include "icons/single.xpm"
#include "icons/trash.xpm"

toResultTableData::toResultTableData(QWidget *parent, const char *name, toWFlags f)
    : QWidget(parent, f)
    , Model(NULL)
    , AllFilter(false)
{
    setupUi(this);
    Logging->setHidden(true);
    ProgressBar->setHidden(true);

    connect(Edit,
            SIGNAL(firstResult(
                       const QString &,
                       const toConnection::exception &,
                       bool)),
            this,
            SLOT(updateForm()));

    connect(Edit, SIGNAL(modelChanged(toResultModel*)), this, SLOT(setModel(toResultModel*)));

    singleRecordForm(false);

    filterAct = toolbar->addAction(
                    QIcon(QPixmap(const_cast<const char**>(filter_xpm))),
                    tr("Define filter for editor"));
    filterAct->setCheckable(true);
    connect(filterAct, SIGNAL(triggered(bool)), this, SLOT(changeFilter(bool)));

    removeAct = toolbar->addAction(
                    QIcon(QPixmap(const_cast<const char**>(nofilter_xpm))),
                    tr("Remove filters"));
    connect(removeAct, SIGNAL(triggered(bool)), this, SLOT(removeFilter(bool)));

    toolbar->addSeparator();

    saveAct = toolbar->addAction(
                  QIcon(QPixmap(const_cast<const char**>(filesave_xpm))),
                  tr("Save changes"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    saveAct->setDisabled(true);

    addAct = toolbar->addAction(
                 QIcon(QPixmap(const_cast<const char**>(addrecord_xpm))),
                 tr("Add a new record"));
    connect(addAct, SIGNAL(triggered()), Edit, SLOT(addRecord()));

    duplicateAct = toolbar->addAction(
                       QIcon(QPixmap(const_cast<const char**>(duplicaterecord_xpm))),
                       tr("Duplicate an existing record"));
    connect(duplicateAct, SIGNAL(triggered()), Edit, SLOT(duplicateRecord()));

    deleteAct = toolbar->addAction(
                    QIcon(QPixmap(const_cast<const char**>(trash_xpm))),
                    tr("Delete current record from table"));
    connect(deleteAct, SIGNAL(triggered()), Edit, SLOT(deleteRecord()));

    toolbar->addSeparator();

    refreshAct = toolbar->addAction(
                     QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                     tr("Refresh data"));
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refreshWarn()));

    toolbar->addSeparator();

    firstAct = toolbar->addAction(
                   QIcon(QPixmap(const_cast<const char**>(rewind_xpm))),
                   tr("Go to first row"));

    previousAct = toolbar->addAction(
                      QIcon(QPixmap(const_cast<const char**>(previous_xpm))),
                      tr("Go to previous row"));

    nextAct = toolbar->addAction(
                  QIcon(QPixmap(const_cast<const char**>(next_xpm))),
                  tr("Go to next row"));

    lastAct = toolbar->addAction(
                  QIcon(QPixmap(const_cast<const char**>(forward_xpm))),
                  tr("Go to last row"));

    toolbar->addSeparator();

    singleAct = toolbar->addAction(
                    QIcon(QPixmap(const_cast<const char**>(single_xpm))),
                    tr("Toggle between table or single record editing"));
    singleAct->setCheckable(true);
    connect(singleAct, SIGNAL(toggled(bool)), this, SLOT(singleRecordForm(bool)));

    connect(toolbar,
            SIGNAL(actionTriggered(QAction *)),
            this,
            SLOT(navigate(QAction *)));
}


void toResultTableData::query(const QString &, toQueryParams const& params)
{
    if (!maybeSave())
        return;

    if (params.size() == 2)
    {
        toQueryParams::const_iterator par = params.begin();
        Owner = (QString)*par;
        par++;
        Table = (QString)*par;
    }

    /* Always check, if either is empty, query will fail */
    if (Owner.isEmpty() || Table.isEmpty())
    {
        Edit->clearData();
        return;
    }

    FilterName = filterName();
    bool filter = AllFilter || Criteria.contains(FilterName);
    filterAct->setChecked(filter);

    toConnection &conn = connection();
    QList<QString> priKeys = conn.getTraits().primaryKeys(conn, toCache::ObjectRef(Owner, Table, Owner));
    SQL = "SELECT ";
    Q_FOREACH(QString c, priKeys)
    {
        SQL += c + ",";
    }
    SQL = SQL + " t.* FROM %1.%2 t ";
    SQL = SQL.arg(conn.getTraits().quote(Owner)).arg(conn.getTraits().quote(Table));

    bool where = false;
    if (filter && !Criteria[FilterName].isEmpty())
    {
        SQL += " WHERE ";
        SQL += Criteria[FilterName];
        SQL += " ";
        where = true;
    }

    if (filter && !Order[FilterName].isEmpty())
    {
        SQL += " ORDER BY ";
        SQL += Order[FilterName];
    }

    Edit->query(SQL, toQueryParams() << Owner << Table);
}

void toResultTableData::clearData()
{
    Edit->clearData();
    ProgressBar->setHidden(true);
    Logging->setHidden(true);
    Logging->clear();
}

void toResultTableData::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void toResultTableData::setModel(toResultModel *model)
{
    if (model == NULL)
        Model = NULL;
    Model = dynamic_cast<toResultModelEdit *>(model);
    Q_ASSERT_X(Model != NULL || model == NULL, qPrintable(__QHERE__), "Invalid subclass of toResultModel");
    connect(Model, SIGNAL(changed(bool)), saveAct, SLOT(setEnabled(bool)));
}

void toResultTableData::navigate(QAction *action)
{
    Edit->setFocus();
    if (!Edit->selectionModel())
        return;

    QModelIndex current = Edit->selectionModel()->currentIndex();
    int row = current.isValid() ? current.row() : 0;
    int col = current.isValid() ? current.column() : 0;

    if (col < 1)
        col = 1;                // can't select hidden first column

    if (action == firstAct)
        row = 0;
    else if (action == previousAct)
        row = (std::max)(--row, 0);
    else if (action == nextAct)
        row = (std::min)(++row, Edit->model()->rowCount() - 1);
    else if (action == lastAct)
        row = Edit->model()->rowCount() - 1;
    else
        return;                 // not a nav action

    QModelIndex left = Edit->model()->createIndex(row, col);
    Edit->selectionModel()->select(QItemSelection(left, left),
                                   QItemSelectionModel::ClearAndSelect);
    Edit->setCurrentIndex(left);

    // will update form if shown
    singleRecordForm(singleAct->isChecked());
}


QString toResultTableData::filterName()
{
    QString nam;
    if (AllFilter)
        nam = "";
    else
    {
        nam = Owner;
        nam += ".";
        nam += Table;
    }

    return nam;
}


void toResultTableData::removeFilter(bool)
{
    if (!AllFilter)
    {
        switch (TOMessageBox::information(
                    this,
                    tr("Remove filter"),
                    tr("Remove the filter for this table only or for all tables?"),
                    tr("&All"), tr("&This"), tr("Cancel"), 0))
        {
            case 0:
                Criteria.clear();
                Order.clear();
                // Intentionally no break
            case 1:
                Criteria.remove(FilterName);
                Order.remove(FilterName);
                filterAct->setChecked(false);
                break;
            case 2:
                return;
        }
    }
    else
    {
        QString nam = "";
        Criteria.remove(nam);
        Order.remove(nam);
        AllFilter = false;
        filterAct->setChecked(false);
    }

    maybeSave();
    refresh();
}


void toResultTableData::changeFilter(bool checked)
{
    QDialog dialog(this);
    Ui::toResultContentFilterUI filter;
    filter.setupUi(&dialog);

    FilterName = filterName();

    filter.AllTables->setChecked(AllFilter);
    filter.Order->setText(Order[FilterName]);
    filter.Criteria->setText(Criteria[FilterName]);

    filter.Columns->changeObject(toCache::ObjectRef(Owner, Table, Owner));

    if (dialog.exec())
    {
        AllFilter = filter.AllTables->isChecked();

        FilterName = filterName();
        Criteria[FilterName] = filter.Criteria->text();
        Order[FilterName] = filter.Order->text();

        maybeSave();
        refresh();
    }

    if (Criteria[FilterName].isEmpty() && Order[FilterName].isEmpty())
        filterAct->setChecked(false);
    else
        filterAct->setChecked(true);
}


bool toResultTableData::maybeSave(void)
{
    if (Edit->editModel() == NULL)
        return true;
    if (!Edit->editModel()->changed())
        return true;

    // grab focus so user can see file and decide to save
    setFocus(Qt::OtherFocusReason);

    int ret = TOMessageBox::information(
                  this,
                  tr("Save changes"),
                  tr("Save changes to %1.%2?").arg(Owner).arg(Table),
                  QMessageBox::Save |
                  QMessageBox::Discard |
                  QMessageBox::Cancel);

    switch (ret)
    {
        case QMessageBox::Save:
            if (commitChanges())
                emit changesSaved();
            return true;
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
            return false;
    }
    Q_ASSERT_X(false, qPrintable(__QHERE__), "Invalid Message box response");
    return true; // never reached
}

bool toResultTableData::commitChanges()
{
    // Check to make sure some changes were actually made
    Q_ASSERT_X(Model, qPrintable(__QHERE__), "toResultModelEdit == NULL");
    if (!Model)
        return false;

    QList<struct toResultModelEdit::ChangeSet>& Changes = Model->changes();

    if (Changes.empty())
    {
        Utils::toStatusMessage(tr("No changes made"), false, false);
        return false;
    }

    ProgressBar->setVisible(true);
    ProgressBar->setMaximum(Changes.size());
    Logging->setVisible(true);

    bool error = false;
    unsigned updated = 0, added = 0, deleted = 0;


    toConnectionSubLoan conn(connection());
    // No primary keys
    for (int changeIndex = 0; changeIndex < Changes.size(); changeIndex++)
    {
        ProgressBar->setValue(changeIndex);

        try
        {
            struct toResultModelEdit::ChangeSet &change = Changes[changeIndex];

            switch (change.kind)
            {
                case toResultModelEdit::Delete:
                    deleted += commitDelete(conn, change);
                    break;
                case toResultModelEdit::Add:
                    added += commitAdd(conn, change);
                    break;
                case toResultModelEdit::Update:
                    updated += commitUpdate(conn, change);
                    break;
                default:
                    Utils::toStatusMessage(tr("Internal error."));
                    break;
            }
        }
        catch (const QString &str)
        {
            conn->rollback();
            Utils::toStatusMessage(str);
            error = true;
            break;
        }
    }

    if (!error)
    {
        conn->commit();
        ProgressBar->setValue(Changes.size());
        Changes.clear();
    }

    Utils::toStatusMessage(tr("Saved %1 changes(updated %2, added %3, deleted %4)")
                           .arg(Changes.size(), 0, 10)
                           .arg(updated, 0, 10)
                           .arg(added, 0, 10)
                           .arg(deleted, 0, 10)
                           , false, false);

    return !error;
}

void toResultTableData::singleRecordForm(bool display)
{
    if (display)
    {
        if (!Edit->selectionModel()->currentIndex().isValid())
        {
            QModelIndex left = Edit->model()->createIndex(0, 0);
            Edit->selectionModel()->select(QItemSelection(left, left),
                                           QItemSelectionModel::ClearAndSelect);
            Edit->setCurrentIndex(left);
        }
        Form->changeRow(Edit->selectionModel()->currentIndex().row());
    }
    else
    {
        if (Form->currentRow() >= 0)
        {
            // set selection to same as the form's
            QModelIndex left = Edit->model()->createIndex(Form->currentRow(), 1);
            Edit->selectionModel()->select(QItemSelection(left, left),
                                           QItemSelectionModel::ClearAndSelect);
            Edit->setCurrentIndex(left);
        }
    }

    Form->setVisible(display);
    Edit->setVisible(!display);
}


void toResultTableData::updateForm()
{
    // update single edit form
    if (Model)
    {
        Form->changeSource(Model, Edit->selectionModel()->currentIndex().row());
    }
}


void toResultTableData::save()
{
    if (commitChanges())
        emit changesSaved();
}

void toResultTableData::refreshWarn()
{

    if (Edit->editModel() && Edit->editModel()->changed())
    {
        switch (TOMessageBox(
                    QMessageBox::Warning,
                    tr("Warning"),
                    tr("Refreshing from the database will lose current changes."),
                    QMessageBox::Ok | QMessageBox::Cancel,
                    this).exec())
        {
            case QMessageBox::Ok:
                break;
            case QMessageBox::Cancel:
                return;
            default:
                return;
        }
    }

    clearData();
    refresh();
}

unsigned toResultTableData::commitUpdate(toConnectionSubLoan &conn, toResultModelEdit::ChangeSet &change)
{
    static const QString UPDATE = QString("UPDATE %1.%2 SET %3 WHERE 1=1 %4");
    static const QString CONJUNCTION = QString(" AND %1 = %2");
    static const QString ASSIGNMENT = QString("%1 = %2");

    if (Model->getPriKeys().empty())
    {
        Utils::toStatusMessage(tr("This table has no known primary keys"));
        return 0;
    }

    toConnectionTraits const& connTraits = conn.ParentConnection.getTraits();
    QString sqlValuePlaceHolders, sqlCondPlaceHolders;

    // set new value in update statement
    if (change.newValue.isNull())
        sqlValuePlaceHolders = ASSIGNMENT.arg(change.columnName).arg("NULL");
    else
        sqlValuePlaceHolders = ASSIGNMENT.arg(change.columnName).arg((QString)change.newValue);

    for (int i = 1; i < Model->getPriKeys().size() + 1; i++)
    {
        sqlCondPlaceHolders += CONJUNCTION
                               .arg(connTraits.quote(Model->headerData(
                                           i,
                                           Qt::Horizontal,
                                           Qt::DisplayRole).toString()))
                               .arg(connTraits.quoteVarchar(change.row[i].editData()));
    }

    QString sql = UPDATE.arg(connTraits.quote(Owner)).arg(connTraits.quote(Table)).arg(sqlValuePlaceHolders).arg(sqlCondPlaceHolders);
    Logging->appendPlainText(sql);
    {
        toQuery q(conn, sql, toQueryParams());
        q.eof();
        if (q.rowsProcessed() > 1)
        {
            Logging->appendPlainText("Rollback;");
            conn->rollback();
            return 0;
        }
        return q.rowsProcessed();
    }
}

unsigned toResultTableData::commitAdd(toConnectionSubLoan &conn, toResultModelEdit::ChangeSet &change)
{
    static const QString INSERT = QString("INSERT INTO %1.%2 ( %3 ) VALUES( %4 ) ");

    toConnectionTraits const& connTraits = conn.ParentConnection.getTraits();
    const toResultModel::HeaderList & Headers = Model->headers();

    QString sqlColumns, sqlValuePlaceHolders;

    for (int i = 1 + Model->PriKeys.size(), col = 0; i < change.row.size(); i++, col++)
    {
        if (col > 0)
            sqlColumns += ", ";
        sqlColumns += connTraits.quote(Model->headerData(
                                           i,
                                           Qt::Horizontal,
                                           Qt::DisplayRole).toString());

        toQValue const &val = change.row[i].editData();
        if (val.isComplexType()) // If it's a complex type then it's not NULL
        {
            Utils::toStatusMessage(tr("This table contains complex/user defined columns "
                                      "and can not be edited"));
            return 0;
        }

        if (col > 0)
            sqlValuePlaceHolders += (",");

        if (val.isBinary())
        {
            if (Headers[i].datatype.toUpper().contains("LOB"))
            {
                sqlValuePlaceHolders += ("empty_blob()");
                continue;
            }

            Utils::toStatusMessage(QString("Unsupported datatype(%1)").arg(Headers[i].datatype));
            return 0;
        }

        // Everything else --> varchar
        {
            if (Headers[col].datatype.toUpper().contains("LOB"))
            {
                sqlValuePlaceHolders += ("empty_clob()");
                continue;
            }
            sqlValuePlaceHolders += connTraits.quoteVarchar(val.editData());
            continue;
        }
    }

    QString sql = INSERT.arg(connTraits.quote(Owner)).arg(connTraits.quote(Table)).arg(sqlColumns).arg(sqlValuePlaceHolders);
    Logging->appendPlainText(sql);

    {
        // TODO use event query
        toQuery q(conn, sql, toQueryParams());
        q.eof();
        return q.rowsProcessed();
    }
}

unsigned toResultTableData::commitDelete(toConnectionSubLoan &conn, toResultModelEdit::ChangeSet &change)
{
    static const QString DELETESTAT = QString::fromLatin1("DELETE FROM %1.%2 WHERE 1=1 %3");
    static const QString CONJUNCTION = QString::fromLatin1(" AND %1 = %2");
    if (Model->getPriKeys().empty())
    {
        Utils::toStatusMessage(tr("This table has no known primary keys"));
        return 0;
    }

    toConnectionTraits const& connTraits = conn.ParentConnection.getTraits();
    QString sqlValuePlaceHolders;
    for (int i = 1; i < Model->getPriKeys().size() + 1; i++)
    {
        sqlValuePlaceHolders += CONJUNCTION
                                .arg(connTraits.quote(Model->headerData(
                                            i,
                                            Qt::Horizontal,
                                            Qt::DisplayRole).toString()))
                                .arg(connTraits.quoteVarchar(change.row[i].editData()));
    }

    QString sql = DELETESTAT.arg(connTraits.quote(Owner)).arg(connTraits.quote(Table)).arg(sqlValuePlaceHolders);
    Logging->appendPlainText(sql);

    {
        toQuery q(conn, sql, toQueryParams());
        q.eof();
        if (q.rowsProcessed() > 1)
        {
            Logging->appendPlainText("Rollback;");
            conn->rollback();
            return 0;
        }
        return q.rowsProcessed();
    }
}
