
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
#include "core/toresultfield.h"
#include "core/toresultitem.h"
#include "core/toresultplan.h"
#include "core/toresultresources.h"
//#include "core/tosqlparse.h"
#include "core/tosql.h"
#include "core/toconfiguration_new.h"
#include "core/utils.h"
#include "editor/toscintilla.h"
#include "connection/tooraclesetting.h"

void toSGAStatement::viewResources(void)
{
    try
    {
        Resources->refreshWithParams(toQueryParams() << Address);
    }
    catch (...)
    {
        Utils::toStatusMessage(tr("Couldn't find SQL statement in SGA"), false, false);
    }
}

static toSQL SQLParsingSchema(
    "toSGAStatement:ParsingSchema",
    "SELECT username\n"
    "  FROM v$sql a,\n"
    "       all_users b\n"
    " WHERE b.user_id = a.parsing_schema_id\n"
    "   AND a.address || ':' || a.hash_value = :f1<char[101]> AND a.child_number = 0",
    "Get the schema that parsed a statement");

static toSQL SQLcheckVSQL(
    "toSGAStatement:checkVSQL",
    "SELECT count(*) FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = '%1'",
    "Check whether plan for the statement with specified addres exists in V$SQL_PLAN",
    "0900");

static toSQL SQLBackendSql(
    "toSGAStatement:BackendSql",
    "SELECT pg_stat_get_backend_activity ( :backend<int> )",
    "Get the backend's SQL statement.",
    "",
    "QPSQL");

toSGAStatement::toSGAStatement(QWidget *parent)
    : QTabWidget(parent)
{

    SQLText = new toResultField(this);
    addTab(SQLText, tr("SQL"));

    if (toConnection::currentConnection(this).providerIs("Oracle"))
    {
        Plan = new toResultPlan(this);
        addTab(Plan, tr("Execution plan"));
        Resources = new toResultResources(this);
        addTab(Resources, tr("Information"));
    }
    else
        QTabWidget::tabBar()->hide();

    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(changeTab(int)));
    CurrentTab = SQLText;
}

void toSGAStatement::changeTab(int index)
{
    QWidget *widget = QTabWidget::widget(index);
    try
    {
        CurrentTab = widget;
        if (!Address.isEmpty() && Address != QString::fromLatin1("00:0"))
        {
            if (CurrentTab == SQLText)
            {
                QString sql;
                toConnection &conn = toConnection::currentConnection(this);

                if (conn.providerIs("Oracle"))
                    sql = Utils::toSQLString(conn, Address);
                else if (conn.providerIs("QPSQL"))
                {
                    toQList vals = toQuery::readQuery(conn, SQLBackendSql, toQueryParams() << Address);

                    for (toQList::iterator i = vals.begin(); i != vals.end(); i++)
                        sql.append((QString)*i);
                }

                // TODO: toSQLParse disabled
                // if (toConfigurationSingle::Instance().autoIndent())
                // sql = toSQLParse::indent(sql);
                SQLText->sciEditor()->setText(sql);
            }
            else if (CurrentTab == Plan)
            {
                toConnection &conn = toConnection::currentConnection(this);
                /*
                    Plan->query(toSQLString(toConnection::currentConnection(this), Address),
                                toQuery::readQuery(toConnection::currentConnection(this),
                                                   SQLParsingSchema, Address));
                               */
                if (////conn.version() >= "0900" &&
                    toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::VSqlPlansBool).toBool() &&
                    toQuery::readQuery(conn, toSQL::string(SQLcheckVSQL, conn).arg(Address), toQueryParams()).begin()->toInt() > 0
                )
                {
                    Plan->query(QString::fromLatin1("SGA:") + Address, toQueryParams() << QString("SGA") << Address);
                } else {
                    toQueryParams params;
                    toQList l = toQuery::readQuery(conn, SQLParsingSchema, toQueryParams() << Address);
                    if(!l.empty())
                    {
                        toQValue schema = l.front();
                        Plan->query(Utils::toSQLString(conn, Address), toQueryParams() << schema);
                    }
                }
            }
            else if (CurrentTab == Resources)
                viewResources();
        }
    }
    TOCATCH;
}

void toSGAStatement::changeAddress(const QString &str)
{
    Address = str;
    changeTab(QTabWidget::indexOf(CurrentTab));
}
