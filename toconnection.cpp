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

#include <qwidget.h>
#include <qapplication.h>

#include "toconnection.h"
#include "tosql.h"
#include "toconf.h"
#include "tomain.h"
#include "totool.h"

// Connection provider implementation

std::map<QString,toConnectionProvider *> *toConnectionProvider::Providers;

void toConnectionProvider::checkAlloc(void)
{
  if (!Providers)
    Providers=new std::map<QString,toConnectionProvider *>;
}

toConnectionProvider::toConnectionProvider(const QString &provider)
{
  checkAlloc();
  Provider=provider; (*Providers)[Provider]=this;
}

toConnectionProvider::~toConnectionProvider()
{
  std::map<QString,toConnectionProvider *>::iterator i=Providers->find(Provider);
  if (i!=Providers->end())
    Providers->erase(i);
}

std::list<QString> toConnectionProvider::hosts(void)
{
  std::list<QString> ret;
  return ret;
}

std::list<QString> toConnectionProvider::providers(void)
{
  std::list<QString> ret;
  if (!Providers)
    return ret;
  for(std::map<QString,toConnectionProvider *>::iterator i=Providers->begin();i!=Providers->end();i++)
    ret.insert(ret.end(),(*i).first);
  return ret;
}

std::list<QString> toConnectionProvider::modes(void)
{
  std::list<QString> ret;
  ret.insert(ret.end(),"Normal");
  return ret;
}

toConnectionProvider &toConnectionProvider::fetchProvider(const QString &provider)
{
  checkAlloc();
  std::map<QString,toConnectionProvider *>::iterator i=Providers->find(provider);
  if (i==Providers->end())
    throw QString("Tried to fetch unknown provider %1").arg(provider);
  return *((*i).second);
}

std::list<QString> toConnectionProvider::modes(const QString &provider)
{
  return fetchProvider(provider).hosts();
}

toConnection::connectionImpl *toConnectionProvider::connection(const QString &provider,
							       toConnection *conn)
{
  return fetchProvider(provider).connection(conn);
}

std::list<QString> toConnectionProvider::hosts(const QString &provider)
{
  return fetchProvider(provider).hosts();
}

std::list<QString> toConnectionProvider::databases(const QString &provider,const QString &host,
						   const QString &user,const QString &pwd)
{
  return fetchProvider(provider).databases(host,user,pwd);
}

const QString &toConnectionProvider::config(const QString &tag,const QString &def)
{
  QString str=provider();
  str.append(":");
  str.append(tag);
  return toTool::globalConfig(str,def);
}

void toConnectionProvider::setConfig(const QString &tag,const QString &def)
{
  QString str=provider();
  str.append(":");
  str.append(tag);
  toTool::globalSetConfig(str,def);
}

// Query value implementation

toQValue::toQValue(int i)
{
  Type=intType;
  Value.Int=i;
}

toQValue::toQValue(double i)
{
  Type=doubleType;
  Value.Double=i;
}

toQValue::toQValue(const toQValue &copy)
{
  Type=copy.Type;
  switch(Type) {
  case intType:
    Value.Int=copy.Value.Int;
    break;
  case doubleType:
    Value.Double=copy.Value.Double;
    break;
  case stringType:
    Value.String=new QString(*copy.Value.String);
    break;
  case nullType:
    break;
  }
}

const toQValue &toQValue::operator = (const toQValue &copy)
{
  if (Type==stringType)
    delete Value.String;

  Type=copy.Type;
  switch(Type) {
  case intType:
    Value.Int=copy.Value.Int;
    break;
  case doubleType:
    Value.Double=copy.Value.Double;
    break;
  case stringType:
    Value.String=new QString(*copy.Value.String);
    break;
  case nullType:
    break;
  }
  return *this;
}

toQValue::toQValue(const QString &str)
{
  Type=stringType;
  Value.String=new QString(str);
}

toQValue::toQValue(void)
{
  Type=nullType;
}

toQValue::~toQValue()
{
  if (Type==stringType)
    delete Value.String;
}

