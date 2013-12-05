
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

#include "core/toresulttableviewedit.h"
#include "core/toresultmodeledit.h"
#include "core/utils.h"
#include "core/toconfiguration.h"
#include "core/toconnectiontraits.h"
#include "core/toconnectionsub.h"
#include "core/toglobalevent.h"

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QScrollBar>
#include <QtGui/QItemDelegate>
#include <QtCore/QSize>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QProgressDialog>

toResultTableViewEdit::toResultTableViewEdit(QWidget *parent, const char *name)
    : toResultTableView(false, false, parent, name, true)
{

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
    viewport()->setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
}


toResultTableViewEdit::~toResultTableViewEdit()
{
}


void toResultTableViewEdit::query(const QString &SQL, toQueryParams const& params)
{
    revertChanges();

    setSqlAndParams(SQL, params);

    if (params.size() == 2)
    {
        toQueryParams::const_iterator par = params.begin();
        Owner = *par;
        par++;
        Table = *par;
        PriKeys = connection().getTraits().primaryKeys(connection(), toCache::ObjectRef(Owner, Table));
    }

    toResultTableView::query(SQL, toQueryParams());
#if 0
    emit changed(false);
#endif
    if (!Model)
        return;                 // error

    // must be reconnected after every query
    toResultModelEdit *model = editModel();

    connect(model,
            SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this,
            SLOT(handleNewRows(const QModelIndex &, int, int)));

    connect(model,
            SIGNAL(modelReset()),
            this,
            SLOT(revertChanges()));

    verticalHeader()->setVisible(true);
}

toResultModel* toResultTableViewEdit::allocModel(toEventQuery *query)
{
	toResultModelEdit *m = new toResultModelEdit(query, PriKeys, this, ReadableColumns);
    m->setOwner(Owner);
    m->setTable(Table);
	return m;
}

toResultModelEdit* toResultTableViewEdit::editModel()
{
	return dynamic_cast<toResultModelEdit *>(Model.data());
}

void toResultTableViewEdit::slotHandleDoubleClick(const QModelIndex &)
{
	// do nothing, when editable toResultTableView was clicked
}

void toResultTableViewEdit::slotApplyColumnRules()
{
	Q_ASSERT_X(!model()->headers().empty(), qPrintable(__QHERE__), "Headers were not received yet");
	// Hide all "additional" PriKeys
	for (int i = 1; i <= PriKeys.size(); i++)
	{
		hideColumn(i);
	}
	toResultTableView::slotApplyColumnRules();
}

bool toResultTableViewEdit::commitChanges(bool status)
{
    // Check to make sure some changes were actually made
	if (!Model)
		return false;

	toResultModelEdit * EditModel = editModel();
	Q_ASSERT_X(EditModel, qPrintable(__QHERE__), "Invalid instance of toResultModelEdit");
	QList<struct toResultModelEdit::ChangeSet>& Changes = EditModel->changes();

    if (Changes.empty())
    {
        if (status)
            Utils::toStatusMessage(tr("No changes made"), false, false);
        return false;
    }

    QProgressDialog progress(tr("Performing changes"),
                             tr("Cancel"),
                             0,
                             Changes.size(),
                             this);

    bool error = false;
    unsigned updated = 0, added = 0, deleted = 0;
    if(EditModel->getPriKeys().empty())
    {
    	toConnectionSubLoan conn(connection());
        // No primary keys
        for (int changeIndex = 0; changeIndex < Changes.size(); changeIndex++)
        {
            progress.setValue(changeIndex);
            qApp->processEvents();
            if (progress.wasCanceled())
                break;

            try
            {
                struct toResultModelEdit::ChangeSet &change = Changes[changeIndex];

                switch (change.kind)
                {
                case toResultModelEdit::Delete:
                    deleted += EditModel->commitDelete(conn, change);
                    break;
                case toResultModelEdit::Add:
                    added += EditModel->commitAdd(conn, change);
                    break;
                case toResultModelEdit::Update:
                    updated += EditModel->commitUpdate(conn, change);
                    break;
                default:
                    Utils::toStatusMessage(tr("Internal error."));
                    break;
                }
            }
            catch (const QString &str)
            {
                Utils::toStatusMessage(str);
                error = true;
                break;
            }
        }
    }
    else
    {
    	toConnectionSubLoan conn(connection());
		try
		{
			editModel()->commitChanges(conn, updated, added, deleted);
			if (toConfigurationSingle::Instance().autoCommit())
				conn->commit();
			else
			{
				///throw QString("Not implemented yet. %1").arg(__QHERE__);
				///toGlobalEventSingle::Instance().setNeedCommit(conn);
			}
		}
		catch(...)
		{
			conn->rollback();
			throw;
		}
    }

    Utils::toStatusMessage(tr("Saved %1 changes(updated %2, added %3, deleted %4)")
                           .arg(Changes.size(), 0, 10)
                           .arg(updated, 0, 10)
                           .arg(added, 0, 10)
                           .arg(deleted, 0, 10)
                           , false, false);
    if (!error)
        Changes.clear();
#if 0
    emit changed(changed());
#endif
    return !error;
}


void toResultTableViewEdit::commitChanges(toConnection &conn)
{
    // make sure this is the same connection, we don't want to commit
    // this connection when somebody clicked 'commit' when in another
    // tool and another database.
    if (&conn != &connection())
        return;

    try {
    	bool success = commitChanges(false);
    	connection().commit();  // make sure to commit connection
    	// where our changes are.
    	// some cleanup work
    	if (success)
    	{
    		Q_ASSERT_X(editModel(), qPrintable(__QHERE__), "commitChanges into NULL Model");
    		editModel()->clearStatus();
    	}
    }
    TOCATCH
}

void toResultTableViewEdit::rollbackChanges(toConnection &conn)
{
    // make sure this is the same connection, we don't want to commit
    // this connection when somebody clicked 'commit' when in another
    // tool and another database.
    if (&conn != &connection())
        return;

    refresh();
}

void toResultTableViewEdit::revertChanges()
{
	if (editModel())
		editModel()->revertChanges();
}

void toResultTableViewEdit::handleNewRows(const QModelIndex &parent,
        int start,
        int end)
{
    // mrj: this was a work around for a scrolling bug. Don't think we
    // need this anymore.

    // int col = selectionModel()->currentIndex().column();
    // if (col < 1)
    //     col = 1;
    // QModelIndex index = Model->index(start - 1, col);

    // selectionModel()->select(QItemSelection(index, index),
    //                          QItemSelectionModel::ClearAndSelect);
    // setCurrentIndex(index);
}


void toResultTableViewEdit::addRecord(void)
{
	Q_ASSERT_X(editModel(), qPrintable(__QHERE__), "addRecord into NULL Model");
	editModel()->addRow(selectionModel()->currentIndex(), false);
}


void toResultTableViewEdit::duplicateRecord(void)
{
	Q_ASSERT_X(editModel(), qPrintable(__QHERE__), "duplicateRecord into NULL Model");
	editModel()->addRow(selectionModel()->currentIndex(), true);
}


void toResultTableViewEdit::deleteRecord(void)
{
    if (!selectionModel())
        return;

    QModelIndex ind = selectionModel()->currentIndex();
    if (ind.isValid())
    {
    	Q_ASSERT_X(editModel(), qPrintable(__QHERE__), "deleteRecord into NULL Model");
    	editModel()->deleteRow(ind);
    }
}
