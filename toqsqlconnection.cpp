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

static toSQL SQLVersionSapDb("toQSqlConnection:Version",
			"select ltrim(substring(kernel,10,10)) from versions",
			"",
			"",
			"SapDB");

static toSQL SQLVersionPgSQL("toQSqlConnection:Version",
			     "SELECT SUBSTR(version(), STRPOS(version(), ' ') + 1, STRPOS(version(), 'on') - STRPOS(version(), ' ') - 2)",
			     "",
			     "7.1",
			     "PostgreSQL");

static toSQL SQLListObjects("toQSqlConnection:ListObjects",
			    "show tables",
			    "Get the available tables for a mysql connection",
			    "3.0",
			    "MySQL");

static toSQL SQLListObjectsSapDb("toQSqlConnection:ListObjects",
			    "select tablename \"Tablename\",\n"
			    "       owner     \"Owner\",\n"
			    "       tabletype \"Type\",\n"
			    "from tables \n"
			    "where tabletype not in (\'SYNONYM\',\'RESULT\') \n"
			    "order by tablename",
			    "",
			    "",
			    "SapDB");

static toSQL SQLListObjectsPgSQL("toQSqlConnection:ListObjects",
				 "SELECT c.relname AS \"Tablename\",\n"
				 "       u.usename AS \"Owner\",\n"
				 "       c.relkind AS \"Type\"\n"
				 "  FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
				 " ORDER BY \"Tablename\"",
				 "",
				 "7.1",
				 "PostgreSQL");

static toSQL SQLListSynonymsSapDb("toQSqlConnection:ListSynonyms",
			     "SELECT synonymname \"Synonym\", owner \"Schema\", tablename \"Object\"\n"
			    "from synonyms \n"
			    "where tabletype not in (\'SYNONYM\',\'RESULT\') \n"
			    "order by owner,tablename",
			     "Get synonym list, should have same columns",
			     "",
			     "SapDb");

static toSQL SQLListSynonyms("toQSqlConnection:ListSynonyms",
			     "SELECT c.relname AS \"Synonym\", u.usename AS \"Schema\", c.relname AS \"Object\"\n"
			     "  FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
			     " ORDER BY u.usename, c.relname",
			     "",
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
				 "",
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
    QChar rc=query.at(i);
    char c=rc.latin1();
    switch(c) {
    case '\\':
      ret+=rc;
      ret+=query.at(++i);
      break;
    case '\'':
      inString=!inString;
      ret+=rc;
      break;
    case ':':
      if (!inString) {
	QString nam;
	for (i++;i<query.length();i++) {
	  rc=query.at(i);
	  if (!rc.isLetterOrNumber())
	    break;
	  nam+=rc;
	}
	c=rc.latin1();
	QString in;
	if (c=='<') {
	  for (i++;i<query.length();i++) {
	    rc=query.at(i);
	    c=rc.latin1();
	    if (c=='>') {
	      i++;
	      break;
	    }
	    in+=rc;
	  }
	}
	i--;
	
	if (nam.isEmpty())
	  throw QString::fromLatin1("No bind name");

	if (binds.find(nam)!=binds.end()) {
	  ret+=binds[nam];
	  break;
	}
	if (cpar==params.end())
	  throw QString::fromLatin1("Not all bind variables suplied");
	QString str;
	if ((*cpar).isNull()) {
	  str=QString::fromLatin1("NULL");
	} else if ((*cpar).isInt()||(*cpar).isDouble()) {
	  str=QString(*cpar);
	} else {
	  if (in!=QString::fromLatin1("noquote"))
	    str+=QString::fromLatin1("'");
	  QString tmp=(*cpar);
	  for(unsigned int j=0;j<tmp.length();j++) {
	    QChar d=tmp.at(j);
	    switch(d.latin1()) {
	    case 0:
	      str+=QString::fromLatin1("\\0");
	      break;
	    case '\n':
	      str+=QString::fromLatin1("\\n");
	      break;
	    case '\t':
	      str+=QString::fromLatin1("\\t");
	      break;
	    case '\r':
	      str+=QString::fromLatin1("\\r");
	      break;
	    case '\'':
	      str+=QString::fromLatin1("\\\'");
	      break;
	    case '\"':
	      str+=QString::fromLatin1("\\\"");
	      break;
	    case '\\':
	      str+=QString::fromLatin1("\\\\");
	      break;
	    default:
	      str+=d;
	    }
	  }
	  if (in!=QString::fromLatin1("noquote"))
	    str+=QString::fromLatin1("'");
	}
	binds[nam]=str;
	ret+=str;
	cpar++;
	break;
      }
    default:
      ret+=rc;
    }
  }
  return ret;
}

