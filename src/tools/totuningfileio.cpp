
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

#include "tools/totuningfileio.h"
#include "tools/toresultbar.h"
#include "tools/toresultline.h"
#include "core/toeventquery.h"
#include "core/tosql.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QComboBox>

static toSQL SQLFileIO("toTuning:FileIO",
                       "select a.name,b.name,sysdate,\n"
                       "       c.phyrds,c.phywrts,c.phyblkrd,c.phyblkwrt,\n"
                       "       c.avgiotim*10,c.miniotim*10,c.maxiortm*10,c.maxiowtm*10\n"
                       "  from v$tablespace a,v$datafile b,v$filestat c\n"
                       " where a.ts# = b.ts# and b.file# = c.file#\n"
                       "union\n"
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
#if 0
        connect(toToolWidget::currentTool(this)->timer(), SIGNAL(timeout()), this, SLOT(refresh()));
#endif
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
        if (toConnection::currentConnection(this).version() >= "0800")
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
            toConnection &conn = toConnection::currentConnection(this);
            if (conn.version() < "0800")
                return ;
            LastStamp = CurrentStamp;
            CurrentStamp = time(NULL);
            Query = new toEventQuery(this, conn, toSQL::string(SQLFileIO, conn), toQueryParams(), toEventQuery::READ_ALL);

            auto c1 = connect(Query, &toEventQuery::dataAvailable, this, &toTuningFileIO::receiveData);
            auto c2 = connect(Query, &toEventQuery::done, this, &toTuningFileIO::queryDone);


            Query->start();
            LastTablespace = QString::null;
        }
        TOCATCH
    }
}

void toTuningFileIO::queryDone(toEventQuery *, unsigned long)
{
    if (Query)
    {
        delete Query;
        Query = NULL;
    }
}

void toTuningFileIO::receiveData(toEventQuery*)
{
    try
    {
        if (Query)
        {
            QString tablespace;
            QString datafile;
            QString timestr;
            while (Query->hasMore())
            {
                if (!Query->eof())
                {
                    tablespace = Query->readValue();
                    datafile = Query->readValue();
                    timestr = Query->readValue();
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

                double reads = Query->readValue().toDouble();
                double writes = Query->readValue().toDouble();
                double readBlk = Query->readValue().toDouble();
                double writeBlk = Query->readValue().toDouble();
                double avgTim = Query->readValue().toDouble();
                double minTim = Query->readValue().toDouble();
                double maxRead = Query->readValue().toDouble();
                double maxWrite = Query->readValue().toDouble();

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
                delete Query;
                Query = NULL;
            }
        }
    }
    TOCATCH
}

void toTuningFileIO::changeConnection(void)
{
    foreach(toBarChart *b, ReadsCharts.values())
        delete b;
    ReadsCharts.clear();

    foreach(toLineChart *l, TimeCharts.values())
        delete l;
    TimeCharts.clear();
    LastValues.clear();
    refresh();
}
