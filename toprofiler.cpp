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

#include "tochangeconnection.h"
#include "toconnection.h"
#include "tohighlightedtext.h"
#include "toprofiler.h"
#include "toresultitem.h"
#include "toresultlong.h"
#include "tosql.h"
#include "totool.h"
#include "toworksheet.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include "toprofiler.moc"

#include "icons/clock.xpm"
#include "icons/execute.xpm"
#include "icons/refresh.xpm"
#if 0
#include "icons/background.xpm"
#endif

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
			  "SELECT unit_owner \"Owner\",\n"
			  "       unit_name \"Name\",\n"
			  "       total_time \"Total time\",\n"
			  "       unit_timestamp \"Changed\",\n"
			  "       unit_type \" \",\n"
			  "       runid \" \",\n"
			  "       unit_number \" \"\n"
			  "  FROM plsql_profiler_units\n"
			  " WHERE runid = :run<char[101]>\n"
			  "   AND unit_owner != '<anonymous>'\n"
			  " ORDER BY total_time DESC",
			  "Get information of available units for a run, same columns and binds");

static toSQL SQLListSource("toProfiler:ListSource",
                           "SELECT *\n"
                           "  FROM (SELECT a.line \"#\",\n"
                           "               NVL(b.total_occur,0) \"Count\",\n"
                           "               b.total_time \"Total\",\n"
                           "               b.max_time \"Max\",\n"
                           "               b.min_time \"Min\",\n"
                           "               TRANSLATE(a.text,'A'||CHR(10),'A') \"Source\"\n"
                           "          FROM all_source a,\n"
                           "               plsql_profiler_data b\n"
                           "         WHERE a.owner = :owner<char[101]>\n"
                           "           AND a.name = :name<char[101]>\n"
                           "           AND a.type = :typ<char[101]>\n"
                           "           AND b.runid = :run<char[101]>\n"
                           "           AND b.unit_number = :unit<char[101]>\n"
                           "           AND a.line = b.line#\n"
                           "        UNION\n"
                           "        SELECT a.line,\n"
                           "               0,\n"
                           "               0,\n"
                           "               0,\n"
                           "               0,\n"
                           "               TRANSLATE(a.text,'A'||CHR(10),'A')\n"
                           "          FROM all_source a\n"
                           "         WHERE a.owner = :owner<char[101]>\n"
                           "           AND a.name = :name<char[101]>\n"
                           "           AND a.type = :typ<char[101]>\n"
                           "           AND not exists (select 'X'\n"
                           "                             from plsql_profiler_data b\n"
                           "                            where b.runid = :run<char[1019>\n"
                           "                              and b.unit_number = :unit<char[101]>\n"
                           "                              and a.line = b.line#)\n"
                           "       )\n"
			   " ORDER BY 1",
			   "Get information about an object with it's statistics data. This one is tricky, must have same binds and columns");

static toSQL SQLRunInfo("toProfiler:RunInfo",
			"SELECT Related_Run \"Related run\",\n"
			"       Run_Owner \"Owner\",\n"
			"       Run_Comment \"Comment\",\n"
			"       to_char(to_date(trunc(run_total_time/1E9),'SSSSS'),'HH24:MI:SS')||'.'||\n"
			"       SUBSTR(TO_CHAR(TRUNC(run_total_time/1E7)-TRUNC(run_total_time/1E7,-2),'00'),2) \"Total time\",\n"
			"       Run_System_Info \"System info\",\n"
			"       Run_Comment1 \"Extra comment\"\n"
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
			 " ORDER BY runid DESC",
			 "Get list of available runs, same columns");

static toSQL SQLStartProfiler("toProfiler:StartProfiler",
			      "DECLARE\n"
			      "  PRAGMA AUTONOMOUS_TRANSACTION;\n"
			      "BEGIN\n"
			      "  DBMS_PROFILER.START_PROFILER(:comment<char[4000],in>,\n"
			      "                               :com2<char[100],in>,\n"
			      "                               :runid<char[4000],out>);\n",
			      "Start profiler run, must have same binds");

static toSQL SQLStopProfiler("toProfiler:StopProfiler",
			     "  DBMS_PROFILER.STOP_PROFILER;\n"
			     "  UPDATE plsql_profiler_units a\n"
			     "     SET total_time = (SELECT SUM(total_time)\n"
			     "                         FROM plsql_profiler_data b\n"
			     "                        WHERE a.runid = b.runid\n"
			     "                          AND a.unit_number = b.unit_number);\n"
			     "  COMMIT;\n"
			     "END;",
			     "Stop profiler run");

