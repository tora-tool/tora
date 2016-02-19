#pragma once

#include "widgets/totoolwidget.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"

#include "result/tomvc.h"
#include "result/totableview.h"
#include "result/tocomboview.h"
#include "result/totreeview.h"
#include "result/totablemodel.h"
#include "result/totreemodel.h"

#if 0
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
#endif

class toSgaTraceNew : public toToolWidget
{
        Q_OBJECT;

    public:

        toSgaTraceNew(QWidget *parent, toConnection &connection);
        virtual ~toSgaTraceNew() {};
        virtual void slotWindowActivated(toToolWidget*) override {};

    private slots:

    private:
};