static QString ErrorString(const QSqlError &err,const QString &sql=QString::null)
{
  QString ret;
  if (err.databaseText().isEmpty()) {
    if (err.driverText().isEmpty())
      ret=QString::fromLatin1("Unknown error");
    else
      ret=err.driverText();
  } else
    ret=err.databaseText();
  if (!sql.isEmpty())
    ret+=QString::fromLatin1("\n\n")+sql;
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

// MySQL datatypes (From mysql_com.h)

enum enum_field_types { FIELD_TYPE_DECIMAL, FIELD_TYPE_TINY,
                        FIELD_TYPE_SHORT,  FIELD_TYPE_LONG,
                        FIELD_TYPE_FLOAT,  FIELD_TYPE_DOUBLE,
                        FIELD_TYPE_NULL,   FIELD_TYPE_TIMESTAMP,
                        FIELD_TYPE_LONGLONG,FIELD_TYPE_INT24,
                        FIELD_TYPE_DATE,   FIELD_TYPE_TIME,
                        FIELD_TYPE_DATETIME, FIELD_TYPE_YEAR,
                        FIELD_TYPE_NEWDATE,
                        FIELD_TYPE_ENUM=247,
                        FIELD_TYPE_SET=248,
                        FIELD_TYPE_TINY_BLOB=249,
                        FIELD_TYPE_MEDIUM_BLOB=250,
                        FIELD_TYPE_LONG_BLOB=251,
                        FIELD_TYPE_BLOB=252,
                        FIELD_TYPE_VAR_STRING=253,
                        FIELD_TYPE_STRING=254
};

// SAPDb Datatypes (from /opt/sapdb/interfaces/odbc/incl/*.h)
#define SAP_SQL_UNKNOWN_TYPE    0
#define SAP_SQL_DECIMAL         3
#define SAP_SQL_NUMERIC         2
#define SAP_SQL_REAL            7
#define SAP_SQL_FLOAT           6
#define SAP_SQL_DOUBLE          8

#define SAP_SQL_INTEGER         4
#define SAP_SQL_SMALLINT        5
#define SAP_SQL_BIT            (-7) 
#define SAP_SQL_TINYINT        (-6)     
#define SAP_SQL_BIGINT         (-5)    

#define SAP_SQL_CHAR            1
#define SAP_SQL_VARCHAR        12
#define SAP_SQL_LONGVARCHAR    (-1) 

#define SAP_SQL_BINARY         (-2)
#define SAP_SQL_VARBINARY      (-3) 
#define SAP_SQL_LONGVARBINARY  (-4)      

#define SAP_SQL_DATE      	9
#define SAP_SQL_DATETIME      	9
#define SAP_SQL_TYPE_DATE      91

#define SAP_SQL_TIME     	10
#define SAP_SQL_TYPE_TIME       92

#define SAP_SQL_TIMESTAMP	11 
#define SAP_SQL_TYPE_TIMESTAMP  93



static std::list<toQuery::queryDescribe> Describe(const QCString &type,QSqlRecordInfo recInfo)
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
    if (type=="MySQL") {
      switch(info.typeID()) {
      case FIELD_TYPE_DECIMAL:
	desc.Datatype=QString::fromLatin1("DECIMAL");
	break;
      case FIELD_TYPE_TINY:
	desc.Datatype=QString::fromLatin1("TINY");
	break;
      case FIELD_TYPE_SHORT:
	desc.Datatype=QString::fromLatin1("SHORT");
	break;
      case FIELD_TYPE_LONG:
	desc.Datatype=QString::fromLatin1("LONG");
	break;
      case FIELD_TYPE_FLOAT:
	desc.Datatype=QString::fromLatin1("FLOAT");
	break;
      case FIELD_TYPE_DOUBLE:
	desc.Datatype=QString::fromLatin1("DOUBLE");
	break;
      case FIELD_TYPE_NULL:
	desc.Datatype=QString::fromLatin1("NULL");
	break;
      case FIELD_TYPE_TIMESTAMP:
	desc.Datatype=QString::fromLatin1("TIMESTAMP");
	break;
      case FIELD_TYPE_LONGLONG:
	desc.Datatype=QString::fromLatin1("LONGLONG");
	break;
      case FIELD_TYPE_INT24:
	desc.Datatype=QString::fromLatin1("INT23");
	break;
      case FIELD_TYPE_DATE:
	desc.Datatype=QString::fromLatin1("DATE");
	break;
      case FIELD_TYPE_TIME:
	desc.Datatype=QString::fromLatin1("TIME");
	break;
      case FIELD_TYPE_DATETIME:
	desc.Datatype=QString::fromLatin1("DATETIME");
	break;
      case FIELD_TYPE_YEAR:
	desc.Datatype=QString::fromLatin1("YEAR");
	break;
      case FIELD_TYPE_NEWDATE:
	desc.Datatype=QString::fromLatin1("NEWDATE");
	break;
      case FIELD_TYPE_ENUM:
	desc.Datatype=QString::fromLatin1("ENUM");
	break;
      case FIELD_TYPE_SET:
	desc.Datatype=QString::fromLatin1("SET");
	break;
      case FIELD_TYPE_TINY_BLOB:
	desc.Datatype=QString::fromLatin1("TINY_BLOB");
	break;
      case FIELD_TYPE_MEDIUM_BLOB:
	desc.Datatype=QString::fromLatin1("MEDIUM_BLOB");
	break;
      case FIELD_TYPE_LONG_BLOB:
	desc.Datatype=QString::fromLatin1("LONG_BLOB");
	break;
      case FIELD_TYPE_BLOB:
	desc.Datatype=QString::fromLatin1("BLOB");
	break;
      case FIELD_TYPE_VAR_STRING:
	desc.Datatype=QString::fromLatin1("VAR_STRING");
	break;
      case FIELD_TYPE_STRING:
	desc.Datatype=QString::fromLatin1("STRING");
	break;
      default:
	desc.Datatype=QString::fromLatin1("UNKNOWN");
	break;
      }
    } else if (type=="PostgreSQL") {
      switch(info.typeID()) {
      case BOOLOID:
	desc.Datatype=QString::fromLatin1("BOOL");
	break;
      case BYTEAOID:
	desc.Datatype=QString::fromLatin1("BYTEA");
	break;
      case CHAROID:
	desc.Datatype=QString::fromLatin1("CHAR");
	break;
      case NAMEOID:
	size=32;
	desc.Datatype=QString::fromLatin1("NAME");
	break;
      case INT8OID:
	size=8;
	desc.Datatype=QString::fromLatin1("INT8");
	break;
      case INT2OID:
	size=2;
	desc.Datatype=QString::fromLatin1("INT2");
	break;
      case INT2VECTOROID:
	size=2;
	desc.Datatype=QString::fromLatin1("INT2VECTOR");
	break;
      case INT4OID:
	size=4;
	desc.Datatype=QString::fromLatin1("INT4");
	break;
      case REGPROCOID:
	size=4;
	desc.Datatype=QString::fromLatin1("REGPROC");
	break;
      case TEXTOID:
	desc.Datatype=QString::fromLatin1("TEXT");
	break;
      case OIDOID:
	size=4;
	desc.Datatype=QString::fromLatin1("OID");
	break;
      case TIDOID:
	size=6;
	desc.Datatype=QString::fromLatin1("TID");
	break;
      case XIDOID:
	size=4;
	desc.Datatype=QString::fromLatin1("XID");
	break;
      case CIDOID:
	size=4;
	desc.Datatype=QString::fromLatin1("CID");
	break;
      case OIDVECTOROID:
	size=4;
	desc.Datatype=QString::fromLatin1("OIDVECTOR");
	break;
      case POINTOID:
	size=16;
	desc.Datatype=QString::fromLatin1("POINT");
	break;
      case LSEGOID:
	size=32;
	desc.Datatype=QString::fromLatin1("LSEG");
	break;
      case PATHOID:
	desc.Datatype=QString::fromLatin1("PATH");
	break;
      case BOXOID:
	size=32;
	desc.Datatype=QString::fromLatin1("BOX");
	break;
      case POLYGONOID:
	desc.Datatype=QString::fromLatin1("POLYGON");
	break;
      case LINEOID:
	size=32;
	desc.Datatype=QString::fromLatin1("LINE");
	break;
      case FLOAT4OID:
	size=4;
	desc.Datatype=QString::fromLatin1("FLOAT4");
	break;
      case FLOAT8OID:
	size=8;
	desc.Datatype=QString::fromLatin1("FLOAT8");
	break;
      case ABSTIMEOID:
	size=4;
	desc.Datatype=QString::fromLatin1("ABSTIME");
	break;
      case RELTIMEOID:
	size=4;
	desc.Datatype=QString::fromLatin1("RELTIME");
	break;
      case TINTERVALOID:
	size=12;
	desc.Datatype=QString::fromLatin1("TINTERVAL");
	break;
      case UNKNOWNOID:
	desc.Datatype=QString::fromLatin1("UNKNOWN");
	break;
      case CIRCLEOID:
	size=24;
	desc.Datatype=QString::fromLatin1("CIRCLE");
	break;
      case CASHOID:
	size=4;
	desc.Datatype=QString::fromLatin1("MONEY");
	break;
      case MACADDROID:
	size=6;
	desc.Datatype=QString::fromLatin1("MACADDR");
	break;
      case INETOID:
	desc.Datatype=QString::fromLatin1("INET");	
	break;
      case CIDROID:
	desc.Datatype=QString::fromLatin1("CIDR");
	break;
      case BPCHAROID:
	desc.Datatype=QString::fromLatin1("BPCHAR");
	break;
      case VARCHAROID:
	desc.Datatype=QString::fromLatin1("VARCHAR");
	break;
      case DATEOID:
	size=4;
	desc.Datatype=QString::fromLatin1("DATE");
	break;
      case TIMEOID:
	size=8;
	desc.Datatype=QString::fromLatin1("TIME");
	break;
      case TIMESTAMPOID:
	size=8;
	desc.Datatype=QString::fromLatin1("TIMESTAMP");
	break;
      case TIMESTAMPTZOID:
	size=8;
	desc.Datatype=QString::fromLatin1("TIMESTAMPTZ");
	break;
      case INTERVALOID:
	size=12;
	desc.Datatype=QString::fromLatin1("INTERVAL");
	break;
      case TIMETZOID:
	size=12;
	desc.Datatype=QString::fromLatin1("TIMETZ");
	break;
      case BITOID:
	desc.Datatype=QString::fromLatin1("BIT");
	break;
      case VARBITOID:
	desc.Datatype=QString::fromLatin1("VARBIT");
	break;
      case NUMERICOID:
	desc.Datatype=QString::fromLatin1("NUMERIC");
	break;
      case REFCURSOROID:
	desc.Datatype=QString::fromLatin1("REFCURSOR");
	break;
      default:
	desc.Datatype=QString::fromLatin1("UNKNOWN");
	break;
      }
    } else if (type=="SapDB") {
      switch(info.typeID()) {
      case SAP_SQL_UNKNOWN_TYPE:
	desc.Datatype=QString::fromLatin1("UNKNOWN");
	break;
      case SAP_SQL_BIT:
	desc.Datatype=QString::fromLatin1("BIT");
	break;
      case SAP_SQL_INTEGER:
	desc.Datatype=QString::fromLatin1("INTEGER");
	break;
      case SAP_SQL_SMALLINT:
	desc.Datatype=QString::fromLatin1("SMALLINT");
	break;
      case SAP_SQL_BIGINT:
	desc.Datatype=QString::fromLatin1("BIGINT");
	break;
      case SAP_SQL_TINYINT:
	desc.Datatype=QString::fromLatin1("TINYINT");
	break;
      case SAP_SQL_NUMERIC:
	desc.Datatype=QString::fromLatin1("NUMERIC");
	break;
      case SAP_SQL_DECIMAL:
	desc.Datatype=QString::fromLatin1("DECIMAL");
	break;
      case SAP_SQL_FLOAT:
	desc.Datatype=QString::fromLatin1("FLOAT");
	break;
      case SAP_SQL_REAL:
	desc.Datatype=QString::fromLatin1("REAL");
	break;
      case SAP_SQL_DOUBLE:
	desc.Datatype=QString::fromLatin1("DOUBLE");
	break;
      case SAP_SQL_DATE:
	desc.Datatype=QString::fromLatin1("DATE");
	break;
      case SAP_SQL_TYPE_DATE:
	desc.Datatype=QString::fromLatin1("DATE");
	break;
      case SAP_SQL_TIME:
	desc.Datatype=QString::fromLatin1("TIME");
	break;
      case SAP_SQL_TYPE_TIME:
	desc.Datatype=QString::fromLatin1("TIME");
	break;
      case SAP_SQL_TIMESTAMP:
	desc.Datatype=QString::fromLatin1("TIMESTAMP");
	break;
      case SAP_SQL_TYPE_TIMESTAMP:
	desc.Datatype=QString::fromLatin1("TIMESTAMP");
	break;
      case SAP_SQL_CHAR:
	desc.Datatype=QString::fromLatin1("CHAR");
	break;
      case SAP_SQL_VARCHAR:
	desc.Datatype=QString::fromLatin1("VARCHAR");
	break;
      case SAP_SQL_LONGVARCHAR:
	desc.Datatype=QString::fromLatin1("LONGVARCHAR");
	break;
      case SAP_SQL_BINARY:
	desc.Datatype=QString::fromLatin1("BINARY");
	break;
      case SAP_SQL_VARBINARY:
	desc.Datatype=QString::fromLatin1("BINARY");
	break;
      case SAP_SQL_LONGVARBINARY:
	desc.Datatype=QString::fromLatin1("LONGVARBINARY");
	break;
      default:
	desc.Datatype=QString::fromLatin1("UNKNOWN");
	break;
      }	
    } else {
      switch(info.type()) {
      default:
	desc.Datatype=QString::fromLatin1("UNKNOWN");
	break;
      case QVariant::Invalid:
	desc.Datatype=QString::fromLatin1("INVALID");
	break;
      case QVariant::List:
	desc.Datatype=QString::fromLatin1("LIST");
	break;
      case QVariant::Map:
	desc.Datatype=QString::fromLatin1("MAP");
	break;
      case QVariant::String:
	if (info.isTrim())
	  desc.Datatype=QString::fromLatin1("CHAR");
	else
	  desc.Datatype=QString::fromLatin1("VARCHAR");
	break;
      case QVariant::StringList:
	desc.Datatype=QString::fromLatin1("STRINGLIST");
	break;
      case QVariant::Font:
	desc.Datatype=QString::fromLatin1("FONT");
	break;
      case QVariant::Pixmap:
	desc.Datatype=QString::fromLatin1("PIXMAP");
	break;
      case QVariant::Brush:
	desc.Datatype=QString::fromLatin1("BRUSH");
	break;
      case QVariant::Rect:
	desc.Datatype=QString::fromLatin1("RECT");
	break;
      case QVariant::Size:
	desc.Datatype=QString::fromLatin1("SIZE");
	break;
      case QVariant::Color:
	desc.Datatype=QString::fromLatin1("COLOR");
	break;
      case QVariant::Palette:
	desc.Datatype=QString::fromLatin1("PALETTE");
	break;
      case QVariant::ColorGroup:
	desc.Datatype=QString::fromLatin1("COLORGROUP");
	break;
      case QVariant::IconSet:
	desc.Datatype=QString::fromLatin1("ICONSET");
	break;
      case QVariant::Point:
	desc.Datatype=QString::fromLatin1("POINT");
	break;
      case QVariant::Image:
	desc.Datatype=QString::fromLatin1("IMAGE");
	break;
      case QVariant::Int:
	desc.Datatype=QString::fromLatin1("INT");
	desc.AlignRight=true;
	break;
      case QVariant::UInt:       
	desc.Datatype=QString::fromLatin1("UINT");
	desc.AlignRight=true;
	break;
      case QVariant::Bool:       
	desc.Datatype=QString::fromLatin1("BOOL");
	break;
      case QVariant::Double:     
	desc.Datatype=QString::fromLatin1("DOUBLE");
	desc.AlignRight=true;
	break;
      case QVariant::CString:
	if (info.isTrim())
	  desc.Datatype=QString::fromLatin1("CHAR");
	else
	  desc.Datatype=QString::fromLatin1("VARCHAR");
	break;
      case QVariant::PointArray: 
	desc.Datatype=QString::fromLatin1("POINTARRAY");
	break;
      case QVariant::Region:     
	desc.Datatype=QString::fromLatin1("REGION");
	break;
      case QVariant::Bitmap:     
	desc.Datatype=QString::fromLatin1("BITMAP");
	break;
      case QVariant::Cursor:     
	desc.Datatype=QString::fromLatin1("CURSOR");
	break;
      case QVariant::Date:
	desc.Datatype=QString::fromLatin1("DATE");
	break;
      case QVariant::Time:
	desc.Datatype=QString::fromLatin1("TIME");
	break;
      case QVariant::DateTime:   
	desc.Datatype=QString::fromLatin1("DATETIME");
	break;
      case QVariant::ByteArray:  
	desc.Datatype=QString::fromLatin1("BLOB");
	break;
      case QVariant::BitArray:
	desc.Datatype=QString::fromLatin1("BITARRAY");
	break;
      case QVariant::SizePolicy:
	desc.Datatype=QString::fromLatin1("SIZEPOLICY");
	break;
      case QVariant::KeySequence:
	desc.Datatype=QString::fromLatin1("KEYSEQUENCE");
	break;
      }
    }

    if (info.length()>size) {
      desc.Datatype+=QString::fromLatin1(" (");
      if (info.length()%size==0)
	desc.Datatype+=QString::number(info.length()/size);
      else
	desc.Datatype+=QString::number(info.length());
      if (info.precision()>0) {
	desc.Datatype+=QString::fromLatin1(",");
	desc.Datatype+=QString::number(info.precision());
      }
      desc.Datatype+=QString::fromLatin1(")");
    }
    desc.Null=!info.isRequired();
    
    ret.insert(ret.end(),desc);
  }
  return ret;
}

