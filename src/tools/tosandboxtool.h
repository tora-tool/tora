#ifndef TO_SANDBOXTOOL_H
#define TO_SANDBOXTOOL_H

#include "widgets/totoolwidget.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"

#include "result/tomvc.h"
#include "views/totableview.h"
#include "views/tocomboview.h"
#include "views/totreeview.h"
#include "core/totablemodel.h"
#include "core/totreemodel.h"

#include <QTreeView>

//class toResultView;
class toToolWidget;
class toTableModelPriv;
class QLineEdit;
class QAction;
class QWidget;
//class QTableView;
class toEventQuery;

struct SandboxViewTraits : public MVCTraits
{
    static const bool AlternatingRowColorsEnabled = true;
    static const int  ShowRowNumber = NoRowNumber;
    static const int  ColumnResize = RowColumResize;

    //typedef Views::toComboBoxView    View;
    typedef Views::toTreeView     View;
};

class SandboxMVC
    : public TOMVC<
    SandboxViewTraits,
    //::DefaultComboBoxViewPolicy,
    Views::DefaultTreeViewPolicy,
     ::DefaultDataProviderPolicy
     >
{
        Q_OBJECT;
    public:
        typedef TOMVC<
        SandboxViewTraits,
        //::DefaultComboBoxViewPolicy,
        Views::DefaultTreeViewPolicy,
        ::DefaultDataProviderPolicy
        > _s;
        SandboxMVC(QWidget *parent) : _s(parent)
        {};
        virtual ~SandboxMVC() {};
};

class toSandbox : public toToolWidget //, toEventQuery::Client
{
        Q_OBJECT;

    public:

        toSandbox(/*toTool *tool,*/ QWidget *parent, toConnection &connection);
        virtual ~toSandbox() {};
        virtual void slotWindowActivated(toToolWidget*) {};

    private slots:
        void execute(void);

    private:
        QLineEdit    *m_statement;
        QAction      *m_updateAct;
        toTableModelPriv *m_tableModel;
        QTableView   *m_tableView;
        toEventQuery *m_eventQuery;
        SandboxMVC   *m_mvc;
};

#endif



