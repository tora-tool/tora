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

#include <QMessageBox>

#include "migratetool/sqldeveloper.h"
#include "migratetool/squirrelsql.h"
#include "toconnectionimport.h"



toConnectionImport::toConnectionImport(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);
    m_tool = toConnectionImport::None;
    toolComboBox->addItem("None", toConnectionImport::None);
    toolComboBox->addItem("Oracle SQL Developer (1.5)",
                           toConnectionImport::OracleSQLDeveloper);
    toolComboBox->addItem("SquirrelSQL (2.6.x)",
                           toConnectionImport::SquirrelSQL);

    toolComboBox_changed(0);

    availableModel = new MigrateTool::ConnectionModel();

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
        notificationLabel->setText(tr("Handle exported connections XML file."
                                      "No passwords and options are imported."));
    else if (m_tool == toConnectionImport::SquirrelSQL)
        notificationLabel->setText(tr("Handle connections from tool config directory."));
    else
        notificationLabel->setText(tr("Select one of tools available.\n"
                                      "Connections could require manual handling after import."));
}

void toConnectionImport::refreshAvailable()
{
    if (m_tool == toConnectionImport::None)
    {
        QMessageBox::information(this, "TOra",
                                  "Select Import application first, please.");
        return;
    }
    if (m_tool == toConnectionImport::OracleSQLDeveloper)
        availableModel->setupData(MigrateTool::sqlDeveloper(this));
    else if (m_tool == toConnectionImport::SquirrelSQL)
        availableModel->setupData(MigrateTool::squirrelSql(this));

    tableView->resizeColumnsToContents();
}
