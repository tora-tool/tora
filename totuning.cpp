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

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tonoblockquery.h"
#include "toresultbar.h"
#include "toresultitem.h"
#include "toresultline.h"
#include "toresultlock.h"
#include "toresultlong.h"
#include "toresultparam.h"
#include "toresultpie.h"
#include "toresultstats.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"
#include "totuning.h"
#include "totuningsettingui.h"
#include "towaitevents.h"

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <time.h>
#include <stdio.h>

#include <qcombobox.h>
#include <qgrid.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qobjcoll.h>
#include <qpainter.h>
#include <qscrollview.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qvbox.h>
#include <qworkspace.h>

#include "totuning.moc"
#include "totuningoverviewui.moc"
#include "totuningsettingui.moc"

#include "icons/refresh.xpm"
#include "icons/totuning.xpm"
#include "icons/compile.xpm"

#define CONF_OVERVIEW "Overview"
#define CONF_FILEIO   "File I/O"
#define CONF_WAITS    "Wait events"
#define CONF_CHART    "chart"

static std::list<QCString> TabList(void)
{
  std::list<QCString> ret;
  ret.insert(ret.end(),CONF_OVERVIEW);
  std::list<QCString> val=toSQL::range("toTuning:Charts");
  QString last;
  for(std::list<QCString>::iterator i=val.begin();i!=val.end();i++) {
    QStringList parts=QStringList::split(QString::fromLatin1(":"),QString::fromLatin1(*i));
    if (parts.count()==3) {
      parts.append(parts[2]);
      parts[2]=QString::fromLatin1("Charts");
    }
    if (last!=parts[2])
      ret.insert(ret.end(),parts[2].latin1());
    last=parts[2];
  }
  ret.insert(ret.end(),CONF_WAITS);
  ret.insert(ret.end(),CONF_FILEIO);
  return ret;
}

class toTuningSetup : public toTuningSettingUI, public toSettingTab {
  toTool *Tool;
public:
  toTuningSetup(toTool *tool,QWidget* parent = 0,const char* name = 0)
    : toTuningSettingUI(parent,name),toSettingTab("tuning.html#preferences"),Tool(tool)
  {
    std::list<QCString> tabs=TabList();
    for(std::list<QCString>::iterator i=tabs.begin();i!=tabs.end();i++) {
      QListViewItem *item=new QListViewItem(EnabledTabs,QString::fromLatin1(*i));
      if (!tool->config(*i,"").isEmpty())
	item->setSelected(true);
    }
    EnabledTabs->setSorting(0);
  }
  virtual void saveSetting(void)
  {
    for (QListViewItem *item=EnabledTabs->firstChild();item;item=item->nextSibling()) {
      if (item->isSelected()||Tool->config(item->text(0).latin1(),"Undefined")!="Undefined")
	Tool->setConfig(item->text(0).latin1(),QString::fromLatin1((item->isSelected()?"Yes":"")));
    }
  }
};

class toTuningTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return totuning_xpm; }
public:
  toTuningTool()
    : toTool(30,"Server Tuning")
  { }
  virtual const char *menuItem()
  { return "Server Tuning"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toTuning(parent,connection);
  }
  virtual QWidget *configurationTab(QWidget *parent)
  {
    return new toTuningSetup(this,parent);
  }
};

static toTuningTool TuningTool;

static toSQL SQLDictionaryMiss("toTuning:Indicators:Important ratios:1DictionaryMiss",
			       "select sum(getmisses)/sum(gets)*100,' %' from v$rowcache",
			       "Data dictionary row cache miss ratio (%). < 10%-15%");

static toSQL SQLImportantLibraryCache("toTuning:Indicators:Important ratios:2LibraryCache",
				      "select sum(reloads)/sum(pins)*100,' %' from v$librarycache",
				      "Library cache (Shared SQL) miss ratio (%). < 1%");

static toSQL SQLDataCache("toTuning:Indicators:Important ratios:3DataCache",
			  "SELECT (1-SUM(DECODE(statistic#,40,value,0))/SUM(DECODE(statistic#,38,value,39,value,0)))*100,' %'\n"
			  "  FROM v$sysstat\n"
			  " WHERE statistic# IN (38,39,40)",
			  "Data buffer cache hit ratio (%). > 60% - 70%",
			  "8.0");

static toSQL SQLDataCache9("toTuning:Indicators:Important ratios:3DataCache",
			   "SELECT (1-SUM(DECODE(statistic#,42,value,0))/SUM(DECODE(statistic#,40,value,41,value,0)))*100,' %'\n"
			   "  FROM v$sysstat\n"
			   " WHERE statistic# IN (40,41,42)",
			   "",
			   "9.0");

static toSQL SQLDataCache7("toTuning:Indicators:Important ratios:3DataCache",
			   "SELECT (1-SUM(DECODE(statistic#,39,value,0))/SUM(DECODE(statistic#,37,value,38,value,0)))*100,' %'\n"
			   "  FROM v$sysstat\n"
			   " WHERE statistic# IN (37,38,39)",
			   "",
			   "7.3");

static toSQL SQLLogRedo("toTuning:Indicators:Redo log contention:1LogSpace",
			"select value from v$sysstat where statistic# = 108",
			"Redo log space requests. Close to 0",
			"8.0");

static toSQL SQLLogRedo9("toTuning:Indicators:Redo log contention:1LogSpace",
			 "select value from v$sysstat where statistic# = 122",
			 "",
			 "9.0");

static toSQL SQLLogRedo7("toTuning:Indicators:Redo log contention:1LogSpace",
			 "select value from v$sysstat where statistic# = 94",
			 "",
			 "7.3");

static toSQL SQLSystemHeadUndo("toTuning:Indicators:RBS contention:1SystemHeadUndo",
			       "SELECT count/blocks*100,' %'\n"
			       "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo header') a,\n"
			       "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (38,39)) b",
			       "System undo header waits (%). < 1%",
			       "8.0");

static toSQL SQLSystemHeadUndo9("toTuning:Indicators:RBS contention:1SystemHeadUndo",
				"SELECT count/blocks*100,' %'\n"
				"  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo header') a,\n"
				"       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (40,41)) b",
				"",
				"9.0");

static toSQL SQLSystemHeadUndo7("toTuning:Indicators:RBS contention:1SystemHeadUndo",
				"SELECT count/blocks*100,' %'\n"
				"  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo header') a,\n"
				"       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
				"",
				"7.3");

static toSQL SQLSystemBlockUndo("toTuning:Indicators:RBS contention:2SystemBlockUndo",
				"SELECT count/blocks*100,' %'\n"
				"  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo block') a,\n"
				"       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (38,39)) b",
				"System undo block waits (%). < 1%",
				"8.0");

static toSQL SQLSystemBlockUndo9("toTuning:Indicators:RBS contention:2SystemBlockUndo",
				 "SELECT count/blocks*100,' %'\n"
				 "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo block') a,\n"
				 "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (40,41)) b",
				 "",
				 "9.0");

static toSQL SQLSystemBlockUndo7("toTuning:Indicators:RBS contention:2SystemBlockUndo",
				 "SELECT count/blocks*100,' %'\n"
				 "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo block') a,\n"
				 "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
				 "",
				 "7.3");

static toSQL SQLHeadUndo("toTuning:Indicators:RBS contention:3HeadUndo",
			 "SELECT count/blocks*100,' %'\n"
			 "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo header') a,\n"
			 "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (38,39)) b",
			 "Undo head waits (%). < 1%",
			 "8.0");

static toSQL SQLHeadUndo9("toTuning:Indicators:RBS contention:3HeadUndo",
			  "SELECT count/blocks*100,' %'\n"
			  "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo header') a,\n"
			  "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (40,41)) b",
			  "",
			  "9.0");

static toSQL SQLHeadUndo7("toTuning:Indicators:RBS contention:3HeadUndo",
			  "SELECT count/blocks*100,' %'\n"
			  "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo header') a,\n"
			  "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
			  "",
			  "7.3");

static toSQL SQLBlockUndo("toTuning:Indicators:RBS contention:4BlockUndo",
			  "SELECT count/blocks*100,' %'\n"
			  "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo block') a,\n"
			  "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (38,39)) b",
			  "Undo block waits (%). < 1%",
			  "8.0");

static toSQL SQLBlockUndo9("toTuning:Indicators:RBS contention:4BlockUndo",
			   "SELECT count/blocks*100,' %'\n"
			   "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo block') a,\n"
			   "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (40,41)) b",
			   "",
			   "9.0");

static toSQL SQLBlockUndo7("toTuning:Indicators:RBS contention:4BlockUndo",
			   "SELECT count/blocks*100,' %'\n"
			   "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo block') a,\n"
			   "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
			   "",
			   "7.3");

static toSQL SQLTotalWaits("toTuning:Indicators:RBS contention:5TotalWaits",
			  "select sum(waits)/sum(gets)*100,' %' from v$rollstat",
			  "Total RBS waits (%). < 1%");

static toSQL SQLBusyRate("toTuning:Indicators:MTS contention:1BusyRate",
			 "SELECT NVL(SUM(busy)/(SUM(busy)+SUM(idle)),0)*100,' %' FROM v$dispatcher",
			 "Busy rate for dispatchers (%). < 50%");

static toSQL SQLDispQueue("toTuning:Indicators:MTS contention:2DispQueue",
			  "SELECT DECODE(NVL(SUM(totalq),0),0, 'No responses', ROUND(SUM(wait)/SUM(totalq),3)) FROM v$queue WHERE type='DISPATCHER'",
			  "Dispatcher response queues waits (1/100s).");

static toSQL SQLServerQueue("toTuning:Indicators:MTS contention:3ServerQueue",
			    "SELECT DECODE (NVL(SUM(totalq),0),0, 'No requests', ROUND(SUM(wait)/SUM(totalq),3)) FROM v$queue WHERE type='COMMON'",
			    "Server request queue waits (1/100s).");

