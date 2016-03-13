
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

#include "tools/tobrowsersequencewidget.h"
#include "widgets/toresultitem.h"
#include "core/utils.h"
#include "toresultcode.h"
#include "toresultgrants.h"
#include "toresulttableview.h"

Util::RegisterInFactory<toBrowserSequenceWidget, toBrowserWidgetFactory, toCache::CacheEntryType> regToBrowserSequenceWidget(toCache::SEQUENCE);

static toSQL SQLSequenceInfoPgSQL("toBrowser:SequenceInformation",
                                  "SELECT *, substr(:f1,1) as \"Owner\" FROM :f2<noquote>",
                                  "Display information about a sequence",
                                  "0701",
                                  "QPSQL");
static toSQL SQLSequenceInfo("toBrowser:SequenceInformation",
                             "SELECT * FROM SYS.ALL_SEQUENCES\n"
                             " WHERE Sequence_Owner = :f1<char[101]>\n"
                             "   AND Sequence_Name = :f2<char[101]>",
                             "");




toBrowserSequenceWidget::toBrowserSequenceWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
    setObjectName("toBrowserSequenceWidget");

    resultInfo = new toResultItem(this);
    resultInfo->setObjectName("resultInfo");
    resultInfo->setSQL(SQLSequenceInfo);

    grantsView = new toResultGrants(this);
    grantsView->setObjectName("grantsView");

    extractView = new toResultCode(this);
    extractView->setObjectName("extractView");

    changeConnection();
}

void toBrowserSequenceWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();
    // changeConnection() clears type field which should be set so
    // that extractor knows what type of object TOra is handling.
    setType("SEQUENCE");

    toConnection & c = toConnection::currentConnection(this);

    if (c.providerIs("Oracle") || c.providerIs("QPSQL"))
        addTab(resultInfo, "Info");
    else
        resultInfo->hide();

    if (c.providerIs("Oracle"))
    {
        addTab(grantsView, "&Grants");
        addTab(extractView, "Script");
    }
    else
    {
        grantsView->hide();
        extractView->hide();
    }
}
