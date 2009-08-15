
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tonoblockquery.h"
#include "toresultbar.h"
#include "toresultitem.h"
#include "toresultline.h"
#include "toresultlock.h"
#include "toresulttableview.h"
#include "toresultparam.h"
#include "toresultpie.h"
#include "toresultstats.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"
#include "totuning.h"
#include "ui_totuningsettingui.h"
#include "towaitevents.h"

#include <time.h>
#include <stdio.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qpainter.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <QMdiArea>

#include <QResizeEvent>
#include <QPixmap>
#include <QString>
#include <QAction>
#include <QVBoxLayout>
#include <QGridLayout>

#include "icons/refresh.xpm"
#include "icons/totuning.xpm"
#include "icons/compile.xpm"

// #define CONF_OVERVIEW "Overview"
// #define CONF_FILEIO   "File I/O"
// #define CONF_WAITS    "Wait events"
// #define CONF_CHART    "chart"


static std::list<QString> TabList(void)
{
    std::list<QString> ret;
    ret.insert(ret.end(), CONF_OVERVIEW);
    std::list<QString> val = toSQL::range("toTuning:Charts");
    QString last;
    for (std::list<QString>::iterator i = val.begin();i != val.end();i++)
    {
        QStringList parts = (*i).split(":");
        if (parts.count() == 3)
        {
            parts.append(parts[2]);
            parts[2] = QString::fromLatin1("Charts");
        }
        if (last != parts[2])
            ret.insert(ret.end(), parts[2].toLatin1());
        last = parts[2];
    }
    ret.insert(ret.end(), CONF_WAITS);
    ret.insert(ret.end(), CONF_FILEIO);
    return ret;
}

class toTuningSetup : public QWidget, public Ui::toTuningSettingUI, public toSettingTab
{
    toTool *Tool;
public:
    toTuningSetup(toTool *tool, QWidget* parent = 0, const char* name = 0)
            : QWidget(parent), toSettingTab("tuning.html#preferences"), Tool(tool)
    {
        setupUi(this);
//         std::list<QString> tabs = TabList();
//         for (std::list<QString>::iterator i = tabs.begin();i != tabs.end();i++)
//         {
//             toTreeWidgetItem *item = new toTreeWidgetItem(EnabledTabs, (*i));
//             if (!tool->config(*i, "").isEmpty())
//                 item->setSelected(true);
//         }
        toTreeWidgetItem *item = new toTreeWidgetItem(EnabledTabs, CONF_OVERVIEW);
        item->setSelected(toConfigurationSingle::Instance().tuningOverview());
        toTreeWidgetItem *item1 = new toTreeWidgetItem(EnabledTabs, CONF_FILEIO);
        item1->setSelected(toConfigurationSingle::Instance().tuningFileIO());
        toTreeWidgetItem *item2 = new toTreeWidgetItem(EnabledTabs, CONF_WAITS);
        item2->setSelected(toConfigurationSingle::Instance().tuningWaits());
        toTreeWidgetItem *item3 = new toTreeWidgetItem(EnabledTabs, CONF_CHART);
        item3->setSelected(toConfigurationSingle::Instance().tuningCharts());

        EnabledTabs->setSorting(0);
    }
    virtual void saveSetting(void)
    {
        for (toTreeWidgetItem *item = EnabledTabs->firstChild();item;item = item->nextSibling())
        {
            // NOTE: OK, it's ugly, but this is the only place where new QSettings fails
            if (item->text(0) == CONF_OVERVIEW)
                toConfigurationSingle::Instance().setTuningOverview(item->isSelected());
            else if (item->text(0) == CONF_FILEIO)
                toConfigurationSingle::Instance().setTuningFileIO(item->isSelected());
            else if (item->text(0) == CONF_WAITS)
                toConfigurationSingle::Instance().setTuningWaits(item->isSelected());
            else if (item->text(0) == CONF_CHART)
                toConfigurationSingle::Instance().setTuningCharts(item->isSelected());

//             if (item->isSelected() || Tool->config(item->text(0).toLatin1(), "Undefined") != "Undefined")
//                 Tool->setConfig(item->text(0).toLatin1(), QString::fromLatin1((item->isSelected() ? "Yes" : "")));
        }
    }
};

class toTuningTool : public toTool
{
protected:
    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(totuning_xpm);
    }
public:
    toTuningTool()
            : toTool(30, "Server Tuning")
    { }
    virtual const char *menuItem()
    {
        return "Server Tuning";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toTuning(parent, connection);
    }
    virtual QWidget *configurationTab(QWidget *parent)
    {
        return new toTuningSetup(this, parent);
    }
    virtual void closeWindow(toConnection &connection){};
};

static toTuningTool TuningTool;

static toSQL SQLDictionaryMiss("toTuning:Indicators:Important ratios:1DictionaryMiss",
                               "select sum(getmisses)/sum(gets)*100,' %' from v$rowcache",
                               "Data dictionary row cache miss ratio (%). < 10%-15%");

static toSQL SQLImportantLibraryCache("toTuning:Indicators:Important ratios:2LibraryCache",
                                      "select sum(reloads)/sum(pins)*100,' %' from v$librarycache",
                                      "Library cache (Shared SQL) miss ratio (%). < 1%");

static toSQL SQLDataCache(
    "toTuning:Indicators:Important ratios:3DataCache",
    "SELECT ( 1 - SUM ( DECODE ( name,\n"
    "                            'physical reads',\n"
    "                            value,\n"
    "                            0 ) ) / SUM ( DECODE ( name,\n"
    "                                                   'db block gets',\n"
    "                                                   value,\n"
    "                                                   'consistent gets',\n"
    "                                                   value,\n"
    "                                                   0 ) ) ) * 100,\n"
    "       ' %'\n"
    "  FROM v$sysstat\n"
    " WHERE name IN ( 'db block gets',\n"
    "                 'consistent gets',\n"
    "                 'physical reads' )",
    "Data buffer cache hit ratio (%). > 60% - 70%",
    "0800");

static toSQL SQLDataCache7(
    "toTuning:Indicators:Important ratios:3DataCache",
    "SELECT (1-SUM(DECODE(statistic#,39,value,0))/SUM(DECODE(statistic#,37,value,38,value,0)))*100,' %'\n"
    "  FROM v$sysstat\n"
    " WHERE statistic# IN (37,38,39)",
    "",
    "0703");

static toSQL SQLLogRedo(
    "toTuning:Indicators:Redo log contention:1LogSpace",
    "SELECT value\n"
    "  FROM v$sysstat\n"
    " WHERE name = 'redo log space requests'",
    "Redo log space requests. Close to 0",
    "0800");

static toSQL SQLLogRedo7(
    "toTuning:Indicators:Redo log contention:1LogSpace",
    "select value from v$sysstat where statistic# = 94",
    "",
    "0703");

static toSQL SQLSystemHeadUndo(
    "toTuning:Indicators:RBS contention:1SystemHeadUndo",
    "SELECT count / blocks * 100,\n"
    "       ' %'\n"
    "  FROM ( SELECT MAX ( count ) count\n"
    "           FROM v$waitstat\n"
    "          WHERE class = 'system undo header' ) a,\n"
    "       ( SELECT SUM ( value ) blocks\n"
    "           FROM v$sysstat\n"
    "          WHERE name IN ( 'db block gets',\n"
    "                          'consistent gets' ) ) b",
    "System undo header waits (%). < 1%",
    "0800");

static toSQL SQLSystemHeadUndo7(
    "toTuning:Indicators:RBS contention:1SystemHeadUndo",
    "SELECT count/blocks*100,' %'\n"
    "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo header') a,\n"
    "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
    "",
    "0703");

static toSQL SQLSystemBlockUndo(
    "toTuning:Indicators:RBS contention:2SystemBlockUndo",
    "SELECT count / blocks * 100,\n"
    "       ' %'\n"
    "  FROM ( SELECT MAX ( count ) count\n"
    "           FROM v$waitstat\n"
    "          WHERE class = 'system undo block' ) a,\n"
    "       ( SELECT SUM ( value ) blocks\n"
    "           FROM v$sysstat\n"
    "          WHERE name IN ( 'db block gets',\n"
    "                          'consistent gets' ) ) b",
    "System undo block waits (%). < 1%",
    "0800");

static toSQL SQLSystemBlockUndo7(
    "toTuning:Indicators:RBS contention:2SystemBlockUndo",
    "SELECT count/blocks*100,' %'\n"
    "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo block') a,\n"
    "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
    "",
    "0703");

static toSQL SQLHeadUndo(
    "toTuning:Indicators:RBS contention:3HeadUndo",
    "SELECT count / blocks * 100,\n"
    "       ' %'\n"
    "  FROM ( SELECT MAX ( count ) count\n"
    "           FROM v$waitstat\n"
    "          WHERE class = 'undo header' ) a,\n"
    "       ( SELECT SUM ( value ) blocks\n"
    "           FROM v$sysstat\n"
    "          WHERE name IN ( 'db block gets',\n"
    "                          'consistent gets' ) ) b",
    "Undo head waits (%). < 1%",
    "0800");