static toSQL SQLOptions("toTuning:Options",
			"select parameter \"Parameter\",\n"
			"       decode(value,'TRUE','Enabled','Disabled') \"Enabled\"\n"
			"  from v$option order by parameter",
			"Display options available in database");

static toSQL SQLLicense("toTuning:License",
			"select * from v$license",
			"Display licenses available for database");

static toSQL SQLChartsPhysical("toTuning:Charts:1BBPhysical I/O",
			       "select SYSDATE,\n"
			       "       sum(decode(statistic#,40,value,0)) \"Physical reads\",\n"
			       "       sum(decode(statistic#,44,value,0)) \"Physical writes\",\n"
			       "       sum(decode(statistic#,105,value,0)) \"Redo writes\"\n"
			       "  from v$sysstat where statistic# in (40,44,105)",
			       "Generate chart of physical I/O of database.",
			       "8.0");

static toSQL SQLChartsPhysical9("toTuning:Charts:1BBPhysical I/O",
				"select SYSDATE,\n"
				"       sum(decode(statistic#,42,value,0)) \"Physical reads\",\n"
				"       sum(decode(statistic#,46,value,0)) \"Physical writes\",\n"
				"       sum(decode(statistic#,119,value,0)) \"Redo writes\"\n"
				"  from v$sysstat where statistic# in (42,46,119)",
				"",
				"9.0");

static toSQL SQLChartsPhysical7("toTuning:Charts:1BBPhysical I/O",
				"select SYSDATE,\n"
				"       sum(decode(statistic#,39,value,0)) \"Physical reads\",\n"
				"       sum(decode(statistic#,40,value,0)) \"Physical writes\",\n"
				"       sum(decode(statistic#,91,value,0)) \"Redo writes\"\n"
				"  from v$sysstat where statistic# in (39,40,91)",
				"",
				"7.3");

static toSQL SQLChartsLogical("toTuning:Charts:2BBLogical I/O",
			      "select SYSDATE,\n"
			      "       sum(decode(statistic#,38,value,0)) \"Block gets\",\n"
			      "       sum(decode(statistic#,39,value,0)) \"Consistent gets\",\n"
			      "       sum(decode(statistic#,41,value,0)) \"Block changes\",\n"
			      "       sum(decode(statistic#,42,value,0)) \"Consistent changes\"\n"
			      "  from v$sysstat where statistic# in (38,39,41,42)",
			      "Generate chart of physical I/O of database.",
			      "8.0");

static toSQL SQLChartsLogical9("toTuning:Charts:2BBLogical I/O",
			       "select SYSDATE,\n"
			       "       sum(decode(statistic#,40,value,0)) \"Block gets\",\n"
			       "       sum(decode(statistic#,41,value,0)) \"Consistent gets\",\n"
			       "       sum(decode(statistic#,43,value,0)) \"Block changes\",\n"
			       "       sum(decode(statistic#,44,value,0)) \"Consistent changes\"\n"
			       "  from v$sysstat where statistic# in (40,41,43,44)",
			       "",
			       "9.0");

static toSQL SQLChartsLogical7("toTuning:Charts:2BBLogical I/O",
			       "select SYSDATE,\n"
			       "       sum(decode(statistic#,37,value,0)) \"Block gets\",\n"
			       "       sum(decode(statistic#,38,value,0)) \"Consistent gets\",\n"
			       "       sum(decode(statistic#,43,value,0)) \"Block changes\",\n"
			       "       sum(decode(statistic#,45,value,0)) \"Consistent changes\"\n"
			       "  from v$sysstat where statistic# in (37,38,43,45)",
			       "",
			       "7.3");

static toSQL SQLChartsWait("toTuning:Charts:3BMWait events",
			   "select sysdate,\n"
			   "       cpu*10 \"CPU\",\n"
			   "       parallel*10 \"Parallel execution\",\n"
			   "       filewrite*10 \"DB File Write\",\n"
			   "       writecomplete*10 \"Write Complete\",\n"
			   "       fileread*10 \"DB File Read\",\n"
			   "       singleread*10 \"DB Single File Read\",\n"
			   "       control*10 \"Control File I/O\",\n"
			   "       direct*10 \"Direct I/O\",\n"
			   "       log*10 \"Log file\",\n"
			   "       net*10 \"SQL*Net\"\n"
			   "  from (select SUM(DECODE(SUBSTR(event,1,2),'PX',time_waited,0))-SUM(DECODE(event,'PX Idle Wait',time_waited,0)) parallel,\n"
			   "               SUM(DECODE(event,'db file parallel write',time_waited,'db file single write',time_waited,0)) filewrite,\n"
			   "               SUM(DECODE(event,'write complete waits',time_waited,NULL)) writecomplete,\n"
			   "               SUM(DECODE(event,'db file parallel read',time_waited,'db file sequential read',time_waited,0)) fileread,\n"
			   "               SUM(DECODE(event,'db file scattered read',time_waited,0)) singleread,\n"
			   "               SUM(DECODE(SUBSTR(event,1,12),'control file',time_waited,0)) control,\n"
			   "               SUM(DECODE(SUBSTR(event,1,11),'direct path',time_waited,0)) direct,\n"
			   "               SUM(DECODE(SUBSTR(event,1,3),'log',time_waited,0)) log,\n"
			   "               SUM(DECODE(SUBSTR(event,1,7),'SQL*Net',time_waited,0))-SUM(DECODE(event,'SQL*Net message from client',time_waited,0)) net\n"
			   "          from v$system_event),\n"
			   "       (select value*10 cpu from v$sysstat where statistic# = 12)",
			   "Used to generate chart for system wait time.");

static toSQL SQLChartsExecution("toTuning:Charts:6LNExecution",
				"select SYSDATE,\n"
				"       sum(decode(statistic#,181,value,0)) \"Execute\",\n"
				"       sum(decode(statistic#,180,value,0)) \"Hard parse\",\n"
				"       sum(decode(statistic#,179,value,0)) \"Parse\",\n"
				"       sum(decode(statistic#,6,value,0)) \"Calls\",\n"
				"       sum(decode(statistic#,4,value,0))  \"Commit\",\n"
				"       sum(decode(statistic#,5,value,0)) \"Rollbacks\"\n"
				"  from v$sysstat where statistic# in (181,180,179,4,5,6)",
				"Used to generate chart for execution statistics.",
				"8.0");

static toSQL SQLChartsExecution9("toTuning:Charts:6LNExecution",
				 "select SYSDATE,\n"
				 "       sum(decode(statistic#,222,value,0)) \"Execute\",\n"
				 "       sum(decode(statistic#,220,value,0)) \"Hard parse\",\n"
				 "       sum(decode(statistic#,219,value,0)) \"Parse\",\n"
				 "       sum(decode(statistic#,6,value,0)) \"Calls\",\n"
				 "       sum(decode(statistic#,4,value,0))  \"Commit\",\n"
				 "       sum(decode(statistic#,5,value,0)) \"Rollbacks\"\n"
				 "  from v$sysstat where statistic# in (222,220,219,4,5,6)",
				 "",
				 "9.0");

static toSQL SQLChartsExecution7("toTuning:Charts:6LNExecution",
				 "select SYSDATE,\n"
				 "       sum(decode(statistic#,132,value,0)) \"Execute\",\n"
				 "       sum(decode(statistic#,131,value,0)) \"Parse\",\n"
				 "       sum(decode(statistic#,6,value,0)) \"Calls\",\n"
				 "       sum(decode(statistic#,4,value,0))  \"Commit\",\n"
				 "       sum(decode(statistic#,5,value,0)) \"Rollbacks\"\n"
				 "  from v$sysstat where statistic# in (132,131,4,5,6)",
				 "",
				 "7.3");

static toSQL SQLChartsExecutionPie("toTuning:Charts:8PNExecution Total",
				   "select value,decode(statistic#,181,'Execute',180,'Hard parse',179,'Parse',\n"
				   "                              6,'Calls',4,'Commit',5,'Rollbacks')\n"
				   "  from v$sysstat where statistic# in (180,179,181,6,4,5) order by name",
				   "Used to generate pie chart for execution statistics.",
				   "8.0");

static toSQL SQLChartsExecutionPie9("toTuning:Charts:8PNExecution Total",
				    "select value,decode(statistic#,222,'Execute',220,'Hard parse',219,'Parse',\n"
				    "                              6,'Calls',4,'Commit',5,'Rollbacks')\n"
				    "  from v$sysstat where statistic# in (219,220,222,6,4,5) order by name",
				    "",
				    "9.0");

static toSQL SQLChartsExecutionPie7("toTuning:Charts:8PNExecution Total",
				    "select value,decode(statistic#,132,'Execute',131,'Parse',\n"
				    "                              6,'Calls',4,'Commit',5,'Rollbacks')\n"
				    "  from v$sysstat where statistic# in (132,131,6,4,5) order by name",
				    "",
				    "7.3");

static toSQL SQLChartsClients("toTuning:Charts:4BAClients",
			      "select sysdate,\n"
			      "       sum(decode(decode(type,'BACKGROUND','WHATEVER',status),'ACTIVE',1,0)) \"Active\",\n"
			      "       sum(decode(status,'INACTIVE',1,0)) \"Inactive\",\n"
			      "       sum(decode(type,'BACKGROUND',1,0)) \"System\"\n"
			      "  from v$session where sid not in (select nvl(sid,0) from v$px_process)",
			      "Chart displaying connected clients");

static toSQL SQLChartsClients8("toTuning:Charts:4BAClients",
			       "select sysdate,\n"
			       "       sum(decode(decode(type,'BACKGROUND','WHATEVER',status),'ACTIVE',1,0)) \"Active\",\n"
			       "       sum(decode(status,'INACTIVE',1,0)) \"Inactive\",\n"
			       "       sum(decode(type,'BACKGROUND',1,0)) \"System\"\n"
			       "  from v$session",
			       "",
			       "8.0");

