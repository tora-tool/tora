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

#include <time.h>

#include <qpainter.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qgrid.h>
#include <qcombobox.h>
#include <qscrollview.h>
#include <qvbox.h>
#include <qworkspace.h>
#include <qmenubar.h>
#include <qobjcoll.h>
#include <qmessagebox.h>

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include "toresultparam.h"
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
#include "toresultitem.h"
#include "toresultlong.h"
#include "totuningsettingui.h"
#include "tonoblockquery.h"
#include "toresultlock.h"

#include "totuning.moc"
#include "totuningoverviewui.moc"
#include "totuningsettingui.moc"

#include "icons/refresh.xpm"
#include "icons/totuning.xpm"
#include "icons/compile.xpm"

#define CONF_OVERVIEW "Overview"
#define CONF_FILEIO   "File I/O"
#define CONF_CHARTS   "Charts"
#define CONF_WAITS    "Wait events"

static std::list<QString> TabList(void)
{
  std::list<QString> ret;
  ret.insert(ret.end(),CONF_OVERVIEW);
  ret.insert(ret.end(),CONF_CHARTS);
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
    std::list<QString> tabs=TabList();
    for(std::list<QString>::iterator i=tabs.begin();i!=tabs.end();i++) {
      QListViewItem *item=new QListViewItem(EnabledTabs,*i);
      if (!tool->config(*i,"").isEmpty())
	item->setSelected(true);
    }
    EnabledTabs->setSorting(0);
  }
  virtual void saveSetting(void)
  {
    for (QListViewItem *item=EnabledTabs->firstChild();item;item=item->nextSibling()) {
      if (item->isSelected()||Tool->config(item->text(0),"Undefined")!="Undefined")
	Tool->setConfig(item->text(0),(item->isSelected()?"Yes":""));
    }
  }
};

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
    return new toTuning(parent,connection);
  }
  virtual QWidget *configurationTab(QWidget *parent)
  {
    return new toTuningSetup(this,parent);
  }
};

static toTuningTool TuningTool;

static toSQL SQLDictionaryMiss("toTuning:Indicators:Important ratios:1DictionaryMiss",
			       "select to_char(round(sum(getmisses)/sum(gets)*100,2))||' %' from v$rowcache",
			       "Data dictionary row cache miss ratio (%). < 10%-15%");

static toSQL SQLLibraryCache("toTuning:Indicators:Important ratios:2LibraryCache",
			     "select to_char(round(sum(reloads)/sum(pins)*100,2))||' %' from v$librarycache",
			     "Library cache (Shared SQL) miss ratio (%). < 1%");

static toSQL SQLDataCache("toTuning:Indicators:Important ratios:3DataCache",
			  "SELECT TO_CHAR(ROUND((1-SUM(DECODE(statistic#,40,value,0))/SUM(DECODE(statistic#,38,value,39,value,0)))*100,2))||' %'\n"
			  "  FROM v$sysstat\n"
			  " WHERE statistic# IN (38,39,40)",
			  "Data buffer cache hit ratio (%). > 60% - 70%",
			  "8.0");

static toSQL SQLDataCache9("toTuning:Indicators:Important ratios:3DataCache",
			   "SELECT TO_CHAR(ROUND((1-SUM(DECODE(statistic#,42,value,0))/SUM(DECODE(statistic#,40,value,41,value,0)))*100,2))||' %'\n"
			   "  FROM v$sysstat\n"
			   " WHERE statistic# IN (40,41,42)",
			   QString::null,
			   "9.0");

static toSQL SQLDataCache7("toTuning:Indicators:Important ratios:3DataCache",
			   "SELECT TO_CHAR(ROUND((1-SUM(DECODE(statistic#,39,value,0))/SUM(DECODE(statistic#,37,value,38,value,0)))*100,2))||' %'\n"
			   "  FROM v$sysstat\n"
			   " WHERE statistic# IN (37,38,39)",
			   QString::null,
			   "7.3");

static toSQL SQLLogRedo("toTuning:Indicators:Redo log contention:1LogSpace",
			"select value from v$sysstat where statistic# = 108",
			"Redo log space requests. Close to 0",
			"8.0");

static toSQL SQLLogRedo9("toTuning:Indicators:Redo log contention:1LogSpace",
			 "select value from v$sysstat where statistic# = 122",
			 QString::null,
			 "9.0");

static toSQL SQLLogRedo7("toTuning:Indicators:Redo log contention:1LogSpace",
			 "select value from v$sysstat where statistic# = 94",
			 QString::null,
			 "7.3");

static toSQL SQLSystemHeadUndo("toTuning:Indicators:RBS contention:1SystemHeadUndo",
			       "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			       "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo header') a,\n"
			       "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (38,39)) b",
			       "System undo header waits (%). < 1%",
			       "8.0");

static toSQL SQLSystemHeadUndo9("toTuning:Indicators:RBS contention:1SystemHeadUndo",
				"SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
				"  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo header') a,\n"
				"       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (40,41)) b",
				QString::null,
				"9.0");

static toSQL SQLSystemHeadUndo7("toTuning:Indicators:RBS contention:1SystemHeadUndo",
				"SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
				"  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo header') a,\n"
				"       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
				QString::null,
				"7.3");

static toSQL SQLSystemBlockUndo("toTuning:Indicators:RBS contention:2SystemBlockUndo",
				"SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
				"  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo block') a,\n"
				"       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (38,39)) b",
				"System undo block waits (%). < 1%",
				"8.0");

static toSQL SQLSystemBlockUndo9("toTuning:Indicators:RBS contention:2SystemBlockUndo",
				 "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
				 "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo block') a,\n"
				 "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (40,41)) b",
				 QString::null,
				 "9.0");

static toSQL SQLSystemBlockUndo7("toTuning:Indicators:RBS contention:2SystemBlockUndo",
				 "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
				 "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo block') a,\n"
				 "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
				 QString::null,
				 "7.3");

static toSQL SQLHeadUndo("toTuning:Indicators:RBS contention:3HeadUndo",
			 "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			 "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo header') a,\n"
			 "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (38,39)) b",
			 "Undo head waits (%). < 1%",
			 "8.0");

static toSQL SQLHeadUndo9("toTuning:Indicators:RBS contention:3HeadUndo",
			  "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			  "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo header') a,\n"
			  "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (40,41)) b",
			  QString::null,
			  "9.0");

