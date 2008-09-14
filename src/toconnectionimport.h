/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOCONNECTIONIMPORT_H
#define TOCONNECTIONIMPORT_H

#include "ui_toconnectionimportui.h"
#include "toconnectionmodel.h"


/*! \brief GUI to load conections from the others tools.
\author Petr Vanek <petr@scribus.info>
*/
class toConnectionImport : public QDialog, public Ui::toConnectionImport
{
    Q_OBJECT

    public:
        toConnectionImport(QWidget * parent = 0);

        //! \brief Supported loaders
        enum ToolUsed {
            None = 0,
            OracleSQLDeveloper = 1,
            SquirrelSQL = 2,
            Tora3 = 3
        };

        //! \brief Send all imported connections to the caller
        QMap<int,toConnectionOptions> availableConnections()
        {
            return availableModel->availableConnections();
        };

    private:
        toConnectionModel * availableModel;

        ToolUsed m_tool;

    private slots:
        //! Change m_tool
        void toolComboBox_changed(int);
        //! Read new connections from the outside
        void refreshAvailable();

};

#endif
