/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

#include <qvbox.h>
#include <qmultilineedit.h>
#include <qframe.h>
#include <qtoolbutton.h>

#include "tomain.h"
#include "totool.h"
#include "tomarkedtext.h"

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
class toTool;

class toWorksheetPrefs : public QFrame, public toSettingTab
{ 
  Q_OBJECT

  QGroupBox* GroupBox1;
  QPushButton* FileChoose;
  QLineEdit* DefaultFile;
  QCheckBox* AutoCommit;
  QCheckBox* AutoSave;
  QCheckBox* CheckSave;
  QCheckBox* LogAtEnd;
  QCheckBox* LogMulti;
  QCheckBox* PLSQLParse;
  QCheckBox* Statistics;
  QLabel* TextLabel2;
  toTool *Tool;

public:
  toWorksheetPrefs(toTool *tool,QWidget* parent = 0,const char* name = 0);
  virtual void saveSetting(void);
public slots:
  void chooseFile(void);
};

class toWorksheet : public QVBox {
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
  bool TryStrip;
  toResultCols *Columns;

  QPopupMenu *ToolMenu;

  toConnection &Connection;

  void viewResources(void);
  void query(const QString &str,bool direct);
  bool checkSave(bool input);
public:
  toWorksheet(QWidget *parent,toConnection &connection,bool autoLoad=true);
  virtual ~toWorksheet();

  virtual bool close(bool del);

  void execute(bool all,bool step=false);

  toMarkedText *editor(void)
  { return Editor; }
  bool describe(const QString &query);

public slots:
  void refresh();
  void execute()
  { execute(false); }
  void executeAll()
  { execute(true); }
  void executeStep()
  { execute(false,true); }
  void commitButton()
  { Connection.commit(); }
  void rollbackButton()
  { Connection.rollback(); }
  void eraseLogButton();
  void changeResult(QWidget *widget);
  void windowActivated(QWidget *w);
  void queryDone(void);
  void enableStatistic(bool);
  void toggleStatistic(void)
  { StatisticButton->setOn(!StatisticButton->isOn()); }

  void addLog(const QString &sql,const QString &result);
};

#endif