static toSQL SQLHeadUndo7(
    "toTuning:Indicators:RBS contention:3HeadUndo",
    "SELECT count/blocks*100,' %'\n"
    "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo header') a,\n"
    "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
    "",
    "0703");

static toSQL SQLBlockUndo(
    "toTuning:Indicators:RBS contention:4BlockUndo",
    "SELECT count / blocks * 100,\n"
    "       ' %'\n"
    "  FROM ( SELECT MAX ( count ) count\n"
    "           FROM v$waitstat\n"
    "          WHERE class = 'undo block' ) a,\n"
    "       ( SELECT SUM ( value ) blocks\n"
    "           FROM v$sysstat\n"
    "          WHERE name IN ( 'db block gets',\n"
    "                          'consistent gets' ) ) b",
    "Undo block waits (%). < 1%",
    "0800");

static toSQL SQLBlockUndo7(
    "toTuning:Indicators:RBS contention:4BlockUndo",
    "SELECT count/blocks*100,' %'\n"
    "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo block') a,\n"
    "       (SELECT SUM(value) blocks FROM v$sysstat WHERE statistic# IN (37,38)) b",
    "",
    "0703");

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

static toSQL SQLChartsPhysical(
    "toTuning:Charts:1BBPhysical I/O",
    "SELECT SYSDATE,\n"
    "       sum ( DECODE ( name,\n"
    "                      'physical reads',\n"
    "                      value,\n"
    "                      0 ) ) \"Physical reads\",\n"
    "       sum ( DECODE ( name,\n"
    "                      'physical writes',\n"
    "                      value,\n"
    "                      0 ) ) \"Physical writes\",\n"
    "       sum ( DECODE ( name,\n"
    "                      'redo writes',\n"
    "                      value,\n"
    "                      0 ) ) \"Redo writes\"\n"
    "  FROM v$sysstat\n"
    " WHERE name IN ( 'physical reads',\n"
    "                 'physical writes',\n"
    "                 'redo writes' )",
    "Generate chart of physical I/O of database.",
    "0800");

static toSQL SQLChartsPhysical7(
    "toTuning:Charts:1BBPhysical I/O",
    "select SYSDATE,\n"
    "       sum(decode(statistic#,39,value,0)) \"Physical reads\",\n"
    "       sum(decode(statistic#,40,value,0)) \"Physical writes\",\n"
    "       sum(decode(statistic#,91,value,0)) \"Redo writes\"\n"
    "  from v$sysstat where statistic# in (39,40,91)",
    "",
    "0703");

static toSQL SQLChartsLogical(
    "toTuning:Charts:2BBLogical I/O",
    "SELECT SYSDATE,\n"
    "       sum ( DECODE ( name,\n"
    "                      'db block gets',\n"
    "                      value,\n"
    "                      0 ) ) \"Block gets\",\n"
    "       sum ( DECODE ( name,\n"
    "                      'consistent gets',\n"
    "                      value,\n"
    "                      0 ) ) \"Consistent gets\",\n"
    "       sum ( DECODE ( name,\n"
    "                      'db block changes',\n"
    "                      value,\n"
    "                      0 ) ) \"Block changes\",\n"
    "       sum ( DECODE ( name,\n"
    "                      'consistent changes',\n"
    "                      value,\n"
    "                      0 ) ) \"Consistent changes\"\n"
    "  FROM v$sysstat\n"
    " WHERE name IN ( 'db block gets',\n"
    "                 'consistent gets',\n"
    "                 'db block changes',\n"
    "                 'consistent changes' )",
    "Generate chart of physical I/O of database.",
    "0800");

static toSQL SQLChartsLogical7(
    "toTuning:Charts:2BBLogical I/O",
    "select SYSDATE,\n"
    "       sum(decode(statistic#,37,value,0)) \"Block gets\",\n"
    "       sum(decode(statistic#,38,value,0)) \"Consistent gets\",\n"
    "       sum(decode(statistic#,43,value,0)) \"Block changes\",\n"
    "       sum(decode(statistic#,45,value,0)) \"Consistent changes\"\n"
    "  from v$sysstat where statistic# in (37,38,43,45)",
    "",
    "0703");

static toSQL SQLChartsWait(
    "toTuning:Charts:3BMWait events",
    "SELECT SYSDATE,\n"
    "       cpu * 10 \"CPU\",\n"
    "       PARALLEL * 10 \"Parallel execution\",\n"
    "       filewrite * 10 \"DB File Write\",\n"
    "       writecomplete * 10 \"Write Complete\",\n"
    "       fileread * 10 \"DB File Read\",\n"
    "       singleread * 10 \"DB Single File Read\",\n"
    "       control * 10 \"Control File I/O\",\n"
    "       direct * 10 \"Direct I/O\",\n"
    "       LOG * 10 \"Log file\",\n"
    "       net * 10 \"SQL*Net\"\n"
    "  FROM ( SELECT   SUM ( DECODE ( SUBSTR ( event,\n"
    "                                          1,\n"
    "                                          2 ),\n"
    "                                 'PX',\n"
    "                                 time_waited,\n"
    "                                 0 ) ) - SUM ( DECODE ( event,\n"
    "                                                        'PX Idle Wait',\n"
    "                                                        time_waited,\n"
    "                                                        0 ) ) as parallel,\n"
    "                  SUM ( DECODE ( event,\n"
    "                                 'db file parallel write',\n"
    "                                 time_waited,\n"
    "                                 'db file single write',\n"
    "                                 time_waited,\n"
    "                                 0 ) ) filewrite,\n"
    "                  SUM ( DECODE ( event,\n"
    "                                 'write complete waits',\n"
    "                                 time_waited,\n"
    "                                 NULL ) ) writecomplete,\n"
    "                  SUM ( DECODE ( event,\n"
    "                                 'db file parallel read',\n"
    "                                 time_waited,\n"
    "                                 'db file sequential read',\n"
    "                                 time_waited,\n"
    "                                 0 ) ) fileread,\n"
    "                  SUM ( DECODE ( event,\n"
    "                                 'db file scattered read',\n"
    "                                 time_waited,\n"
    "                                 0 ) ) singleread,\n"
    "                  SUM ( DECODE ( SUBSTR ( event,\n"
    "                                          1,\n"
    "                                          12 ),\n"
    "                                 'control file',\n"
    "                                 time_waited,\n"
    "                                 0 ) ) control,\n"
    "                  SUM ( DECODE ( SUBSTR ( event,\n"
    "                                          1,\n"
    "                                          11 ),\n"
    "                                 'direct path',\n"
    "                                 time_waited,\n"
    "                                 0 ) ) direct,\n"
    "                  SUM ( DECODE ( SUBSTR ( event,\n"
    "                                          1,\n"
    "                                          3 ),\n"
    "                                 'log',\n"
    "                                 time_waited,\n"
    "                                 0 ) ) as log,\n"
    "                  SUM ( DECODE ( SUBSTR ( event,\n"
    "                                          1,\n"
    "                                          7 ),\n"
    "                                 'SQL*Net',\n"
    "                                 time_waited,\n"
    "                                 0 ) ) - SUM ( DECODE ( event,\n"
    "                                                        'SQL*Net message from client',\n"
    "                                                        time_waited,\n"
    "                                                        0 ) ) net\n"
    "             FROM v$system_event ),\n"
    "       ( SELECT value * 10 cpu\n"
    "           FROM v$sysstat\n"
    "          WHERE name = 'CPU used by this session' )",
    "Used to generate chart for system wait time.");

static toSQL SQLChartsExecution(
    "toTuning:Charts:6LNExecution",
    "select SYSDATE,\n"
    "       sum(decode(name,'execute count',value,0)) \"Execute\",\n"
    "       sum(decode(name,'parse count (hard)',value,0)) \"Hard parse\",\n"
    "       sum(decode(name,'parse count (total)',value,0)) \"Parse\",\n"
    "       sum(decode(name,'user calls',value,0)) \"Calls\",\n"
    "       sum(decode(name,'user commits',value,0))  \"Commit\",\n"
    "       sum(decode(name,'user rollbacks',value,0)) \"Rollbacks\"\n"
    "  from v$sysstat where name IN ('execute count','parse count (hard)','parse count (total)','user calls','user commits','user rollbacks')",
    "Used to generate chart for execution statistics.",
    "0800");


static toSQL SQLChartsExecution7(
    "toTuning:Charts:6LNExecution",
    "select SYSDATE,\n"
    "       sum(decode(statistic#,132,value,0)) \"Execute\",\n"
    "       sum(decode(statistic#,131,value,0)) \"Parse\",\n"
    "       sum(decode(statistic#,6,value,0)) \"Calls\",\n"
    "       sum(decode(statistic#,4,value,0))  \"Commit\",\n"
    "       sum(decode(statistic#,5,value,0)) \"Rollbacks\"\n"
    "  from v$sysstat where statistic# in (132,131,4,5,6)",
    "",
    "0703");