bool toQValue::isInt(void) const
{
  return Type==intType;
}

bool toQValue::isDouble(void) const
{
  return Type==doubleType;
}

bool toQValue::isString(void) const
{
  return Type==stringType;
}

bool toQValue::isNull(void) const
{
  if (Type==nullType)
    return true;
  if (Type==stringType&&Value.String->isNull())
    return true;
  return false;
}

QCString toQValue::utf8Value(void) const
{
  switch(Type) {
  case nullType:
    {
      QCString ret;
      return ret;
    }
  case intType:
    {
      QCString ret;
      ret.setNum(Value.Int);
      return ret;
    }
  case doubleType:
    {
      QCString ret;
      ret.setNum(Value.Double);
      return ret;
    }
  case stringType:
    return Value.String->utf8();
  }
  throw QString("Unknown type of query value");
}

int toQValue::toInt(void) const
{
  switch(Type) {
  case nullType:
    return 0;
  case intType:
    return Value.Int;
  case doubleType:
    return int(Value.Double);
  case stringType:
    return Value.String->toInt();
  }
  throw QString("Unknown type of query value");
}

double toQValue::toDouble(void) const
{
  switch(Type) {
  case nullType:
    return 0;
  case intType:
    return double(Value.Int);
  case doubleType:
    return Value.Double;
  case stringType:
    return Value.String->toDouble();
  }
  throw QString("Unknown type of query value");
}

toQValue::operator QString() const
{
  switch(Type) {
  case nullType:
    return QString::null;
  case intType:
    return QString::number(Value.Int);
  case doubleType:
    return QString::number(Value.Double);
  case stringType:
    return *Value.String;
  }
  throw QString("Unknown type of query value");
}

// toQuery implementation

toQuery::toQuery(toConnection &conn,toSQL &sql,
		 const QString &arg1,const QString &arg2,
		 const QString &arg3,const QString &arg4,
		 const QString &arg5,const QString &arg6,
		 const QString &arg7,const QString &arg8,
		 const QString &arg9)
  : Connection(conn),ConnectionSub(conn.mainConnection()),SQL(sql(Connection))
{
  Mode=Normal;
  int numArgs;
  if (!arg9.isNull())
    numArgs=9;
  else if (!arg8.isNull())
    numArgs=8;
  else if (!arg7.isNull())
    numArgs=7;
  else if (!arg6.isNull())
    numArgs=6;
  else if (!arg5.isNull())
    numArgs=5;
  else if (!arg4.isNull())
    numArgs=4;
  else if (!arg3.isNull())
    numArgs=3;
  else if (!arg2.isNull())
    numArgs=2;
  else if (!arg1.isNull())
    numArgs=1;
  else
    numArgs=0;

  if (numArgs>0)
    Params.insert(Params.end(),arg1);
  if (numArgs>1)
    Params.insert(Params.end(),arg2);
  if (numArgs>2)
    Params.insert(Params.end(),arg3);
  if (numArgs>3)
    Params.insert(Params.end(),arg4);
  if (numArgs>4)
    Params.insert(Params.end(),arg5);
  if (numArgs>5)
    Params.insert(Params.end(),arg6);
  if (numArgs>6)
    Params.insert(Params.end(),arg7);
  if (numArgs>7)
    Params.insert(Params.end(),arg8);
  if (numArgs>8)
    Params.insert(Params.end(),arg9);
  
  toBusy busy;
  Query=conn.Connection->createQuery(this,ConnectionSub);
  Query->execute();
}