static toSQL SQLChartsCacheMisses("toTuning:Charts:5CPCache misses",
				  "select sysdate,\n"
				  "       pread \"Data buffer cache\",read \"Dictionary row cache\",\n"
				  "       getmiss \"Library cache\",gets \" \",\n"
				  "       reloads \" \",pins \" \"\n"
				  "  from (select 100*SUM(DECODE(statistic#,40,value,0)) pread,SUM(DECODE(statistic#,38,value,39,value,0)) read\n"
				  "          from v$sysstat where statistic# IN (38,39,40)) \"Data buffer cache\",\n"
				  "       (select 100*sum(getmisses) getmiss,sum(gets) gets from v$rowcache) \"Dictionary row cache\",\n"
				  "       (select 100*sum(reloads) reloads,sum(pins) pins from v$librarycache) \"Library cache\"\n",
				  "Chart display memory allocation. This is really weird to change, especially since the column names "
				  "don't correspond exactly to the column data in the chart. Each group of two are divided with each other before drawn.",
				  "8.0");

static toSQL SQLChartsCacheMisses9("toTuning:Charts:5CPCache misses",
				   "select sysdate,\n"
				   "       pread \"Data buffer cache\",read \"Dictionary row cache\",\n"
				   "       getmiss \"Library cache\",gets \" \",\n"
				   "       reloads \" \",pins \" \"\n"
				   "  from (select 100*SUM(DECODE(statistic#,42,value,0)) pread,SUM(DECODE(statistic#,40,value,41,value,0)) read\n"
				   "          from v$sysstat where statistic# IN (40,41,42)) \"Data buffer cache\",\n"
				   "       (select 100*sum(getmisses) getmiss,sum(gets) gets from v$rowcache) \"Dictionary row cache\",\n"
				   "       (select 100*sum(reloads) reloads,sum(pins) pins from v$librarycache) \"Library cache\"\n",
				   "",
				   "9.0");

static toSQL SQLChartsCacheMisses7("toTuning:Charts:5CPCache misses",
				   "select sysdate,\n"
				   "       pread \"Data buffer cache\",read \"Dictionary row cache\",\n"
				   "       getmiss \"Library cache\",gets \" \",\n"
				   "       reloads \" \",pins \" \"\n"
				   "  from (select 100*SUM(DECODE(statistic#,39,value,0)) pread,SUM(DECODE(statistic#,37,value,38,value,0)) read\n"
				   "          from v$sysstat where statistic# IN (37,38,39)) \"Data buffer cache\",\n"
				   "       (select 100*sum(getmisses) getmiss,sum(gets) gets from v$rowcache) \"Dictionary row cache\",\n"
				   "       (select 100*sum(reloads) reloads,sum(pins) pins from v$librarycache) \"Library cache\"\n",
				   "",
				   "7.3");

static toSQL SQLChartsRedo("toTuning:Charts:7BSRedo log I/O",
			   "select SYSDATE,\n"
			   "       sum(decode(statistic#,101,value,0))/:unit<int> \"Redo size\",\n"
			   "       sum(decode(statistic#,103,value,0))/:unit<int> \"Redo wastage\"\n"
			   "  from v$sysstat where statistic# in (101,103)",
			   "Used to generate chart for redo I/O statistics.",
			   "8.0");

static toSQL SQLChartsRedo9("toTuning:Charts:7BSRedo log I/O",
			    "select SYSDATE,\n"
			    "       sum(decode(statistic#,115,value,0))/:unit<int> \"Redo size\",\n"
			    "       sum(decode(statistic#,117,value,0))/:unit<int> \"Redo wastage\"\n"
			    "  from v$sysstat where statistic# in (115,117)",
			    "",
			    "9.0");

static toSQL SQLChartsRedo7("toTuning:Charts:7BSRedo log I/O",
			    "select SYSDATE,\n"
			    "       sum(decode(statistic#,85,value,0))/:unit<int> \"Redo size\",\n"
			    "       sum(decode(statistic#,89,value,0))/:unit<int> \"Redo wastage\"\n"
			    "  from v$sysstat where statistic# in (85,89)",
			    "",
			    "7.3");

static toSQL SQLOverviewArchiveWrite("toTuning:Overview:ArchiveWrite",
				     "select sysdate,sum(blocks) from v$archived_log",
				     "Archive log write",
				     "8.0");

static toSQL SQLOverviewArchiveWrite7("toTuning:Overview:ArchiveWrite",
				      "select sysdate,0 from sys.dual",
				      "",
				      "7.3");

static toSQL SQLOverviewBufferHit("toTuning:Overview:BufferHit",
				  "SELECT SYSDATE,(1-SUM(DECODE(statistic#,40,value,0))/SUM(DECODE(statistic#,38,value,39,value,0)))*100\n"
				  "  FROM v$sysstat\n"
				  " WHERE statistic# IN (38,39,40)",
				  "Buffer hitrate",
				  "8.0");

static toSQL SQLOverviewBufferHit9("toTuning:Overview:BufferHit",
				   "SELECT SYSDATE,(1-SUM(DECODE(statistic#,42,value,0))/SUM(DECODE(statistic#,40,value,41,value,0)))*100\n"
				   "  FROM v$sysstat\n"
				   " WHERE statistic# IN (40,41,42)",
				   "",
				   "9.0");

static toSQL SQLOverviewBufferHit7("toTuning:Overview:BufferHit",
				   "SELECT SYSDATE,(1-SUM(DECODE(statistic#,39,value,0))/SUM(DECODE(statistic#,37,value,38,value,0)))*100\n"
				   "  FROM v$sysstat\n"
				   " WHERE statistic# IN (37,38,39)",
				   "",
				   "7.3");

static toSQL SQLOverviewClientInput("toTuning:Overview:ClientInput",
				    "select sysdate,value/:f1<int>\n"
				    "  from v$sysstat where statistic# = 182",
				    "Bytes sent to client",
				    "8.0");

static toSQL SQLOverviewClientInput9("toTuning:Overview:ClientInput",
				     "select sysdate,value/:f1<int>\n"
				     "  from v$sysstat where statistic# = 223",
				     "",
				     "9.0");

static toSQL SQLOverviewClientInput7("toTuning:Overview:ClientInput",
				     "select sysdate,value/:f1<int>\n"
				     "  from v$sysstat where statistic# = 134",
				     "",
				     "7.3");

static toSQL SQLOverviewClientOutput("toTuning:Overview:ClientOutput",
				     "select sysdate,value/:f1<int>\n"
				     "  from v$sysstat where statistic# = 183",
				     "Bytes sent from client",
				     "8.0");

static toSQL SQLOverviewClientOutput9("toTuning:Overview:ClientOutput",
				      "select sysdate,value/:f1<int>\n"
				      "  from v$sysstat where statistic# = 224",
				      "",
				      "9.0");

static toSQL SQLOverviewClientOutput7("toTuning:Overview:ClientOutput",
				      "select sysdate,value/:f1<int>\n"
				      "  from v$sysstat where statistic# = 133",
				      "",
				      "7.3");

static toSQL SQLOverviewExecute("toTuning:Overview:Execute",
				"select sysdate,value\n"
				"  from v$sysstat where statistic# = 181",
				"Execute count",
				"8.0");

static toSQL SQLOverviewExecute9("toTuning:Overview:Execute",
				 "select sysdate,value\n"
				 "  from v$sysstat where statistic# = 222",
				 "",
				 "9.0");

static toSQL SQLOverviewExecute7("toTuning:Overview:Execute",
				 "select sysdate,value\n"
				 "  from v$sysstat where statistic# = 132",
				 "",
				 "7.3");

static toSQL SQLOverviewParse("toTuning:Overview:Parse",
			      "select sysdate,value\n"
			      "  from v$sysstat where statistic# = 179",
			      "Parse count",
			      "8.0");

static toSQL SQLOverviewParse9("toTuning:Overview:Parse",
			       "select sysdate,value\n"
			       "  from v$sysstat where statistic# = 219",
			       "",
			       "9.0");

static toSQL SQLOverviewParse7("toTuning:Overview:Parse",
			       "select sysdate,value\n"
			       "  from v$sysstat where statistic# = 131",
			       "",
			       "7.3");

static toSQL SQLOverviewRedoEntries("toTuning:Overview:RedoEntries",
				    "select sysdate,value\n"
				    "  from v$sysstat where statistic# = 100",
				    "Redo entries",
				    "8.0");

static toSQL SQLOverviewRedoEntries9("toTuning:Overview:RedoEntries",
				     "select sysdate,value\n"
				     "  from v$sysstat where statistic# = 114",
				     "",
				     "9.0");

static toSQL SQLOverviewRedoEntries7("toTuning:Overview:RedoEntries",
				     "select sysdate,value\n"
				     "  from v$sysstat where statistic# = 84",
				     "",
				     "7.3");

static toSQL SQLOverviewRedoBlocks("toTuning:Overview:RedoBlocks",
				   "select sysdate,value\n"
				   "  from v$sysstat where statistic# = 106",
				   "Redo blocks written",
				   "8.0");

static toSQL SQLOverviewRedoBlocks9("toTuning:Overview:RedoBlocks",
				    "select sysdate,value\n"
				    "  from v$sysstat where statistic# = 120",
				    "",
				    "9.0");

static toSQL SQLOverviewRedoBlocks7("toTuning:Overview:RedoBlocks",
				    "select sysdate,value\n"
				    "  from v$sysstat where statistic# = 92",
				    "",
				    "7.3");

static toSQL SQLOverviewLogicalRead("toTuning:Overview:LogicalRead",
				    "select sysdate,sum(value)\n"
				    "  from v$sysstat where statistic# in (38,39)",
				    "Blocks read",
				    "8.0");

static toSQL SQLOverviewLogicalRead9("toTuning:Overview:LogicalRead",
				     "select sysdate,sum(value)\n"
				     "  from v$sysstat where statistic# in (40,41)",
				     "",
				     "9.0");

