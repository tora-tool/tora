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

#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qgrid.h>

#include "tochangeconnection.h"
#include "toconnection.h"
#include "tomain.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"
#include "totuning.h"
#include "toresultstats.h"
#include "toresultbar.h"
#include "toresultline.h"
#include "toresultpie.h"
#include "toconf.h"

#include "totuning.moc"

#include "icons/refresh.xpm"
#include "icons/totuning.xpm"

static toSQL SQLDictionaryMiss("toTuning:Indicators:Important ratios:1DictionaryMiss",
			       "select to_char(round(sum(getmisses)/sum(gets)*100,2))||' %' from v$rowcache",
			       "Data dictionary row cache miss ratio (%). < 10%-15%");

static toSQL SQLLibraryCache("toTuning:Indicators:Important ratios:2LibraryCache",
			     "select to_char(round(sum(reloads)/sum(pins)*100,2))||' %' from v$librarycache",
			     "Library cache (Shared SQL) miss ratio (%). < 1%");

static toSQL SQLDataCache("toTuning:Indicators:Important ratios:3DataCache",
			  "SELECT TO_CHAR(ROUND((1-(physread/(dbblocks+consist)))*100,2))||' %'\n"
			  "  FROM (SELECT MAX(value) dbblocks FROM v$sysstat WHERE name = 'db block gets') a,\n"
			  "       (SELECT MAX(value) consist FROM v$sysstat WHERE name = 'consistent gets') b,\n"
			  "       (SELECT MAX(value) physread FROM v$sysstat WHERE name = 'physical reads') c",
			  "Data buffer cache hit ratio (%). > 60% - 70%");

static toSQL SQLLogRedo("toTuning:Indicators:Redo log contention:1LogSpace",
			"select value from v$sysstat where name='redo log space requests'",
			"Redo log space requests. Close to 0");

static toSQL SQLSystemHeadUndo("toTuning:Indicators:RBS contention:1SystemHeadUndo",
			       "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			       "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo header') a,\n"
			       "       (SELECT SUM(value) blocks FROM v$sysstat WHERE name IN ('consistent gets','db block gets')) b",
			       "System undo header waits (%). < 1%");

static toSQL SQLSystemBlockUndo("toTuning:Indicators:RBS contention:2SystemBlockUndo",
				"SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
				"  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo block') a,\n"
				"       (SELECT SUM(value) blocks FROM v$sysstat WHERE name IN ('consistent gets','db block gets')) b",
				"System undo block waits (%). < 1%");

static toSQL SQLHeadUndo("toTuning:Indicators:RBS contention:3HeadUndo",
			 "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			 "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo header') a,\n"
			 "       (SELECT SUM(value) blocks FROM v$sysstat WHERE name IN ('consistent gets','db block gets')) b",
			 "Undo head waits (%). < 1%");

static toSQL SQLBlockUndo("toTuning:Indicators:RBS contention:4BlockUndo",
			  "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			  "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo block') a,\n"
			  "       (SELECT SUM(value) blocks FROM v$sysstat WHERE name IN ('consistent gets','db block gets')) b",
			  "Undo block waits (%). < 1%");

static toSQL SQLTotalWaits("toTuning:Indicators:RBS contention:5TotalWaits",
			  "select to_char(round(sum(waits)/sum(gets)*100,2))||' %' from v$rollstat",
			  "Total RBS waits (%). < 1%");

static toSQL SQLBusyRate("toTuning:Indicators:MTS contention:1BusyRate",
			 "SELECT TO_CHAR(ROUND(NVL(SUM(busy)/(SUM(busy)+SUM(idle)),0)*100,2))||' %' FROM v$dispatcher",
			 "Busy rate for dispatchers (%). < 50%");

static toSQL SQLDispQueue("toTuning:Indicators:MTS contention:2DispQueue",
			  "SELECT DECODE(NVL(SUM(totalq),0),0, 'No responses', ROUND(SUM(wait)/SUM(totalq),3)) FROM v$queue WHERE type='DISPATCHER'",
			  "Dispatcher response queues waits (1/100s).");

static toSQL SQLServerQueue("toTuning:Indicators:MTS contention:3ServerQueue",
			    "SELECT DECODE (NVL(SUM(totalq),0),0, 'No requests', ROUND(SUM(wait)/SUM(totalq),3)) FROM v$queue WHERE type='COMMON'",
			    "Server request queue waits (1/100s).");

static toSQL SQLParameters("toTuning:Parameters",
			   "select name,value,description from v$parameter",
			   "Display parameters of Oracle server");

static toSQL SQLChartsPhysical("toTuning:Charts:1BBPhysical I/O",
			       "select SYSDATE,\n"
			       "       sum(decode(statistic#,40,value,0)) \"Physical reads\",\n"
			       "       sum(decode(statistic#,44,value,0)) \"Physical writes\",\n"
			       "       sum(decode(statistic#,105,value,0)) \"Redo writes\"\n"
			       "  from v$sysstat",
			       "Generate chart of physical I/O of database.");

