/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSESSION_H
#define TOSESSION_H

#include "config.h"
#include "totool.h"
#include "toresultlong.h"

#include <list>
#include <QLabel>
#include <QMenu>
#include <QAction>

class QComboBox;
class toTreeWidgetItem;
class QSplitter;
class QTabWidget;
class toConnection;
class toMain;
class toResultBar;
class toResultCombo;
class toResultLock;
class toResultStats;
class toSGAStatement;
class toWaitEvents;
class toResultTableView;

#define TO_SESSION_WAIT "toSession:SessionWait"
#define TO_SESSION_IO   "toSession:SessionIO"

class toSessionList : public toResultLong
{
public:
class sessionFilter : public toResultFilter
    {
        struct sessionID
        {
            int Session;
            int Serial;
            sessionID(int session, int serial)
            {
                Session = session;
                Serial = serial;
            }
            bool operator == (const sessionID &ses) const
            {
                return Session == ses.Session && Serial == ses.Serial;
            }
        };
        std::list<sessionID> Serials;
        bool Show;
        QString OnlyDatabase;
    public:

        sessionFilter()
        {
            Show = true;
        }
        sessionFilter(const std::list<sessionID> &serials, bool show)
        {
            Serials = serials;
            Show = show;
        }
        virtual bool check(const toTreeWidgetItem *item);
        virtual toResultFilter *clone(void)
        {
            return new sessionFilter(Serials, Show);
        }
        void setShow(bool show)
        {
            Show = show;
        }
        bool show()
        {
            return Show;
        }
        void onlyDatabase(const QString &db)
        {
            OnlyDatabase = db;
        }
        void updateList(toResultLong *lst);
    };
    toSessionList(QWidget *parent)
            : toResultLong(false, false, toQuery::Background, parent)
    {
        setFilter(new sessionFilter);
    }
    virtual toTreeWidgetItem *createItem(toTreeWidgetItem *last, const QString &str);
    void updateFilter(void);
    virtual void refresh(void)
    {
        updateFilter();
        toResultLong::refresh();
    }
};

class toSession : public toToolWidget
{
    Q_OBJECT

    toResultLong *Sessions;
    QTabWidget *ResultTab;

    QWidget *CurrentTab;

    toResultCombo *Select;

    toSGAStatement    *CurrentStatement;
    toSGAStatement    *PreviousStatement;
    toResultStats     *SessionStatistics;
    toResultLong      *LongOps;
    toResultLong      *ConnectInfo;
    toResultTableView *LockedObjects;
    toResultLock      *PendingLocks;
    toResultLong      *AccessedObjects;
    toResultBar       *WaitBar;
    toResultBar       *IOBar;
    toWaitEvents      *Waits;
    QSplitter         *OpenSplitter;
    QSplitter         *StatisticSplitter;
    toSGAStatement    *OpenStatement;
    toResultLong      *OpenCursors;
    QString            LastSession;
    QMenu             *ToolMenu;
    QComboBox         *Refresh;
    QLabel            *Total;

    QAction *refreshAct;
    QAction *enableTimedAct;
    QAction *disableTimedAct;
    QAction *disconnectAct;

    QString Session;
    QString Serial;

    void updateSchemas(void);
    void enableStatistics(bool enable);

public:
    toSession(QWidget *parent, toConnection &connection);

    virtual bool canHandle(toConnection &conn);
public slots:
    void changeTab(int);
    void changeItem(toTreeWidgetItem *item);
    void changeCursor(toTreeWidgetItem *item);
    void changeRefresh(const QString &str);
    void refresh(void);
    void refreshTabs(void);
    void enableStatistics(void)
    {
        enableStatistics(true);
    }
    void disableStatistics(void)
    {
        enableStatistics(false);
    }
    void cancelBackend(void);
    void disconnectSession(void);
    void windowActivated(QMdiSubWindow *widget);
    void done(void);
    void excludeSelection(bool);
    void selectAll(void);
    void selectNone(void);
};

#ifdef TOEXTENDED_MYSQL
#include "tosessionmysql.h"
#endif

#endif