class toProfilerTool : public toTool {
  virtual char **pictureXPM(void)
  { return clock_xpm; }
public:
  toProfilerTool()
    : toTool(101,"PL/SQL Profiler")
  { }
  virtual const char *menuItem()
  { return "PL/SQL Profiler"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  { return new toProfiler(parent,connection); }
};

static toProfilerTool ProfilerTool;

static QString FormatTime(double tim)
{
  QString timstr;
  if (tim>=3600)
    timstr.sprintf("%d:%02d:%02d.%02d",
		   int(tim)/3600,
		   (int(tim)/60)%60,
		   int(tim)%60,
		   int(tim*100)%100);
  else if (tim > 1)
    timstr.sprintf("%d:%02d.%02d",
		   (int(tim)/60)%60,
		   int(tim)%60,
		   int(tim*100)%100);
  else if (tim > 0.001)
    timstr.sprintf("%d ms",(int)(tim*1000));
  else if (tim > 0)
    timstr.sprintf("%d ns",(int)(tim*1E9));
  else
    timstr="0";
  return timstr;
}

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
      if (column==2) {
	toProfilerUnits *units=dynamic_cast<toProfilerUnits *>(listView());
	if (!units) {
	  QListViewItem::paintCell(p,cg,column,width,alignment);
	  return;
	}
	double total=allText(column).toDouble();
	QString timstr=FormatTime(total/1E9);
	double val=total/units->total();
	
	p->fillRect(0,0,int(val*width),height(),QBrush(blue));
	p->fillRect(int(val*width),0,width,height(),
		    QBrush(isSelected()?cg.highlight():cg.base()));

	QPen pen(isSelected()?cg.highlightedText():cg.foreground());
	p->setPen(pen);
	p->drawText(0,0,width,height(),AlignRight,timstr);
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
    setColumnAlignment(3,AlignRight);
    setSQL(SQLListUnits);
  }
  double total(void) const
  { return Total; }
  void setTotal(double total)
  { Total=total; update(); }
};

class toProfilerSource : public toResultLong {
  double TotalOccur;
  double TotalTime;
  double MaxTime;
  double MinTime;
public:
  class listItem : public toResultViewItem {
  public:
    listItem(QListView *parent,QListViewItem *after,const QString &buf=QString::null)
      : toResultViewItem(parent,after,buf)
    { }
    virtual QString text(int col) const
    {
      if (col==2||col==3||col==4)
	return FormatTime(allText(col).toDouble()/1E9);
      else
	return toResultViewItem::text(col);
    }
    virtual void paintCell(QPainter *p,const QColorGroup &cg,
			   int column,int width,int alignment)
    {
      if (column==1||column==2||column==3||column==4) {
	toProfilerSource *source=dynamic_cast<toProfilerSource *>(listView());
	if (!source) {
	  QListViewItem::paintCell(p,cg,column,width,alignment);
	  return;
	}
	double total=allText(column).toDouble();
	double val;
	switch (column) {
	case 1:
	  val=total/source->TotalOccur;
	  break;
	case 2:
	  val=total/source->TotalTime;
	  break;
	case 3:
	  val=total/source->MaxTime;
	  break;
	case 4:
	  val=total/source->MinTime;
	  break;
	default:
	  val=0;
	}
	
	p->fillRect(0,0,int(val*width),height(),QBrush(blue));
	p->fillRect(int(val*width),0,width,height(),
		    QBrush(isSelected()?cg.highlight():cg.base()));

	QPen pen(isSelected()?cg.highlightedText():cg.foreground());
	p->setPen(pen);
	p->drawText(0,0,width,height(),AlignRight,text(column));
      } else {
	QListViewItem::paintCell(p,cg,column,width,alignment);
      }
    }
  };

  virtual QListViewItem *createItem(QListViewItem *last,const QString &str)
  { return new listItem(this,last,str); }
  void clearTotals(void)
  { TotalOccur=TotalTime=MaxTime=MinTime=0; }
  toProfilerSource(QWidget *parent)
    : toResultLong(true,false,toQuery::Normal,parent)
  {
    setColumnAlignment(1,AlignRight);
    setColumnAlignment(2,AlignRight);
    setColumnAlignment(3,AlignRight);
    setColumnAlignment(4,AlignRight);
    clearTotals();
  }
  void calcTotals(void)
  {
    clearTotals();
    for(QListViewItem *qi=firstChild();qi;qi=qi->nextSibling()) {
      toResultViewItem *item=dynamic_cast<toResultViewItem *>(qi);
      if (item) {
	TotalOccur+=item->allText(1).toDouble();
	TotalTime+=item->allText(2).toDouble();
	double t;
	t=item->allText(3).toDouble();
	if (t>MaxTime)
	  MaxTime=t;
	t=item->allText(4).toDouble();
	if (t>MinTime)
	  MinTime=t;
      }
    }
    update();
  }
  friend class toProfilerSource::listItem;
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
  Repeat->setMaxValue(1000);

  toolbar->addSeparator();

  new QLabel("Comment ",toolbar);
  Comment=new QLineEdit(toolbar);
  Comment->setText("Unknown");

  toolbar->addSeparator();
  
#if 0
  Background=new QToolButton(toolbar);
  Background->setToggleButton(true);
  Background->setIconSet(QIconSet(QPixmap((const char **)background_xpm)));
  QToolTip::add(Background,"Run profiling in background");

