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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/



#ifndef __TOWORKSHEET_H
#define __TOWORKSHEET_H

#include <qvbox.h>
#include <qmultilineedit.h>
#include <qframe.h>

#include "tomain.h"
#include "totool.h"
#include "tomarkedtext.h"

class QTabWindow;
class toResultView;
class toResultItem;
class toResultStats;
class toResultPlan;
class toMarkedText;
class QGroupBox;
class QPushButton;
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
  toResultView *Result;
  toResultPlan *Plan;
  QString CurrentQuery;
  QWidget *CurrentTab;
  QString QueryString;
  toResultItem *Resources;
  toResultStats *Statistics;
  QListView *Logging;
  QListViewItem *LastLogItem;

  toConnection &Connection;

  void viewResources(void);
  void query(const QString &str);
  bool checkSave(bool input);
public:
  toWorksheet(toMain *parent,toConnection &connection);
  virtual ~toWorksheet();

  virtual bool close(bool del);

  void execute(bool all,bool step=false);
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
  void eraseLogButton()
  { Logging->clear(); LastLogItem=NULL; }
  void changeResult(QWidget *widget);
};

#endif
