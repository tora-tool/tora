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

#include "utils.h"

#include "toconnection.h"
#include "tomain.h"
#include "tosql.h"

#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlrecord.h>
#include <qsqlresult.h>

static toSQL SQLVersion("toQSqlConnection:Version",
			"SHOW VARIABLES LIKE 'version'",
			"Show version of database, "
			"last value of first return record of result is used.",
			"3.0",
			"MySQL");

static toSQL SQLListTables("toQSqlConnection:ListTables",
			   "show tables",
			   "Get the available tables for a mysql connection",
			   "3.0",
			   "MySQL");

static QString QueryParam(const QString &in,toQList &params)
{
  QString ret;
  bool inString=false;
  toQList::iterator cpar=params.begin();
  QString query=QString::fromUtf8(in);

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

  return ret;
}

static QString ErrorString(const QSqlError &err)
{
  QString ret;
  if (err.databaseText().isEmpty()) {
    if (err.driverText().isEmpty())
      ret="Unknown error";
    else
      ret=err.driverText();
  } else
    ret=err.databaseText();
  return ret;
}

class toQSqlProvider : public toConnectionProvider {
public:
  static QString fromQSqlName(const QString &driv)
  {
    if (driv=="QMYSQL3")
      return "MySQL";
    else if (driv=="QPSQL7")
      return "PostgreSQL";
    else if (driv=="QTDS")
      return "Microsoft SQL/TDS";
    else if (driv=="QODBC3")
      return "ODBC";
    return QString::null;
  }
  static QString toQSqlName(const QString &driv)
  {
    if (driv=="MySQL")
      return "QMYSQL3";
    else if (driv=="PostgreSQL")
      return "QPSQL7";
    else if (driv=="Microsoft SQL/TDS")
      return "QTDS";
    else if (driv=="ODBC")
      return "QODBC3";
    return QString::null;
  }

  class qSqlSub : public toConnectionSub {
  public:
    toSemaphore Lock;
    QSqlDatabase *Connection;
    QString name;

    qSqlSub(QSqlDatabase *conn,const QString &name)
      : Lock(1),Connection(conn)
    { }
    ~qSqlSub()
    { QSqlDatabase::removeDatabase(name); }
    void throwError(void)
    { throw ErrorString(Connection->lastError()); }
  };

  class qSqlQuery : public toQuery::queryImpl {
    QSqlQuery *Query;
    QSqlRecord Record;
    qSqlSub *Connection;
    bool EOQ;
    unsigned int Column;
  public:
    qSqlQuery(toQuery *query,qSqlSub *conn)
      : toQuery::queryImpl(query),Connection(conn)
    { Column=0; EOQ=true; Column=0; Query=NULL; }
    virtual ~qSqlQuery()
    { delete Query; }
    virtual void execute(void);

    virtual void cancel(void)
    {
      // Not implemented
    }

