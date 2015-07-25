
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

#include "tools/tobrowsercodewidget.h"
#include "core/toresultdepend.h"
#include "core/toresultcode.h"
#include "core/toresultfield.h"
#include "core/toresultgrants.h"
#include "core/utils.h"

Util::RegisterInFactory<toBrowserCodeWidget, toBrowserWidgetFactory, toCache::CacheEntryType> regToBrowserCodeWidget1(toCache::FUNCTION);
Util::RegisterInFactory<toBrowserCodeWidget, toBrowserWidgetFactory, toCache::CacheEntryType> regToBrowserCodeWidget2(toCache::PROCEDURE);
Util::RegisterInFactory<toBrowserCodeWidget, toBrowserWidgetFactory, toCache::CacheEntryType> regToBrowserCodeWidget3(toCache::PACKAGE);
Util::RegisterInFactory<toBrowserCodeWidget, toBrowserWidgetFactory, toCache::CacheEntryType> regToBrowserCodeWidget4(toCache::PACKAGE_BODY);

// PostgreSQL does not distinguish between Head and Body for Stored SQL
// package code will be returnd for both Head and Body
static toSQL SQLSQLHeadPgSQL("toBrowser:CodeHead",
                             "SELECT p.prosrc\n"
                             "FROM pg_proc p LEFT OUTER JOIN pg_namespace n ON p.pronamespace=n.oid\n"
                             "WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                             "  AND p.proname = :f2\n",
                             "Declaration of object",
                             "7.1",
                             "QPSQL");
static toSQL SQLSQLHead("toBrowser:CodeHead",
                        "SELECT Text FROM SYS.ALL_SOURCE\n"
                        " WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]>\n"
                        "   AND Type IN ('PACKAGE','TYPE')",
                        "");

static toSQL SQLSQLBodyPgSQL("toBrowser:CodeBody",
                             "SELECT 'CREATE OR REPLACE FUNCTION ' || p.proname || ' ( ' ||\n"
                             "( SELECT array_to_string ( ARRAY ( SELECT t.typname\n"
                             "   FROM pg_type t\n"
                             "  WHERE t.OID = ANY ( p.proargtypes ) ),\n"
                             "   ', ' ) ) || ' ) RETURNS ' || ( SELECT t.typname\n"
                             "                                   from pg_type t\n"
                             "                                   where p.prorettype = t.oid) ||\n"
                             "   ' AS ' || quote_literal ( p.prosrc ) || ' language plpgsql;'\n"
                             "  FROM pg_proc p\n"
                             "  LEFT OUTER JOIN pg_namespace n\n"
                             "    ON p.pronamespace = n.OID\n"
                             " WHERE ( n.nspname = :f1 OR n.OID IS NULL )\n"
                             "   AND p.proname = :f2",
                             "Implementation of object",
                             "7.1",
                             "QPSQL");
static toSQL SQLSQLBody("toBrowser:CodeBody",
                        "SELECT Text FROM SYS.ALL_SOURCE\n"
                        " WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]>\n"
                        "   AND Type IN ('PROCEDURE','FUNCTION','PACKAGE BODY','TYPE BODY')",
                        "");

// Fetching from routine_definition table is not enough as it does not return information about
// routine arguments, return type etc.
/*static toSQL SQLSQLBodyMySQL("toBrowser:CodeBody",
                             "select routine_definition from information_schema.routines\n"
                             " where routine_schema = :f1<char[101]> and routine_name = :f2<char[101]>",
                             "",
                             "5.0",
                             "QMYSQL");*/

// NOTE! MySQL query has an additional argument :f1 which should be set to 'FUNCTION' or 'PROCEDURE'!
static toSQL SQLSQLBodyMySQL("toBrowser:CodeBody",
                             "show create :f3<noquote> `:f1<noquote>`.`:f2<noquote>`;",
                             "",
                             "5.0",
                             "QMYSQL");

static toSQL SQLSQLBodyTeradata("toBrowser:CodeBody",
                                "show :f3<noquote> :f1<noquote>.:f2<noquote>",
                                "",
                                "",
                                "Teradata");

toBrowserCodeWidget::toBrowserCodeWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
    setObjectName("toBrowserCodeWidget");

    declarationResult = new toResultField(this);
    declarationResult->setObjectName("declarationResult");
    declarationResult->setSQL(SQLSQLHead);

    bodyResult = new toResultField(this);
    bodyResult->setObjectName("bodyResult");
    bodyResult->setSQL(SQLSQLBody);
    toConnection & c = toConnection::currentConnection(this);
    if (c.providerIs("QMYSQL"))
        // For MySQL we need value from third column. As it is not a query which is fetching
        // routine creation script - particular field cannot be specified (or I do not know
        // how to do it). Therefore we need this workaround.
        bodyResult->setWhichResultField(3);

    grantsView = new toResultGrants(this);
    grantsView->setObjectName("grantsView");

    dependsWidget = new toResultDepend(this, "dependsWidget");
//     dependsWidget->setObjectName("dependsWidget");

    extractView = new toResultCode(true, this);
    extractView->setObjectName("extractView");

    changeConnection();
}

void toBrowserCodeWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();

    toConnection & c = toConnection::currentConnection(this);
    if (c.providerIs("Oracle"))
        addTab(declarationResult, "&Declaration");
    else
        declarationResult->hide();

    addTab(bodyResult, "B&ody");

    if (c.providerIs("Oracle") || c.providerIs("SapDB"))
        addTab(grantsView, "&Grants");
    else
        grantsView->hide();

    if (c.providerIs("Oracle"))
    {
        addTab(dependsWidget, "De&pendencies");
        addTab(extractView, "Script");
    }
    else
    {
        dependsWidget->hide();
        extractView->hide();
    }
}
