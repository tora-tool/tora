//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include <qcombobox.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qmainwindow.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qtooltip.h>
#include <qpopupmenu.h>
#include <qworkspace.h>
#include <qmenubar.h>

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

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
#include "toresultlong.h"

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
    : toTool(202,"SGA Trace")
  { }
  virtual const char *menuItem()
  { return "SGA Trace"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toSGATrace(parent,connection);
  }
  virtual QWidget *configurationTab(QWidget *parent)
  { return new toSGATracePrefs(this,parent); }
};

static toSGATraceTool SGATraceTool;

toSGATrace::toSGATrace(QWidget *main,toConnection &connection)
  : toToolWidget(SGATraceTool,"trace.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,"SGA trace",connection.description());

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Fetch statements in SGA",
		  "Fetch statements in SGA",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QLabel("Schema ",toolbar);
  Schema=new QComboBox(false,toolbar);
  connect(Schema,SIGNAL(activated(const QString &)),this,SLOT(changeSchema(const QString &)));

  toolbar->addSeparator();
  new QLabel("Refresh ",toolbar);
  connect(Refresh=toRefreshCreate(toolbar),
	  SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));

  toolbar->addSeparator();
  new QLabel("Type ",toolbar);
  Type=new QComboBox(toolbar);
  Type->insertItem("SGA");
  Type->insertItem("Long operations");

  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  QSplitter *splitter=new QSplitter(Vertical,this);

  Trace=new toResultLong(false,false,toQuery::Background,splitter);
  Trace->setSorting(0);
  Trace->setReadAll(true);
  Statement=new toSGAStatement(splitter);

  connect(Trace,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  CurrentSchema=connection.user().upper();
  updateSchemas();

  connect(timer(),SIGNAL(timeout(void)),this,SLOT(refresh(void)));

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  toRefreshParse(timer(),toTool::globalConfig(CONF_REFRESH,DEFAULT_REFRESH));

  setFocusProxy(Trace);
}

void toSGATrace::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),"&Refresh",
			   this,SLOT(refresh(void)),Key_F5);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem("Change &schema",
			   Schema,SLOT(setFocus()),ALT+Key_S);
      ToolMenu->insertItem("Change &Refresh",
			   Refresh,SLOT(setFocus(void)),
			   Key_R+ALT);
      ToolMenu->insertItem("Change t&ype",
			   Type,SLOT(setFocus()),ALT+Key_Y);
      toMainWidget()->menuBar()->insertItem("&Session",ToolMenu,-1,toToolMenuIndex());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
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

static toSQL SQLSGATrace("toSGATrace:SGATrace",
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
			 "       sys.all_users b\n"
			 " where a.parsing_user_id = b.user_id",
			 "Display the contents of the SGA stack. Must have one hidden column "
			 "with SGA address at the end and a table name 'b' with a column username.");

static toSQL SQLLongOps("toSGATrace:LongOps",
			"SELECT b.opname \"Type\",\n"
			"       a.SQL_Text \"SQL Text\",\n"
			"       b.start_time \"Start Time\",\n"
			"       b.elapsed_seconds||'/'||(b.elapsed_seconds+b.time_remaining) \"Remaining/Total Time\",\n"
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
			"       b.SQL_Address||':'||b.SQL_Hash_Value \" \"\n"
			"  from v$sqlarea a,\n"
			"       v$session_longops b\n"
			" where b.sql_address = a.address(+)\n"
			"   and b.sql_hash_value = a.hash_value(+)\n"
			"   and b.opname is not null",
			"Display the contents of long the long operations list. Must have a hidden "
			"with SGA address and at the end and a table name 'b' with a column username.");

void toSGATrace::refresh(void)
{
  updateSchemas();

  QString select;
  switch(Type->currentItem()) {
  case 0:
    select=toSQL::string(SQLSGATrace,connection());
    break;
  case 1:
    select=toSQL::string(SQLLongOps,connection());
    break;
  default:
    toStatusMessage("Unknown type of trace");
    return;
  }
  if (!CurrentSchema.isEmpty())
    select.append("\n   and b.username = :f1<char[101]>");
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
