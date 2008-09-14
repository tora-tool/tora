
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#include "utils.h"

#include "toanalyze.h"
#include "tobarchart.h"
#include "toconf.h"
#include "toresultview.h"
#include "toworksheetstatistic.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

#include <QString>
#include <QList>
#include <QPixmap>
#include <QMenu>
#include <QVBoxLayout>
#include <QAction>

#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/grid.xpm"
#include "icons/trash.xpm"
#include "icons/tree.xpm"

toAnalyze *toWorksheetStatistic::Widget;

toWorksheetStatistic::toWorksheetStatistic(QWidget *parent)
        : QWidget(parent)
{

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0, 0, 0, 0);

    QToolBar *toolbar = toAllocBar(this, tr("Server Tuning"));
    vlayout->addWidget(toolbar);

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(fileopen_xpm))),
                       tr("Load statistics from file"),
                       this,
                       SLOT(load(void)));

    SaveMenu = new QMenu(toolbar);
    SaveMenu->setIcon(QPixmap(const_cast<const char**>(filesave_xpm)));
    SaveMenu->setTitle(tr("Save statistics to file"));
    toolbar->addAction(SaveMenu->menuAction());
    connect(SaveMenu, SIGNAL(aboutToShow()), this, SLOT(displayMenu()));
    connect(SaveMenu, SIGNAL(triggered(QAction *)), this, SLOT(save(QAction *)));

    toolbar->addSeparator();

    RemoveMenu = new QMenu(toolbar);
    RemoveMenu->setIcon(QPixmap(const_cast<const char**>(trash_xpm)));
    RemoveMenu->setTitle(tr("Remove statistics"));
    toolbar->addAction(RemoveMenu->menuAction());
    connect(RemoveMenu, SIGNAL(aboutToShow()), this, SLOT(displayMenu()));
    connect(RemoveMenu, SIGNAL(triggered(QAction *)), this, SLOT(remove(QAction *)));

    Splitter = new QSplitter(Qt::Vertical, this);
    vlayout->addWidget(Splitter);

    toolbar->addSeparator();

    ShowCharts = new QToolButton(toolbar);
    ShowCharts->setCheckable(true);
    ShowCharts->setIcon(QIcon(QPixmap(const_cast<const char**>(grid_xpm))));
    ShowCharts->setToolTip(tr("Display charts"));
    ShowCharts->setChecked(true);
    connect(ShowCharts, SIGNAL(toggled(bool)), this, SLOT(showCharts(bool)));
    toolbar->addWidget(ShowCharts);

    ShowPlans = new QToolButton(toolbar);
    ShowPlans->setCheckable(true);
    ShowPlans->setIcon(QIcon(QPixmap(const_cast<const char**>(tree_xpm))));
    ShowPlans->setToolTip(tr("Display execution plans"));
    ShowPlans->setChecked(true);
    connect(ShowPlans, SIGNAL(toggled(bool)), this, SLOT(showPlans(bool)));
    toolbar->addWidget(ShowPlans);

    QWidget *w = new QWidget(toolbar);
    toolbar->addWidget(w);
    w->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                 QSizePolicy::Minimum));

    Dummy = new QWidget(Splitter);

    try
    {
        Tool = dynamic_cast<toAnalyze *>(toCurrentTool(this));
        if (!Widget)
            Widget = Tool;
    }
    TOCATCH;

    setLayout(vlayout);
}

toWorksheetStatistic::~toWorksheetStatistic()
{
    if (Widget == Tool)
        Widget = NULL;
}

void toWorksheetStatistic::saveStatistics(std::map<QString, QString> &stats)
{
    if (!Widget)
        toAnalyze::createTool();
    if (Widget)
        Widget->worksheet()->addStatistics(stats);
}

