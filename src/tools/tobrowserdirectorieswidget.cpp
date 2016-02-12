
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

#include "tools/tobrowserdirectorieswidget.h"
#include "widgets/toresultcode.h"
#include "widgets/toresulttableview.h"
#include "core/utils.h"

static toSQL SQLDirectoriesInfo("toBrowser:DirectoriesInformation",
                                "SELECT * FROM sys.all_directories\n"
                                " WHERE (Owner = 'SYS' OR Owner = :f1<char[101]>)\n"
                                " AND directory_name = :f2<char[101]>\n"
                                " ORDER BY owner, directory_name",
                                "Display information about directories by name");



toBrowserDirectoriesWidget::toBrowserDirectoriesWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
    setObjectName("toBrowserDirectoriesWidget");

    directoriesView = new toResultTableView(this);
    directoriesView->setObjectName("directoriesView");
    directoriesView->setSQL(SQLDirectoriesInfo);

    extractDirectory = new toResultCode(this);
    extractDirectory->setObjectName("extractDirecory");

    changeConnection();
}

void toBrowserDirectoriesWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();
    toConnection & c = toConnection::currentConnection(this);
    if (c.providerIs("Oracle"))
    {
        addTab(directoriesView, "&Directories");
        addTab(extractDirectory, "Script");
    }
    else
    {
        directoriesView->setVisible(false);
    }
}
