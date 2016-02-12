
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

#include "tools/towaitevents.h"
#include "core/utils.h"
#include "core/tosql.h"
#include "core/toeventquery.h"
#include "widgets/toresultview.h"
#include "core/totool.h"
#include "widgets/totreewidget.h"
#include "core/totimer.h"
#include "core/toconfiguration.h"
#include "core/toeditorsetting.h"
#include "widgets/totoolwidget.h"

#include <QtCore/QSettings>
#include <QComboBox>
#include <QLabel>
#include <QSplitter>
#include <QToolBar>
#include <QVBoxLayout>

class toWaitEventsItem : public toResultViewItem
{
        int Color;
    public:
        toWaitEventsItem(toTreeWidget *parent, toTreeWidgetItem *after, const QString &buf = QString::null)
            : toResultViewItem(parent, after, QString::null)
        {
            Color = 0;
            setText(1, buf);
            int num = 1;
            if (after)
                num = after->text(0).toInt() + 1;
            setText(0, QString::number(num));
        }
        void setColor(int color)
        {
            Color = color;
        }
#if 0                           // disabled, wrong override
        virtual void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
        {
            if (column == 0)
            {
                QString ct = text(column);

                QBrush brush(isSelected() ? toChartBrush(Color) : QBrush(cg.base()));

                p->fillRect(0, 0, width, height(), QBrush(brush.color()));
                if (brush.style() != Qt::SolidPattern)
                    p->fillRect(0, 0, width, height(), QBrush(Qt::white, brush.style()));
            }
            else
                toTreeWidgetItem::paintCell(p, cg, column, width, align);
        }
#endif
};

toWaitEvents::toWaitEvents(QWidget *parent, const char *name)
    : QWidget(parent)
{

    if (name)
        setObjectName(name);

    setup( -1);
}

toWaitEvents::toWaitEvents(int session, QWidget *parent, const char *name)
    : QWidget(parent)
{

    if (name)
        setObjectName(name);

    setup(session);
}

toWaitEvents::~toWaitEvents()
{
    QSettings s;
    s.beginGroup("toWaitEvents");
    s.setValue("splitter", splitter->saveState());
    s.endGroup();
}

void toWaitEvents::setup(int session)
{
    Session = session;

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    QToolBar *toolbar = Utils::toAllocBar(this, tr("Server Tuning"));
    vbox->addWidget(toolbar);

    toolbar->addWidget(
        new QLabel(tr("Display") + " ", toolbar));

    QComboBox *type = new QComboBox(toolbar);
    type->addItem(tr("Time"));
    type->addItem(tr("Count"));
    toolbar->addWidget(type);
    connect(type, SIGNAL(activated(int)), this, SLOT(changeType(int)));

    QLabel *stretch = new QLabel(toolbar);
    toolbar->addWidget(stretch);
    stretch->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    stretch->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                       QSizePolicy::Minimum));

    splitter = new QSplitter(Qt::Horizontal, this);
    vbox->addWidget(splitter);

    Types = new toTreeWidget(splitter);
    Types->addColumn(tr("Color"));
    Types->addColumn(tr("Wait type"));
    Types->addColumn(tr("Delta (ms/s)"));
    Types->addColumn(tr("Total (ms)"));
    Types->addColumn(tr("Delta (1/s)"));
    Types->addColumn(tr("Total"));
    Types->setColumnAlignment(2, Qt::AlignRight);
    Types->setColumnAlignment(3, Qt::AlignRight);
    Types->setColumnAlignment(4, Qt::AlignRight);
    Types->setColumnAlignment(5, Qt::AlignRight);
    Types->setAllColumnsShowFocus(true);
    Types->setSelectionMode(toTreeWidget::Multi);
    Types->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    QString str(toConfigurationNewSingle::Instance().option(ToConfiguration::Editor::ListTextFont).toString());
    if (!str.isEmpty())
    {
        QFont font(Utils::toStringToFont(str));
        Types->setFont(font);
    }
    connect(Types, SIGNAL(selectionChanged()), this, SLOT(changeSelection()));

    QFrame *frame = new QFrame(splitter);
    QGridLayout *layout = new QGridLayout(frame);