class toQSqlProvider : public toConnectionProvider {
  QStringList Drivers;
public:
  static QCString fromQSqlName(const QString &driv)
  {
    if (driv==QString::fromLatin1("QMYSQL3"))
      return "MySQL";
    else if (driv==QString::fromLatin1("QPSQL7"))
      return "PostgreSQL";
    else if (driv==QString::fromLatin1("QTDS"))
      return "Microsoft SQL/TDS";
    else if (driv==QString::fromLatin1("QSAPDB7"))
      return "SapDB";
    else if (driv==QString::fromLatin1("QODBC3"))
      return "ODBC";
    return "";
  }
  static QString toQSqlName(const QCString &driv)
  {
    if (driv=="MySQL")
      return QString::fromLatin1("QMYSQL3");
    else if (driv=="PostgreSQL")
      return QString::fromLatin1("QPSQL7");
    else if (driv=="Microsoft SQL/TDS")
      return QString::fromLatin1("QTDS");
    else if (driv=="SapDB")
      return QString::fromLatin1("QSAPDB7");
    else if (driv=="ODBC")
      return QString::fromLatin1("QODBC3");
    return QString::null;
  }
 

  class qSqlSub : public toConnectionSub {
    toSemaphore Lock;
  public:
    QSqlDatabase *Connection;
    QString name;