static toSQL SQLChartsExecutionPie(
    "toTuning:Charts:8PNExecution Total",
    "select value,decode(name,'execute count','Execute','parse count (hard)','Hard parse','parse count (total)','Parse',\n"
    "                              'user calls','Calls','user commits','Commit','user rollbacks','Rollbacks')\n"
    "  from v$sysstat where name IN ('execute count','parse count (hard)','parse count (total)','user calls','user commits','user rollbacks') order by name",
    "Used to generate pie chart for execution statistics.",
    "0800");


static toSQL SQLChartsExecutionPie7(
    "toTuning:Charts:8PNExecution Total",
    "select value,decode(statistic#,132,'Execute',131,'Parse',\n"
    "                              6,'Calls',4,'Commit',5,'Rollbacks')\n"
    "  from v$sysstat where statistic# in (132,131,6,4,5) order by name",
    "",
    "0703");

static toSQL SQLChartsClients(
    "toTuning:Charts:4BAClients",
    "select sysdate,\n"
    "       sum(decode(decode(type,'BACKGROUND','WHATEVER',status),'ACTIVE',1,0)) \"Active\",\n"
    "       sum(decode(status,'INACTIVE',1,0)) \"Inactive\",\n"
    "       sum(decode(type,'BACKGROUND',1,0)) \"System\"\n"
    "  from v$session where sid not in (select nvl(sid,0) from v$px_process)",
    "Chart displaying connected clients");

static toSQL SQLChartsClients8(
    "toTuning:Charts:4BAClients",
    "select sysdate,\n"
    "       sum(decode(decode(type,'BACKGROUND','WHATEVER',status),'ACTIVE',1,0)) \"Active\",\n"
    "       sum(decode(status,'INACTIVE',1,0)) \"Inactive\",\n"
    "       sum(decode(type,'BACKGROUND',1,0)) \"System\"\n"
    "  from v$session",
    "",
    "0800");

static toSQL SQLChartsCacheMisses(
    "toTuning:Charts:5CPCache misses",
    "SELECT SYSDATE,\n"
    "       pread \"Data buffer cache\",\n"
    "       read \"Dictionary row cache\",\n"
    "       getmiss \"Library cache\",\n"
    "       gets \" \",\n"
    "       reloads \" \",\n"
    "       pins \" \"\n"
    "  FROM ( SELECT 100 * SUM ( DECODE ( name,\n"
    "                                     'physical reads',\n"
    "                                     value,\n"
    "                                     0 ) ) pread,\n"
    "                SUM ( DECODE ( name,\n"
    "                               'db block gets',\n"
    "                               value,\n"
    "                               'consistent gets',\n"
    "                               value,\n"
    "                               41,\n"
    "                               value,\n"
    "                               0 ) ) as read\n"
    "           FROM v$sysstat\n"
    "          WHERE name IN ( 'db block gets',\n"
    "                          'consistent gets',\n"
    "                          'physical reads' ) ) \"Data buffer cache\",\n"
    "       ( SELECT 100 * sum ( getmisses ) getmiss,\n"
    "                sum ( gets ) gets\n"
    "           FROM v$rowcache ) \"Dictionary row cache\",\n"
    "       ( SELECT 100 * sum ( reloads ) reloads,\n"
    "                sum ( pins ) pins\n"
    "           FROM v$librarycache ) \"Library cache\"",
    "Chart display memory allocation. This is really weird to change, especially since the column names "
    "don't correspond exactly to the column data in the chart. Each group of two are divided with each other before drawn.",
    "0800");

static toSQL SQLChartsCacheMisses7(
    "toTuning:Charts:5CPCache misses",
    "select sysdate,\n"
    "       pread \"Data buffer cache\",read \"Dictionary row cache\",\n"
    "       getmiss \"Library cache\",gets \" \",\n"
    "       reloads \" \",pins \" \"\n"
    "  from (select 100*SUM(DECODE(statistic#,39,value,0)) pread,SUM(DECODE(statistic#,37,value,38,value,0)) read\n"
    "          from v$sysstat where statistic# IN (37,38,39)) \"Data buffer cache\",\n"
    "       (select 100*sum(getmisses) getmiss,sum(gets) gets from v$rowcache) \"Dictionary row cache\",\n"
    "       (select 100*sum(reloads) reloads,sum(pins) pins from v$librarycache) \"Library cache\"\n",
    "",
    "0703");

static toSQL SQLChartsRedo(
    "toTuning:Charts:7BSRedo log I/O",
    "SELECT SYSDATE,\n"
    "       sum ( DECODE ( name,\n"
    "                      'redo size',\n"
    "                      value,\n"
    "                      0 ) ) / :unit<int> \"Redo size\",\n"
    "       sum ( DECODE ( name,\n"
    "                      'redo wastage',\n"
    "                      value,\n"
    "                      0 ) ) / :unit<int> \"Redo wastage\"\n"
    "  FROM v$sysstat\n"
    " WHERE name IN ( 'redo size',\n"
    "                 'redo wastage' )",
    "Used to generate chart for redo I/O statistics.",
    "0800");

static toSQL SQLChartsRedo7(
    "toTuning:Charts:7BSRedo log I/O",
    "select SYSDATE,\n"
    "       sum(decode(statistic#,85,value,0))/:unit<int> \"Redo size\",\n"
    "       sum(decode(statistic#,89,value,0))/:unit<int> \"Redo wastage\"\n"
    "  from v$sysstat where statistic# in (85,89)",
    "",
    "0703");

static toSQL SQLOverviewArchiveWrite("toTuning:Overview:ArchiveWrite",
                                     "select sysdate,sum(blocks) from v$archived_log",
                                     "Archive log write",
                                     "0800");

static toSQL SQLOverviewArchiveWrite7("toTuning:Overview:ArchiveWrite",
                                      "select sysdate,0 from sys.dual",
                                      "",
                                      "0703");

static toSQL SQLOverviewBufferHit(
    "toTuning:Overview:BufferHit",
    "SELECT SYSDATE,\n"
    "       ( 1 - SUM ( DECODE ( name,\n"
    "                            'physical reads',\n"
    "                            value,\n"
    "                            0 ) ) / SUM ( DECODE ( name,\n"
    "                                                   'db block gets',\n"
    "                                                   value,\n"
    "                                                   'consistent gets',\n"
    "                                                   value,\n"
    "                                                   0 ) ) ) * 100\n"
    "  FROM v$sysstat\n"
    " WHERE name IN ( 'db block gets',\n"
    "                 'consistent gets',\n"
    "                 'physical reads' )",
    "Buffer hitrate",
    "0800");

static toSQL SQLOverviewBufferHit7(
    "toTuning:Overview:BufferHit",
    "SELECT SYSDATE,(1-SUM(DECODE(statistic#,39,value,0))/SUM(DECODE(statistic#,37,value,38,value,0)))*100\n"
    "  FROM v$sysstat\n"
    " WHERE statistic# IN (37,38,39)",
    "",
    "0703");

static toSQL SQLOverviewClientInput(
    "toTuning:Overview:ClientInput",
    "SELECT SYSDATE,\n"
    "       value / :f1<int>\n"
    "  FROM v$sysstat\n"
    " WHERE name = 'bytes sent via SQL*Net to client'",
    "Bytes sent to client",
    "0800");

static toSQL SQLOverviewClientInput7(
    "toTuning:Overview:ClientInput",
    "select sysdate,value/:f1<int>\n"
    "  from v$sysstat where statistic# = 134",
    "",
    "0703");

static toSQL SQLOverviewClientOutput(
    "toTuning:Overview:ClientOutput",
    "SELECT SYSDATE,\n"
    "       value / :f1<int>\n"
    "  FROM v$sysstat\n"
    " WHERE name = 'bytes received via SQL*Net from client'",
    "Bytes sent from client",
    "0800");

static toSQL SQLOverviewClientOutput7(
    "toTuning:Overview:ClientOutput",
    "select sysdate,value/:f1<int>\n"
    "  from v$sysstat where statistic# = 133",
    "",
    "0703");

static toSQL SQLOverviewExecute(
    "toTuning:Overview:Execute",
    "SELECT SYSDATE,\n"
    "       value\n"
    "  FROM v$sysstat\n"
    " WHERE name = 'execute count'",
    "Execute count",
    "0800");

static toSQL SQLOverviewExecute7(
    "toTuning:Overview:Execute",
    "select sysdate,value\n"
    "  from v$sysstat where statistic# = 132",
    "",
    "0703");

static toSQL SQLOverviewParse(
    "toTuning:Overview:Parse",
    "SELECT SYSDATE,\n"
    "       value\n"
    "  FROM v$sysstat\n"
    " WHERE name = 'parse count (total)'",
    "Parse count",
    "0800");

static toSQL SQLOverviewParse7(
    "toTuning:Overview:Parse",
    "select sysdate,value\n"
    "  from v$sysstat where statistic# = 131",
    "",
    "0703");