toQuery::toQuery(toConnection &conn,const QString &sql,
		 const QString &arg1,const QString &arg2,
		 const QString &arg3,const QString &arg4,
		 const QString &arg5,const QString &arg6,
		 const QString &arg7,const QString &arg8,
		 const QString &arg9)
  : Connection(conn),ConnectionSub(conn.mainConnection()),SQL(sql.utf8())
{
  Mode=Normal;
  int numArgs;
  if (!arg9.isNull())
    numArgs=9;
  else if (!arg8.isNull())
    numArgs=8;
  else if (!arg7.isNull())
    numArgs=7;
  else if (!arg6.isNull())
    numArgs=6;
  else if (!arg5.isNull())
    numArgs=5;
  else if (!arg4.isNull())
    numArgs=4;
  else if (!arg3.isNull())
    numArgs=3;
  else if (!arg2.isNull())
    numArgs=2;
  else if (!arg1.isNull())
    numArgs=1;
  else
    numArgs=0;

  if (numArgs>0)
    Params.insert(Params.end(),arg1);
  if (numArgs>1)
    Params.insert(Params.end(),arg2);
  if (numArgs>2)
    Params.insert(Params.end(),arg3);
  if (numArgs>3)
    Params.insert(Params.end(),arg4);
  if (numArgs>4)
    Params.insert(Params.end(),arg5);
  if (numArgs>5)
    Params.insert(Params.end(),arg6);
  if (numArgs>6)
    Params.insert(Params.end(),arg7);
  if (numArgs>7)
    Params.insert(Params.end(),arg8);
  if (numArgs>8)
    Params.insert(Params.end(),arg9);
  
  toBusy busy;
  Query=conn.Connection->createQuery(this,ConnectionSub);
  Query->execute();
}

toQuery::toQuery(toConnection &conn,toSQL &sql,const toQList &params)
  : Connection(conn),ConnectionSub(conn.mainConnection()),Params(params),SQL(sql(conn))
{  
  Mode=Normal;
  toBusy busy;
  Query=conn.Connection->createQuery(this,ConnectionSub);
  Query->execute();
}

toQuery::toQuery(toConnection &conn,const QString &sql,const toQList &params)
  : Connection(conn),ConnectionSub(conn.mainConnection()),Params(params),SQL(sql.utf8())
{
  Mode=Normal;
  toBusy busy;
  Query=conn.Connection->createQuery(this,ConnectionSub);
  Query->execute();
}

toQuery::toQuery(toConnection &conn,queryMode mode,toSQL &sql,const toQList &params)
  : Connection(conn),
    Params(params),
    SQL(sql(conn))
{
  Mode=mode;
  ConnectionSub=(mode!=Long?conn.mainConnection():conn.longConnection());
  toBusy busy;
  Query=conn.Connection->createQuery(this,ConnectionSub);
  Query->execute();
}

toQuery::toQuery(toConnection &conn,queryMode mode,const QString &sql,const toQList &params)
  : Connection(conn),
    Params(params),
    SQL(sql.utf8())
{
  Mode=mode;
  ConnectionSub=(mode!=Long?conn.mainConnection():conn.longConnection());
  toBusy busy;
  Query=conn.Connection->createQuery(this,ConnectionSub);
  Query->execute();
}

toQuery::toQuery(toConnection &conn,queryMode mode)
  : Connection(conn)
{
  Mode=mode;
  ConnectionSub=(mode!=Long?conn.mainConnection():conn.longConnection());
  toBusy busy;
  Query=conn.Connection->createQuery(this,ConnectionSub);
}

void toQuery::execute(toSQL &sql,const toQList &params)
{
  toBusy busy;
  SQL=sql(Connection);
  Params=params;
  Query->execute();
}

void toQuery::execute(const QString &sql,const toQList &params)
{
  toBusy busy;
  SQL=sql.utf8();
  Params=params;
  Query->execute();
}

toQuery::~toQuery()
{
  toBusy busy;
  delete Query;
  Connection.freeConnection(ConnectionSub);
}

bool toQuery::eof(void)
{
  if (Mode==All) {
    if (Query->eof()) {
      std::list<toConnectionSub *> &cons=Connection.connections();
      for(std::list<toConnectionSub *>::iterator i=cons.begin();i!=cons.end();i++) {
	if (*i==ConnectionSub) {
	  i++;
	  if (i!=cons.end()) {
	    ConnectionSub=*i;
	    delete Query;
	    Query=connection().Connection->createQuery(this,ConnectionSub);
	  }
	  break;
	}
      }
    }
  }
  return Query->eof();
}

