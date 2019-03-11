
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

#include "widgets/totableview.h"
#include "core/toquery.h"
#include "core/tocontextmenu.h"
#include "core/tolistviewformatter.h"
#include "core/tolistviewformatteridentifier.h"

#include <QHeaderView>
#include <QContextMenuEvent>

using namespace Views;

toTableView::toTableView(QWidget *parent)
    : QTableView(parent)
{
    connect(horizontalHeader(), &QHeaderView::sectionResized, this, [this](int, int, int){ columnWasResized(); });
}

/* Controls height of all table views in TOra. Will use standard Qt function to
   calculate a row height and will control that it is not larger than a predefined
   size. Note: this height is only used in QTableView when resizeRowsToContents
   is called. */
int toTableView::sizeHintForRow(int row) const
{
    int s = super::sizeHintForRow(row);
    if (s > 60) s = 60; // TODO: This should probably be moved to configuration file
    return s;
}

/* Controls width of all table views in TOra. Will use standard Qt function to
   calculate a columns width and will control that it is not larger than a predefined
   size. Note: this height is only used in QTableView when resizeColumnsToContents
   is called. Column width is also adjusted when calculating width of column headers! */
int toTableView::sizeHintForColumn(int col) const
{
    int s = super::sizeHintForColumn(col);
    if (s > 200) s = 200; // TODO: This should probably be moved to configuration file
    return s;
}

// After data model is set we need to connect to it's signal dataChanged. This signal
// will be emitted after sorting on column and we need to resize Row's again then
// because height of rows do not "move" together with their rows when sorting.
void toTableView::setModel(QAbstractItemModel *model)
{
    super::setModel(model);
    connect(model, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &TL, const QModelIndex &BR){ resizeRowsToContents(); });
}

void toTableView::contextMenuEvent(QContextMenuEvent *e)
{
    // create menu
    QMenu *popup = new QMenu(this);

    // Handle parent widget's context menu fields
    toContextMenuHandler::traverse(this, popup);

    populateContextMenu(popup);
    contextMenuIndex =  indexAt(e->pos());
    // Display and "run" the menu
    e->accept();
    popup->exec(e->globalPos());
    delete popup;
}

void toTableView::populateContextMenu(QMenu *menu)
{
    QAction *copyAct = menu->addAction(tr("&Copy"));
    copyAct->setEnabled(contextMenuIndex.isValid());
    connect(copyAct, &QAction::triggered, this, [this](bool){ editCopy(); });
}

void toTableView::editCopy()
{
    if(contextMenuIndex.isValid())
    {
        QClipboard *clip = qApp->clipboard();
        QVariant data = model()->data(contextMenuIndex, Qt::EditRole);
        clip->setText(data.toString());
    }
}

void toTableView::columnWasResized()
{
    super::resizeRowsToContents();
}

void toTableView::applyColumnRules()
{
    int VisibleColumns(0);

    // loop through columns and hide anything starting with a ' '
    for (int col = 1; col < model()->columnCount(); col++)
    {
        if (model()->headerData(
                    col,
                    Qt::Horizontal,
                    Qt::UserRole).value<toQueryAbstr::HeaderDesc>().hidden)
        {
            hideColumn(col);
        } else {
            VisibleColumns++;
            showColumn(col);
        }
    }

    //if (!m_columnsResized)
    super::resizeColumnsToContents();
}
