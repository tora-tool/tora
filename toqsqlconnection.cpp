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

static toSQL SQLVersionPgSQL("toQSqlConnection:Version",
			     "SELECT SUBSTR(version(), STRPOS(version(), ' ') + 1, STRPOS(version(), 'on') - STRPOS(version(), ' ') - 2)",
			     QString::null,
			     "7.1",
			     "PostgreSQL");

static toSQL SQLListObjects("toQSqlConnection:ListObjects",
			    "show tables",
			    "Get the available tables for a mysql connection",
			    "3.0",
			    "MySQL");

static toSQL SQLListObjectsPgSQL("toQSqlConnection:ListObjects",
				 "SELECT c.relname AS \"Tablename\",\n"
				 "       u.usename AS \"Owner\",\n"
				 "       c.relkind AS \"Type\"\n"
				 "  FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
				 " ORDER BY \"Tablename\"",
				 QString::null,
				 "7.1",
				 "PostgreSQL");

static toSQL SQLListSynonyms("toQSqlConnection:ListSynonyms",
			     "SELECT c.relname AS \"Synonym\", u.usename AS \"Schema\", c.relname AS \"Object\"\n"
			     "  FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
			     " ORDER BY u.usename, c.relname",
			     "Get synonym list, should have same columns",
			     "7.1",
			     "PostgreSQL");

static toSQL SQLColumnComments("toQSqlConnection:ColumnComments",
			       "select a.attname,b.description\n"
			       "from\n"
			       "  pg_attribute a,\n"
			       "  pg_description b,\n"
			       "  pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
			       "where\n"
			       "  a.oid=b.objoid\n"
			       "  and c.oid=a.attrelid\n"
			       "  and (u.usename = :owner OR u.usesysid IS NULL)\n"
			       "  and c.relname=:table",
			       "Get the available comments on columns of a table, "
			       "must have same binds and columns",
			       "7.1",
			       "PostgreSQL");

static toSQL SQLColumnComments72("toQSqlConnection:ColumnComments",
				 "select a.attname,b.description\n"
				 "from\n"
				 "  pg_attribute a,\n"
				 "  pg_description b,\n"
				 "  pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
				 "where\n"
				 "  a.attnum=b.objsubid\n"
				 "  and b.objoid=a.attrelid\n"
				 "  and c.oid=a.attrelid\n"
				 "  and (u.usename = :owner OR u.usesysid IS NULL)\n"
				 "  and c.relname=:table",
				 QString::null,
				 "7.2",
				 "PostgreSQL");

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

static QString ErrorString(const QSqlError &err,const QString &sql=QString::null)
{
  QString ret;
  if (err.databaseText().isEmpty()) {
    if (err.driverText().isEmpty())
      ret="Unknown error";
    else
      ret=err.driverText();
  } else
    ret=err.databaseText();
  if (!sql.isEmpty())
    ret+="\n\n"+sql;
  return ret;
}

// PostgreSQL datatypes (From pg_type.h)

#define BOOLOID                 16
#define BYTEAOID                17
#define CHAROID                 18
#define NAMEOID                 19
#define INT8OID                 20
#define INT2OID                 21
#define INT2VECTOROID   	22
#define INT4OID                 23
#define REGPROCOID              24
#define TEXTOID                 25
#define OIDOID                  26
#define TIDOID          	27
#define XIDOID 			28
#define CIDOID 			29
#define OIDVECTOROID    	30
#define POINTOID                600
#define LSEGOID                 601
#define PATHOID                 602
#define BOXOID                  603
#define POLYGONOID              604
#define LINEOID                 628
#define FLOAT4OID 		700
#define FLOAT8OID 		701
#define ABSTIMEOID              702
#define RELTIMEOID              703
#define TINTERVALOID    	704
#define UNKNOWNOID              705
#define CIRCLEOID               718
#define CASHOID 		790
#define MACADDROID 		829
#define INETOID 		869
#define CIDROID 		650
#define BPCHAROID               1042
#define VARCHAROID              1043
#define DATEOID                 1082
#define TIMEOID                 1083
#define TIMESTAMPOID    	1114
#define TIMESTAMPTZOID  	1184
#define INTERVALOID             1186
#define TIMETZOID               1266
#define BITOID   		1560
#define VARBITOID       	1562
#define NUMERICOID              1700
#define REFCURSOROID    	1790