    qSqlSub(QSqlDatabase *conn,const QString &)
      : Lock(1),Connection(conn)
    { }

    void lockUp() {
	Lock.up();
    }

    void lockDown () {
	Lock.down();
    }

    int getLockValue() {
	return Lock.getValue();
    } 

    ~qSqlSub()
    { QSqlDatabase::removeDatabase(name); }
    void throwError(const QString &sql)
    { throw ErrorString(Connection->lastError(),QString::fromUtf8(sql)); }
  };

  class qSqlQuery : public toQuery::queryImpl {
    QSqlQuery *Query;
    QSqlRecord Record;
    qSqlSub *Connection;
    bool EOQ;
    bool cancelSupported;
    bool validCheck;
    unsigned int Column;
  public:
    qSqlQuery(toQuery *query,qSqlSub *conn)
      : toQuery::queryImpl(query),Connection(conn)
    { Column=0; 
      EOQ=true; 
      Query=NULL;
      // sapdb marks value as invalid on some views	
      // for example tables,indexes etc
      if(query->connection().provider() == "SapDB")  {
	validCheck =false;
        cancelSupported=true;
      }else {
	validCheck =true;
	cancelSupported=false;
      }
    }
    virtual ~qSqlQuery()
    { delete Query; }
    virtual void execute(void);

