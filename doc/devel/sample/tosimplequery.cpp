
#include "tools/tosampletool.h"
#include "core/utils.h"
#include "core/totool.h"
#include "core/toresultview.h"
#include "core/tochangeconnection.h"

#include <QtGui/QLayout>
#include <QtGui/QToolBar>
#include <QtGui/QLineEdit>

static const char * execute_xpm[] =
{
    "16 16 3 1",
    " 	c None",
    ".	c #000000",
    "+	c #0FFE14",
    "                ",
    "                ",
    "                ",
    "     .          ",
    "     ..         ",
    "     .+.        ",
    "     .++.       ",
    "     .+++.      ",
    "     .+++.      ",
    "     .++.       ",
    "     .+.        ",
    "     ..         ",
    "     .          ",
    "                ",
    "                ",
    "                "
};

static const char * tosimplequery_xpm[] =
{
    "16 16 3 1",
    " 	c None",
    ".	c #000000",
    "+	c #FFFFFF",
    "       ......   ",
    "      ..++++.   ",
    "     .+.++++.   ",
    "    .++.++++.   ",
    "   .....++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   .++++++++.   ",
    "   ..........   "
};

class toSampleTool : public toTool
{
protected:
    virtual const char **pictureXPM(void);
public:
    toSampleTool() : toTool(10003, "Sample")
    { }
    virtual const char *menuItem()
    {
        return "Simple Query";
    }
    virtual toToolWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toSample(/*this,*/ parent, connection);
    }
    virtual void closeWindow(toConnection &connection) {};
};

const char **toSampleTool::pictureXPM(void)
{
    return tosimplequery_xpm;
};


static toSampleTool SampleTool;

toSample::toSample(/*toTool *tool,*/ QWidget *parent, toConnection &connection)
    : toToolWidget(/* *tool*/SampleTool, "simplequery.html", parent, connection, "toSample")
{
    QToolBar *toolbar = Utils::toAllocBar(this, tr("Simple Query"));
    layout()->addWidget(toolbar);

    QAction *executeAct = new QAction(QPixmap(execute_xpm), tr("Execute_ current statement"), this);
    executeAct->setShortcut(QKeySequence::Refresh);
    connect(executeAct, SIGNAL(triggered()), this, SLOT(refresh(void)));

    new toChangeConnection(toolbar);

    Statement = new QLineEdit(this);
    layout()->addWidget(Statement);
    Result = new toResultView(this);
    layout()->addWidget(Result);
    connect(Statement, SIGNAL(returnPressed()), this, SLOT(execute()));
}

void toSample::execute(void)
{
    try
    {
        QString sql = Statement->text();
        //toQList params=toParamGet::getParam(this,sql);
        //Result->query(sql,params);
        Result->query(sql, toQueryParams());
    }
    TOCATCH
}