static toSQL SQLOverviewRedoEntries(
    "toTuning:Overview:RedoEntries",
    "SELECT SYSDATE,\n"
    "       value\n"
    "  FROM v$sysstat\n"
    " WHERE name = 'redo entries'",
    "Redo entries",
    "0800");

static toSQL SQLOverviewRedoEntries7(
    "toTuning:Overview:RedoEntries",
    "select sysdate,value\n"
    "  from v$sysstat where statistic# = 84",
    "",
    "0703");

static toSQL SQLOverviewRedoBlocks(
    "toTuning:Overview:RedoBlocks",
    "SELECT SYSDATE,\n"
    "       value\n"
    "  FROM v$sysstat\n"
    " WHERE name = 'redo blocks written'",
    "Redo blocks written",
    "0800");

static toSQL SQLOverviewRedoBlocks7(
    "toTuning:Overview:RedoBlocks",
    "select sysdate,value\n"
    "  from v$sysstat where statistic# = 92",
    "",
    "0703");

static toSQL SQLOverviewLogicalRead(
    "toTuning:Overview:LogicalRead",
    "SELECT SYSDATE,\n"
    "       sum ( value )\n"
    "  FROM v$sysstat\n"
    " WHERE name IN ( 'db block gets',\n"
    "                 'consistent gets' )",
    "Blocks read",
    "0800");

static toSQL SQLOverviewLogicalRead7(
    "toTuning:Overview:LogicalRead",
    "select sysdate,sum(value)\n"
    "  from v$sysstat where statistic# in (37,38)",
    "",
    "0703");

static toSQL SQLOverviewLogicalWrite(
    "toTuning:Overview:LogicalWrite",
    "SELECT SYSDATE,\n"
    "       sum ( value )\n"
    "  FROM v$sysstat\n"
    " WHERE name IN ( 'db block changes',\n"
    "                 'consistent changes' )",
    "Blocks written",
    "0800");

static toSQL SQLOverviewLogicalWrite7(
    "toTuning:Overview:LogicalWrite",
    "select sysdate,sum(value)\n"
    "  from v$sysstat where statistic# in (43,45)",
    "",
    "0703");

static toSQL SQLOverviewPhysicalRead(
    "toTuning:Overview:PhysicalRead",
    "SELECT SYSDATE,\n"
    "       value\n"
    "  FROM v$sysstat\n"
    " WHERE name = 'physical reads'",
    "Blocks physically read",
    "0800");

static toSQL SQLOverviewPhysicalRead7(
    "toTuning:Overview:PhysicalRead",
    "select sysdate,value\n"
    "  from v$sysstat where statistic# = 39",
    "",
    "0703");

static toSQL SQLOverviewPhysicalWrite(
    "toTuning:Overview:PhysicalWrite",
    "SELECT SYSDATE,\n"
    "       value\n"
    "  FROM v$sysstat\n"
    " WHERE name = 'physical writes'",
    "Blocks physically written",
    "0800");

static toSQL SQLOverviewPhysicalWrite7(
    "toTuning:Overview:PhysicalWrite",
    "select sysdate,value\n"
    "  from v$sysstat where statistic# = 40",
    "",
    "0703");

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
                                "0800");

static toSQL SQLOverviewSGAUsed("toTuning:Overview:SGAUsed",
                                "select sysdate,100*(total-free)/total\n"
                                "  from (select sum(value) total from v$sga where name in ('Fixed Size','Variable Size')),\n"
                                "       (select bytes free from v$sgastat where pool = 'shared pool' and name = 'free memory')",
                                "SGA used",
                                "0800");

static toSQL SQLOverviewSGAUsed7("toTuning:Overview:SGAUsed",
                                 "select sysdate,100*(total-free)/total\n"
                                 "  from (select sum(value) total from v$sga where name in ('Fixed Size','Variable Size')),\n"
                                 "       (select bytes free from v$sgastat where name = 'free memory')",
                                 "",
                                 "0703");

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

void toTuningOverview::setupChart(toResultLine *chart, const QString &title, const QString &postfix, const toSQL &sql)
{
    chart->setMinValue(0);
    chart->showGrid(0);
    chart->showLegend(false);
    chart->showAxisLegend(false);
    chart->setTitle(title);
    chart->showLast(true);
    toQList val;
    if (postfix == QString::fromLatin1("b/s"))
    {
        QString unitStr(toConfigurationSingle::Instance().sizeUnit());
        val.insert(val.end(), toQValue(toSizeDecode(unitStr)));
        unitStr += QString::fromLatin1("/s");
        chart->setYPostfix(unitStr);
    }
    else
        chart->setYPostfix(postfix);
    chart->query(sql, val);
}

toTuningOverview::toTuningOverview(QWidget *parent, const char *name, Qt::WFlags fl)
        : QWidget(parent)
{
    setupUi(this);

    BackgroundGroup->setLayout(new QVBoxLayout);

    setupChart(ArchiveWrite, tr("< Archive write"), " " + tr("blocks/s"), SQLOverviewArchiveWrite);
    setupChart(BufferHit, tr("Hitrate"), QString::fromLatin1("%"), SQLOverviewBufferHit);
    BufferHit->setMaxValue(100);
    BufferHit->setFlow(false);
    setupChart(ClientInput, tr("< Client input"), QString::fromLatin1("b/s"), SQLOverviewClientInput);
    setupChart(ClientOutput, tr("Client output >"), QString::fromLatin1("b/s"), SQLOverviewClientOutput);
    setupChart(ExecuteCount, tr("Executes >"), QString::fromLatin1("/s"), SQLOverviewExecute);
    setupChart(LogWrite, tr("Log writer >"), " " + tr("blocks/s"), SQLOverviewRedoBlocks);
    setupChart(LogicalChange, tr("Buffer changed >"), tr(" blocks/s"), SQLOverviewLogicalWrite);
    setupChart(LogicalRead, tr("< Buffer gets"), tr(" blocks/s"), SQLOverviewLogicalRead);
    setupChart(ParseCount, tr("Parse >"), QString::fromLatin1("/s"), SQLOverviewParse);
    setupChart(PhysicalRead, tr("< Physical read"), tr(" blocks/s"), SQLOverviewPhysicalRead);
    setupChart(PhysicalWrite, tr("Physical write >"), tr(" blocks/s"), SQLOverviewPhysicalWrite);
    setupChart(RedoEntries, tr("Redo entries >"), QString::fromLatin1("/s"), SQLOverviewRedoEntries);
    setupChart(Timescale, tr("Timescale"), QString::null, SQLOverviewTimescale);
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

    try
    {
        toQList val;
        val.insert(val.end(), toQValue(toSizeDecode(toConfigurationSingle::Instance().sizeUnit())));
        FileUsed->query(toSQL::string(SQLOverviewFilespace, toCurrentConnection(this)), (const toQList)val);
    }
    TOCATCH
    FileUsed->showLegend(false);

    Done.up();
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));

    // Will be called later anyway
    //refresh();
}

toTuningOverview::~toTuningOverview()
{
    if (Done.getValue() == 0)
    {
        Quit = true;
        Done.down();
    }
}

void toTuningOverview::stop(void)
{
    try
    {
        disconnect(toCurrentTool(this)->timer(), SIGNAL(timeout()), this, SLOT(refresh()));
    }
    TOCATCH

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
    try
    {
        connect(toCurrentTool(this)->timer(), SIGNAL(timeout()), this, SLOT(refresh()));
    }
    TOCATCH

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
                                "0800");

static toSQL SQLOverviewArchive7("toTuning:Overview:Archive",
                                 "select 'N/A',\n"
                                 "       'N/A'\n"
                                 "  from sys.dual where 0 != :f1<int>",
                                 "",
                                 "0703");

static toSQL SQLOverviewLog("toTuning:Overview:Log",
                            "select count(1),\n"
                            "       max(decode(status,'CURRENT',group#,0)),\n"
                            "       sum(decode(status,'CURRENT',bytes,0))/:f1<int>,\n"
                            "       sum(bytes)/:f1<int> from v$log\n",
                            "Information about redo logs");

static toSQL SQLOverviewTablespaces("toTuning:Overview:Tablespaces",
                                    "select count(1) from v$tablespace",
                                    "Number of tablespaces",
                                    "0800");

static toSQL SQLOverviewTablespaces7("toTuning:Overview:Tablespaces",
                                     "select count(1) from sys.dba_tablespaces",
                                     "",
                                     "0703");

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
                                  "0801");

static toSQL SQLOverviewDedicated7("toTuning:Overview:Dedicated",
                                   "select count(1) from v$session where type = 'USER' and server = 'DEDICATED'",
                                   "",
                                   "0800");

static toSQL SQLOverviewDispatcher("toTuning:Overview:Dispatcher",
                                   "select count(1) from v$dispatcher",
                                   "Dispatcher processes");

static toSQL SQLOverviewParallell("toTuning:Overview:Parallel",
                                  "select count(1) from v$px_process",
                                  "Parallel processes",
                                  "0801");

static toSQL SQLOverviewParallell8("toTuning:Overview:Parallel",
                                   "select 'N/A' from sys.dual",
                                   "",
                                   "0800");

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
                                    "0801");

