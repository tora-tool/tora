#include <list>

#include <qtoolbar.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qlineedit.h>

#include "totool.h"
#include "tosimplequery.h"
#include "toresultview.h"
#include "toparamget.h"
#include "tochangeconnection.h"

#include "tosimplequery.moc"

static char * execute_xpm[] = {
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

static char * tosimplequery_xpm[] = {
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

class toSimpleQueryTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return tosimplequery_xpm; }
public:
  toSimpleQueryTool()
    : toTool(203,"Simple Query")
  { }
  virtual const char *menuItem()
  { return "Simple Query"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toSimpleQuery(parent,connection);
  }
};

static toSimpleQueryTool SimpleQueryTool;

toSimpleQuery::toSimpleQuery(QWidget *main,toConnection &connection)
  : toToolWidget(SimpleQueryTool,"simplequery.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,"Simple Query",connection.description());
  QPixmap executePixmap((const char **)execute_xpm);
  new QToolButton(executePixmap,
                  "Execute current statement",
                  "Execute current statement",
		  this,SLOT(execute()),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  Statement=new QLineEdit(this);
  Result=new toResultView(this);
  connect(Statement,SIGNAL(returnPressed()),this,SLOT(execute()));
}

void toSimpleQuery::execute(void)
{
  try {
    QString sql=Statement->text();
    toQList params=toParamGet::getParam(this,sql);
    Result->query(sql,params);
  } TOCATCH
}
