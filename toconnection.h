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


#ifndef __TOCONNECTION_H
#define __TOCONNECTION_H

#include <qstring.h>
#include "otlv32.h"
#include <list>

class QWidget;

class toConnection {
  otl_connect *Connection;
  bool SqlNet;
  QString User;
  QString Password;
  QString Host;
  QString Version;
  list<QWidget *> Widgets;
  bool NeedCommit;
  otl_connect *newConnection(void);
  void setup(void);
  list<otl_connect *> FreeConnect;
public:
  toConnection(bool sqlNet,const char *iuser,const char *ipassword,const char *ihost);
  toConnection(const toConnection &conn);
  ~toConnection();
  bool closeWidgets(void);
  const QString &user() const
  { return User; }
  const QString &password() const
  { return Password; }
  const QString &host() const
  { return Host; }
  const QString &version() const
  { return Version; }
  bool needCommit(void) const
  { return NeedCommit; }
  void commit(void);
  void rollback(void);
  void setNeedCommit(bool needCommit=true)
  { NeedCommit=needCommit; }
  QString connectString(bool pw=false) const;
  otl_connect &connection();
  void addWidget(QWidget *widget)
  { Widgets.insert(Widgets.end(),widget); }
  void delWidget(QWidget *widget);

  otl_connect *longOperation(void);
  void longOperationFree(otl_connect *conn);
};

#endif