    virtual toQValue readValue(void)
    {
      if (!Query)
	throw QString("Fetching from unexecuted query");
      if (EOQ)
	throw QString("Tried to read past end of query");

      Connection->Lock.down();
      QVariant ret=Query->value(Column);
      if (!ret.isValid()) {
	Connection->Lock.up();
	Connection->throwError();
      }
      Column++;
      if (Column==Record.count()) {
	Column=0;
	EOQ=!Query->next();
      }
      Connection->Lock.up();
      return ret.toString();
    }
    virtual bool eof(void)
    {
      return EOQ;
    }
    virtual int rowsProcessed(void)
    {
      if (!Query)
	return 0;
      Connection->Lock.down();
      int ret=Query->numRowsAffected();
      Connection->Lock.up();
      return ret;
    }
    virtual int columns(void)
    {
      Connection->Lock.down();
      int ret=Record.count();;
      Connection->Lock.up();
      return ret;
    }
    virtual std::list<toQuery::queryDescribe> describe(void)
    {
      Connection->Lock.down();
      std::list<toQuery::queryDescribe> ret;

      for (unsigned int i=0;i<Record.count();i++) {
	toQuery::queryDescribe desc;
	desc.AlignRight=false;
	desc.Name=Record.fieldName(i);
	desc.AlignRight=false;

	QSqlRecordInfo recInfo=Connection->Connection->recordInfo(*Query);
	QSqlFieldInfo info(recInfo.find(desc.Name));

	switch(info.type()) {
	default:
	  desc.Datatype="UNKNOWN";
	  break;
	case QVariant::Invalid:
	  desc.Datatype="INVALID";
	  break;
	case QVariant::List:
	  desc.Datatype="LIST";
	  break;
	case QVariant::Map:
	  desc.Datatype="MAP";
	  break;
	case QVariant::String:
#if 0
	  if (info.isTrim())
	    desc.Datatype="NCHAR";
	  else
	    desc.Datatype="NVARCHAR";
#else
	  if (info.isTrim())
	    desc.Datatype="CHAR";
	  else
	    desc.Datatype="VARCHAR";
#endif
	  break;
	case QVariant::StringList:
	  desc.Datatype="STRINGLIST";
	  break;
	case QVariant::Font:
	  desc.Datatype="FONT";
	  break;
	case QVariant::Pixmap:
	  desc.Datatype="PIXMAP";
	  break;
	case QVariant::Brush:
	  desc.Datatype="BRUSH";
	  break;
	case QVariant::Rect:
	  desc.Datatype="RECT";
	  break;
	case QVariant::Size:
	  desc.Datatype="SIZE";
	  break;
	case QVariant::Color:
	  desc.Datatype="COLOR";
	  break;
	case QVariant::Palette:
	  desc.Datatype="PALETTE";
	  break;
	case QVariant::ColorGroup:
	  desc.Datatype="COLORGROUP";
	  break;
	case QVariant::IconSet:
	  desc.Datatype="ICONSET";
	  break;
	case QVariant::Point:
	  desc.Datatype="POINT";
	  break;
	case QVariant::Image:
	  desc.Datatype="IMAGE";
	  break;
	case QVariant::Int:
	  desc.Datatype="INT";
	  desc.AlignRight=true;
	  break;
	case QVariant::UInt:       
	  desc.Datatype="UINT";
	  desc.AlignRight=true;
	  break;
	case QVariant::Bool:       
	  desc.Datatype="BOOL";
	  break;
	case QVariant::Double:     
	  desc.Datatype="DOUBLE";
	  desc.AlignRight=true;
	  break;
	case QVariant::CString:
	  if (info.isTrim())
	    desc.Datatype="CHAR";
	  else
	    desc.Datatype="VARCHAR";
	  break;
	case QVariant::PointArray: 
	  desc.Datatype="POINTARRAY";
	  break;
	case QVariant::Region:     
	  desc.Datatype="REGION";
	  break;
	case QVariant::Bitmap:     
	  desc.Datatype="BITMAP";
	  break;
	case QVariant::Cursor:     
	  desc.Datatype="CURSOR";
	  break;
	case QVariant::Date:
	  desc.Datatype="DATE";
	  break;
	case QVariant::Time:
	  desc.Datatype="TIME";
	  break;
	case QVariant::DateTime:   
	  desc.Datatype="DATETIME";
	  break;
	case QVariant::ByteArray:  
	  desc.Datatype="BLOB";
	  break;
	case QVariant::BitArray:
	  desc.Datatype="BITARRAY";
	  break;
	case QVariant::SizePolicy:
	  desc.Datatype="SIZEPOLICY";
	  break;
	case QVariant::KeySequence:
	  desc.Datatype="KEYSEQUENCE";
	  break;
	}

	if (info.length()>=0) {
	  desc.Datatype+=" (";
	  desc.Datatype+=QString::number(info.length());
	  if (info.precision()>0) {
	    desc.Datatype+=",";
	    desc.Datatype+=QString::number(info.precision());
	  }
	  desc.Datatype+=")";
	}
	desc.Null=!info.isRequired();

	ret.insert(ret.end(),desc);
      }
      Connection->Lock.up();
      return ret;
    }
  };

  class qSqlConnection : public toConnection::connectionImpl {
    qSqlSub *qSqlConv(toConnectionSub *sub)
    {
      qSqlSub *conn=dynamic_cast<qSqlSub *>(sub);
      if (!conn)
	throw QString("Internal error, not QSql sub connection");
      return conn;
    }
  public:
    qSqlConnection(toConnection *conn)
      : toConnection::connectionImpl(conn)
    { }

    virtual std::list<toConnection::objectName> objectNames(void)
    {
      std::list<toConnection::objectName> ret;

      toQuery tables(connection(),SQLListTables);
      toConnection::objectName cur;
      while(!tables.eof()) {
	cur.Name=tables.readValueNull();
	if (tables.columns()>1)
	  cur.Owner=tables.readValueNull();
	else
	  cur.Owner=connection().database();
	if (tables.columns()>2)
	  cur.Type=tables.readValueNull();
	else
	  cur.Type="TABLE";
	ret.insert(ret.end(),cur);
      }

      return ret;
    }

    virtual std::map<QString,toConnection::objectName> synonymMap(std::list<toConnection::objectName> &objects)
    {
      std::map<QString,toConnection::objectName> ret;

      // Need implementation.

      return ret;
    }