static toSQL SQLChartsLogical("toTuning:Charts:2BBLogical I/O",
			       "select SYSDATE,\n"
			       "       sum(decode(statistic#,38,value,0)) \"Block gets\",\n"
			       "       sum(decode(statistic#,39,value,0)) \"Consistent gets\",\n"
			       "       sum(decode(statistic#,41,value,0)) \"Block changes\",\n"
			       "       sum(decode(statistic#,42,value,0)) \"Consistent changes\"\n"
			       "  from v$sysstat",
			       "Generate chart of physical I/O of database.");

static toSQL SQLChartsWait("toTuning:Charts:3BMWait events",
			   "select sysdate,\n"
			   "       QueueWait \"Dispatching\",\n"
			   "       DBWriteWait \"DB File Write\",\n"
			   "       WriteWait \"Write Complete\",\n"
			   "       DBReadWait-DBReadSingleWait \"DB File Read\",\n"
			   "       DBReadSingleWait \"DB File Single Read\",\n"
			   "       CtrlWait \"Control File I/O\",\n"
			   "       DirectWait \"Direct I/O\",\n"
			   "       LogWait \"Log file I/O\",\n"
			   "       SQLWait \"SQL*Net\"\n"
			   "  from (select sum(time_waited) TotalWait from v$system_event),\n"
			   "       (select sum(time_waited) SQLWait from v$system_event where event like 'SQL*Net%'),\n"
			   "       (select sum(time_waited) QueueWait from v$system_event where event like 'PX%' and event != 'PX Idle Wait'),\n"
			   "       (select sum(time_waited) WriteWait from v$system_event where event like 'write complete waits'),\n"
			   "       (select sum(time_waited) DBReadWait from v$system_event where event like 'db file % read'),\n"
			   "       (select sum(time_waited) DBWriteWait from v$system_event where event like 'db file % write'),\n"
			   "       (select sum(time_waited) DBReadSingleWait from v$system_event where event like 'db file scattered read'),\n"
			   "       (select sum(time_waited) CtrlWait from v$system_event where event like 'control file%'),\n"
			   "       (select sum(time_waited) DirectWait from v$system_event where event like 'direct path%'),\n"
			   "       (select sum(time_waited) LogWait from v$system_event where event like 'log file%' or event like 'LGRW%')",
			   "Used to generate chart for system wait time.");

static toSQL SQLChartsExecution("toTuning:Charts:6LNExecution",
				"select SYSDATE,\n"
				"       sum(decode(statistic#,181,value,0)) \"Execute\",\n"
				"       sum(decode(statistic#,180,value,0)) \"Hard parse\",\n"
				"       sum(decode(statistic#,179,value,0)) \"Parse\",\n"
				"       sum(decode(statistic#,6,value,0)) \"Calls\",\n"
				"       sum(decode(statistic#,4,value,0))  \"Commit\",\n"
				"       sum(decode(statistic#,5,value,0)) \"Rollbacks\"\n"
				"  from v$sysstat",
				"Used to generate chart for execution statistics.");

static toSQL SQLChartsExecutionPie("toTuning:Charts:5PNExecution Total",
				   "select value,decode(statistic#,181,'Execute',180,'Hard parse',179,'Parse',\n"
				   "                              6,'Calls',4,'Commit',5,'Rollbacks')\n"
				   "  from v$sysstat where statistic# in (180,179,181,6,4,5) order by name",
				   "Used to generate pie chart for execution statistics.");

static toSQL SQLChartsClients("toTuning:Charts:4BAClients",
			      "select sysdate,\n"
			      "       sum(decode(decode(type,'BACKGROUND','WHATEVER',status),'ACTIVE',1,0)) \"Active\",\n"
			      "       sum(decode(status,'INACTIVE',1,0)) \"Inactive\",\n"
			      "       sum(decode(type,'BACKGROUND',1,0)) \"System\"\n"
			      "  from v$session",
			      "Chart displaying connected clients");

static toSQL SQLChartsSGA("toTuning:Charts:7PSSGA",
			  "select value/to_number(:unit<char[101]>),name from v$sga",
			  "Chart display memory allocation");

static toSQL SQLChartsRedo("toTuning:Charts:8BSRedo log I/O",
			   "select SYSDATE,\n"
			   "       sum(decode(statistic#,101,value,0))/to_number(:unit<char[101]>) \"Redo size\",\n"
			   "       sum(decode(statistic#,103,value,0))/to_number(:unit<char[101]>) \"Redo wastage\"\n"
			   "  from v$sysstat",
			   "Used to generate chart for redo I/O statistics.");

class toTuningTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return totuning_xpm; }
public:
  toTuningTool()
    : toTool(10,"Server Tuning")
  { }
  virtual const char *menuItem()
  { return "Server Tuning"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    QWidget *window=new toTuning(parent,connection);
    window->setIcon(*toolbarImage());
    return window;
  }
};

