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

#ifndef __TOSECURITY_H
#define __TOSECURITY_H

#include <list>

#include <qlistview.h>
#include "totool.h"

class toConnection;
class toResultView;
class toSecurityPage;
class toSecurityQuota;
class QTabWidget;
class QToolButton;

class toSecuritySystem : public QListView {
  Q_OBJECT

  toConnection &Connection;
public:
  toSecuritySystem(toConnection &conn,QWidget *parent);
  void changeUser(const QString &);
  void eraseUser(bool all=true);
  void sql(const QString &user,std::list<QString> &sql);
public slots:
  virtual void changed(QListViewItem *item);
};

class toSecurityRoleGrant : public QListView {
  Q_OBJECT

  toConnection &Connection;
  QCheckListItem *findChild(QListViewItem *parent,const QString &name);
public:
  toSecurityRoleGrant(toConnection &conn,QWidget *parent);
  void changeUser(bool user,const QString &);
  void sql(const QString &user,std::list<QString> &sql);
  void eraseUser(bool user,bool all=true);
public slots:
  virtual void changed(QListViewItem *item);
};

class toSecurityObject : public QListView {
  Q_OBJECT

  toConnection &Connection;
public:
  toSecurityObject(toConnection &conn,QWidget *parent);
  void changeUser(const QString &);
  void sql(const QString &user,std::list<QString> &sql);
  void eraseUser(bool all=true);
public slots:
  virtual void changed(QListViewItem *item);
};

class toSecurity : public toToolWidget {
  Q_OBJECT

  struct privilege {
    QString Owner;
    QString Object;
    QString Access;
    QString Value;
    bool Admin;
    privilege(const QString &owner,const QString &object,const QString &access,
	      const QString &value,bool admin)
      : Owner(owner),Object(object),Access(access),Value(value),Admin(admin)
    { }
  };

  QString UserID;

  toResultView *UserList;
  toSecuritySystem *SystemGrant;
  toSecurityRoleGrant *RoleGrant;
  toSecurityObject *ObjectGrant;
  toSecurityPage *General;
  toSecurityQuota *Quota;
  QToolButton *DropButton;
  QToolButton *CopyButton;
  QTabWidget *Tabs;
  std::list<QString> sql(void);
public:
  toSecurity(QWidget *parent,toConnection &connection);
  virtual void changeUser(bool);

public slots:
  virtual void refresh(void);
  virtual void changeUser(QListViewItem *)
  { changeUser(true); }
  virtual void saveChanges(void);
  virtual void addUser(void);
  virtual void addRole(void);
  virtual void drop(void);
  virtual void copy(void);
  virtual void displaySQL(void);
};

#endif
