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

class QTabWidget;
class QListView;
class QListViewItem;
class QToolButton;
class toHighlightedText;

class toDebug : public QVBox {
  Q_OBJECT

  toConnection &Connection;

  QString CurrentSchema;
  QString CurrentObject;
  QString CurrentType;

  QComboBox *Schema;
  QToolButton *ShowButton;
  QToolButton *StopButton;
  QToolButton *StepOverButton;
  QToolButton *StepIntoButton;
  QToolButton *ReturnButton;

  QTabWidget *DebugTabs;

  QListView *Objects;
  QListView *Contents;
  toHighlightedText *HeadEditor;
  toHighlightedText *BodyEditor;

  toLock Lock;
  toSemaphore TargetSemaphore;
  toSemaphore ChildSemaphore;
  toThread *TargetThread;
  QString TargetSQL;
  bool RunningTarget;
  QString TargetID;
  QString TargetSession;

  class targetTask : public toTask {
    toDebug &Parent;
  public:
    targetTask(toDebug &parent)
      : Parent(parent)
    { }
    virtual void run(void);
  };

  bool checkCompile(void);
  bool compile(const QString &str);
  void updateCurrent(void);
  void startTarget(void);

  void updateContent(bool body);
  void reorderContent(QListViewItem *item,int,int);
public:
  toDebug(toMain *parent,toConnection &connection);

  virtual ~toDebug();

  friend targetTask;

public slots:
  void stop(void);
  void compile(void);
  void refresh(void);
  void changeSchema(int);
  void changePackage(QListViewItem *);
  void changeView(bool);
  void prevError(void);
  void nextError(void);
  void changeContent(QListViewItem *);
  void scanSource(void);
  void reorderContent(int,int);
};

#endif
