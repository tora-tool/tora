
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

#include <QMessageBox>

#include "migratetool/sqldeveloper.h"
#include "migratetool/squirrelsql.h"
#include "migratetool/tora3.h"
#include "toconnectionimport.h"
#include "toconnectionmodel.h"



toConnectionImport::toConnectionImport(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);
    m_tool = toConnectionImport::None;
    toolComboBox->addItem("None", toConnectionImport::None);
    toolComboBox->addItem("TOra (1.3.x)",
                           toConnectionImport::Tora3);
    toolComboBox->addItem("Oracle SQL Developer (1.5)",
                           toConnectionImport::OracleSQLDeveloper);
    toolComboBox->addItem("SquirrelSQL (2.6.x)",
                           toConnectionImport::SquirrelSQL);

    toolComboBox_changed(0);

    availableModel = new toConnectionModel();

    tableView->setModel(availableModel);

    connect(toolComboBox, SIGNAL(currentIndexChanged(int)),
             this, SLOT(toolComboBox_changed(int)));
    connect(runButton, SIGNAL(clicked()),
             this, SLOT(refreshAvailable()));
}

void toConnectionImport::toolComboBox_changed(int ix)
{
    m_tool = (ToolUsed)toolComboBox->itemData(ix).toInt();
    if (m_tool == toConnectionImport::OracleSQLDeveloper)
        notificationLabel->setText(tr("Import connections from an XML file.\n"
                                      "No passwords or options are imported."));
    else if (m_tool == toConnectionImport::SquirrelSQL)
        notificationLabel->setText(tr("Import connections from tool config directory."));
    else if (m_tool == toConnectionImport::Tora3)
        notificationLabel->setText(tr("Import connections from ~/.torarc"));
    else
        notificationLabel->setText(tr("Select one of tools available.\n"
                                      "Connections could require manual changes after importing."));
}

void toConnectionImport::refreshAvailable()
{
    if (m_tool == toConnectionImport::None)
    {
        QMessageBox::information(this, "TOra",
                                 "Please select an import tool first.");
        return;
    }
    if (m_tool == toConnectionImport::OracleSQLDeveloper)
        availableModel->setupData(MigrateTool::sqlDeveloper(this));
    else if (m_tool == toConnectionImport::SquirrelSQL)
        availableModel->setupData(MigrateTool::squirrelSql(this));
    else if (m_tool == toConnectionImport::Tora3)
        availableModel->setupData(MigrateTool::tora3(this));

    tableView->resizeColumnsToContents();
}