  toolbar->addSeparator();
#endif

  new QToolButton(QPixmap((const char **)execute_xpm),
		  "Execute current profiling",
		  "Execute current profiling",
		  this,SLOT(execute()),
		  toolbar);

  toolbar->setStretchableWidget(new QLabel(toolbar));

  new toChangeConnection(toolbar);

  Tabs=new QTabWidget(this);
  Script=new toWorksheet(Tabs,NULL,connection);
  Tabs->addTab(Script,"Script");

  Result=new QSplitter(Tabs);
  Tabs->addTab(Result,"Result");

  QVBox *box=new QVBox(Result);
  Run=new QComboBox(box);
  QSplitter *vsplit=new QSplitter(Vertical,box);
  Info=new toResultItem(2,vsplit);
  Info->setSQL(SQLRunInfo);
  connect(Run,SIGNAL(activated(int)),this,SLOT(changeRun()));
  Units=new toProfilerUnits(vsplit);
  Units->setReadAll(true);
  Units->setSelectionMode(QListView::Single);
  connect(Units,SIGNAL(selectionChanged()),this,SLOT(changeObject()));
  Lines=new toProfilerSource(Result);
  Lines->setReadAll(true);
  connect(Lines,SIGNAL(done()),this,SLOT(calcTotals()));

  LastUnit=CurrentRun=0;
  show();

  try {
    toQuery query(connection,SQLProfilerDetect);
  } catch(const QString &) {
    int ret=TOMessageBox::warning(this,
				  "Profiler tables doesn't exist",
				  "Profiler tables doesn't exist. Should TOra\n"
				  "try to create them in the current schema.\n"
				  "Should TOra try to create it?",
				  "&Yes","&No",0,1);
    if (ret==0) {
      try {
	connection.execute(SQLProfilerRuns);
	connection.execute(SQLProfilerUnits);
	connection.execute(SQLProfilerData);
	connection.execute(SQLProfilerNumber);
      } catch (const QString &str) {
	toStatusMessage(str);
	QTimer::singleShot(1,this,SLOT(noTables()));
	return;
      }
    } else {
      QTimer::singleShot(1,this,SLOT(noTables()));
      return;
    }
  }

  refresh();
}

void toProfiler::refresh(void)
{
  Run->clear();
  Run->insertItem("Select run");
  try {
    toQuery query(connection(),SQLListRuns);
    int id=1;
    while(!query.eof()) {
      QString runid=query.readValueNull();
      QString owner=query.readValueNull();
      QString comment=query.readValueNull();
      double total=query.readValueNull().toDouble()/1E9;
      if (!owner.isEmpty())
	owner="("+owner+")";
      QString timstr=" ["+FormatTime(total)+"]";
      Run->insertItem(runid+owner+": "+comment+timstr);
      if (runid.toInt()==CurrentRun) {
	Run->setCurrentItem(id);
        changeRun();
      }
      id++;
    }
  } TOCATCH
}

void toProfiler::execute(void)
{
  try {
    QString exc;
    exc=toSQL::string(SQLStartProfiler,connection());
    for(int i=0;i<Repeat->value();i++) {
      exc+=Script->editor()->text();
      exc+="\n";
    }
    exc+=toSQL::string(SQLStopProfiler,connection());

    toQuery query(connection(),
		  exc,
		  Comment->text(),
		  QString::number(Repeat->value())+" runs");
    CurrentRun=query.readValue().toInt();
    if (CurrentRun>0) {
      Tabs->showPage(Result);
      refresh();
    } else
      toStatusMessage("Something went wrong collecting statistics");
  } TOCATCH
}

void toProfiler::changeRun(void)
{
  QString t=Run->currentText();
  int pos=t.find("(");
  if (pos<0)
    pos=t.find(":");
  if (pos>=0)
    CurrentRun=t.mid(0,pos).toInt();
  QString run=QString::number(CurrentRun);
  try {
    toQList vals=toQuery::readQuery(connection(),SQLTotalTime,run);
    Units->setTotal(toShift(vals).toDouble());
    Units->changeParams(run);
    Info->changeParams(run);
  } TOCATCH
}

void toProfiler::calcTotals(void)
{
  Lines->calcTotals();
}

void toProfiler::changeObject(void)
{
  QListViewItem *qi=Units->selectedItem();
  toResultViewItem *item=qi?dynamic_cast<toResultViewItem *>(qi):NULL;
  if (item) {
    toQList par;
    toPush(par,toQValue(item->allText(0)));
    toPush(par,toQValue(item->allText(1)));
    toPush(par,toQValue(item->allText(4)));
    toPush(par,toQValue(item->allText(5)));
    toPush(par,toQValue(item->allText(6)));
    Lines->clearTotals();
    Lines->query(toSQL::string(SQLListSource,connection()),par);
  }
}