#ifdef TORA3_GRAPH
    Delta = new toResultBar(frame);
    Delta->setTitle(tr("System wait events"));
    Delta->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    Delta->showLegend(false);
    Delta->setYPostfix(" " + tr("ms/s"));
    Delta->setYPostfix(" " + tr("ms/s"));
    Delta->setSQLName(QString::fromLatin1("toTuning:WaitEvents"));
    layout->addWidget(Delta, 0, 0, 1, 0);

    DeltaTimes = new toResultBar(frame);
    DeltaTimes->setTitle(tr("System wait events count"));
    DeltaTimes->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    DeltaTimes->showLegend(false);
    DeltaTimes->setYPostfix(" " + tr("ms/s"));
    DeltaTimes->hide();
    DeltaTimes->setYPostfix(" " + tr("waits/s"));
    DeltaTimes->setSQLName(QString::fromLatin1("toTuning:WaitEventsCount"));
    layout->addWidget(DeltaTimes, 0, 0, 1, 0);

    DeltaPie = new toPieChart(frame);
    DeltaPie->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    DeltaPie->showLegend(false);
    layout->addWidget(DeltaPie, 1, 0);
    AbsolutePie = new toPieChart(frame);
    AbsolutePie->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    AbsolutePie->showLegend(false);
    layout->addWidget(AbsolutePie, 1, 1);
#endif
    Query = NULL;
    start();
    try
    {
        connect(toToolWidget::currentTool(this), SIGNAL(connectionChange()), this, SLOT(connectionChanged()));
    }
    TOCATCH

    QSettings s;
    s.beginGroup("toWaitEvents");
    QByteArray ba = s.value("splitter", QByteArray()).toByteArray();
    s.endGroup();
    if (ba.isNull())
    {
        // first run
        QList<int> siz;
        siz << 1 << 2;
        splitter->setSizes(siz);
    }
    else
        splitter->restoreState(ba);

    LastTime = 0;

    First = true;
    ShowTimes = false;
}

void toWaitEvents::changeType(int item)
{
    ShowTimes = item;
#ifdef TORA3_GRAPH
    if (ShowTimes)
    {
        DeltaTimes->show();
        Delta->hide();
    }
    else
    {
        DeltaTimes->hide();
        Delta->show();
    }
#endif
    changeSelection();
}

void toWaitEvents::setSession(int session)
{
    try
    {
        Types->clear();
        Session = session;
        First = true;
        Now = QString::null;
        LastTime = 0;
        Labels.clear();
        LastCurrent.clear();
        Current.clear();
        CurrentTimes.clear();
        Relative.clear();
        RelativeTimes.clear();
        Enabled.clear();
        delete Query;
        Query = NULL;
        refresh();
    }
    TOCATCH
}

void toWaitEvents::start(void)
{
    try
    {
        connect(toToolWidget::currentTool(this)->timer(), SIGNAL(timeout()), this, SLOT(refresh()));
    }
    TOCATCH
}

void toWaitEvents::stop(void)
{
    try
    {
        disconnect(toToolWidget::currentTool(this)->timer(), SIGNAL(timeout()), this, SLOT(refresh()));
    }
    TOCATCH
}