static toSQL SQLHeadUndo7("toTuning:Indicators:RBS contention:3HeadUndo",
			  "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			  "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo header') a,\n"
			  "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
			  QString::null,
			  "7.3");

static toSQL SQLBlockUndo("toTuning:Indicators:RBS contention:4BlockUndo",
			  "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			  "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo block') a,\n"
			  "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (38,39)) b",
			  "Undo block waits (%). < 1%",
			  "8.0");

static toSQL SQLBlockUndo9("toTuning:Indicators:RBS contention:4BlockUndo",
			   "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			   "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo block') a,\n"
			   "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (40,41)) b",
			   QString::null,
			   "9.0");

static toSQL SQLBlockUndo7("toTuning:Indicators:RBS contention:4BlockUndo",
			   "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			   "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo block') a,\n"
			   "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
			   QString::null,
			   "7.3");

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
				QString::null,
				"9.0");

static toSQL SQLChartsPhysical7("toTuning:Charts:1BBPhysical I/O",
				"select SYSDATE,\n"
				"       sum(decode(statistic#,39,value,0)) \"Physical reads\",\n"
				"       sum(decode(statistic#,40,value,0)) \"Physical writes\",\n"
				"       sum(decode(statistic#,91,value,0)) \"Redo writes\"\n"
				"  from v$sysstat where statistic# in (39,40,91)",
				QString::null,
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
			       QString::null,
			       "9.0");

static toSQL SQLChartsLogical7("toTuning:Charts:2BBLogical I/O",
			       "select SYSDATE,\n"
			       "       sum(decode(statistic#,37,value,0)) \"Block gets\",\n"
			       "       sum(decode(statistic#,38,value,0)) \"Consistent gets\",\n"
			       "       sum(decode(statistic#,43,value,0)) \"Block changes\",\n"
			       "       sum(decode(statistic#,45,value,0)) \"Consistent changes\"\n"
			       "  from v$sysstat where statistic# in (37,38,43,45)",
			       QString::null,
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
				 QString::null,
				 "9.0");

static toSQL SQLChartsExecution7("toTuning:Charts:6LNExecution",
				 "select SYSDATE,\n"
				 "       sum(decode(statistic#,132,value,0)) \"Execute\",\n"
				 "       sum(decode(statistic#,131,value,0)) \"Parse\",\n"
				 "       sum(decode(statistic#,6,value,0)) \"Calls\",\n"
				 "       sum(decode(statistic#,4,value,0))  \"Commit\",\n"
				 "       sum(decode(statistic#,5,value,0)) \"Rollbacks\"\n"
				 "  from v$sysstat where statistic# in (132,131,4,5,6)",
				 QString::null,
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
				    QString::null,
				    "9.0");

static toSQL SQLChartsExecutionPie7("toTuning:Charts:8PNExecution Total",
				    "select value,decode(statistic#,132,'Execute',131,'Parse',\n"
				    "                              6,'Calls',4,'Commit',5,'Rollbacks')\n"
				    "  from v$sysstat where statistic# in (132,131,6,4,5) order by name",
				    QString::null,
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
			       QString::null,
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
				   QString::null,
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
				   QString::null,
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
			    QString::null,
			    "9.0");

static toSQL SQLChartsRedo7("toTuning:Charts:7BSRedo log I/O",
			    "select SYSDATE,\n"
			    "       sum(decode(statistic#,85,value,0))/:unit<int> \"Redo size\",\n"
			    "       sum(decode(statistic#,89,value,0))/:unit<int> \"Redo wastage\"\n"
			    "  from v$sysstat where statistic# in (85,89)",
			    QString::null,
			    "7.3");

static toSQL SQLOverviewArchiveWrite("toTuning:Overview:ArchiveWrite",
				     "select sysdate,sum(blocks) from v$archived_log",
				     "Archive log write",
				     "8.0");

static toSQL SQLOverviewArchiveWrite7("toTuning:Overview:ArchiveWrite",
				      "select sysdate,0 from sys.dual",
				      QString::null,
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
				   QString::null,
				   "9.0");

static toSQL SQLOverviewBufferHit7("toTuning:Overview:BufferHit",
				   "SELECT SYSDATE,(1-SUM(DECODE(statistic#,39,value,0))/SUM(DECODE(statistic#,37,value,38,value,0)))*100\n"
				   "  FROM v$sysstat\n"
				   " WHERE statistic# IN (37,38,39)",
				   QString::null,
				   "7.3");

static toSQL SQLOverviewClientInput("toTuning:Overview:ClientInput",
				    "select sysdate,value/:f1<int>\n"
				    "  from v$sysstat where statistic# = 182",
				    "Bytes sent to client",
				    "8.0");

static toSQL SQLOverviewClientInput9("toTuning:Overview:ClientInput",
				     "select sysdate,value/:f1<int>\n"
				     "  from v$sysstat where statistic# = 223",
				     QString::null,
				     "9.0");

static toSQL SQLOverviewClientInput7("toTuning:Overview:ClientInput",
				     "select sysdate,value/:f1<int>\n"
				     "  from v$sysstat where statistic# = 134",
				     QString::null,
				     "7.3");

static toSQL SQLOverviewClientOutput("toTuning:Overview:ClientOutput",
				     "select sysdate,value/:f1<int>\n"
				     "  from v$sysstat where statistic# = 183",
				     "Bytes sent from client",
				     "8.0");

static toSQL SQLOverviewClientOutput9("toTuning:Overview:ClientOutput",
				      "select sysdate,value/:f1<int>\n"
				      "  from v$sysstat where statistic# = 224",
				      QString::null,
				      "9.0");

static toSQL SQLOverviewClientOutput7("toTuning:Overview:ClientOutput",
				      "select sysdate,value/:f1<int>\n"
				      "  from v$sysstat where statistic# = 133",
				      QString::null,
				      "7.3");

static toSQL SQLOverviewExecute("toTuning:Overview:Execute",
				"select sysdate,value\n"
				"  from v$sysstat where statistic# = 181",
				"Execute count",
				"8.0");

static toSQL SQLOverviewExecute9("toTuning:Overview:Execute",
				 "select sysdate,value\n"
				 "  from v$sysstat where statistic# = 222",
				 QString::null,
				 "9.0");

static toSQL SQLOverviewExecute7("toTuning:Overview:Execute",
				 "select sysdate,value\n"
				 "  from v$sysstat where statistic# = 132",
				 QString::null,
				 "7.3");

