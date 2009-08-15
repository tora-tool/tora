
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

#include "config.h"
#include "totool.h"
#include "toresultlong.h"

#include <list>
#include <QLabel>
#include <QMenu>
#include <QAction>

class QComboBox;
class QSplitter;
class QTabWidget;

class toConnection;
class toMain;
class toResultBar;
class toResultCombo;
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
    Q_OBJECT

    toResultTableView *Sessions;
    QTabWidget        *ResultTab;

    QWidget *CurrentTab;

    toResultCombo *Select;

    toSGAStatement    *CurrentStatement;
    toSGAStatement    *PreviousStatement;
    toResultStats     *SessionStatistics;
    toResultLong      *LongOps;
    toResultTableView *ConnectInfo;
    toResultTableView *LockedObjects;
    toResultLock      *PendingLocks;
    toResultTableView *AccessedObjects;
    toResultBar       *WaitBar;
    toResultBar       *IOBar;
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

    virtual bool canHandle(toConnection &conn);
public slots:
    void changeTab(int);
    void changeItem();
    void changeCursor();
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
    void selectAll(void);
    void selectNone(void);
    void filterChanged(const QString &text);
};

#ifdef TOEXTENDED_MYSQL
#include "tosessionmysql.h"
#endif

#endif