void toWaitEvents::changeSelection(void)
{
    int count = int(Labels.size());

    bool *enabled = new bool[count];
    int typ = 0;
    std::map<QString, int> usedMap;
    {
        for (std::list<QString>::iterator i = Labels.begin(); i != Labels.end(); i++)
        {
            usedMap[*i] = typ;
            enabled[typ] = false;
            typ++;
        }
    }
    for (toTreeWidgetItem *item = Types->firstChild(); item; item = item->nextSibling())
    {
        QString txt = item->text(1);
        if (usedMap.find(txt) == usedMap.end())
            Utils::toStatusMessage(tr("Internal error, can't find (%1) in usedMap").arg(txt));
        if (item->isSelected())
            enabled[usedMap[txt]] = true;
    }

    try
    {
        Enabled.clear();
        int ind = 0;
        {
            for (std::list<QString>::iterator i = Labels.begin(); i != Labels.end(); i++)
            {
                Enabled.insert(Enabled.end(), enabled[ind]);
                ind++;
            }
        }
#ifdef TORA3_GRAPH
        Delta->setEnabledCharts(Enabled);
        DeltaTimes->setEnabledCharts(Enabled);
#endif

        std::list<double> absolute;
        std::list<double> relative;
        {
            std::list<double>::iterator i = (ShowTimes ? LastTimes : LastCurrent).begin();
            std::list<double>::iterator j = (ShowTimes ? RelativeTimes : Relative).begin();
            std::list<bool>::iterator k = Enabled.begin();
            while (i != (ShowTimes ? LastTimes : LastCurrent).end() && k != Enabled.end())
            {
                if (*k)
                {
                    if (j != (ShowTimes ? RelativeTimes : Relative).end())
                        relative.insert(relative.end(), *j);
                    absolute.insert(absolute.end(), *i);
                }
                else
                {
                    if (j != (ShowTimes ? RelativeTimes : Relative).end())
                        relative.insert(relative.end(), 0);
                    absolute.insert(absolute.end(), 0);
                }
                i++;
                if (j != (ShowTimes ? RelativeTimes : Relative).end())
                    j++;
                k++;
            }
        }

        double total = 0;
        {
            for (std::list<double>::iterator i = absolute.begin(); i != absolute.end(); i++)
                total += *i;
        }
#ifdef TORA3_GRAPH
        AbsolutePie->setValues(absolute, Labels);
        AbsolutePie->setTitle(tr("Absolute system wait events\nTotal %1%2").
                              arg(total / 1000).arg(QString::fromLatin1(ShowTimes ? "" : " s")));

        total = 0;
        for (std::list<double>::iterator i = relative.begin(); i != relative.end(); i++)
            total += *i;
        DeltaPie->setValues(relative, Labels);
        if (total > 0)
            DeltaPie->setTitle(tr("Delta system wait events\nTotal %1%2").
                               arg(total).arg(QString::fromLatin1(ShowTimes ? "/s" : " ms/s")));

        else
            DeltaPie->setTitle(QString::null);
#endif
    }
    TOCATCH
    delete[] enabled;
}

void toWaitEvents::connectionChanged(void)
{
    LastCurrent.clear();
    LastTimes.clear();
    Labels.clear();

    Relative.clear();
    RelativeTimes.clear();

    delete Query;
    Query = NULL;
    LastTime = 0;

    First = true;
    refresh();
}

void toWaitEvents::poll(void)
{
    if (!Utils::toCheckModal(this))
        return;

    try
    {
        while (Query->hasMore())
        {
            QString cur = (QString)Query->readValue();
            Now = (QString)Query->readValue();
            if (First)
            {
                Labels.insert(Labels.end(), cur);
                Current.insert(Current.end(), Query->readValue().toDouble());
                CurrentTimes.insert(CurrentTimes.end(), Query->readValue().toDouble());
            }
            else
            {
                double val = Query->readValue().toDouble();
                double tim = Query->readValue().toDouble();
                std::list<double>::iterator i = Current.begin();
                std::list<double>::iterator j = CurrentTimes.begin();
                std::list<QString>::iterator k = Labels.begin();
                while (i != Current.end() && j != CurrentTimes.end() && k != Labels.end())
                {
                    if (*k == cur)
                    {
                        *i = val;
                        *j = tim;
                        break;
                    }
                    i++;
                    j++;
                    k++;
                }
            }
            Query->readValue().toDouble();
        }
    }
    catch (const QString &exc)
    {
        delete Query;
        Query = NULL;
        Utils::toStatusMessage(exc);
    }
}

