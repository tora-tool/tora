
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "tools/totuning.h"
#include "tools/totuningcharts.h"
#include "tools/totuningoverview.h"
#include "tools/totuningfileio.h"
#include "core/tochangeconnection.h"
#include "core/toconf.h"
#include "core/toconnection.h"
#include "core/tomainwindow.h"
#include "core/toeventquery.h"
#include "core/toconfiguration.h"
#include "core/toglobalevent.h"
#include "core/tosettingtab.h"
#include "tools/toresultbar.h"
#include "widgets/toresultitem.h"
#include "widgets/torefreshcombo.h"
#include "tools/toresultline.h"

#include "tools/toresulttableview.h"
#include "tools/toresultparam.h"
#include "tools/toresultpie.h"
#include "tools/toresultstats.h"
#include "tools/toresultview.h"
#include "tools/toresultline.h"
#include "core/tosql.h"
#include "core/totool.h"
#include "core/utils.h"
#include "core/toglobalconfiguration.h"
#include "ui_totuningsettingui.h"
#include "tools/towaitevents.h"

#include "result/toresultwaitchains.h"

#include <QComboBox>
#include <QToolBar>
#include <QtGui/QResizeEvent>
#include <QtGui/QPixmap>
#include <QAction>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QtCore/QString>

#include "icons/refresh.xpm"
#include "icons/totuning.xpm"
#include "icons/compile.xpm"

#define CONF_OVERVIEW "Overview"
#define CONF_CHART    "Charts"
#define CONF_FILEIO   "File I/O"
#define CONF_WAITS    "Wait events"

QVariant ToConfiguration::Tuning::defaultValue(int option) const
{
    switch (option)
    {
    case FirstRunBool:
		return false;
    case OverviewBool:
		return false;
    case FileIOBool:
		return false;
    case WaitsBool:
		return false;
    case ChartsBool:
		return false;
    default:
        Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Tuning un-registered enum value: %1").arg(option)));
        return QVariant();
    }
}

static QList<QString> TabList(void)
{
    QList<QString> ret;
    ret.append(CONF_OVERVIEW);
    ret.append(CONF_CHART);
    ret.append(CONF_WAITS);
    ret.append(CONF_FILEIO);
    return ret;
}

class toTuningSetup : public QWidget, public Ui::toTuningSettingUI, public toSettingTab
{
    toTool *Tool;
public:
    toTuningSetup(toTool *tool, QWidget* parent = 0, const char* name = 0)
        : QWidget(parent), toSettingTab("tuning.html#preferences"), Tool(tool)
    {
        using namespace ToConfiguration;
        setupUi(this);
//         std::list<QString> tabs = TabList();
//         for (std::list<QString>::iterator i = tabs.begin();i != tabs.end();i++)
//         {
//             toTreeWidgetItem *item = new toTreeWidgetItem(EnabledTabs, (*i));
//             if (!tool->config(*i, "").isEmpty())
//                 item->setSelected(true);
//         }
        toTreeWidgetItem *item = new toTreeWidgetItem(EnabledTabs, CONF_OVERVIEW);
        item->setSelected(toConfigurationNewSingle::Instance().option(Tuning::OverviewBool).toBool());
        toTreeWidgetItem *item1 = new toTreeWidgetItem(EnabledTabs, CONF_FILEIO);
        item1->setSelected(toConfigurationNewSingle::Instance().option(Tuning::FileIOBool).toBool());
        toTreeWidgetItem *item2 = new toTreeWidgetItem(EnabledTabs, CONF_WAITS);
        item2->setSelected(toConfigurationNewSingle::Instance().option(Tuning::WaitsBool).toBool());
        toTreeWidgetItem *item3 = new toTreeWidgetItem(EnabledTabs, CONF_CHART);
        item3->setSelected(toConfigurationNewSingle::Instance().option(Tuning::ChartsBool).toBool());

        EnabledTabs->setSorting(0);
    }

