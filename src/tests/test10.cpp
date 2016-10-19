
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


#include <QApplication>
#include <QSplitter>

#include <QTableView>
#include <QtGui/QStandardItemModel>

#include "widgets/totabledetailview.h"

int main(int argc, char **argv)
{
    QApplication app( argc, argv );

    QTableView *table = new QTableView();
    toTableDetailView *selectionView = new toTableDetailView();

    QSplitter splitter;
    splitter.addWidget( table );
    splitter.addWidget( selectionView );

    QStandardItemModel model( 5, 2 );
    for( int r=0; r<15; r++ )
      for( int c=0; c<6; c++)
      {
        QStandardItem *item = new QStandardItem( QString("Row:%0, Column:%1").arg(r).arg(c) );
        item->setData(QString("Custom[%0, %1]").arg(r).arg(c), Qt::UserRole);
        model.setItem(r, c, item);
      }

    table->setModel( &model );
    selectionView->setModel( &model );

    selectionView->setSelectionModel( table->selectionModel() );

    splitter.show();

    return app.exec();
}

