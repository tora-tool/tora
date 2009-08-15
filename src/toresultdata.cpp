
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

#include "config.h"
#include "toresultdata.h"
#include "utils.h"
#include "toconf.h"
#include "toconfiguration.h"
#include "toresultdatasingle.h"

#include "ui_toresultcontentfilterui.h"

#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QCloseEvent>

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


toResultData::toResultData(QWidget *parent,
                           const char *name,
                           Qt::WindowFlags f)
    : QWidget(parent, f)
{
    AllFilter = false;
    Discard   = false;

    if (name)
        setObjectName(name);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    QToolBar *toolbar = toAllocBar(this, tr("Content editor"));
    vbox->addWidget(toolbar);

    Edit = new toResultTableViewEdit(false, false, this);
    vbox->addWidget(Edit);

    Form = new toResultDataSingle(this);
    vbox->addWidget(Form);

    connect(Edit,
            SIGNAL(firstResult(
                       const QString &,
                       const toConnection::exception &,
                       bool)),
            this,
            SLOT(updateForm()));

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
    connect(Edit, SIGNAL(changed(bool)), saveAct, SLOT(setEnabled(bool)));

    addAct = toolbar->addAction(
                 QIcon(QPixmap(const_cast<const char**>(addrecord_xpm))),
                 tr("Add a new record"));
    connect(addAct, SIGNAL(triggered()), this, SLOT(addRecord()));

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

    setLayout(vbox);
}


void toResultData::query(const QString &, const toQList &params)
{
    if (!maybeSave())
        return;

    Discard = false;

    if (params.size() == 2)
    {
        toQList::const_iterator par = params.begin();
        Owner = *par;
        par++;
        Table = *par;
    }

    /* Always check, if either is empty, query will fail */
    if (Owner.isEmpty() || Table.isEmpty())
    {
        /* Need to clear columns here */
        return;
    }

    FilterName = filterName();
    bool filter = AllFilter || Criteria.contains(FilterName);
    filterAct->setChecked(filter);

    toConnection &conn = connection();

    SQL = "SELECT * FROM %1.%2";
    SQL = SQL.arg(conn.quote(Owner)).arg(conn.quote(Table));

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

    Edit->query(SQL, params);
}


void toResultData::exportData(std::map<QString, QString> &data,
                              const QString &prefix)
{
    ;                           // stub
}


void toResultData::importData(std::map<QString, QString> &data,
                              const QString &prefix)
{
    ;                           // stub
}


void toResultData::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}


void toResultData::navigate(QAction *action)
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


QString toResultData::filterName()
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


void toResultData::removeFilter(bool)
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


void toResultData::changeFilter(bool checked)
{
    QDialog dialog(this);
    Ui::toResultContentFilterUI filter;
    filter.setupUi(&dialog);

    FilterName = filterName();

    filter.AllTables->setChecked(AllFilter);
    filter.Order->setText(Order[FilterName]);
    filter.Criteria->setText(Criteria[FilterName]);

    filter.Columns->changeParams(Owner, Table);

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


bool toResultData::maybeSave(void)
{
    if (!Edit->changed())
        return true;
    if (!isVisible())
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

    if (ret == QMessageBox::Save)
    {
        if (Edit->commitChanges())
            emit changesSaved();
        return true;
    }
    else if (ret == QMessageBox::Discard)
        return Discard = true;
    else
        return false;
}


void toResultData::singleRecordForm(bool display)
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
    else {
        if(Form->currentRow() >= 0) {
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


void toResultData::updateForm()
{
    // update single edit form
    if (Edit->model())
    {
        Form->changeSource(Edit->model(),
                           Edit->selectionModel()->currentIndex().row());
    }
}


void toResultData::save()
{
    if (Edit->commitChanges())
        emit changesSaved();
}


void toResultData::addRecord()
{
    Edit->addRecord();
    // don't navigate here. done by Edit.
//     navigate(lastAct);
}


void toResultData::refreshWarn()
{
    if (Edit->changed())
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
