//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef TOSESSION_H
#define TOSESSION_H

#include "totool.h"

class QComboBox;
class QListViewItem;
class QSplitter;
class QTabWidget;
class toConnection;
class toMain;
class toPopupMenu;
class toResultBar;
class toResultLock;
class toResultLong;
class toResultStats;
class toSGAStatement;

#define TO_SESSION_WAIT "toSession:SessionWait"
#define TO_SESSION_IO   "toSession:SessionIO"

class toSession : public toToolWidget {
  Q_OBJECT

  toResultLong *Sessions;
  QTabWidget *ResultTab;

  QWidget *CurrentTab;

  toSGAStatement *CurrentStatement;
  toSGAStatement *PreviousStatement;
  toResultStats *SessionStatistics;
  toResultLong *ConnectInfo;
  toResultLong *LockedObjects;
  toResultLock *PendingLocks;
  toResultBar *WaitBar;
  toResultBar *IOBar;
  QSplitter *OpenSplitter;
  QSplitter *StatisticSplitter;
  toSGAStatement *OpenStatement;
  toResultLong *OpenCursors;
  QString LastSession;
  QPopupMenu *ToolMenu;
  QComboBox *Refresh;

  QString Session;
  QString Serial;

  void updateSchemas(void);
  void enableStatistics(bool enable);

public:
  toSession(QWidget *parent,toConnection &connection);
public slots:
  void changeTab(QWidget *tab);
  void changeItem(QListViewItem *item);
  void changeCursor(QListViewItem *item);
  void changeRefresh(const QString &str);
  void refresh(void);
  void enableStatistics(void)
  { enableStatistics(true); }
  void disableStatistics(void)
  { enableStatistics(false); }
  void disconnectSession(void);
  void windowActivated(QWidget *widget);
  void done(void);
};

#endif