    virtual void saveSetting(void)
    {
///s        for (toTreeWidgetItem *item = EnabledTabs->firstChild(); item; item = item->nextSibling())
///s        {
///s            // NOTE: OK, it's ugly, but this is the only place where new QSettings fails
///s            if (item->text(0) == CONF_OVERVIEW)
///s                toConfigurationNewSingle::Instance().setTuningOverview(item->isSelected());
///s            else if (item->text(0) == CONF_FILEIO)
///s                toConfigurationNewSingle::Instance().setTuningFileIO(item->isSelected());
///s            else if (item->text(0) == CONF_WAITS)
///s                toConfigurationNewSingle::Instance().setTuningWaits(item->isSelected());
///s            else if (item->text(0) == CONF_CHART)
///s                toConfigurationNewSingle::Instance().setTuningCharts(item->isSelected());

//             if (item->isSelected() || Tool->config(item->text(0).toLatin1(), "Undefined") != "Undefined")
//                 Tool->setConfig(item->text(0).toLatin1(), QString::fromLatin1((item->isSelected() ? "Yes" : "")));
///s        }
    }

};

class toTuningTool : public toTool
{
protected:
    const char **pictureXPM(void) override
    {
        return const_cast<const char**>(totuning_xpm);
    }
public:
    toTuningTool()
        : toTool(30, "Server Tuning")
    { }
    const char *menuItem() override
    {
        return "Server Tuning";
    }
    toToolWidget *toolWindow(QWidget *parent, toConnection &connection) override
    {
        return new toTuning(parent, connection);
    }
    QWidget *configurationTab(QWidget *parent) override
    {
        return new toTuningSetup(this, parent);
    }
    void closeWindow(toConnection &connection) override {};
private:
    static ToConfiguration::Tuning s_tuningConf;
};

ToConfiguration::Tuning toTuningTool::s_tuningConf;

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
    using namespace ToConfiguration;
    if (toConfigurationNewSingle::Instance().option(Tuning::FirstRunBool).toBool())
    {
        bool def = false;
        if (TOMessageBox::warning(
                    this,
                    tr("Enable all tuning statistics"),
                    tr("Are you sure you want to enable all tuning features.\n"
                       "This can put heavy strain on a database and unless you\n"
                       "are the DBA you probably don't want this. Selecting\n"
                       "no here will give you the option to enable or disable\n"
                       "tabs individually as they are needed."),
                    tr("Yes"),
                    tr("&No"),
                    QString(),
                    1) == 0)
        {
            def = true;
        }

        toConfigurationNewSingle::Instance().setOption(Tuning::OverviewBool, def);
        toConfigurationNewSingle::Instance().setOption(Tuning::FileIOBool, def);
        toConfigurationNewSingle::Instance().setOption(Tuning::WaitsBool, def);
        toConfigurationNewSingle::Instance().setOption(Tuning::ChartsBool, def);
        toConfigurationNewSingle::Instance().saveAll();
    }

    QToolBar *toolbar = Utils::toAllocBar(this, tr("Server Tuning"));
    layout()->addWidget(toolbar);

    refreshAct = toolbar->addAction(
                     QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                     tr("Refresh"),
                     this,
                     SLOT(refresh()));
    refreshAct->setShortcut(QKeySequence::Refresh);

    toolbar->addSeparator();

    toolbar->addWidget(new QLabel(tr("Refresh") + " ", toolbar));

    Refresh = new toRefreshCombo(toolbar);
    connect(Refresh->timer(), SIGNAL(timeout(void)), this, SLOT(refresh(void)));
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

    new toChangeConnection(toolbar);

    Tabs = new QTabWidget(this);
    layout()->addWidget(Tabs);

    Overview = new toTuningOverview(this);
    Tabs->addTab(Overview, tr("&Overview"));

	///d Utils::toRefreshParse(timer());
	///tconnect(timer(), SIGNAL(timeout()), Overview, SLOT(refresh()));

    Charts = new toTuningCharts(this);
    Tabs->addTab(Charts, tr("&Charts"));

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

    BlockingLocks = new toResultWaitChains(Tabs, "locks");
    Tabs->addTab(BlockingLocks->view(), tr("&Blocking locks"));

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

    if (!toConfigurationNewSingle::Instance().option(Tuning::OverviewBool).toBool())
        enableTab(CONF_OVERVIEW, false);
    if (!toConfigurationNewSingle::Instance().option(Tuning::FileIOBool).toBool())
        enableTab(CONF_FILEIO, false);
	if (!toConfigurationNewSingle::Instance().option(Tuning::WaitsBool).toBool())
        enableTab(CONF_WAITS, false);
    if (!toConfigurationNewSingle::Instance().option(Tuning::ChartsBool).toBool())
        enableTab(CONF_CHART, false);
    refresh();
    setFocusProxy(Tabs);
}

