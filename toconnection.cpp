//***************************************************************************
/*
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

TO_NAMESPACE;

#include <qstring.h>
#include <qregexp.h>

#include "otlv32.h"
#include "totool.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"

otl_connect *toConnection::newConnection(void)
{
  QString oldSid;
  if (!SqlNet) {
    oldSid=getenv("ORACLE_SID");
    toSetEnv("ORACLE_SID",Host.latin1());
  }
  otl_connect *conn;
  try {
    int oper=0;
    int dba=0;
    if (Mode=="SYS_OPER")
      oper=1;
    else if (Mode=="SYS_DBA")
      dba=1;
    conn=new otl_connect((const char *)connectString(true),0,oper,dba);
  } catch (...) {
    if (!SqlNet) {
      if (oldSid.isNull())
	toUnSetEnv("ORACLE_SID");
      else
	toSetEnv("ORACLE_SID",oldSid.latin1());
    }
    throw;
  }
  if (!SqlNet) {
    if (oldSid.isNull())
      toUnSetEnv("ORACLE_SID");
    else {
      toSetEnv("ORACLE_SID",oldSid.latin1());
    }
  }

  QString str="ALTER SESSION SET NLS_DATE_FORMAT = '";
  str+=toTool::globalConfig(CONF_DATE_FORMAT,DEFAULT_DATE_FORMAT);
  str+="'";
  otl_stream date(1,str.utf8(),*conn);
  for (list<QString>::iterator i=InitStrings.begin();i!=InitStrings.end();i++) {
    try {
      otl_cursor::direct_exec(*conn,(*i).utf8());
    } catch (const otl_exception &exc) {
      toStatusMessage(QString::fromUtf8((const char *)exc.msg));
    }
  }
  return conn;
}

void toConnection::setup(void)
{
  Connection=newConnection();
  NeedCommit=false;

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
	  Version=QString::fromUtf8(buffer+pos);
	  Version.truncate(len);
	  break;
	}
      }
    }
  } catch (...) {
    // Ignore any errors here
  }
}

toConnection::toConnection(bool sqlNet,
			   const QString &iuser,const QString &ipassword,const QString &ihost,
			   const QString &imode)
  : SqlNet(sqlNet),User(iuser),Password(ipassword),Host(ihost),Mode(imode)
{
  if (Mode=="Normal")
    Mode=QString::null;
  else if (Mode!="SYS_DBA"&&Mode!="SYS_OPER")
    throw QString("Malformed mode string");
  setup();
}

toConnection::toConnection(const toConnection &conn)
  : SqlNet(conn.SqlNet),User(conn.User),Password(conn.Password),Host(conn.Host),Mode(conn.Mode)
{
  setup();
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
  for (list<otl_connect *>::iterator j=FreeConnect.begin();j!=FreeConnect.end();j++)
    delete (*j);
}

QString toConnection::connectString(bool pw) const
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
  if (!pw && !Mode.isEmpty()) {
    ret+=" ";
    ret+=Mode;
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

otl_connect *toConnection::longOperation(void)
{
  if (toTool::globalConfig(CONF_LONG_SESSION,"").isEmpty())
    return Connection;
  if (FreeConnect.begin()!=FreeConnect.end()) {
    otl_connect *ret=*(FreeConnect.begin());
    FreeConnect.erase(FreeConnect.begin());
    return ret;
  } else
    return newConnection();
}

void toConnection::longOperationFree(otl_connect *conn)
{
  if (!conn)
    throw QString("Tried to give back NULL long connection");
  if (conn!=Connection)
    FreeConnect.insert(FreeConnect.end(),conn);
}

void toConnection::commit(void)
{
  NeedCommit=false;
  connection().commit();
  otl_connect *tmp=NULL;
  for (list<otl_connect *>::iterator i=FreeConnect.begin();i!=FreeConnect.end();i++) {
    (*i)->commit();
    if (tmp)
      delete tmp;
    tmp=(*i);
  }
  FreeConnect.clear();
  if (tmp)
    FreeConnect.insert(FreeConnect.end(),tmp);
}

void toConnection::rollback(void)
{
  NeedCommit=false;
  connection().rollback();
  otl_connect *tmp=NULL;
  for (list<otl_connect *>::iterator i=FreeConnect.begin();i!=FreeConnect.end();i++) {
    (*i)->rollback();
    if (tmp)
      delete tmp;
    tmp=(*i);
  }
  FreeConnect.clear();
  if (tmp)
    FreeConnect.insert(FreeConnect.end(),tmp);
}

void toConnection::addInit(const QString &sql)
{
  delInit(sql);
  InitStrings.insert(InitStrings.end(),sql);
}

void toConnection::delInit(const QString &sql)
{
  list<QString>::iterator i=InitStrings.begin();
  while (i!=InitStrings.end()) {
    if ((*i)==sql) {
      InitStrings.erase(i);
      i=InitStrings.begin();
    } else
      i++;
  }
}