static toSQL SQLOverviewParse("toTuning:Overview:Parse",
			      "select sysdate,value\n"
			      "  from v$sysstat where statistic# = 179",
			      "Parse count",
			      "8.0");

static toSQL SQLOverviewParse9("toTuning:Overview:Parse",
			       "select sysdate,value\n"
			       "  from v$sysstat where statistic# = 219",
			       QString::null,
			       "9.0");

static toSQL SQLOverviewParse7("toTuning:Overview:Parse",
			       "select sysdate,value\n"
			       "  from v$sysstat where statistic# = 131",
			       QString::null,
			       "7.3");

static toSQL SQLOverviewRedoEntries("toTuning:Overview:RedoEntries",
				    "select sysdate,value\n"
				    "  from v$sysstat where statistic# = 100",
				    "Redo entries",
				    "8.0");

static toSQL SQLOverviewRedoEntries9("toTuning:Overview:RedoEntries",
				     "select sysdate,value\n"
				     "  from v$sysstat where statistic# = 114",
				     QString::null,
				     "9.0");

static toSQL SQLOverviewRedoEntries7("toTuning:Overview:RedoEntries",
				     "select sysdate,value\n"
				     "  from v$sysstat where statistic# = 84",
				     QString::null,
				     "7.3");

static toSQL SQLOverviewRedoBlocks("toTuning:Overview:RedoBlocks",
				   "select sysdate,value\n"
				   "  from v$sysstat where statistic# = 106",
				   "Redo blocks written",
				   "8.0");

static toSQL SQLOverviewRedoBlocks9("toTuning:Overview:RedoBlocks",
				    "select sysdate,value\n"
				    "  from v$sysstat where statistic# = 120",
				    QString::null,
				    "9.0");

static toSQL SQLOverviewRedoBlocks7("toTuning:Overview:RedoBlocks",
				    "select sysdate,value\n"
				    "  from v$sysstat where statistic# = 92",
				    QString::null,
				    "7.3");

static toSQL SQLOverviewLogicalRead("toTuning:Overview:LogicalRead",
				    "select sysdate,sum(value)\n"
				    "  from v$sysstat where statistic# in (38,39)",
				    "Blocks read",
				    "8.0");

static toSQL SQLOverviewLogicalRead9("toTuning:Overview:LogicalRead",
				     "select sysdate,sum(value)\n"
				     "  from v$sysstat where statistic# in (40,41)",
				     QString::null,
				     "9.0");

static toSQL SQLOverviewLogicalRead7("toTuning:Overview:LogicalRead",
				     "select sysdate,sum(value)\n"
				     "  from v$sysstat where statistic# in (37,38)",
				     QString::null,
				     "7.3");

static toSQL SQLOverviewLogicalWrite("toTuning:Overview:LogicalWrite",
				     "select sysdate,sum(value)\n"
				     "  from v$sysstat where statistic# in (41,42)",
				     "Blocks written",
				     "8.0");

static toSQL SQLOverviewLogicalWrite9("toTuning:Overview:LogicalWrite",
				      "select sysdate,sum(value)\n"
				      "  from v$sysstat where statistic# in (43,44)",
				      QString::null,
				      "9.0");

static toSQL SQLOverviewLogicalWrite7("toTuning:Overview:LogicalWrite",
				      "select sysdate,sum(value)\n"
				      "  from v$sysstat where statistic# in (43,45)",
				      QString::null,
				      "7.3");

static toSQL SQLOverviewPhysicalRead("toTuning:Overview:PhysicalRead",
				     "select sysdate,value\n"
				     "  from v$sysstat where statistic# = 40",
				     "Blocks physically read",
				     "8.0");

static toSQL SQLOverviewPhysicalRead9("toTuning:Overview:PhysicalRead",
				      "select sysdate,value\n"
				      "  from v$sysstat where statistic# = 42",
				      QString::null,
				      "9.0");

static toSQL SQLOverviewPhysicalRead7("toTuning:Overview:PhysicalRead",
				      "select sysdate,value\n"
				      "  from v$sysstat where statistic# = 39",
				      QString::null,
				      "7.3");

static toSQL SQLOverviewPhysicalWrite("toTuning:Overview:PhysicalWrite",
				      "select sysdate,value\n"
				      "  from v$sysstat where statistic# = 44",
				      "Blocks physically written",
				      "8.0");

static toSQL SQLOverviewPhysicalWrite9("toTuning:Overview:PhysicalWrite",
				       "select sysdate,value\n"
				       "  from v$sysstat where statistic# = 46",
				       QString::null,
				       "9.0");

