//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

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
class toHighlightedText;
class QGroupBox;
class QPushButton;
class QToolButton;
class QLineEdit;
class QCheckBox;
class QLabel;
class QComboBox;
class toTool;
class QSplitter;

class toWorksheet : public toToolWidget {
  Q_OBJECT

  bool Light;
  toHighlightedText *Editor;
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
  QComboBox *SavedSQL;
  QTime Timer;

  QPopupMenu *ToolMenu;

  void viewResources(void);
  void query(const QString &str,bool direct);
  bool checkSave(bool input);
  void saveDefaults(void);
  void setup(bool autoLoad);
public:
  toWorksheet(QWidget *parent,toConnection &connection,bool autoLoad=true);
  toWorksheet(QWidget *parent,const char *name,toConnection &connection);
  virtual ~toWorksheet();

  virtual bool close(bool del);

  void execute(bool all,bool step=false);

  toHighlightedText *editor(void)
  { return Editor; }
  bool describe(const QString &query);

  virtual bool canHandle(toConnection &conn)
  { return true; }

signals:
  void executed(void);
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
  void describe();
  void rollbackButton();
  void eraseLogButton();
  void changeResult(QWidget *widget);
  void windowActivated(QWidget *w);
  void queryDone(void);
  void enableStatistic(bool);
  void toggleStatistic(void)
  { StatisticButton->setOn(!StatisticButton->isOn()); }
  void executeSaved(void);
  void editSQL(void);
  void addLog(const QString &sql,const QString &result);
  void changeRefresh(const QString &);
};

#endif