static toSQL SQLOverviewClientTotal8("toTuning:Overview:ClientTotal",
                                     "select count(1),\n"
                                     "       sum(decode(status,'ACTIVE',1,0))\n"
                                     "  from v$session\n"
                                     " where type != 'BACKGROUND'",
                                     "",
                                     "0800");

static toSQL SQLOverviewDatafiles("toTuning:Overview:Datafiles",
                                  "select (select count(1) from v$datafile)+(select count(1) from v$tempfile) from dual",
                                  "Number of datafiles",
                                  "0800");

static toSQL SQLOverviewDatafiles8("toTuning:Overview:Datafiles",
                                   "select count(1) from v$datafile",
                                   "",
                                   "0800");

static toSQL SQLOverviewDatafiles7("toTuning:Overview:Datafiles",
                                   "select count(1) from sys.dba_tablespaces",
                                   "",
                                   "0703");

void toTuningOverview::overviewQuery::setValue(const QString &nam, const QString &val)
{
    if (Parent.Quit)
        throw 1;
    toLocker lock (Parent.Lock)
    ;
    Parent.Values[nam] = val;
}

void toTuningOverview::overviewQuery::run(void)
{
    try
    {
        toQList val;
        val.insert(val.end(), toQValue(toSizeDecode(Parent.UnitString)));

        toQList res = toQuery::readQuery(*Parent.Connection, SQLOverviewArchive, val);
        QString tmp = toShift(res);
        tmp += QString::fromLatin1("/");
        tmp += toShift(res);
        tmp += Parent.UnitString;
        setValue("ArchiveInfo", tmp);

        res = toQuery::readQuery(*Parent.Connection, SQLOverviewRound);
        tmp = toShift(res);
        tmp += QString::fromLatin1(" ms");
        setValue("SendFromClient", tmp);
        tmp = toShift(res);
        tmp += QString::fromLatin1(" ms");
        setValue("SendToClient", tmp);

        res = toQuery::readQuery(*Parent.Connection, SQLOverviewClientTotal);
        tmp = toShift(res);
        setValue("TotalClient", tmp);
        tmp = toShift(res);
        setValue("ActiveClient", tmp);

        int totJob = 0;
        res = toQuery::readQuery(*Parent.Connection, SQLOverviewDedicated);
        tmp = toShift(res);
        totJob += tmp.toInt();
        setValue("DedicatedServer", tmp);

        res = toQuery::readQuery(*Parent.Connection, SQLOverviewDispatcher);
        tmp = toShift(res);
        totJob += tmp.toInt();
        setValue("DispatcherServer", tmp);

        res = toQuery::readQuery(*Parent.Connection, SQLOverviewShared);
        tmp = toShift(res);
        totJob += tmp.toInt();
        setValue("SharedServer", tmp);

        res = toQuery::readQuery(*Parent.Connection, SQLOverviewParallell);
        tmp = toShift(res);
        totJob += tmp.toInt();
        setValue("ParallellServer", tmp);

        res = toQuery::readQuery(*Parent.Connection, SQLOverviewBackground);
        QStringList back;
        while (!res.empty())
        {
            tmp = toShift(res);
            if (tmp == QString::fromLatin1("DBW"))
                tmp = QString::fromLatin1("DBWR");
            else if (tmp == QString::fromLatin1("PMO"))
                tmp = QString::fromLatin1("PMON");
            else if (tmp == QString::fromLatin1("ARC"))
                tmp = QString::fromLatin1("ARCH");
            else if (tmp == QString::fromLatin1("CKP"))
                tmp = QString::fromLatin1("CKPT");
            else if (tmp == QString::fromLatin1("LGW"))
                tmp = QString::fromLatin1("LGWR");
            else if (tmp == QString::fromLatin1("LMO"))
                tmp = QString::fromLatin1("LMON");
            else if (tmp == QString::fromLatin1("REC"))
                tmp = QString::fromLatin1("RECO");
            else if (tmp == QString::fromLatin1("TRW"))
                tmp = QString::fromLatin1("TRWR");
            else if (tmp == QString::fromLatin1("SMO"))
                tmp = QString::fromLatin1("SMON");

            tmp += QString::fromLatin1(": <B>");
            QString job = toShift(res);
            totJob += job.toInt();
            tmp += job;
            tmp += QString::fromLatin1("</B>");
            back << tmp;
        }
        setValue("Background", back.join(QString::fromLatin1(",")));
        setValue("TotalProcess", QString::number(totJob));

        double tot = 0;
        double sql = 0;
        res = toQuery::readQuery(*Parent.Connection, SQLOverviewSGA, val);
        while (!res.empty())
        {
            QString nam = toShift(res);
            tmp = toShift(res);
            if (nam == "Database Buffers" || nam == "Redo Buffers")
                setValue(nam.toLatin1(), tmp + Parent.UnitString);
            else if (nam == "Fixed Size" || nam == "Variable Size")
                sql += tmp.toDouble();
            tot += tmp.toDouble();
        }
        tmp = toQValue::formatNumber(tot);
        tmp += Parent.UnitString;
        setValue("SGATotal", tmp);
        tmp = toQValue::formatNumber(sql);
        tmp += Parent.UnitString;
        setValue("SharedSize", tmp);

        res = toQuery::readQuery(*Parent.Connection, SQLOverviewLog, val);
        setValue("RedoFiles", toShift(res));
        setValue("ActiveRedo", toShift(res));
        tmp = toShift(res);
        tmp += QString::fromLatin1("/");
        tmp += toShift(res);
        tmp += Parent.UnitString;
        setValue("RedoSize", tmp);

        res = toQuery::readQuery(*Parent.Connection, SQLOverviewTablespaces);
        setValue("Tablespaces", toShift(res));

        res = toQuery::readQuery(*Parent.Connection, SQLOverviewDatafiles);
        setValue("Files", toShift(res));
    }
    catch (const QString &str)
    {
        fprintf(stderr, "Exception occured:\n\n%s\n", (const char *)str.toLatin1());
    }
    catch (int)
        {}
    Parent.Done.up();
}

void toTuningOverview::refresh(void)
{
    try
    {
        if (Done.getValue() == 1)
        {
            Done.down();
            Quit = false;
            Connection = &toCurrentConnection(this);
            UnitString = toConfigurationSingle::Instance().sizeUnit();
            toThread *thread = new toThread(new overviewQuery(*this));
            thread->start();
            Poll.start(500);
        }
    }
    TOCATCH
}

void toTuningOverview::setValue(QLabel *label, const QString &nam)
{
    toLocker lock (Lock)
    ;
    std::map<QString, QString>::iterator i = Values.find(nam);
    if (i != Values.end())
    {
        label->setText((*i).second);
        Values.erase(i);
    }
}

void toTuningOverview::poll(void)
{
    try
    {
        setValue(ArchiveInfo, "ArchiveInfo");
        setValue(SendFromClient, "SendFromClient");
        setValue(SendToClient, "SendToClient");
        setValue(TotalClient, "TotalClient");
        setValue(ActiveClient, "ActiveClient");
        setValue(DedicatedServer, "DedicatedServer");
        setValue(DispatcherServer, "DispatcherServer");
        setValue(SharedServer, "SharedServer");
        setValue(ParallellServer, "ParallellServer");

        {
            toLocker lock (Lock)
            ;
            std::map<QString, QString>::iterator i = Values.find("Background");
            if (i != Values.end())
            {
                std::list<QLabel *>::iterator labIt = Backgrounds.begin();

                QStringList lst = (*i).second.split(",");
                for (int j = 0;j < lst.count();j++)
                {
                    QLabel *label;
                    if (labIt == Backgrounds.end() || *labIt == NULL)
                    {
                        label = new QLabel(BackgroundGroup);
                        BackgroundGroup->layout()->addWidget(label);
                        label->show();
                        if (labIt == Backgrounds.end())
                        {
                            Backgrounds.insert(Backgrounds.end(), label);
                            labIt = Backgrounds.end();
                        }
                        else
                        {
                            *labIt = label;
                            labIt++;
                        }
                    }
                    else
                    {
                        label = *labIt;
                        labIt++;
                    }
                    label->setText(lst[j]);
                }
                while (labIt != Backgrounds.end())
                {
                    delete *labIt;
                    *labIt = NULL;
                    labIt++;
                }
                Values.erase(i);
            }
        }

        setValue(TotalProcess, "TotalProcess");
        setValue(BufferSize, "Database Buffers");
        setValue(RedoBuffer, "Redo Buffers");
        setValue(SGATotal, "SGATotal");
        setValue(SharedSize, "SharedSize");
        setValue(RedoFiles, "RedoFiles");
        setValue(ActiveRedo, "ActiveRedo");
        setValue(RedoSize, "RedoSize");

        std::list<double> &values = FileUsed->values();
        std::list<double>::iterator j = values.begin();
        if (j != values.end())
        {
            double size = 0;
            double used = 0;
            if (j != values.end())
                used = size = (*j);
            j++;
            if (j != values.end())
                size += (*j);
            QString tmp = toQValue::formatNumber(used);
            tmp += QString::fromLatin1("/");
            tmp += toQValue::formatNumber(size);
            tmp += UnitString;
            if (tmp != Filesize->text())
                Filesize->setText(tmp);
        }

        setValue(Tablespaces, "Tablespaces");
        setValue(Files, "Files");
        if (Done.getValue() == 1)
            Poll.stop();
    }
    TOCATCH
}

