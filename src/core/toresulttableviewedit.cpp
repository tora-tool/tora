
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
	toResultModelEdit *retval = new toResultModelEdit(query, PriKeys, this, ReadableColumns);
	retval->setInitialRows(visibleRows());
	return retval;
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

void toResultTableViewEdit::keyPressEvent(QKeyEvent * event)
{
    if (event->matches(QKeySequence::Delete))
    {
        deleteRecord();
        return;
    }
    toResultTableView::keyPressEvent(event);
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

	Q_ASSERT_X(editModel(), qPrintable(__QHERE__), "deleteRecord into NULL Model");

    if (selectionModel()->hasSelection())
    {
    	QSet<QModelIndex> rows;
    	QItemSelection const sel(selectionModel()->selection());
    	Q_FOREACH(QItemSelectionRange r, sel)
    	{
    		Q_FOREACH(QModelIndex i, r.indexes())
			{
    			rows << Model->createIndex(i.row(), 0);
			}
    	}
    	Q_FOREACH(QModelIndex ind, rows)
    	{
    		editModel()->deleteRow(ind);
    	}
    	return;
    }

    QModelIndex ind = selectionModel()->currentIndex();
    if (ind.isValid())
    {
    	editModel()->deleteRow(ind);
    }
}
