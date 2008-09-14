/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
