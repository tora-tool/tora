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

#include <qwidget.h>
#include <qapplication.h>

#include "toconnection.h"
#include "tosql.h"
#include "toconf.h"
#include "tomain.h"

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

std::list<QString> toConnectionProvider::databases(const QString &provider,const QString &host)
{
  return fetchProvider(provider).databases(host);
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
		 const QString &arg1=QString::null,const QString &arg2=QString::null,
		 const QString &arg3=QString::null,const QString &arg4=QString::null,
		 const QString &arg5=QString::null,const QString &arg6=QString::null,
		 const QString &arg7=QString::null,const QString &arg8=QString::null,
		 const QString &arg9=QString::null)
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
		 const QString &arg1=QString::null,const QString &arg2=QString::null,
		 const QString &arg3=QString::null,const QString &arg4=QString::null,
		 const QString &arg5=QString::null,const QString &arg6=QString::null,
		 const QString &arg7=QString::null,const QString &arg8=QString::null,
		 const QString &arg9=QString::null)
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
  SQL=sql(Connection);
  Params=params;
  Query->execute();
}

void toQuery::execute(const QString &sql,const toQList &params)
{
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
			   const QString &mode)
  : Provider(toConnectionProvider::fetchProvider(provider)),
    User(user),Password(password),Host(host),Database(database),Mode(mode)
{
  Connection=Provider.connection(this);
  addConnection();
  Version=Connection->version(mainConnection());
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
}

toConnection::~toConnection()
{
  toBusy busy;
  for (std::list<QWidget *>::iterator i=Widgets.begin();i!=Widgets.end();i=Widgets.begin()) {
    delete (*i);
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

static toSQL SQLListObjects("toConnection:ListObjects",
			    "select table_name,owner,null from all_tables\n"
			    "union\n"
			    "select view_name,owner,null from all_views\n"
			    "union\n"
			    "select table_name,table_owner,synonym_name\n"
			    "  from all_synonyms where owner in ('PUBLIC',:owner<char[101]>)",
			    "List the objects to cache for a connection, should have same "
			    "columns and binds");

void toConnection::readObjects(void)
{
  if (TableNames.size()==0) {
    toBusy busy;
    toStatusMessage("Reading available objects",true);
    qApp->processEvents();
    toQuery tables(*this,SQLListObjects,user());
    while(!tables.eof()) {
      tableName cur;
      cur.Name=tables.readValueNull();
      cur.Owner=tables.readValueNull();
      cur.Synonym=tables.readValueNull();
      TableNames.insert(TableNames.end(),cur);
    }
    TableNames.sort();
    toStatusMessage("");
  }
}

std::list<toConnection::tableName> &toConnection::tables(void)
{
  readObjects();
  return TableNames;
}

const toConnection::tableName &toConnection::realName(const QString &object)
{
  QString uo=object.upper();
  readObjects();
  for(std::list<tableName>::iterator i=TableNames.begin();i!=TableNames.end();i++) {
    if ((*i).Synonym==uo)
      return *i;
    if ((*i).Name==uo&&(*i).Owner==user())
      return *i;
  }
  throw QString("Object %1 not available for %2").arg(object).arg(user());
}

std::list<QString> &toConnection::columns(const tableName &table)
{
  std::map<tableName,std::list<QString> >::iterator i=ColumnCache.find(table);
  if (i==ColumnCache.end()) {
    std::list<QString> cols;
    QString SQL="SELECT * FROM \"";
    SQL+=table.Owner;
    SQL+="\".\"";
    SQL+=table.Name;
    SQL+="\" WHERE NULL=NULL";
    toQuery query(*this,SQL);
    toQDescList desc=query.describe();
    for(toQDescList::iterator j=desc.begin();j!=desc.end();j++)
      toPush(cols,(*j).Name);
    ColumnCache[table]=cols;
  }
  return ColumnCache[table];
}

bool toConnection::tableName::operator < (const tableName &nam) const
{
  if (Owner<nam.Owner)
    return true;
  if (Owner>nam.Owner)
    return false;
  if (Name<nam.Name)
    return true;
  return false;
}

bool toConnection::tableName::operator == (const tableName &nam) const
{
  return Owner==nam.Owner&&Name==nam.Name;
}
