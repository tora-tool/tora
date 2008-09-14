/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOWAITEVENTS_H
#define TOWAITEVENTS_H

#include "config.h"
#include "tobackground.h"

#include <list>
#include <map>
#include <algorithm>

#include <QWidget>
#include <QString>

class toTreeWidget;
class toNoBlockQuery;
class toPieChart;
class toResultBar;
class QSplitter;


class toWaitEvents : public QWidget
{
    Q_OBJECT;

    QSplitter * splitter;
    toResultBar *Delta;
    toResultBar *DeltaTimes;
    toPieChart *AbsolutePie;
    toPieChart *DeltaPie;
    toTreeWidget *Types;
    toNoBlockQuery *Query;
    toBackground Poll;

    bool First;
    bool ShowTimes;
    QString Now;
    std::list<QString> Labels;
    time_t LastTime;
    std::list<double> LastCurrent;
    std::list<double> LastTimes;
    std::list<double> Current;
    std::list<double> CurrentTimes;
    std::list<double> Relative;
    std::list<double> RelativeTimes;
    std::list<bool> Enabled;

    int Session;

    std::map<QString, bool> HideMap;

    void setup(int session);
public:
    toWaitEvents(QWidget *parent, const char *name);
    toWaitEvents(int session, QWidget *parent, const char *name);
    ~toWaitEvents();

    void setSession(int session);

    virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
    virtual void importData(std::map<QString, QString> &data, const QString &prefix);
public slots:
    virtual void connectionChanged(void);
    virtual void changeSelection(void);
    virtual void poll(void);
    virtual void refresh(void);
    virtual void start(void);
    virtual void stop(void);
    virtual void changeType(int);
};

#endif