void toWorksheetStatistic::addStatistics(std::map<QString, QString> &stats)
{
    data cur;
    cur.Top = new QWidget(Splitter);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);

    QWidget *hcontainer = new QWidget(cur.Top);
    QHBoxLayout *box = new QHBoxLayout;
    box->setSpacing(0);
    box->setContentsMargins(0, 0, 0, 0);
    hcontainer->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                          QSizePolicy::Fixed));
    vbox->addWidget(hcontainer);

    cur.Action = new QAction(stats["Description"], cur.Top);

    cur.Label = new QLabel(stats["Description"], hcontainer);
    cur.Label->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    box->addWidget(cur.Label);

    QCheckBox *check = new QCheckBox(tr("Hide"), hcontainer);
    check->setCheckState(Qt::Unchecked);
    box->addWidget(check);

    cur.Charts = new QSplitter(Qt::Horizontal, cur.Top);
    vbox->addWidget(cur.Charts);

    cur.Statistics = new toListView(cur.Charts);
    cur.Statistics->importData(stats, "Stat");

    cur.Wait = new toBarChart(cur.Charts);
    cur.Wait->importData(stats, "Wait");

    cur.IO = new toBarChart(cur.Charts);
    cur.IO->importData(stats, "IO");

    cur.Plan = new toListView(cur.Charts);
    cur.Plan->importData(stats, "Plan");

    connect(check, SIGNAL(toggled(bool)), cur.Charts, SLOT(setHidden(bool)));

    cur.Top->setLayout(vbox);
    hcontainer->setLayout(box);

    QList<int> sizes;
    sizes << 1 << 1 << 1 << 1;
    cur.Charts->setSizes(sizes);
    if (!cur.Plan->firstChild())
    {
        delete cur.Plan;
        cur.Plan = NULL;
    }
    else if (!ShowPlans->isChecked())
        cur.Plan->hide();

    if (!ShowCharts->isChecked())
    {
        cur.Statistics->hide();
        cur.Wait->hide();
        cur.IO->hide();
    }

    cur.Top->show();
    hcontainer->show();
    cur.Charts->show();
    setFocus();

    Open.insert(Open.end(), cur);

    if (Dummy)
    {
        delete Dummy;
        Dummy = NULL;
    }
}

void toWorksheetStatistic::showPlans(bool show)
{
    for (std::list<data>::iterator i = Open.begin();i != Open.end();i++)
    {
        if ((*i).Plan)
        {
            if (show)
            {
                (*i).Plan->show();
            }
            else
            {
                (*i).Plan->hide();
            }
        }
    }
}

void toWorksheetStatistic::showCharts(bool show)
{
    for (std::list<data>::iterator i = Open.begin();i != Open.end();i++)
    {
        if (show)
        {
            (*i).Statistics->show();
            (*i).Wait->show();
            (*i).IO->show();
        }
        else
        {
            (*i).Statistics->hide();
            (*i).Wait->hide();
            (*i).IO->hide();
        }
    }
}

void toWorksheetStatistic::save(QAction *action)
{
    for (std::list<data>::iterator i = Open.begin();i != Open.end();i++)
    {
        if ((*i).Action == action)
        {
            QString fn = toSaveFilename(QString::null, QString::fromLatin1("*.stat"), this);
            if (!fn.isEmpty())
            {
                std::map<QString, QString> stat;
                (*i).Statistics->exportData(stat, "Stat");
                (*i).IO->exportData(stat, "IO");
                (*i).Wait->exportData(stat, "Wait");
                if ((*i).Plan)
                    (*i).Plan->exportData(stat, "Plan");
                stat["Description"] = (*i).Label->text();
                try
                {
                    toConfigurationSingle::Instance().saveMap(fn, stat);
                }
                TOCATCH;
            }
            break;
        }
    }
}

void toWorksheetStatistic::remove(QAction *action)
{
    for (std::list<data>::iterator i = Open.begin();i != Open.end();i++)
    {
        if ((*i).Action == action)
        {
            delete(*i).Top;
            if (Open.size() == 1)
            {
                Dummy = new QWidget(Splitter);
                Dummy->show();
            }
            Open.erase(i);
            break;
        }
    }
}

void toWorksheetStatistic::load(void)
{
    QString filename = toOpenFilename(QString::null, QString::fromLatin1("*.stat"), this);
    if (!filename.isEmpty())
    {
        try
        {
            std::map<QString, QString> ret;
            toConfigurationSingle::Instance().loadMap(filename, ret);
            addStatistics(ret);
        }
        TOCATCH
    }
}

void toWorksheetStatistic::displayMenu(void)
{
    SaveMenu->clear();
    RemoveMenu->clear();
    for (std::list<data>::iterator i = Open.begin(); i != Open.end(); i++)
    {
        SaveMenu->addAction((*i).Action);
        RemoveMenu->addAction((*i).Action);
    }
}
