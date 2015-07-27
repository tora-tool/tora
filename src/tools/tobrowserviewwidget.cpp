
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

#include "tools/tobrowserviewwidget.h"
#include "core/toresultcols.h"
#include "core/toresultfield.h"
#include "core/toresulttableview.h"
#include "result/toresulttabledata.h"
#include "core/toresultgrants.h"
#include "core/toresultdepend.h"
#include "core/toresultcode.h"
#include "core/utils.h"

Util::RegisterInFactory<toBrowserViewWidget, toBrowserWidgetFactory, toCache::CacheEntryType> regToBrowserViewWidget(toCache::VIEW);

static toSQL SQLViewSQLPgSQL("toBrowser:ViewSQL",
                             "SELECT pg_get_viewdef(c.relname)\n"
                             "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                             " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                             "   AND c.relkind = 'v' AND c.relname = :f2",
                             "Display SQL of a specified view",
                             "7.1",
                             "QPSQL");
static toSQL SQLViewSQL("toBrowser:ViewSQL",
                        "SELECT Text SQL\n"
                        "  FROM SYS.ALL_Views\n"
                        " WHERE Owner = :f1<char[101]> AND View_Name = :f2<char[101]>",
                        "");
static toSQL SQLViewSQLSapDb("toBrowser:ViewSQL",
                             "SELECT definition \"SQL\"\n"
                             " FROM viewdefs \n"
                             " WHERE  viewname = :f2<char[101]> and owner = upper(:f1<char[101]>)\n",
                             "",
                             "",
                             "SapDB");
static toSQL SQLViewTrigger("toBrowser:ViewTrigger",
                            "SELECT Trigger_Name,Triggering_Event,Column_Name,Status,Description \n"
                            "  FROM SYS.ALL_TRIGGERS\n"
                            " WHERE Table_Owner = :f1<char[101]> AND Table_Name = :f2<char[101]>",
                            "Display list of instead-of triggers for a view");


toBrowserViewWidget::toBrowserViewWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
    setObjectName("toBrowserViewWidget");

    columnsWidget = new toResultCols(this);
    columnsWidget->setObjectName("columnsWidget");

    resultField = new toResultField(this);
    resultField->setObjectName("resultField");
    resultField->setSQL(SQLViewSQL);

    triggersView = new toResultTableView(this);
    triggersView->setObjectName("triggersView");
    triggersView->setSQL(SQLViewTrigger);
    triggersView->setReadAll(true);

    resultData = new toResultTableData(this);
    resultData->setObjectName("resultData");

    grantsView = new toResultGrants(this);
    grantsView->setObjectName("grantsView");

    resultDependencies = new toResultDepend(this, "resultDependencies");
//     resultDependencies->setObjectName("resultDependencies");

    extractView = new toResultCode(this);
    extractView->setObjectName("extractView");

    changeConnection();
}

void toBrowserViewWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();
    // changeConnection() clears type field which should be set so
    // that extractor knows what type of object TOra is handling.
    setType("VIEW");

    toConnection & c = toConnection::currentConnection(this);

    addTab(columnsWidget, "&Columns");

    if (c.providerIs("Oracle") || c.providerIs("SapDB") || c.providerIs("QPSQL"))
        addTab(resultField, "SQL");
    else
        resultField->hide();

    if (c.providerIs("Oracle"))
    {
        addTab(triggersView, "&Triggers");
    }

    addTab(resultData, "&Data");

    if (c.providerIs("Oracle"))
    {
        addTab(grantsView, "&Grants");
        addTab(resultDependencies, "De&pendencies");
        addTab(extractView, "Script");
    }
    else
    {
        grantsView->hide();
        resultDependencies->hide();
        extractView->hide();
        triggersView->hide();
    }
}
