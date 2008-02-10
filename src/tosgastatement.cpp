/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toconnection.h"
#include "toconf.h"
#include "toresultfield.h"
#include "toresultplan.h"
#include "toresultplan.h"
#include "toresultresources.h"
#include "toresultview.h"
#include "tosgastatement.h"
#include "tosgatrace.h"
#include "tosqlparse.h"
#include "totool.h"


void toSGAStatement::viewResources(void)
{
    try
    {
        Resources->changeParams(Address);
    }
    catch (...)
    {
        toStatusMessage(tr("Couldn't find SQL statement in SGA"), false, false);
    }
}

static toSQL SQLParsingSchema("toSGAStatement:ParsingSchema",
                              "SELECT username\n"
                              "  FROM v$sql a,\n"
                              "       all_users b\n"
                              " WHERE b.user_id = a.parsing_schema_id\n"
                              "   AND a.address || ':' || a.hash_value = :f1<char[101]> AND a.child_number = 0",
                              "Get the schema that parsed a statement");

toSGAStatement::toSGAStatement(QWidget *parent)
        : QTabWidget(parent)
{
    SQLText = new toResultField(this);
    addTab(SQLText, tr("SQL"));
    Plan = new toResultPlan(this);
    addTab(Plan, tr("Execution plan"));
    Resources = new toResultResources(this);
    addTab(Resources, tr("Information"));
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
        if (!Address.isEmpty())
        {
            if (CurrentTab == SQLText)
            {
                if (!toConfigurationSingle::Instance().globalConfig(CONF_AUTO_INDENT_RO, "Yes").isEmpty())
                    SQLText->setText(toSQLParse::indent(toSQLString(toCurrentConnection(this),
                                                        Address)));
                else
                    SQLText->setText(toSQLString(toCurrentConnection(this), Address));
            }
            else if (CurrentTab == Plan)
            {
                Plan->query(toSQLString(toCurrentConnection(this), Address),
                            toQuery::readQuery(toCurrentConnection(this),
                                               SQLParsingSchema, Address));
            }
            else if (CurrentTab == Resources)
                viewResources();
        }
    }
    TOCATCH
}

void toSGAStatement::changeAddress(const QString &str)
{
    Address = str;
    changeTab(QTabWidget::indexOf(CurrentTab));
}
