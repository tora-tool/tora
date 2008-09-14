/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOPROFILER_H
#define TOPROFILER_H

#include "config.h"
#include "totool.h"

class QComboBox;
class QLineEdit;
class QSpinBox;
class QSplitter;
class QTabWidget;
class QToolButton;
class toProfilerSource;
class toProfilerUnits;
class toResultItem;
class toResultLong;
class toWorksheetWidget;

class toProfiler : public toToolWidget
{
    Q_OBJECT

    int CurrentRun;
    int LastUnit;

    QToolButton *Background;
    QSpinBox *Repeat;
    QLineEdit *Comment;
    QTabWidget *Tabs;
    QSplitter *Result;

    QComboBox *Run;
    toResultItem *Info;
    toProfilerUnits *Units;
    toProfilerSource *Lines;
    toWorksheetWidget *Script;
public:
    toProfiler(QWidget *parent, toConnection &connection);
public slots:
    void refresh(void);
    void changeRun(void);
    void execute(void);
    void changeObject(void);
    void calcTotals(void);
    void noTables(void)
    {
        close();
    }
};

#endif
