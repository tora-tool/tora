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


#include <qstring.h>
#include "otlv32.h"
#include "totool.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"

toConnection::toConnection(const char *iuser,const char *ipassword,const char *ihost)
: User(iuser),Password(ipassword),Host(ihost)
{
  Connection=new otl_connect((const char *)connectString(true),0);

  QString str="ALTER SESSION SET NLS_DATE_FORMAT = '";
  str.append(toTool::globalConfig(CONF_DATE_FORMAT,DEFAULT_DATE_FORMAT));
  str.append("'");
  otl_stream date(1,(const char *)str,*Connection);
  NeedCommit=false;
}

bool toConnection::closeWidgets(void)
{
  for (list<QWidget *>::iterator i=Widgets.begin();i!=Widgets.end();i=Widgets.begin()) {
    if (!(*i)->close(true))
      return false;
    list<QWidget *>::iterator nextI=Widgets.begin();
    if (i==nextI)
      throw QString("All tool widgets need to have autodelete flag set");
  }
  return true;
}

toConnection::~toConnection()
{
  for (list<QWidget *>::iterator i=Widgets.begin();i!=Widgets.end();i=Widgets.begin()) {
    delete (*i);
  }
  delete Connection;
}

QString toConnection::connectString(bool pw=false) const
{
  QString ret(User);
  if (pw) {
    ret.append("/");
    ret.append(Password);
  }
  ret.append("@");
  ret.append(Host);
  return ret;
}

otl_connect &toConnection::connection()
{
  if (!Connection)
    throw QString("Tried to used unassigned connection");
  return *Connection;
}

void toConnection::delWidget(QWidget *widget)
{
  for (list<QWidget *>::iterator i=Widgets.begin();i!=Widgets.end();i++) {
    if ((*i)==widget) {
      Widgets.erase(i);
      break;
    }
  }
}
