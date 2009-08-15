
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

#ifndef TOTUNING_H
#define TOTUNING_H

#include "config.h"
#include "tobackground.h"
#include "toresultline.h"
#include "totool.h"
#include "tothread.h"
#include "ui_totuningoverviewui.h"

#include <list>
#include <map>

#include <QString>
#include <QResizeEvent>
#include <QLabel>
#include <QScrollArea>
#include <QList>

class QAction;
class QMenu;
class QComboBox;
class QWidget;
class QLabel;
class QTabWidget;
class toBarChart;
class toConnection;
class toListView;
class toNoBlockQuery;
class toResultItem;
class toResultLock;
class toResultTableView;
class toResultParam;
class toResultStats;
class toWaitEvents;

class toTuningMiss : public toResultLine
{
public:
    toTuningMiss(QWidget *parent = 0, const char *name = 0);
    virtual std::list<double> transform(std::list<double> &trans);
};

class toTuningFileIO : public QWidget
{
    Q_OBJECT

    QWidget *TablespaceTime;
    QWidget *TablespaceReads;
    QWidget *FileTime;
    QWidget *FileReads;

    QString LastTablespace;
    double TblReads;
    double TblWrites;
    double TblReadBlk;
    double TblWriteBlk;
    double TblAvg;
    double TblMin;
    double TblMaxRead;
    double TblMaxWrite;
    toNoBlockQuery *Query;

    time_t CurrentStamp;
    time_t LastStamp;
    std::map<QString, toBarChart *> ReadsCharts;
    std::map<QString, toLineChart *> TimeCharts;
    std::map<QString, std::list<double> > LastValues;

    void saveSample(const QString &, const QString &,
                    double reads, double writes,
                    double readBlk, double writeBlk,
                    double avgTim, double minTim,
                    double maxRead, double maxWrite);

    void allocCharts(const QString &);

    toBackground Poll;
public:
    toTuningFileIO(QWidget *parent = 0);
    ~toTuningFileIO();

    void stop(void);
    void start(void);
public slots:
    void refresh(void);
    void changeCharts(int val);
    void changeConnection(void);
    void poll(void);
};

class toTuningOverview : public QWidget, public Ui::toTuningOverviewUI
{
    Q_OBJECT

    bool Quit;
    toSemaphore Done;
    std::map<QString, QString> Values;
    toConnection *Connection;
    toLock Lock;
    QString UnitString;

struct overviewQuery : public toTask
    {
        toTuningOverview &Parent;
        overviewQuery(toTuningOverview &parent)
                : Parent(parent)
        { }
        virtual void run(void);
        void setValue(const QString &name, const QString &val);
    };

    friend struct overviewQuery;

    toBackground Poll;

    std::list<QLabel *> Backgrounds;
    void setupChart(toResultLine *chart, const QString &, const QString &, const toSQL &sql);
    void setValue(QLabel *label, const QString &val);
public:
    toTuningOverview(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);
    ~toTuningOverview();
    void stop(void);
    void start(void);

public slots:
    void refresh(void);
    void poll(void);
};

class toTuning : public toToolWidget
{
    Q_OBJECT

    QTabWidget        *Tabs;
    toListView        *Indicators;
    toResultParam     *Parameters;
    toResultTableView *Options;
    toResultLock      *BlockingLocks;
    toResultItem      *Licenses;
    toResultStats     *Statistics;
    toResultTableView *LibraryCache;
    toResultTableView *ControlFiles;
    toWaitEvents      *Waits;
    QScrollArea       *ChartContainer;

    QList<QWidget *> Charts;

    QComboBox        *Refresh;
    toTuningOverview *Overview;
    toTuningFileIO   *FileIO;
    QWidget          *LastTab;

    QMenu   *ToolMenu;
    QAction *refreshAct;
    QAction *changeRefreshAct;
    QMenu   *tabMenu;

    virtual void enableTab(const QString &name, bool enable);
    virtual QWidget *tabWidget(const QString &name);

    virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
    virtual void importData(std::map<QString, QString> &data, const QString &prefix);

public:
    toTuning(QWidget *parent, toConnection &connection);

public slots:
    virtual void refresh(void);
    virtual void changeTab(int);
    virtual void changeRefresh(const QString &str);
    virtual void windowActivated(QMdiSubWindow *widget);

    virtual void showTabMenu(void);
    virtual void enableTabMenu(QAction *);
};

#endif