static toSQL SQLOverviewLogicalRead7("toTuning:Overview:LogicalRead",
				     "select sysdate,sum(value)\n"
				     "  from v$sysstat where statistic# in (37,38)",
				     "",
				     "7.3");

static toSQL SQLOverviewLogicalWrite("toTuning:Overview:LogicalWrite",
				     "select sysdate,sum(value)\n"
				     "  from v$sysstat where statistic# in (41,42)",
				     "Blocks written",
				     "8.0");

static toSQL SQLOverviewLogicalWrite9("toTuning:Overview:LogicalWrite",
				      "select sysdate,sum(value)\n"
				      "  from v$sysstat where statistic# in (43,44)",
				      "",
				      "9.0");

static toSQL SQLOverviewLogicalWrite7("toTuning:Overview:LogicalWrite",
				      "select sysdate,sum(value)\n"
				      "  from v$sysstat where statistic# in (43,45)",
				      "",
				      "7.3");

static toSQL SQLOverviewPhysicalRead("toTuning:Overview:PhysicalRead",
				     "select sysdate,value\n"
				     "  from v$sysstat where statistic# = 40",
				     "Blocks physically read",
				     "8.0");

static toSQL SQLOverviewPhysicalRead9("toTuning:Overview:PhysicalRead",
				      "select sysdate,value\n"
				      "  from v$sysstat where statistic# = 42",
				      "",
				      "9.0");

static toSQL SQLOverviewPhysicalRead7("toTuning:Overview:PhysicalRead",
				      "select sysdate,value\n"
				      "  from v$sysstat where statistic# = 39",
				      "",
				      "7.3");

static toSQL SQLOverviewPhysicalWrite("toTuning:Overview:PhysicalWrite",
				      "select sysdate,value\n"
				      "  from v$sysstat where statistic# = 44",
				      "Blocks physically written",
				      "8.0");

static toSQL SQLOverviewPhysicalWrite9("toTuning:Overview:PhysicalWrite",
				       "select sysdate,value\n"
				       "  from v$sysstat where statistic# = 46",
				       "",
				       "9.0");

static toSQL SQLOverviewPhysicalWrite7("toTuning:Overview:PhysicalWrite",
				       "select sysdate,value\n"
				       "  from v$sysstat where statistic# = 40",
				       "",
				       "7.3");

static toSQL SQLOverviewClient("toTuning:Overview:Client",
			       "select sysdate,\n"
			       "       sum(decode(status,'INACTIVE',1,0)),\n"
			       "       sum(decode(status,'ACTIVE',1,0))\n"
			       "  from v$session\n"
			       " where type != 'BACKGROUND' and sid not in (select nvl(sid,0) from v$px_process)",
			       "Information about active/inactive clients");

static toSQL SQLOverviewClient8("toTuning:Overview:Client",
				"select sysdate,\n"
				"       sum(decode(status,'INACTIVE',1,0)),\n"
				"       sum(decode(status,'ACTIVE',1,0))\n"
				"  from v$session\n"
				" where type != 'BACKGROUND'",
				"",
				"8.0");

static toSQL SQLOverviewSGAUsed("toTuning:Overview:SGAUsed",
				"select sysdate,100*(total-free)/total\n"
				"  from (select sum(value) total from v$sga where name in ('Fixed Size','Variable Size')),\n"
				"       (select bytes free from v$sgastat where pool = 'shared pool' and name = 'free memory')",
				"SGA used",
				"8.0");

static toSQL SQLOverviewSGAUsed7("toTuning:Overview:SGAUsed",
				 "select sysdate,100*(total-free)/total\n"
				 "  from (select sum(value) total from v$sga where name in ('Fixed Size','Variable Size')),\n"
				 "       (select bytes free from v$sgastat where name = 'free memory')",
				 "",
				 "7.3");

static toSQL SQLOverviewTimescale("toTuning:Overview:Timescale",
				  "select sysdate,0 from sys.dual",
				  "Get timescale of other graphs");

static toSQL SQLOverviewFilespace("toTuning:Overview:Filespace",
				  "select sum(bytes)/:f1<int>,'Free'\n"
				  "  from sys.dba_free_space\n"
				  "union\n"
				  "select (total-free)/:f1<int>,'Used'\n"
				  "  from (select sum(bytes) free from sys.dba_free_space),\n"
				  "       (select sum(bytes) total from sys.dba_data_files)",
				  "Filespace used");

void toTuningOverview::setupChart(toResultLine *chart,const QString &title,const QString &postfix,const toSQL &sql)
{
  chart->setMinValue(0);
  chart->showGrid(0);
  chart->showLegend(false);
  chart->showAxisLegend(false);
  chart->setTitle(title);
  chart->showLast(true);
  toQList val;
  if (postfix==QString::fromLatin1("b/s")) {
    QString unitStr=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
    val.insert(val.end(),toQValue(toSizeDecode(unitStr)));
    unitStr+=QString::fromLatin1("/s");
    chart->setYPostfix(unitStr);
  } else
    chart->setYPostfix(postfix);
  chart->query(sql,val);
}

toTuningOverview::toTuningOverview(QWidget *parent,const char *name,WFlags fl)
  : toTuningOverviewUI(parent,name,fl)
{
  BackgroundGroup->setColumnLayout(1,Horizontal);

  setupChart(ArchiveWrite,tr("< Archive write")," "+tr("blocks/s"),SQLOverviewArchiveWrite);
  setupChart(BufferHit,tr("Hitrate"),QString::fromLatin1("%"),SQLOverviewBufferHit);
  BufferHit->setMaxValue(100);
  BufferHit->setFlow(false);
  setupChart(ClientInput,tr("< Client input"),QString::fromLatin1("b/s"),SQLOverviewClientInput);
  setupChart(ClientOutput,tr("Client output >"),QString::fromLatin1("b/s"),SQLOverviewClientOutput);
  setupChart(ExecuteCount,tr("Executes >"),QString::fromLatin1("/s"),SQLOverviewExecute);
  setupChart(LogWrite,tr("Log writer >")," "+tr("blocks/s"),SQLOverviewRedoBlocks);
  setupChart(LogicalChange,tr("Buffer changed >"),tr(" blocks/s"),SQLOverviewLogicalWrite);
  setupChart(LogicalRead,tr("< Buffer gets"),tr(" blocks/s"),SQLOverviewLogicalRead);
  setupChart(ParseCount,tr("Parse >"),QString::fromLatin1("/s"),SQLOverviewParse);
  setupChart(PhysicalRead,tr("< Physical read"),tr(" blocks/s"),SQLOverviewPhysicalRead);
  setupChart(PhysicalWrite,tr("Physical write >"),tr(" blocks/s"),SQLOverviewPhysicalWrite);
  setupChart(RedoEntries,tr("Redo entries >"),QString::fromLatin1("/s"),SQLOverviewRedoEntries);
  setupChart(Timescale,tr("Timescale"),QString::null,SQLOverviewTimescale);
  Timescale->showAxisLegend(true);
  Timescale->showLast(false);

  ClientChart->showGrid(0);
  ClientChart->showLegend(false);
  ClientChart->showAxisLegend(false);
  ClientChart->query(SQLOverviewClient);
  ClientChart->setFlow(false);

  SharedUsed->showGrid(0);
  SharedUsed->showLegend(false);
  SharedUsed->showAxisLegend(false);
  SharedUsed->query(SQLOverviewSGAUsed);
  SharedUsed->setFlow(false);
  SharedUsed->setMaxValue(100);
  SharedUsed->setYPostfix(QString::fromLatin1("%"));
  SharedUsed->showLast(true);

  try {
    toQList val;
    val.insert(val.end(),
	       toQValue(toSizeDecode(toTool::globalConfig(CONF_SIZE_UNIT,
							  DEFAULT_SIZE_UNIT))));
    FileUsed->query(toSQL::string(SQLOverviewFilespace,toCurrentConnection(this)),(const toQList)val);
  } TOCATCH
  FileUsed->showLegend(false);

  Done.up();
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));

  // Will be called later anyway
  //refresh();
}

toTuningOverview::~toTuningOverview()
{
    if (Done.getValue()==0) {
      Quit=true;
      Done.down();
    }
}

void toTuningOverview::stop(void)
{
  try {
    disconnect(toCurrentTool(this)->timer(),SIGNAL(timeout()),this,SLOT(refresh()));
  } TOCATCH

  ArchiveWrite->stop();
  BufferHit->stop();
  ClientInput->stop();
  ClientOutput->stop();
  ExecuteCount->stop();
  LogWrite->stop();
  LogicalChange->stop();
  LogicalRead->stop();
  ParseCount->stop();
  PhysicalRead->stop();
  PhysicalWrite->stop();
  RedoEntries->stop();
  Timescale->stop();
  ClientChart->stop();
  SharedUsed->stop();
  FileUsed->stop();
}

void toTuningOverview::start(void)
{
  try {
    connect(toCurrentTool(this)->timer(),SIGNAL(timeout()),this,SLOT(refresh()));
  } TOCATCH

  ArchiveWrite->start();
  BufferHit->start();
  ClientInput->start();
  ClientOutput->start();
  ExecuteCount->start();
  LogWrite->start();
  LogicalChange->start();
  LogicalRead->start();
  ParseCount->start();
  PhysicalRead->start();
  PhysicalWrite->start();
  RedoEntries->start();
  Timescale->start();
  ClientChart->start();
  SharedUsed->start();
  FileUsed->start();
}

static toSQL SQLOverviewArchive("toTuning:Overview:Archive",
				"select count(1),\n"
				"       nvl(sum(blocks*block_size),0)/:f1<int>\n"
				"  from v$archived_log where deleted = 'NO'",
				"Information about archive logs",
				"8.0");

