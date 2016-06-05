
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

#include "tools/totuningcharts.h"
#include "tools/toresultbar.h"
#include "tools/toresultline.h"
#include "tools/toresultpie.h"
#include "core/toeventquery.h"
#include "core/tosql.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"

toTuningCharts::toTuningCharts(QWidget *parent)
    : QWidget(parent)
    , Mapper(new QSignalMapper(this))
{
    using namespace ToConfiguration;
    QVBoxLayout *chartBox = new QVBoxLayout;
    chartBox->setSpacing(0);
    chartBox->setContentsMargins(0, 0, 0, 0);
    setLayout(chartBox);

    QString unitStr = toConfigurationNewSingle::Instance().option(Global::SizeUnit).toString();
    toQueryParams unit;
    unit << toQValue(Utils::toSizeDecode(unitStr));

    QList<QString> val = toSQL::range("toTuning:Charts");
    for (QList<QString>::iterator i = val.begin(); i != val.end(); i++)
    {
        QStringList parts = (*i).split(":");
        if (parts.count() == 3)
        {
            parts.append(parts[2]);
            parts[2] = QString::fromLatin1("Charts");
        }

        if (parts[3].mid(1, 1) == QString::fromLatin1("B"))
        {
            toResultBar *chart = new toResultBar(this);
            chartBox->addWidget(chart);
            Charts.append(chart);
            chart->setTitle(parts[3].mid(3));
            toQueryParams par;
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
            chart->setSQL(toSQL::sql(*i));
            chart->setParams(par);
            connect(chart, SIGNAL(done()), Mapper, SLOT(map()));
            Mapper->setMapping(chart, Charts.indexOf(chart));
        }
        else if (parts[3].mid(1, 1) == QString::fromLatin1("L") || parts[3].mid(1, 1) == QString::fromLatin1("C"))
        {
            toResultLine *chart;
            if (parts[3].mid(1, 1) == QString::fromLatin1("C"))
                chart = new toTuningMiss(this);
            else
                chart = new toResultLine(this);
            chartBox->addWidget(chart);
            Charts.append(chart);
            toQueryParams par;
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
            chart->setSQL(toSQL::sql(*i));
            chart->setParams(par);
            connect(chart, SIGNAL(done()), Mapper, SLOT(map()));
            Mapper->setMapping(chart, Charts.indexOf(chart));
        }
        else if (parts[3].mid(1, 1) == QString::fromLatin1("P"))
        {
            toResultPie *chart = new toResultPie(this);
            chart->setTitle(parts[3].mid(3));
            chartBox->addWidget(chart);
            Charts.append(chart);
            if (parts[3].mid(2, 1) == QString::fromLatin1("S"))
            {
                chart->setSQL(toSQL::sql(*i));
                chart->setParams(toQueryParams() << unit);
                chart->setPostfix(unitStr);
            }
            else
                chart->setSQL(toSQL::sql(*i));
            connect(chart, SIGNAL(done()), Mapper, SLOT(map()));
            Mapper->setMapping(chart, Charts.indexOf(chart));
        }
        else
            Utils::toStatusMessage(tr("Wrong format of name on chart (%1).").arg(QString(*i)));
    }

    connect(Mapper, SIGNAL(mapped(int)), this, SLOT(refreshNext(int)));

}

toTuningCharts::~toTuningCharts(void)
{
}

void toTuningCharts::refresh(void)
{
    Charts[0]->refresh();
}

void toTuningCharts::refreshNext(int i)
{
    if (i >= 0 && i <= Charts.size()-2)
    {   // refresh next chart
        Charts[i+1]->refresh();
    }
}

toTuningMiss::toTuningMiss(QWidget *parent, const char *name)
    : toResultLine(parent, name)
{}

std::list<double> toTuningMiss::transform(std::list<double> &inp)
{
    std::list<double> ret;
    for (std::list<double>::iterator i = inp.begin(); i != inp.end(); i++)
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
