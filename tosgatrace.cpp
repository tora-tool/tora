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

TO_NAMESPACE;

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

#include "totool.h"
#include "tosgatrace.h"
#include "toresultview.h"
#include "toresultresources.h"
#include "toconf.h"
#include "tosql.h"
#include "tosgastatement.h"
#include "tomain.h"

#include "tosgatrace.moc"

#include "icons/refresh.xpm"
#include "icons/tosgatrace.xpm"

static QComboBox *CreateSort(QWidget *parent,const char *name=NULL);

#define CONF_AUTO_UPDATE    "AutoUpdate"
#define CONF_DEFAULT_SORT   "DefaultSort"

class toSGATracePrefs : public QFrame, public toSettingTab
{ 
  QCheckBox* AutoUpdate;
  QComboBox* DefaultSort;
  toTool *Tool;

public:
  toSGATracePrefs(toTool *tool,QWidget* parent = 0,const char* name = 0)
    : QFrame(parent,name),Tool(tool)
  {
    QGroupBox *GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setGeometry( QRect( 10, 10, 380, 380 ) ); 
    GroupBox1->setTitle( tr( "SGA Trace"  ) );
    
    AutoUpdate = new QCheckBox( GroupBox1, "AutoRefresh" );
    AutoUpdate->setGeometry( QRect( 20, 30, 340, 20 ) ); 
    AutoUpdate->setText( tr( "&Auto update"  ) );
    if (!Tool->config(CONF_AUTO_UPDATE,"Yes").isEmpty())
      AutoUpdate->setChecked(true);
    QToolTip::add(  AutoUpdate, tr( "Update automatically after change\n"
				    "on sort order or schema." ) );
  
    QLabel *label=new QLabel("&Default Sort Order",GroupBox1);
    label->setGeometry(QRect(20,70,100,20));
    QToolTip::add(label,tr("Default sort order on SGA trace."));

    DefaultSort=CreateSort(GroupBox1);
    DefaultSort->setGeometry( QRect( 130,70,220,20));
    label->setBuddy(DefaultSort);
  }
  virtual void saveSetting(void)
  {
    Tool->setConfig(CONF_DEFAULT_SORT,DefaultSort->currentText());
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

static QComboBox *CreateSort(QWidget *parent,const char *name)
{
  QComboBox *sort=new QComboBox(false,parent,name);
  sort->insertItem("SQL Text");
  sort->insertItem("First Load Time");
  sort->insertItem("Parsing Schema");
  sort->insertItem("Parse Calls");
  sort->insertItem("Executions");
  sort->insertItem("Sorts");
  sort->insertItem("Disk Reads");
  sort->insertItem("Buffer Gets");
  QString str=SGATraceTool.config(CONF_DEFAULT_SORT,"SQL Text");
  if (str=="First Load Time")
    sort->setCurrentItem(1);
  else if (str=="Parsing Schema")
    sort->setCurrentItem(2);
  else if (str=="Parse Calls")
    sort->setCurrentItem(3);
  else if (str=="Executions")
    sort->setCurrentItem(4);
  else if (str=="Sorts")
    sort->setCurrentItem(5);
  else if (str=="Disk Reads")
    sort->setCurrentItem(6);
  else if (str=="Buffer Gets")
    sort->setCurrentItem(7);
  return sort;
}

toSGATrace::toSGATrace(QWidget *main,toConnection &connection)
  : toToolWidget(main,connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);

  QToolBar *toolbar=toAllocBar(this,"SGA trace",connection.connectString());

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

  new QLabel("Sort on",toolbar);
  QComboBox *sort=CreateSort(toolbar);
  connect(sort,SIGNAL(activated(const QString &)),this,SLOT(changeSort(const QString &)));

  toolbar->addSeparator();
  new QLabel("Refresh",toolbar);
  connect(toRefreshCreate(toolbar),SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));

  toolbar->setStretchableWidget(new QLabel("",toolbar));

  QSplitter *splitter=new QSplitter(Vertical,this);

  Trace=new toResultView(false,false,connection,splitter);
  Statement=new toSGAStatement(splitter,connection);

  connect(Trace,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  CurrentSchema=connection.user().upper();
  updateSchemas();

  Timer=new QTimer(this);
  connect(Timer,SIGNAL(timeout(void)),this,SLOT(refresh(void)));
  toRefreshParse(Timer,toTool::globalConfig(CONF_REFRESH,DEFAULT_REFRESH));
}

void toSGATrace::changeSort(const QString &str)
{
  CurrentSort=str;
  if (!SGATraceTool.config(CONF_AUTO_UPDATE,"Yes").isEmpty())
    refresh();
}

void toSGATrace::changeRefresh(const QString &str)
{
  toRefreshParse(Timer,str);
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

void toSGATrace::refresh(void)
{
  updateSchemas();

  QString select=
    "SELECT a.SQL_Text \"SQL Text\","
    "       a.First_Load_Time \"First Load Time\","
    "       b.username \"Parsing Schema\","
    "       a.Parse_Calls \"Parse Calls\","
    "       a.Executions \"Executions\","
    "       a.Sorts \"Sorts\","
    "       a.Disk_Reads \"Disk Reads\","
    "       a.Buffer_Gets \"Buffer Gets\","
    "       a.Address||':'||a.Hash_Value \" \""
    "  from v$sqlarea a,"
    "       all_users b"
    " where a.parsing_user_id = b.user_id";
  if (!CurrentSchema.isEmpty())
    select.append("   and b.username = :f1<char[31]>");
  if (!CurrentSort.isEmpty()) {
    select.append(" ORDER BY \"");
    select.append(CurrentSort);
    select.append("\"");
    if (CurrentSort != "SQL Text" &&
	CurrentSort != "Parsing Schema" &&
	CurrentSort != "First Load Time")
      select.append(" DESC");
  }
  if (!CurrentSchema.isEmpty()) {
    list<QString> p;
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
    otl_stream users(1,
		     toSQL::sql(toSQL::TOSQL_USERLIST,connection()),
		     otlConnect());
    Schema->insertItem("Any");
    for(int i=0;!users.eof();i++) {
      char buffer[31];
      users>>buffer;
      Schema->insertItem(QString::fromUtf8(buffer));
      if (CurrentSchema==QString::fromUtf8(buffer))
	Schema->setCurrentItem(i+1);
    }
  } TOCATCH
}

void toSGATrace::changeItem(QListViewItem *item)
{
  if (item)
    Statement->changeAddress(item->text(8));
}