static toSQL SQLOverviewArchive7("toTuning:Overview:Archive",
				 "select 'N/A',\n"
				 "       'N/A'\n"
				 "  from sys.dual where 0 != :f1<int>",
				 "",
				 "7.3");

static toSQL SQLOverviewLog("toTuning:Overview:Log",
			    "select count(1),\n"
			    "       max(decode(status,'CURRENT',group#,0)),\n"
			    "       sum(decode(status,'CURRENT',bytes,0))/:f1<int>,\n"
			    "       sum(bytes)/:f1<int> from v$log\n",
			    "Information about redo logs");

static toSQL SQLOverviewTablespaces("toTuning:Overview:Tablespaces",
				    "select count(1) from v$tablespace",
				    "Number of tablespaces",
				    "8.0");

static toSQL SQLOverviewTablespaces7("toTuning:Overview:Tablespaces",
				    "select count(1) from sys.dba_tablespaces",
				    "",
				     "7.3");

static toSQL SQLOverviewSGA("toTuning:Overview:SGA",
			    "select name,value/:f1<int> from v$sga",
			    "Information about SGA");

static toSQL SQLOverviewBackground("toTuning:Overview:Background",
				   "select substr(name,1,3),count(1) from v$bgprocess where paddr != '00'\n"
				   " group by substr(name,1,3) order by substr(name,1,3)",
				   "Background processes");

static toSQL SQLOverviewDedicated("toTuning:Overview:Dedicated",
				  "select count(1) from v$session\n"
				  " where type = 'USER' and server = 'DEDICATED'\n"
				  "   and sid not in (select nvl(sid,0) from v$px_process)",
				  "Dedicated server process",
				  "8.1");

static toSQL SQLOverviewDedicated7("toTuning:Overview:Dedicated",
				   "select count(1) from v$session where type = 'USER' and server = 'DEDICATED'",
				   "",
				   "8.0");

static toSQL SQLOverviewDispatcher("toTuning:Overview:Dispatcher",
				   "select count(1) from v$dispatcher",
				   "Dispatcher processes");

static toSQL SQLOverviewParallell("toTuning:Overview:Parallel",
				  "select count(1) from v$px_process",
				  "Parallel processes",
				  "8.1");

static toSQL SQLOverviewParallell8("toTuning:Overview:Parallel",
				   "select 'N/A' from sys.dual",
				   "",
				   "8.0");

static toSQL SQLOverviewShared("toTuning:Overview:Shared",
			       "select count(1) from v$shared_server",
			       "Shared processes");

static toSQL SQLOverviewRound("toTuning:Overview:Roundtime",
			      "select round(average_wait,2) from v$system_event\n"
			      " where event in ('SQL*Net message from client',\n"
			      "                 'SQL*Net message to client') order by event",
			      "Client roundtime info");

static toSQL SQLOverviewClientTotal("toTuning:Overview:ClientTotal",
				    "select count(1),\n"
				    "       sum(decode(status,'ACTIVE',1,0))\n"
				    "  from v$session\n"
				    " where type != 'BACKGROUND' and sid not in (select nvl(sid,0) from v$px_process)",
				    "Information about total and active clients",
				    "8.1");

static toSQL SQLOverviewClientTotal8("toTuning:Overview:ClientTotal",
				     "select count(1),\n"
				     "       sum(decode(status,'ACTIVE',1,0))\n"
				     "  from v$session\n"
				     " where type != 'BACKGROUND'",
				     "",
				     "8.0");

static toSQL SQLOverviewDatafiles("toTuning:Overview:Datafiles",
				  "select (select count(1) from v$datafile)+(select count(1) from v$tempfile) from dual",
				  "Number of datafiles",
				  "8.0");

static toSQL SQLOverviewDatafiles8("toTuning:Overview:Datafiles",
				  "select count(1) from v$datafile",
				  "",
				  "8.0");

static toSQL SQLOverviewDatafiles7("toTuning:Overview:Datafiles",
				   "select count(1) from sys.dba_tablespaces",
				   "",
				   "7.3");

void toTuningOverview::overviewQuery::setValue(const QCString &nam,const QString &val)
{
  if (Parent.Quit)
    throw 1;
  toLocker lock(Parent.Lock);
  Parent.Values[nam]=val;
}

void toTuningOverview::overviewQuery::run(void)
{
  try {
    toQList val;
    val.insert(val.end(),toQValue(toSizeDecode(Parent.UnitString)));

    toQList res=toQuery::readQuery(*Parent.Connection,SQLOverviewArchive,val);
    QString tmp=toShift(res);
    tmp+=QString::fromLatin1("/");
    tmp+=toShift(res);
    tmp+=Parent.UnitString;
    setValue("ArchiveInfo",tmp);

    res=toQuery::readQuery(*Parent.Connection,SQLOverviewRound);
    tmp=toShift(res);
    tmp+=QString::fromLatin1(" ms");
    setValue("SendFromClient",tmp);
    tmp=toShift(res);
    tmp+=QString::fromLatin1(" ms");
    setValue("SendToClient",tmp);

    res=toQuery::readQuery(*Parent.Connection,SQLOverviewClientTotal);
    tmp=toShift(res);
    setValue("TotalClient",tmp);
    tmp=toShift(res);
    setValue("ActiveClient",tmp);

    int totJob=0;
    res=toQuery::readQuery(*Parent.Connection,SQLOverviewDedicated);
    tmp=toShift(res);
    totJob+=tmp.toInt();
    setValue("DedicatedServer",tmp);

    res=toQuery::readQuery(*Parent.Connection,SQLOverviewDispatcher);
    tmp=toShift(res);
    totJob+=tmp.toInt();
    setValue("DispatcherServer",tmp);

    res=toQuery::readQuery(*Parent.Connection,SQLOverviewShared);
    tmp=toShift(res);
    totJob+=tmp.toInt();
    setValue("SharedServer",tmp);

    res=toQuery::readQuery(*Parent.Connection,SQLOverviewParallell);
    tmp=toShift(res);
    totJob+=tmp.toInt();
    setValue("ParallellServer",tmp);

    res=toQuery::readQuery(*Parent.Connection,SQLOverviewBackground);
    QStringList back;
    while(res.size()>0) {
      tmp=toShift(res);
      if(tmp==QString::fromLatin1("DBW"))
	tmp=QString::fromLatin1("DBWR");
      else if (tmp==QString::fromLatin1("PMO"))
	tmp=QString::fromLatin1("PMON");
      else if (tmp==QString::fromLatin1("ARC"))
	tmp=QString::fromLatin1("ARCH");
      else if (tmp==QString::fromLatin1("CKP"))
	tmp=QString::fromLatin1("CKPT");
      else if (tmp==QString::fromLatin1("LGW"))
	tmp=QString::fromLatin1("LGWR");
      else if (tmp==QString::fromLatin1("LMO"))
	tmp=QString::fromLatin1("LMON");
      else if (tmp==QString::fromLatin1("REC"))
	tmp=QString::fromLatin1("RECO");
      else if (tmp==QString::fromLatin1("TRW"))
	tmp=QString::fromLatin1("TRWR");
      else if (tmp==QString::fromLatin1("SMO"))
	tmp=QString::fromLatin1("SMON");

      tmp+=QString::fromLatin1(": <B>");
      QString job=toShift(res);
      totJob+=job.toInt();
      tmp+=job;
      tmp+=QString::fromLatin1("</B>");
      back<<tmp;
    }
    setValue("Background",back.join(QString::fromLatin1(",")));
    setValue("TotalProcess",QString::number(totJob));

    double tot=0;
    double sql=0;
    res=toQuery::readQuery(*Parent.Connection,SQLOverviewSGA,val);
    while(res.size()>0) {
      QString nam=toShift(res);
      tmp=toShift(res);
      if (nam=="Database Buffers"||nam=="Redo Buffers")
	setValue(nam.latin1(),tmp);
      else if (nam=="Fixed Size"||nam=="Variable Size")
	sql+=tmp.toDouble();
      tot+=tmp.toDouble();
    }
    tmp=toQValue::formatNumber(tot);
    tmp+=Parent.UnitString;
    setValue("SGATotal",tmp);
    tmp=toQValue::formatNumber(sql);
    tmp+=Parent.UnitString;
    setValue("SharedSize",tmp);

    res=toQuery::readQuery(*Parent.Connection,SQLOverviewLog,val);
    setValue("RedoFiles",toShift(res));
    setValue("ActiveRedo",toShift(res));
    tmp=toShift(res);
    tmp+=QString::fromLatin1("/");
    tmp+=toShift(res);
    tmp+=Parent.UnitString;
    setValue("RedoSize",tmp);

    res=toQuery::readQuery(*Parent.Connection,SQLOverviewTablespaces);
    setValue("Tablespaces",toShift(res));

    res=toQuery::readQuery(*Parent.Connection,SQLOverviewDatafiles);
    setValue("Files",toShift(res));
  } catch(const QString &str) {
    fprintf(stderr,"Exception occured:\n\n%s\n",(const char *)str.latin1());
  } catch(int) {
  }
  Parent.Done.up();
}

void toTuningOverview::refresh(void)
{
  try {
    if (Done.getValue()==1) {
      Done.down();
      Quit=false;
      Connection=&toCurrentConnection(this);
      UnitString=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
      toThread *thread=new toThread(new overviewQuery(*this));
      thread->start();
      Poll.start(500);
    }
  } TOCATCH
}

void toTuningOverview::setValue(QLabel *label,const QCString &nam)
{
  toLocker lock(Lock);
  std::map<QCString,QString>::iterator i=Values.find(nam);
  if (i!=Values.end()) {
    label->setText((*i).second);
    Values.erase(i);
  }
}

