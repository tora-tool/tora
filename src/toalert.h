/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOALERT_H
#define TOALERT_H

#include "config.h"

#include <list>

#include <qtimer.h>
#include <QMenu>

#include "totool.h"
#include "tothread.h"

class QComboBox;
class QLineEdit;
class toListView;
class toConnection;

class toAlert : public toToolWidget
{
    Q_OBJECT;

class pollTask : public toTask
    {
        toAlert &Parent;

    public:
        pollTask(toAlert &parent) : Parent(parent)
        { }

        virtual void run(void);
    };

    toListView   *Alerts;
    QComboBox    *Registered;
    QLineEdit    *Name;
    QLineEdit    *Message;
    QTimer        Timer;
    toConnection &Connection;
    toConnection *SendConnection;

    QMenu        *ToolMenu;
    QAction *addAct;
    QAction *removeAct;
    QAction *memoEditAct;
    QAction *sendAct;

    toLock             Lock;
    toSemaphore        Semaphore;
    std::list<QString> AddNames;
    std::list<QString> DelNames;
    std::list<QString> Names;
    QString            Error;

    std::list<QString> NewAlerts;
    std::list<QString> NewMessages;

    std::list<QString> SendAlerts;
    std::list<QString> SendMessages;

    enum
    {
        Started,
        Quit,
        Done
    } State;

protected:
    void closeEvent(QCloseEvent *event);

public:
    toAlert(QWidget *parent, toConnection &connection);
    virtual ~toAlert();

    friend class pollTask;

public slots:
    virtual void poll(void);
    virtual void send(void);
    virtual void memo(void);
    virtual void changeMessage(int, int, const QString &str);
    virtual void add(void);
    virtual void remove(void);
    virtual void windowActivated(QMdiSubWindow *widget);
};

#endif