static toSQL SQLLibraryCache("toTuning:LibraryCache",
                             "SELECT namespace,\n"
                             "       gets \"Gets\",\n"
                             "       gethits \"Get Hits\",\n"
                             "       to_char(trunc(gethitratio*100,\n"
                             "       1))||'%' \"-Ratio\",\n"
                             "       pins \"Pins\",\n"
                             "       pinhits \"Pin Hits\",\n"
                             "       to_char(trunc(pinhitratio*100,\n"
                             "       1))||'%' \"-Ratio\"\n"
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

toTuning::toTuning(QWidget *main, toConnection &connection)
        : toToolWidget(TuningTool, "tuning.html", main, connection, "toTuning")
{
    if (toConfigurationSingle::Instance().tuningFirstRun())
    {
        bool def = false;
        if (TOMessageBox::warning(
                    toMainWidget(),
                    tr("Enable all tuning statistics"),
                    tr("Are you sure you want to enable all tuning features.\n"
                       "This can put heavy strain on a database and unless you\n"
                       "are the DBA you probably don't want this. Selecting\n"
                       "no here will give you the option to enable or disable\n"
                       "tabs individually as they are needed."),
                    tr("Yes"),
                    tr("&No"),
                    QString::null,
                    1) == 0)
        {
            def = true;
        }

        toConfigurationSingle::Instance().setTuningOverview(def);
        toConfigurationSingle::Instance().setTuningFileIO(def);
        toConfigurationSingle::Instance().setTuningWaits(def);
        toConfigurationSingle::Instance().setTuningCharts(def);
        toConfigurationSingle::Instance().saveConfig();
    }

    QToolBar *toolbar = toAllocBar(this, tr("Server Tuning"));
    layout()->addWidget(toolbar);

    refreshAct = toolbar->addAction(
                     QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                     tr("Refresh"),
                     this,
                     SLOT(refresh()));
    refreshAct->setShortcut(QKeySequence::Refresh);

    toolbar->addSeparator();

    toolbar->addWidget(new QLabel(tr("Refresh") + " ", toolbar));

    Refresh = toRefreshCreate(toolbar, TO_TOOLBAR_WIDGET_NAME);
    connect(Refresh, SIGNAL(activated(const QString &)), this, SLOT(changeRefresh(const QString &)));
    toolbar->addWidget(Refresh);

    // used in pulldown menu
    changeRefreshAct = new QAction(tr("&Change Refresh"), this);
    changeRefreshAct->setShortcut(Qt::ALT + Qt::Key_R);
    connect(changeRefreshAct, SIGNAL(triggered()), Refresh, SLOT(setFocus(void)));

    toolbar->addSeparator();

    tabMenu = new QMenu(toolbar);
    tabMenu->setIcon(QIcon(QPixmap(const_cast<const char**>(compile_xpm))));
    tabMenu->setTitle(tr("Enable and disable tuning tabs"));
    toolbar->addAction(tabMenu->menuAction());

    connect(tabMenu, SIGNAL(aboutToShow()), this, SLOT(showTabMenu()));
    connect(tabMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(enableTabMenu(QAction *)));

    QLabel *stretch = new QLabel(toolbar);
    toolbar->addWidget(stretch);
    stretch->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    stretch->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                       QSizePolicy::Minimum));

    new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

    Tabs = new QTabWidget(this);
    layout()->addWidget(Tabs);

    Overview = new toTuningOverview(this, "overview");
    Tabs->addTab(Overview, tr("&Overview"));

    try
    {
        toRefreshParse(timer());
        connect(timer(), SIGNAL(timeout()), Overview, SLOT(refresh()));
    }
    TOCATCH;

    ChartContainer = new QScrollArea(Tabs);
    QWidget *chartWidget = new QWidget(ChartContainer);
    ChartContainer->setWidget(chartWidget);
    ChartContainer->setWidgetResizable(true);
    chartWidget->setMinimumHeight(1800);
    Tabs->addTab(ChartContainer, tr("&Charts"));

    QVBoxLayout *chartBox = new QVBoxLayout;
    chartWidget->setLayout(chartBox);

    QString unitStr = toConfigurationSingle::Instance().sizeUnit();
    toQList unit;
    unit.insert(unit.end(), toQValue(toSizeDecode(unitStr)));

    {
        std::list<QString> val = toSQL::range("toTuning:Charts");
        for (std::list<QString>::iterator i = val.begin();i != val.end();i++)
        {
            QStringList parts = (*i).split(":");
            if (parts.count() == 3)
            {
                parts.append(parts[2]);
                parts[2] = QString::fromLatin1("Charts");
            }

            if (parts[3].mid(1, 1) == QString::fromLatin1("B"))
            {
                toResultBar *chart = new toResultBar(chartWidget);
                chartBox->addWidget(chart);
                Charts.append(chart);
                chart->setTitle(parts[3].mid(3));
                toQList par;
                if (parts[3].mid(2, 1) == QString::fromLatin1("B"))
                    chart->setYPostfix(tr(" blocks/s"));
                else if (parts[3].mid(2, 1) == QString::fromLatin1("M"))
                    chart->setYPostfix(QString::fromLatin1(" ms/s"));
                else if (parts[3].mid(2, 1) == QString::fromLatin1("S"))
                {
                    par = unit;
                    QString t = unitStr;
                    t += QString::fromLatin1("/s");
                    chart->setYPostfix(t);
                }
                else if (parts[3].mid(2, 1) == QString::fromLatin1("A"))
                    chart->setFlow(false);
                else
                    chart->setYPostfix(QString::fromLatin1("/s"));
                chart->query(toSQL::sql(*i), par);
            }
            else if (parts[3].mid(1, 1) == QString::fromLatin1("L") || parts[3].mid(1, 1) == QString::fromLatin1("C"))
            {
                toResultLine *chart;
                if (parts[3].mid(1, 1) == QString::fromLatin1("C"))
                    chart = new toTuningMiss(chartWidget);
                else
                    chart = new toResultLine(chartWidget);
                chartBox->addWidget(chart);
                Charts.append(chart);
                toQList par;
                if (parts[3].mid(2, 1) == QString::fromLatin1("B"))
                    chart->setYPostfix(tr(" blocks/s"));
                else if (parts[3].mid(2, 1) == QString::fromLatin1("S"))
                {
                    par = unit;
                    QString t = unitStr;
                    t += QString::fromLatin1("/s");
                    chart->setYPostfix(t);
                }
                else if (parts[3].mid(2, 1) == QString::fromLatin1("P"))
                {
                    chart->setYPostfix(QString::fromLatin1(" %"));
                    chart->setMinValue(0);
                }
                else
                    chart->setYPostfix(QString::fromLatin1("/s"));
                chart->query(toSQL::sql(*i), par);
            }
            else if (parts[3].mid(1, 1) == QString::fromLatin1("P"))
            {
                toResultPie *chart = new toResultPie(chartWidget);
                chart->setTitle(parts[3].mid(3));
                chartBox->addWidget(chart);
                Charts.append(chart);
                if (parts[3].mid(2, 1) == QString::fromLatin1("S"))
                {
                    chart->query(toSQL::sql(*i), unit);
                    chart->setPostfix(unitStr);
                }
                else
                    chart->query(toSQL::sql(*i));
            }
            else
                toStatusMessage(tr("Wrong format of name on chart (%1).").arg(QString(*i)));
        }
    }

    Waits = new toWaitEvents(this, "waits");
    Tabs->addTab(Waits, tr("&Wait events"));

    FileIO = new toTuningFileIO(Tabs);
    connect(this, SIGNAL(connectionChange()), FileIO, SLOT(changeConnection()));
    Tabs->addTab(FileIO, tr("&File I/O"));

    Indicators = new toListView(Tabs, "indicators");
    Indicators->setRootIsDecorated(true);
    Indicators->addColumn(tr("Indicator"));
    Indicators->addColumn(tr("Value"));
    Indicators->addColumn(tr("Reference"));
    Tabs->addTab(Indicators, tr("&Indicators"));

    Statistics = new toResultStats(Tabs, "stats");
    Tabs->addTab(Statistics, tr("&Statistics"));

    Parameters = new toResultParam(Tabs, "parameters");
    Tabs->addTab(Parameters, tr("&Parameters"));

    BlockingLocks = new toResultLock(Tabs, "locks");
    Tabs->addTab(BlockingLocks, tr("&Blocking locks"));

    LibraryCache = new toResultTableView(true, false, Tabs, "cache");
    LibraryCache->setSQL(SQLLibraryCache);
    Tabs->addTab(LibraryCache, tr("Library C&ache"));

    ControlFiles = new toResultTableView(true, false, Tabs, "control");
    ControlFiles->setSQL(SQLControlFiles);
    Tabs->addTab(ControlFiles, tr("C&ontrol Files"));

    Options = new toResultTableView(true, false, Tabs, "options");
    Options->setSQL(SQLOptions);
    Tabs->addTab(Options, tr("Optio&ns"));

    Licenses = new toResultItem(2, true, Tabs, "licenses");
    Licenses->setSQL(SQLLicense);
    Tabs->addTab(Licenses, tr("&Licenses"));

    Tabs->setCurrentIndex(0);

    LastTab = NULL;

    connect(Tabs, SIGNAL(currentChanged(int)), this, SLOT(changeTab(int)));
    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(windowActivated(QMdiSubWindow *)));

    if (!toConfigurationSingle::Instance().tuningOverview())
        enableTab(CONF_OVERVIEW, false);
    if (!toConfigurationSingle::Instance().tuningFileIO())
        enableTab(CONF_FILEIO, false);
    if (!toConfigurationSingle::Instance().tuningWaits())
        enableTab(CONF_WAITS, false);
    if (!toConfigurationSingle::Instance().tuningCharts())
        enableTab(CONF_CHART, false);
    refresh();
    setFocusProxy(Tabs);
}

