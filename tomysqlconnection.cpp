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

#include <qfile.h>
#include <qregexp.h>

#include <mysql.h>

#include "toconnection.h"
#include "toconf.h"
#include "totool.h"

#define BUF_SIZE 102400

static QCString QueryParam(const QString &query,toQList &params,const QString &codec)
{
  QString ret;
  bool inString=false;
  toQList::iterator cpar=params.begin();
  bool quotes=true;

  map<QString,QString> binds;

  {
    QString t=query.upper();
    if (t.startsWith("SHOW")||t.startsWith("CONNECT"))
      quotes=false;
  }

  for(unsigned int i=0;i<query.length();i++) {
    QChar c=query[i];
    switch(char(c)) {
    case '\\':
      ret+=c;
      ret+=query[++i];
      break;
    case '\'':
      inString=!inString;
      ret+=c;
      break;
    case ':':
      if (!inString) {
	QString nam;
	for (i++;i<query.length();i++) {
	  c=query[i];
	  if (!c.isLetterOrNumber())
	    break;
	  nam+=c;
	}
	if (c=='<') {
	  for (i++;i<query.length();i++) {
	    c=query[i];
	    if (c!='>')
	      break;
	  }
	}
	i--;
	
	if (nam.isEmpty())
	  throw QString("No bind name");

	if (binds.find(nam)!=binds.end()) {
	  ret+=binds[nam];
	  break;
	}
	if (cpar==params.end())
	  throw QString("Not all bind variables suplied");
	QString str;
	if ((*cpar).isNull()) {
	  str="NULL";
	} else if ((*cpar).isInt()||(*cpar).isDouble()) {
	  str=QString(*cpar);
	} else {
	  if (quotes)
	    str+="'";
	  QString tmp=(*cpar);
	  for(unsigned int j=0;j<tmp.length();j++) {
	    QChar d=tmp[j];
	    switch(char(d)) {
	    case 0:
	      str+="\\0";
	      break;
	    case '\n':
	      str+="\\n";
	      break;
	    case '\t':
	      str+="\\t";
	      break;
	    case '\r':
	      str+="\\r";
	      break;
	    case '\'':
	      str+="\\\'";
	      break;
	    case '\"':
	      str+="\\\"";
	      break;
	    case '\\':
	      str+="\\\\";
	      break;
	    default:
	      str+=d;
	    }
	  }
	  if (quotes)
	    str+="'";
	}
	binds[nam]=str;
	ret+=str;
	cpar++;
	break;
      }
    default:
      ret+=c;
    }
  }
  // More complex conversion here later
  return ret.latin1();
}

class toMysqlProvider : public toConnectionProvider {
public:
  class mysqlConnection;
  struct mysqlSub : public toConnectionSub {
    mysqlConnection *Connection;
    mysqlSub(mysqlConnection *connection)
    { Connection=connection; }
  };
  class mysqlQuery : public toQuery::queryImpl {
    MYSQL_RES *Result;
    MYSQL_FIELD *Fields;
    MYSQL_ROW Row;
    int CurrentCol;
    int Columns;
    int AffectedRows;
    mysqlConnection *mysql(void);
  public:
    mysqlQuery(toQuery *query,toConnectionSub *conn)
      : toQuery::queryImpl(query)
    {
      Result=NULL;
      AffectedRows=Columns=0;
      Fields=NULL;
    }
    virtual ~mysqlQuery()
    {
      if (Result)
	mysql_free_result(Result);
    }
    virtual void execute(void);

    virtual toQValue readValue(void)
    {
      if (eof())
	throw QString("Tried to read past end of query");
      // More complex conversion here later
      QString ret(Row[CurrentCol]);
      CurrentCol++;
      if (CurrentCol==Columns) {
	CurrentCol=0;
	Row=mysql_fetch_row(Result);
      }
      return ret;
    }
    virtual bool eof(void)
    {
      if (!Result)
	return true;
      if (!Row)
	return true;
      return false;
    }
    virtual int rowsProcessed(void)
    { return AffectedRows; }
    virtual int columns(void)
    { return Columns; }
    virtual std::list<toQuery::queryDescribe> describe(void)
    {
      toQDescList ret;
      toQDescribe cur;
      for(int i=0;i<Columns;i++) {
	QString type;
	cur.Name=Fields[i].name;
	if (IS_NUM(Fields[i].type)&&Fields[i].flags&UNSIGNED_FLAG)
	  type="UNSIGNED ";
	switch(Fields[i].type) {
	case FIELD_TYPE_TINY:
	  type+="TINYINT";
	  break;
	case FIELD_TYPE_SHORT:
	  type+="SMALLINT";
	  break;
	case FIELD_TYPE_LONG:
	  type+="INTEGER";
	  break;
	case FIELD_TYPE_INT24:
	  type+="MEDIUMINT";
	  break;
	case FIELD_TYPE_LONGLONG:
	  type+="BIGINT";
	  break;
	case FIELD_TYPE_DECIMAL:
	  type+="DECIMAL";
	  break;
	case FIELD_TYPE_FLOAT:
	  type+="FLOAT";
	  break;
	case FIELD_TYPE_DOUBLE:
	  type+="DOUBLE";
	  break;
	case FIELD_TYPE_TIMESTAMP:
	  type+="TIMESTAMP";
	  break;
	case FIELD_TYPE_DATETIME:
	  type+="DATETIME";
	  break;
	case FIELD_TYPE_DATE:
	  type+="DATE";
	  break;
	case FIELD_TYPE_TIME:
	  type+="TIME";
	  break;
	case FIELD_TYPE_YEAR:
	  type+="YEAR";
	  break;
	case FIELD_TYPE_VAR_STRING:
	  type+="VARCHAR(";
	  type+=QString::number(Fields[i].length);
	  type+=")";
	  break;
	case FIELD_TYPE_STRING:
	  type+="CHAR(";
	  type+=QString::number(Fields[i].length);
	  type+=")";
	  break;
	case FIELD_TYPE_TINY_BLOB:
	case FIELD_TYPE_MEDIUM_BLOB:
	case FIELD_TYPE_LONG_BLOB:
	case FIELD_TYPE_BLOB:
	  type+="BLOB";
	  break;
	case FIELD_TYPE_SET:
	  type+="SET";
	  break;
	case FIELD_TYPE_ENUM:
	  type+="ENUM";
	  break;
	default:
	  type+="UNKNOWN";
	}
	cur.Datatype=type;
	cur.Null=!(Fields[i].type&NOT_NULL_FLAG);
	ret.insert(ret.end(),cur);
      }
      return ret;
    }
  };

