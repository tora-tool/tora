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


#ifndef __TODEBUG_H
#define __TODEBUG_H

#include <qvbox.h>
#include "tothread.h"
#include "tohighlightedtext.h"

#define TO_SUCCESS		0
#define TO_ERROR_ILLEGAL_LINE	12
#define TO_ERROR_BAD_HANDLE	16
#define TO_ERROR_TIMEOUT	31

#define TO_REASON_KNL_EXIT	25
#define TO_REASON_NO_SESSION	-1
#define TO_REASON_TIMEOUT	17
#define TO_REASON_EXIT		15

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

class toDebug : public QVBox {
  Q_OBJECT

  struct debugParam {
    debugParam()
    { In=false; Out=false; }
    QString Name;
    QString Type;
    bool In;
    bool Out;
  };

  toConnection &Connection;

  list<debugParam> CurrentParams;

  // Toolbar
  QComboBox *Schema;
  QToolButton *ShowButton;
  QToolButton *StopButton;
  QToolButton *StepOverButton;
  QToolButton *StepIntoButton;
  QToolButton *ReturnButton;
  QToolButton *DebugButton;

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

  toDebugText *HeadEditor;
  toDebugText *BodyEditor;

  // Must hold lock before reading or writing to these
  toLock Lock;
  toSemaphore TargetSemaphore;
  toSemaphore ChildSemaphore;
  toThread *TargetThread;
  QString TargetSQL;
  list<QString> InputData;
  list<QString> OutputData;
  int ColumnSize;
  bool RunningTarget;
  // Can be read after thread startup
  QString TargetID;
  QString TargetSession;
  // End of lock stuff

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
  void startTarget(void);
  toDebugText *currentEditor(void);
  QString currentName(void);
  int sync(void);
  bool hasMembers(const QString &str);
  void updateState(int reason);
  void updateContent(bool body);
  void reorderContent(QListViewItem *item,int,int);
  bool viewSource(const QString &schema,const QString &name,const QString &type,
		  int line,bool current=false);

  void setDeferedBreakpoints(void);
  bool checkStop(void);
public:
  toDebug(QWidget *parent,toConnection &connection);

  virtual ~toDebug();

  friend targetTask;

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
  void stepInto(void)
  { continueExecution(TO_BREAK_ANY_CALL); }
  void stepOver(void)
  { continueExecution(TO_BREAK_NEXT_LINE); }
  void returnFrom(void)
  { continueExecution(TO_BREAK_ANY_RETURN); }
};

#endif
