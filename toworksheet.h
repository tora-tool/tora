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

#ifndef __TOWORKSHEET_H
#define __TOWORKSHEET_H

#include <qmultilineedit.h>
#include <qframe.h>
#include <qtoolbutton.h>
#include <qdatetime.h>

#include "totool.h"

class QTabWidget;
class toResultLong;
class toResultView;
class toResultItem;
class toResultStats;
class toResultPlan;
class toResultCols;
class toMarkedText;
class QGroupBox;
class QPushButton;
class QToolButton;
class QLineEdit;
class QCheckBox;
class QLabel;
class QComboBox;
class toTool;

class toWorksheet : public toToolWidget {
  Q_OBJECT

  toMarkedText *Editor;
  QTabWidget *ResultTab;
  toResultLong *Result;
  toResultPlan *Plan;
  QString CurrentQuery;
  QWidget *CurrentTab;
  QString QueryString;
  toResultItem *Resources;
  toResultStats *Statistics;
  toResultView *Logging;
  QListViewItem *LastLogItem;
  QToolButton *StopButton;
  QToolButton *StatisticButton;
  QSplitter *StatSplitter;
  bool TryStrip;
  toResultCols *Columns;
  QComboBox *Refresh;
  QTime Timer;

  QPopupMenu *ToolMenu;

  void viewResources(void);
  void query(const QString &str,bool direct);
  bool checkSave(bool input);
  void saveDefaults(void);
public:
  toWorksheet(QWidget *parent,toConnection &connection,bool autoLoad=true);
  virtual ~toWorksheet();

  virtual bool close(bool del);

  void execute(bool all,bool step=false);

  toMarkedText *editor(void)
  { return Editor; }
  bool describe(const QString &query);

  virtual bool canHandle(toConnection &conn)
  { return true; }

public slots:
  void connectionChanged(void);
  void refresh();
  void execute()
  { execute(false); }
  void executeAll()
  { execute(true); }
  void executeStep()
  { execute(false,true); }
  void executeNewline();
  void commitButton();
  void rollbackButton();
  void eraseLogButton();
  void changeResult(QWidget *widget);
  void windowActivated(QWidget *w);
  void queryDone(void);
  void enableStatistic(bool);
  void toggleStatistic(void)
  { StatisticButton->setOn(!StatisticButton->isOn()); }

  void addLog(const QString &sql,const QString &result);
  void changeRefresh(const QString &);
};

#endif
