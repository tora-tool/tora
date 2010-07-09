#include <list>
 
#include <qtoolbar.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qlineedit.h>
 
#include "totool.h"
#include "toawr.h"
#include "toresultview.h"
#include "toparamget.h"
#include "tochangeconnection.h"
 
//#include "tosimplequery.moc"
 
static const char * execute_xpm[] = {
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
	"                "};
 
static const char * tosimplequery_xpm[] = {
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
	"   ..........   "};
 
class toAWRTool : public toTool {
protected:
	virtual const char **pictureXPM(void);
public:
	toAWRTool()
		: toTool(10003,"AWR")
	{ }
	virtual const char *menuItem()
	{ return "Simple Query"; }
	virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
	{
		return new toAWR(/*this,*/ parent, connection);
	}
	virtual void closeWindow(toConnection &connection){};
};

const char **toAWRTool::pictureXPM(void) { return tosimplequery_xpm; };


static toAWRTool AWRTool;
 
toAWR::toAWR(/*toTool *tool,*/ QWidget *parent, toConnection &connection)
	: toToolWidget(/* *tool*/AWRTool, "simplequery.html", parent, connection, "toAWR")
{
	QToolBar *toolbar=toAllocBar(this, tr("Simple Query"));
	layout()->addWidget(toolbar);
	
	QAction *executeAct = new QAction(QPixmap(execute_xpm), tr("Execute_ current statement"), this);
        executeAct->setShortcut(QKeySequence::Refresh);
	connect(executeAct, SIGNAL(triggered()), this, SLOT(refresh(void)));

	new toChangeConnection(toolbar);
 
	Statement=new QLineEdit(this);
	layout()->addWidget(Statement);
	Result=new toResultView(this);
	layout()->addWidget(Result);
	connect(Statement, SIGNAL(returnPressed()), this, SLOT(execute()));
}
 
void toAWR::execute(void)
{
	try {
		QString sql=Statement->text();
		//toQList params=toParamGet::getParam(this,sql);
		//Result->query(sql,params);
		Result->query(sql,toQList());
	} TOCATCH
}
 
toAWR::~toAWR() {};
