/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOOUTPUT_H
#define TOOUTPUT_H

#include "config.h"
#include "totool.h"
#include "tomarkedtext.h"

#include <QCloseEvent>
#include <QAction>
#include <QMenu>
#include <QToolBar>

class QComboBox;
class toConnection;
class toResultView;

class toOutput : public toToolWidget
{
    Q_OBJECT;

    toMarkedText *Output;
    QMenu        *ToolMenu;
    QComboBox    *Refresh;
    QToolBar     *Toolbar;
    QAction      *refreshAct;
    QAction      *enableAct;
    QAction      *clearAct;

    void poll(void);

protected:
    virtual void closeEvent(QCloseEvent *);

public:
    toOutput(QWidget *parent, toConnection &connection, bool enabled = true);
    virtual ~toOutput();
    bool enabled(void);

    void insertLine(const QString &str)
    {
        Output->append(str);
        Output->setCursorPosition(Output->lines(), 0);
    }

    toMarkedText *output(void)
    {
        return Output;
    }

    QToolBar *toolBar(void)
    {
        return Toolbar;
    }

public slots:
    void clear(void);
    virtual void refresh(void);
    virtual void disable(bool);
    void changeRefresh(const QString &str);
    void windowActivated(QMdiSubWindow *widget);
    void toggleMenu();
};

class toLogOutput : public toOutput
{
    Q_OBJECT;

    QComboBox *Type;
    toResultView *Log;

public:
    toLogOutput(QWidget *parent, toConnection &connection);

public slots:
    virtual void refresh(void);
    void changeType(void);
};

#endif
