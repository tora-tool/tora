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

#include <qspinbox.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtoolbar.h>
#include <qworkspace.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtimer.h>

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include "totool.h"
#include "toresultview.h"
#include "tosql.h"
#include "toconnection.h"
#include "toanalyze.h"
#include "toextract.h"
#include "toresultlong.h"

#include "toanalyze.moc"

#include "icons/refresh.xpm"
#include "icons/toanalyze.xpm"
#include "icons/execute.xpm"
#include "icons/stop.xpm"

class toAnalyzeTool : public toTool {
  virtual char **pictureXPM(void)
  { return toanalyze_xpm; }
public:
  toAnalyzeTool()
    : toTool(201,"Statistics Manager")
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
			   "select owner,\n"
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
			   " where owner = :own<char[101]> and iot_name is null",
			   "Get table statistics, first two columns and binds must be same",
			   "8.0");
static toSQL SQLListTables7("toAnalyze:ListTables",
			    "select owner,\n"
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
			    " where owner = :own<char[101]>",
			    QString::null,
			    "7.3");

toAnalyze::toAnalyze(QWidget *main,toConnection &connection)
  : toToolWidget(AnalyzeTool,"analyze.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,"Statistics Manager",connection.description());

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Refresh",
		  "Refresh",
		  this,SLOT(refresh()),
		  toolbar);

  toolbar->addSeparator();
  Schema=new QComboBox(toolbar);
  toQList users=toQuery::readQuery(connection,
				   toSQL::string(toSQL::TOSQL_USERLIST,connection));
  int j=0;
  for(toQList::iterator i=users.begin();i!=users.end();i++,j++) {
    Schema->insertItem(*i);
    if ((*i)==connection.user().upper())
      Schema->setCurrentItem(j);
  }
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

  Statistics=new toResultLong(true,false,toQuery::Normal,this);
  Statistics->setSelectionMode(QListView::Extended);
  Statistics->setReadAll(true);

  connect(Schema,SIGNAL(activated(int)),
	  this,SLOT(refresh()));

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));

  refresh();
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

void toAnalyze::changeOperation(int op)
{
  Sample->setEnabled(op==1);
  For->setEnabled(op==0||op==1);
}

void toAnalyze::refresh(void)
{
  Statistics->setSQL(QString::null);
  toQList par;
  par.insert(par.end(),Schema->currentText());
  Statistics->query(toSQL::string(SQLListTables,connection()),par);
}

void toAnalyze::poll(void)
{
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
}

void toAnalyze::execute(void)
{
  stop();

  QString sql="ANALYZE TABLE %1.%2 ";
  QString forc;
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

  for(QListViewItem *item=Statistics->firstChild();item;item=item->nextSibling()) {
    if (item->isSelected())
      toPush(Pending,sql.arg(item->text(0)).arg(item->text(1)));
  }

  toQList par;
  for(int i=0;i<Parallel->value();i++) {
    QString sql=toShift(Pending);
    if (!sql.isEmpty())
      toPush(Running,new toNoBlockQuery(connection(),sql,par));
  }
  Poll.start(100);
  Stop->setEnabled(true);
  poll();
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
