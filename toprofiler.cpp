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
#include <qtoolbar.h>
#include <qspinbox.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qsplitter.h>
#include <qmessagebox.h>
#include <qlineedit.h>

#include "toprofiler.h"
#include "tosql.h"
#include "totool.h"
#include "tohighlightedtext.h"
#include "toconnection.h"
#include "toresultlong.h"

#include "toprofiler.moc"

#include "icons/clock.xpm"
#include "icons/refresh.xpm"
#include "icons/background.xpm"
#include "icons/execute.xpm"

static toSQL SQLProfilerRuns("toProfiler:ProfilerRuns",
			     "create table plsql_profiler_runs\n"
			     "(\n"
			     "  runid           number primary key,\n"
			     "  related_run     number,\n"
			     "  run_owner       varchar2(32),\n"
			     "  run_date        date,\n"
			     "  run_comment     varchar2(2047),\n"
			     "  run_total_time  number,\n"
			     "  run_system_info varchar2(2047),\n"
			     "  run_comment1    varchar2(2047),\n"
			     "  spare1          varchar2(256)\n"
			     ")",
			     "Create table plsql_profiler_runs");

static toSQL SQLProfilerUnits("toProfiler:ProfilerUnits",
			      "create table plsql_profiler_units\n"
			      "(\n"
			      "  runid              number references plsql_profiler_runs,\n"
			      "  unit_number        number,\n"
			      "  unit_type          varchar2(32),\n"
			      "  unit_owner         varchar2(32),\n"
			      "  unit_name          varchar2(32),\n"
			      "  unit_timestamp     date,\n"
			      "  total_time         number DEFAULT 0 NOT NULL,\n"
			      "  spare1             number,\n"
			      "  spare2             number,\n"
			      "  primary key (runid, unit_number)\n"
			      ")",
			      "Create table plsql_profiler_unites");

static toSQL SQLProfilerData("toProfiler:ProfilerData",
			     "create table plsql_profiler_data\n"
			     "(\n"
			     "  runid           number,\n"
			     "  unit_number     number,\n"
			     "  line#           number not null,\n"
			     "  total_occur     number,\n"
			     "  total_time      number,\n"
			     "  min_time        number,\n"
			     "  max_time        number,\n"
			     "  spare1          number,\n"
			     "  spare2          number,\n"
			     "  spare3          number,\n"
			     "  spare4          number,\n"
			     "  primary key (runid, unit_number, line#),\n"
			     "  foreign key (runid, unit_number) references plsql_profiler_units\n"
			     ")",
			     "Create table plsql_profiler_data");

static toSQL SQLProfilerNumber("toProfiler:ProfilerNumber",
			       "create sequence plsql_profiler_runnumber start with 1 nocache",
			       "Create sequence plsql_profiler_runnumber");

static toSQL SQLProfilerDetect("toProfiler:ProfilerDetect",
			       "select count(1)\n"
			       "  from plsql_profiler_data a,\n"
			       "       plsql_profiler_units b,\n"
			       "       plsql_profiler_runs c\n"
			       " where a.runid = null and b.runid = null and c.runid = null",
			       "Detect if profiler tables seems to exist");

static toSQL SQLListUnits("toProfiler:ListUnits",
			  "SELECT unit_type \"Type\",\n"
			  "       unit_owner \"Unit\",\n"
			  "       total_time \"Total time\",\n"
			  "       unit_timestamp \"Changed\",\n"
			  "       runid \" \",\n"
			  "       unit_number \" \"\n"
			  "  FROM plsql_profiler_units\n"
			  " WHERE runid = :run<char[101]>\n"
			  " ORDER BY unit_type||'.'||unit_owner",
			  "Get information of available units for a run, same columns and binds");

static toSQL SQLRunInfo("toProfiler:RunInfo",
			"SELECT *\n"
			"  FROM plsql_profiler_runs\n"
			" WHERE runid = :run<char[101]>",
			"Get information about a run, same binds");

static toSQL SQLTotalTime("toProfiler:TotalTime",
			  "SELECT run_total_time\n"
			  "  FROM plsql_profiler_runs\n"
			  " WHERE runid = :run<char[101]>",
			  "Get total time of a run, same binds");

static toSQL SQLListRuns("toProfiler:ListRuns",
			 "SELECT runid,run_owner,run_comment,run_total_time\n"
			 "  FROM plsql_profiler_runs\n"
			 " ORDER BY runid",
			 "Get list of available runs, same columns");

class toProfilerTool : public toTool {
  virtual char **pictureXPM(void)
  { return clock_xpm; }
public:
  toProfilerTool()
    : toTool(203,"PL/SQL Profiler")
  { }
  virtual const char *menuItem()
  { return "PL/SQL Profiler"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  { return new toProfiler(parent,connection); }
};

static toProfilerTool ProfilerTool;

class toProfilerUnits : public toResultLong {
  double Total;
public:
  class unitsItem : public toResultViewItem {
  public:
    unitsItem(QListView *parent,QListViewItem *after,const QString &buf=QString::null)
      : toResultViewItem(parent,after,buf)
    { }
    virtual void paintCell(QPainter *p,const QColorGroup &cg,
			   int column,int width,int alignment)
    {
      if (column==1) {
	toProfilerUnits *units=dynamic_cast<toProfilerUnits *>(listView());
	if (!units) {
	  QListViewItem::paintCell(p,cg,column,width,alignment);
	  return;
	}
	double val=100*text(column).toDouble()/units->total();
	QString ct;
	ct.sprintf("%0.1f",val);

	p->fillRect(0,0,int(val*width/100),height(),QBrush(blue));
	p->fillRect(int(val*width/100),0,width,height(),
		    QBrush(isSelected()?cg.highlight():cg.base()));

	QPen pen(isSelected()?cg.highlightedText():cg.foreground());
	p->setPen(pen);
	p->drawText(0,0,width,height(),AlignCenter,text(column));
      } else {
	QListViewItem::paintCell(p,cg,column,width,alignment);
      }
    }
  };

