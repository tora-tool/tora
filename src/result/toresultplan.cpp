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

#include "result/toresultplan.h"
#include "core/tomainwindow.h"

static toSQL SQLDisplayCursor("toResultPlan:DisplayCursor",
                              "SELECT * FROM TABLE(DBMS_XPLAN.DISPLAY_CURSOR(:sqlid<char[40],in>, :chld<char[10],in>))",
                              "Get the contents of SQL plan from using DBMS_XPLAN.DISPLAY_CURSOR",
        "1000");

toResultPlanNew::toResultPlanNew(QWidget *parent, const char *name)
    : QWidget(parent)
{
    if (name)
        QWidget::setObjectName(name);
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    QWidget::setLayout(vbox);

    explainFormat = new toXPlanFormatButton(this);
    explainFormat->setToolTip(name);

//    toResultPlanFormatButton *b = new toResultPlanFormatButton(this);
//    QWidget::layout()->addWidget(b);

    mvca = new ResultPlan::MVCA(this);
    mvca->setSQLName("toResultPlan:DisplayCursor");
    QWidget::layout()->addWidget(mvca->widget());
}

toResultPlanNew::~toResultPlanNew()
{
    toMainWindow::lookup()->statusBar()->removeWidget(explainFormat);
}

void
toResultPlanNew::refreshWithParams (const toQueryParams& params)
{
    mvca->refreshWithParams(params);
}

void toResultPlanNew::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
    QMainWindow *main = toMainWindow::lookup();
    if(main)
    {
        main->statusBar()->addWidget(explainFormat);
        //main->statusBar()->insertWidget(0, explainFormat, 0);
        //explainType->show();
    }
}

void toResultPlanNew::hideEvent(QHideEvent * event)
{
    QWidget::hideEvent(event);
    QMainWindow *main = toMainWindow::lookup();
    if(main)
    {
        main->statusBar()->removeWidget(explainFormat);
    }
}

////////////////////////////////////////////////////////////////////////////////
//                     toXPlanFormatButton
////////////////////////////////////////////////////////////////////////////////
toXPlanFormatButton::toXPlanFormatButton(QWidget *parent, const char *name)
        : toToggleButton(toResultPlanNew::staticMetaObject.enumerator(toResultPlanNew::staticMetaObject.indexOfEnumerator("XPlanFormat"))
        , parent
        , name
        )
{
    enablePopUp();
}

toXPlanFormatButton::toXPlanFormatButton()
        : toToggleButton(toResultPlanNew::staticMetaObject.enumerator(toResultPlanNew::staticMetaObject.indexOfEnumerator("XPlanFormat"))
        , NULL
        )
{
    enablePopUp();
}