    virtual void cancel(void)
    {
	if (cancelSupported) {
        }
    }

    virtual toQValue readValue(void)
    {
      if (!Query)
	throw QString::fromLatin1("Fetching from unexecuted query");
      if (EOQ)
	throw QString::fromLatin1("Tried to read past end of query");

      Connection->lockDown();
      QVariant val=Query->value(Column);
      // sapdb marks value as invalid on some views	
      // for example tables,indexes etc, so ignore this check
      if (validCheck && !val.isValid()) {
	Connection->lockUp();
    	QString msg = QString::fromLatin1("Query Value not valid <");
	msg +=Column;
	msg +=QString::fromLatin1("> ");
	msg +=query()->sql();
	Connection->throwError(msg);
      }
      Column++;
      if (Column==Record.count()) {
	Column=0;
	EOQ=!Query->next();
      }
      Connection->lockUp();

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
      Connection->lockDown();
      int ret=Query->numRowsAffected();
      Connection->lockUp();
      return ret;
    }
    virtual int columns(void)
    {
      Connection->lockDown();
      int ret=Record.count();;
      Connection->lockUp();
      return ret;
    }
    virtual std::list<toQuery::queryDescribe> describe(void)
    {
      QCString provider=query()->connection().provider();
      Connection->lockDown();
      QSqlRecordInfo recInfo=Connection->Connection->recordInfo(*Query);
      std::list<toQuery::queryDescribe> ret=Describe(provider,recInfo);
      Connection->lockUp();
      return ret;
    }
  };

