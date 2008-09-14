/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOWORKSHEETSTATISTIC_H
#define TOWORKSHEETSTATISTIC_H

#include "config.h"

#include <list>
#include <map>

#include <qsplitter.h>

#include <QMenu>
#include <QString>
#include <QLabel>
#include <QToolButton>

class QLabel;
class QSplitter;
class toAnalyze;
class toBarChart;
class toListView;
class toWorksheetStatistic;
class QAction;


class toWorksheetStatistic : public QWidget
{
    Q_OBJECT;

    struct data
    {
        QAction    *Action;
        QWidget    *Top;
        QLabel     *Label;
        QSplitter  *Charts;
        toListView *Statistics;
        toBarChart *Wait;
        toBarChart *IO;
        toListView *Plan;
    };

    std::list<data> Open;

    QMenu *SaveMenu;
    QMenu *RemoveMenu;

    static toAnalyze *Widget;
    toAnalyze        *Tool;
    QSplitter        *Splitter;
    QWidget          *Dummy;

    QToolButton *ShowPlans;
    QToolButton *ShowCharts;

public:
    toWorksheetStatistic(QWidget *parent);
    ~toWorksheetStatistic();

    static void saveStatistics(std::map<QString, QString> &stats);

    void addStatistics(std::map<QString, QString> &stats);

public slots:

    virtual void showPlans(bool);
    virtual void showCharts(bool);

    virtual void save(QAction *);
    virtual void remove(QAction *);
    virtual void load(void);
    virtual void displayMenu(void);
};

#endif
