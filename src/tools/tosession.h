
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries.
 *
 *      You may link this product with any GPL'd Qt library.
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
class toResultLock;
class toResultStats;
class toResultTableView;
class toSGAStatement;
class toSessionFilter;
class toTreeWidgetItem;
class toWaitEvents;

#define TO_SESSION_WAIT "toSession:SessionWait"
#define TO_SESSION_IO   "toSession:SessionIO"

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

    virtual bool canHandle(toConnection &conn);

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