QWidget *toTuning::tabWidget(const QString &name)
{
    if (name == CONF_OVERVIEW)
        return Overview;
    else if (name == CONF_FILEIO)
        return FileIO;
    else if (name == CONF_WAITS)
        return Waits;
    else if (name == CONF_CHART)
        return Charts;
    return NULL;
}

void toTuning::showTabMenu(void)
{
    tabMenu->clear();
    QList<QString> tab = TabList();
    for (QList<QString>::iterator i = tab.begin(); i != tab.end(); i++)
    {
        QAction *act = new QAction(*i, tabMenu);
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
	using namespace ToConfiguration;
    QWidget *widget = NULL;
    if (name == CONF_OVERVIEW)
    {
        toConfigurationNewSingle::Instance().setOption(Tuning::OverviewBool, enable);
        widget = Overview;
    }
    else if (name == CONF_CHART)
    {
        toConfigurationNewSingle::Instance().setOption(Tuning::ChartsBool, enable);
        widget = Charts;
    }
    else if (name == CONF_WAITS)
    {
        toConfigurationNewSingle::Instance().setOption(Tuning::WaitsBool, enable);
        widget = Waits;
    }
    else if (name == CONF_FILEIO)
    {
        toConfigurationNewSingle::Instance().setOption(Tuning::FileIOBool, enable);
        widget = FileIO;
    }

    if (!widget)
        return;

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

void toTuning::changeTab(int index)
{
    QWidget *widget = Tabs->widget(index);
    if (!widget || LastTab == widget)
        return;
    refresh();
}

void toTuning::slotWindowActivated(toToolWidget *widget)
{
    if (!widget)
        return;
    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Tuning"), this);

            ToolMenu->addAction(refreshAct);
            ToolMenu->addAction(changeRefreshAct);

            toGlobalEventSingle::Instance().addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toTuning::refresh(void)
{
    using namespace ToConfiguration;
    LastTab = Tabs->currentWidget();
	if (LastTab == Overview)
	{
		Overview->refresh(connection());
	}
	else if (LastTab == Charts)
	{
		Charts->refresh();
	}
	else if (LastTab == Indicators)
	{
		Indicators->clear();
		QList<QString> val = toSQL::range("toTuning:Indicators");
		toTreeWidgetItem *parent = NULL;
		toTreeWidgetItem *last = NULL;
		for (QList<QString>::iterator i = val.begin(); i != val.end(); i++)
		{
			try
			{
				toQList val = toQuery::readQuery(connection(), toSQL::string(*i, connection()), toQueryParams());
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
				for (toQList::iterator j = val.begin(); j != val.end(); j++)
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
		Statistics->slotRefreshStats();
	else if (LastTab == Parameters)
		Parameters->refresh();
	else if (LastTab == BlockingLocks->view())
		BlockingLocks->refreshWithParams(toQueryParams());
    else if (LastTab == LibraryCache)
        LibraryCache->refresh();
    else if (LastTab == ControlFiles)
    {
        QString unit = toConfigurationNewSingle::Instance().option(Global::SizeUnit).toString();
        ControlFiles->refreshWithParams(toQueryParams() << QString::number(Utils::toSizeDecode(unit)) << unit);
    }
    else if (LastTab == Options)
        Options->refresh();
    else if (LastTab == Licenses)
        Licenses->refresh();
}

#ifdef TORA3_CHART
void toTuning::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    toToolWidget::exportData(data, prefix);
    std::list<QString> ret = TabList();
    for (std::list<QString>::iterator i = ret.begin(); i != ret.end(); i++)
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
    for (std::list<QString>::iterator i = ret.begin(); i != ret.end(); i++)
        enableTab(*i, data[prefix + ":" + (*i)].isEmpty());
    QWidget *chld = findChild<QWidget *>(data[prefix + ":Current"]);
    if (chld)
        Tabs->setCurrentIndex(Tabs->indexOf(chld));
    Waits->importData(data, prefix + ":Waits");
}
#endif
