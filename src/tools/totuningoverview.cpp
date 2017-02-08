
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

#include "tools/totuningoverview.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"

#include <QtCore/QSignalMapper>

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
    toQueryParams params;
    if (postfix == QString::fromLatin1("b/s"))
    {
        QString unitStr = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::SizeUnit).toString();
        params << toQValue(Utils::toSizeDecode(unitStr));
        unitStr += QString::fromLatin1("/s");
        chart->setYPostfix(unitStr);
    }
    else
        chart->setYPostfix(postfix);
    chart->setSQL(sql);
    chart->setParams(params);

    Charts.append(chart);
    connect(chart, SIGNAL(done()), Mapper, SLOT(map()));
    Mapper->setMapping(chart, Charts.indexOf(chart));
}

toTuningOverview::toTuningOverview(QWidget *parent)
    : QWidget(parent)
    , Mapper(new QSignalMapper(this))
    , UnitString(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::SizeUnit).toString())
{
    setupUi(this);

    GroupBoxBackground->setLayout(new QVBoxLayout);

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
    ClientChart->setSQL(SQLOverviewClient);
    ClientChart->setFlow(false);
    connect(ClientChart, SIGNAL(done()), Mapper, SLOT(map()));
    Charts.append(ClientChart);
    Mapper->setMapping(ClientChart, Charts.indexOf(ClientChart));

    SharedUsed->showGrid(0);
    SharedUsed->showLegend(false);
    SharedUsed->showAxisLegend(false);
    SharedUsed->setSQL(SQLOverviewSGAUsed);
    SharedUsed->setFlow(false);
    SharedUsed->setMaxValue(100);
    SharedUsed->setYPostfix(QString::fromLatin1("%"));
    SharedUsed->showLast(true);
    connect(SharedUsed, SIGNAL(done()), Mapper, SLOT(map()));
    Charts.append(SharedUsed);
    Mapper->setMapping(SharedUsed, Charts.indexOf(SharedUsed));

    toQueryParams params;
    params << toQValue(Utils::toSizeDecode(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::SizeUnit).toString()));
    FileUsed->setSQL(SQLOverviewFilespace);
    FileUsed->setParams(params);
    FileUsed->showLegend(false);
    connect(FileUsed, SIGNAL(done()), Mapper, SLOT(map()));
    Charts.append(FileUsed);
    Mapper->setMapping(FileUsed, Charts.indexOf(FileUsed));

    connect(Mapper, SIGNAL(mapped(int)), this, SLOT(refreshNext(int)));
}

toTuningOverview::~toTuningOverview()
{
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

void toTuningOverview::refresh(toConnection &conn)
{
    Utils::toBusy busy;
    try
    {
        toQueryParams params;
        params << toQValue(Utils::toSizeDecode(UnitString));

        toQList res = toQuery::readQuery(conn, SQLOverviewArchive, params);
        QString tmp = Utils::toShift(res);
        tmp += QString::fromLatin1("/");
        tmp += Utils::toShift(res);
        tmp += UnitString;
        Values["ArchiveInfo"] = tmp;

        res = toQuery::readQuery(conn, SQLOverviewRound, toQueryParams());
        tmp = Utils::toShift(res);
        tmp += QString::fromLatin1(" ms");
        Values["SendFromClient"] = tmp;

        tmp = Utils::toShift(res);
        tmp += QString::fromLatin1(" ms");
        Values["SendToClient"] = tmp;

        res = toQuery::readQuery(conn, SQLOverviewClientTotal, toQueryParams());
        tmp = Utils::toShift(res);
        Values["TotalClient"] = tmp;
        tmp = Utils::toShift(res);
        Values["ActiveClient"] = tmp;

        int totJob = 0;
        res = toQuery::readQuery(conn, SQLOverviewDedicated, toQueryParams());
        tmp = Utils::toShift(res);
        totJob += tmp.toInt();
        Values["DedicatedServer"] = tmp;

        res = toQuery::readQuery(conn, SQLOverviewDispatcher, toQueryParams());
        tmp = Utils::toShift(res);
        totJob += tmp.toInt();
        Values["DispatcherServer"] = tmp;

        res = toQuery::readQuery(conn, SQLOverviewShared, toQueryParams());
        tmp = Utils::toShift(res);
        totJob += tmp.toInt();
        Values["SharedServer"] = tmp;

        res = toQuery::readQuery(conn, SQLOverviewParallell, toQueryParams());
        tmp = Utils::toShift(res);
        totJob += tmp.toInt();
        Values["ParallellServer"] = tmp;

        res = toQuery::readQuery(conn, SQLOverviewBackground, toQueryParams());
        QStringList back;
        while (!res.empty())
        {
            tmp = Utils::toShift(res);
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
            QString job = Utils::toShift(res);
            totJob += job.toInt();
            tmp += job;
            tmp += QString::fromLatin1("</B>");
            back << tmp;
        }
        Values["Background"] = back.join(QString::fromLatin1(","));
        Values["TotalProcess"] = QString::number(totJob);

        double tot = 0;
        double sql = 0;
        res = toQuery::readQuery(conn, SQLOverviewSGA, params);
        while (!res.empty())
        {
            QString nam = Utils::toShift(res);
            tmp = Utils::toShift(res);
            if (nam == "Database Buffers" || nam == "Redo Buffers")
                Values[nam] = tmp + UnitString;
            else if (nam == "Fixed Size" || nam == "Variable Size")
                sql += tmp.toDouble();
            tot += tmp.toDouble();
        }
        tmp = toQValue::formatNumber(tot);
        tmp += UnitString;
        Values["SGATotal"] = tmp;
        tmp = toQValue::formatNumber(sql);
        tmp += UnitString;
        Values["SharedSize"] = tmp;

        res = toQuery::readQuery(conn, SQLOverviewLog, params);
        Values["RedoFiles"] = Utils::toShift(res);
        Values["ActiveRedo"] =  Utils::toShift(res);
        tmp = Utils::toShift(res);
        tmp += QString::fromLatin1("/");
        tmp += Utils::toShift(res);
        tmp += UnitString;
        Values["RedoSize"] = tmp;

        res = toQuery::readQuery(conn, SQLOverviewTablespaces, toQueryParams());
        Values["Tablespaces"] = Utils::toShift(res);

        res = toQuery::readQuery(conn, SQLOverviewDatafiles, toQueryParams());
        Values["Files"] = Utils::toShift(res);
    }
    TOCATCH

    Charts[0]->refresh();
}

void toTuningOverview::refreshNext(int i)
{
    if (i >= 0 && i <= Charts.size()-2)
    {   // refresh next chart
        Charts[i+1]->refresh();
    } else if (i == Charts.size() - 1) {
        // all charts refreshed
        poll();
    }
}

void toTuningOverview::setValue(QLabel *label, const QString &name)
{
    if (Values.contains(name))
    {
        label->setText(Values[name]);
        Values.remove(name);
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
            QMap<QString, QString>::iterator i = Values.find("Background");
            if (i != Values.end())
            {
                QList<QLabel*>::iterator labIt = Backgrounds.begin();

                QStringList lst = (*i).split(",");
                for (int j = 0; j < lst.count(); j++)
                {
                    QLabel *label;
                    if (labIt == Backgrounds.end() || *labIt == NULL)
                    {
                        label = new QLabel(GroupBoxBackground);
						GroupBoxBackground->layout()->addWidget(label);
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
    }
    TOCATCH
}