static std::list<toQuery::queryDescribe> Describe(const QString &type,QSqlRecordInfo recInfo)
{
  std::list<toQuery::queryDescribe> ret;
  QSqlRecord record=recInfo.toRecord();
  unsigned int count=record.count();
  for (unsigned int i=0;i<count;i++) {
    toQuery::queryDescribe desc;
    desc.AlignRight=false;
    desc.Name=record.fieldName(i);
    desc.AlignRight=false;

    int size=1;

    QSqlFieldInfo info(recInfo.find(desc.Name));
    if (type=="PostgreSQL") {
      switch(info.typeID()) {
      case BOOLOID:
	desc.Datatype="BOOL";
	break;
      case BYTEAOID:
	desc.Datatype="BYTEA";
	break;
      case CHAROID:
	desc.Datatype="CHAR";
	break;
      case NAMEOID:
	size=32;
	desc.Datatype="NAME";
	break;
      case INT8OID:
	size=8;
	desc.Datatype="INT8";
	break;
      case INT2OID:
	size=2;
	desc.Datatype="INT2";
	break;
      case INT2VECTOROID:
	size=2;
	desc.Datatype="INT2VECTOR";
	break;
      case INT4OID:
	size=4;
	desc.Datatype="INT4";
	break;
      case REGPROCOID:
	size=4;
	desc.Datatype="REGPROC";
	break;
      case TEXTOID:
	desc.Datatype="TEXT";
	break;
      case OIDOID:
	size=4;
	desc.Datatype="OID";
	break;
      case TIDOID:
	size=6;
	desc.Datatype="TID";
	break;
      case XIDOID:
	size=4;
	desc.Datatype="XID";
	break;
      case CIDOID:
	size=4;
	desc.Datatype="CID";
	break;
      case OIDVECTOROID:
	size=4;
	desc.Datatype="OIDVECTOR";
	break;
      case POINTOID:
	size=16;
	desc.Datatype="POINT";
	break;
      case LSEGOID:
	size=32;
	desc.Datatype="LSEG";
	break;
      case PATHOID:
	desc.Datatype="PATH";
	break;
      case BOXOID:
	size=32;
	desc.Datatype="BOX";
	break;
      case POLYGONOID:
	desc.Datatype="POLYGON";
	break;
      case LINEOID:
	size=32;
	desc.Datatype="LINE";
	break;
      case FLOAT4OID:
	size=4;
	desc.Datatype="FLOAT4";
	break;
      case FLOAT8OID:
	size=8;
	desc.Datatype="FLOAT8";
	break;
      case ABSTIMEOID:
	size=4;
	desc.Datatype="ABSTIME";
	break;
      case RELTIMEOID:
	size=4;
	desc.Datatype="RELTIME";
	break;
      case TINTERVALOID:
	size=12;
	desc.Datatype="TINTERVAL";
	break;
      case UNKNOWNOID:
	desc.Datatype="UNKNOWN";
	break;
      case CIRCLEOID:
	size=24;
	desc.Datatype="CIRCLE";
	break;
      case CASHOID:
	size=4;
	desc.Datatype="MONEY";
	break;
      case MACADDROID:
	size=6;
	desc.Datatype="MACADDR";
	break;
      case INETOID:
	desc.Datatype="INET";	
	break;
      case CIDROID:
	desc.Datatype="CIDR";
	break;
      case BPCHAROID:
	desc.Datatype="BPCHAR";
	break;
      case VARCHAROID:
	desc.Datatype="VARCHAR";
	break;
      case DATEOID:
	size=4;
	desc.Datatype="DATE";
	break;
      case TIMEOID:
	size=8;
	desc.Datatype="TIME";
	break;
      case TIMESTAMPOID:
	size=8;
	desc.Datatype="TIMESTAMP";
	break;
      case TIMESTAMPTZOID:
	size=8;
	desc.Datatype="TIMESTAMPTZ";
	break;
      case INTERVALOID:
	size=12;
	desc.Datatype="INTERVAL";
	break;
      case TIMETZOID:
	size=12;
	desc.Datatype="TIMETZ";
	break;
      case BITOID:
	desc.Datatype="BIT";
	break;
      case VARBITOID:
	desc.Datatype="VARBIT";
	break;
      case NUMERICOID:
	desc.Datatype="NUMERIC";
	break;
      case REFCURSOROID:
	desc.Datatype="REFCURSOR";
	break;
      default:
	desc.Datatype="UNKNOWN";
	break;
      }
    } else {
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
	if (info.isTrim())
	  desc.Datatype="CHAR";
	else
	  desc.Datatype="VARCHAR";
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
    }

    if (info.length()>size) {
      desc.Datatype+=" (";
      if (info.length()%size==0)
	desc.Datatype+=QString::number(info.length()/size);
      else
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
    void throwError(const QCString &sql)
    { throw ErrorString(Connection->lastError(),QString::fromUtf8(sql)); }
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
      QVariant val=Query->value(Column);
      if (!val.isValid()) {
	Connection->Lock.up();
	Connection->throwError(query()->sql());
      }
      Column++;
      if (Column==Record.count()) {
	Column=0;
	EOQ=!Query->next();
      }
      Connection->Lock.up();

      return val.toString();
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
      QString provider=query()->connection().provider();
      Connection->Lock.down();
      QSqlRecordInfo recInfo=Connection->Connection->recordInfo(*Query);
      std::list<toQuery::queryDescribe> ret=Describe(provider,recInfo);
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

      toQuery tables(connection(),SQLListObjects);
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

      try {
	toConnection::objectName cur;
	cur.Type="A";

	toQuery synonyms(connection(),SQLListSynonyms);
	std::list<toConnection::objectName>::iterator i=objects.begin();
	while(!synonyms.eof()) {
	  QString synonym=synonyms.readValueNull();
	  cur.Owner=synonyms.readValueNull();
	  cur.Name=synonyms.readValueNull();
	  while(i!=objects.end()&&(*i)<cur)
	    i++;
	  if (i==objects.end())
	    break;
	  if (cur.Name==(*i).Name&&cur.Owner==(*i).Owner)
	    ret[synonym]=(*i);
	}
      } catch(...) {
      }

      return ret;
    }

    virtual toQDescList columnDesc(const toConnection::objectName &table)
    {
      toBusy busy;

      std::map<QString,QString> comments;
      try {
	toQuery comment(connection(),SQLColumnComments,table.Owner,table.Name);
	while(!comment.eof()) {
	  QString col=comment.readValue();
	  comments[col]=comment.readValueNull();
	}
      } catch (...) {
      }

      try {
	toQDescList desc;
	if (connection().provider()=="PostgreSQL") {
	  toQuery query(connection(),toQuery::Normal);
	  qSqlSub *sub=dynamic_cast<qSqlSub *>(query.connectionSub());
	  if (sub) {
	    sub->Lock.down();
	    desc=Describe(connection().provider(),sub->Connection->recordInfo(quote(table.Name)));
	    sub->Lock.up();
	  }
	} else {
	  QString SQL="SELECT * FROM ";
	  SQL+=quote(table.Owner);
	  SQL+=".";
	  SQL+=quote(table.Name);
	  SQL+=" WHERE NULL=NULL";
	  toQuery query(connection(),SQL);
	  desc=query.describe();
	}
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
	conn->throwError("COMMIT");
    }
    virtual void rollback(toConnectionSub *sub)
    {
      qSqlSub *conn=qSqlConv(sub);
      if (!conn->Connection->rollback())
	conn->throwError("ROLLBACK");
    }

    virtual toConnectionSub *createConnection(void);

    void closeConnection(toConnectionSub *conn)
    {
      delete conn;
    }

    virtual QString version(toConnectionSub *sub)
    {
      QString ret;
      qSqlSub *conn=qSqlConv(sub);
      conn->Lock.down();
      try {
	QSqlQuery query=conn->Connection->exec(toSQL::string(SQLVersion,connection()));
	if (query.next()) {
	  if (query.isValid()) {
	    QSqlRecord record=conn->Connection->record(query);
	    QVariant val=query.value(record.count()-1);
	    ret=val.toString();
	  }
	}
      } catch(...) {
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
	conn->throwError(sql);
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
    Connection->throwError(query()->sql());
  }
  
  if (Query->isSelect()) {
    Record=Connection->Connection->record(*Query);
    EOQ=!Query->next();
    Column=0;
  } else
    EOQ=true;
  
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
