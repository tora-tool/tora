//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
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

class toSGATracePrefs : public QFrame, public toSettingTab
{ 
  QCheckBox* AutoUpdate;
  toTool *Tool;

public:
  toSGATracePrefs(toTool *tool,QWidget* parent = 0,const char* name = 0)
    : QFrame(parent,name),toSettingTab("trace.html"),Tool(tool)
  {
    QGroupBox *GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setGeometry( QRect( 10, 10, 380, 380 ) ); 
    GroupBox1->setTitle( tr( "SGA Trace"  ) );
    
    AutoUpdate = new QCheckBox( GroupBox1, "AutoRefresh" );
    AutoUpdate->setGeometry( QRect( 20, 30, 340, 20 ) ); 
    AutoUpdate->setText( tr( "&Auto update"  ) );
    if (!Tool->config(CONF_AUTO_UPDATE,"Yes").isEmpty())
      AutoUpdate->setChecked(true);
    QToolTip::add(  AutoUpdate, tr( "Update automatically after change of schema." ) );
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
