//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef __TOSGATRACE_H
#define __TOSGATRACE_H

#include "totool.h"

class QTabWidget;
class toConnection;
class toSGAStatement;
class QListViewItem;
class toResultView;
class toMain;
class toResultStats;
class toResultLock;
class toResultBar;

#define TO_SESSION_WAIT "toSession:SessionIO"
#define TO_SESSION_IO   "toSession:SessionWait"

class toSession : public toToolWidget {
  Q_OBJECT

  toResultView *Sessions;
  QTabWidget *ResultTab;

  QWidget *CurrentTab;
  QListViewItem *CurrentItem;

  toSGAStatement *CurrentStatement;
  toSGAStatement *PreviousStatement;
  toResultStats *SessionStatistics;
  toResultView *ConnectInfo;
  toResultView *LockedObjects;
  toResultLock *PendingLocks;
  toResultBar *WaitBar;
  toResultBar *IOBar;
  QSplitter *OpenSplitter;
  QSplitter *StatisticSplitter;
  toSGAStatement *OpenStatement;
  toResultView *OpenCursors;
  QString LastSession;

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
};

#endif
