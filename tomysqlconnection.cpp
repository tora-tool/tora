//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include <qfile.h>
#include <qregexp.h>

#ifdef WIN32
#include <winsock.h>
#endif
#include <mysql.h>

#include "toconnection.h"
#include "toconf.h"
#include "totool.h"
#include "tosql.h"

#include <qapplication.h>

#define BUF_SIZE 102400

static toSQL SQLListTables("toMysqlConnection:ListTables",
			   "show tables",
			   "Get the available tables for a mysql connection",
			   "3.0",
			   "MySQL");

static QCString QueryParam(const QString &query,toQList &params,const QString &codec)
{
  QString ret;
  bool inString=false;
  toQList::iterator cpar=params.begin();

  std::map<QString,QString> binds;

  for(unsigned int i=0;i<query.length();i++) {
    QChar c=query.at(i);
    switch(char(c)) {
    case '\\':
      ret+=c;
      ret+=query.at(++i);
      break;
    case '\'':
      inString=!inString;
      ret+=c;
      break;
    case ':':
      if (!inString) {
	QString nam;
	for (i++;i<query.length();i++) {
	  c=query.at(i);
	  if (!c.isLetterOrNumber())
	    break;
	  nam+=c;
	}
	QString in;
	if (c=='<') {
	  for (i++;i<query.length();i++) {
	    c=query.at(i);
	    if (c=='>') {
	      i++;
	      break;
	    }
	    in+=c;
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
	  if (in!="noquote")
	    str+="'";
	  QString tmp=(*cpar);
	  for(unsigned int j=0;j<tmp.length();j++) {
	    QChar d=tmp.at(j);
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
	  if (in!="noquote")
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
  //  printf("Parsed: %s\n",(const char *)ret);
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
    virtual void cancel(void)
    {
      // Not implemented
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
	cur.AlignRight=true;
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
	  cur.AlignRight=false;
	  type+="VARCHAR(";
	  type+=QString::number(Fields[i].length);
	  type+=")";
	  break;
	case FIELD_TYPE_STRING:
	  cur.AlignRight=false;
	  type+="CHAR(";
	  type+=QString::number(Fields[i].length);
	  type+=")";
	  break;
	case FIELD_TYPE_TINY_BLOB:
	case FIELD_TYPE_MEDIUM_BLOB:
	case FIELD_TYPE_LONG_BLOB:
	case FIELD_TYPE_BLOB:
	  cur.AlignRight=false;
	  type+="BLOB";
	  break;
	case FIELD_TYPE_SET:
	  cur.AlignRight=false;
	  type+="SET";
	  break;
	case FIELD_TYPE_ENUM:
	  cur.AlignRight=false;
	  type+="ENUM";
	  break;
	default:
	  cur.AlignRight=false;
	  type+="UNKNOWN";
	  break;
	}
	cur.Datatype=type;
	cur.Null=!(Fields[i].flags&NOT_NULL_FLAG);
	ret.insert(ret.end(),cur);
      }
      return ret;
    }
  };

  class mysqlConnection : public toConnection::connectionImpl {
    toLock Lock;
    MYSQL *Connection;
    QString Codec;
  public:
    mysqlConnection(toConnection *conn)
      : toConnection::connectionImpl(conn)
    {
      Connection=mysql_init(NULL);
      if (!Connection)
	throw QString("Can't initialize MySQL connection");
      QString db=conn->database();
      QString host=conn->host(); 
      int port=0;
      int pos=host.find(":"); 
      if (pos>=0) { 
	port=host.mid(pos+1).toInt(); 
	host=host.mid(0,pos); 
      }
      Connection=mysql_real_connect(Connection,
				    host,
				    conn->user(),
				    conn->password(),
				    db==QString::null?NULL:(const char *)db,
				    port,NULL,0);
      if (!Connection)
	throw QString("Couldn't open MySQL connection");
    }

    virtual std::list<toConnection::objectName> objectNames(void)
    {
      std::list<toConnection::objectName> ret;

      toQuery tables(connection(),SQLListTables);
      toConnection::objectName cur;
      cur.Type="TABLE";
      while(!tables.eof()) {
	cur.Name=tables.readValueNull();
	cur.Owner=connection().database();
	ret.insert(ret.end(),cur);
      }

      return ret;
    }
    virtual toQDescList columnDesc(const toConnection::objectName &table)
    {
      try {
	QString SQL="SELECT * FROM ";
	SQL+=table.Name;
	SQL+=" WHERE NULL=NULL";
	toQuery query(connection(),SQL);
	return query.describe();
      } catch(...) {
      }

      toQDescList ret;
      return ret;
    }

    virtual void commit(toConnectionSub *sub)
    {
      // Doesn't support transactions
    }
    virtual void rollback(toConnectionSub *sub)
    {
      // Doesn't support transactions
    }

    void throwError(const QString &sql=QString::null)
    {
      const char *str=mysql_error(Connection);
      if (str) {
	if (!sql.isEmpty()) {
	  QString err=str;
	  err+="\n";
	  err+=sql;
	  throw QString(err);
	} else
	  throw QString(str);
      } else
	throw QString("Unknown error has occured in MySQL connection");
    }
    MYSQL *mysql(void)
    { return Connection; }
    const QString &codec(void) const
    { return Codec; }
    toLock &lock(void)
    { return Lock; }


    virtual toConnectionSub *createConnection(void)
    {
      return new mysqlSub(this);
    }
    void closeConnection(toConnectionSub *conn)
    {
      delete conn;
    }

    virtual QCString version(toConnectionSub *sub)
    {
      const char *version=mysql_get_server_info(Connection);
      if (!version)
	return "Unknown";
      return version;
    }

    virtual toQuery::queryImpl *createQuery(toQuery *query,toConnectionSub *sub)
    { return new mysqlQuery(query,sub); }
    virtual void execute(toConnectionSub *sub,const QCString &sql,toQList &params)
    {
      toLocker lock(Lock);
      QCString query=QueryParam(sql,params,Codec);
      if(mysql_real_query(Connection,query,query.length())!=0)
	throwError();
    }
  };

  toMysqlProvider(void)
    : toConnectionProvider("MySQL")
  {
  }

  virtual toConnection::connectionImpl *provideConnection(const QCString &,toConnection *conn)
  { return new mysqlConnection(conn); }
  virtual std::list<QString> providedHosts(const QCString &)
  {
    std::list<QString> ret;
    ret.insert(ret.end(),"localhost");
    return ret;
  }
  virtual std::list<QString> providedDatabases(const QCString &,const QString &host,
					       const QString &user,const QString &pwd)
  {
    std::list<QString> ret;
    try {
      toConnection conn("MySQL",user,pwd,host,QString::null,"Normal",false);
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
  toLocker lock(mysql()->lock());
  if(mysql_real_query(mysql()->mysql(),
		      sql,
		      sql.length())!=0)
    mysql()->throwError(sql);
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
    mysql()->throwError(sql);
}

toMysqlProvider::mysqlConnection *toMysqlProvider::mysqlQuery::mysql(void)
{
  mysqlSub *conn=dynamic_cast<mysqlSub *>(query()->connectionSub());
  if (!conn)
    throw QString("Internal error MySQL query on non MySQL connection");
  return conn->Connection;
}
