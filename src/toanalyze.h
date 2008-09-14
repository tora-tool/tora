/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOANALYZE_H
#define TOANALYZE_H

#include "config.h"
#include "tobackground.h"
#include "tonoblockquery.h"
#include "totool.h"

#include <list>

#include <QAction>
#include <QLabel>

class QComboBox;
class QMenu;
class QSpinBox;
class QTabWidget;
class QTimer;
class QLabel;
class toResultCombo;
class toResultPlan;
class toResultTableView;
class toWorksheetStatistic;

class toAnalyze : public toToolWidget
{
    Q_OBJECT;

    QTabWidget           *Tabs;
    toResultTableView    *Statistics;
    QComboBox            *Analyzed;
    toResultCombo        *Schema;
    QComboBox            *Type;
    QComboBox            *Operation;
    QComboBox            *For;
    QSpinBox             *Sample;
    QSpinBox             *Parallel;
    QLabel               *Current;
    QToolButton          *Stop;
    toBackground          Poll;
    toResultTableView    *Plans;
    toResultPlan         *CurrentPlan;
    toWorksheetStatistic *Worksheet;

    std::list<toNoBlockQuery *> Running;
    std::list<QString> Pending;

    std::list<QString> getSQL(void);

public:
    toAnalyze(QWidget *parent, toConnection &connection);

    toWorksheetStatistic *worksheet(void);

    static void createTool(void);

public slots:
    virtual void displaySQL(void);
    virtual void changeOperation(int);
    virtual void execute(void);
    virtual void poll(void);
    virtual void stop(void);
    virtual void refresh(void);
    virtual void selectPlan(void);
    virtual void fillOwner(void);
    virtual void displayMenu(QMenu *);
};

#endif
