
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSESSION_H
#define TOSESSION_H


#include "core/totool.h"
#include "core/toresultlong.h"

#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QAction>

#include <list>

class QComboBox;
class QSplitter;
class QTabWidget;

class toConnection;
class toMain;
class toResultBar;
class toResultSchema;
class toResultItem;
class toResultLock;
class toResultStats;
class toResultTableView;
class toSGAStatement;
class toSessionFilter;
class toTreeWidgetItem;
class toWaitEvents;

#define TO_SESSION_WAIT "toSession:SessionWait"
#define TO_SESSION_IO   "toSession:SessionIO"
#define TO_SESSION_TXN   "toSession:SessionTXN"

class toSession : public toToolWidget
{
    Q_OBJECT;

    toResultTableView *Sessions;
    QTabWidget        *ResultTab;

    QWidget *CurrentTab;

    toResultSchema *Select;

    toSGAStatement    *CurrentStatement;
    toSGAStatement    *PreviousStatement;
    toResultStats     *SessionStatistics;
    toResultLong      *LongOps;
    toResultTableView *ConnectInfo;
    toResultTableView *LockedObjects;
    toResultLock      *PendingLocks;
    toResultTableView *AccessedObjects;
#ifdef TORA3_GRAPH    
    toResultBar       *WaitBar;
    toResultBar       *IOBar;
#endif    
    toWaitEvents      *Waits;
    QSplitter         *OpenSplitter;
    QSplitter         *StatisticSplitter;
    toSGAStatement    *OpenStatement;
    toResultTableView *OpenCursors;
    toResultItem      *Transaction;
    QString            LastSession;
    QMenu             *ToolMenu;
    QComboBox         *Refresh;
    QLabel            *Total;
    toSessionFilter   *SessionFilter;

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
    ~toSession();

    virtual bool canHandle(const toConnection &conn);

public slots:
    void slotChangeTab(int);
    void slotChangeItem();
    void slotChangeCursor();
    void slotChangeRefresh(const QString &str);
    void slotRefresh(void);
    void slotRefreshTabs(void);
    void slotEnableStatistics(void)
    {
        enableStatistics(true);
    }
    void slotDisableStatistics(void)
    {
        enableStatistics(false);
    }
    void slotCancelBackend(void);
    void slotDisconnectSession(void);
    virtual void slotWindowActivated(toToolWidget*);
    void slotDone(void);
    void slotSelectAll(void);
    void slotSelectNone(void);
    void slotFilterChanged(const QString &text);
};

#ifdef TOEXTENDED_MYSQL
#include "tosessionmysql.h"
#endif

#endif
