
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

#include "tests/test18window.h"

#include <QStatusBar>
#include <QScrollArea>
#include <QSplitter>
#include <QToolBar>
#include <QStandardItemModel>
#include <QTimer>

#include "core/totablemodel.h"
#include "core/totreemodel.h"

Test18Window::Test18Window()
    : Ui::Test18Window()
    , toContextMenuHandler()
{
    Ui::Test18Window::setupUi(this);

    //auto model = new QStandardItemModel(this);
//    for( int r=0; r<15; r++ )
//      for( int c=0; c<6; c++)
//      {
//        QStandardItem *item = new QStandardItem( QString("Row:%0, Column:%1").arg(r).arg(c) );
//        item->setData(QString("Custom[%0, %1]").arg(r).arg(c), Qt::UserRole);
//        model->setItem(r, c, item);
//      }

//    model = new toTableModelPriv(this);
//    for( int r=0; r<15; r++ )
//    {
//        QList<toQValue> row;
//        for( int c=0; c<6; c++)
//        {
//            row.append(toQValue::fromVariant(QVariant(QString("Custom[%0, %1]").arg(r).arg(c))));
//        }
//        model->appendRow(row);
//    }
//
    toResultXSessions *s = new toResultXSessions(this);
    model = s;

    toQueryAbstr::HeaderList headers;
    for( int c=0; c<6; c++)
    {
        toQueryAbstr::HeaderDesc h;
        h.name = QString(c);
        headers.append(h);
    }
    model->setHeaders(headers);
    int r = model->rowCount();
    int c = model->columnCount();

//    auto view = new Views::toTableView(this);
//    view->setModel(model);
//    model->index(0, 0, QModelIndex());

    //QMainWindow::setCentralWidget(view);
    QMainWindow::setCentralWidget(s->view());

    timer = new QTimer(this);
    timer->setInterval(5000);
    timer->start();
    connect(timer, &QTimer::timeout, this, &Test18Window::timerEvent);

    QMainWindow::show();
}

void Test18Window::timerEvent()
{
    toQueryAbstr::RowList rowlist;
    for( int r=0; r<15; r++ )
    {
        QList<toQValue> row;
        for( int c=0; c<6; c++)
        {
            row.append(toQValue::fromVariant(QVariant(QString("Custom[%0, %1]").arg(r + model->rowCount()).arg(c))));
        }
        rowlist.append(row);
    }
    model->appendRows(rowlist);
}

void Test18Window::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
}

void Test18Window::handle(QObject* obj, QMenu* menu)
{
    QString name = obj->objectName();

    // if (toSqlText* t = dynamic_cast<toSqlText*>(obj))
    // {
    //     Q_UNUSED(t);
    //     menu->addSeparator();
    //     menu->addAction(parseAct);
    // }
}
