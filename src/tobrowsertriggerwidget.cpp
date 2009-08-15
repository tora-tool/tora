
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

#include "toresultcols.h"
#include "toresultdepend.h"
#include "toresultextract.h"
#include "toresultfield.h"
#include "toresultitem.h"
#include "toresultgrants.h"
#include "utils.h"

#include "tobrowsertriggerwidget.h"


static toSQL SQLTriggerInfo("toBrowser:TriggerInfo",
                            "SELECT Owner,Trigger_Name,\n"
                            "       Trigger_Type,Triggering_Event,\n"
                            "       Table_Owner,Base_Object_Type,Table_Name,Column_Name,\n"
                            "       Referencing_Names,When_Clause,Status,\n"
                            "       Description,Action_Type\n"
                            "  FROM SYS.ALL_TRIGGERS\n"
                            "WHERE Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
                            "Display information about a trigger",
                            "0801");
static toSQL SQLTriggerInfo8("toBrowser:TriggerInfo",
                             "SELECT Owner,Trigger_Name,\n"
                             "       Trigger_Type,Triggering_Event,\n"
                             "       Table_Owner,Table_Name,\n"
                             "       Referencing_Names,When_Clause,Status,\n"
                             "       Description\n"
                             "  FROM SYS.ALL_TRIGGERS\n"
                             "WHERE Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
                             "",
                             "0800");
static toSQL SQLTriggerBody("toBrowser:TriggerBody",
                            "SELECT Trigger_Body FROM SYS.ALL_TRIGGERS\n"
                            " WHERE Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
                            "Implementation of trigger");
static toSQL SQLTriggerCols("toBrowser:TriggerCols",
                            "SELECT Column_Name,Column_List \"In Update\",Column_Usage Usage\n"
                            "  FROM SYS.ALL_TRIGGER_COLS\n"
                            " WHERE Trigger_Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
                            "Columns used by trigger");


toBrowserTriggerWidget::toBrowserTriggerWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
    setObjectName("toBrowserTriggerWidget");

    infoView = new toResultItem(this);
    infoView->setObjectName("infoView");
    infoView->setSQL(SQLTriggerInfo);

    codeView = new toResultField(this);
    codeView->setObjectName("codeView");
    codeView->setSQL(SQLTriggerBody);

    columnsWidget = new toResultCols(this);
    columnsWidget->setObjectName("columnsWidget");
    columnsWidget->setSQL(SQLTriggerCols);

    grantsView = new toResultGrants(this);
    grantsView->setObjectName("grantsView");

    dependView = new toResultDepend(this, "dependView");
//     dependView->setObjectName("dependView");

    extractView = new toResultExtract(this);
    extractView->setObjectName("extractView");

    changeConnection();
}

void toBrowserTriggerWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();

    toConnection & c = toCurrentConnection(this);

    addTab(infoView, "Info");
    addTab(codeView, "Code");
    addTab(columnsWidget, "&Columns");
    addTab(grantsView, "&Grants");

    if (toIsOracle(c))
        addTab(dependView, "De&pendencies");
    else
        dependView->setVisible(false);

    addTab(extractView, "Script");
}
