
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

#include "tools/tosgastatement.h"

#include "connection/tooracleconfiguration.h"
#include "widgets/toresultitem.h"
#include "widgets/toresultresources.h"
#include "core/tosql.h"
#include "core/toconfiguration.h"
#include "core/utils.h"
#include "editor/toscintilla.h"
#include "tools/toresultplan.h"
#include "tools/toresulttableview.h"

#include "result/toresultplan.h"
#include "result/toresultsql.h"

#include "views/toplantablesview.h"

static toSQL SQLParsingSchema(
    "toSGAStatement:ParsingSchema",
    "SELECT username\n"
    "  FROM v$sql a,\n"
    "       all_users b\n"
    " WHERE b.user_id = a.parsing_schema_id\n"
    "   AND a.sql_id = :f1<char[101]> AND a.child_number = :f2<char[101]>",
    "Get the schema that parsed a statement");

static toSQL SQLcheckVSQL(
    "toSGAStatement:checkVSQL",
    "SELECT count(*) FROM V$SQL_PLAN WHERE SQL_ID = '%1'",
    "Check whether plan for the statement with specified addres exists in V$SQL_PLAN",
    "0900");

static toSQL SQLBackendSql(
    "toSGAStatement:BackendSql",
    "SELECT pg_stat_get_backend_activity ( :backend<int> )",
    "Get the backend's SQL statement.",
    "",
    "QPSQL");

static toSQL SQLPlanHistory("toSGATrace:PlanHistory",
		"select ss.snap_id, ss.instance_number node, begin_interval_time, sql_id, plan_hash_value,     \n"
		" nvl(executions_delta,0) execs,                                                               \n"
		" round((elapsed_time_delta/decode(nvl(executions_delta,0),0,1,executions_delta))/1000000, 2) avg_etime, \n"
		" round((buffer_gets_delta/decode(nvl(buffer_gets_delta,0),0,1,executions_delta)), 2) avg_lio  \n"
		" from DBA_HIST_SQLSTAT S, DBA_HIST_SNAPSHOT SS                                                \n"
		" where sql_id = :sql_id<char[100],in>                                                         \n"
		" and ss.snap_id = S.snap_id                                                                   \n"
		" and ss.instance_number = S.instance_number                                                   \n"
		" and executions_delta > 0                                                                     \n"
		" order by 1, 2, 3                                                                             ",
		"Display sql plan history");

toSGAStatement::toSGAStatement(QWidget *parent, const char* name)
    : QTabWidget(parent)
{
    if (name)
        setObjectName(name);

    SQLText = new toResultSql(this);
    SQLText->setSQLName("Global:SQLTextSQLID");
    addTab(SQLText->view(), tr("SQL"));

    if (toConnection::currentConnection(this).providerIs("Oracle"))
    {
    	Plan = new toResultPlanCursor(this);
        addTab(Plan, tr("Execution plan"));

        PlanNew = new toResultPlanNew(this, name);
        addTab(PlanNew, tr("Execution plan Text"));

        Resources = new toResultResources(this);
        addTab(Resources, tr("Information"));

        PlanHistory  = new toResultTableView(this);
        PlanHistory->setObjectName("columnView");
        //PlanHistory->setSQLName("toSGATrace:PlanHistory");
        PlanHistory->setSQL(SQLPlanHistory);
        PlanHistory->setReadAll(true);
        addTab(PlanHistory, tr("Plan History"));

        SQLTables = new toPlanTablesView(this);
        SQLTables->setObjectName("toSQLTablesView");
        addTab(SQLTables->view(), tr("Tables"));
    }
    else
        QTabWidget::tabBar()->hide();

    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(changeTab(int)));
    CurrentTab = SQLText->view();
}

void toSGAStatement::changeTab(int index)
{
    if (Address.isEmpty() || Address == QString::fromLatin1("00:0"))
        return;

    CurrentTab = QTabWidget::widget(index);
    try
    {
        if (CurrentTab == SQLText->view())
            SQLText->refreshWithParams(toQueryParams() << Address);
        else if (CurrentTab == Plan)
            Plan->queryCursorPlan(toQueryParams() << Address << ChildNumber);
        else if (CurrentTab == PlanNew)
            PlanNew->refreshWithParams(toQueryParams() << Address << ChildNumber);
        else if (CurrentTab == Resources)
            Resources->refreshWithParams(toQueryParams() << Address);
        else if (CurrentTab == PlanHistory)
            PlanHistory->refreshWithParams(toQueryParams() << Address);
        else if (CurrentTab == SQLTables->view())
            SQLTables->refreshWithParams(toQueryParams() << Address << ChildNumber);
    }
    TOCATCH;
}

void toSGAStatement::changeAddress(toQueryParams const& sqlid)
{
    Q_ASSERT_X( sqlid.size() == 2, qPrintable(__QHERE__), "Expecting sql_id + child_id");
    Address = sqlid[0];
    ChildNumber  = sqlid[1];
    changeTab(QTabWidget::indexOf(CurrentTab));
}