void toTuningOverview::poll(void)
{
  try {
    setValue(ArchiveInfo,"ArchiveInfo");
    setValue(SendFromClient,"SendFromClient");
    setValue(SendToClient,"SendToClient");
    setValue(TotalClient,"TotalClient");
    setValue(ActiveClient,"ActiveClient");
    setValue(DedicatedServer,"DedicatedServer");
    setValue(DispatcherServer,"DispatcherServer");
    setValue(SharedServer,"SharedServer");
    setValue(ParallellServer,"ParallellServer");

    {
      toLocker lock(Lock);
      std::map<QCString,QString>::iterator i=Values.find("Background");
      if (i!=Values.end()) {
	std::list<QLabel *>::iterator labIt=Backgrounds.begin();

	QStringList lst=QStringList::split(QString::fromLatin1(","),(*i).second);
	for(unsigned int j=0;j<lst.count();j++) {
	  QLabel *label;
	  if (labIt==Backgrounds.end()||*labIt==NULL) {
	    label=new QLabel(BackgroundGroup);
	    label->show();
	    if (labIt==Backgrounds.end()) {
	      Backgrounds.insert(Backgrounds.end(),label);
	      labIt=Backgrounds.end();
	    } else {
	      *labIt=label;
	      labIt++;
	    }
	  } else {
	    label=*labIt;
	    labIt++;
	  }
	  label->setText(lst[j]);
	}
	while(labIt!=Backgrounds.end()) {
	  delete *labIt;
	  *labIt=NULL;
	  labIt++;
	}
	Values.erase(i);
      }
    }

    setValue(TotalProcess,"TotalProcess");
    setValue(BufferSize,"Database Buffers");
    setValue(RedoBuffer,"Redo Buffers");
    setValue(SGATotal,"SGATotal");
    setValue(SharedSize,"SharedSize");
    setValue(RedoFiles,"RedoFiles");
    setValue(ActiveRedo,"ActiveRedo");
    setValue(RedoSize,"RedoSize");

    std::list<double> &values=FileUsed->values();
    std::list<double>::iterator j=values.begin();
    if (j!=values.end()) {
      double size=0;
      double used=0;
      if (j!=values.end())
	used=size=(*j);
      j++;
      if (j!=values.end())
	size+=(*j);
      QString tmp=toQValue::formatNumber(used);
      tmp+=QString::fromLatin1("/");
      tmp+=toQValue::formatNumber(size);
      tmp+=UnitString;
      if (tmp!=Filesize->text())
	Filesize->setText(tmp);
    }

    setValue(Tablespaces,"Tablespaces");
    setValue(Files,"Files");
    if (Done.getValue()==1)
      Poll.stop();
  } TOCATCH
}

static toSQL SQLLibraryCache("toTuning:LibraryCache",
			     "SELECT namespace,\n"
			     "       gets \"Gets\",\n"
			     "       gethits \"Get Hits\",\n"
			     "       to_char(trunc(gethitratio*100,\n"
			     "		     1))||'%' \"-Ratio\",\n"
			     "       pins \"Pins\",\n"
			     "       pinhits \"Pin Hits\",\n"
			     "       to_char(trunc(pinhitratio*100,\n"
			     "		     1))||'%' \"-Ratio\"\n"
			     "  FROM v$librarycache",
			     "Library cache view");

static toSQL SQLControlFiles("toTuning:ControlFileRecords",
			     "SELECT type \"Type\",\n"
			     "       record_size \"Size\",\n"
			     "       records_total \"Total Records\",\n"
			     "       records_used \"Used Records\",\n"
			     "       round(record_size*records_total/:unt<int>,1)||' '||:unitstr<char[101]> \"-Allocated\",\n"
			     "       to_char(round(records_used/records_total*100,1))||'%' \"-Used\",\n"
			     "       type \" Sort\"\n"
			     "  FROM v$controlfile_record_section\n"
			     "UNION\n"
			     "SELECT 'TOTAL',\n"
			     "        round(avg(record_size),1),\n"
			     "        sum(records_total),\n"
			     "        sum(records_used),\n"
			     "        round(sum(record_size*records_total/:unt<int>),1)||' '||:unitstr<char[101]>,\n"
			     "        to_char(round(sum(record_size*records_used)/sum(record_size*records_total)*100,1))||'%' \"-Used\",\n"
			     "        'ZZZZZZZZZZZZZ'\n"
			     "  FROM v$controlfile_record_section\n"
			     "ORDER BY 7",
			     "Control file record info");

toTuning::toTuning(QWidget *main,toConnection &connection)
  : toToolWidget(TuningTool,"tuning.html",main,connection)
{
  if (TuningTool.config(CONF_OVERVIEW,"Undefined")=="Undefined") {
    QString def=QString::null;
    if (TOMessageBox::warning(toMainWidget(),
			      tr("Enable all tuning statistics"),
			      tr("Are you sure you want to enable all tuning features.\n"
				 "This can put heavy strain on a database and unless you\n"
				 "are the DBA you probably don't want this. Selecting\n"
				 "no here will give you the option to enable or disable\n"
				 "tabs individually as they are needed."),
			      tr("Yes"),tr("&No"),QString::null,1)==0) {
      def="Yes";
    }
    std::list<QCString> tabs=TabList();
    for(std::list<QCString>::iterator i=tabs.begin();i!=tabs.end();i++)
      TuningTool.setConfig(*i,def);
    toTool::saveConfig();
  }

  QToolBar *toolbar=toAllocBar(this,tr("Server Tuning"));

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  tr("Refresh"),
		  tr("Refresh"),
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QLabel(tr("Refresh")+" ",toolbar,TO_KDE_TOOLBAR_WIDGET);
  Refresh=toRefreshCreate(toolbar,TO_KDE_TOOLBAR_WIDGET);
  connect(Refresh,SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));
  toolbar->addSeparator();
  TabButton=new toPopupButton(QPixmap((const char **)compile_xpm),
			      tr("Enable and disable tuning tabs"),
			      tr("Enable and disable tuning tabs"),
			      toolbar);
  TabMenu=new QPopupMenu(TabButton);
  TabButton->setPopup(TabMenu);
  connect(TabMenu,SIGNAL(aboutToShow()),this,SLOT(showTabMenu()));
  connect(TabMenu,SIGNAL(activated(int)),this,SLOT(enableTabMenu(int)));

  toolbar->setStretchableWidget(new QLabel(toolbar,TO_KDE_TOOLBAR_WIDGET));
  new toChangeConnection(toolbar,TO_KDE_TOOLBAR_WIDGET);

  Tabs=new QTabWidget(this);

  Overview=new toTuningOverview(this,"overview");
  Tabs->addTab(Overview,tr("&Overview"));

  try {
    toRefreshParse(timer());
    connect(timer(),SIGNAL(timeout()),Overview,SLOT(refresh()));
  } TOCATCH

  QString unitStr=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
  toQList unit;
  unit.insert(unit.end(),toQValue(toSizeDecode(unitStr)));
  {
    std::list<QCString> val=toSQL::range("toTuning:Charts");
    for(std::list<QCString>::iterator i=val.begin();i!=val.end();i++) {
      QStringList parts=QStringList::split(QString::fromLatin1(":"),QString::fromLatin1(*i));
      if (parts.count()==3) {
	parts.append(parts[2]);
	parts[2]=QString::fromLatin1("Charts");
      }
      std::map<QCString,QGrid *>::iterator j=Charts.find(QCString(CONF_CHART)+parts[2].latin1());
      QGrid *cchart;
      if (j==Charts.end())
	Charts[QCString(CONF_CHART)+parts[2].latin1()]=cchart=new QGrid(2,Tabs,QCString(CONF_CHART)+parts[2].latin1());
      else
	cchart=(*j).second;

      if (parts[3].mid(1,1)==QString::fromLatin1("B")) {
	toResultBar *chart=new toResultBar(cchart);
	chart->setTitle(parts[3].mid(3));
	toQList par;
	if (parts[3].mid(2,1)==QString::fromLatin1("B"))
	  chart->setYPostfix(tr(" blocks/s"));
	else if (parts[3].mid(2,1)==QString::fromLatin1("M"))
	  chart->setYPostfix(QString::fromLatin1(" ms/s"));
	else if (parts[3].mid(2,1)==QString::fromLatin1("S")) {
	  par=unit;
	  QString t=unitStr;
	  t+=QString::fromLatin1("/s");
	  chart->setYPostfix(t);
	} else if (parts[3].mid(2,1)==QString::fromLatin1("A"))
	  chart->setFlow(false);
	else
	  chart->setYPostfix(QString::fromLatin1("/s"));
	chart->query(toSQL::sql(*i),par);
      } else if (parts[3].mid(1,1)==QString::fromLatin1("L")||parts[3].mid(1,1)==QString::fromLatin1("C")) {
	toResultLine *chart;
	if (parts[3].mid(1,1)==QString::fromLatin1("C"))
	  chart=new toTuningMiss(cchart);
	else
	  chart=new toResultLine(cchart);
	chart->setTitle(parts[3].mid(3));
	toQList par;
	if (parts[3].mid(2,1)==QString::fromLatin1("B"))
	  chart->setYPostfix(tr(" blocks/s"));
	else if (parts[3].mid(2,1)==QString::fromLatin1("S")) {
	  par=unit;
	  QString t=unitStr;
	  t+=QString::fromLatin1("/s");
	  chart->setYPostfix(t);
	} else if (parts[3].mid(2,1)==QString::fromLatin1("P")) {
	  chart->setYPostfix(QString::fromLatin1(" %"));
	  chart->setMinValue(0);
        } else
	  chart->setYPostfix(QString::fromLatin1("/s"));
	chart->query(toSQL::sql(*i),par);
      } else if (parts[3].mid(1,1)==QString::fromLatin1("P")) {
	toResultPie *chart=new toResultPie(cchart);
	chart->setTitle(parts[3].mid(3));
	if (parts[3].mid(2,1)==QString::fromLatin1("S")) {
	  chart->query(toSQL::sql(*i),unit);
	  chart->setPostfix(unitStr);
	} else
	  chart->query(toSQL::sql(*i));
      } else
	toStatusMessage(tr("Wrong format of name on chart (%1).").arg(*i));
    }
  }

  for (std::map<QCString,QGrid *>::iterator k=Charts.begin();k!=Charts.end();k++)
    Tabs->addTab((*k).second,tr((*k).first.mid(strlen(CONF_CHART))));

  FileIO=new toTuningFileIO(this,"fileio");
  connect(this,SIGNAL(connectionChange()),FileIO,SLOT(changeConnection()));

  Waits=new toWaitEvents(this,"waits");
  Tabs->addTab(Waits,tr("Wait events"));

  Tabs->addTab(FileIO,tr("&File I/O"));

  Indicators=new toListView(Tabs,"indicators");
  Indicators->setRootIsDecorated(true);
  Indicators->addColumn(tr("Indicator"));
  Indicators->addColumn(tr("Value"));
  Indicators->addColumn(tr("Reference"));
  Tabs->addTab(Indicators,tr("&Indicators"));

  Statistics=new toResultStats(Tabs,"stats");
  Tabs->addTab(Statistics,tr("&Statistics"));

  Parameters=new toResultParam(Tabs,"parameters");
  Tabs->addTab(Parameters,tr("&Parameters"));

  BlockingLocks=new toResultLock(Tabs,"locks");
  Tabs->addTab(BlockingLocks,tr("&Blocking locks"));

  LibraryCache=new toResultLong(true,false,toQuery::Background,Tabs,"cache");
  LibraryCache->setSQL(SQLLibraryCache);
  Tabs->addTab(LibraryCache,tr("Library C&ache"));

  ControlFiles=new toResultLong(true,false,toQuery::Background,Tabs,"control");
  ControlFiles->setSQL(SQLControlFiles);
  Tabs->addTab(ControlFiles,tr("Control Files"));

  Options=new toResultLong(true,false,toQuery::Background,Tabs,"options");
  Options->setSQL(SQLOptions);
  Tabs->addTab(Options,tr("Optio&ns"));

  Licenses=new toResultItem(2,true,Tabs,"licenses");
  Licenses->setSQL(SQLLicense);
  Tabs->addTab(Licenses,tr("&Licenses"));

  Tabs->setCurrentPage(0);

  LastTab=NULL;

  connect(Tabs,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeTab(QWidget *)));
  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  std::list<QCString> tabs=TabList();
  for(std::list<QCString>::iterator i=tabs.begin();i!=tabs.end();i++)
    if (TuningTool.config(*i,"").isEmpty())
      enableTab(*i,false);

  refresh();
  setFocusProxy(Tabs);
}

