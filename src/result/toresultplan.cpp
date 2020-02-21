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

#ifndef TO_NO_ORACLE
#include "core/toconfiguration.h"
#include "connection/tooracleconfiguration.h"
#endif

#include <QVBoxLayout>
#include <QStatusBar>

// TODO This is done by adding '+peeked_binds' to the format argument when using display_cursor().
static toSQL SQLDisplayCursor("toResultPlan:DisplayCursor",
                              "SELECT * FROM TABLE(DBMS_XPLAN.DISPLAY_CURSOR(:sqlid<char[40],in>, :chld<char[10],in>, :format<char[20],in>))"
                              //" WHERE sys.slow_one() = 1 "
                              ,
                              "Get the contents of SQL plan from using DBMS_XPLAN.DISPLAY_CURSOR",
                              "1000");

static toSQL SQLDisplayStatement("toResultPlan:DisplayStatement",
                                 "SELECT * FROM TABLE(dbms_xplan.display(:plan_table<char[100],in>, :stat_id<char[100],in>))",
                                 "Display explain plan",
                                 "1000");


void ResultPlan::PlanTextMVC::observeDone()
{
    view()->moveCursor(QTextCursor::Start);
    view()->ensureCursorVisible();
}

toResultPlanNew::toResultPlanNew(QWidget *parent, const char *name)
    : QWidget(parent)
{
    if (name)
        QWidget::setObjectName(name);
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    QWidget::setLayout(vbox);

    mvca = new ResultPlan::PlanTextMVC(this);
    mvca->setSQLName("toResultPlan:DisplayCursor");
    QWidget::layout()->addWidget(mvca->widget());

    explainFormat = new toXPlanFormatButton(this);
    explainFormat->setToolTip(name);
}

toResultPlanNew::~toResultPlanNew()
{
    toMainWindow::lookup()->statusBar()->removeWidget(explainFormat);
}

void
toResultPlanNew::refreshWithParams (const toQueryParams& params)
{
    toQueryParams p(params);

#ifndef TO_NO_ORACLE
    p << toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::XPlanFormat).toString();
#endif

    mvca->refreshWithParams(p);
}

void toResultPlanNew::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
    QMainWindow *main = toMainWindow::lookup();
    if(main)
    {
        main->statusBar()->addWidget(explainFormat);
        explainFormat->show();
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
    : toToggleButton(ENUM_REF(toResultPlanNew, XPlanFormat), parent, name)
{
    enablePopUp();
    setEnabled(toConnection::currentConnection(this).providerIs("Oracle"));
#ifndef TO_NO_ORACLE
    setValue(toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::XPlanFormat).toString());
#endif
}

toXPlanFormatButton::toXPlanFormatButton()
    : toToggleButton(ENUM_REF(toResultPlanNew, XPlanFormat), NULL)
{
    enablePopUp();
    setEnabled(toConnection::currentConnection(this).providerIs("Oracle"));
#ifndef TO_NO_ORACLE
    setValue(toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::XPlanFormat).toString());
#endif
}

void toXPlanFormatButton::toggle()
{
    toToggleButton::toggle();
#ifndef TO_NO_ORACLE
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Oracle::XPlanFormat, text());
#endif
}

void toXPlanFormatButton::toggle(const QModelIndex &index)
{
    toToggleButton::toggle(index);
#ifndef TO_NO_ORACLE
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Oracle::XPlanFormat, text());
#endif
}
