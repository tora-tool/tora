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

#include "utils.h"

#include "toanalyze.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "toresultcombo.h"
#include "toresultlong.h"
#include "toresultplan.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"
#include "toworksheetstatistic.h"

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <qcombobox.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include "toanalyze.moc"

#include "icons/execute.xpm"
#include "icons/refresh.xpm"
#include "icons/stop.xpm"
#include "icons/toanalyze.xpm"

class toAnalyzeTool : public toTool {
  virtual char **pictureXPM(void)
  { return toanalyze_xpm; }
public:
  toAnalyzeTool()
    : toTool(320,"Statistics Manager")
  { }
  virtual const char *menuItem()
  { return "Statistics Manager"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toAnalyze(parent,connection);
  }
};

static toAnalyzeTool AnalyzeTool;

static toSQL SQLListTables("toAnalyze:ListTables",
			   "select 'TABLE' \"Type\",\n"	
			   "       owner,\n"
			   "       table_name,\n"
			   "       num_rows,\n"
			   "       blocks,\n"
			   "       empty_blocks,\n"
			   "       avg_space \"Free space/block\",\n"
			   "       chain_cnt \"Chained rows\",\n"
			   "       avg_row_len \"Average row length\",\n"
			   "       sample_size,\n"
			   "       last_analyzed\n"
			   "  from sys.all_all_tables\n"
			   " where iot_name is null",
			   "Get table statistics, first three columns and binds must be same",
			   "8.0");
static toSQL SQLListTables7("toAnalyze:ListTables",
			    "select 'TABLE' \"Type\",\n"
			    "       owner,\n"
			    "       table_name,\n"
			    "       num_rows,\n"
			    "       blocks,\n"
			    "       empty_blocks,\n"
			    "       avg_space \"Free space/block\",\n"
			    "       chain_cnt \"Chained rows\",\n"
			    "       avg_row_len \"Average row length\",\n"
			    "       sample_size,\n"
			    "       last_analyzed\n"
			    "  from sys.all_tables\n"
			    " WHERE 1 = 1",
			    QString::null,
			    "7.3");

static toSQL SQLListIndex("toAnalyze:ListIndex",
			  "SELECT 'INDEX' \"Type\",\n"
			  "       Owner,\n"
			  "       Index_Name,\n"
			  "       Num_rows,\n"
			  "       Distinct_Keys,\n"
			  "       Leaf_Blocks,\n"
			  "       Avg_Leaf_Blocks_Per_Key,\n"
			  "       Avg_Data_Blocks_Per_Key,\n"
			  "       Clustering_Factor,\n"
			  "       Sample_Size,\n"
			  "       Last_Analyzed\n"
			  "  FROM SYS.ALL_INDEXES\n"
			  " WHERE 1 = 1",
			  "List the available indexes, first three column and binds must be same");

static toSQL SQLListPlans("toAnalyze:ListPlans",
			  "SELECT DISTINCT\n"
			  "       statement_id \"Statement\",\n"
			  "       MAX(timestamp) \"Timestamp\",\n"
			  "       MAX(remarks) \"Remarks\" FROM %1\n"
			  " GROUP BY statement_id",
			  "Display available saved statements. Must have same first "
			  "column and %1");

toAnalyze::toAnalyze(QWidget *main,toConnection &connection)
  : toToolWidget(AnalyzeTool,"analyze.html",main,connection)
{
  Tabs=new QTabWidget(this);
  QVBox *box=new QVBox(Tabs);
  Tabs->addTab(box,"Analyze");

  QToolBar *toolbar=toAllocBar(box,"Statistics Manager");

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Refresh",
		  "Refresh",
		  this,SLOT(refresh()),
		  toolbar);

  toolbar->addSeparator();
  Analyzed=new QComboBox(toolbar);
  Analyzed->insertItem("All");
  Analyzed->insertItem("Not analyzed");
  Analyzed->insertItem("Analyzed");

  Schema=new toResultCombo(toolbar);
  Schema->setSelected("All");
  Schema->additionalItem("All");
  Schema->query(toSQL::sql(toSQL::TOSQL_USERLIST));

  Type=new QComboBox(toolbar);
  Type->insertItem("Tables");
  Type->insertItem("Indexes");

  toolbar->addSeparator();
  Operation=new QComboBox(toolbar);
  Operation->insertItem("Compute statistics");
  Operation->insertItem("Estimate statistics");
  Operation->insertItem("Delete statistics");
  Operation->insertItem("Validate references");
  connect(Operation,SIGNAL(activated(int)),
	  this,SLOT(changeOperation(int)));

  new QLabel(" for ",toolbar);
  For=new QComboBox(toolbar);
  For->insertItem("All");
  For->insertItem("Table");
  For->insertItem("Indexed columns");
  For->insertItem("Local indexes");
  toolbar->addSeparator();
  new QLabel("Sample ",toolbar);
  Sample=new QSpinBox(1,100,1,toolbar);
  Sample->setValue(100);
  Sample->setSuffix(" %");
  Sample->setEnabled(false);
  toolbar->addSeparator();
  new QLabel("Parallel ",toolbar);
  Parallel=new QSpinBox(1,100,1,toolbar);
  toolbar->addSeparator();
  
  new QToolButton(QPixmap((const char **)execute_xpm),
		  "Start analyzing",
		  "Start analyzing",
		  this,SLOT(execute()),
		  toolbar);

  Current=new QLabel(toolbar);
  Current->setAlignment(AlignRight|AlignVCenter|ExpandTabs);
  toolbar->setStretchableWidget(Current);

  Stop=new QToolButton(QPixmap((const char **)stop_xpm),
		       "Stop current run",
		       "Stop current run",
		       this,SLOT(stop()),
		       toolbar);
  Stop->setEnabled(false);

  Statistics=new toResultLong(true,false,toQuery::Background,box);
  Statistics->setSelectionMode(QListView::Extended);
  Statistics->setReadAll(true);

  connect(Analyzed,SIGNAL(activated(int)),this,SLOT(refresh()));
  connect(Schema,SIGNAL(activated(int)),this,SLOT(refresh()));
  connect(Type,SIGNAL(activated(int)),this,SLOT(refresh()));

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));

  box=new QVBox(Tabs);
  toolbar=toAllocBar(box,"Explain plans");

  Tabs->addTab(box,"Explain plans");
  QSplitter *splitter=new QSplitter(Horizontal,box);
  Plans=new toResultLong(false,false,toQuery::Background,splitter);
  Plans->setSelectionMode(QListView::Single);
  Plans->query(toSQL::string(SQLListPlans,
			     connection).arg(toTool::globalConfig(CONF_PLAN_TABLE,
								  DEFAULT_PLAN_TABLE)));

  connect(Plans,SIGNAL(selectionChanged()),
	  this,SLOT(selectPlan()));
  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Refresh",
		  "Refresh",
		  Plans,SLOT(refresh()),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel(toolbar));

  CurrentPlan=new toResultPlan(splitter);

  Worksheet=new toWorksheetStatistic(Tabs);
  Tabs->addTab(Worksheet,"Worksheet statistics");

  refresh();
  setFocusProxy(Tabs);
}