QWidget *toTuning::tabWidget(const QCString &name)
{
  QWidget *widget=NULL;
  if (name==CONF_OVERVIEW) {
    widget=Overview;
  } else if (name==CONF_FILEIO) {
    widget=FileIO;
  } else if (name==CONF_WAITS) {
    widget=Waits;
  } else if (Charts.find(CONF_CHART+name)!=Charts.end()) {
    widget=Charts[CONF_CHART+name];
  }
  return widget;
}

void toTuning::showTabMenu(void)
{
  int id=1;
  TabMenu->clear();
  std::list<QCString> tab=TabList();
  for(std::list<QCString>::iterator i=tab.begin();i!=tab.end();i++) {
    id++;
    TabMenu->insertItem(tr(*i),id);
    QWidget *widget=tabWidget(*i);
    if (widget&&Tabs->isTabEnabled(widget))
      TabMenu->setItemChecked(id,true);
  }
}

void toTuning::enableTabMenu(int selid)
{
  std::list<QCString> tab=TabList();
  int id=1;
  for(std::list<QCString>::iterator i=tab.begin();i!=tab.end();i++) {
    id++;
    if (selid==id) {
      QWidget *widget=tabWidget(*i);
      if (widget)
	enableTab(*i,!Tabs->isTabEnabled(widget));
      break;
    }
  }
}

void toTuning::enableTab(const QCString &name,bool enable)
{
  QWidget *widget=NULL;
  if (name==CONF_OVERVIEW) {
    if (enable)
      Overview->start();
    else
      Overview->stop();
    widget=Overview;
  } else if (Charts.find(QCString(CONF_CHART)+name)!=Charts.end()) {
    QGrid *chart=Charts[QCString(CONF_CHART)+name];
    QObjectList *childs=(QObjectList *)chart->children();
    for(unsigned int i=0;i<childs->count();i++) {
      toResultLine *line=dynamic_cast<toResultLine *>(childs->at(i));
      if (line) {
	if (enable)
	  line->start();
	else
	  line->stop();
      }
      toResultBar *bar=dynamic_cast<toResultBar *>(childs->at(i));
      if (bar) {
	if (enable)
	  bar->start();
	else
	  bar->stop();
      }
      toResultPie *pie=dynamic_cast<toResultPie *>(childs->at(i));
      if (pie) {
	if (enable)
	  pie->start();
	else
	  pie->stop();
      }
    }
    widget=chart;
  } else if (name==CONF_WAITS) {
    if (enable)
      Waits->start();
    else
      Waits->stop();
    widget=Waits;
  } else if (name==CONF_FILEIO) {
    if (enable)
      FileIO->start();
    else
      FileIO->stop();
    widget=FileIO;
  }
  if (widget)
    Tabs->setTabEnabled(widget,enable);
}

void toTuning::changeTab(QWidget *widget)
{
  if(LastTab==widget)
    return;
  refresh();
}

void toTuning::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),tr("&Refresh"),this,SLOT(refresh(void)),
			   Key_F5);
      ToolMenu->insertItem(tr("&Change Refresh"),Refresh,SLOT(setFocus(void)),
			   Key_R+ALT);
      toMainWidget()->menuBar()->insertItem(tr("&Tuning"),ToolMenu,-1,toToolMenuIndex());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

void toTuning::changeRefresh(const QString &str)
{
  try {
    toRefreshParse(timer(),str);
  } TOCATCH
}

void toTuning::refresh(void)
{
  LastTab=Tabs->currentPage();
  if (LastTab==Overview) {
    Overview->refresh();
  } else if (LastTab==Indicators) {
    Indicators->clear();
    std::list<QCString> val=toSQL::range("toTuning:Indicators");
    QListViewItem *parent=NULL;
    QListViewItem *last=NULL;
    for(std::list<QCString>::iterator i=val.begin();i!=val.end();i++) {
      try {
	toQList val=toQuery::readQuery(connection(),toSQL::string(*i,connection()));
	QStringList parts=QStringList::split(QString::fromLatin1(":"),*i);
	if (!parent||parent->text(0)!=parts[2]) {
	  parent=new toResultViewItem(Indicators,NULL,parts[2]);
	  parent->setOpen(true);
	  last=NULL;
	}
	QStringList dsc=QStringList::split(QString::fromLatin1("."),toSQL::description(*i));
	QString first=dsc[0];
	first+=QString::fromLatin1(".");
	last=new toResultViewItem(parent,last,first);
	QString str;
	for(toQList::iterator j=val.begin();j!=val.end();j++)
	  str+=*j;
	last->setText(1,str);
	if (dsc.count()>1)
	  last->setText(2,dsc[1]);
      } TOCATCH
    }
  } else if (LastTab==Waits)
    Waits->refresh();
  else if (LastTab==Statistics)
    Statistics->refreshStats();
  else if (LastTab==Parameters)
    Parameters->refresh();
  else if (LastTab==BlockingLocks)
    BlockingLocks->refresh();
  else if (LastTab==LibraryCache)
    LibraryCache->refresh();
  else if (LastTab==ControlFiles) {
    QString unit=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
    ControlFiles->changeParams(QString::number(toSizeDecode(unit)),unit);
  } else if (LastTab==Options)
    Options->refresh();
  else if (LastTab==Licenses)
    Licenses->refresh();
}

void toTuning::exportData(std::map<QCString,QString> &data,const QCString &prefix)
{
  toToolWidget::exportData(data,prefix);
  std::list<QCString> ret=TabList();
  for(std::list<QCString>::iterator i=ret.begin();i!=ret.end();i++) {
    QWidget *widget=tabWidget(*i);
    if (widget) {
      if (!Tabs->isTabEnabled(widget))
	data[prefix+":"+*i]=QString::fromLatin1("Disabled");
    }
  }
  data[prefix+":Current"]=Tabs->currentPage()->name();

  Waits->exportData(data,prefix+":Waits");
}

void toTuning::importData(std::map<QCString,QString> &data,const QCString &prefix)
{
  toToolWidget::importData(data,prefix);
  std::list<QCString> ret=TabList();
  for(std::list<QCString>::iterator i=ret.begin();i!=ret.end();i++)
    enableTab(*i,data[prefix+":"+(*i)].isEmpty());
  QWidget *chld=(QWidget *)child(data[prefix+":Current"]);
  if(chld)
    Tabs->showPage(chld);
  Waits->importData(data,prefix+":Waits");
}

static toSQL SQLFileIO("toTuning:FileIO",
		       "select a.name,b.name,sysdate,\n"
		       "       c.phyrds,c.phywrts,c.phyblkrd,c.phyblkwrt,\n"
		       "       c.avgiotim*10,c.miniotim*10,c.maxiortm*10,c.maxiowtm*10\n"
		       "  from v$tablespace a,v$datafile b,v$filestat c\n"
		       " where a.ts# = b.ts# and b.file# = c.file#\n"
		       "union\n",
		       "select a.name,b.name,sysdate,\n"
		       "       c.phyrds,c.phywrts,c.phyblkrd,c.phyblkwrt,\n"
		       "       c.avgiotim*10,c.miniotim*10,c.maxiortm*10,c.maxiowtm*10\n"
		       "  from v$tablespace a,v$tempfile b,v$tempstat c\n"
		       " where a.ts# = b.ts# and b.file# = c.file#\n"
		       " order by 1,2",
		       "Get file for files and tablespaces. Must have same columns.",
		       "8.1");

