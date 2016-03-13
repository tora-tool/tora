
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

#include "tools/tobrowsersynonymwidget.h"
#include "core/utils.h"
#include "toresultcode.h"
#include "toresultgrants.h"
#include "widgets/toresultitem.h"

Util::RegisterInFactory<toBrowserSynonymWidget, toBrowserWidgetFactory, toCache::CacheEntryType> regToBrowserSynonymWidget(toCache::SYNONYM);

static toSQL SQLSynonymInfo("toBrowser:SynonymInformation",
                            "SELECT * FROM Sys.All_Synonyms a      \n"
                            " WHERE Owner = :f1<char[101]>         \n"
                            "   AND Synonym_Name = :f2<char[101]>  \n"
                            "UNION ALL                             \n"
                            "SELECT * FROM Sys.All_Synonyms a      \n"
                            " WHERE Owner = 'PUBLIC'               \n"
                            "   AND Synonym_Name = :f2<char[101]>  \n",
                            "Display information about a synonym");


toBrowserSynonymWidget::toBrowserSynonymWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
    setObjectName("toBrowserSynonymWidget");

    resultInfo = new toResultItem(this);
    resultInfo->setObjectName("resultInfo");
    resultInfo->setSQL(SQLSynonymInfo);

    grantsView = new toResultGrants(this);
    grantsView->setObjectName("grantsView");

    extractView = new toResultCode(this);
    extractView->setObjectName("extractView");

    changeConnection();
}

void toBrowserSynonymWidget::changeParams(QString const& schema, QString const& object, QString const& type)
{
    int pos = object.indexOf(".");
    QString own("PUBLIC");
    QString name(object);

    if (pos >= 0)
    {
        own = object.mid(0, pos);
        name = object.mid(pos + 1);
    }

    toBrowserBaseWidget::changeParams(own, name);
}

void toBrowserSynonymWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();
    // changeConnection() clears type field which should be set so
    // that extractor knows what type of object TOra is handling.
    setType("SYNONYM");

    toConnection & c = toConnection::currentConnection(this);

    if (c.providerIs("Oracle"))
    {
        addTab(resultInfo, "Info");
        addTab(grantsView, "&Grants");
        addTab(extractView, "Script");
    }
    else
    {
        resultInfo->setVisible(false);
        grantsView->setVisible(false);
        extractView->setVisible(false);
    }
}