static toSQL SQLOverviewPhysicalWrite7("toTuning:Overview:PhysicalWrite",
				       "select sysdate,value\n"
				       "  from v$sysstat where statistic# = 40",
				       QString::null,
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
				 QString::null,
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

void toTuningOverview::setupChart(toResultLine *chart,const QString &title,const QString &postfix,toSQL &sql)
{
  chart->setMinValue(0);
  chart->showGrid(0);
  chart->showLegend(false);
  chart->showAxisLegend(false);
  chart->setTitle(title);
  chart->showLast(true);
  toQList val;
  if (postfix=="b/s") {
    QString unitStr=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
    val.insert(val.end(),toQValue(toSizeDecode(unitStr)));
    unitStr+="/s";
    chart->setYPostfix(unitStr);
  } else
    chart->setYPostfix(postfix);
  chart->query(toSQL::string(sql,toCurrentConnection(this)),val);
}

toTuningOverview::toTuningOverview(QWidget *parent,const char *name,WFlags fl)
  : toTuningOverviewUI(parent,name,fl)
{
  BackgroundGroup->setColumnLayout(1,Horizontal);

  setupChart(ArchiveWrite,"< Archive write"," blocks/s",SQLOverviewArchiveWrite);
  setupChart(BufferHit,"Hitrate","%",SQLOverviewBufferHit);
  BufferHit->setMaxValue(100);
  BufferHit->setFlow(false);
  setupChart(ClientInput,"< Client input","b/s",SQLOverviewClientInput);
  setupChart(ClientOutput,"Client output >","b/s",SQLOverviewClientOutput);
  setupChart(ExecuteCount,"Executes >","/s",SQLOverviewExecute);
  setupChart(LogWrite,"Log writer >"," blocks/s",SQLOverviewRedoBlocks);
  setupChart(LogicalChange,"Buffer changed >"," blocks/s",SQLOverviewLogicalWrite);
  setupChart(LogicalRead,"< Buffer gets"," blocks/s",SQLOverviewLogicalRead);
  setupChart(ParseCount,"Parse >","/s",SQLOverviewParse);
  setupChart(PhysicalRead,"< Physical read"," blocks/s",SQLOverviewPhysicalRead);
  setupChart(PhysicalWrite,"Physical write >"," blocks/s",SQLOverviewPhysicalWrite);
  setupChart(RedoEntries,"Redo entries >","/s",SQLOverviewRedoEntries);
  setupChart(Timescale,"Timescale","",SQLOverviewTimescale);
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
  SharedUsed->setYPostfix("%");
  SharedUsed->showLast(true);

  toQList val;
  val.insert(val.end(),
	     toQValue(toSizeDecode(toTool::globalConfig(CONF_SIZE_UNIT,
							DEFAULT_SIZE_UNIT))));
  FileUsed->query(toSQL::string(SQLOverviewFilespace,toCurrentConnection(this)),val);
  FileUsed->showLegend(false);

  // Will be called later anyway
  //refresh();
}

void toTuningOverview::stop(void)
{
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
				 QString::null,
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
				    QString::null,
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
				   QString::null,
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
				   QString::null,
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
				     QString::null,
				     "8.0");

static toSQL SQLOverviewDatafiles("toTuning:Overview:Datafiles",
				  "select count(1) from v$datafile",
				  "Number of datafiles",
				  "8.0");

static toSQL SQLOverviewDatafiles7("toTuning:Overview:Datafiles",
				   "select count(1) from sys.dba_tablespaces",
				   QString::null,
				   "7.3");

void toTuningOverview::refresh(void)
{
  try {
    toConnection &conn=toCurrentConnection(this);

    toQList val;
    QString unitStr=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
    val.insert(val.end(),toQValue(toSizeDecode(unitStr)));

    toQList res=toQuery::readQuery(conn,SQLOverviewArchive,val);
    QString tmp=toShift(res);
    tmp+="/";
    tmp+=toShift(res);
    tmp+=unitStr;
    ArchiveInfo->setText(tmp);

    std::list<QLabel *>::iterator labIt=Backgrounds.begin();

    res=toQuery::readQuery(conn,SQLOverviewRound);
    tmp=toShift(res);
    tmp+=" ms";
    SendFromClient->setText(tmp);
    tmp=toShift(res);
    tmp+=" ms";
    SendToClient->setText(tmp);

    res=toQuery::readQuery(conn,SQLOverviewClientTotal);
    tmp=toShift(res);
    TotalClient->setText(tmp);
    tmp=toShift(res);
    ActiveClient->setText(tmp);

    int totJob=0;
    res=toQuery::readQuery(conn,SQLOverviewDedicated);
    tmp=toShift(res);
    totJob+=tmp.toInt();
    DedicatedServer->setText(tmp);

    res=toQuery::readQuery(conn,SQLOverviewDispatcher);
    tmp=toShift(res);
    totJob+=tmp.toInt();
    DispatcherServer->setText(tmp);

    res=toQuery::readQuery(conn,SQLOverviewShared);
    tmp=toShift(res);
    totJob+=tmp.toInt();
    SharedServer->setText(tmp);

    res=toQuery::readQuery(conn,SQLOverviewParallell);
    tmp=toShift(res);
    totJob+=tmp.toInt();
    ParallellServer->setText(tmp);

    res=toQuery::readQuery(conn,SQLOverviewBackground);
    while(res.size()>0) {
      tmp=toShift(res);
      QLabel *label;
      if (labIt==Backgrounds.end()||*labIt==NULL) {
	label=new QLabel(BackgroundGroup);
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
      if(tmp=="DBW")
	tmp="DBWR";
      else if (tmp=="PMO")
	tmp="PMON";
      else if (tmp=="ARC")
	tmp="ARCH";
      else if (tmp=="CKP")
	tmp="CKPT";
      else if (tmp=="LGW")
	tmp="LGWR";
      else if (tmp=="LMO")
	tmp="LMON";
      else if (tmp=="REC")
	tmp="RECO";
      else if (tmp=="TRW")
	tmp="TRWR";
      else if (tmp=="SMO")
	tmp="SMON";

      tmp+=": <B>";
      QString job=toShift(res);
      totJob+=job.toInt();
      tmp+=job;
      tmp+="</B>";
      label->setText(tmp);
    }
    while(labIt!=Backgrounds.end()) {
      delete *labIt;
      *labIt=NULL;
      labIt++;
    }
    TotalProcess->setText(QString::number(totJob));

    double tot=0;
    double sql=0;
    res=toQuery::readQuery(conn,SQLOverviewSGA,val);
    while(res.size()>0) {
      QLabel *widget=NULL;
      QString nam=toShift(res);
      tmp=toShift(res);
      if (nam=="Database Buffers")
	widget=BufferSize;
      else if (nam=="Redo Buffers")
	widget=RedoBuffer;
      else if (nam=="Fixed Size"||nam=="Variable Size")
	sql+=tmp.toDouble();
      tot+=tmp.toDouble();
      if (widget) {
	tmp+=unitStr;
	widget->setText(tmp);
      }
    }
    tmp=QString::number(tot);
    tmp+=unitStr;
    SGATotal->setText(tmp);
    tmp=QString::number(sql);
    tmp+=unitStr;
    SharedSize->setText(tmp);

    res=toQuery::readQuery(conn,SQLOverviewLog,val);
    RedoFiles->setText(toShift(res));
    ActiveRedo->setText(toShift(res));
    tmp=toShift(res);
    tmp+="/";
    tmp+=toShift(res);
    tmp+=unitStr;
    RedoSize->setText(tmp);

    std::list<double> &values=FileUsed->values();
    std::list<double>::iterator i=values.begin();
    double size=0;
    double used=0;
    if (i!=values.end())
      used=size=(*i);
    i++;
    if (i!=values.end())
      size+=(*i);
    tmp=QString::number(used);
    tmp+="/";
    tmp+=QString::number(size);
    tmp+=unitStr;
    Filesize->setText(tmp);

    res=toQuery::readQuery(conn,SQLOverviewTablespaces);
    Tablespaces->setText(toShift(res));

    res=toQuery::readQuery(conn,SQLOverviewDatafiles);
    Files->setText(toShift(res));
  } TOCATCH
}

toTuning::toTuning(QWidget *main,toConnection &connection)
  : toToolWidget(TuningTool,"tuning.html",main,connection)
{
  if (TuningTool.config(CONF_OVERVIEW,"Undefined")=="Undefined") {
    QString def=QString::null;
    if (TOMessageBox::warning(toMainWidget(),
			      "Enable all tuning statistics",
			      "Are you sure you want to enable all tuning features.\n"
			      "This can put heavy strain on a database and unless you\n"
			      "are the DBA you probably don't want this. Selecting\n"
			      "no here will give you the option to enable or disable\n"
			      "tabs individually as they are needed.",
			      "Yes","&No",QString::null,1)==0) {
      def="Yes";
    }
    std::list<QString> tabs=TabList();
    for(std::list<QString>::iterator i=tabs.begin();i!=tabs.end();i++)
      TuningTool.setConfig(*i,def);
  }

  QToolBar *toolbar=toAllocBar(this,"Server Tuning",connection.description());

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Refresh",
		  "Refresh",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QLabel("Refresh ",toolbar);
  Refresh=toRefreshCreate(toolbar);
  connect(Refresh,SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));
  toRefreshParse(timer());

  toolbar->addSeparator();
  TabButton=new toPopupButton(QPixmap((const char **)compile_xpm),
		 	     "Enable and disable tuning tabs",
			     "Enable and disable tuning tabs",
			     toolbar);
  TabMenu=new QPopupMenu(TabButton);
  TabButton->setPopup(TabMenu);
  connect(TabMenu,SIGNAL(aboutToShow()),this,SLOT(showTabMenu()));
  connect(TabMenu,SIGNAL(activated(int)),this,SLOT(enableTabMenu(int)));

  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  Tabs=new QTabWidget(this);

  Overview=new toTuningOverview(this,"overview");
  Tabs->addTab(Overview,"&Overview");

  Charts=new QGrid(2,Tabs,"charts");

  QString unitStr=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
  toQList unit;
  unit.insert(unit.end(),toQValue(toSizeDecode(unitStr)));
  {
    std::list<QString> val=toSQL::range("toTuning:Charts");
    for(std::list<QString>::iterator i=val.begin();i!=val.end();i++) {
      QStringList parts=QStringList::split(":",*i);
      if (parts[2].mid(1,1)=="B") {
	toResultBar *chart=new toResultBar(Charts);
	chart->setTitle(parts[2].mid(3));
	toQList par;
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
	chart->query(toSQL::string(*i,connection),par);
      } else if (parts[2].mid(1,1)=="L"||parts[2].mid(1,1)=="C") {
	toResultLine *chart;
	if (parts[2].mid(1,1)=="C")
	  chart=new toTuningMiss(Charts);
	else
	  chart=new toResultLine(Charts);
	chart->setTitle(parts[2].mid(3));
	toQList par;
	if (parts[2].mid(2,1)=="B")
	  chart->setYPostfix(" blocks/s");
	else if (parts[2].mid(2,1)=="S") {
	  par=unit;
	  QString t=unitStr;
	  t+="/s";
	  chart->setYPostfix(t);
	} else if (parts[2].mid(2,1)=="P") {
	  chart->setYPostfix(" %");
	  chart->setMinValue(0);
        } else
	  chart->setYPostfix("/s");
	chart->query(toSQL::string(*i,connection),par);
      } else if (parts[2].mid(1,1)=="P") {
	toResultPie *chart=new toResultPie(Charts);
	chart->setTitle(parts[2].mid(3));
	if (parts[2].mid(2,1)=="S") {
	  chart->query(toSQL::string(*i,connection),unit);
	  chart->setPostfix(unitStr);
	} else
	  chart->query(toSQL::string(*i,connection));
      } else
	throw QString("Wrong format of name on chart.");
    }
  }

  Tabs->addTab(Charts,"&Charts");

  FileIO=new toTuningFileIO(this,"fileio");
  connect(this,SIGNAL(connectionChange()),FileIO,SLOT(changeConnection()));

  Waits=new toTuningWait(this,"waits");
  Tabs->addTab(Waits,"Wait events");

  Tabs->addTab(FileIO,"&File I/O");

  Indicators=new toListView(Tabs,"indicators");
  Indicators->setRootIsDecorated(true);
  Indicators->addColumn("Indicator");
  Indicators->addColumn("Value");
  Indicators->addColumn("Reference");
  Tabs->addTab(Indicators,"&Indicators");

  Statistics=new toResultStats(Tabs,"stats");
  Tabs->addTab(Statistics,"&Statistics");

  Parameters=new toResultParam(Tabs,"parameters");
  Tabs->addTab(Parameters,"&Parameters");

  BlockingLocks=new toResultLock(Tabs,"locks");
  Tabs->addTab(BlockingLocks,"&Blocking locks");

  Options=new toResultLong(true,false,toQuery::Background,Tabs,"options");
  Options->setSQL(SQLOptions);
  Tabs->addTab(Options,"Optio&ns");

  Licenses=new toResultItem(2,true,Tabs,"licenses");
  Licenses->setSQL(SQLLicense);
  Tabs->addTab(Licenses,"&Licenses");

  Tabs->setCurrentPage(0);

  LastTab=NULL;

  connect(Tabs,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeTab(QWidget *)));
  connect(timer(),SIGNAL(timeout(void)),this,SLOT(refresh(void)));
  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  std::list<QString> tabs=TabList();
  for(std::list<QString>::iterator i=tabs.begin();i!=tabs.end();i++)
    if (TuningTool.config(*i,QString::null).isEmpty())
      enableTab(*i,false);

  refresh();
  setFocusProxy(Tabs);
}