static toSQL SQLFileIO8("toTuning:FileIO",
			"select a.name,b.name,sysdate,\n"
			"       c.phyrds,c.phywrts,c.phyblkrd,c.phyblkwrt,\n"
			"       c.avgiotim*10,c.miniotim*10,c.maxiortm*10,c.maxiowtm*10\n"
			"  from v$tablespace a,v$datafile b,v$filestat c\n"
			" where a.ts# = b.ts# and b.file# = c.file#\n"
			" order by a.name",
			"",
			"8.0");

toTuningFileIO::toTuningFileIO(QWidget *parent,const char *name,WFlags fl)
  : QScrollView(parent,name,fl)
{
  try {
    connect(toCurrentTool(this)->timer(),SIGNAL(timeout()),this,SLOT(refresh()));

    viewport()->setBackgroundColor(qApp->palette().active().background());
    
    QVBox *Box=new QVBox(this->viewport());
    addChild(Box);

    QComboBox *combo=new QComboBox(Box);
    combo->insertItem(tr("File I/O"));
    combo->insertItem(tr("File timing"));
    if (toCurrentConnection(this).version()>="8.0") {
      combo->insertItem(tr("Tablespace I/O"));
      combo->insertItem(tr("Tablespace timing"));
    }
    connect(combo,SIGNAL(activated(int)),this,SLOT(changeCharts(int)));

    FileReads=new QGrid(2,Box);
    FileTime=new QGrid(2,Box);
    TablespaceReads=new QGrid(2,Box);
    TablespaceTime=new QGrid(2,Box);

    FileReads->setFixedWidth(viewport()->width()-50);
    FileTime->setFixedWidth(viewport()->width()-50);
    TablespaceReads->setFixedWidth(viewport()->width()-50);
    TablespaceTime->setFixedWidth(viewport()->width()-50);
    changeCharts(0);
    CurrentStamp=0;
    connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
    Query=NULL;
    refresh();
  } TOCATCH
}

toTuningFileIO::~toTuningFileIO()
{
  delete Query;
}

void toTuningFileIO::changeCharts(int val)
{
  FileReads->hide();
  FileTime->hide();
  TablespaceReads->hide();
  TablespaceTime->hide();
  switch(val) {
  case 0:
    FileReads->show();
    break;
  case 1:
    FileTime->show();
    break;
  case 2:
    TablespaceReads->show();
    break;
  case 3:
    TablespaceTime->show();
    break;
  }
}

void toTuningFileIO::allocCharts(const QString &name)
{
  std::list<QString> labels;
  labels.insert(labels.end(),tr("Reads"));
  labels.insert(labels.end(),tr("Blocks Read"));
  labels.insert(labels.end(),tr("Writes"));
  labels.insert(labels.end(),tr("Blocks Written"));
  std::list<QString> labelTime;
  labelTime.insert(labelTime.end(),tr("Average"));
  labelTime.insert(labelTime.end(),tr("Minimum"));
  labelTime.insert(labelTime.end(),tr("Maximum Read"));
  labelTime.insert(labelTime.end(),tr("Maximum Write"));

  toResultBar *barchart;
  if (name.startsWith(QString::fromLatin1("tspc:")))
    barchart=new toResultBar(TablespaceReads);
  else
    barchart=new toResultBar(FileReads);
  ReadsCharts[name]=barchart;
  barchart->setTitle(name.mid(5));
  barchart->setMinimumSize(200,170);
  barchart->setYPostfix(tr("blocks/s"));
  barchart->setLabels(labels);
  barchart->setSQLName(QString::fromLatin1("toTuning:FileIO:Reads:"+name));
  barchart->show();

  toResultLine *linechart;
  if (name.startsWith(QString::fromLatin1("tspc:")))
    linechart=new toResultLine(TablespaceTime);
  else
    linechart=new toResultLine(FileTime);
  TimeCharts[name]=linechart;
  linechart->setTitle(name.mid(5));
  linechart->setMinimumSize(200,170);
  linechart->setYPostfix(QString::fromLatin1("ms"));
  linechart->setLabels(labelTime);
  linechart->setSQLName(QString::fromLatin1("toTuning:FileIO:Time:"+name));
  linechart->show();
}

void toTuningFileIO::saveSample(const QString &name,const QString &label,
				double reads,double writes,
				double readBlk,double writeBlk,
				double avgTim,double minTim,
				double maxRead,double maxWrite)
{
  if (CurrentStamp!=LastStamp) {

    std::list<double> vals;
    vals.insert(vals.end(),reads);
    vals.insert(vals.end(),writes);
    vals.insert(vals.end(),readBlk);
    vals.insert(vals.end(),writeBlk);

    std::list<double> last=LastValues[name];
    std::list<double> dispVal;
    if (last.size()>0) {
      std::list<double>::iterator i=vals.begin();
      std::list<double>::iterator j=last.begin();
      while(i!=vals.end()&&j!=last.end()) {
	dispVal.insert(dispVal.end(),(*i-*j)/(CurrentStamp-LastStamp));
	i++;
	j++;
      }
    }
    LastValues[name]=vals;

    if (ReadsCharts.find(name)==ReadsCharts.end())
      allocCharts(name);

    if (dispVal.size()>0) {
      toBarChart *chart=ReadsCharts[name];
      chart->addValues(dispVal,label);
    }

    vals.clear();
    vals.insert(vals.end(),avgTim);
    vals.insert(vals.end(),minTim);
    vals.insert(vals.end(),maxRead);
    vals.insert(vals.end(),maxWrite);

    toLineChart *chart=TimeCharts[name];
    chart->addValues(vals,label);
  }
}

void toTuningFileIO::refresh(void)
{
  if (!Query) {
    try {
      toConnection &conn=toCurrentConnection(this);
      if (conn.version()<"8.0")
	return;
      toQList par;
      LastStamp=CurrentStamp;
      CurrentStamp=time(NULL);
      Query=new toNoBlockQuery(conn,toQuery::Background,toSQL::string(SQLFileIO,conn),par);
      LastTablespace=QString::null;

      Poll.start(100);
    } TOCATCH
  }
}

void toTuningFileIO::poll(void)
{
  try {
    if (Query&&Query->poll()) {
      QString tablespace;
      QString datafile;
      QString timestr;
      while(Query->poll()) {
	if (!Query->eof()) {
	  tablespace=Query->readValueNull();
	  datafile=Query->readValueNull();
	  timestr=Query->readValueNull();
	} else
	  tablespace=QString::null;
	if (tablespace!=LastTablespace) {
	  if (!LastTablespace.isNull()) {
	    QString name=QString::fromLatin1("tspc:");
	    name+=LastTablespace;

	    saveSample(name,timestr,
		       TblReads,TblWrites,TblReadBlk,TblWriteBlk,
		       TblAvg,TblMin,TblMaxRead,TblMaxWrite);
	  }

	  TblReads=TblWrites=TblReadBlk=TblWriteBlk=TblAvg=TblMin=TblMaxRead=TblMaxWrite=0;
	  LastTablespace=tablespace;
	}
	if (Query->eof())
	  break;

	double reads=Query->readValueNull().toDouble();
	double writes=Query->readValueNull().toDouble();
	double readBlk=Query->readValueNull().toDouble();
	double writeBlk=Query->readValueNull().toDouble();
	double avgTim=Query->readValueNull().toDouble();
	double minTim=Query->readValueNull().toDouble();
	double maxRead=Query->readValueNull().toDouble();
	double maxWrite=Query->readValueNull().toDouble();

	TblReads+=reads;
	TblWrites+=writes;
	TblReadBlk+=readBlk;
	TblWriteBlk+=writeBlk;
	TblAvg+=avgTim;
	TblMin+=minTim;
	TblMaxRead+=maxRead;
	TblMaxWrite+=maxWrite;

	QString name=QString::fromLatin1("file:");
	name+=datafile;

	saveSample(name,timestr,
		   reads,writes,readBlk,writeBlk,
		   avgTim,minTim,maxRead,maxWrite);
      }
      if (Query->eof()) {
	Poll.stop();
	delete Query;
	Query=NULL;
      }
    }
  } TOCATCH
}

void toTuningFileIO::resizeEvent(QResizeEvent *e)
{
  QScrollView::resizeEvent(e);

  FileReads->setFixedWidth(viewport()->width()-50);
  FileTime->setFixedWidth(viewport()->width()-50);
  TablespaceReads->setFixedWidth(viewport()->width()-50);
  TablespaceTime->setFixedWidth(viewport()->width()-50);
}

void toTuningFileIO::changeConnection(void)
{
  {
    for(std::map<QString,toBarChart *>::iterator i=ReadsCharts.begin();
        i!=ReadsCharts.end();i++)
      delete (*i).second;
  }
  ReadsCharts.clear();
  for(std::map<QString,toLineChart *>::iterator i=TimeCharts.begin();
      i!=TimeCharts.end();i++)
    delete (*i).second;
  TimeCharts.clear();
  LastValues.clear();
  refresh();
}

toTuningMiss::toTuningMiss(QWidget *parent,const char *name)
  : toResultLine(parent,name)
{
}

std::list<double> toTuningMiss::transform(std::list<double> &inp)
{
  std::list<double> ret;
  for(std::list<double>::iterator i=inp.begin();i!=inp.end();i++) {
    double first=*i;
    i++;
    if (i!=inp.end()) {
      double second=*i;
      if (second==0)
	ret.insert(ret.end(),0);
      else
	ret.insert(ret.end(),first/second);
    }
  }
  return ret;
}

void toTuningFileIO::stop(void)
{
  try {
    disconnect(toCurrentTool(this)->timer(),SIGNAL(timeout()),this,SLOT(refresh()));
  } TOCATCH
}

void toTuningFileIO::start(void)
{
  try {
    connect(toCurrentTool(this)->timer(),SIGNAL(timeout()),this,SLOT(refresh()));
  } TOCATCH
}
