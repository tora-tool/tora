
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
#include "core/toconf.h"
#include "core/toconfiguration.h"
#include "core/toresultdatasingle.h"
#include "core/toresulttableviewedit.h"
#include "core/toresultmodeledit.h"
#include "core/toconnectiontraits.h"
#include "ui_toresultcontentfilterui.h"

#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>

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

toResultTableData::toResultTableData(QWidget *parent, const char *name, Qt::WindowFlags f)
    : QWidget(parent, f)
    , Model(NULL)
	, AllFilter(false)
	, Discard(false)
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

    Discard = false;

    if (params.size() == 2)
    {
        toQueryParams::const_iterator par = params.begin();
        Owner = *par;
        par++;
        Table = *par;
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
    QList<QString> priKeys = conn.getTraits().primaryKeys(conn, toCache::ObjectRef(Owner, Table));
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
    int row = current.row();
    int col = current.column();

    if (col < 1)
        col = 1;                // can't select hidden first column

    if (action == firstAct)
        row = 0;
    else if (action == previousAct)
        row--;
    else if (action == nextAct)
        row++;
    else if (action == lastAct)
        row = Edit->model()->rowCount() - 1;
    else
        return;                 // not a nav action

    if (row < 0)
        row = 0;
    if (row >= Edit->model()->rowCount())
        row = Edit->model()->rowCount() - 1;

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

	filter.Columns->changeObject(toCache::ObjectRef(Owner, Table));

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

    switch(ret)
    {
    case QMessageBox::Save:
        if (Edit->commitChanges())
            emit changesSaved();
        return true;
    case QMessageBox::Discard:
    	return Discard = true;
    case QMessageBox::Cancel:
        return false;
    }
	Q_ASSERT_X(false, qPrintable(__QHERE__), "Invalid Message box response");
	return true; // never reached
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
        if(Form->currentRow() >= 0)
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
    if (Edit->model())
    {
        Form->changeSource(Edit->model(),
                           Edit->selectionModel()->currentIndex().row());
    }
}


void toResultTableData::save()
{
    if (Edit->commitChanges())
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

    Edit->refresh();
}