QWidget *toTuning::tabWidget(const QString &name)
{
    QWidget *widget = NULL;
    if (name == CONF_OVERVIEW)
    {
        widget = Overview;
    }
    else if (name == CONF_FILEIO)
    {
        widget = FileIO;
    }
    else if (name == CONF_WAITS)
    {
        widget = Waits;
    }
    else if (name == CONF_CHART)
    {
        widget = ChartContainer;
    }
    return widget;
}

void toTuning::showTabMenu(void)
{
    tabMenu->clear();
    std::list<QString> tab = TabList();
    for (std::list<QString>::iterator i = tab.begin(); i != tab.end(); i++)
    {
        QAction *act = new QAction(tr((*i).toAscii().constData()), tabMenu);
        QWidget *widget = tabWidget(*i);

        act->setCheckable(true);
        if (widget && Tabs->isTabEnabled(Tabs->indexOf(widget)))
            act->setChecked(true);

        tabMenu->addAction(act);
    }
}

void toTuning::enableTabMenu(QAction *act)
{
    if (!act)
        return;

    QWidget *widget = tabWidget(act->text());
    if (widget)
        enableTab(act->text(), !Tabs->isTabEnabled(Tabs->indexOf(widget)));
}

void toTuning::enableTab(const QString &name, bool enable)
{
    QWidget *widget = NULL;
    if (name == CONF_OVERVIEW)
    {
        if (enable)
            Overview->start();
        else
            Overview->stop();

        toConfigurationSingle::Instance().setTuningOverview(enable);
        widget = Overview;
    }
    else if (name == CONF_CHART)
    {
        Q_FOREACH(QWidget *child, Charts)
        {
            toResultLine *line = dynamic_cast<toResultLine *>(child);
            if (line)
            {
                if (enable)
                    line->start();
                else
                    line->stop();
            }
            toResultBar *bar = dynamic_cast<toResultBar *>(child);
            if (bar)
            {
                if (enable)
                    bar->start();
                else
                    bar->stop();
            }
            toResultPie *pie = dynamic_cast<toResultPie *>(child);
            if (pie)
            {
                if (enable)
                    pie->start();
                else
                    pie->stop();
            }
        }
 
        toConfigurationSingle::Instance().setTuningCharts(enable);
        widget = ChartContainer;
    }
    else if (name == CONF_WAITS)
    {
        if (enable)
            Waits->start();
        else
            Waits->stop();

        toConfigurationSingle::Instance().setTuningWaits(enable);
        widget = Waits;
    }
    else if (name == CONF_FILEIO)
    {
        if (enable)
            FileIO->start();
        else
            FileIO->stop();

        toConfigurationSingle::Instance().setTuningFileIO(enable);
        widget = FileIO;
    }

    if (widget)
    {
        int ind = Tabs->indexOf(widget);
        if(ind < 0)
            return;

        if(enable)
            Tabs->setCurrentIndex(ind);
        else
        {
            // qtabwidget is enabling some tabs when we disable one,
            // so i'm going to pick the next tab to show here
            for(int pos = 0; pos < Tabs->count(); pos++)
            {
                if(pos != ind && Tabs->isTabEnabled(pos))
                {
                    Tabs->setCurrentIndex(pos);
                    break;
                }
            }
        }

        Tabs->setTabEnabled(ind, enable);
    }
}

void toTuning::changeTab(int index)
{
    QWidget *widget = Tabs->widget(index);
    if (!widget || LastTab == widget)
        return;
    refresh();
}

void toTuning::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;
    if (widget->widget() == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Tuning"), this);

            ToolMenu->addAction(refreshAct);
            ToolMenu->addAction(changeRefreshAct);

            toMainWidget()->addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toTuning::changeRefresh(const QString &str)
{
    try
    {
        toRefreshParse(timer(), str);
    }
    TOCATCH
}

void toTuning::refresh(void)
{
    LastTab = Tabs->currentWidget();
    if (LastTab == Overview)
    {
        Overview->refresh();
    }
    else if (LastTab == Indicators)
    {
        Indicators->clear();
        std::list<QString> val = toSQL::range("toTuning:Indicators");
        toTreeWidgetItem *parent = NULL;
        toTreeWidgetItem *last = NULL;
        for (std::list<QString>::iterator i = val.begin();i != val.end();i++)
        {
            try
            {
                toQList val = toQuery::readQuery(connection(), toSQL::string(*i, connection()));
                QStringList parts = (*i).split(":");
                if (!parent || parent->text(0) != parts[2])
                {
                    parent = new toResultViewItem(Indicators, NULL, parts[2]);
                    parent->setOpen(true);
                    last = NULL;
                }
                QStringList dsc = toSQL::description(*i).split(".");
                QString first = dsc[0];
                first += QString::fromLatin1(".");
                last = new toResultViewItem(parent, last, first);
                QString str;
                for (toQList::iterator j = val.begin();j != val.end();j++)
                    str += *j;
                last->setText(1, str);
                if (dsc.count() > 1)
                    last->setText(2, dsc[1]);
            }
            TOCATCH
        }
        Indicators->resizeColumnsToContents();
    }
    else if (LastTab == Waits)
        Waits->refresh();
    else if (LastTab == Statistics)
        Statistics->refreshStats();
    else if (LastTab == Parameters)
        Parameters->refresh();
    else if (LastTab == BlockingLocks)
        BlockingLocks->refresh();
    else if (LastTab == LibraryCache)
        LibraryCache->refresh();
    else if (LastTab == ControlFiles)
    {
        QString unit = toConfigurationSingle::Instance().sizeUnit();
        ControlFiles->changeParams(QString::number(toSizeDecode(unit)), unit);
    }
    else if (LastTab == Options)
        Options->refresh();
    else if (LastTab == Licenses)
        Licenses->refresh();
}

void toTuning::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    toToolWidget::exportData(data, prefix);
    std::list<QString> ret = TabList();
    for (std::list<QString>::iterator i = ret.begin();i != ret.end();i++)
    {
        QWidget *widget = tabWidget(*i);
        if (widget)
        {
            if (!Tabs->isTabEnabled(Tabs->indexOf(widget)))
                data[prefix + ":" + *i] = QString::fromLatin1("Disabled");
        }
    }
    data[prefix + ":Current"] = Tabs->currentWidget()->objectName();

    Waits->exportData(data, prefix + ":Waits");
}

void toTuning::importData(std::map<QString, QString> &data, const QString &prefix)
{
    toToolWidget::importData(data, prefix);
    std::list<QString> ret = TabList();
    for (std::list<QString>::iterator i = ret.begin();i != ret.end();i++)
        enableTab(*i, data[prefix + ":" + (*i)].isEmpty());
    QWidget *chld = findChild<QWidget *>(data[prefix + ":Current"]);
    if (chld)
        Tabs->setCurrentIndex(Tabs->indexOf(chld));
    Waits->importData(data, prefix + ":Waits");
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
                       "0801");

static toSQL SQLFileIO8("toTuning:FileIO",
                        "select a.name,b.name,sysdate,\n"
                        "       c.phyrds,c.phywrts,c.phyblkrd,c.phyblkwrt,\n"
                        "       c.avgiotim*10,c.miniotim*10,c.maxiortm*10,c.maxiowtm*10\n"
                        "  from v$tablespace a,v$datafile b,v$filestat c\n"
                        " where a.ts# = b.ts# and b.file# = c.file#\n"
                        " order by a.name",
                        "",
                        "0800");

