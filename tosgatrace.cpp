//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qcombobox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qmainwindow.h>
#include <qworkspace.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qtooltip.h>

#include "tochangeconnection.h"
#include "totool.h"
#include "tosgatrace.h"
#include "toresultview.h"
#include "toresultresources.h"
#include "toconf.h"
#include "tosql.h"
#include "tosgastatement.h"
#include "tomain.h"
#include "toconnection.h"

#include "tosgatrace.moc"

#include "icons/refresh.xpm"
#include "icons/tosgatrace.xpm"

#define CONF_AUTO_UPDATE    "AutoUpdate"

class toSGATracePrefs : public QGroupBox, public toSettingTab
{ 
  QCheckBox* AutoUpdate;
  toTool *Tool;

public:
  toSGATracePrefs(toTool *tool,QWidget* parent = 0,const char* name = 0)
    : QGroupBox(1,Horizontal,parent,name),toSettingTab("trace.html"),Tool(tool)
  {
    setTitle("SGA Trace");

    AutoUpdate = new QCheckBox(this,"AutoRefresh");
    AutoUpdate->setText(tr("&Auto update"));
    QToolTip::add(AutoUpdate,tr("Update automatically after change of schema."));

    if (!Tool->config(CONF_AUTO_UPDATE,"Yes").isEmpty())
      AutoUpdate->setChecked(true);
  }
  virtual void saveSetting(void)
  {
    if (AutoUpdate->isChecked())
      Tool->setConfig(CONF_AUTO_UPDATE,"Yes");
    else
      Tool->setConfig(CONF_AUTO_UPDATE,"");
  }
};

class toSGATraceTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return tosgatrace_xpm; }
public:
  toSGATraceTool()
    : toTool(102,"SGA Trace")
  { }
  virtual const char *menuItem()
  { return "SGA Trace"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    QWidget *window=new toSGATrace(parent,connection);
    window->setIcon(*toolbarImage());
    return window;
  }
  virtual QWidget *configurationTab(QWidget *parent)
  { return new toSGATracePrefs(this,parent); }
};

static toSGATraceTool SGATraceTool;

static QPixmap *toRefreshPixmap;

toSGATrace::toSGATrace(QWidget *main,toConnection &connection)
  : toToolWidget(SGATraceTool,"trace.html",main,connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);

  QToolBar *toolbar=toAllocBar(this,"SGA trace",connection.description());

  new QToolButton(*toRefreshPixmap,
		  "Fetch statements in SGA",
		  "Fetch statements in SGA",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QLabel("Schema",toolbar);
  Schema=new QComboBox(false,toolbar);
  connect(Schema,SIGNAL(activated(const QString &)),this,SLOT(changeSchema(const QString &)));

  toolbar->addSeparator();
  new QLabel("Refresh",toolbar);
  connect(toRefreshCreate(toolbar),SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));

  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  QSplitter *splitter=new QSplitter(Vertical,this);

  Trace=new toResultView(false,false,splitter);
  Trace->setSorting(0);
  Statement=new toSGAStatement(splitter);

  connect(Trace,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  CurrentSchema=connection.user().upper();
  updateSchemas();

  connect(timer(),SIGNAL(timeout(void)),this,SLOT(refresh(void)));
  toRefreshParse(timer(),toTool::globalConfig(CONF_REFRESH,DEFAULT_REFRESH));
}

void toSGATrace::changeRefresh(const QString &str)
{
  toRefreshParse(timer(),str);
}

#define LARGE_BUFFER 4096

void toSGATrace::changeSchema(const QString &str)
{
  if (str!="Any")
    CurrentSchema=str;
  else
    CurrentSchema=QString::null;
  if (!SGATraceTool.config(CONF_AUTO_UPDATE,"Yes").isEmpty())
    refresh();
}

toSQL SQLSGATrace("toSGATrace:SGATrace",
		  "SELECT a.SQL_Text \"SQL Text\",\n"
		  "       a.First_Load_Time \"First Load Time\",\n"
		  "       b.username \"Parsing Schema\",\n"
		  "       a.Parse_Calls \"Parse Calls\",\n"
		  "       a.Executions \"Executions\",\n"
		  "       a.Sorts \"Sorts\",\n"
		  "       a.Disk_Reads \"Disk Reads\",\n"
		  "       a.Buffer_Gets \"Buffer Gets\",\n"
		  "       a.Rows_Processed \"Rows\",\n"
		  "       DECODE(a.Executions,0,'N/A',ROUND(a.Sorts/a.Executions,3)) \"Sorts/Exec\",\n"
		  "       DECODE(a.Executions,0,'N/A',ROUND(a.Disk_Reads/a.Executions,3)) \"Disk/Exec\",\n"
		  "       DECODE(a.Executions,0,'N/A',ROUND(a.Buffer_Gets/a.Executions,3)) \"Buffers/Exec\",\n"
		  "       DECODE(a.Executions,0,'N/A',ROUND(a.Rows_Processed/a.Executions,3)) \"Rows/Exec\",\n"
		  "       DECODE(a.Rows_Processed,0,'N/A',ROUND(a.Sorts/a.Rows_Processed,3)) \"Sorts/Rows\",\n"
		  "       DECODE(a.Rows_Processed,0,'N/A',ROUND(a.Disk_Reads/a.Rows_Processed,3)) \"Disk/Rows\",\n"
		  "       DECODE(a.Rows_Processed,0,'N/A',ROUND(a.Buffer_Gets/a.Rows_Processed,3)) \"Buffers/Rows\",\n"
		  "       a.Address||':'||a.Hash_Value \" \"\n"
		  "  from v$sqlarea a,\n"
		  "       all_users b\n"
		  " where a.parsing_user_id = b.user_id",
		  "Display the contents of the SGA stack. Must have one hidden column "
		  "with SGA address at the end and a table name 'b' with a column username.");

void toSGATrace::refresh(void)
{
  updateSchemas();

  QString select=toSQL::string(SQLSGATrace,connection());
  if (!CurrentSchema.isEmpty())
    select.append("   and b.username = :f1<char[101]>");
  if (!CurrentSchema.isEmpty()) {
    toQList p;
    p.insert(p.end(),CurrentSchema);
    Trace->query(select,p);
  } else
    Trace->query(select);

  Statement->refresh();
}

void toSGATrace::updateSchemas(void)
{
  try {
    Schema->clear();
    toQuery users(connection(),
		  toSQL::string(toSQL::TOSQL_USERLIST,connection()));
    Schema->insertItem("Any");
    for(int i=0;!users.eof();i++) {
      QString user=users.readValue();
      Schema->insertItem(user);
      if (CurrentSchema==user)
	Schema->setCurrentItem(i+1);
    }
  } TOCATCH
}

void toSGATrace::changeItem(QListViewItem *item)
{
  if (item)
    Statement->changeAddress(item->text(Trace->columns()));
}