    virtual toQDescList columnDesc(const toConnection::objectName &table)
    {
      toBusy busy;

      std::map<QString,QString> comments;
#if 0 // Uncomment as soon as there are some SQLComments to use
      try {
	toQuery comment(connection(),SQLComment,table.Owner,table.Name);
	while(!comment.eof()) {
	  QString col=comment.readValue();
	  comments[col]=comment.readValueNull();
	}
      } catch (...) {
      }
#endif

      try {
	QString SQL="SELECT * FROM ";
	SQL+=quote(table.Owner);
	SQL+=".";
	SQL+=quote(table.Name);
	SQL+=" WHERE NULL=NULL";
	toQuery query(connection(),SQL);
	toQDescList desc=query.describe();
	for(toQDescList::iterator j=desc.begin();j!=desc.end();j++)
	  (*j).Comment=comments[(*j).Name];

	return desc;
      } catch(...) {
      }

      toQDescList ret;
      return ret;
    }

    virtual void commit(toConnectionSub *sub)
    {
      qSqlSub *conn=qSqlConv(sub);
      if (!conn->Connection->commit())
	conn->throwError();
    }
    virtual void rollback(toConnectionSub *sub)
    {
      qSqlSub *conn=qSqlConv(sub);
      if (!conn->Connection->rollback())
	conn->throwError();
    }

    virtual toConnectionSub *createConnection(void);

    void closeConnection(toConnectionSub *conn)
    {
      delete conn;
    }

    virtual QString version(toConnectionSub *sub)
    {
      qSqlSub *conn=qSqlConv(sub);

      conn->Lock.down();
      QSqlQuery query=conn->Connection->exec(toSQL::string(SQLVersion,connection()));
      QString ret;
      if (query.isValid()) {
	if (query.next()) {
	  QSqlRecord record=conn->Connection->record(query);
	  ret=query.value(record.count()-1).toString();
	}
      }
      conn->Lock.up();
      return ret;
    }

    virtual toQuery::queryImpl *createQuery(toQuery *query,toConnectionSub *sub)
    { return new qSqlQuery(query,qSqlConv(sub)); }
    virtual void execute(toConnectionSub *sub,const QCString &sql,toQList &params)
    {
      qSqlSub *conn=qSqlConv(sub);

      conn->Lock.down();
      QSqlQuery Query(conn->Connection->exec(QueryParam(sql,params)));
      if (!Query.isActive()) {
	conn->Lock.up();
	conn->throwError();
      }

      conn->Lock.up();
    }
  };

  toQSqlProvider(void)
    : toConnectionProvider("QSql",false)
  { }

  virtual void initialize(void)
  {
    QStringList lst=QSqlDatabase::drivers();
    for(unsigned int i=0;i<lst.count();i++) {
      QString t=fromQSqlName(lst[i]);
      if (!t.isNull())
	addProvider(t);
    }
  }

  ~toQSqlProvider()
  {
    QStringList lst=QSqlDatabase::drivers();
    for(unsigned int i=0;i<lst.count();i++) {
      QString t=fromQSqlName(lst[i]);
      if (!t.isNull())
	removeProvider(t);
    }
  }

  virtual toConnection::connectionImpl *provideConnection(const QString &prov,toConnection *conn)
  { return new qSqlConnection(conn); }
  virtual std::list<QString> providedHosts(const QString &)
  {
    std::list<QString> ret;
    ret.insert(ret.end(),"localhost");
    return ret;
  }
  virtual std::list<QString> providedDatabases(const QString &,const QString &host,const QString &,const QString &)
  {
    std::list<QString> ret;

    // Need implementation

    return ret;
  }
};

static toQSqlProvider QSqlProvider;

void toQSqlProvider::qSqlQuery::execute(void)
{
  while (Connection->Lock.getValue()>1) {
    Connection->Lock.down();
    toStatusMessage("Too high value on connection lock semaphore");
  }
  
  Connection->Lock.down();
  Query=new QSqlQuery(Connection->Connection->exec(QueryParam(query()->sql(),query()->params())));
  if (!Query->isActive()) {
    Connection->Lock.up();
    Connection->throwError();
  }
  
  Record=Connection->Connection->record(*Query);
  EOQ=!Query->next();
  Column=0;
  
  Connection->Lock.up();
}

toConnectionSub *toQSqlProvider::qSqlConnection::createConnection(void)
{
  static int ID=0;
  ID++;

  QString dbName=QString::number(ID);
  QSqlDatabase *db=QSqlDatabase::addDatabase(toQSqlName(connection().provider()),dbName);
  if (!db)
    throw QString("Couldn't create QSqlDatabase object");
  db->setHostName(connection().host());
  QString dbname=connection().database();
  int pos=dbname.find(":");
  if (pos<0)
    db->setDatabaseName(dbname);
  else {
    db->setDatabaseName(dbname.mid(0,pos));
    db->setPort(dbname.mid(pos+1).toInt());
  }
  db->open(connection().user(),connection().password());
  if (!db->isOpen()) {
    QString t=ErrorString(db->lastError());
    QSqlDatabase::removeDatabase(dbName);
    throw t;
  }

  return new qSqlSub(db,dbName);
}