  class mysqlConnection : public toConnection::connectionImpl {
    MYSQL *Connection;
    QString Codec;
  public:
    mysqlConnection(toConnection *conn)
      : toConnection::connectionImpl(conn)
    {
      Connection=mysql_init(NULL);
      if (!Connection)
	throw QString("Can't initialize MySQL connection");
      Connection=mysql_real_connect(Connection,
				    conn->host(),
				    conn->user(),
				    conn->password(),
				    conn->database()==QString::null?NULL:(const char *)conn->database(),
				    0,NULL,0);
      if (!Connection)
	throw QString("Couldn't open MySQL connection");
    }

    virtual void commit(toConnectionSub *sub)
    {
      // Doesn't support transactions
    }
    virtual void rollback(toConnectionSub *sub)
    {
      // Doesn't support transactions
    }

    void throwError(void)
    {
      char *str=mysql_error(Connection);
      if (str)
	throw QString(str);
      else
	throw QString("Unknown error has occured in MySQL connection");
    }
    MYSQL *mysql(void)
    { return Connection; }
    const QString &codec(void) const
    { return Codec; }


    virtual toConnectionSub *createConnection(void)
    {
      return new mysqlSub(this);
    }
    void closeConnection(toConnectionSub *conn)
    {
      delete conn;
    }

    virtual QString version(toConnectionSub *sub)
    {
      char *version=mysql_get_server_info(Connection);
      if (!version)
	return "Unknown";
      return version;
    }
    virtual toConnection::connectionImpl *clone(toConnection *newConn) const
    { return new mysqlConnection(newConn); }

    virtual toQuery::queryImpl *createQuery(toQuery *query,toConnectionSub *sub)
    { return new mysqlQuery(query,sub); }
    virtual void execute(toConnectionSub *sub,const QCString &sql,toQList &params)
    {
      QCString query=QueryParam(sql,params,Codec);
      if(mysql_real_query(Connection,query,query.length())!=0)
	throwError();
    }
  };

  toMysqlProvider(void)
    : toConnectionProvider("MySQL")
  {
  }

  virtual toConnection::connectionImpl *connection(toConnection *conn)
  { return new mysqlConnection(conn); }
  virtual std::list<QString> hosts(void)
  {
    std::list<QString> ret;
    ret.insert(ret.end(),"localhost");
    return ret;
  }
  virtual std::list<QString> databases(const QString &host,
				       const QString &user,const QString &pwd)
  {
    std::list<QString> ret;
    try {
      toConnection conn("MySQL",user,pwd,host,QString::null,"Normal");
      {
	toQuery query(conn,"SHOW DATABASES");
	while(!query.eof())
	  ret.insert(ret.end(),QString(query.readValue()));
      }
    } catch (...) {

    }
    return ret;
  }
};

static toMysqlProvider MysqlProvider;

void toMysqlProvider::mysqlQuery::execute(void)
{
  if (Result) {
    mysql_free_result(Result);
    Result=NULL;
  }
  QCString sql=QueryParam(QString::fromUtf8(query()->sql()),
			  query()->params(),mysql()->codec());
  if(mysql_real_query(mysql()->mysql(),
		      sql,
		      sql.length())!=0)
    mysql()->throwError();
  Result=mysql_store_result(mysql()->mysql());
  if (Result) {
    Columns=mysql_num_fields(Result);
    Fields=mysql_fetch_fields(Result);
    AffectedRows=0;
    CurrentCol=0;
    Row=mysql_fetch_row(Result);
  } else if (mysql_field_count(mysql()->mysql())==0) {
    AffectedRows=mysql_affected_rows(mysql()->mysql());
    Fields=NULL;
    Columns=0;
  } else
    mysql()->throwError();
}

toMysqlProvider::mysqlConnection *toMysqlProvider::mysqlQuery::mysql(void)
{
  mysqlSub *conn=dynamic_cast<mysqlSub *>(query()->connectionSub());
  if (!conn)
    throw QString("Internal error MySQL query on non MySQL connection");
  return conn->Connection;
}