QWidget *toTuning::tabWidget(const QString &name)
{
  QWidget *widget=NULL;
  if (name==CONF_OVERVIEW) {
    widget=Overview;
  } else if (name==CONF_CHARTS) {
    widget=Charts;
  } else if (name==CONF_FILEIO) {
    widget=FileIO;
  } else if (name==CONF_WAITS) {
    widget=Waits;
  }
  return widget;
}

void toTuning::showTabMenu(void)
{
  int id=1;
  TabMenu->clear();
  std::list<QString> tab=TabList();
  for(std::list<QString>::iterator i=tab.begin();i!=tab.end();i++) {
    id++;
    TabMenu->insertItem(*i,id);
    QWidget *widget=tabWidget(*i);
    if (widget&&Tabs->isTabEnabled(widget))
      TabMenu->setItemChecked(id,true);
  }
}

void toTuning::enableTabMenu(int selid)
{
  std::list<QString> tab=TabList();
  int id=1;
  for(std::list<QString>::iterator i=tab.begin();i!=tab.end();i++) {
    id++;
    if (selid==id) {
      QWidget *widget=tabWidget(*i);
      if (widget)
	enableTab(*i,!Tabs->isTabEnabled(widget));
      break;
    }
  }
}

void toTuning::enableTab(const QString &name,bool enable)
{
  QWidget *widget=NULL;
  if (name==CONF_OVERVIEW) {
    if (enable)
      Overview->start();
    else
      Overview->stop();
    widget=Overview;
  } else if (name==CONF_CHARTS) {
    QObjectList *childs=(QObjectList *)Charts->children();
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
    widget=Charts;
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
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),"&Refresh",this,SLOT(refresh(void)),
			   Key_F5);
      ToolMenu->insertItem("&Change Refresh",Refresh,SLOT(setFocus(void)),
			   Key_R+ALT);
      toMainWidget()->menuBar()->insertItem("&Tuning",ToolMenu,-1,toToolMenuIndex());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