void toWaitEvents::queryDone(void )
{
    std::map<QString, bool> types;
    toTreeWidgetItem *item = NULL;
    {
        for (toTreeWidgetItem *ci = Types->firstChild(); ci; ci = ci->nextSibling())
        {
            types[ci->text(1)] = true;
            item = ci;
        }
    }

    {
        std::list<double>::iterator j = CurrentTimes.begin();
        for (std::list<QString>::iterator i = Labels.begin(); i != Labels.end(); i++, j++)
        {
            if ((*j) != 0 && types.find(*i) == types.end())
            {
                item = new toWaitEventsItem(Types, item, *i);
                item->setSelected(First && HideMap.find(*i) == HideMap.end());
                types[*i] = true;
            }
        }
    }
    if (First)
    {
#ifdef TORA3_GRAPH
        Delta->setLabels(Labels);
        DeltaTimes->setLabels(Labels);
#endif
        First = false;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch()/1000;

    for (toTreeWidgetItem *ci = Types->firstChild(); ci; ci = ci->nextSibling())
    {
        toWaitEventsItem * item = dynamic_cast<toWaitEventsItem *>(ci);
        if (item)
        {
            int col = 0;
            std::list<double>::iterator i = Current.begin();
            std::list<double>::iterator j = CurrentTimes.begin();
            std::list<QString>::iterator k = Labels.begin();
            while (i != Current.end() && j != CurrentTimes.end() && k != Labels.end())
            {
                if (item->text(1) == *k)
                {
                    item->setColor(col);
                    item->setText(2, QString::number((*i - item->text(3).toDouble()) / (std::max)(int(now - LastTime), 1)));
                    item->setText(3, QString::number(*i));
                    item->setText(4, QString::number((*j - item->text(5).toDouble()) / (std::max)(int(now - LastTime), 1)));
                    item->setText(5, QString::number(*j));
                    break;
                }
                col++;
                i++;
                j++;
                k++;
            }
        }
    }

    Relative.clear();
    RelativeTimes.clear();

    std::list<double>::iterator j = LastCurrent.begin();
    std::list<double>::iterator i = Current.begin();
    while (i != Current.end() && j != LastCurrent.end())
    {
        Relative.insert(Relative.end(), ((*i) - (*j)) / (std::max)(int(now - LastTime), 1));
        i++;
        j++;
    }

    j = LastTimes.begin();
    i = CurrentTimes.begin();
    while (i != CurrentTimes.end() && j != LastTimes.end())
    {
        RelativeTimes.insert(RelativeTimes.end(), ((*i) - (*j)) / (std::max)(int(now - LastTime), 1));
        i++;
        j++;
    }

    LastTime = now;
    LastTimes = CurrentTimes;
    LastCurrent = Current;

#ifdef TORA3_GRAPH
    if (Relative.begin() != Relative.end())
    {
        Delta->addValues(Relative, Now);
        DeltaTimes->addValues(RelativeTimes, Now);
    }
#endif

    changeSelection();

    delete Query;
    Query = NULL;
} // queryDone

static toSQL SQLSessionWaitEvents("toWaitEvents:Session",
                                  "SELECT b.name,\n"
                                  "       SYSDATE,\n"
                                  "       NVL(a.time_waited,0)*10,\n"
                                  "       NVL(a.total_waits,0),\n"
                                  "       NVL(a.time_waited,0)\n"
                                  "  FROM v$session_event a,\n"
                                  "       v$event_name b\n"
                                  " WHERE b.name=a.event(+)\n"
                                  "   AND a.sid=:sid<int>\n"
                                  "   AND b.name NOT LIKE'%timer%'\n"
                                  "   AND b.name NOT IN('rdbms ipc message',\n"
                                  "                     'SQL*Net message from client')\n"
                                  " UNION ALL SELECT b.name,\n"
                                  "       SYSDATE,\n"
                                  "       NVL(a.time_waited,0)*10,\n"
                                  "       NVL(a.total_waits,0),\n"
                                  "       1\n"
                                  "  FROM v$session_event a,\n"
                                  "       v$event_name b\n"
                                  " WHERE b.name=a.event(+)\n"
                                  "   AND a.sid=:sid<int>\n"
                                  "   AND (b.name LIKE'%timer%'OR b.name IN('rdbms ipc message',\n"
                                  "                                         'SQL*Net message from client'))\n"
                                  " UNION ALL SELECT b.name,\n"
                                  "       SYSDATE,\n"
                                  "       NVL(a.VALUE,0)*10,\n"
                                  "       1,\n"
                                  "       NVL(a.VALUE,0)\n"
                                  "  FROM v$sesstat a,\n"
                                  "       v$statname b\n"
                                  " WHERE b.name='CPU used by this session'\n"
                                  "   AND a.sid=:sid<int>\n"
                                  "   AND a.statistic#=b.statistic#\n"
                                  " ORDER BY 5 DESC,\n"
                                  "          3 DESC,\n"
                                  "          4 DESC",
                                  "Get all available session wait events");

static toSQL SQLWaitEvents("toWaitEvents:System",
                           "SELECT b.name,\n"
                           "       SYSDATE,\n"
                           "       NVL(a.time_waited,0)*10,\n"
                           "       NVL(a.total_waits,0),\n"
                           "       NVL(a.time_waited,0)\n"
                           "  FROM v$system_event a,\n"
                           "       v$event_name b\n"
                           " WHERE b.name=a.event(+)\n"
                           "   AND b.name NOT LIKE'%timer%'\n"
                           "   AND b.name NOT IN('rdbms ipc message',\n"
                           "                     'SQL*Net message from client')\n"
                           " UNION ALL SELECT b.name,\n"
                           "       SYSDATE,\n"
                           "       NVL(a.time_waited,0)*10,\n"
                           "       NVL(a.total_waits,0),\n"
                           "       1\n"
                           "  FROM v$system_event a,\n"
                           "       v$event_name b\n"
                           " WHERE b.name=a.event(+)\n"
                           "   AND (b.name LIKE'%timer%'OR b.name IN('rdbms ipc message',\n"
                           "                                         'SQL*Net message from client'))\n"
                           " UNION ALL SELECT s.name,\n"
                           "       SYSDATE,\n"
                           "       NVL(s.VALUE,0)*10,\n"
                           "       1,\n"
                           "       NVL(s.VALUE,0)\n"
                           "  FROM v$sysstat s\n"
                           " WHERE s.name='CPU used by this session'\n"
                           " ORDER BY 5 DESC,\n"
                           "          3 DESC,\n"
                           "          4 DESC",
                           "Get all available system wait events");

void toWaitEvents::refresh(void)
{
    try
    {
        if (Query || LastTime == QDateTime::currentMSecsSinceEpoch()/1000)
            return ;

        toConnection &conn = toToolWidget::currentTool(this)->connection();
        toQueryParams par;
        QString sql;

        if (Session > 0)
        {
            sql = toSQL::string(SQLSessionWaitEvents, conn);
            par.insert(par.end(), toQValue(Session));
        }
        else
            sql = toSQL::string(SQLWaitEvents, conn);
        Query = new toEventQuery(this, conn, sql, par, toEventQuery::READ_ALL);
        connect(Query, SIGNAL(dataAvailable()), this, SLOT(poll()));
        connect(Query, SIGNAL(done()), this, SLOT(queryDone()));
        Query->start();
    }
    TOCATCH
}

void toWaitEvents::importData(std::map<QString, QString> &data, const QString &prefix)
{
    std::map<QString, QString>::iterator i;
    int id = 1;
    while ((i = data.find(prefix + ":" + QString::number(id).toLatin1())) != data.end())
    {
        HideMap[(*i).second] = true;
        id++;
    }
}

void toWaitEvents::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    int id = 1;
    for (toTreeWidgetItem *ci = Types->firstChild(); ci; ci = ci->nextSibling())
    {
        toWaitEventsItem * item = dynamic_cast<toWaitEventsItem *>(ci);
        if (!item->isSelected())
        {
            data[prefix + ":" + QString::number(id).toLatin1()] = item->allText(1);
            id++;
        }
    }
}
