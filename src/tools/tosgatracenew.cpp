
#include "tools/tosgatracenew.h"
#include "core/totool.h"
#include "core/tochangeconnection.h"
#include "core/toconnectionregistry.h"

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

class toSgaTraceNewTool : public toTool
{
    protected:
        virtual const char **pictureXPM(void);
    public:
        toSgaTraceNewTool() : toTool(10003, "SgaTraceNew")
        { }
        virtual const char *menuItem()
        {
            return "SgaTraceNew";
        }
        virtual toToolWidget *toolWindow(QWidget *parent, toConnection &connection)
        {
            return new toSgaTraceNew(parent, connection);
        }
        virtual void closeWindow(toConnection &connection) {};
};

const char **toSgaTraceNewTool::pictureXPM(void)
{
    return tosimplequery_xpm;
};


static toSgaTraceNewTool SandboxTool;

toSgaTraceNew::toSgaTraceNew(QWidget *parent, toConnection &connection)
    : toToolWidget(SandboxTool, "simplequery.html", parent, connection, "toSandbox")
{
    QToolBar *toolbar = Utils::toAllocBar(this, tr("Simple Query"));
    layout()->addWidget(toolbar);

    QAction *executeAct = new QAction(QPixmap(execute_xpm), tr("Execute_ current statement"), this);
    executeAct->setShortcut(QKeySequence::Refresh);
    toolbar->addAction(executeAct);
    connect(executeAct, SIGNAL(triggered()), this, SLOT(execute(void)));

    new toChangeConnection(toolbar);
}