  virtual QListViewItem *createItem(QListViewItem *last,const QString &str)
  { return new unitsItem(this,last,str); }
  toProfilerUnits(QWidget *parent)
    : toResultLong(true,false,toQuery::Normal,parent)
  {
    setColumnAlignment(1,AlignCenter);
    setSQL(SQLListUnits);
  }
  double total(void) const
  { return Total; }
  void setTotal(double total)
  { Total=total; update(); }
};

toProfiler::toProfiler(QWidget *parent,toConnection &connection)
  : toToolWidget(ProfilerTool,"toprofiler.html",parent,connection)
{
  QToolBar *toolbar=toAllocBar(this,"Invalid Objects",connection.description());

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Refresh list",
		  "Refresh list",
		  this,SLOT(refresh()),
		  toolbar);

  toolbar->addSeparator();

  new QLabel("Repeat run ",toolbar);

  Repeat=new QSpinBox(toolbar);
  Repeat->setValue(5);

  toolbar->addSeparator();

  new QLabel("Comment ",toolbar);
  Comment=new QLineEdit(toolbar);
  Comment->setText("Unknown");

  toolbar->addSeparator();
  
  Background=new QToolButton(toolbar);
  Background->setToggleButton(true);
  Background->setIconSet(QIconSet(QPixmap((const char **)background_xpm)));
  QToolTip::add(Background,"Run profiling in background");

  toolbar->addSeparator();

  new QToolButton(QPixmap((const char **)execute_xpm),
		  "Execute current profiling",
		  "Execute current profiling",
		  this,SLOT(execute()),
		  toolbar);

  toolbar->setStretchableWidget(new QLabel(toolbar));

  QTabWidget *tabs=new QTabWidget(this);
  Script=new toHighlightedText(tabs);
  tabs->addTab(Script,"Script");

  QSplitter *splitter=new QSplitter(tabs);
  tabs->addTab(splitter,"Result");

  QVBox *box=new QVBox(splitter);
  Run=new QComboBox(box);
  connect(Run,SIGNAL(activated(int)),this,SLOT(changeRun()));
  Units=new toProfilerUnits(box);
  Lines=new toResultLong(splitter);

  CurrentRun=0;
}

void toProfiler::refresh(void)
{
  Run->clear();
  try {
    toQuery query(connection(),SQLListRuns);
    int id=0;
    while(!query.eof()) {
      QString runid=query.readValueNull();
      QString owner=query.readValueNull();
      QString comment=query.readValueNull();
      double total=query.readValueNull().toDouble()/1E9;
      if (!owner.isEmpty())
	owner="("+owner+")";
      QString timstr;
      if (total>=3600)
	timstr.sprintf(" [%d:%02d:%02d.%02d]",
		       int(total)/3600,
		       (int(total)/60)%60,
		       int(total)%60,
		       int(total*100)%100);
      else
	timstr.sprintf(" [%d:%02d.%02d]",
		       (int(total)/60)%60,
		       int(total)%60,
		       int(total*100)%100);
      Run->insertItem(runid+owner+": "+comment+timstr);
      if (runid.toInt()==CurrentRun)
	Run->setCurrentItem(id);
      id++;
    }
  } TOCATCH
}

static toSQL SQLStartProfiler("toProfiler:StartProfiler",
			       "BEGIN\n"
			       "  DBMS_PROFILER.START_PROFILER(:comment<char[4000],in>,\n"
			       "                               :com2<char[100],in>,\n"
			       "                               :runid<char[4000],out>);\n",
			       "Start profiler run, must have same binds");

static toSQL SQLStopProfiler("toProfiler:StopProfiler",
			      "  DBMS_PROFILER.STOP_PROFILER;\n"
			      "END;",
			      "Stop profiler run");

void toProfiler::execute(void)
{
  try {
    try {
      toQuery query(connection(),SQLProfilerDetect);
    } catch(const QString &str) {
      int ret=TOMessageBox::warning(this,
				    "Profiler tables doesn't exist",
				    "Profiler tables doesn't exist. Should TOra\n"
				    "try to create them in the current schema.\n"
				    "Should TOra try to create it?",
				    "&Yes","&No",0,1);
      if (ret==0) {
	connection().execute(SQLProfilerRuns);
	connection().execute(SQLProfilerUnits);
	connection().execute(SQLProfilerData);
	connection().execute(SQLProfilerNumber);
      } else
	return;
    }
    QString exc;
    exc=toSQL::string(SQLStartProfiler,connection());
    for(int i=0;i<Repeat->value();i++) {
      exc+=Script->text();
      exc+="\n";
    }
    exc+=toSQL::string(SQLStopProfiler,connection());

    toQuery query(connection(),
		  exc,
		  Comment->text(),
		  QString::number(Repeat->value())+" runs");
    CurrentRun=query.readValue().toInt();
    if (CurrentRun>0)
      refresh();
    else
      toStatusMessage("Something went wrong collecting statistics");
  } TOCATCH
}

void toProfiler::changeRun(void)
{
  CurrentRun=Run->currentText().toInt();
  QString run=QString::number(CurrentRun);
  try {
    toQList vals=toQuery::readQuery(connection(),SQLTotalTime,run);
    Units->setTotal(toShift(vals).toDouble());
    Units->changeParams(run);
  } TOCATCH
}