static toTuningTool TuningTool;

static QPixmap *toRefreshPixmap;

toTuning::toTuning(QWidget *main,toConnection &connection)
  : toToolWidget("tuning.html",main,connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);

  QToolBar *toolbar=toAllocBar(this,"Server Tuning",connection.connectString());

  new QToolButton(*toRefreshPixmap,
		  "Refresh",
		  "Refresh",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QLabel("Refresh",toolbar);
  connect(toRefreshCreate(toolbar),SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));
  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  Tabs=new QTabWidget(this);

  QGrid *grid=new QGrid(2,Tabs);

  QString unitStr=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
  list<QString> unit;
  unit.insert(unit.end(),QString::number(toSizeDecode(unitStr)));
  {
    list<QString> val=toSQL::range("toTuning:Charts");
    for(list<QString>::iterator i=val.begin();i!=val.end();i++) {
      QStringList parts=QStringList::split(":",*i);
      if (parts[2].mid(1,1)=="B") {
	toResultBar *chart=new toResultBar(grid);
	chart->setTitle(parts[2].mid(3));
	list<QString> par;
	if (parts[2].mid(2,1)=="B")
	  chart->setYPostfix(" blocks/s");
	else if (parts[2].mid(2,1)=="M")
	  chart->setYPostfix(" ms/s");
	else if (parts[2].mid(2,1)=="S") {
	  par=unit;
	  QString t=unitStr;
	  t+="/s";
	  chart->setYPostfix(t);
	} else if (parts[2].mid(2,1)=="A")
	  chart->setFlow(false);
	else
	  chart->setYPostfix("/s");
	chart->setSamples(100);
	chart->query(toSQL::sql(*i,connection),par);
      } else if (parts[2].mid(1,1)=="L") {
	toResultLine *chart=new toResultLine(grid);
	chart->setTitle(parts[2].mid(3));
	list<QString> par;
	if (parts[2].mid(2,1)=="B")
	  chart->setYPostfix(" blocks/s");
	else if (parts[2].mid(2,1)=="S") {
	  par=unit;
	  QString t=unitStr;
	  t+="/s";
	  chart->setYPostfix(t);
	} else
	  chart->setYPostfix("/s");
	chart->setSamples(100);
	chart->query(toSQL::sql(*i,connection),par);
      } else if (parts[2].mid(1,1)=="P") {
	toResultPie *chart=new toResultPie(grid);
	chart->setTitle(parts[2].mid(3));
	if (parts[2].mid(2,1)=="S") {
	  chart->query(toSQL::sql(*i,connection),unit);
	  chart->setPostfix(unitStr);
	} else
	  chart->query(toSQL::sql(*i,connection));
      } else
	throw QString("Wrong format of name on chart.");
    }
  }

  Tabs->addTab(grid,"Charts");

  Indicators=new toListView(Tabs);
  Indicators->setRootIsDecorated(true);
  Indicators->addColumn("Indicator");
  Indicators->addColumn("Value");
  Indicators->addColumn("Reference");
  Tabs->addTab(Indicators,"Indicators");

  Statistics=new toResultStats(Tabs);
  Tabs->addTab(Statistics,"Statistics");

  Parameters=new toResultView(true,false,Tabs);
  Parameters->setSQL(SQLParameters);
  Tabs->addTab(Parameters,"Parameters");

  connect(Tabs,SIGNAL(currentChanged(QWidget *)),this,SLOT(refresh()));
  connect(timer(),SIGNAL(timeout(void)),this,SLOT(refresh(void)));

  Tabs->setCurrentPage(0);
  refresh();
}

void toTuning::changeRefresh(const QString &str)
{
  toRefreshParse(timer(),str);
}

void toTuning::refresh(void)
{
  QWidget *current=Tabs->currentPage();
  if (current==Indicators) {
    Indicators->clear();
    list<QString> val=toSQL::range("toTuning:Indicators");
    QListViewItem *parent=NULL;
    QListViewItem *last=NULL;
    for(list<QString>::iterator i=val.begin();i!=val.end();i++) {
      try {
	list<QString> val=toReadQuery(connection(),toSQL::sql(*i,connection()));
	QStringList parts=QStringList::split(":",*i);
	if (!parent||parent->text(0)!=parts[2]) {
	  parent=new toResultViewItem(Indicators,NULL,parts[2]);
	  parent->setOpen(true);
	  last=NULL;
	}
	QStringList dsc=QStringList::split(".",toSQL::description(*i));
	QString first=dsc[0];
	first+=".";
	last=new toResultViewItem(parent,last,first);
	last->setText(1,*(val.begin()));
	if (dsc.count()>1)
	  last->setText(2,dsc[1]);
      } TOCATCH
    }
  } else if (current==Statistics)
    Statistics->refreshStats();
  else if (current==Parameters)
    Parameters->refresh();
}
