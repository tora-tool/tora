
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

#ifndef TOSGASTATEMENT_H
#define TOSGASTATEMENT_H

#include "core/toqvalue.h"

#include <QTabWidget>

class QComboBox;
class QTabWidget;
class toResultField;
class toResultItem;
class toResultPlan;
class toResultPlanNew;
class toResultView;

/** This widget displays information about a statement in the Oracle SGA. To get an
 * address use the @ref toSQLToAddress function.
 */

class toSGAStatement : public QTabWidget
{
        Q_OBJECT
        /** The SQL run.
         */
        toResultField *SQLText;
        /** Tab widget
         */
        QWidget *CurrentTab;
        /** Resources used by the statement.
         */
        toResultItem *Resources;
        /** Address of the statement.
         */
        QString Address;
        /** Cursor
         */
        QString Cursor;
        /** Execution plan of the statement.
         */
        toResultPlan *Plan;
        toResultPlanNew *PlanNew;

        /** Display resources of statement.
         */
        void viewResources(void);
    private slots:
        /** Change the displayed tab.
         */
        void changeTab(int);
    public:
        /** Create widget.
         * @param parent Parent widget.
         */
        toSGAStatement(QWidget *parent);

        /** Display another statement.
         * @param address Address of the statement to display.
         */
        void changeAddress(toQueryParams const& sqlid);

    public slots:
        /** Update the contents of this widget.
         */
        void refresh(void)
        {
            changeTab(QTabWidget::indexOf(CurrentTab));
        }
};

#endif
