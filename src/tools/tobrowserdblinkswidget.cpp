
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

#include "tools/tobrowserdblinkswidget.h"
#include "core/utils.h"
#include "core/toresultitem.h"
#include "core/toresulttableview.h"
#include "core/toresultcode.h"
#include "core/toquery.h"

#include <QtGui/QMessageBox>

static toSQL SQLDBLinkInfo("toBrowser:DBLinkInformation",
                           "SELECT * FROM Sys.all_db_links a\n"
                           " WHERE Owner = :f1<char[101]>\n"
                           "   AND DB_LINK = :f2<char[101]>",
                           "Display information about database link");
static toSQL SQLDBLinkSynonyms("toBrowser:DBLinkSynonyms",
                               "SELECT * FROM Sys.all_synonyms a\n"
                               " WHERE Owner = :f1<char[101]>\n"
                               "   AND DB_LINK = :f2<char[101]>",
                               "Display foreign synonyms");



toBrowserDBLinksWidget::toBrowserDBLinksWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
    setObjectName("toBrowserDBLinksWidget");

    resultDBLink = new toResultItem(this);
    resultDBLink->setObjectName("resultDBLink");
    resultDBLink->setSQL(SQLDBLinkInfo);

    synonymsView = new toResultTableView(this);
    synonymsView->setObjectName("synonymsView");
    synonymsView->setSQL(SQLDBLinkSynonyms);

    extractDBLink = new toResultCode(this);
    extractDBLink->setObjectName("extractDBLink");

    changeConnection();
}

void toBrowserDBLinksWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();
    toConnection & c = toConnection::currentConnection(this);
    if (c.providerIs("Oracle"))
    {
        addTab(resultDBLink, "Info");
        addTab(synonymsView, "&Synonyms");
        addTab(extractDBLink, "Script");
    }
    else
    {
        resultDBLink->setVisible(false);
        synonymsView->setVisible(false);
    }
}

void toBrowserDBLinksWidget::testDBLink()
{
    if (object().isEmpty())
    {
        TOMessageBox::information(this, "Database link", "No selected database link to test.");
        return;
    }

    toQList resultset;
    try
    {
        resultset = toQuery::readQuery(toConnection::currentConnection(this)
                                       , QString("SELECT * FROM dual@%1").arg(object())
                                       , toQueryParams());
    }
    TOCATCH;

    QString status(resultset.empty() ? tr("status: FAILED") : tr("status: OK"));
    TOMessageBox::information(this, "Database link", object() + " " + status);
}
