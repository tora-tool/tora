
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "toresultdepend.h"
#include "toresultextract.h"
#include "toresultfield.h"
#include "toresultgrants.h"
#include "utils.h"

#include "tobrowsercodewidget.h"

// PostgreSQL does not distinguish between Head and Body for Stored SQL
// package code will be returnd for both Head and Body
static toSQL SQLSQLHeadPgSQL("toBrowser:CodeHead",
                             "SELECT p.prosrc\n"
                             "FROM pg_proc p LEFT OUTER JOIN pg_namespace n ON p.pronamespace=n.oid\n"
                             "WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                             "  AND p.proname = :f2\n",
                             "Declaration of object",
                             "7.1",
                             "PostgreSQL");
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
                             "PostgreSQL");
static toSQL SQLSQLBody("toBrowser:CodeBody",
                        "SELECT Text FROM SYS.ALL_SOURCE\n"
                        " WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]>\n"
                        "   AND Type IN ('PROCEDURE','FUNCTION','PACKAGE BODY','TYPE BODY')",
                        "");


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

    grantsView = new toResultGrants(this);
    grantsView->setObjectName("grantsView");

    dependsWidget = new toResultDepend(this, "dependsWidget");
//     dependsWidget->setObjectName("dependsWidget");

    extractView = new toResultExtract(true, this);
    extractView->setObjectName("extractView");

    changeConnection();
}

void toBrowserCodeWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();

    toConnection & c = toCurrentConnection(this);
    if (toIsOracle(c))
        addTab(declarationResult, "&Declaration");
    else
        declarationResult->hide();

    addTab(bodyResult, "B&ody");

    if (toIsOracle(c) || toIsSapDB(c))
        addTab(grantsView, "&Grants");
    else
        grantsView->hide();

    if (toIsOracle(c))
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
