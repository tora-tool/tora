
#include "tools/tosandboxtool.h"
#include "core/tochangeconnection.h"
#include "core/toconnectionregistry.h"

#include <QtGui/QLayout>
#include <QtGui/QToolBar>
#include <QtGui/QLineEdit>
#include <QtGui/QAction>

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

class toSandboxTool : public toTool
{
protected:
    virtual const char **pictureXPM(void);
public:
    toSandboxTool() : toTool(10003, "Sandbox")
    { }
    virtual const char *menuItem()
    {
        return "Simple Query";
    }
    virtual toToolWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toSandbox(/*this,*/ parent, connection);
    }
    virtual void closeWindow(toConnection &connection) {};
};

const char **toSandboxTool::pictureXPM(void)
{
    return tosimplequery_xpm;
};


static toSandboxTool SandboxTool;

toSandbox::toSandbox(/*toTool *tool,*/ QWidget *parent, toConnection &connection)
    : toToolWidget(/* *tool*/SandboxTool, "simplequery.html", parent, connection, "toSandbox")
    , m_statement(NULL)
    , m_updateAct(NULL)
    , m_tableModel(NULL)
    , m_tableView(NULL)
    , m_eventQuery(NULL)
    , m_mvc(NULL)
{
    QToolBar *toolbar = Utils::toAllocBar(this, tr("Simple Query"));
    layout()->addWidget(toolbar);

    QAction *executeAct = new QAction(QPixmap(execute_xpm), tr("Execute_ current statement"), this);
    executeAct->setShortcut(QKeySequence::Refresh);
    connect(executeAct, SIGNAL(triggered()), this, SLOT(execute(void)));

    new toChangeConnection(toolbar);

    m_statement = new QLineEdit(this);
    m_statement->setText(QString::fromAscii("select * from dba_objects"));
    layout()->addWidget(m_statement);


    m_mvc = new SandboxMVC(this);
    layout()->addWidget(m_mvc->widget());

    // QComboBox *cb = new QComboBox(this);
    // layout()->addWidget(cb);
    // cb->addItem("AAA");

    connect(m_statement, SIGNAL(returnPressed()), this, SLOT(execute()));
}

void toSandbox::execute(void)
{
    try
    {
    	QString sql = m_statement->text();
    	m_eventQuery = new toEventQuery(this
    			, toConnectionRegistrySing::Instance().currentConnection()
    			, sql
    			, toQueryParams()
    			, toEventQuery::READ_FIRST);
    	//toQList params=toParamGet::getParam(this,sql);

    	m_mvc->setQuery(m_eventQuery);

    }
    TOCATCH
}
