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

#ifndef __TOALERT_H
#define __TOALERT_H

#include <list>

#include "totool.h"
#include "tothread.h"

class toListView;
class QLineEdit;
class QComboBox;

class toAlert : public toToolWidget {
  Q_OBJECT

  class pollTask : public toTask {
    toAlert &Parent;
  public:
    pollTask(toAlert &parent)
      : Parent(parent)
    { }
    virtual void run(void);
  };

  toListView *Alerts;
  QComboBox *Registered;
  QLineEdit *Name;
  QLineEdit *Message;
  QTimer Timer;
  toConnection Connection;
  toConnection *SendConnection;

  toLock Lock;
  std::list<QString> AddNames;
  std::list<QString> DelNames;
  std::list<QString> Names;

  std::list<QString> NewAlerts;
  std::list<QString> NewMessages;

  std::list<QString> SendAlerts;
  std::list<QString> SendMessages;

  enum {
    Started,
    Quit,
    Done
  } State;
public:
  toAlert(QWidget *parent,toConnection &connection);
  virtual ~toAlert();

  friend class pollTask;
public slots:
  virtual void poll(void);
  virtual void send(void);
  virtual void memo(void);
  virtual void changeMessage(int,int,const QString &str);
  virtual void add(void);
  virtual void remove(void);
};

#endif
