
/* BEGIN_COMMON_COPYRIGHT_HEADER
*
* TOra - An Oracle Toolkit for DBA's and developers
*
* Shared/mixed copyright is held throughout files in this product
*
* Portions Copyright (C) 2000-2001 Underscore AB
* Portions Copyright (C) 2003-2005 Quest Software, Inc.
* Portions Copyright (C) 2004-2008 Numerous Other Contributors
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
#include "toresultfield.h"
#include "toresultdata.h"
#include "toresultgrants.h"
#include "toresultdepend.h"
#include "toresultextract.h"

#include "tobrowserviewwidget.h"


static toSQL SQLViewSQLPgSQL("toBrowser:ViewSQL",
                             "SELECT pg_get_viewdef(c.relname)\n"
                             "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                             " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                             "   AND c.relkind = 'v' AND c.relname = :f2",
                             "Display SQL of a specified view",
                             "7.1",
                             "PostgreSQL");
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


toBrowserViewWidget::toBrowserViewWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
//     setupUi(this);
    setObjectName("toBrowserViewWidget");

    columnsWidget = new toResultCols(this);
    addTab(columnsWidget, "&Columns");

    resultField = new toResultField(this);
    resultField->setSQL(SQLViewSQL);
    addTab(resultField, "SQL");

    resultData = new toResultData(this);
    addTab(resultData, "&Data");

    grantsView = new toResultGrants(this);
    addTab(grantsView, "&Grants");

    resultDependencies = new toResultDepend(this);
    addTab(resultDependencies, "De&pendencies");

    extractView = new toResultExtract(this);
    addTab(extractView, "Script");
}