void toAnalyze::selectPlan(void)
{
  QListViewItem *item=Plans->selectedItem();
  if (item)
    CurrentPlan->query("SAVED:"+item->text(0));
}

void toAnalyze::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),"&Refresh",
			   this,SLOT(refresh(void)),Key_F5);

      toMainWidget()->menuBar()->insertItem("&Statistics",ToolMenu,-1,toToolMenuIndex());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

toWorksheetStatistic *toAnalyze::worksheet(void)
{
  Tabs->showPage(Worksheet);
  return Worksheet;
}

void toAnalyze::changeOperation(int op)
{
  Sample->setEnabled(op==1);
  For->setEnabled(op==0||op==1);
}

void toAnalyze::refresh(void)
{
  try {
    Statistics->setSQL(QString::null);
    toQList par;
    QString sql;
    if (Type->currentItem()==0)
      sql=toSQL::string(SQLListTables,connection());
    else
      sql=toSQL::string(SQLListIndex,connection());
    if (Schema->selected()!="All") {
      par.insert(par.end(),Schema->selected());
      sql+="\n   AND owner = :own<char[100]>";
    }
    switch (Analyzed->currentItem()) {
    default:
      break;
    case 1:
      sql+="\n  AND Last_Analyzed IS NULL";
      break;
    case 2:
      sql+="\n  AND Last_Analyzed IS NOT NULL";
      break;
    }

    Statistics->query(sql,(const toQList)par);
  } TOCATCH
}

void toAnalyze::poll(void)
{
  try {
    int running=0;
    for(std::list<toNoBlockQuery *>::iterator i=Running.begin();i!=Running.end();i++) {
      bool eof=false;
      try {
	eof=(*i)->eof();
      } catch(const QString &) {
	eof=true;
      }
      if (eof) {
	QString sql=toShift(Pending);
	if (!sql.isEmpty()) {
	  delete (*i);
	  toQList par;
	  (*i)=new toNoBlockQuery(connection(),sql,par);
	  running++;
	}
      } else
	running++;
    }
    if (!running) {
      Poll.stop();
      refresh();
      stop();
    } else
      Current->setText("Running "+QString::number(running)+
		       " Pending "+QString::number(Pending.size()));
  } TOCATCH
}

void toAnalyze::execute(void)
{
  stop();

  for(QListViewItem *item=Statistics->firstChild();item;item=item->nextSibling()) {
    if (item->isSelected()) {
      QString sql="ANALYZE %3 %1.%2 ";
      QString forc;
      if (item->text(0)=="TABLE") {
	switch(For->currentItem()) {
	case 0:
	  forc="";
	  break;
	case 1:
	  forc=" FOR TABLE";
	  break;
	case 2:
	  forc=" FOR ALL INDEXED COLUMNS";
	  break;
	case 3:
	  forc=" FOR ALL LOCAL INDEXES";
	  break;
	}
      }

      switch(Operation->currentItem()) {
      case 0:
	sql+="COMPUTE STATISTICS";
	sql+=forc;
	break;
      case 1:
	sql+="ESTIMATE STATISTICS";
	sql+=forc;
	sql+=" SAMPLE "+QString::number(Sample->value())+" PERCENT";
	break;
      case 2:
	sql+="DELETE STATISTICS";
	break;
      case 3:
	sql+="VALIDATE REF UPDATE";
	break;
      }

      toPush(Pending,sql.arg(item->text(1)).arg(item->text(2)).arg(item->text(0)));
    }
  }

  try {
    toQList par;
    for(int i=0;i<Parallel->value();i++) {
      QString sql=toShift(Pending);
      if (!sql.isEmpty())
	toPush(Running,new toNoBlockQuery(connection(),sql,par));
    }
    Poll.start(100);
    Stop->setEnabled(true);
    poll();
  } TOCATCH
}

void toAnalyze::stop(void)
{
  try {
    for(std::list<toNoBlockQuery *>::iterator i=Running.begin();i!=Running.end();i++)
      delete (*i);
    Running.clear();
    Pending.clear();
    Stop->setEnabled(false);
    Current->setText(QString::null);
    if (!connection().needCommit())
      connection().rollback();
  } TOCATCH
}

void toAnalyze::createTool(void)
{
  AnalyzeTool.createWindow();
}