toTuningFileIO::toTuningFileIO(QWidget *parent)
        : QWidget(parent)
{
    try
    {
        connect(toCurrentTool(this)->timer(), SIGNAL(timeout()), this, SLOT(refresh()));

        // fscking qscrollarea won't resize unless this is added
        QHBoxLayout *h = new QHBoxLayout;
        h->setSpacing(0);
        h->setContentsMargins(0, 0, 0, 0);

        QScrollArea *sa = new QScrollArea(this);
        h->addWidget(sa);
        setLayout(h);

        QWidget     *Box  = new QWidget;
        QVBoxLayout *vbox = new QVBoxLayout;

        QComboBox *combo = new QComboBox(Box);
        combo->addItem(tr("File I/O"));
        combo->addItem(tr("File timing"));
        if (toCurrentConnection(this).version() >= "0800")
        {
            combo->addItem(tr("Tablespace I/O"));
            combo->addItem(tr("Tablespace timing"));
        }
        vbox->addWidget(combo);
        connect(combo, SIGNAL(activated(int)), this, SLOT(changeCharts(int)));

        FileReads       = new QWidget(Box);
        FileTime        = new QWidget(Box);
        TablespaceReads = new QWidget(Box);
        TablespaceTime  = new QWidget(Box);

        FileReads->setLayout(new QGridLayout);
        FileTime->setLayout(new QGridLayout);
        TablespaceReads->setLayout(new QGridLayout);
        TablespaceTime->setLayout(new QGridLayout);

        vbox->addWidget(FileReads);
        vbox->addWidget(FileTime);
        vbox->addWidget(TablespaceReads);
        vbox->addWidget(TablespaceTime);

        Box->setLayout(vbox);
        sa->setWidget(Box);
        sa->setWidgetResizable(true);

        changeCharts(0);
        CurrentStamp = 0;
        connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
        Query = NULL;
        refresh();
    }
    TOCATCH
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
    switch (val)
    {
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
    labels.insert(labels.end(), tr("Reads"));
    labels.insert(labels.end(), tr("Blocks Read"));
    labels.insert(labels.end(), tr("Writes"));
    labels.insert(labels.end(), tr("Blocks Written"));
    std::list<QString> labelTime;
    labelTime.insert(labelTime.end(), tr("Average"));
    labelTime.insert(labelTime.end(), tr("Minimum"));
    labelTime.insert(labelTime.end(), tr("Maximum Read"));
    labelTime.insert(labelTime.end(), tr("Maximum Write"));

    toResultBar *barchart;
    if (name.startsWith(QString::fromLatin1("tspc:")))
    {
        barchart = new toResultBar(TablespaceReads);
        TablespaceReads->layout()->addWidget(barchart);
    }
    else
    {
        barchart = new toResultBar(FileReads);
        FileReads->layout()->addWidget(barchart);
    }
    ReadsCharts[name] = barchart;
    barchart->setTitle(name.mid(5));
    barchart->setMinimumSize(200, 170);
    barchart->setYPostfix(tr("blocks/s"));
    barchart->setLabels(labels);
    barchart->setSQLName(QString("toTuning:FileIO:Reads:" + name));
    barchart->show();

    toResultLine *linechart;
    if (name.startsWith(QString::fromLatin1("tspc:")))
    {
        linechart = new toResultLine(TablespaceTime);
        TablespaceTime->layout()->addWidget(linechart);
    }
    else
    {
        linechart = new toResultLine(FileTime);
        FileTime->layout()->addWidget(linechart);
    }
    TimeCharts[name] = linechart;
    linechart->setTitle(name.mid(5));
    linechart->setMinimumSize(200, 170);
    linechart->setYPostfix(QString::fromLatin1("ms"));
    linechart->setLabels(labelTime);
    linechart->setSQLName(QString("toTuning:FileIO:Time:" + name));
    linechart->show();
}

void toTuningFileIO::saveSample(const QString &name, const QString &label,
                                double reads, double writes,
                                double readBlk, double writeBlk,
                                double avgTim, double minTim,
                                double maxRead, double maxWrite)
{
    if (CurrentStamp != LastStamp)
    {

        std::list<double> vals;
        vals.insert(vals.end(), reads);
        vals.insert(vals.end(), writes);
        vals.insert(vals.end(), readBlk);
        vals.insert(vals.end(), writeBlk);

        std::list<double> last = LastValues[name];
        std::list<double> dispVal;
        if (!last.empty())
        {
            std::list<double>::iterator i = vals.begin();
            std::list<double>::iterator j = last.begin();
            while (i != vals.end() && j != last.end())
            {
                dispVal.insert(dispVal.end(), (*i - *j) / (CurrentStamp - LastStamp));
                i++;
                j++;
            }
        }
        LastValues[name] = vals;

        if (ReadsCharts.find(name) == ReadsCharts.end())
            allocCharts(name);

        if (!dispVal.empty())
        {
            toBarChart *chart = ReadsCharts[name];
            chart->addValues(dispVal, label);
        }

        vals.clear();
        vals.insert(vals.end(), avgTim);
        vals.insert(vals.end(), minTim);
        vals.insert(vals.end(), maxRead);
        vals.insert(vals.end(), maxWrite);

        toLineChart *chart = TimeCharts[name];
        chart->addValues(vals, label);
    }
}

void toTuningFileIO::refresh(void)
{
    if (!Query)
    {
        try
        {
            toConnection &conn = toCurrentConnection(this);
            if (conn.version() < "0800")
                return ;
            toQList par;
            LastStamp = CurrentStamp;
            CurrentStamp = time(NULL);
            Query = new toNoBlockQuery(conn, toQuery::Background, toSQL::string(SQLFileIO, conn), par);
            LastTablespace = QString::null;

            Poll.start(100);
        }
        TOCATCH
    }
}

void toTuningFileIO::poll(void)
{
    try
    {
        if (Query && Query->poll())
        {
            QString tablespace;
            QString datafile;
            QString timestr;
            while (Query->poll())
            {
                if (!Query->eof())
                {
                    tablespace = Query->readValueNull();
                    datafile = Query->readValueNull();
                    timestr = Query->readValueNull();
                }
                else
                    tablespace = QString::null;
                if (tablespace != LastTablespace)
                {
                    if (!LastTablespace.isNull())
                    {
                        QString name = QString::fromLatin1("tspc:");
                        name += LastTablespace;

                        saveSample(name, timestr,
                                   TblReads, TblWrites, TblReadBlk, TblWriteBlk,
                                   TblAvg, TblMin, TblMaxRead, TblMaxWrite);
                    }

                    TblReads = TblWrites = TblReadBlk = TblWriteBlk = TblAvg = TblMin = TblMaxRead = TblMaxWrite = 0;
                    LastTablespace = tablespace;
                }
                if (Query->eof())
                    break;

                double reads = Query->readValueNull().toDouble();
                double writes = Query->readValueNull().toDouble();
                double readBlk = Query->readValueNull().toDouble();
                double writeBlk = Query->readValueNull().toDouble();
                double avgTim = Query->readValueNull().toDouble();
                double minTim = Query->readValueNull().toDouble();
                double maxRead = Query->readValueNull().toDouble();
                double maxWrite = Query->readValueNull().toDouble();

                TblReads += reads;
                TblWrites += writes;
                TblReadBlk += readBlk;
                TblWriteBlk += writeBlk;
                TblAvg += avgTim;
                TblMin += minTim;
                TblMaxRead += maxRead;
                TblMaxWrite += maxWrite;

                QString name = QString::fromLatin1("file:");
                name += datafile;

                saveSample(name, timestr,
                           reads, writes, readBlk, writeBlk,
                           avgTim, minTim, maxRead, maxWrite);
            }
            if (Query->eof())
            {
                Poll.stop();
                delete Query;
                Query = NULL;
            }
        }
    }
    TOCATCH
}

void toTuningFileIO::changeConnection(void)
{
    {
        for (std::map<QString, toBarChart *>::iterator i = ReadsCharts.begin();
                i != ReadsCharts.end();i++)
            delete (*i).second;
    }
    ReadsCharts.clear();
    for (std::map<QString, toLineChart *>::iterator i = TimeCharts.begin();
            i != TimeCharts.end();i++)
        delete (*i).second;
    TimeCharts.clear();
    LastValues.clear();
    refresh();
}

toTuningMiss::toTuningMiss(QWidget *parent, const char *name)
        : toResultLine(parent, name)
{}

std::list<double> toTuningMiss::transform(std::list<double> &inp)
{
    std::list<double> ret;
    for (std::list<double>::iterator i = inp.begin();i != inp.end();i++)
    {
        double first = *i;
        i++;
        if (i != inp.end())
        {
            double second = *i;
            if (second == 0)
                ret.insert(ret.end(), 0);
            else
                ret.insert(ret.end(), first / second);
        }
    }
    return ret;
}

void toTuningFileIO::stop(void)
{
    try
    {
        disconnect(toCurrentTool(this)->timer(), SIGNAL(timeout()), this, SLOT(refresh()));
    }
    TOCATCH
}

void toTuningFileIO::start(void)
{
    try
    {
        connect(toCurrentTool(this)->timer(), SIGNAL(timeout()), this, SLOT(refresh()));
    }
    TOCATCH
}