toQList toQuery::readQuery(toConnection &conn,toSQL &sql,toQList &params)
{
  toBusy busy;
  toQuery query(conn,sql,params);
  toQList ret;
  while(!query.eof())
    ret.insert(ret.end(),query.readValue());
  return ret;
}

toQList toQuery::readQuery(toConnection &conn,const QString &sql,toQList &params)
{
  toBusy busy;
  toQuery query(conn,sql,params);
  toQList ret;
  while(!query.eof())
    ret.insert(ret.end(),query.readValue());
  return ret;
}

toQList toQuery::readQuery(toConnection &conn,toSQL &sql,
			   const QString &arg1,const QString &arg2,
			   const QString &arg3,const QString &arg4,
			   const QString &arg5,const QString &arg6,
			   const QString &arg7,const QString &arg8,
			   const QString &arg9)
{
  toBusy busy;
  toQuery query(conn,sql,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
  toQList ret;
  while(!query.eof())
    ret.insert(ret.end(),query.readValue());
  return ret;
}

toQList toQuery::readQuery(toConnection &conn,const QString &sql,
			   const QString &arg1,const QString &arg2,
			   const QString &arg3,const QString &arg4,
			   const QString &arg5,const QString &arg6,
			   const QString &arg7,const QString &arg8,
			   const QString &arg9)
{
  toBusy busy;
  toQuery query(conn,sql,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
  toQList ret;
  while(!query.eof())
    ret.insert(ret.end(),query.readValue());
  return ret;
}

toQList toQuery::readQueryNull(toConnection &conn,toSQL &sql,toQList &params)
{
  toBusy busy;
  toQuery query(conn,sql,params);
  toQList ret;
  while(!query.eof())
    ret.insert(ret.end(),query.readValueNull());
  return ret;
}

toQList toQuery::readQueryNull(toConnection &conn,const QString &sql,toQList &params)
{
  toBusy busy;
  toQuery query(conn,sql,params);
  toQList ret;
  while(!query.eof())
    ret.insert(ret.end(),query.readValueNull());
  return ret;
}

toQList toQuery::readQueryNull(toConnection &conn,toSQL &sql,
			       const QString &arg1,const QString &arg2,
			       const QString &arg3,const QString &arg4,
			       const QString &arg5,const QString &arg6,
			       const QString &arg7,const QString &arg8,
			       const QString &arg9)
{
  toBusy busy;
  toQuery query(conn,sql,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
  toQList ret;
  while(!query.eof())
    ret.insert(ret.end(),query.readValueNull());
  return ret;
}

toQList toQuery::readQueryNull(toConnection &conn,const QString &sql,
			       const QString &arg1,const QString &arg2,
			       const QString &arg3,const QString &arg4,
			       const QString &arg5,const QString &arg6,
			       const QString &arg7,const QString &arg8,
			       const QString &arg9)
{
  toBusy busy;
  toQuery query(conn,sql,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
  toQList ret;
  while(!query.eof())
    ret.insert(ret.end(),query.readValueNull());
  return ret;
}

toQValue toQuery::readValue(void)
{
  toBusy busy;
  if (Mode==All)
    eof();
  toQValue ret=Query->readValue();
  if (ret.isNull())
    return QString("{null}");
  return ret;
}

toQValue toQuery::readValueNull(void)
{
  toBusy busy;
  if (Mode==All)
    eof();
  return Query->readValue();
}

void toQuery::cancel(void)
{
  Query->cancel();
}

// toConnection implementation

void toConnection::addConnection(void)
{
  toBusy busy;
  toConnectionSub *sub=Connection->createConnection();
  Connections.insert(Connections.end(),sub);
  toQList params;
  for(std::list<QCString>::iterator i=InitStrings.begin();i!=InitStrings.end();i++) {
    try {
      Connection->execute(sub,*i,params);
    } TOCATCH
  }
}

toConnection::toConnection(const QString &provider,
			   const QString &user,const QString &password,
			   const QString &host,const QString &database,
			   const QString &mode,bool cache)
  : Provider(toConnectionProvider::fetchProvider(provider)),
    User(user),Password(password),Host(host),Database(database),Mode(mode)
{
  Connection=Provider.connection(this);
  addConnection();
  Version=Connection->version(mainConnection());
  NeedCommit=false;
  if (cache)
    readObjects();
  else
    ReadingValues.up();
}

toConnection::toConnection(const toConnection &conn)
  : Provider(conn.Provider),
    User(conn.User),
    Password(conn.Password),
    Host(conn.Host),
    Database(conn.Database),
    Mode(conn.Database)
{
  Connection=Provider.connection(this);
  addConnection();
  Version=Connection->version(mainConnection());
  ReadingValues.up();
}

toConnection::~toConnection()
{
  toBusy busy;
  ReadingValues.down();
  {
    for (std::list<QWidget *>::iterator i=Widgets.begin();i!=Widgets.end();i=Widgets.begin()) {
      delete (*i);
    }
  }
  for(std::list<toConnectionSub *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    Connection->closeConnection(*i);
  }
  delete Connection;
}

toConnectionSub *toConnection::mainConnection()
{
  return (*(Connections.begin()));
}

toConnectionSub *toConnection::longConnection()
{
#if 0
  if (toTool::globalConfig(CONF_LONG_SESSION,"").isEmpty())
    return mainConnection();
#endif
  if (Connections.size()==1)
    addConnection();
  std::list<toConnectionSub *>::iterator i=Connections.begin();
  i++;
  return (*i);
}

void toConnection::freeConnection(toConnectionSub *sub)
{
  for(std::list<toConnectionSub *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if (*i==sub)
      return;
  }
  Connections.insert(Connections.end(),sub);
}

void toConnection::commit(void)
{
  toBusy busy;
  for(std::list<toConnectionSub *>::iterator i=Connections.begin();i!=Connections.end();i++)
    Connection->commit(*i);
  while(Connections.size()>2) {
    std::list<toConnectionSub *>::iterator i=Connections.begin();
    i++;
    delete (*i);
    Connections.erase(i);
  }
  setNeedCommit(false);
}

void toConnection::rollback(void)
{
  toBusy busy;
  for(std::list<toConnectionSub *>::iterator i=Connections.begin();i!=Connections.end();i++)
    Connection->rollback(*i);
  while(Connections.size()>2) {
    std::list<toConnectionSub *>::iterator i=Connections.begin();
    i++;
    delete (*i);
    Connections.erase(i);
  }
  setNeedCommit(false);
}

void toConnection::delWidget(QWidget *widget)
{
  for (std::list<QWidget *>::iterator i=Widgets.begin();i!=Widgets.end();i++) {
    if ((*i)==widget) {
      Widgets.erase(i);
      break;
    }
  }
}

bool toConnection::closeWidgets(void)
{
  for (std::list<QWidget *>::iterator i=Widgets.begin();i!=Widgets.end();i=Widgets.begin()) {
    if (!(*i)->close(true))
      return false;
    std::list<QWidget *>::iterator nextI=Widgets.begin();
    if (i==nextI)
      throw QString("All tool widgets need to have autodelete flag set");
  }
  return true;
}

QString toConnection::description(void) const
{
  QString ret(User);
  ret+="@";
  ret+=Database;
  if (!Host.isEmpty()&&Host!="SQL*Net") {
    ret+=".";
    ret+=Host;
  }

  ret+=" [";
  if (!Version.isEmpty())
    ret+=Version;
  ret+="]";
  return ret;
}

void toConnection::addInit(const QString &sql)
{
  delInit(sql);
  InitStrings.insert(InitStrings.end(),sql.utf8());
}

void toConnection::delInit(const QString &sql)
{
  QCString utf=sql.utf8();
  std::list<QCString>::iterator i=InitStrings.begin();
  while (i!=InitStrings.end()) {
    if ((*i)==utf) {
      InitStrings.erase(i);
      i=InitStrings.begin();
    } else
      i++;
  }
}

void toConnection::execute(toSQL &sql,toQList &params)
{
  toBusy busy;
  Connection->execute(mainConnection(),toSQL::sql(sql,*this),params);
}

void toConnection::execute(const QString &sql,toQList &params)
{
  toBusy busy;
  Connection->execute(mainConnection(),sql.utf8(),params);
}

void toConnection::execute(toSQL &sql,
			   const QString &arg1,const QString &arg2,
			   const QString &arg3,const QString &arg4,
			   const QString &arg5,const QString &arg6,
			   const QString &arg7,const QString &arg8,
			   const QString &arg9)
{
  int numArgs;
  if (!arg9.isNull())
    numArgs=9;
  else if (!arg8.isNull())
    numArgs=8;
  else if (!arg7.isNull())
    numArgs=7;
  else if (!arg6.isNull())
    numArgs=6;
  else if (!arg5.isNull())
    numArgs=5;
  else if (!arg4.isNull())
    numArgs=4;
  else if (!arg3.isNull())
    numArgs=3;
  else if (!arg2.isNull())
    numArgs=2;
  else if (!arg1.isNull())
    numArgs=1;
  else
    numArgs=0;

  toQList args;
  if (numArgs>0)
    args.insert(args.end(),arg1);
  if (numArgs>1)
    args.insert(args.end(),arg2);
  if (numArgs>2)
    args.insert(args.end(),arg3);
  if (numArgs>3)
    args.insert(args.end(),arg4);
  if (numArgs>4)
    args.insert(args.end(),arg5);
  if (numArgs>5)
    args.insert(args.end(),arg6);
  if (numArgs>6)
    args.insert(args.end(),arg7);
  if (numArgs>7)
    args.insert(args.end(),arg8);
  if (numArgs>8)
    args.insert(args.end(),arg9);

  toBusy busy;
  Connection->execute(mainConnection(),toSQL::sql(sql,*this),args);
}

void toConnection::execute(const QString &sql,
			   const QString &arg1,const QString &arg2,
			   const QString &arg3,const QString &arg4,
			   const QString &arg5,const QString &arg6,
			   const QString &arg7,const QString &arg8,
			   const QString &arg9)
{
  int numArgs;
  if (!arg9.isNull())
    numArgs=9;
  else if (!arg8.isNull())
    numArgs=8;
  else if (!arg7.isNull())
    numArgs=7;
  else if (!arg6.isNull())
    numArgs=6;
  else if (!arg5.isNull())
    numArgs=5;
  else if (!arg4.isNull())
    numArgs=4;
  else if (!arg3.isNull())
    numArgs=3;
  else if (!arg2.isNull())
    numArgs=2;
  else if (!arg1.isNull())
    numArgs=1;
  else
    numArgs=0;

  toQList args;
  if (numArgs>0)
    args.insert(args.end(),arg1);
  if (numArgs>1)
    args.insert(args.end(),arg2);
  if (numArgs>2)
    args.insert(args.end(),arg3);
  if (numArgs>3)
    args.insert(args.end(),arg4);
  if (numArgs>4)
    args.insert(args.end(),arg5);
  if (numArgs>5)
    args.insert(args.end(),arg6);
  if (numArgs>6)
    args.insert(args.end(),arg7);
  if (numArgs>7)
    args.insert(args.end(),arg8);
  if (numArgs>8)
    args.insert(args.end(),arg9);

  toBusy busy;
  Connection->execute(mainConnection(),sql.utf8(),args);
}

void toConnection::allExecute(toSQL &sql,toQList &params)
{
  toBusy busy;
  for(std::list<toConnectionSub *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    try {
      Connection->execute(*i,toSQL::sql(sql,*this),params);
    } TOCATCH
  }
}

void toConnection::allExecute(const QString &sql,toQList &params)
{
  toBusy busy;
  for(std::list<toConnectionSub *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    try {
      Connection->execute(*i,sql.utf8(),params);
    } TOCATCH
  }
}

void toConnection::allExecute(toSQL &sql,
			      const QString &arg1,const QString &arg2,
			      const QString &arg3,const QString &arg4,
			      const QString &arg5,const QString &arg6,
			      const QString &arg7,const QString &arg8,
			      const QString &arg9)
{
  int numArgs;
  if (!arg9.isNull())
    numArgs=9;
  else if (!arg8.isNull())
    numArgs=8;
  else if (!arg7.isNull())
    numArgs=7;
  else if (!arg6.isNull())
    numArgs=6;
  else if (!arg5.isNull())
    numArgs=5;
  else if (!arg4.isNull())
    numArgs=4;
  else if (!arg3.isNull())
    numArgs=3;
  else if (!arg2.isNull())
    numArgs=2;
  else if (!arg1.isNull())
    numArgs=1;
  else
    numArgs=0;

  toQList args;
  if (numArgs>0)
    args.insert(args.end(),arg1);
  if (numArgs>1)
    args.insert(args.end(),arg2);
  if (numArgs>2)
    args.insert(args.end(),arg3);
  if (numArgs>3)
    args.insert(args.end(),arg4);
  if (numArgs>4)
    args.insert(args.end(),arg5);
  if (numArgs>5)
    args.insert(args.end(),arg6);
  if (numArgs>6)
    args.insert(args.end(),arg7);
  if (numArgs>7)
    args.insert(args.end(),arg8);
  if (numArgs>8)
    args.insert(args.end(),arg9);

  toBusy busy;
  for(std::list<toConnectionSub *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    try {
      Connection->execute(*i,toSQL::sql(sql,*this),args);
    } TOCATCH
  }
}

void toConnection::allExecute(const QString &sql,
			      const QString &arg1,const QString &arg2,
			      const QString &arg3,const QString &arg4,
			      const QString &arg5,const QString &arg6,
			      const QString &arg7,const QString &arg8,
			      const QString &arg9)
{
  int numArgs;
  if (!arg9.isNull())
    numArgs=9;
  else if (!arg8.isNull())
    numArgs=8;
  else if (!arg7.isNull())
    numArgs=7;
  else if (!arg6.isNull())
    numArgs=6;
  else if (!arg5.isNull())
    numArgs=5;
  else if (!arg4.isNull())
    numArgs=4;
  else if (!arg3.isNull())
    numArgs=3;
  else if (!arg2.isNull())
    numArgs=2;
  else if (!arg1.isNull())
    numArgs=1;
  else
    numArgs=0;

  toQList args;
  if (numArgs>0)
    args.insert(args.end(),arg1);
  if (numArgs>1)
    args.insert(args.end(),arg2);
  if (numArgs>2)
    args.insert(args.end(),arg3);
  if (numArgs>3)
    args.insert(args.end(),arg4);
  if (numArgs>4)
    args.insert(args.end(),arg5);
  if (numArgs>5)
    args.insert(args.end(),arg6);
  if (numArgs>6)
    args.insert(args.end(),arg7);
  if (numArgs>7)
    args.insert(args.end(),arg8);
  if (numArgs>8)
    args.insert(args.end(),arg9);

  toBusy busy;
  for(std::list<toConnectionSub *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    try {
      Connection->execute(*i,sql.utf8(),args);
    } TOCATCH
  }
}

const QString &toConnection::provider(void) const
{
  return Provider.provider();
}

void toConnection::cacheObjects::run()
{
  try {
    Connection.ObjectNames=Connection.Connection->objectNames();
    Connection.SynonymMap=Connection.Connection->synonymMap(Connection.ObjectNames);
    Connection.ObjectNames.sort();
  } catch(...) {
  }
  Connection.ReadingValues.up();
}


void toConnection::readObjects(void)
{
  (new toThread(new cacheObjects(*this)))->start();
}

void toConnection::rereadCache(void)
{
  if(ReadingValues.getValue()==0) {
    toStatusMessage("Not done caching objects, can not clear unread cache");
    return;
  }
  while(ReadingValues.getValue()>0)
    ReadingValues.down();
  ObjectNames.clear();
  ColumnCache.clear();
  SynonymMap.clear();
  readObjects();
}

QString toConnection::quote(const QString &name)
{
  return Connection->quote(name);
}

QString toConnection::unQuote(const QString &name)
{
  return Connection->unQuote(name);
}

bool toConnection::checkAvail(bool block)
{
  if (ReadingValues.getValue()==0) {
    if (block) {
      ReadingValues.down();
      ReadingValues.up();
    } else
      return false;
  }
  return true;
}

std::list<toConnection::objectName> &toConnection::objects(bool block)
{
  if (!checkAvail(block)) {
    toStatusMessage("Not done caching objects");
    static std::list<objectName> ret;
    return ret;
  }

  return ObjectNames;
}

std::map<QString,toConnection::objectName> &toConnection::synonyms(bool block)
{
  if (!checkAvail(block)) {
    toStatusMessage("Not done caching objects");
    static std::map<QString,objectName> ret;
    return ret;
  }

  return SynonymMap;
}

const toConnection::objectName &toConnection::realName(const QString &object,
						       QString &synonym,
						       bool block)
{
  if (!checkAvail(block))
    throw QString("Not done caching objects");

  QString name;
  QString owner;

  bool quote=false;
  for (unsigned int pos=0;pos<object.length();pos++) {
    if (object.at(pos)=="\"") {
      quote=!quote;
    } else {
      if (!quote&&object.at(pos)=='.') {
	owner=name;
	name="";
      } else
	name+=object.at(pos);
    }
  }

  QString uo=owner.upper();
  QString un=name.upper();

  synonym=QString::null;
  for(std::list<objectName>::iterator i=ObjectNames.begin();i!=ObjectNames.end();i++) {
    if (owner.isEmpty()) {
      if (((*i).Name==un||(*i).Name==name)&&
	  ((*i).Owner==user().upper()||(*i).Owner==database()))
	return *i;
    } else if (((*i).Name==un||(*i).Name==name)&&
	       ((*i).Owner==uo||(*i).Owner==owner))
      return *i;
  }
  if (owner.isEmpty()) {
    std::map<QString,objectName>::iterator i=SynonymMap.find(name);
    if (i==SynonymMap.end()&&un!=name) {
      i=SynonymMap.find(un);
      synonym=un;
    } else
      synonym=name;
    if (i!=SynonymMap.end()) {
      return (*i).second;
    }
  }
  throw QString("Object %1 not available for %2").arg(object).arg(user());
}

const toConnection::objectName &toConnection::realName(const QString &object,bool block)
{
  QString dummy;
  return realName(object,dummy,block);
}

toQDescList &toConnection::columns(const objectName &object)
{
  std::map<objectName,toQDescList>::iterator i=ColumnCache.find(object);
  if (i==ColumnCache.end())
    ColumnCache[object]=Connection->columnDesc(object);

  return ColumnCache[object];
}

bool toConnection::objectName::operator < (const objectName &nam) const
{
  if (Owner<nam.Owner||(Owner.isNull()&&!nam.Owner.isNull()))
    return true;
  if (Owner>nam.Owner||(!Owner.isNull()&&nam.Owner.isNull()))
    return false;
  if (Name<nam.Name||(Name.isNull()&&!nam.Name.isNull()))
    return true;
  if (Name>nam.Name||(!Name.isNull()&&nam.Name.isNull()))
    return false;
  if (Type<nam.Type)
    return true;
  return false;
}

bool toConnection::objectName::operator == (const objectName &nam) const
{
  return Owner==nam.Owner&&Name==nam.Name&&Type==nam.Type;
}

std::list<toConnection::objectName> toConnection::connectionImpl::objectNames(void)
{
  std::list<toConnection::objectName> ret;
  return ret;
}

std::map<QString,toConnection::objectName> toConnection::connectionImpl::synonymMap(std::list<toConnection::objectName> &)
{
  std::map<QString,toConnection::objectName> ret;
  return ret;
}

toQDescList toConnection::connectionImpl::columnDesc(const objectName &)
{
  toQDescList ret;
  return ret;
}
