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

TO_NAMESPACE;

#include <qstring.h>

#include "otlv32.h"
#include "totool.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"

void toConnection::connect(void)
{
  QString oldSid;
  if (!SqlNet) {
    oldSid=getenv("ORACLE_SID");
    setenv("ORACLE_SID",Host,1);
  }
  try {
    Connection=new otl_connect((const char *)connectString(true),0);
  } catch (...) {
    if (!SqlNet) {
      if (oldSid.isNull())
	unsetenv("ORACLE_SID");
      else
	setenv("ORACLE_SID",oldSid,1);
    }
    throw;
  }
  if (!SqlNet) {
    if (oldSid.isNull())
      unsetenv("ORACLE_SID");
    else {
      setenv("ORACLE_SID",oldSid,1);
    }
  }

  QString str="ALTER SESSION SET NLS_DATE_FORMAT = '";
  str+=toTool::globalConfig(CONF_DATE_FORMAT,DEFAULT_DATE_FORMAT);
  str+="'";
  otl_stream date(1,str,*Connection);

  try {
    otl_stream version(1,
		       "SELECT banner FROM v$version",
		       *Connection);
    QRegExp verre("[0-9]\\.[0-9\\.]+[0-9]");
    QRegExp orare("^oracle",false);
    while(!version.eof()) {
      char buffer[1024];
      version>>buffer;
      if (orare.match(buffer)>=0) {
	int pos;
	int len;
	pos=verre.match(buffer,0,&len);
	if (pos>=0) {
	  Version=buffer+pos;
	  Version.truncate(len);
	  break;
	}
      }
    }
  } catch (...) {
    // Ignore any errors here
  }
  NeedCommit=false;
}

toConnection::toConnection(bool sqlNet,const char *iuser,const char *ipassword,const char *ihost)
: SqlNet(sqlNet),User(iuser),Password(ipassword),Host(ihost)
{
  connect();
}

toConnection::toConnection(const toConnection &conn)
: SqlNet(conn.SqlNet),User(conn.User),Password(conn.Password),Host(conn.Host)
{
  connect();
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
  if (!pw||SqlNet) {
    ret.append("@");
    ret.append(Host);
  }
  if (!pw && !Version.isEmpty()) {
    ret+=" [";
    ret+=Version;
    ret+="]";
  }
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
