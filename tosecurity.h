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


#ifndef __TOSECURITY_H
#define __TOSECURITY_H

#include <qvbox.h>
#include <list>

class toConnection;
class toResultView;
class toSecurityPage;
class toSecurityQuota;
class QTabWidget;

class toSecuritySystem : public QListView {
  Q_OBJECT

  toConnection &Connection;
  void eraseUser();
  void setOn(QListViewItem *,bool);
public:
  toSecuritySystem(toConnection &conn,QWidget *parent);
  void changeUser(const QString &);
  void sql(const QString &user,list<QString> &sql);
public slots:
  virtual void changed(QListViewItem *item);
};

class toSecurityObject : public QListView {
  Q_OBJECT

  toConnection &Connection;
  void eraseUser();
public:
  toSecurityObject(toConnection &conn,QWidget *parent);
  void changeUser(const QString &);
  void sql(const QString &user,list<QString> &sql);
public slots:
  virtual void changed(QListViewItem *item);
};

class toSecurity : public QVBox {
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

  toConnection &Connection;

  toResultView *UserList;
  toSecuritySystem *SystemGrant;
  toSecurityObject *ObjectGrant;
  toSecurityPage *General;
  toSecurityQuota *Quota;
  QTabWidget *Tabs;
  list<QString> sql(void);
public:
  toSecurity(QWidget *parent,toConnection &connection);
  virtual ~toSecurity();
public slots:
  virtual void refresh(void);
  virtual void changeUser(void);
  virtual void saveChanges(void);
};

#endif
