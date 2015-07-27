#ifndef TO_SANDBOXTOOL_H
#define TO_SANDBOXTOOL_H

#include "core/totool.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"

#include "result/tomvc.h"
#include "result/totableview.h"
#include "result/tocomboview.h"
#include "result/totreeview.h"
#include "result/totablemodel.h"
#include "result/totreemodel.h"

#include <QTreeView>

class toResultView;
class toToolWidget;
class toTableModelPriv;
class QLineEdit;
class QAction;
class QWidget;
class QTableView;
class toEventQuery;

struct SandboxViewTraits : public MVCTraits
{
    enum
    {
        AlternatingRowColorsEnabled = true,
        ShowRowNumber = NoRowNumber,
        ColumnResize = RowColumResize
    };
    //typedef toComboBoxView    View;
    typedef toTreeViewPriv View;
};

class SandboxMVC
    : public TOMVC<
    SandboxViewTraits,
    //::DefaultComboBoxViewPolicy,
    ::DefaultTreeViewPolicy,
     ::DefaultDataProviderPolicy
     >
{
        Q_OBJECT;
    public:
        typedef TOMVC<
        SandboxViewTraits,
        //::DefaultComboBoxViewPolicy,
        ::DefaultTreeViewPolicy,
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



