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

#ifndef __TODEBUG_H
#define __TODEBUG_H

#include <qtimer.h>

#include "totool.h"
#include "tothread.h"
#include "tohighlightedtext.h"
#include "todebugwatch.h"
#include "toconnection.h"

#define TO_SUCCESS		0
#define TO_ERROR_ILLEGAL_LINE	12
#define TO_ERROR_BAD_HANDLE	16
#define TO_ERROR_UNIMPLEMENTED	17
#define TO_ERROR_TIMEOUT	31
#define TO_ERROR_NULLVALUE	32
#define TO_ERROR_NULLCOLLECTION	40
#define TO_ERROR_INDEX_TABLE	18

#define TO_REASON_KNL_EXIT	25
#define TO_REASON_NO_SESSION	-1
#define TO_REASON_TIMEOUT	17
#define TO_REASON_EXIT		15
#define TO_REASON_WHATEVER	0

#define TO_NAME_CURSOR		0	
#define TO_NAME_TOPLEVEL	1
#define TO_NAME_BODY		2
#define TO_NAME_TRIGGER		3
#define TO_NAME_NONE		127

#define TO_BREAK_EXCEPTION	2
#define TO_BREAK_ANY_CALL	12
#define TO_BREAK_RETURN		16
#define TO_BREAK_NEXT_LINE	32
#define TO_BREAK_ANY_RETURN	512
#define TO_BREAK_HANDLER	2048
#define TO_ABORT_EXECUTION	8192

class QTabWidget;
class QListView;
class QListViewItem;
class QToolButton;
class toOutput;
class toDebugText;
class QComboBox;
class toConnection;
class QPopupMenu;
class toMarkedText;

class toDebug : public toToolWidget {
  Q_OBJECT

  struct debugParam {
    debugParam()
    { In=false; Out=false; }
    QString Name;
    QString Type;
    bool In;
    bool Out;
  };

  std::list<debugParam> CurrentParams;

  // Toolbar
  QComboBox *Schema;
  QToolButton *ShowButton;
  QToolButton *StopButton;
  QToolButton *StepOverButton;
  QToolButton *StepIntoButton;
  QToolButton *ReturnButton;
  QToolButton *DebugButton;
  QToolButton *DelWatchButton;
  QToolButton *ChangeWatchButton;

  // Extra menu
  QPopupMenu *ToolMenu;

  // Content pane
  QListView *Objects;
  QListView *Contents;

  // Debug pane
  QTabWidget *DebugTabs;
  QListView *Breakpoints;
  QListView *StackTrace;
  QListView *Watch;
  QListView *Parameters;
  toOutput *Output;
  toMarkedText *RuntimeLog;

  // Editors
  toDebugText *HeadEditor;
  toDebugText *BodyEditor;

  // Must hold lock before reading or writing to these
  toLock Lock;
  toSemaphore TargetSemaphore;
  toSemaphore ChildSemaphore;
  toSemaphore StartedSemaphore;
  toThread *TargetThread;
  QString TargetSQL;
  QString TargetLog;
  toQList InputData;
  toQList OutputData;
  int ColumnSize;
  bool RunningTarget;
  bool DebuggerStarted;
  // Can be read after thread startup
  QString TargetID;
  QString TargetSession;
  // End of lock stuff
  toTimer StartTimer;

  class targetTask : public toTask {
    toDebug &Parent;
  public:
    targetTask(toDebug &parent)
      : Parent(parent)
    { }
    virtual void run(void);
  };

  virtual bool close(bool del);

  int continueExecution(int stopon);

  bool checkCompile(void);
  void updateCurrent(void);
  QString currentName(void);
  int sync(void);
  bool hasMembers(const QString &str);
  void readLog(void);
  void updateState(int reason);
  void updateContent(bool body);
  void reorderContent(QListViewItem *item,int,int);
  bool viewSource(const QString &schema,const QString &name,const QString &type,
		  int line,bool current=false);

  void setDeferedBreakpoints(void);
  bool checkStop(void);
private slots:
  void startTarget(void);
public:
  toDebug(QWidget *parent,toConnection &connection);

  virtual ~toDebug();

  friend class targetTask;

  bool isRunning(void);

  QListViewItem *contents(void);
  toDebugText *currentEditor(void);
  toDebugText *headEditor(void)
  { return HeadEditor; }
  toDebugText *bodyEditor(void)
  { return BodyEditor; }
  QString currentSchema(void);

  void executeInTarget(const QString &,toQList &params);

  virtual void exportData(std::map<QString,QString> &data,const QString &prefix);
  virtual void importData(std::map<QString,QString> &data,const QString &prefix);
public slots:
  void stop(void);
  void compile(void);
  void refresh(void);
  void changeSchema(int);
  void changePackage(QListViewItem *);
  void changeView(bool);
  void showDebug(bool);
  void prevError(void);
  void nextError(void);
  void changeContent(QListViewItem *);
  void showSource(QListViewItem *);
  void scanSource(void);
  void reorderContent(int,int);
  void newSheet(void);
  void execute(void);
  void toggleBreak(void);
  void toggleEnable(void);
  void addWatch(void);
  void stepInto(void)
  { continueExecution(TO_BREAK_ANY_CALL); }
  void stepOver(void)
  { continueExecution(TO_BREAK_NEXT_LINE); }
  void returnFrom(void)
  { continueExecution(TO_BREAK_ANY_RETURN); }
  void windowActivated(QWidget *w);
  void toggleHead(void);
  void toggleDebug(void);
  void selectedWatch(void);
  void deleteWatch(void);
  void clearLog(void);
  void changeWatch(void);
  void changeWatch(QListViewItem *item);
};

class toDebugWatch : public toDebugWatchUI {
  Q_OBJECT

  QListViewItem *HeadItems;
  QListViewItem *BodyItems;
  toDebug *Debugger;
  QString Object;
  QString Default;
  QListViewItem *findMisc(const QString &str,QListViewItem *,toDebugText *);
public:
  toDebugWatch(toDebug *parent);

  QListViewItem *createWatch(QListView *watches);
public slots:
  void changeScope(int num);
};

#endif