  class qSqlConnection : public toConnection::connectionImpl {
    bool multiple;
    qSqlSub *qSqlConv(toConnectionSub *sub)
    {
      qSqlSub *conn=dynamic_cast<qSqlSub *>(sub);
      if (!conn)
	throw QString::fromLatin1("Internal error, not QSql sub connection");
      return conn;
    }
  public:
    qSqlConnection(toConnection *conn)
      : toConnection::connectionImpl(conn)
    { 
	if (conn->provider() == "SapDB")
		multiple=false;
	else 
		multiple=true;
    }

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
	  cur.Type=QString::fromLatin1("TABLE");
	ret.insert(ret.end(),cur);
      }

      return ret;
    }

    virtual std::map<QString,toConnection::objectName> synonymMap(std::list<toConnection::objectName> &objects)
    {
      std::map<QString,toConnection::objectName> ret;

      try {
	toConnection::objectName cur;
	cur.Type=QString::fromLatin1("A");

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
	  if (cur.Name==(*i).Name&&cur.Owner==(*i).Owner) {
	    ret[synonym]=(*i);
            (*i).Synonyms.insert((*i).Synonyms.end(),synonym);
          }
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
	    sub->lockDown();
	    desc=Describe(connection().provider(),sub->Connection->recordInfo(quote(table.Name)));
	    sub->lockUp();
	  }
	} else {
	  QString SQL=QString::fromLatin1("SELECT * FROM ");
	  SQL+=quote(table.Owner);
	  SQL+=QString::fromLatin1(".");
	  SQL+=quote(table.Name);
	  SQL+=QString::fromLatin1(" WHERE NULL=NULL");
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

    virtual bool handleMultipleQueries() { 
	return multiple; 
    }

    virtual void commit(toConnectionSub *sub)
    {
      qSqlSub *conn=qSqlConv(sub);
      if (!conn->Connection->commit())
	conn->throwError(QString::fromLatin1("COMMIT"));
    }
    virtual void rollback(toConnectionSub *sub)
    {
      qSqlSub *conn=qSqlConv(sub);
      if (!conn->Connection->rollback())
	conn->throwError(QString::fromLatin1("ROLLBACK"));
    }

    virtual toConnectionSub *createConnection(void);

    void closeConnection(toConnectionSub *conn)
    {
      delete conn;
    }

    virtual QCString version(toConnectionSub *sub)
    {
      QCString ret;
      qSqlSub *conn=qSqlConv(sub);
      conn->lockDown();
      try {
	QSqlQuery query=conn->Connection->exec(toSQL::string(SQLVersion,connection()));
	if (query.next()) {
	  if (query.isValid()) {
	    QSqlRecord record=conn->Connection->record(query);
	    QVariant val=query.value(record.count()-1);
	    ret=val.toString().latin1();
	  }
	}
      } catch(...) {
      }
      conn->lockUp();
      return ret;
    }

    virtual toQuery::queryImpl *createQuery(toQuery *query,toConnectionSub *sub)
    { return new qSqlQuery(query,qSqlConv(sub)); }
    virtual void execute(toConnectionSub *sub,const QCString &sql,toQList &params)
    {
      qSqlSub *conn=qSqlConv(sub);
      conn->lockDown();
      QSqlQuery Query(conn->Connection->exec(QueryParam(sql,params)));
      if (!Query.isActive()) {
	conn->lockUp();
	QString msg = QString::fromLatin1("Query not active ");
	msg +=sql;
	conn->throwError(msg);
      }

      conn->lockUp();
    }
  };

  toQSqlProvider(void)
    : toConnectionProvider("QSql",false)
  { }

  virtual void initialize(void)
  {
    Drivers=QSqlDatabase::drivers();
    for(unsigned int i=0;i<Drivers.count();i++) {
      QCString t=fromQSqlName(Drivers[i]);
      if (!t.isEmpty())
	addProvider(t);
    }
  }

  ~toQSqlProvider()
  {
    for(unsigned int i=0;i<Drivers.count();i++) {
      QCString t=fromQSqlName(Drivers[i]);
      if (!t.isEmpty())
	removeProvider(t);
    }
  }

  virtual toConnection::connectionImpl *provideConnection(const QCString &,toConnection *conn)
  { return new qSqlConnection(conn); }
  virtual std::list<QString> providedHosts(const QCString &)
  {
    std::list<QString> ret;
    ret.insert(ret.end(),QString::fromLatin1("localhost"));
    return ret;
  }
  virtual std::list<QString> providedDatabases(const QCString &,const QString &,const QString &,const QString &)
  {
    std::list<QString> ret;

    // Need implementation

    return ret;
  }
};