void toTuning::changeRefresh(const QString &str)
{
  toRefreshParse(timer(),str);
}

void toTuning::refresh(void)
{
  LastTab=Tabs->currentPage();
  if (LastTab==Overview) {
    Overview->refresh();
  } else if (LastTab==Indicators) {
    Indicators->clear();
    std::list<QString> val=toSQL::range("toTuning:Indicators");
    QListViewItem *parent=NULL;
    QListViewItem *last=NULL;
    for(std::list<QString>::iterator i=val.begin();i!=val.end();i++) {
      try {
	toQList val=toQuery::readQuery(connection(),toSQL::string(*i,connection()));
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
  } else if (LastTab==Waits)
    Waits->refresh();
  else if (LastTab==Statistics)
    Statistics->refreshStats();
  else if (LastTab==Parameters)
    Parameters->refresh();
  else if (LastTab==BlockingLocks)
    BlockingLocks->refresh();
  else if (LastTab==Options)
    Options->refresh();
  else if (LastTab==Licenses)
    Licenses->refresh();
}

void toTuning::exportData(std::map<QString,QString> &data,const QString &prefix)
{
  toToolWidget::exportData(data,prefix);
  std::list<QString> ret=TabList();
  for(std::list<QString>::iterator i=ret.begin();i!=ret.end();i++) {
    QWidget *widget=tabWidget(*i);
    if (widget) {
      if (!Tabs->isTabEnabled(widget))
	data[prefix+":"+*i]="Disabled";
    }
  }
  data[prefix+":Current"]=Tabs->currentPage()->name();
}

void toTuning::importData(std::map<QString,QString> &data,const QString &prefix)
{
  toToolWidget::importData(data,prefix);
  std::list<QString> ret=TabList();
  for(std::list<QString>::iterator i=ret.begin();i!=ret.end();i++)
    enableTab(*i,data[prefix+":"+(*i)].isEmpty());
  QWidget *chld=(QWidget *)child(data[prefix+":Current"]);
  if(chld)
    Tabs->showPage(chld);
}

static toSQL SQLFileIO("toTuning:FileIO",
		       "select a.name,b.name,sysdate,\n"
		       "       c.phyrds,c.phywrts,c.phyblkrd,c.phyblkwrt,\n"
		       "       c.avgiotim*10,c.miniotim*10,c.maxiortm*10,c.maxiowtm*10\n"
		       "  from v$tablespace a,v$datafile b,v$filestat c\n"
		       " where a.ts# = b.ts# and b.file# = c.file#\n"
		       " order by a.name",
		       "Get file for files and tablespaces. Must have same columns.",
		       "8.0");

toTuningFileIO::toTuningFileIO(QWidget *parent,const char *name,WFlags fl)
  : QScrollView(parent,name,fl)
{
  connect(toCurrentTool(this)->timer(),SIGNAL(timeout()),this,SLOT(refresh()));

  try {
    viewport()->setBackgroundColor(qApp->palette().active().background());
    
    QVBox *Box=new QVBox(this->viewport());
    addChild(Box);

    QComboBox *combo=new QComboBox(Box);
    combo->insertItem("File I/O");
    combo->insertItem("File timing");
    if (toCurrentConnection(this).version()>="8.0") {
      combo->insertItem("Tablespace I/O");
      combo->insertItem("Tablespace timing");
    }
    connect(combo,SIGNAL(activated(int)),this,SLOT(changeCharts(int)));

    FileReads=new QGrid(2,Box);
    FileTime=new QGrid(2,Box);
    TablespaceReads=new QGrid(2,Box);
    TablespaceTime=new QGrid(2,Box);

    refresh();
    FileReads->setFixedWidth(viewport()->width()-50);
    FileTime->setFixedWidth(viewport()->width()-50);
    TablespaceReads->setFixedWidth(viewport()->width()-50);
    TablespaceTime->setFixedWidth(viewport()->width()-50);
    changeCharts(0);
  } TOCATCH
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

void toTuningFileIO::allocCharts(const QString &name,const QString &label)
{
  std::list<QString> labels;
  labels.insert(labels.end(),"Reads");
  labels.insert(labels.end(),"Blocks Read");
  labels.insert(labels.end(),"Writes");
  labels.insert(labels.end(),"Blocks Written");
  std::list<QString> labelTime;
  labelTime.insert(labelTime.end(),"Average");
  labelTime.insert(labelTime.end(),"Minimum");
  labelTime.insert(labelTime.end(),"Maximum Read");
  labelTime.insert(labelTime.end(),"Maximum Write");

  toBarChart *barchart;
  if (name.startsWith("tspc:"))
    barchart=new toBarChart(TablespaceReads);
  else
    barchart=new toBarChart(FileReads);
  ReadsCharts[name]=barchart;
  barchart->setTitle(name.mid(5));
  barchart->setMinimumSize(200,170);
  barchart->setYPostfix("blocks/s");
  barchart->setLabels(labels);

  toLineChart *linechart;
  if (name.startsWith("tspc:"))
    linechart=new toLineChart(TablespaceTime);
  else
    linechart=new toLineChart(FileTime);
  TimeCharts[name]=linechart;
  linechart->setTitle(name.mid(5));
  linechart->setMinimumSize(200,170);
  linechart->setYPostfix("ms");
  linechart->setLabels(labelTime);
}

void toTuningFileIO::saveSample(const QString &name,const QString &label,
				double reads,double writes,
				double readBlk,double writeBlk,
				double avgTim,double minTim,
				double maxRead,double maxWrite)
{
  time_t now=time(NULL);
  if (now!=LastStamp) {

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
	dispVal.insert(dispVal.end(),(*i-*j)/(now-LastStamp));
	i++;
	j++;
      }
    }
    LastValues[name]=vals;

    if (ReadsCharts.find(name)==ReadsCharts.end())
      allocCharts(name,label);

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
  try {
    toConnection &conn=toCurrentConnection(this);
    if (conn.version()<"8.0")
      return;
    toQList Files=toQuery::readQuery(conn,SQLFileIO);

    QString lastTablespace;
    double tblReads;
    double tblWrites;
    double tblReadBlk;
    double tblWriteBlk;
    double tblAvg;
    double tblMin;
    double tblMaxRead;
    double tblMaxWrite;
    for(;;) {
      QString tablespace=toShift(Files);
      QString datafile=toShift(Files);
      QString timestr=toShift(Files);
      if (tablespace!=lastTablespace) {
	if (!lastTablespace.isNull()) {
	  QString name="tspc:";
	  name+=lastTablespace;

	  saveSample(name,timestr,
		     tblReads,tblWrites,tblReadBlk,tblWriteBlk,
		     tblAvg,tblMin,tblMaxRead,tblMaxWrite);
	}

	tblReads=tblWrites=tblReadBlk=tblWriteBlk=tblAvg=tblMin=tblMaxRead=tblMaxWrite=0;
	lastTablespace=tablespace;
      }
      if (tablespace.isNull())
	break;

      double reads=toShift(Files).toDouble();
      double writes=toShift(Files).toDouble();
      double readBlk=toShift(Files).toDouble();
      double writeBlk=toShift(Files).toDouble();
      double avgTim=toShift(Files).toDouble();
      double minTim=toShift(Files).toDouble();
      double maxRead=toShift(Files).toDouble();
      double maxWrite=toShift(Files).toDouble();

      tblReads+=reads;
      tblWrites+=writes;
      tblReadBlk+=readBlk;
      tblWriteBlk+=writeBlk;
      tblAvg+=avgTim;
      tblMin+=minTim;
      tblMaxRead+=maxRead;
      tblMaxWrite+=maxWrite;

      QString name="file:";
      name+=datafile;

      saveSample(name,timestr,
		 reads,writes,readBlk,writeBlk,
		 avgTim,minTim,maxRead,maxWrite);

    }
    LastStamp=time(NULL);
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
  disconnect(toCurrentTool(this)->timer(),SIGNAL(timeout()),this,SLOT(refresh()));
}

void toTuningFileIO::start(void)
{
  connect(toCurrentTool(this)->timer(),SIGNAL(timeout()),this,SLOT(refresh()));
}

class toTuningWaitItem : public QListViewItem {
  int Color;
public:
  toTuningWaitItem(QListView *parent,QListViewItem *after,const QString &buf=QString::null)
    : QListViewItem(parent,after,QString::null,buf)
  { Color=0; }
  void setColor(int color)
  { Color=color; }
  virtual void paintCell(QPainter * p,const QColorGroup & cg,int column,int width,int align)
  {
    if (column==0) {
      QString ct=text(column);

      QBrush brush(isSelected()?toChartBrush(Color):QBrush(cg.base()));

      p->fillRect(0,0,width,height(),QBrush(brush.color()));
      if (brush.style()!=QBrush::SolidPattern)
	p->fillRect(0,0,width,height(),QBrush(Qt::white,brush.style()));
    } else
      QListViewItem::paintCell(p,cg,column,width,align);
  }
};

toTuningWait::toTuningWait(QWidget *parent,const char *name)
  : QFrame(parent,name)
{
  QGridLayout *layout=new QGridLayout(this);

  QToolBar *toolbar=toAllocBar(this,"Server Tuning",toCurrentConnection(this).description());
  layout->addMultiCellWidget(toolbar,0,0,0,2);
  new QLabel("Display ",toolbar);
  QComboBox *type=new QComboBox(toolbar);
  type->insertItem("Time");
  type->insertItem("Count");
  connect(type,SIGNAL(activated(int)),this,SLOT(changeType(int)));
  toolbar->setStretchableWidget(new QLabel("",toolbar));

  layout->setColStretch(0,1);
  layout->setColStretch(1,2);
  layout->setColStretch(2,2);

  Delta=new toBarChart(this);
  Delta->setTitle("System wait events");
  Delta->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  Delta->showLegend(false);
  Delta->setYPostfix(" ms/sec");
  layout->addMultiCellWidget(Delta,1,1,1,2);

  Types=new QListView(this);
  Types->addColumn("Color");
  Types->addColumn("Wait type");
  Types->setAllColumnsShowFocus(true);
  Types->setSelectionMode(QListView::Multi);
  Types->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  QString str=toTool::globalConfig(CONF_LIST,"");
  if (!str.isEmpty()) {
    QFont font(toStringToFont(str));
    Types->setFont(font);
  }
  layout->addMultiCellWidget(Types,1,2,0,0);

  connect(Types,SIGNAL(selectionChanged()),this,SLOT(changeSelection()));
  DeltaPie=new toPieChart(this);
  DeltaPie->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  DeltaPie->showLegend(false);
  layout->addWidget(DeltaPie,2,1);
  AbsolutePie=new toPieChart(this);
  AbsolutePie->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  AbsolutePie->showLegend(false);
  layout->addWidget(AbsolutePie,2,2);
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
  Query=NULL;
  start();
  connect(toCurrentTool(this),SIGNAL(connectionChange()),this,SLOT(connectionChanged()));
  First=true;
  ShowTimes=false;
}

void toTuningWait::changeType(int item)
{
  ShowTimes=item;
  if (ShowTimes)
    Delta->setYPostfix(" waits/sec");
  else
    Delta->setYPostfix(" ms/sec");
    
  changeSelection();
}

void toTuningWait::start(void)
{
  connect(toCurrentTool(this)->timer(),SIGNAL(timeout()),this,SLOT(refresh()));
}

void toTuningWait::stop(void)
{
  disconnect(toCurrentTool(this)->timer(),SIGNAL(timeout()),this,SLOT(refresh()));
}

void toTuningWait::changeSelection(void)
{

  int count=int(Labels.size());

  bool *enabled=new bool[count];
  bool *included=new bool[count];
  int typ=0;
  std::list<QString> used;
  std::map<QString,int> usedMap;
  {
    for (std::list<QString>::iterator i=Labels.begin();i!=Labels.end();i++) {
      usedMap[*i]=typ;
      enabled[typ]=false;
      included[typ]=false;
      typ++;
    }
  }
  for (QListViewItem *item=Types->firstChild();item;item=item->nextSibling()) {
    QString txt=item->text(1);
    if (usedMap.find(txt)==usedMap.end())
      toStatusMessage("Internal error, can't find ("+txt+") in usedMap");
    if (item->isSelected())
      enabled[usedMap[txt]]=true;
    used.insert(used.end(),txt);
    included[usedMap[txt]]=true;
  }
  used.sort();
  {
    for(std::list<QString>::iterator i=used.begin();i!=used.end();i++) {
      if (!enabled[usedMap[*i]])
	*i=QString::null;
    }
  }
      
  try {
    Delta->clear();

    std::list<double> lastAbsolute;
    std::list<double> relative;
    std::list<QString>::iterator xval=XValues.begin();
    time_t last;
    std::list<time_t>::iterator ctime=TimeStamp.begin();

    std::list<std::list<double> > &cur=(ShowTimes?Times:Values);

    {
      for(std::list<std::list<double> >::iterator i=cur.begin();
  	  i!=cur.end();i++) {
	typ=0;
	relative.clear();
	std::list<double> current;
	std::list<double>::iterator k=lastAbsolute.begin();
	for(std::list<double>::iterator j=(*i).begin();j!=(*i).end();j++) {
	  if (included[typ]) {
	    if (enabled[typ]) {
	      current.insert(current.end(),*j);
	      if (k!=lastAbsolute.end()) {
		relative.insert(relative.end(),max(double(0),((*j)-(*k))/((*ctime)-last)));
		k++;
	      }
	    } else {
	      current.insert(current.end(),0);
	      if (k!=lastAbsolute.end()) {
		relative.insert(relative.end(),0);
		k++;
	      }
	    }
	  }
	  typ++;
	}
	if (relative.size()>0&&xval!=XValues.end()) {
	  Delta->addValues(relative,*xval);
	  xval++;
	}
	lastAbsolute=current;
	if (ctime!=TimeStamp.end()) {
	  last=*ctime;
	  ctime++;
	}
      }
    }
    double total=0;
    {
      for (std::list<double>::iterator i=lastAbsolute.begin();i!=lastAbsolute.end();i++)
	total+=*i;
    }
    AbsolutePie->setValues(lastAbsolute,used);
    AbsolutePie->setTitle("Absolute system wait events\nTotal "+QString::number(total/1000)+" s");
    total=0;
    for (std::list<double>::iterator i=relative.begin();i!=relative.end();i++)
      total+=*i;
    DeltaPie->setValues(relative,used);
    if (total>0)
      DeltaPie->setTitle("Delta system wait events\nTotal "+QString::number(total)+" ms");
    else
      DeltaPie->setTitle(QString::null);
  } TOCATCH
  delete enabled;
  delete included;
}

void toTuningWait::connectionChanged(void)
{
  Values.clear();
  Labels.clear();
  First=true;
}

void toTuningWait::poll(void)
{
  try {
    if (Query&&Query->poll()) {
      while(Query->poll()&&!Query->eof()) {
	QString cur=Query->readValueNull();
	if (First)
	  Labels.insert(Labels.end(),cur);
	Now=Query->readValueNull();
	Current.insert(Current.end(),Query->readValueNull().toDouble());
	CurrentTimes.insert(CurrentTimes.end(),Query->readValueNull().toDouble());
      }
      if (Query->eof()) {
	QListViewItem *item=NULL;
	std::map<QString,bool> types;
	int typ=0;
	{
	  for(QListViewItem *ci=Types->firstChild();ci;ci=ci->nextSibling()) {
	    types[ci->text(1)]=true;
	    item=ci;
	    typ++;
	  }
	}

	std::list<double>::iterator j=CurrentTimes.begin();
	for(std::list<QString>::iterator i=Labels.begin();i!=Labels.end();i++) {
	  if ((*j)!=0&&!types[*i]) {
	    item=new toTuningWaitItem(Types,item,*i);
	    item->setSelected(First);
	    types[*i]=typ;
	    typ++;
	    Types->setSorting(1);
	  }
	  j++;
	}
	if (First)
	  First=false;
	else
	  XValues.insert(XValues.end(),Now);
	int col=0;
	for(QListViewItem *ci=Types->firstChild();ci;ci=ci->nextSibling()) {
	  toTuningWaitItem *item=dynamic_cast<toTuningWaitItem *>(ci);
	  if (item)
	    item->setColor(col);
	  col++;
	}
	TimeStamp.insert(TimeStamp.end(),time(NULL));
	Values.insert(Values.end(),Current);
	Times.insert(Times.end(),CurrentTimes);
	if (int(Values.size())>Delta->samples()+1) {
	  Values.erase(Values.begin());
	  Times.erase(Times.begin());
	  XValues.erase(XValues.begin());
	  TimeStamp.erase(TimeStamp.begin());
	}
	changeSelection();
	delete Query;
	Query=NULL;
	Poll.stop();
      }
    }
  } catch(const QString &exc) {
    delete Query;
    Query=NULL;
    Poll.stop();
    toStatusMessage(exc);
  }
}

static toSQL SQLWaitEvents("toTuning:WaitEvents",
			   "select b.name,sysdate,a.time_waited*10,a.total_waits\n"
			   "  from v$system_event a,v$event_name b\n"
			   " where b.name=a.event(+)\n"
			   " order by b.name",
			   "Get all available system wait events");

void toTuningWait::refresh(void)
{
  if (Query||(TimeStamp.begin()!=TimeStamp.end()&&(*TimeStamp.rbegin())==time(NULL)))
    return;
  toConnection &conn=toCurrentTool(this)->connection();
  toQList par;
  Current.clear();
  CurrentTimes.clear();
  Query=new toNoBlockQuery(conn,toSQL::string(SQLWaitEvents,conn),par);
  Poll.start(100);
}
