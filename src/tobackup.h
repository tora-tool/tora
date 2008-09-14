/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOBACKUP_H
#define TOBACKUP_H

#include "config.h"
#include "totool.h"

#include <QLabel>

class QLabel;
class QMenu;
class QTabWidget;
class toResultTableView;

class toBackup : public toToolWidget
{
    Q_OBJECT;

    QAction      *updateAct;
    QTabWidget   *Tabs;
    toResultTableView *LogSwitches;
    toResultTableView *LogHistory;
    QLabel       *LastLabel;
    toResultTableView *LastBackup;
    toResultTableView *CurrentBackup;
    QMenu        *ToolMenu;
    toTool       *tool_;

public:
    toBackup(toTool* tool, QWidget *parent, toConnection &connection);
    virtual ~toBackup();

public slots:
    void refresh(void);
    void windowActivated(QMdiSubWindow *widget);
};

#endif
