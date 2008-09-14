/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOCHARTMANAGER_H
#define TOCHARTMANAGER_H

#include "config.h"
#include "tohelp.h"
#include "ui_tochartsetupui.h"
#include "ui_tochartalarmui.h"

#include <qfile.h>

#include <QTimer>

#include <list>
#include <map>
#include <algorithm>

class toChartManager;
class toLineChart;
class toListView;
class toResult;
class toChartSetup;
class toChartAlarm;
class toChartHandler;

class toChartReceiver : public QObject
{
    Q_OBJECT

    QString LastName;
    toChartHandler *Parent;
    toLineChart *Chart;
    toResult *Result;
public:
    toChartReceiver(toChartHandler *parent, toLineChart *chart);

    toLineChart *chart(void)
    {
        return Chart;
    }
    toResult *result(void);
    QString name(void);
public slots:
    void valueAdded(std::list<double> &value, const QString &xValues);
};

class toChartManager : public QWidget, public toHelpContext
{
    Q_OBJECT

    struct alarmSignal;
    struct chartAlarm;
    friend struct chartAlarm;
    friend struct alarmSignal;

    enum action
    {
        StatusMessage = 0,
        Email = 1,
        Ignore = 2
    };

    struct alarmSignal
    {
        action Action;
        QString xValue;
        QString Chart;
        QString Alarm;
        QString Extra;
        alarmSignal();
        alarmSignal(action act, const QString &xvalue, const QString &chart, const QString &alarm,
                    const QString &extra)
                : Action(act), xValue(xvalue), Chart(chart), Alarm(alarm), Extra(extra)
        { }
    }
    ;

    struct chartTrack
    {
        QFile File;
        bool Persistent;

        chartTrack()
        {
            Persistent = true;
        }
        chartTrack(const QString &file, bool pers)
                : File(file)
        {
            Persistent = pers;
        }
        chartTrack(const chartTrack &fil)
                : File(fil.File.fileName())
        {
            Persistent = fil.Persistent;
        }
        const chartTrack &operator =(const chartTrack &fil)
        {
            if (fil.File.fileName() != File.fileName())
            {
                File.close();
                File.setFileName(fil.File.fileName());
            }
            Persistent = fil.Persistent;
            return *this;
        }
    };

    struct chartAlarm
    {
        enum operation
        {
            Any,
            All,
            Sum,
            Average,
            Max,
            Min
        };
        enum comparison
        {
            Equal,
            NotEqual,
            Less,
            Greater,
            LessEqual,
            GreaterEqual
        };

        operation Operation;
        comparison Comparison;
        action Action;
        double Value;
        std::list<int> Columns;
        bool Persistent;
        bool Signal;
        QString Extra;

        chartAlarm();
        chartAlarm(const QString &, bool persistent);
        chartAlarm(operation oper, comparison comp, action act,
                   double value, std::list<int> &cols, const QString &extra,
                   bool persistent);
        QString toString(void);

        bool checkValue(double value);

        void valueAdded(toChartHandler *handler, const QString &str,
                        std::list<double> &value, const QString &xValue);
    };

    QTimer Refresh;
    toListView *List;

    toChartReceiver *selectedChart(void);
public:
    toChartManager(QWidget *parent);
    ~toChartManager();

    friend class toChartSetup;
    friend class toChartAlarm;
    friend class toChartHandler;
public slots:
    void refresh(void);
    void setupChart(void);
    void openChart(void);
};

class toChartHandler : public QObject
{
    Q_OBJECT

    QTimer Timer;
    std::map<QString, std::list<toChartManager::chartAlarm> > Alarms;
    std::map<QString, toChartManager::chartTrack> Files;

    std::list<toChartReceiver *> Charts;
    std::list<toChartManager::alarmSignal> SignalAlarms;

    void loadSettings(void);
    void saveSettings(void);
public:
    toChartHandler();
    ~toChartHandler();

    void valueAdded(toLineChart *chart, const QString &chartName,
                    std::list<double> &vale, const QString &xValue);

    friend class toChartManager;
    friend struct toChartManager::alarmSignal;
    friend struct toChartManager::chartAlarm;
public slots:
    void alarm(void);
    void addChart(toLineChart *chart);
    void setupChart(toLineChart *chart);
    void removeChart(toLineChart *chart);
};


class toChartSetup : public QDialog, public Ui::toChartSetupUI
{
    Q_OBJECT;

    toLineChart *Chart;

public:
    toChartSetup(toLineChart *chart, QWidget* parent, const char* name, bool modal);

public slots:
    virtual void browseFile();

    virtual QString modifyAlarm(const QString &str, bool &persistent);
    virtual void addAlarm();
    virtual void modifyAlarm();
    virtual void removeAlarm();
};


class toChartAlarm : public QDialog, public Ui::toChartAlarmUI
{
    Q_OBJECT;

public:
    toChartAlarm(QWidget *parent = 0,
                 const char *name = 0,
                 bool modal = FALSE,
                 Qt::WFlags fl = 0);

public slots:
    virtual void changeValue(int val);
};
#endif
