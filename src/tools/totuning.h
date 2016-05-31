
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

#pragma once

#include "core/totool.h"
#include "core/tobackground.h"
#include "widgets/totoolwidget.h"
#include "ui_totuningoverviewui.h"

#include <list>
#include <map>

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QSemaphore>
#include <QtCore/QRunnable>
#include <QtGui/QResizeEvent>
#include <QLabel>
#include <QScrollArea>

class QAction;
class QMenu;
class QComboBox;
class QWidget;
class QLabel;
class QTabWidget;
class toBarChart;
class toConnection;
class toListView;
class toEventQuery;
class toResultItem;
class toResultLine;
class toResultLock;
class toResultTableView;
class toResultParam;
class toResultStats;
class toWaitEvents;

namespace ToConfiguration
{
    class Tuning : public ConfigContext
    {
            Q_OBJECT;
            Q_ENUMS(OptionTypeEnum);
        public:
            Tuning() : ConfigContext("Tuning", ENUM_REF(Tuning,OptionTypeEnum)) {};
            enum OptionTypeEnum
            {
                /*! True if there are no prefs for tuning.
                It's used when there is no m_tuningOverview defined in prefs. */
                FirstRunBool = 16000,
                OverviewBool,
                FileIOBool,
                WaitsBool,
                ChartsBool
            };
            QVariant defaultValue(int option) const;
    };
};

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
    toEventQuery *Query;

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
private slots:
    void queryDone(void);
};

class toTuningOverview : public QWidget, public Ui::toTuningOverviewUI
{
    Q_OBJECT;

    std::map<QString, QString> Values;
    QString UnitString;

    toBackground Poll;

    std::list<QLabel *> Backgrounds;
    void setupChart(toResultLine *chart, const QString &, const QString &, const toSQL &sql);
    void setValue(QLabel *label, const QString &val);
public:
    toTuningOverview(QWidget *parent = 0, const char *name = 0, toWFlags fl = 0);
    ~toTuningOverview();
#if 0
    void stop(void);
    void start(void);
#endif
    void refresh(toConnection &);
public slots:
    void poll(void);

private:
    QTimer *timer();
};

class toTuning : public toToolWidget
{
    Q_OBJECT;

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

#ifdef TORA3_CHART
    virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
    virtual void importData(std::map<QString, QString> &data, const QString &prefix);
#endif

public:
    toTuning(QWidget *parent, toConnection &connection);

public slots:
    virtual void refresh(void);
    virtual void changeTab(int);
    virtual void changeRefresh(const QString &str);
    virtual void slotWindowActivated(toToolWidget *widget);

    virtual void showTabMenu(void);
    virtual void enableTabMenu(QAction *);
};
