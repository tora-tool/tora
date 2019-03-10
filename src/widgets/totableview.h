
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

#pragma once

#include "ts_log/ts_log_utils.h"

#include <QTableView>

namespace Views
{

class toTableView : public QTableView
{
        Q_OBJECT;
        typedef QTableView super;
    public:
        explicit toTableView(QWidget *parent = 0);

    public slots:
        virtual void slotApplyColumnRules();

        /** Controls height of all table views in TOra. Will use standart Qt function to
           calculate a row height and will control that it is not larger than a predefined
           size. Note: this height is only used in QTableView when resizeRowsToContents
           is called. */
        virtual int sizeHintForRow(int row) const;

        /* Controls width of all table views in TOra. Will use standart Qt function to
           calculate a columns width and will control that it is not larger than a predefined
           size. Note: this height is only used in QTableView when resizeColumnsToContents
           is called. Column width is also adjusted when calculating width of column headers! */
        virtual int sizeHintForColumn(int row) const;


    protected:

    private:
        bool m_columnsResized;
};

template<typename _T>
class DefaultTableViewPolicy
{
    private:
        typedef _T Traits;
        typedef typename Traits::View View;
    public:
        void setup(View* pView);
};

template<typename Traits>
void DefaultTableViewPolicy<Traits>::setup(View* pView)
{

    pView->setSelectionBehavior( (QAbstractItemView::SelectionBehavior) Traits::SelectionBehavior);
    pView->setSelectionMode( (QAbstractItemView::SelectionMode) Traits::SelectionMode);
    pView->setAlternatingRowColors( Traits::AlternatingRowColorsEnabled);
    pView->setContextMenuPolicy( (Qt::ContextMenuPolicy) Traits::ContextMenuPolicy);
    pView->setSortingEnabled(Traits::SortingEnabled);

    if ( Traits::ShowRowNumber != Traits::BuiltInRowNumber )
        pView->verticalHeader()->hide();
    pView->verticalHeader()->setDefaultSectionSize(QFontMetrics(QFont()).height() + 4);

    switch (Traits::ColumnResize)
    {
        case Traits::NoColumnResize:
            break;
        case Traits::HeaderColumnResize:
            Q_ASSERT_X(false, qPrintable(__QHERE__), "Not implemented yet");
            break;
        case Traits::RowColumResize:
            {
                bool retval = QObject::connect(pView->model(), SIGNAL(firstResultReceived()), pView, SLOT(slotApplyColumnRules()));
                Q_ASSERT_X(retval, qPrintable(__QHERE__), "Connection failed: Model -> View");
            }
            break;
        case Traits::CustomColumnResize:
            Q_ASSERT_X(false, qPrintable(__QHERE__), "Not implemented yet");
            break;
    }
}

}
