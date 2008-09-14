/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOTEMPORARY_H
#define TOTEMPORARY_H

#include "config.h"
#include "totool.h"
#include <list>

#include <QMenu>
#include <totreewidget.h>

class toResultView;
class toTask;
class toSGAStatement;

class toTemporary : public toToolWidget
{
    Q_OBJECT;

    toResultView   *Objects;
    toSGAStatement *Statement;
    QMenu          *ToolMenu;
    QAction        *refreshAct;

public:
    toTemporary(QWidget *parent, toConnection &connection);

public slots:
    virtual void refresh(void);
    virtual void windowActivated(QMdiSubWindow *widget);
    virtual void changeItem(toTreeWidgetItem *item);
};

#endif
