
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

#ifndef TOBROWSERTABLEWIDGET_H
#define TOBROWSERTABLEWIDGET_H

#include "widgets/tobrowserbasewidget.h"

class toResultCols;
class toResultTableView;
class toResultData;
class toResultItem;
class toResultExtent;
class toResultCode;
class toResultGrants;
class toResultDrawing;
class toResultTableData;

/*! \brief Table browser for toBrowser tool.
See toBrowserBaseWidget for more info.
\author Petr Vanek <petr@scribus.info>
*/
class toBrowserTableWidget : public toBrowserBaseWidget
{
        Q_OBJECT

        toResultCols *columnsWidget;
        toResultTableView *indexView;
        toResultTableView *constraintsView;
        toResultTableView *referencesView;
#ifdef TORA_EXPORT
        toResultDrawing *schemaView;
#endif
        toResultGrants *grantsView;
        toResultTableView *triggersView;
        toResultTableData *resultData;
        toResultItem *resultInfo;
        toResultTableView *statisticsView;
        toResultTableView *partitionsView;
        toResultExtent *extentsView;
        toResultCode *extractView;

    public:
        toBrowserTableWidget(QWidget * parent);

        virtual void changeConnection();

        /*! \brief Enable or disable chosen constraints.
        Affected sub-tabs are: Constraints and Dependencies
        */
        //void enableConstraints(bool enable);
    private slots:
        void slotSelected(const QString&);
};

#endif