static toQSqlProvider QSqlProvider;
toLock myLock;

void toQSqlProvider::qSqlQuery::execute(void)
{
  while (Connection->getLockValue()>1) {
    Connection->lockDown();
    toStatusMessage(QString::fromLatin1("Too high value on connection lock semaphore"));
  }
  Connection->lockDown(); 
  Query=new QSqlQuery(Connection->Connection->exec(QueryParam(query()->sql(),query()->params())));
  if (!Query->isActive()) {
    Connection->lockUp();
    QString msg = QString::fromLatin1("Query not active ");
    msg +=query()->sql();
    Connection->throwError(msg);
  }
  
  if (Query->isSelect()) {
    Record=Connection->Connection->record(*Query);
    EOQ=!Query->next();
    Column=0;
  } else
    EOQ=true;
  
  Connection->lockUp();
}

toConnectionSub *toQSqlProvider::qSqlConnection::createConnection(void)
{
  static int ID=0;
  ID++;

  QString dbName=QString::number(ID);
  QSqlDatabase *db=QSqlDatabase::addDatabase(toQSqlName(connection().provider()),dbName);
  if (!db)
    throw QString(QString::fromLatin1("Couldn't create QSqlDatabase object"));
  db->setDatabaseName(connection().database());
  QString host=connection().host();
  int pos=host.find(QString::fromLatin1(":"));
  if (pos<0)
    db->setHostName(host);
  else {
    db->setHostName(host.mid(0,pos));
    db->setPort(host.mid(pos+1).toInt());
  }
  db->open(connection().user(),connection().password());
  if (!db->isOpen()) {
    QString t=ErrorString(db->lastError());
    QSqlDatabase::removeDatabase(dbName);
    throw t;
  }

  return new qSqlSub(db,dbName);
}
