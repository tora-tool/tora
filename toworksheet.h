/*****
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003-2005 Quest Software, Inc
 * Portions Copyright (C) 2005 Other Contributors
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 *      You may link this product with any GPL'd Qt library such as Qt/Free
 *
 * All trademarks belong to their respective owners.
 *
 *****/

#ifndef TOWORKSHEET_H
#define TOWORKSHEET_H

#include "config.h"

#include "toconnection.h"
#include "tosqlparse.h"
#include "totool.h"

#include <map>

#include <qdatetime.h>
#include <qframe.h>
#include <qmultilineedit.h>
#include <qtimer.h>
#include <qtoolbutton.h>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPopupMenu;
class QPushButton;
class QSplitter;
class QTabWidget;
class QToolButton;
class toHighlightedText;
class toListView;
class toResultBar;
class toResultCols;
class toResultCombo;
class toResultItem;
class toResultLong;
class toResultPlan;
class toResultStats;
class toResultView;
class toTool;
class toVisualize;
class toTabWidget;

class toWorksheet : public toToolWidget {
  Q_OBJECT

  bool Light;
  toHighlightedText *Editor;
  toTabWidget *ResultTab;
  toResultLong *Result;
  toResultPlan *Plan;
  QWidget *CurrentTab;
  QString QueryString;
  toResultItem *Resources;
  toResultStats *Statistics;
  toResultBar *WaitChart;
  toResultBar *IOChart;
  toListView *Logging;
  QSplitter *ResourceSplitter;
  toResultLong *LongOps;
  toVisualize *Visualize;
  QListViewItem *LastLogItem;
  QToolButton *PlanButton;
  QToolButton *StopButton;
  QToolButton *StatisticButton;
  QWidget *StatTab;
  bool TryStrip;
  toResultCols *Columns;
  QComboBox *Refresh;
  QLabel *Started;
  toResultCombo *Schema;

  int RefreshSeconds;
  QTimer RefreshTimer;

  QPopupMenu *SavedMenu;
  QToolButton *SavedButton;
  QCString SavedLast;

  QPopupMenu *InsertSavedMenu;
  QToolButton *InsertSavedButton;
  QCString InsertSavedLast;

  bool First;
  QTime Timer;
  QTimer Poll;

  QWidget *Current;
  std::map<int,QWidget *> History;
  int LastID;

  int LastLine;
  int LastOffset;

  enum execType {
    Normal,
    Direct,
    Parse,
    OnlyPlan
  };

  QPopupMenu *ToolMenu;

  QString duration(int,bool hundreds=true);
  void saveHistory(void);
  void viewResources(void);
  void query(const QString &str,execType type);
  bool checkSave(bool input);
  void saveDefaults(void);
  void setup(bool autoLoad);

  void execute(toSQLParse::tokenizer &tokens,int line,int pos,execType type);

  void insertStatement(const QString &);
public:
  toWorksheet(QWidget *parent,toConnection &connection,bool autoLoad=true);
  toWorksheet(QWidget *parent,const char *name,toConnection &connection);
  virtual ~toWorksheet();

  virtual bool close(bool del);

  toHighlightedText *editor(void)
  { return Editor; }
  bool describe(const QString &query);

  virtual bool canHandle(toConnection &)
  { return true; }

  virtual void exportData(std::map<QCString,QString> &data,const QCString &prefix);
  virtual void importData(std::map<QCString,QString> &data,const QCString &prefix);

  static toWorksheet *fileWorksheet(const QString &file);
  void setCaption(void);
signals:
  void executed(void);
public slots:
  void changeSchema(void);
  void connectionChanged(void);
  void refresh();
  void execute();
  void parseAll();
  void executeAll();
  void executeStep();
  void executeNewline();
  void describe();
  void eraseLogButton();
  void changeResult(QWidget *widget);
  void windowActivated(QWidget *w);
  void queryDone(void);
  void enableStatistic(bool);
  void explainPlan(void);
  void toggleStatistic(void)
  { StatisticButton->setOn(!StatisticButton->isOn()); }
  void showInsertSaved(void);
  void showSaved(void);
  void insertSaved(int index);
  void insertSaved(void);
  void executeSaved(int index);
  void executeSaved(void);
  void editSaved(void);
  void selectSaved(void);
  void executePreviousLog(void);
  void executeNextLog(void);
  void executeLog(void);
  void addLog(const QString &sql,const toConnection::exception &result,bool error);
  void changeRefresh(const QString &);
  void saveLast(void);
  void saveStatistics(void);
  void refreshSetup(void);
  void stop(void);
private slots:
  void poll(void);
  void displayMenu(QPopupMenu *menu);
};

#endif
