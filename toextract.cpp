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
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#ifndef WIN32
#include <unistd.h>
#else
#include <winsock.h>
#endif

#include <qapplication.h>
#include <qdatetime.h>
#include <qprogressdialog.h>
#include <qregexp.h>
#include <qstringlist.h>

#include "toextract.h"
#include "tosql.h"
#include "tomain.h"
#include "toconf.h"

static toSQL SQLSetSizing("toExtract:SetSizing",
			  "SELECT block_size\n"
			  "  FROM (SELECT bytes/blocks/1024   AS block_size\n"
			  "          FROM user_segments\n"
			  "         WHERE bytes  IS NOT NULL\n"
			  "           AND blocks IS NOT NULL\n"
			  "         UNION\n"
			  "        SELECT bytes/blocks/1024   AS block_size\n"
			  "          FROM user_free_space\n"
			  "         WHERE bytes  IS NOT NULL\n"
			  "           AND blocks IS NOT NULL)\n"
			  " WHERE rownum < 2",
			  "Get information about block sizes, same columns");

toExtract::toExtract(toConnection &conn,QWidget *parent)
  : Connection(conn),Parent(parent)
{
  Heading=true;
  Prompt=true;
  Constraints=true;
  Indexes=true;
  Grants=true;
  Storage=true;
  Parallel=true;
  Contents=true;
  Comments=true;

  Schema="1";

  otl_stream inf(1,SQLSetSizing(Connection),Connection.connection());
  inf>>BlockSize;
  setSizes();
}

void toExtract::clearFlags(void)
{
  IsASnapIndex=false;
  IsASnapTable=false;
  Describe=false;
}

void toExtract::addDescription(list<QString> &ret,list<QString> &ctx,
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

  list<QString> args=ctx;
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

  QString str;
  bool first=true;
  for(list<QString>::iterator i=args.begin();i!=args.end();i++) {
    if (first)
      first=false;
    else
      str+="\01";
    if (!(*i).isEmpty())
      str+=*i;
  }
  ret.insert(ret.end(),str);
}

QString toExtract::reContext(list<QString> &ctx,int strip,const QString &str)
{
  QStringList lst=QStringList::split("\01",str);
  QString ret;
  QString sep="";
  for(list<QString>::iterator i=ctx.begin();i!=ctx.end();i++) {
    ret+=sep;
    ret+=*i;
    if (sep.isEmpty())
      sep="\01";
  }
  for(QStringList::Iterator j=lst.begin();j!=lst.end();j++) {
    if (strip>0) {
      ret+=sep;
      ret+=*j;
    } else
      strip--;
    if (sep.isEmpty())
      sep="\01";
  }
  return ret;
}

void toExtract::setSizes(void)
{
  Initial.clear();
  Next.clear();
  Limit.clear();
  if (Resize=="1") {
    for (int i=1;i<10000;i*=10) {
      QString str;
      if (i<1000) {
	str=QString::number(40*i+1);
	str+=" K";
      } else
	str="UNLIMITED";
      toPush(Initial,str);
      str=QString::number(5*BlockSize*i);
      str+=" K";
      toPush(Next,str);
      toPush(Limit,str);
    }
  } else if (!Resize.isEmpty()) {
    QStringList lst=QStringList::split(":",Resize);
    if (lst.count()%3!=0)
      throw QString("Malformed resize string (Should contain multiple of 3 number of ':')");
    for(unsigned int i=0;i<lst.count();i+=3) {
      if (i+3<lst.count())
	toPush(Limit,lst[i]);
      else
	toPush(Limit,"UNLIMITED");
      toPush(Initial,lst[i+1]);
      toPush(Next,lst[i+2]);
    }
  }
}

static toSQL SQLObjectExists("toExtract:ObjectExists",
			     "SELECT\n"
			     "       'X'\n"
			     "  FROM all_objects\n"
			     " WHERE object_name = :nam<char[100]>\n"
			     "   AND owner = :own<char[100]>\n"
			     "   AND object_type = :typ<char[100]>",
			     "Check if object exists, must have same binds.");

void toExtract::objectExists(const QString &owner,const QString &name,const QString &type)
{
  otl_stream str(1,
		 SQLObjectExists(Connection),
		 Connection.connection());
  if(str.eof())
    throw QString("Object %1.%2 didn't exist").arg(owner.lower()).arg(name.lower());
}

QString toExtract::compile(const QString &schema,const QString &owner,const QString &name,
			   const QString &type)
{
  QString type1=(type=="PACKAGE BODY")?QString("PACKAGE"):type;
  QString type2=(type=="PACKAGE BODY")?QString(" BODY"):
    (type=="PACKAGE")?QString(" PACKAGE"):QString::null;

  objectExists(owner,name,type);

  QString sql="ALTER ";
  sql+=type1;
  sql+=schema.lower();
  sql+=name.lower();
  sql+=" COMPILE";
  sql+=type2;

  QString ret;
  if (Prompt) {
    ret+="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";

  return ret;
}

QString toExtract::compilePackage(const QString &schema,const QString &owner,const QString &name,
				  const QString &type)
{
  QString ret=compile(schema,owner,name,"PACKAGE");
  try {
    ret+=compile(schema,owner,name,"PACKAGE BODY");
  } catch (...) {
  }
  return ret;
}

static toSQL SQLConstraintCols("toExtract:ConstraintCols",
			       "SELECT LOWER(column_name)\n"
			       "  FROM all_cons_columns\n"
			       " WHERE owner = :own<char[100]>\n"
			       "   AND constraint_name = :con<char[100]>\n"
			       " ORDER BY position",
			       "List columns in a constraint, must have same binds and columns");

QString toExtract::constraintColumns(const QString &owner,const QString &name)
{
  otl_stream str(1,
		 SQLConstraintCols(Connection),
		 Connection.connection());
  str<<owner.utf8();
  str<<name.utf8();

  QString ret="(\n    ";
  bool first=true;
  while(!str.eof()) {
    char buffer[100];
    str>>buffer;
    if (first)
      first=false;
    else
      ret+=",\n    ";
    ret+=QString::fromUtf8(buffer);
  }
  ret+="\n)\n";
  return ret;
}

static toSQL SQLTableComments("toExtract:TableComment",
			      "SELECT comments\n"
			      "  FROM all_tab_comments\n"
			      " WHERE table_name = :nam<char[100]>\n"
			      "   AND comments IS NOT NULL\n"
			      "   AND owner = :own<char[100]>",
			      "Extract comments about a table, must have same columns and binds");
static toSQL SQLColumnComments("toExtract:ColumnComment",
			       "SELECT column_name,\n"
			       "       comments\n"
			       "  FROM all_col_comments\n"
			       " WHERE table_name = :nam<char[100]>\n"
			       "   AND comments IS NOT NULL\n"
			       "   AND owner = :own<char[100]>",
			       "Extract comments about a columns, must have same columns and binds");

QString toExtract::createComments(const QString &schema,
				  const QString &owner,
				  const QString &name)
{
  QString ret;
  if (Comments) {
    QString sql;
    otl_stream inf(1,
		   SQLTableComments(Connection),
		   Connection.connection());
    inf<<name.utf8();
    inf<<owner.utf8();
    while(!inf.eof()) {
      char buffer[10000];
      inf>>buffer;
      sql=QString("COMMENT ON TABLE %1%2 IS '%3'").
	arg(schema.lower()).
	arg(name.lower()).
	arg(prepareDB(QString::fromUtf8(buffer)));
      if (Prompt) {
	ret+="PROMPT ";
	ret+=sql;
	ret+="\n\n";
      }
      ret+=sql;
      ret+=";\n\n";
    }
    otl_stream col(1,
		   SQLColumnComments(Connection),
		   Connection.connection());
    col<<name.utf8();
    col<<owner.utf8();
    while(!col.eof()) {
      char buffer[10000];
      col>>buffer;
      QString column=QString::fromUtf8(buffer);
      col>>buffer;
      sql=QString("COMMENT ON COLUMN %1%2.%3 IS '%4'").
	arg(schema.lower()).
	arg(name.lower()).
	arg(column.lower()).
	arg(prepareDB(QString::fromUtf8(buffer)));
      if (Prompt) {
	ret+="PROMPT ";
	ret+=sql;
	ret+="\n\n";
      }
      ret+=sql;
      ret+=";\n\n";
    }
  }
  return ret;
}

void toExtract::describeComments(list<QString> &lst,list<QString> &ctx,
				 const QString &schema,const QString &owner,const QString &name)
{
  if (Comments) {
    otl_stream inf(1,
		   SQLTableComments(Connection),
		   Connection.connection());
    inf<<name.utf8();
    inf<<owner.utf8();
    while(!inf.eof()) {
      char buffer[10000];
      inf>>buffer;
      addDescription(lst,ctx,"COMMENT",QString::fromUtf8(buffer));
    }
    otl_stream col(1,
		   SQLColumnComments(Connection),
		   Connection.connection());
    col<<name.utf8();
    col<<owner.utf8();
    while(!col.eof()) {
      char buffer[10000];
      col>>buffer;
      QString column=QString::fromUtf8(buffer);
      col>>buffer;
      addDescription(lst,ctx,"COLUMN",column.lower(),"COMMENT",QString::fromUtf8(buffer));
    }
  }
}

QString toExtract::prepareDB(const QString &db)
{
  static QRegExp quote("'");
  QString ret=db;
  ret.replace(quote,"''");
  return ret;
}

QString toExtract::intSchema(const QString &owner)
{
  if (owner.upper()=="PUBLIC")
    return "PUBLIC";
  if (Schema=="1") {
    QString ret=owner.lower();
    if (!Describe)
      ret+=".";
    return ret;
  } else if (Schema.isEmpty())
    return "";
  QString ret=Schema.lower();
  if (!Describe)
    ret+=".";
  return ret;
}

QString toExtract::generateHeading(const QString &action,
				   list<QString> &lst)
{
  if (!Heading)
    return "";
  char host[1024];
  gethostname(host,1024);

  QString str=QString("REM This DDL was reverse engineered by\n"
		      "REM TOra, Version %1\n"
		      "REM\n"
		      "REM at:   %2\n"
		      "REM from: %3, an Oracle Release %4 instance\n"
		      "REM\n"
		      "REM on:   %5\n"
		      "REM\n").
    arg(TOVERSION).
    arg(host).
    arg(Connection.host()).
    arg(Connection.version()).
    arg(QDateTime::currentDateTime().toString());
  if (action=="FREE SPACE")
    str+="REM Generating free space report";
  else {
    str+="REM Generating ";
    str+=action;
    str+=" statements";
  }
  if (lst.size()!=1)
    str+="s";
  str+=" for:\nREM\n";
  for(list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
    if (!(*i).startsWith("TABLE REFERENCES")) {
      str+="REM ";
      str+=(*i);
      str+="\n";
    }
  }
  str+="\n";
  return str;
}

void toExtract::parseObject(const QString &object,QString &owner,QString &name)
{
  try {
    unsigned int search=0;
    if (object[0]=='\"') {
      int pos=object.find('\"',1);
      if (pos<0)
	throw 1;
      owner=object.left(pos);
      owner=owner.right(owner.length()-1);
      search=pos+1;
    } else {
      int pos=object.find('.');
      if (pos<0)
	pos=object.length();
      owner=object.left(pos).upper();
      search=pos;
    }
    if (search>=object.length()) {
      name=owner;
      owner=Connection.user();
    } else if (object.at(search)!='.')
      throw 2;
    search++;
    if (object.at(search)=='\"') {
      int pos=object.find('\"',search+1);
      if (pos<0)
	throw 3;
      name=object.left(pos);
      name=owner.right(pos-search-1);
      search=pos+1;
      if (search<object.length())
	throw 4;
    } else {
      name=object.right(object.length()-search).upper();
    }
  } catch(int i) {
    QString ret="Failed to extract owner and name from (";
    ret+=object;
    ret+="), internal error ";
    ret+=QString::number(i);
    throw ret;
  }
}

static toSQL SQLListConstraint("toExtract:ListConstraint",
			       "SELECT table_name,\n"
			       "       constraint_type,\n"
			       "       search_condition,\n"
			       "       r_owner,\n"
			       "       r_constraint_name,\n"
			       "       delete_rule,\n"
			       "       DECODE(status,'ENABLED','ENABLE NOVALIDATE','DISABLE'),\n"
			       "       deferrable,\n"
			       "       deferred\n"
			       "  FROM all_constraints\n"
			       " WHERE owner = :own<char[100]>\n"
			       "   AND constraint_name = :nam<char[100]>",
			       "Get information about a constraint, same binds and columns",
			       "8.0");

static toSQL SQLListConstraint7("toExtract:ListConstraint",
				"SELECT table_name,\n"
				"       constraint_type,\n"
				"       search_condition,\n"
				"       r_owner,\n"
				"       r_constraint_name,\n"
				"       delete_rule,\n"
				"       DECODE(status,'ENABLED','ENABLE','DISABLE'),\n"
				"       NULL,\n"
				"       NULL\n"
				"  FROM all_constraints\n"
				" WHERE owner = :own<char[100]>\n"
				"   AND constraint_name = :nam<char[100]>",
				QString::null,
				"7.0");

static toSQL SQLConstraintTable("toExtract:ConstraintTable",
				"SELECT table_name\n"
				"  FROM all_constraints\n"
				" WHERE owner = :own<char[100]>\n"
				"   AND constraint_name = :nam<char[100]>",
				"Get tablename from constraint name, same binds and columns");

QString toExtract::createConstraint(const QString &schema,const QString &owner,const QString &name)
{
  QString ret;
  
  if (Constraints) {
    otl_stream inf(1,
		   SQLListConstraint(Connection),
		   Connection.connection());
    char buffer[8193];
    otl_long_string data(buffer,8192);
    buffer[8192]=0;
    inf<<owner.utf8();
    inf<<name.utf8();
    if (inf.eof())
      throw QString("Constraint %1.%2 doesn't exist").arg(owner).arg(name);
    inf>>buffer;
    QString table(QString::fromUtf8(buffer));
    inf>>buffer;
    QString tchr(QString::fromUtf8(buffer));
    inf>>data;
    buffer[data.len()]=0;
    QString search(QString::fromUtf8(buffer));
    inf>>buffer;
    QString rOwner(QString::fromUtf8(buffer));
    inf>>buffer;
    QString rName(QString::fromUtf8(buffer));
    inf>>buffer;
    QString delRule(QString::fromUtf8(buffer));
    inf>>buffer;
    QString status(QString::fromUtf8(buffer));
    inf>>buffer;
    QString defferable(QString::fromUtf8(buffer));
    inf>>buffer;
    QString deffered(QString::fromUtf8(buffer));
  
    QString type=
      (tchr=="P")?"PRIMARY KEY":
      (tchr=="U")?"UNIQUE":
      (tchr=="R")?"FOREIGN KEY":
      "CHECK";

    QString sql=QString("ALTER TABLE %1%2 ADD CONSTRAINT %3 %4\n").
      arg(schema).arg(table.lower()).arg(name.lower()).arg(type);
    if (Prompt) {
      ret="PROMPT ";
      ret+=sql;
      ret+="\n";
    }
    ret+=sql;
    if (tchr=="C") {
      ret+="    (";
      ret+=search;
      ret+=")\n";
    } else {
      ret+=constraintColumns(owner,name);

      if (tchr=="R") {
	otl_stream str(1,
		       SQLConstraintTable(Connection),
		       Connection.connection());
	str<<rOwner.utf8();
	str<<rName.utf8();
	str>>buffer;
	ret+="REFERENCES ";
	ret+=schema;
	ret+=QString::fromUtf8(buffer).lower();
	ret+=constraintColumns(rOwner,rName);

	if (delRule=="CASCADE")
	  ret+="ON DELETE CASCADE\n";
      }
    }
    if (Connection.version()<"8")
      ret+=status;
    else {
      ret+=defferable;
      ret+="\nINITIALLY ";
      ret+=deffered;
      ret+="\n";
      ret+=status;
    }
    ret+=";\n\n";
  }
  return ret;
}

void toExtract::describeConstraint(list<QString> &lst,const QString &schema,
				   const QString &owner,const QString &name)
{
  if (Constraints) {
    otl_stream inf(1,
		   SQLListConstraint(Connection),
		   Connection.connection());
    char buffer[8193];
    otl_long_string data(buffer,8192);
    buffer[8192]=0;
    inf<<owner.utf8();
    inf<<name.utf8();
    if (inf.eof())
      throw QString("Constraint %1.%2 doesn't exist").arg(owner).arg(name);
    inf>>buffer;
    QString table(QString::fromUtf8(buffer));
    inf>>buffer;
    QString tchr(QString::fromUtf8(buffer));
    inf>>data;
    buffer[data.len()]=0;
    QString search(QString::fromUtf8(buffer));
    inf>>buffer;
    QString rOwner(QString::fromUtf8(buffer));
    inf>>buffer;
    QString rName(QString::fromUtf8(buffer));
    inf>>buffer;
    QString delRule(QString::fromUtf8(buffer));
    inf>>buffer;
    QString status(QString::fromUtf8(buffer));
    inf>>buffer;
    QString defferable(QString::fromUtf8(buffer));
    inf>>buffer;
    QString deffered(QString::fromUtf8(buffer));
  
    QString type=
      (tchr=="P")?"PRIMARY KEY":
      (tchr=="U")?"UNIQUE":
      (tchr=="R")?"FOREIGN KEY":
      "CHECK";

    list<QString> ctx;
    ctx.insert(ctx.end(),schema);
    ctx.insert(ctx.end(),"TABLE");
    ctx.insert(ctx.end(),table.lower());
    ctx.insert(ctx.end(),"CONSTRAINT");
    ctx.insert(ctx.end(),name.lower());

    QString ret;
    if (tchr=="C") {
      ret+="CHECK (";
      ret+=search;
      ret+=")";
    } else {
      ret+=constraintColumns(owner,name).simplifyWhiteSpace();

      if (tchr=="R") {
	otl_stream str(1,
		       SQLConstraintTable(Connection),
		       Connection.connection());
	str<<rOwner.utf8();
	str<<rName.utf8();
	str>>buffer;
	ret+="REFERENCES ";
	ret+=schema;
	ret+=QString::fromUtf8(buffer);
	ret+=constraintColumns(rOwner,rName);

	if (delRule=="CASCADE")
	  ret+="ON DELETE CASCADE";
      }
    }
    addDescription(lst,ctx,ret);

    ret="";
    if (Connection.version()<"8")
      ret+=status;
    else {
      ret+=defferable;
      ret+="\nINITIALLY ";
      ret+=deffered;
      ret+="\n";
      ret+=status;
    }
    addDescription(lst,ctx,ret);
  }
}

static toSQL SQLDBLink("toExtract:ExtractDBLink",
		       "SELECT l.userid,\n"
		       "       l.password,\n"
		       "       l.host\n"
		       "  FROM sys.link$ l,\n"
		       "       sys.user$ u\n"
		       " WHERE u.name    = :own<char[100]>\n"
		       "   AND l.owner# = u.user#\n"
		       "   AND l.name LIKE :nam<char[100]>||'%'",
		       "Get information about a DB Link, must have same binds and columns");

QString toExtract::createDBLink(const QString &schema,const QString &owner,const QString &name)
{
  char buffer[100];
  otl_stream inf(1,
		 SQLDBLink(Connection),
		 Connection.connection());
  inf<<owner.utf8();
  inf<<name.utf8();
  if (inf.eof())
    throw QString("DBLink %1.%2 doesn't exist").arg(owner).arg(name);
  inf>>buffer;
  QString user(QString::fromUtf8(buffer));
  inf>>buffer;
  QString password(QString::fromUtf8(buffer));
  inf>>buffer;
  QString host(QString::fromUtf8(buffer));
  QString publ=(owner=="PUBLIC")?" PUBLIC":"";
  QString sql=QString("CREATE%1 DATABASE LINK %2").arg(publ).arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=QString("\nCONNECT TO %1 IDENTIFIED BY %2 USING '%3';\n\n").
    arg(user.lower()).
    arg(password.lower()).
    arg(prepareDB(host));
  return ret;
}

void toExtract::describeDBLink(list<QString> &lst,const QString &schema,const QString &owner,
			       const QString &name)
{
  char buffer[100];
  otl_stream inf(1,
		 SQLDBLink(Connection),
		 Connection.connection());
  inf<<owner.utf8();
  inf<<name.utf8();
  if (inf.eof())
    throw QString("DBLink %1.%2 doesn't exist").arg(owner).arg(name);
  inf>>buffer;
  QString user(QString::fromUtf8(buffer));
  inf>>buffer;
  QString password(QString::fromUtf8(buffer));
  inf>>buffer;
  QString host(QString::fromUtf8(buffer));
  QString publ=(owner=="PUBLIC")?"PUBLIC":"";
  QString sql=QString("CREATE%1 DATABASE LINK %2").arg(publ).arg(name.lower());
  QString ret;

  list<QString> ctx;
  ctx.insert(ctx.end(),"DATABASE LINK");
  ctx.insert(ctx.end(),name.lower());

  addDescription(lst,ctx,publ);
  addDescription(lst,ctx,QString("%1 IDENTIFIED BY %2 USING '%3'").
		 arg(user.lower()).
		 arg(password.lower()).
		 arg(prepareDB(host)));
}

static toSQL SQLPartitionSegmentType("toExtract:PartitionSegment type",
				     "SELECT SUBSTR(segment_type,7),\n"
				     "       TO_CHAR(blocks)\n"
				     "  FROM dba_segments\n"
				     " WHERE segment_name = :nam<char[100]>\n"
				     "   AND partition_name = :prt<char[100]>\n"
				     "   AND owner = :own<char[100]>",
				     "Get segment type for object partition, same binds and columns");

static toSQL SQLExchangeIndex("toExtract:ExchangeIndex",
			      "SELECT\n"
			      "        LTRIM(i.degree)\n"
			      "      , LTRIM(i.instances)\n"
			      "      , i.table_name\n"
			      "      , DECODE(\n"
			      "                i.uniqueness\n"
			      "               ,'UNIQUE',' UNIQUE'\n"
			      "               ,null\n"
			      "              )                       AS uniqueness\n"
			      "      , DECODE(\n"
			      "                i.index_type\n"
			      "               ,'BITMAP',' BITMAP'\n"
			      "               ,null\n"
			      "              )                       AS index_type\n"
			      "        -- Physical Properties\n"
			      "      , 'INDEX'                       AS organization\n"
			      "        -- Segment Attributes\n"
			      "      , 'N/A'                         AS cache\n"
			      "      , 'N/A'                         AS pct_used\n"
			      "      , p.pct_free\n"
			      "      , DECODE(\n"
			      "                p.ini_trans\n"
			      "               ,0,1\n"
			      "               ,null,1\n"
			      "               ,p.ini_trans\n"
			      "              )                       AS ini_trans\n"
			      "      , DECODE(\n"
			      "                p.max_trans\n"
			      "               ,0,255\n"
			      "               ,null,255\n"
			      "               ,p.max_trans\n"
			      "              )                       AS max_trans\n"
			      "        -- Storage Clause\n"
			      "      , p.initial_extent\n"
			      "      , p.next_extent\n"
			      "      , p.min_extent\n"
			      "      , DECODE(\n"
			      "                p.max_extent\n"
			      "               ,2147483645,'unlimited'\n"
			      "               ,           p.max_extent\n"
			      "              )                       AS max_extent\n"
			      "      , p.pct_increase\n"
			      "      , NVL(p.freelists,1)\n"
			      "      , NVL(p.freelist_groups,1)\n"
			      "      , LOWER(p.buffer_pool)          AS buffer_pool\n"
			      "      , DECODE(\n"
			      "                p.logging\n"
			      "               ,'NO','NOLOGGING'\n"
			      "               ,     'LOGGING'\n"
			      "              )                       AS logging\n"
			      "      , LOWER(p.tablespace_name)      AS tablespace_name\n"
			      "      , %2                            AS blocks\n"
			      " FROM\n"
			      "        dba_indexes  i\n"
			      "      , dba_ind_%1s  p\n"
			      " WHERE\n"
			      "            p.index_name   = :nam<char[100]>\n"
			      "        AND p.%1_name      = :typ<char[100]>\n"
			      "        AND i.index_name   = p.index_name\n"
			      "        AND p.index_owner  = :own<char[100]>\n"
			      "        AND i.owner        = p.index_owner",
			      "Get information about exchange index, "
			      "must have same %, binds and columns");

QString toExtract::createExchangeIndex(const QString &schema,const QString &owner,
				       const QString &name)
{
  QStringList str=QStringList::split(":",name);
  if (str.count()!=2)
    throw ("When calling createExchangeIndex name should contain :");
  QString segment=str.first();
  QString partition=str.last();

  otl_stream inf(1,
		 SQLPartitionSegmentType(Connection),
		 Connection.connection());
  inf<<segment.utf8()<<partition.utf8()<<owner.utf8();
  if (inf.eof())
    throw QString("Exchange index %1.%2 doesn't exist").arg(owner).arg(name);
  char buffer[100];
  inf>>buffer;
  QString type=QString::fromUtf8(buffer);
  QString blocks=QString::fromUtf8(buffer);

  QString sql=toSQL::string(SQLExchangeIndex,Connection).
    arg(type).arg(blocks);
  list<QString> result=toReadQuery(Connection,sql.utf8(),segment,partition,owner);
  QString degree=toShift(result);
  QString instances=toShift(result);
  QString table=toShift(result);
  QString unique=toShift(result);
  QString bitmap=toShift(result);

  toUnShift(result,"");

  sql=QString("CREATE%1%2 INDEX %3%4 ON %3%5\n").
    arg(unique).arg(bitmap).arg(schema).arg(segment).arg(table);

  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n";
  }
  ret+=sql;
  ret+=indexColumns("",owner,segment);
  if (Parallel) {
    ret+="PARALLEL\n(\n  DEGREE            ";
    ret+=degree;
    ret+="\n  INSTANCES         ";
    ret+=instances;
    ret+=")\n";
  }
  ret+=segmentAttributes(result);
  ret+=";\n\n";
  return ret;
}

void toExtract::describeExchangeIndex(list<QString> &lst,const QString &schema,const QString &owner,
				      const QString &name)
{
  QStringList str=QStringList::split(":",name);
  if (str.count()!=2)
    throw ("When calling createExchangeIndex name should contain :");
  QString segment=str.first();
  QString partition=str.last();

  otl_stream inf(1,
		 SQLPartitionSegmentType(Connection),
		 Connection.connection());
  inf<<segment.utf8()<<partition.utf8()<<owner.utf8();
  if (inf.eof())
    throw QString("Exchange index %1.%2 doesn't exist").arg(owner).arg(name);
  char buffer[100];
  inf>>buffer;
  QString type=QString::fromUtf8(buffer);
  QString blocks=QString::fromUtf8(buffer);

  QString sql=toSQL::string(SQLExchangeIndex,Connection).arg(type).arg(blocks);
  list<QString> result=toReadQuery(Connection,sql.utf8(),segment,partition,owner);
  QString degree=toShift(result);
  QString instances=toShift(result);
  QString table=toShift(result);
  QString unique=toShift(result);
  QString bitmap=toShift(result);

  toUnShift(result,"");

  list<QString> ctx;
  ctx.insert(ctx.end(),schema);
  ctx.insert(ctx.end(),"TABLE");
  ctx.insert(ctx.end(),table);
  ctx.insert(ctx.end(),"EXCHANGE INDEX");
  ctx.insert(ctx.end(),segment);
  ctx.insert(ctx.end(),partition);

  describeIndexColumns(lst,ctx,owner,segment);
  addDescription(lst,ctx,QString("%1%2").arg(unique).arg(bitmap));
  addDescription(lst,ctx,QString("%1%2").arg(segment).arg(table));
  if (Parallel) {
    addDescription(lst,ctx,"PARALLEL","DEGREE",degree);
    addDescription(lst,ctx,"PARALLEL","INSTANCES",instances);
  }
  describeAttributes(lst,ctx,result);
}

static toSQL SQLIndexColumns("toExtract:IndexColumns",
			     "SELECT column_name,\n"
			     "       descend\n"
			     "  FROM all_ind_columns\n"
			     " WHERE index_name = :nam<char[100]>\n"
			     "   AND index_owner = :own<char[100]>\n"
			     " ORDER BY column_position",
			     "Get column names from index, same binds and columns",
			     "8.1");
static toSQL SQLIndexColumns7("toExtract:IndexColumns",
			      "SELECT column_name,\n"
			      "       'ASC'\n"
			      "  FROM all_ind_columns\n"
			      " WHERE index_name = :nam<char[100]>\n"
			      "   AND index_owner = :own<char[100]>\n"
			      " ORDER BY column_position",
			      QString::null,
			      "7");
static toSQL SQLIndexFunction("toExtract:IndexFunction",
			      "SELECT c.default$\n"
			      "  FROM sys.col$ c,\n"
			      "       all_indexes i,\n"
			      "       all_objects o\n"
			      " WHERE i.index_name = :ind<char[100]>\n"
			      "   AND o.object_name = i.table_name\n"
			      "   AND c.obj# = o.object_id\n"
			      "   AND c.name = :tab<char[100]>\n"
			      "   AND i.owner = :own<char[100]>\n"
			      "   AND o.owner = i.table_owner",
			      "Get function of index column, same column and binds");

QString toExtract::indexColumns(const QString &indent,
				const QString &owner,
				const QString &name)
{
  static QRegExp quote("\"");
  static QRegExp func("^sys_nc[0-9]g");
  otl_stream inf(1,
		 SQLIndexColumns(Connection),
		 Connection.connection());
  inf<<name.utf8();
  inf<<owner.utf8();
  QString ret=indent;
  ret+="(\n";
  bool first=true;
  while(!inf.eof()) {
    char buffer[1025];
    inf>>buffer;
    QString col=QString::fromUtf8(buffer).lower();
    inf>>buffer;
    QString asc=QString::fromUtf8(buffer);
    QString row;
    if (func.match(col)>=0) {
      otl_stream def(1,
		     SQLIndexFunction(Connection),
		     Connection.connection());
      def<<col.utf8();
      def<<name.utf8();
      def<<owner.utf8();
      otl_long_string data(buffer,1024);
      def>>data;
      buffer[data.len()]=0;
      QString function(QString::fromUtf8(buffer));
      function.replace(quote,"");
      if (asc=="DESC")
	row=QString("%1 DESC").arg(function.lower(),30);
      else
	row=function.lower();
    } else
      row=col;
    ret+=indent;
    if(first) {
      first=false;
      ret+="    ";
    } else
      ret+="  , ";
    ret+=row;
    ret+="\n";
  }
  ret+=indent;
  ret+=")\n";
  return ret;
}

void toExtract::describeIndexColumns(list<QString> &lst,list<QString> &ctx,
				     const QString &owner,const QString &name)
{
  static QRegExp quote("\"");
  static QRegExp func("^sys_nc[0-9]g");
  otl_stream inf(1,
		 SQLIndexColumns(Connection),
		 Connection.connection());
  inf<<name.utf8();
  inf<<owner.utf8();
  while(!inf.eof()) {
    char buffer[1025];
    inf>>buffer;
    QString col=QString::fromUtf8(buffer).lower();
    inf>>buffer;
    QString asc=QString::fromUtf8(buffer);
    QString row;
    if (func.match(col)>=0) {
      otl_stream def(1,
		     SQLIndexFunction(Connection),
		     Connection.connection());
      def<<col.utf8();
      def<<name.utf8();
      def<<owner.utf8();
      otl_long_string data(buffer,1024);
      def>>data;
      buffer[data.len()]=0;
      QString function(QString::fromUtf8(buffer));
      function.replace(quote,"");
      if (asc=="DESC")
	row=QString("%1 DESC").arg(function.lower(),30);
      else
	row=function.lower();
    } else
      row=col;
    addDescription(lst,ctx,"COLUMN",row);
  }
}

static toSQL SQLExchangeTable("toExtract:ExchangeTable",
			      "SELECT\n"
			      "        DECODE(\n"
			      "                t.monitoring\n"
			      "               ,'NO','NOMONITORING'\n"
			      "               ,     'MONITORING'\n"
			      "              )                              AS monitoring\n"
			      "      , t.table_name\n"
			      "      , LTRIM(t.degree)                      AS degree\n"
			      "      , LTRIM(t.instances)                   AS instances\n"
			      "      , 'HEAP'                               AS organization\n"
			      "      , DECODE(\n"
			      "                t.cache\n"
			      "               ,'y','CACHE'\n"
			      "               ,    'NOCACHE'\n"
			      "              )                              AS cache\n"
			      "      , p.pct_used\n"
			      "      , p.pct_free\n"
			      "      , p.ini_trans\n"
			      "      , p.max_trans\n"
			      "      , p.initial_extent\n"
			      "      , p.next_extent\n"
			      "      , p.min_extent\n"
			      "      , DECODE(\n"
			      "                p.max_extent\n"
			      "               ,2147483645,'unlimited'\n"
			      "               ,p.max_extent\n"
			      "              )                              AS max_extents\n"
			      "      , p.pct_increase\n"
			      "      , p.freelists\n"
			      "      , p.freelist_groups\n"
			      "      , LOWER(p.buffer_pool)                 AS buffer_pool\n"
			      "      , DECODE(\n"
			      "                p.logging\n"
			      "               ,'NO','NOLOGGING'\n"
			      "               ,     'LOGGING'\n"
			      "              )                              AS logging\n"
			      "      , LOWER(p.tablespace_name)             AS tablespace_name\n"
			      "      , %2 - NVL(p.empty_blocks,0)           AS blocks\n"
			      " FROM\n"
			      "        dba_tables        t\n"
			      "      , dba_tab_%1s       p\n"
			      " WHERE\n"
			      "            p.table_name   = :nam<char[100]>\n"
			      "        AND p.%1_name      = :sgm<char[100]>\n"
			      "        AND t.table_name   = p.table_name\n"
			      "        AND p.table_owner  = :own<char[100]>\n"
			      "        AND t.owner        = p.table_owner",
			      "Get information about exchange table, "
			      "must have same %, binds and columns");

QString toExtract::createExchangeTable(const QString &schema,const QString &owner,
				       const QString &name)
{
  QStringList str=QStringList::split(":",name);
  if (str.count()!=2)
    throw ("When calling createExchangeTable name should contain :");
  QString segment=str.first();
  QString partition=str.last();

  otl_stream inf(1,
		 SQLPartitionSegmentType(Connection),
		 Connection.connection());
  inf<<segment.utf8()<<partition.utf8()<<owner.utf8();
  if (inf.eof())
    throw QString("Exchange table %1.%2 doesn't exist").arg(owner).arg(name);
  char buffer[100];
  inf>>buffer;
  QString type=QString::fromUtf8(buffer);
  QString blocks=QString::fromUtf8(buffer);

  QString sql=toSQL::string(SQLExchangeTable,Connection).arg(type).arg(blocks);
  list<QString> result=toReadQuery(Connection,sql.utf8(),segment,partition,owner);
  QString ret=createTableText(result,schema,owner,segment);
  ret+=";\n\n";
  return ret;
}

void toExtract::describeExchangeTable(list<QString> &lst,const QString &schema,const QString &owner,
				      const QString &name)
{
  QStringList str=QStringList::split(":",name);
  if (str.count()!=2)
    throw ("When calling createExchangeTable name should contain :");
  QString segment=str.first();
  QString partition=str.last();

  otl_stream inf(1,
		 SQLPartitionSegmentType(Connection),
		 Connection.connection());
  inf<<segment.utf8()<<partition.utf8()<<owner.utf8();
  if (inf.eof())
    throw QString("Exchange table %1.%2 doesn't exist").arg(owner).arg(name);
  char buffer[100];
  inf>>buffer;
  QString type=QString::fromUtf8(buffer);
  QString blocks=QString::fromUtf8(buffer);

  QString sql=toSQL::string(SQLExchangeTable,Connection).arg(type).arg(blocks);
  list<QString> result=toReadQuery(Connection,sql.utf8(),segment,partition,owner);
  list<QString> ctx;
  ctx.insert(ctx.end(),schema);
  ctx.insert(ctx.end(),"EXCHANGE TABLE");
  ctx.insert(ctx.end(),segment);
  ctx.insert(ctx.end(),partition);
  describeTableText(lst,ctx,result,schema,owner,segment);
}

void toExtract::initialNext(const QString &blocks,QString &initial,QString &next)
{
  list<QString>::iterator iinit=Initial.begin();
  list<QString>::iterator inext=Next.begin();
  list<QString>::iterator ilimit=Limit.begin();
  while(ilimit!=Initial.end()) {
    if (*ilimit=="UNLIMITED"||
	(*ilimit).toFloat()>blocks.toFloat()) {
      initial=*iinit;
      next=*inext;
      return;
    }
    iinit++;
    inext++;
    ilimit++;
  }
}

QString toExtract::segmentAttributes(list<QString> &result)
{
  QString ret;
  if (Storage) {
    if (result.size()!=18)
      throw QString("Internal error, result should be 18 in segment attributes (Was %1)").
	arg(result.size());

    list<QString>::iterator i=result.begin();

    QString indent         =*i; i++;
    QString organization   =*i; i++;
    QString cache          =*i; i++;
    QString pctUsed        =*i; i++;
    QString pctFree        =*i; i++;
    QString iniTrans       =*i; i++;
    QString maxTrans       =*i; i++;
    QString initial        =*i; i++;
    QString next           =*i; i++;
    QString minExtents     =*i; i++;
    QString maxExtents     =*i; i++;
    QString pctIncrease    =*i; i++;
    QString freelists      =*i; i++;
    QString freelistGroups =*i; i++;
    QString bufferPool     =*i; i++;
    QString logging        =*i; i++;
    QString tablespace     =*i; i++;
    QString blocks         =*i; i++;

    if (Resize)
      initialNext(blocks,initial,next);

    if (organization == "HEAP") {
      if (cache!="N/A") {
	ret+=indent;
	ret+=cache;
	ret+="\n";
      }
      if (!IsASnapIndex)
	ret+=QString("%1PCTUSED             %2\n").arg(indent).arg(pctUsed);
    }
    if (!IsASnapIndex)
      ret+=QString("%1PCTFREE             %2\n").arg(indent).arg(pctFree);
    if (!IsASnapTable)
      ret+=QString("%1INITRANS            %2\n").arg(indent).arg(iniTrans);

    ret+=QString("%1MAXTRANS            %2\n").arg(indent).arg(maxTrans);
    ret+=indent;
    ret+="STORAGE\n";
    ret+=indent;
    ret+="(\n";
    ret+=QString("%1  INITIAL           %2\n").arg(indent).arg(initial);
    ret+=QString("%1  NEXT              %2\n").arg(indent).arg(next);
    ret+=QString("%1  MINEXTENTS        %2\n").arg(indent).arg(minExtents);
    ret+=QString("%1  MAXEXTENTS        %2\n").arg(indent).arg(maxExtents);
    ret+=QString("%1  PCTINCREASE       %2\n").arg(indent).arg(pctIncrease);
    ret+=QString("%1  FREELISTS         %2\n").arg(indent).arg(freelists);
    ret+=QString("%1  FREELIST GROUPS   %2\n").arg(indent).arg(freelistGroups);
    if (Connection.version()>="8.1") {
      ret+=QString("%1  BUFFER_POOL       %2\n").arg(indent).arg(bufferPool);
    }
    ret+=indent;
    ret+=")\n";
    if (Connection.version()>="8.0")
      ret+=QString("%1%2\n").arg(indent).arg(logging);
    ret+=QString("%1TABLESPACE          %2\n").arg(indent).arg(tablespace.lower());
  }
  return ret;
}

void toExtract::describeAttributes(list<QString> &dsp,list<QString> &ctx,list<QString> &result)
{
  if (!Storage)
    return;

  if (result.size()!=18)
    throw QString("Internal error, result should be 18 in segment attributes");

  list<QString>::iterator i=result.begin();

  QString indent         =*i; i++;
  QString organization   =*i; i++;
  QString cache          =*i; i++;
  QString pctUsed        =*i; i++;
  QString pctFree        =*i; i++;
  QString iniTrans       =*i; i++;
  QString maxTrans       =*i; i++;
  QString initial        =*i; i++;
  QString next           =*i; i++;
  QString minExtents     =*i; i++;
  QString maxExtents     =*i; i++;
  QString pctIncrease    =*i; i++;
  QString freelists      =*i; i++;
  QString freelistGroups =*i; i++;
  QString bufferPool     =*i; i++;
  QString logging        =*i; i++;
  QString tablespace     =*i; i++;
  QString blocks         =*i; i++;

  if (Resize)
    initialNext(blocks,initial,next);

  QString ret;
  if (organization == "HEAP") {
    if (cache!="N/A")
      addDescription(dsp,ctx,cache);
    if (!IsASnapIndex)
      addDescription(dsp,ctx,QString("PCTUSED %1").arg(pctUsed));
  }
  if (!IsASnapIndex) {
    addDescription(dsp,ctx,QString("PCTFREE %1").arg(pctFree));
    addDescription(dsp,ctx,QString("INITRANS %1").arg(iniTrans));
  }
  addDescription(dsp,ctx,QString("MAXTRANS %1").arg(maxTrans));
  addDescription(dsp,ctx,"STORAGE",QString("INITIAL %1").arg(initial));
  addDescription(dsp,ctx,"STORAGE",QString("NEXT %1").arg(next));
  addDescription(dsp,ctx,"STORAGE",QString("MINEXTENTS %1").arg(minExtents));
  addDescription(dsp,ctx,"STORAGE",QString("MAXEXTENTS %1").arg(maxExtents));
  addDescription(dsp,ctx,"STORAGE",QString("PCTINCREASE %1").arg(pctIncrease));
  addDescription(dsp,ctx,"STORAGE",QString("FREELISTS %1").arg(freelists));
  addDescription(dsp,ctx,"STORAGE",QString("FREELIST GROUPS %1").arg(freelistGroups));
  if (Connection.version()>="8.1") {
    addDescription(dsp,ctx,"STORAGE",QString("BUFFER_POOL %1").arg(bufferPool));
  }
  if (Connection.version()>="8.0")
    addDescription(dsp,ctx,QString("%1").arg(logging));
  addDescription(dsp,ctx,QString("TABLESPACE %1").arg(tablespace.lower()));
}

static toSQL SQLPrimaryKey("toExtract:PrimaryKey",
			   "SELECT constraint_name\n"
			   "  FROM all_constraints\n"
			   " WHERE table_name = :nam<char[100]>\n"
			   "   AND constraint_type = 'P'\n"
			   "   AND owner = :own<char[100]>",
			   "Get constraint name for primary key of table, same binds and columns");

QString toExtract::createTableText(list<QString> &result,const QString &schema,
				   const QString &owner,const QString &name)
{
  QString monitoring     =toShift(result);
  QString table          =toShift(result);
  QString degree         =toShift(result);
  QString instances      =toShift(result);
  QString organization   =toShift(result);

  QString ret;
  if (Prompt)
    ret=QString("PROMPT CREATE TABLE %1%2\n\n").arg(schema).arg(name.lower());
  ret+=QString("CREATE TABLE %1%2\n(\n    ").arg(schema).arg(name.lower());
  ret+=tableColumns(owner,name);
  if (organization=="INDEX"&&Storage) {
    list<QString> res=toReadQuery(Connection,SQLPrimaryKey(Connection),name,owner);
    if (res.size()!=1)
      throw QString("Couldn't find primary key of %1.%2").arg(owner).arg(name);
    QString primary=*(res.begin());
    ret+=QString("  , CONSTRAINT %1 PRIMARY KEY\n").arg(primary.lower());
    ret+=indexColumns("      ",owner,primary);
  }
  ret+=")\n";
  if (Connection.version()>="8.0"&&Storage) {
    ret+="ORGANIZATION        ";
    ret+=organization;
    ret+="\n";
  }
  if (Connection.version()>="8.1"&&Storage) {
    ret+=monitoring;
    ret+="\n";
  }
  if (Parallel)
    ret+=QString("PARALLEL\n"
		 "(\n"
		 "  DEGREE            %1\n"
		 "  INSTANCES         %2\n"
		 ")\n").
      arg(degree).
      arg(instances);

  toUnShift(result,organization);
  toUnShift(result,"");

  ret+=segmentAttributes(result);

  return ret;
}

void toExtract::describeTableText(list<QString> &lst,list<QString> &ctx,list<QString> &result,
				  const QString &schema,const QString &owner,const QString &name)
{
  QString monitoring     =toShift(result);
  QString table          =toShift(result);
  QString degree         =toShift(result);
  QString instances      =toShift(result);
  QString organization   =toShift(result);

  QString ret;
  addDescription(lst,ctx);
  describeTableColumns(lst,ctx,owner,name);
  if (Connection.version()>="8.0"&&Storage)
    addDescription(lst,ctx,QString("ORGANIZATION %1").arg(organization));
  if (Connection.version()>="8.1"&&Storage)
    addDescription(lst,ctx,monitoring);
  if (Parallel) {
    addDescription(lst,ctx,"PARALLEL",QString("DEGREE %1").arg(degree));
    addDescription(lst,ctx,"PARALLEL",QString("INSTANCES %1").arg(instances));
  }

  toUnShift(result,organization);
  toUnShift(result,"");

  describeAttributes(lst,ctx,result);
}

static toSQL SQLTableColumns("toExtract:TableColumns",
			     "SELECT  RPAD(LOWER(column_name),32)\n"
			     "     || RPAD(\n"
			     "             DECODE(\n"
			     "                     data_type\n"
			     "                    ,'NUMBER',DECODE(\n"
			     "                                      data_precision\n"
			     "                                     ,null,DECODE(\n"
			     "                                                   data_scale\n"
			     "                                                  ,0,'INTEGER'\n"
			     "                                                  ,  'NUMBER   '\n"
			     "                                                 )\n"
			     "                                     ,'NUMBER   '\n"
			     "                                    )\n"
			     "                    ,'RAW'     ,'RAW      '\n"
			     "                    ,'CHAR'    ,'CHAR     '\n"
			     "                    ,'NCHAR'   ,'NCHAR    '\n"
			     "                    ,'UROWID'  ,'UROWID   '\n"
			     "                    ,'VARCHAR2','VARCHAR2 '\n"
			     "                    ,data_type\n"
			     "                   )\n"
			     "               || DECODE(\n"
			     "                          data_type\n"
			     "                         ,'DATE',null\n"
			     "                         ,'LONG',null\n"
			     "                         ,'NUMBER',DECODE(\n"
			     "                                           data_precision\n"
			     "                                          ,null,null\n"
			     "                                          ,'('\n"
			     "                                         )\n"
			     "                         ,'RAW'      ,'('\n"
			     "                         ,'CHAR'     ,'('\n"
			     "                         ,'NCHAR'    ,'('\n"
			     "                         ,'UROWID'   ,'('\n"
			     "                         ,'VARCHAR2' ,'('\n"
			     "                         ,'NVARCHAR2','('\n"
			     "                         ,null\n"
			     "                        )\n"
			     "               || DECODE(\n"
			     "                          data_type\n"
			     "                         ,'RAW'      ,data_length\n"
			     "                         ,'CHAR'     ,data_length\n"
			     "                         ,'NCHAR'    ,data_length\n"
			     "                         ,'UROWID'   ,data_length\n"
			     "                         ,'VARCHAR2' ,data_length\n"
			     "                         ,'NVARCHAR2',data_length\n"
			     "                         ,'NUMBER'   ,data_precision\n"
			     "                         , null\n"
			     "                        )\n"
			     "               || DECODE(\n"
			     "                          data_type\n"
			     "                         ,'NUMBER',DECODE(\n"
			     "                           data_precision\n"
			     "                          ,null,null\n"
			     "                          ,DECODE(\n"
			     "                                   data_scale\n"
			     "                                  ,null,null\n"
			     "                                  ,0   ,null\n"
			     "                                  ,',' || data_scale\n"
			     "                                 )\n"
			     "                              )\n"
			     "                        )\n"
			     "               || DECODE(\n"
			     "                          data_type\n"
			     "                         ,'DATE',null\n"
			     "                         ,'LONG',null\n"
			     "                         ,'NUMBER',DECODE(\n"
			     "                                           data_precision\n"
			     "                                          ,null,null\n"
			     "                                          ,')'\n"
			     "                                         )\n"
			     "                         ,'RAW'      ,')'\n"
			     "                         ,'CHAR'     ,')'\n"
			     "                         ,'NCHAR'    ,')'\n"
			     "                         ,'UROWID'   ,')'\n"
			     "                         ,'VARCHAR2' ,')'\n"
			     "                         ,'NVARCHAR2',')'\n"
			     "                         ,null\n"
			     "                        )\n"
			     "             ,32\n"
			     "            )\n"
			     "     , data_default\n"
			     "     , DECODE(\n"
			     "                nullable\n"
			     "               ,'N','NOT NULL'\n"
			     "               ,     null\n"
			     "              )\n"
			     "  FROM all_tab_columns\n"
			     " WHERE table_name = :nam<char[100]>\n"
			     "   AND owner = :own<char[100]>\n"
			     " ORDER BY column_id",
			     "Extract column definitions from table",
			     "8.0");
static toSQL SQLTableColumns7("toExtract:TableColumns",
			      "       SELECT\n"
			      "        RPAD(LOWER(column_name),32)\n"
			      "     || RPAD(\n"
			      "             DECODE(\n"
			      "                     data_type\n"
			      "                    ,'NUMBER',DECODE(\n"
			      "                                      data_precision\n"
			      "                                     ,null,DECODE(\n"
			      "                                                   data_scale\n"
			      "                                                  ,0,'INTEGER'\n"
			      "                                                  ,  'NUMBER   '\n"
			      "                                                 )\n"
			      "                                     ,'NUMBER   '\n"
			      "                                    )\n"
			      "                    ,'RAW'     ,'RAW      '\n"
			      "                    ,'CHAR'    ,'CHAR     '\n"
			      "                    ,'NCHAR'   ,'NCHAR    '\n"
			      "                    ,'UROWID'  ,'UROWID   '\n"
			      "                    ,'VARCHAR2','VARCHAR2 '\n"
			      "                    ,data_type\n"
			      "                   )\n"
			      "               || DECODE(\n"
			      "                          data_type\n"
			      "                         ,'DATE',null\n"
			      "                         ,'LONG',null\n"
			      "                         ,'NUMBER',DECODE(\n"
			      "                                           data_precision\n"
			      "                                          ,null,null\n"
			      "                                          ,'('\n"
			      "                                         )\n"
			      "                         ,'RAW'      ,'('\n"
			      "                         ,'CHAR'     ,'('\n"
			      "                         ,'NCHAR'    ,'('\n"
			      "                         ,'UROWID'   ,'('\n"
			      "                         ,'VARCHAR2' ,'('\n"
			      "                         ,'NVARCHAR2','('\n"
			      "                         ,null\n"
			      "                        )\n"
			      "               || DECODE(\n"
			      "                          data_type\n"
			      "                         ,'RAW'      ,data_length\n"
			      "                         ,'CHAR'     ,data_length\n"
			      "                         ,'NCHAR'    ,data_length\n"
			      "                         ,'UROWID'   ,data_length\n"
			      "                         ,'VARCHAR2' ,data_length\n"
			      "                         ,'NVARCHAR2',data_length\n"
			      "                         ,'NUMBER'   ,data_precision\n"
			      "                         , null\n"
			      "                        )\n"
			      "               || DECODE(\n"
			      "                          data_type\n"
			      "                         ,'NUMBER',DECODE(\n"
			      "                           data_precision\n"
			      "                          ,null,null\n"
			      "                          ,DECODE(\n"
			      "                                   data_scale\n"
			      "                                  ,null,null\n"
			      "                                  ,0   ,null\n"
			      "                                  ,',' || data_scale\n"
			      "                                 )\n"
			      "                              )\n"
			      "                        )\n"
			      "               || DECODE(\n"
			      "                          data_type\n"
			      "                         ,'DATE',null\n"
			      "                         ,'LONG',null\n"
			      "                         ,'NUMBER',DECODE(\n"
			      "                                           data_precision\n"
			      "                                          ,null,null\n"
			      "                                          ,')'\n"
			      "                                         )\n"
			      "                         ,'RAW'      ,')'\n"
			      "                         ,'CHAR'     ,')'\n"
			      "                         ,'NCHAR'    ,')'\n"
			      "                         ,'UROWID'   ,')'\n"
			      "                         ,'VARCHAR2' ,')'\n"
			      "                         ,'NVARCHAR2',')'\n"
			      "                         ,null\n"
			      "                        )\n"
			      "             ,33\n"
			      "            )\n"
			      "     , data_default\n"
			      "  FROM all_tab_columns\n"
			      " WHERE table_name = :nam<char[100]>\n"
			      "   AND owner = :own<char[100]>\n"
			      " ORDER BY column_id",
			      QString::null,
			      "7.0");

QString toExtract::tableColumns(const QString &owner,const QString &name)
{
  list<QString> cols=toReadQuery(Connection,
				 SQLTableColumns(Connection),
				 name,owner);
  bool first=true;
  QString ret;
  while(cols.size()>0) {
    if (first)
      first=false;
    else
      ret+="\n  , ";
    ret+=toShift(cols);
    QString def=toShift(cols);
    QString notNull=toShift(cols);
    if (!def.isEmpty()) {
      ret+="DEFAULT ";
      ret+=def;
      ret+=" ";
    }
    ret+=notNull;
  }
  ret+="\n";
  return ret;
}

void toExtract::describeTableColumns(list<QString> &lst,list<QString> &ctx,
				     const QString &owner,const QString &name)
{
  list<QString> cols=toReadQuery(Connection,
				 SQLTableColumns(Connection),
				 name,owner);
  while(cols.size()>0) {
    QString line=toShift(cols);
    QString def=toShift(cols);
    if (!def.isEmpty()) {
      line+=" DEFAULT ";
      line+=def;
    }
    line+=toShift(cols);
    addDescription(lst,ctx,"COLUMN",line.simplifyWhiteSpace());
  }
}

static toSQL SQLDisplaySource("toExtract:ListSource",
			      "SELECT text\n"
			      "  FROM all_source\n"
			      " WHERE type = :typ<char[100]>\n"
			      "   AND name = :nam<char[100]>\n"
			      "   AND owner = :own<char[100]>\n"
			      " ORDER BY line",
			      "Get source of an object from the database, "
			      "must have same columns and binds");

QString toExtract::displaySource(const QString &schema,const QString &owner,
				 const QString &name,const QString &type)
{
  if (!Code)
    return "";

  static QRegExp StripType("^[^ \t]+[ \t]+");
  otl_stream inf(1,
		 SQLDisplaySource(Connection),
		 Connection.connection());
  inf<<type.utf8()<<name.utf8()<<owner.utf8();
  if (inf.eof())
    throw QString("Couldn't find source for of %1.%2").arg(owner).arg(name);
  
  QString ret;
  if (Prompt&&!Describe)
    ret=QString("PROMPT CREATE OR REPLACE %1 %2%3\n\n").
      arg(type).
      arg(schema).
      arg(name.lower());
  if (!Describe)
    ret+="CREATE OR REPLACE ";
  bool first=true;
  while(!inf.eof()) {
    char buffer[1024];
    inf>>buffer;
    QString line=QString::fromUtf8(buffer);
    if (first&&!Describe) {
      unsigned int i=0;
      for (i=0;i<line.length();i++) {
	if (line.at(i).isSpace())
	  break;
	ret+=line.at(i);
      }
      while(i<line.length()&&line.at(i).isSpace())
	i++;
      line=line.right(line.length()-i);
      ret+=" ";
      ret+=schema;
      first=false;
    }
    ret+=line;
  }
  if (!Describe)
    ret+="\n/\n\n";
  return ret;
}

void toExtract::describeSource(list<QString> &lst,
			       const QString &schema,const QString &owner,
			       const QString &name,const QString &type)
{
  if (!Code)
    return;

  list<QString> ctx;
  ctx.insert(ctx.end(),schema);
  ctx.insert(ctx.end(),type);
  ctx.insert(ctx.end(),name.lower());
  addDescription(lst,ctx);
  addDescription(lst,ctx,displaySource(schema,owner,name,type).simplifyWhiteSpace());
}

QString toExtract::createFunction(const QString &schema,const QString &owner,const QString &name)
{
  return displaySource(schema,owner,name,"FUNCTION");
}

void toExtract::describeFunction(list<QString> &lst,const QString &schema,
				 const QString &owner,const QString &name)
{
  describeSource(lst,schema,owner,name,"FUNCTION");
}

static toSQL SQLIndexInfo7("toExtract:IndexInfo",
			   "SELECT  'N/A'                           AS partitioned\n"
			   "      , table_name\n"
			   "      , table_owner\n"
			   "      , DECODE(\n"
			   "                uniqueness\n"
			   "               ,'UNIQUE',' UNIQUE'\n"
			   "               ,null\n"
			   "              )\n"
			   "      , null                            AS bitmap\n"
			   "      , null                            AS domain\n"
			   "      , null\n"
			   "      , null\n"
			   "      , null\n"
			   "  FROM all_indexes\n"
			   " WHERE index_name = :nam<char[100]>\n"
			   "   AND owner = :own<char[100]>",
			   "Initial information about an index, same binds and columns",
			   "7.0");
static toSQL SQLIndexInfo("toExtract:IndexInfo",
			  "SELECT partitioned\n"
			  "      , table_name\n"
			  "      , table_owner\n"
			  "      , DECODE(\n"
			  "                uniqueness\n"
			  "               ,'UNIQUE',' UNIQUE'\n"
			  "               ,null\n"
			  "              )\n"
			  "      , DECODE(\n"
			  "                index_type\n"
			  "               ,'BITMAP',' BITMAP'\n"
			  "               ,null\n"
			  "              )\n"
			  "      , DECODE(\n"
			  "                index_type\n"
			  "               ,'DOMAIN','DOMAIN'\n"
			  "               ,null\n"
			  "              )\n"
			  "      , ityp_owner\n"
			  "      , ityp_name\n"
			  "      , parameters\n"
			  "  FROM all_indexes\n"
			  " WHERE index_name = :nam<char[100]>\n"
			  "   AND owner = :own<char[100]>",
			  QString::null,
			  "8.0");

static toSQL SQLIndexSegment("toExtract:IndexSegment",
			     "SELECT  LTRIM(i.degree)\n"
			     "      , LTRIM(i.instances)\n"
			     "      , DECODE(\n"
			     "                i.compression\n"
			     "               ,'ENABLED',i.prefix_length\n"
			     "               ,0\n"
			     "              )                             AS compressed\n"
			     "        -- Physical Properties\n"
			     "      , 'INDEX'                       AS organization\n"
			     "        -- Segment Attributes\n"
			     "      , 'N/A'                         AS cache\n"
			     "      , 'N/A'                         AS pct_used\n"
			     "      , i.pct_free\n"
			     "      , DECODE(\n"
			     "                i.ini_trans\n"
			     "               ,0,1\n"
			     "               ,null,1\n"
			     "               ,i.ini_trans\n"
			     "              )                       AS ini_trans\n"
			     "      , DECODE(\n"
			     "                i.max_trans\n"
			     "               ,0,255\n"
			     "               ,null,255\n"
			     "               ,i.max_trans\n"
			     "              )                       AS max_trans\n"
			     "        -- Storage Clause\n"
			     "      , i.initial_extent\n"
			     "      , i.next_extent\n"
			     "      , i.min_extents\n"
			     "      , DECODE(\n"
			     "                i.max_extents\n"
			     "               ,2147483645,'unlimited'\n"
			     "               ,           i.max_extents\n"
			     "              )                       AS max_extents\n"
			     "      , i.pct_increase\n"
			     "      , NVL(i.freelists,1)\n"
			     "      , NVL(i.freelist_groups,1)\n"
			     "      , LOWER(i.buffer_pool)          AS buffer_pool\n"
			     "      , DECODE(\n"
			     "                i.logging\n"
			     "               ,'NO','NOLOGGING'\n"
			     "               ,     'LOGGING'\n"
			     "              )                       AS logging\n"
			     "      , LOWER(i.tablespace_name)      AS tablespace_name\n"
			     "      , s.blocks\n"
			     "  FROM  all_indexes   i\n"
			     "      , dba_segments  s\n"
			     " WHERE  i.index_name   = :nam<char[100]>\n"
			     "   AND  s.segment_name = i.index_name\n"
			     "   AND  i.owner        = :own<char[100]>\n"
			     "   AND  s.owner        = i.owner",
			     "Get information about how index is stored",
			     "8.1");

static toSQL SQLIndexSegment8("toExtract:IndexSegment",
			      "SELECT  LTRIM(i.degree)\n"
			      "      , LTRIM(i.instances)\n"
			      "      , 0                             AS compressed\n"
			      "        -- Physical Properties\n"
			      "      , 'INDEX'                       AS organization\n"
			      "        -- Segment Attributes\n"
			      "      , 'N/A'                         AS cache\n"
			      "      , 'N/A'                         AS pct_used\n"
			      "      , i.pct_free\n"
			      "      , DECODE(\n"
			      "                i.ini_trans\n"
			      "               ,0,1\n"
			      "               ,null,1\n"
			      "               ,i.ini_trans\n"
			      "              )                       AS ini_trans\n"
			      "      , DECODE(\n"
			      "                i.max_trans\n"
			      "               ,0,255\n"
			      "               ,null,255\n"
			      "               ,i.max_trans\n"
			      "              )                       AS max_trans\n"
			      "        -- Storage Clause\n"
			      "      , i.initial_extent\n"
			      "      , i.next_extent\n"
			      "      , i.min_extents\n"
			      "      , DECODE(\n"
			      "                i.max_extents\n"
			      "               ,2147483645,'unlimited'\n"
			      "               ,           i.max_extents\n"
			      "              )                       AS max_extents\n"
			      "      , i.pct_increase\n"
			      "      , NVL(i.freelists,1)\n"
			      "      , NVL(i.freelist_groups,1)\n"
			      "      , LOWER(i.buffer_pool)          AS buffer_pool\n"
			      "      , DECODE(\n"
			      "                i.logging\n"
			      "               ,'NO','NOLOGGING'\n"
			      "               ,     'LOGGING'\n"
			      "              )                       AS logging\n"
			      "      , LOWER(i.tablespace_name)      AS tablespace_name\n"
			      "      , s.blocks\n"
			      "  FROM  all_indexes   i\n"
			      "      , dba_segments  s\n"
			      " WHERE  i.index_name   = :nam<char[100]>\n"
			      "   AND  s.segment_name = i.index_name\n"
			      "   AND  i.owner        = :own<char[100]>\n"
			      "   AND  s.owner        = i.owner",
			      QString::null,
			      "8.0");

static toSQL SQLIndexSegment7("toExtract:IndexSegment",
			      "SELECT  'N/A'                         AS degree\n"
			      "      , 'N/A'                         AS instances\n"
			      "      , 0                             AS compressed\n"
			      "        -- Physical Properties\n"
			      "      , 'INDEX'                       AS organization\n"
			      "        -- Segment Attributes\n"
			      "      , 'N/A'                         AS cache\n"
			      "      , 'N/A'                         AS pct_used\n"
			      "      , i.pct_free\n"
			      "      , DECODE(\n"
			      "                i.ini_trans\n"
			      "               ,0,1\n"
			      "               ,null,1\n"
			      "               ,i.ini_trans\n"
			      "              )                       AS ini_trans\n"
			      "      , DECODE(\n"
			      "                i.max_trans\n"
			      "               ,0,255\n"
			      "               ,null,255\n"
			      "               ,i.max_trans\n"
			      "              )                       AS max_trans\n"
			      "        -- Storage Clause\n"
			      "      , i.initial_extent\n"
			      "      , i.next_extent\n"
			      "      , i.min_extents\n"
			      "      , DECODE(\n"
			      "                i.max_extents\n"
			      "               ,2147483645,'unlimited'\n"
			      "               ,           i.max_extents\n"
			      "              )                       AS max_extents\n"
			      "      , i.pct_increase\n"
			      "      , NVL(i.freelists,1)\n"
			      "      , NVL(i.freelist_groups,1)\n"
			      "      , 'N/A'                         AS buffer_pool\n"
			      "      , 'N/A'                         AS logging\n"
			      "      , LOWER(i.tablespace_name)      AS tablespace_name\n"
			      "      , s.blocks\n"
			      "  FROM  all_indexes   i\n"
			      "      , dba_segments  s\n"
			      " WHERE  i.index_name   = :nam<char[100]>\n"
			      "   AND  s.segment_name = i.index_name\n"
			      "   AND  i.owner        = :own<char[100]>\n"
			      "   AND  s.owner        = i.owner",
			      QString::null,
			      "7.0");

QString toExtract::createIndex(const QString &schema,const QString &owner,const QString &name)
{
  if (!Indexes)
    return "";

  list<QString> res=toReadQuery(Connection,SQLIndexInfo(Connection),name,owner);
  if (res.size()!=9)
    throw QString("Couldn't find index %1.%2").arg(owner).arg(name);

  QString partitioned=toShift(res);
  QString table      =toShift(res);
  QString tableOwner =toShift(res);
  QString unique     =toShift(res);
  QString bitmap     =toShift(res);
  QString domain     =toShift(res);
  QString domOwner   =toShift(res);
  QString domName    =toShift(res);
  QString domParam   =toShift(res);

  list<QString> storage=toReadQuery(Connection,SQLIndexSegment(Connection),name,owner);
  QString degree     =toShift(storage);
  QString instances  =toShift(storage);
  QString compressed =toShift(storage);

  QString schema2=intSchema(tableOwner);

  QString ret;
  QString sql=QString("CREATE%1%2 INDEX %3%4 ON %5%6\n").
    arg(unique).
    arg(bitmap).
    arg(schema).
    arg(name.lower()).
    arg(schema2).
    arg(table.lower());
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n";
  }
  ret+=sql;
  ret+=indexColumns("",owner,name);
  if (domain=="DOMAIN") {
    ret+=QString("INDEXTYPE IS \"%1\".\"%2\"\nPARAMTERS ('%3');\n\n").
      arg(domOwner).arg(domName).arg(prepareDB(domParam));
    return ret;
  }
  if (Connection.version()>="8.0"&&Parallel) {
    ret+=QString("PARALLEL\n"
		 "(\n"
		 "  DEGREE            %1\n"
		 "  INSTANCES         %2\n"
		 ")\n").
      arg(degree).
      arg(instances);
  }
  if (partitioned == "YES"&&Partition)
    return createPartitionedIndex(schema,owner,name,ret);

  toUnShift(storage,"");
  ret+=segmentAttributes(storage);
  if (!compressed.isEmpty()&&compressed!="0"&&Storage) {
    ret+="COMPRESS            ";
    ret+=compressed;
    ret+="\n";
  }
  ret+=";\n\n";
  return ret;
}

void toExtract::describeIndex(list<QString> &lst,const QString &schema,
			      const QString &owner,const QString &name)
{
  if (Indexes)
    return;

  list<QString> res=toReadQuery(Connection,SQLIndexInfo(Connection),name,owner);
  if (res.size()!=9)
    throw QString("Couldn't find index %1.%2").arg(owner).arg(name);

  QString partitioned=toShift(res);
  QString table      =toShift(res);
  QString tableOwner =toShift(res);
  QString unique     =toShift(res);
  QString bitmap     =toShift(res);
  QString domain     =toShift(res);
  QString domOwner   =toShift(res);
  QString domName    =toShift(res);
  QString domParam   =toShift(res);

  list<QString> storage=toReadQuery(Connection,SQLIndexSegment(Connection),name,owner);
  QString degree     =toShift(storage);
  QString instances  =toShift(storage);
  QString compressed =toShift(storage);

  QString schema2=intSchema(tableOwner);

  list<QString> ctx;
  ctx.insert(ctx.end(),schema2);
  ctx.insert(ctx.end(),"TABLE");
  ctx.insert(ctx.end(),table.lower());
  ctx.insert(ctx.end(),schema);
  ctx.insert(ctx.end(),"INDEX");
  ctx.insert(ctx.end(),name.lower());

  addDescription(lst,ctx);
  describeIndexColumns(lst,ctx,owner,name);
  if (domain=="DOMAIN") {
    addDescription(lst,ctx,"DOMAIN",QString("DOMAINOWNER \"%1\".\"%2\"").
		   arg(domOwner).
		   arg(domName));
    addDescription(lst,ctx,"DOMAIN",QString("PARAMETERS %1").arg(domParam));
    return;
  }
  if (Connection.version()>="8.0"&&Parallel) {
    addDescription(lst,ctx,"PARALLEL",QString("DEGREE %1").arg(degree));
    addDescription(lst,ctx,"PARALLEL",QString("INSTANCES %1").arg(instances));
  }
  if (partitioned == "YES"&&Partition) {
    describePartitionedIndex(lst,ctx,schema,owner,name);
    return;
  }

  toUnShift(storage,"");
  describeAttributes(lst,ctx,storage);
  if (!compressed.isEmpty()&&compressed!="0"&&Storage)
    addDescription(lst,ctx,QString("COMPRESS %1").arg(compressed));
}

static toSQL SQLIndexPartition8("toExtract:IndexPartition",
				"SELECT  -- 8.0 Indexes may partition only by RANGE\n"
				"        i.partitioning_type\n"
				"      , 'N/A'                         AS subpartitioning_type\n"
				"      , i.locality\n"
				"      , 0                             AS compressed\n"
				"        -- Physical Properties\n"
				"      , 'INDEX'                       AS organization\n"
				"        -- Segment Attributes\n"
				"      , 'N/A'                         AS cache\n"
				"      , 'N/A'                         AS pct_used\n"
				"      , i.def_pct_free\n"
				"      , DECODE(\n"
				"                i.def_ini_trans\n"
				"               ,0,1\n"
				"               ,null,1\n"
				"               ,i.def_ini_trans\n"
				"              )                       AS ini_trans\n"
				"      , DECODE(\n"
				"                i.def_max_trans\n"
				"               ,0,255\n"
				"               ,null,255\n"
				"               ,i.def_max_trans\n"
				"              )                       AS max_trans\n"
				"        -- Storage Clause\n"
				"      ,DECODE(\n"
				"               i.def_initial_extent\n"
				"              ,'DEFAULT',s.initial_extent\n"
				"              ,i.def_initial_extent * :bs<char[100]> * 1024\n"
				"             )                        AS initial_extent\n"
				"      ,DECODE(\n"
				"               i.def_next_extent\n"
				"              ,'DEFAULT',s.next_extent\n"
				"              ,i.def_next_extent * :bs<char[100]> * 1024\n"
				"             )                        AS next_extent\n"
				"      , DECODE(\n"
				"                i.def_min_extents\n"
				"               ,'DEFAULT',s.min_extents\n"
				"               ,i.def_min_extents\n"
				"              )                       AS min_extents\n"
				"      , DECODE(\n"
				"                i.def_max_extents\n"
				"               ,'DEFAULT',DECODE(\n"
				"                                  s.max_extents\n"
				"                                 ,2147483645,'unlimited'\n"
				"                                 ,s.max_extents\n"
				"                                )\n"
				"               ,2147483645,'unlimited'\n"
				"               ,i.def_max_extents\n"
				"              )                       AS max_extents\n"
				"      , DECODE(\n"
				"                i.def_pct_increase\n"
				"               ,'DEFAULT',s.pct_increase\n"
				"               ,i.def_pct_increase\n"
				"              )                       AS pct_increase\n"
				"      , DECODE(\n"
				"                i.def_freelists\n"
				"               ,0,1\n"
				"               ,null,1\n"
				"               ,i.def_freelists\n"
				"              )                       AS freelists\n"
				"      , DECODE(\n"
				"                i.def_freelist_groups\n"
				"               ,0,1\n"
				"               ,null,1\n"
				"               ,i.def_freelist_groups\n"
				"              )                       AS freelist_groups\n"
				"      , 'N/A'                         AS buffer_pool\n"
				"      , DECODE(\n"
				"                i.def_logging\n"
				"               ,'NO','NOLOGGING'\n"
				"               ,     'LOGGING'\n"
				"              )                       AS logging\n"
				"      , LOWER(NVL(i.def_tablespace_name,s.tablespace_name))\n"
				"        -- Don't have default blocks, so use larger of initial/next\n"
				"      , GREATEST(\n"
				"                  DECODE(\n"
				"                          i.def_initial_extent\n"
				"                         ,'DEFAULT',s.initial_extent / :bs<char[100]> / 1024\n"
				"                         ,i.def_initial_extent\n"
				"                        )\n"
				"                 ,DECODE(\n"
				"                          i.def_next_extent\n"
				"                         ,'DEFAULT',s.next_extent / :bs<char[100]> / 1024\n"
				"                         ,i.def_next_extent\n"
				"                        )\n"
				"                )                     AS blocks\n"
				" FROM\n"
				"        all_part_indexes  i\n"
				"      , dba_tablespaces   s\n"
				"      , all_part_tables   t\n"
				" WHERE\n"
				"            -- def_tablspace is sometimes NULL in PART_INDEXES,\n"
				"            -- we'll have to go over to the table for the defaults\n"
				"            i.index_name      = :nam<char[100]>\n"
				"        AND t.table_name      = i.table_name\n"
				"        AND s.tablespace_name = t.def_tablespace_name\n"
				"        AND i.owner           = :own<char[100]>\n"
				"        AND n.owner           = i.owner\n"
				"        AND t.owner           = i.owner",
				"Get information about index partitions, "
				"must have same columns and same columns",
				"8.0");

static toSQL SQLIndexPartition("toExtract:IndexPartition",
			       "SELECT  -- Indexes may partition only by RANGE or RANGE/HASH\n"
			       "        i.partitioning_type\n"
			       "      , i.subpartitioning_type\n"
			       "      , i.locality\n"
			       "      , DECODE(\n"
			       "                n.compression\n"
			       "               ,'ENABLED',n.prefix_length\n"
			       "               ,0\n"
			       "              )                             AS compressed\n"
			       "        -- Physical Properties\n"
			       "      , 'INDEX'                       AS organization\n"
			       "        -- Segment Attributes\n"
			       "      , 'N/A'                         AS cache\n"
			       "      , 'N/A'                         AS pct_used\n"
			       "      , i.def_pct_free\n"
			       "      , DECODE(\n"
			       "                i.def_ini_trans\n"
			       "               ,0,1\n"
			       "               ,null,1\n"
			       "               ,i.def_ini_trans\n"
			       "              )                       AS ini_trans\n"
			       "      , DECODE(\n"
			       "                i.def_max_trans\n"
			       "               ,0,255\n"
			       "               ,null,255\n"
			       "               ,i.def_max_trans\n"
			       "              )                       AS max_trans\n"
			       "        -- Storage Clause\n"
			       "      ,DECODE(\n"
			       "               i.def_initial_extent\n"
			       "              ,'DEFAULT',s.initial_extent\n"
			       "              ,i.def_initial_extent * :bs<char[100]> * 1024\n"
			       "             )                        AS initial_extent\n"
			       "      ,DECODE(\n"
			       "               i.def_next_extent\n"
			       "              ,'DEFAULT',s.next_extent\n"
			       "              ,i.def_next_extent * :bs<char[100]> * 1024\n"
			       "             )                        AS next_extent\n"
			       "      , DECODE(\n"
			       "                i.def_min_extents\n"
			       "               ,'DEFAULT',s.min_extents\n"
			       "               ,i.def_min_extents\n"
			       "              )                       AS min_extents\n"
			       "      , DECODE(\n"
			       "                i.def_max_extents\n"
			       "               ,'DEFAULT',DECODE(\n"
			       "                                  s.max_extents\n"
			       "                                 ,2147483645,'unlimited'\n"
			       "                                 ,s.max_extents\n"
			       "                                )\n"
			       "               ,2147483645,'unlimited'\n"
			       "               ,i.def_max_extents\n"
			       "              )                       AS max_extents\n"
			       "      , DECODE(\n"
			       "                i.def_pct_increase\n"
			       "               ,'DEFAULT',s.pct_increase\n"
			       "               ,i.def_pct_increase\n"
			       "              )                       AS pct_increase\n"
			       "      , DECODE(\n"
			       "                i.def_freelists\n"
			       "               ,0,1\n"
			       "               ,null,1\n"
			       "               ,i.def_freelists\n"
			       "              )                       AS freelists\n"
			       "      , DECODE(\n"
			       "                i.def_freelist_groups\n"
			       "               ,0,1\n"
			       "               ,null,1\n"
			       "               ,i.def_freelist_groups\n"
			       "              )                       AS freelist_groups\n"
			       "      , LOWER(i.def_buffer_pool)        AS buffer_pool\n"
			       "      , DECODE(\n"
			       "                i.def_logging\n"
			       "               ,'NO','NOLOGGING'\n"
			       "               ,     'LOGGING'\n"
			       "              )                       AS logging\n"
			       "      , LOWER(NVL(i.def_tablespace_name,s.tablespace_name))\n"
			       "        -- Don't have default blocks, so use larger of initial/next\n"
			       "      , GREATEST(\n"
			       "                  DECODE(\n"
			       "                          i.def_initial_extent\n"
			       "                         ,'DEFAULT',s.initial_extent / :bs<char[100]> / 1024\n"
			       "                         ,i.def_initial_extent\n"
			       "                        )\n"
			       "                 ,DECODE(\n"
			       "                          i.def_next_extent\n"
			       "                         ,'DEFAULT',s.next_extent / :bs<char[100]> / 1024\n"
			       "                         ,i.def_next_extent\n"
			       "                        )\n"
			       "                )                     AS blocks\n"
			       " FROM\n"
			       "        all_part_indexes  i\n"
			       "      , all_indexes       n\n"
			       "      , dba_tablespaces   s\n"
			       "      , all_part_tables   t\n"
			       " WHERE\n"
			       "            -- def_tablspace is sometimes NULL in PART_INDEXES,\n"
			       "            -- we'll have to go over to the table for the defaults\n"
			       "            i.index_name      = :nam<char[100]>\n"
			       "        AND n.index_name      = i.index_name\n"
			       "        AND t.table_name      = i.table_name\n"
			       "        AND s.tablespace_name = t.def_tablespace_name\n"
			       "        AND i.owner           = :own<char[100]>\n"
			       "        AND n.owner           = i.owner\n"
			       "        AND t.owner           = i.owner",
			       QString::null,
			       "8.1");

QString toExtract::createPartitionedIndex(const QString &schema,const QString &owner,
					  const QString &name,const QString &sql)
{
  if (!Partition)
    return "";

  QString ret=sql;
  list<QString> result=toReadQuery(Connection,SQLIndexPartition(Connection),
				   QString::number(BlockSize),owner,name);
  QString partitionType   =toShift(result);
  QString subPartitionType=toShift(result);
  QString locality        =toShift(result);
  QString compressed      =toShift(result);
  toUnShift(result,"");

  ret+=segmentAttributes(result);

  if (!compressed.isEmpty()) {
    ret+="COMPRESS            ";
    ret+=compressed;
    ret+="\n";
  }

  if (locality=="GLOBAL") {
    ret+=
      "GLOBAL PARTITION BY RANGE\n"
      "(\n"
      "    ";
    ret+=partitionKeyColumns(owner,name,"INDEX");
    ret+="\n)\n";
    ret+=rangePartitions(owner,name,subPartitionType,"GLOBAL");
  } else {
    ret+="LOCAL\n";
    if (partitionType=="RANGE")
      ret+=rangePartitions(owner,name,subPartitionType,"LOCAL");
  }
  return ret;
}

void toExtract::describePartitionedIndex(list<QString> &lst,list<QString> &ctx,
					 const QString &schema,const QString &owner,
					 const QString &name)
{
  if (!Partition)
    return;

  list<QString> result=toReadQuery(Connection,SQLIndexPartition(Connection),
				   QString::number(BlockSize),owner,name);
  QString partitionType   =toShift(result);
  QString subPartitionType=toShift(result);
  QString locality        =toShift(result);
  QString compressed      =toShift(result);
  toUnShift(result,"");

  describeAttributes(lst,ctx,result);

  if (!compressed.isEmpty())
    addDescription(lst,ctx,QString("COMPRESS %1").arg(compressed));

  if (locality=="GLOBAL") {
    addDescription(lst,ctx,"GLOBAL PARTITION COLUMNS",partitionKeyColumns(owner,name,"INDEX"));
    describePartitions(lst,ctx,owner,name,subPartitionType,"GLOBAL");
  } else {
    addDescription(lst,ctx,"LOCAL PARTITION");
    if (partitionType=="RANGE")
      describePartitions(lst,ctx,owner,name,subPartitionType,"LOCAL");
  }
}

QString toExtract::partitionKeyColumns(const QString &owner,const QString &name,
				       const QString &type)
{
  return keyColumns(owner,name,type,"PART");
}

QString toExtract::subPartitionKeyColumns(const QString &owner,const QString &name,
					  const QString &type)
{
  return keyColumns(owner,name,type,"SUBPART");
}

static toSQL SQLKeyColumns("toExtract:KeyColumns",	
			   "SELECT  column_name\n"
			   "  FROM all_%1_key_columns\n"
			   " WHERE name           = :nam<char[100]>\n"
			   "   AND owner          = :owner<char[100]>\n"
			   "   AND object_type LIKE :typ<char[100]>||'%'",
			   "Get key columns of partitions, "
			   "must have same binds, columns and %");

QString toExtract::keyColumns(const QString &owner,const QString &name,
			      const QString &type,const QString &table)
{
  otl_stream inf(1,
		 toSQL::string(SQLKeyColumns,Connection).arg(table).utf8(),
		 Connection.connection());
  inf<<owner.utf8()<<name.utf8()<<type.upper().utf8();
  QString ret;
  bool first=true;
  while(!inf.eof()) {
    char buffer[100];
    inf>>buffer;
    if (first)
      first=false;
    else
      ret+="\n  , ";
    ret+=QString::fromUtf8(buffer).lower();
  }
  return ret;
}

static toSQL SQLRangePartitions("toExtract:RangePartitions",
				"SELECT  partition_name\n"
				"      , high_value\n"
				"      , 'N/A'                         AS cache\n"
				"      , 'N/A'                         AS pct_used\n"
				"      , pct_free\n"
				"      , ini_trans\n"
				"      , max_trans\n"
				"        -- Storage Clause\n"
				"      , initial_extent\n"
				"      , next_extent\n"
				"      , min_extent\n"
				"      , DECODE(\n"
				"                max_extent\n"
				"               ,2147483645,'unlimited'\n"
				"               ,           max_extent\n"
				"              )                       AS max_extents\n"
				"      , pct_increase\n"
				"      , NVL(freelists,1)\n"
				"      , NVL(freelist_groups,1)\n"
				"      , LOWER(buffer_pool)\n"
				"      , DECODE(\n"
				"                logging \n"
				"               ,'NO','NOLOGGING'\n"
				"               ,     'LOGGING'\n"
				"              )                       AS logging\n"
				"      , tablespace_name\n"
				"      , leaf_blocks                   AS blocks\n"
				" FROM   all_ind_partitions\n"
				" WHERE      index_name  =  :nam<char[100]>\n"
				"        AND index_owner =  :own<char[100]>\n"
				" ORDER BY partition_name",
				"Get information about partition ranges, must have same binds "
				"and columns",
				"8.1");

static toSQL SQLRangePartitions8("toExtract:RangePartitions",
				 "SELECT  partition_name\n"
				 "      , high_value\n"
				 "      , 'N/A'                         AS cache\n"
				 "      , 'N/A'                         AS pct_used\n"
				 "      , pct_free\n"
				 "      , ini_trans\n"
				 "      , max_trans\n"
				 "        -- Storage Clause\n"
				 "      , initial_extent\n"
				 "      , next_extent\n"
				 "      , min_extent\n"
				 "      , DECODE(\n"
				 "                max_extent\n"
				 "               ,2147483645,'unlimited'\n"
				 "               ,           max_extent\n"
				 "              )                       AS max_extents\n"
				 "      , pct_increase\n"
				 "      , NVL(freelists,1)\n"
				 "      , NVL(freelist_groups,1)\n"
				 "      , 'N/A'                         AS buffer_pool\n"
				 "      , DECODE(\n"
				 "                logging \n"
				 "               ,'NO','NOLOGGING'\n"
				 "               ,     'LOGGING'\n"
				 "              )                       AS logging\n"
				 "      , tablespace_name\n"
				 "      , leaf_blocks                   AS blocks\n"
				 " FROM   all_ind_partitions\n"
				 " WHERE      index_name  =  :nam<char[100]>\n"
				 "        AND index_owner =  :own<char[100]>\n"
				 " ORDER BY partition_name",
				 QString::null,
				 "8.1");

static toSQL SQLIndexSubPartitionName("toExtract:IndexSubPartitionName",
				      "SELECT subpartition_name,\n"
				      "       tablespace_name\n"
				      "  FROM all_ind_subpartitions\n"
				      " WHERE index_name     = :ind<char[100]>\n"
				      "   AND partition_name = :prt<char[100]>\n"
				      "   AND index_owner    = :own<char[100]>\n"
				      " ORDER BY subpartition_name",
				      "Get information about a subpartition, "
				      "must have same binds and columns");

QString toExtract::rangePartitions(const QString &owner,const QString &name,
				   const QString &subPartitionType,const QString &caller)
{
  if (!Partition)
    return "";

  list<QString> result=toReadQuery(Connection,SQLRangePartitions(Connection),
				   name,owner);
  if (result.size()==0||result.size()%18)
    throw QString("Couldn't find partition range %1.%2").arg(owner).arg(name);

  QString ret;

  QString comma="    ";
  while(result.size()>0) {
    QString partition=toShift(result);
    QString highValue=toShift(result);

    list<QString> storage;
    toPush(storage,"      ");
    toPush(storage,"INDEX");
    for (int i=0;i<16;i++)
      toPush(storage,toShift(result));

    ret+=comma;
    ret+="PARTITION ";
    ret+=partition.lower();
    if (caller!="LOCAL")
      ret+=QString(" VALUES LESS THAN\n"
		   "      (\n"
		   "       %1\n"
		   "      )\n").arg(highValue);
    else
      ret+="\n";
    ret+=segmentAttributes(storage);

    if(subPartitionType=="HASH") {
      otl_stream inf(1,
		     SQLIndexSubPartitionName(Connection),
		     Connection.connection());
      inf<<name.utf8()<<partition.utf8()<<owner.utf8();
      ret+=
	"        (\n"
	"            ";
      bool first=true;
      while(!inf.eof()) {
	char buffer[100];
	inf>>buffer;
	QString sub(QString::fromUtf8(buffer));
	inf>>buffer;
	QString tablespace(QString::fromUtf8(buffer));
	if (first)
	  first=false;
	else
	  ret+="\n          , ";
	ret+=sub.lower();
	ret+=" TABLESPACE ";
	ret+=tablespace.lower();
      }
      ret+="\n        )\n";
    }
    comma="  , ";
  }
  ret+=");\n\n";

  return ret;
}

void toExtract::describePartitions(list<QString> &lst,list<QString> &ctx,
				   const QString &owner,const QString &name,
				   const QString &subPartitionType,const QString &caller)
{
  if (!Partition)
    return;

  list<QString> result=toReadQuery(Connection,SQLRangePartitions(Connection),
				   name,owner);
  if (result.size()==0||result.size()%18)
    throw QString("Couldn't find partition range %1.%2").arg(owner).arg(name);

  while(result.size()>0) {
    QString partition=toShift(result);
    QString highValue=toShift(result);

    list<QString> storage;
    toPush(storage,"      ");
    toPush(storage,"INDEX");
    for (int i=0;i<16;i++)
      toPush(storage,toShift(result));

    list<QString> cctx=ctx;
    cctx.insert(cctx.end(),"PARTITION");
    cctx.insert(cctx.end(),partition.lower());

    addDescription(lst,cctx);
    if (caller!="LOCAL")
      addDescription(lst,cctx,"RANGE",QString("VALUES LESS THAN %1").arg(highValue));
    describeAttributes(lst,cctx,storage);

    if(subPartitionType=="HASH") {
      otl_stream inf(1,
		     SQLIndexSubPartitionName(Connection),
		     Connection.connection());
      inf<<name.utf8()<<partition.utf8()<<owner.utf8();
      while(!inf.eof()) {
	char buffer[100];
	inf>>buffer;
	QString sub(QString::fromUtf8(buffer));
	inf>>buffer;
	QString tablespace(QString::fromUtf8(buffer));
	addDescription(lst,cctx,"HASH",sub.lower(),
		       QString("TABLESPACE %1").arg(tablespace.lower()));
      }
    }
  }
}

QString toExtract::createMaterializedView(const QString &schema,const QString &owner,
					  const QString &name)
{
  return createMView(schema,owner,name,"MATERIALIZED VIEW");
}

QString toExtract::createMaterializedViewLog(const QString &schema,const QString &owner,
					     const QString &name)
{
  return createMViewLog(schema,owner,name,"MATERIALIZED VIEW");
}

void toExtract::describeMaterializedView(list<QString> &lst,const QString &schema,
					 const QString &owner,const QString &name)
{
  describeMView(lst,schema,owner,name,"MATERIALIZED VIEW");
}

void toExtract::describeMaterializedViewLog(list<QString> &lst,const QString &schema,
					    const QString &owner,const QString &name)
{
  describeMViewLog(lst,schema,owner,name,"MATERIALIZED VIEW LOG");
}

static toSQL SQLMViewInfo("toExtract:MaterializedViewInfo",
			  "       SELECT\n"
			  "              m.container_name\n"
			  "            , DECODE(\n"
			  "                      m.build_mode\n"
			  "                     ,'YES','USING PREBUILT TABLE'\n"
			  "                     ,DECODE(\n"
			  "                              m.last_refresh_date\n"
			  "                             ,null,'BUILD DEFERRED'\n"
			  "                             ,'BUILD IMMEDIATE'\n"
			  "                            )\n"
			  "                    )                                  AS build_mode\n"
			  "            , DECODE(\n"
			  "                      m.refresh_method\n"
			  "                     ,'NEVER','NEVER REFRESH'\n"
			  "                     ,'REFRESH ' || m.refresh_method\n"
			  "                    )                                  AS refresh_method\n"
			  "            , DECODE(\n"
			  "                      m.refresh_mode\n"
			  "                     ,'NEVER',null\n"
			  "                     ,'ON ' || m.refresh_mode\n"
			  "                    )                                  AS refresh_mode\n"
			  "            , TO_CHAR(s.start_with, 'DD-MON-YYYY HH24:MI:SS')\n"
			  "                                                       AS start_with\n"
			  "            , s.next\n"
			  "            , DECODE(\n"
			  "                      s.refresh_method\n"
			  "                     ,'PRIMARY KEY','WITH  PRIMARY KEY'\n"
			  "                     ,'ROWID'      ,'WITH  ROWID'\n"
			  "                     ,null\n"
			  "                    )                                  AS using_pk\n"
			  "            , s.master_rollback_seg\n"
			  "            , DECODE(\n"
			  "                      m.updatable\n"
			  "                     ,'N',null\n"
			  "                     ,DECODE(\n"
			  "                              m.rewrite_enabled\n"
			  "                             ,'Y','FOR UPDATE ENABLE QUERY REWRITE'\n"
			  "                             ,'N','FOR UPDATE DISABLE QUERY REWRITE'\n"
			  "                            )\n"
			  "                    )                                  AS updatable\n"
			  "            , s.query\n"
			  "       FROM\n"
			  "              all_mviews     m\n"
			  "            , all_snapshots  s\n"
			  "       WHERE\n"
			  "                  m.mview_name  = :nam<char[100]>\n"
			  "              AND s.name        = m.mview_name\n"
			  "              AND m.owner       = :own<char[100]>\n"
			  "              AND s.owner       = m.owner",
			  "Get information about materialized view, must have same columns and binds");

static toSQL SQLIndexName("toExtract:TableIndexes",
			  "SELECT index_name\n"
			  "  FROM all_indexes\n"
			  " WHERE table_name = :nam<char[100]>\n"
			  "   AND owner = own<char[100]>",
			  "Get indexes available to a table, must have same binds and columns");

QString toExtract::createMView(const QString &schema,const QString &owner,
			       const QString &name,const QString &type)
{
  list<QString> result=toReadQuery(Connection,SQLMViewInfo(Connection),name,owner);
  if (result.size()==0)
    throw QString("Couldn't find materialised table %1.%2").arg(owner.lower()).arg(name.lower());
  QString table        =toShift(result);
  QString buildMode    =toShift(result);
  QString refreshMethod=toShift(result);
  QString refreshMode  =toShift(result);
  QString startWith    =toShift(result);
  QString next         =toShift(result);
  QString usingPK      =toShift(result);
  QString masterRBSeg  =toShift(result);
  QString updatable    =toShift(result);
  QString query        =toShift(result);

  otl_stream inf(1,
		 SQLIndexName(Connection),
		 Connection.connection());
  inf<<table.utf8()<<owner.utf8();
  char buffer[100];
  inf>>buffer;
  QString index(QString::fromUtf8(buffer));

  QString ret;
  QString sql=QString("CREATE %1 %2%3\n").arg(type).arg(schema).arg(name.lower());
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n";
  }
  ret+=sql;
  ret+=createMViewTable(schema,owner,table);
  ret+=buildMode;
  ret+="\nUSING INDEX\n";
  ret+=createMViewIndex(schema,owner,index);

  if (refreshMethod!="NEVER REFRESH") {
    if (!startWith.isEmpty())
      ret+=QString("START WITH TO_DATE('%1','DD-MM-YYYY HH24:MI:SS')\n").
	arg(startWith);
    if (!next.isEmpty())
      ret+=QString("NEXT  %1\n").
	arg(next);
    if (!usingPK.isEmpty()) {
      ret+=usingPK;
      ret+="\n";
    }
    if (!masterRBSeg.isEmpty()&&Storage)
      ret+=QString("USING MASTER ROLLBACK SEGMENT %1\n").
	arg(masterRBSeg.lower());
  }

  if (!updatable.isEmpty()) {
    ret+=updatable;
    ret+="\n";
  }
  ret+="AS\n";
  ret+=query;
  ret+=";\n\n";
  return ret;
}

void toExtract::describeMView(list<QString> &lst,
			      const QString &schema,const QString &owner,
			      const QString &name,const QString &type)
{
  list<QString> result=toReadQuery(Connection,SQLMViewInfo(Connection),name,owner);
  if (result.size()==0)
    throw QString("Couldn't find materialised table %1.%2").arg(owner.lower()).arg(name.lower());
  QString table        =toShift(result);
  QString buildMode    =toShift(result);
  QString refreshMethod=toShift(result);
  QString refreshMode  =toShift(result);
  QString startWith    =toShift(result);
  QString next         =toShift(result);
  QString usingPK      =toShift(result);
  QString masterRBSeg  =toShift(result);
  QString updatable    =toShift(result);
  QString query        =toShift(result);

  otl_stream inf(1,
		 SQLIndexName(Connection),
		 Connection.connection());
  inf<<table.utf8()<<owner.utf8();
  char buffer[100];
  inf>>buffer;
  QString index(QString::fromUtf8(buffer));

  list<QString> ctx;
  ctx.insert(ctx.end(),schema);
  ctx.insert(ctx.end(),type);
  ctx.insert(ctx.end(),name.lower());

  describeMViewTable(lst,ctx,schema,owner,table);
  addDescription(lst,ctx,buildMode);
  ctx.insert(ctx.end(),"INDEX");
  ctx.insert(ctx.end(),index.lower());
  describeMViewIndex(lst,ctx,schema,owner,index);

  if (refreshMethod!="NEVER REFRESH") {
    addDescription(lst,ctx,QString("START WITH %1").arg(startWith));
    addDescription(lst,ctx,QString("NEXT %1").arg(next));
    addDescription(lst,ctx,usingPK);
    if (!masterRBSeg.isEmpty()&&Storage)
      addDescription(lst,ctx,QString("USING MASTER ROLLBACK SEGMENT %1").
		     arg(masterRBSeg.lower()));
  }

  if (!updatable.isEmpty())
    addDescription(lst,ctx,updatable);
  addDescription(lst,ctx,QString("AS %1").arg(query));
}

QString toExtract::createMViewTable(const QString &schema,const QString &owner,
				    const QString &name)
{
  if (!Storage||!Parallel)
    return "";
  IsASnapTable=true;

  static QRegExp parallel("^PARALLEL");

  bool started=false;
  bool done=false;

  QString initial=createTable(schema,owner,name);
  QStringList linesIn=QStringList::split("\n",initial,true);
  QString ret;

  for(QStringList::Iterator i=linesIn.begin();i!=linesIn.end()&&!done;i++) {
    if(parallel.match(*i)>=0)
      started=true;
    if (started) {
      QString line=*i;
      if (line.length()>0&&line.at(line.length()-1)==';') {
	line.truncate(line.length()-1);
	done=true;
      }
      if (line.length()) {
	ret+=line;
	ret+="\n";
      }
    }
  }

  IsASnapTable=false;
  return ret;
}

void toExtract::describeMViewTable(list<QString> &lst,list<QString> &ctx,
				   const QString &schema,const QString &owner,const QString &name)
{
  if (!Storage||!Parallel)
    return;
  IsASnapTable=true;
  //                        Schema        Table         Name
  static QRegExp parallel("^[^\001]+[\001][^\001]+[\001][^\001]+[\001]PARALLEL");

  bool started=false;
  bool done=false;

  list<QString> tbllst;
  describeTable(tbllst,schema,owner,name);
  QString ret;

  for(list<QString>::iterator i=tbllst.begin();i!=tbllst.end()&&!done;i++) {
    if(parallel.match(*i)>=0)
      started=true;
    if (started)
      lst.insert(lst.end(),reContext(ctx,3,*i));
  }

  IsASnapTable=false;
}


QString toExtract::createMViewIndex(const QString &schema,const QString &owner,
				    const QString &name)
{
  if (!Storage||!Parallel)
    return "";
  IsASnapIndex=true;

  static QRegExp start("^INITRANS");
  static QRegExp ignore("LOGGING");

  bool started=false;
  bool done=false;

  QString initial=createIndex(schema,owner,name);
  QStringList linesIn=QStringList::split("\n",initial,true);
  QString ret;

  for(QStringList::Iterator i=linesIn.begin();i!=linesIn.end()&&!done;i++) {
    if(start.match(*i)>=0)
      started=true;
    if (started) {
      QString line=*i;
      if (line.length()>0&&line.at(line.length()-1)==';') {
	line.truncate(line.length()-1);
	done=true;
      }
      if (ignore.match(line)<0&&line.length()>0) {
	ret+=line;
	ret+="\n";
      }
    }
  }

  IsASnapIndex=false;
  return ret;
}

void toExtract::describeMViewIndex(list<QString> &lst,list<QString> &ctx,
				   const QString &schema,const QString &owner,const QString &name)
{
  if (!Storage||!Parallel)
    return;
  IsASnapIndex=true;

  static QRegExp start("^[^\001]+[\001][^\001]+[\001][^\001]+[\001]INITTRANS");
  static QRegExp ignore("^[^\001]+[\001][^\001]+[\001][^\001]+[\001]LOGGING");

  bool started=false;
  bool done=false;

  list<QString> tbllst;
  describeIndex(tbllst,schema,owner,name);
  QString ret;

  for(list<QString>::iterator i=tbllst.begin();i!=tbllst.end()&&!done;i++) {
    if(start.match(*i)>=0)
      started=true;
    if (started)
      lst.insert(lst.end(),reContext(ctx,3,*i));
  }

  IsASnapIndex=false;
}

static toSQL SQLSnapshotInfo("toExtract:SnapshotInfo",
			     "SELECT log_table,\n"
			     "       rowids,\n"
			     "       primary_key,\n"
			     "       filter_columns\n"
			     "  FROM all_snapshot_logs\n"
			     "   AND master = :nam<char[100]>\n"
			     "   AND log_owner = :own<char[100]>",
			     "Get information about snapshot or materialized view log, "
			     "must have same binds and columns");

static toSQL SQLSnapshotColumns("toExtract:SnapshotColumns",
				"SELECT\n"
				"        column_name\n"
				" FROM\n"
				"        dba_snapshot_log_filter_cols\n"
				" WHERE\n"
				"            name  = :nam<char[100]>\n"
				"        AND owner = :own<char[100]>\n"
				" MINUS\n"
				" SELECT\n"
				"        column_name\n"
				" FROM\n"
				"        all_cons_columns  c\n"
				"      , all_constraints   d\n"
				" WHERE\n"
				"            d.table_name      = :nam<char[100]>\n"
				"        AND d.constraint_type = 'P'\n"
				"        AND c.table_name      = d.table_name\n"
				"        AND c.constraint_name = d.constraint_name\n"
				"        AND d.owner           = :own<char[100]>\n"
				"        AND c.owner           = d.owner",
				"Get columns of snapshot log, must have same columns and binds");

QString toExtract::createMViewLog(const QString &schema,const QString &owner,
				  const QString &name,const QString &type)
{
  list<QString> result=toReadQuery(Connection,SQLSnapshotInfo(Connection),name,owner);
  if (result.size()!=4)
    throw QString("Couldn't find log %1.%2").arg(owner).arg(name);

  QString table        =toShift(result);
  QString rowIds       =toShift(result);
  QString primaryKey   =toShift(result);
  QString filterColumns=toShift(result);

  QString ret;
  QString sql=QString("CREATE %1 LOG ON %2%3\n").
    arg(type).
    arg(schema).
    arg(name.lower());
  if (Prompt) {
    QString ret="PROMPT ";
    ret+=sql;
    ret+="\n";
  }
  ret+=sql;
  ret+=createMViewTable(schema,owner,table);

  if (rowIds=="YES" && primaryKey=="YES")
    ret+="WITH PRIMARY KEY, ROWID ";
  else if (rowIds=="YES")
    ret+="WITH ROWID ";
  else if (primaryKey=="YES")
    ret+="WITH PRIMARY KEY";

  otl_stream inf(1,
		 SQLSnapshotColumns(Connection),
		 Connection.connection());
  bool first=true;
  if (!inf.eof()) {
    ret+="\n(\n    ";
    while(!inf.eof()) {
      char buffer[100];
      if (first)
	first=false;
      else
	ret+="\n  , ";
      inf>>buffer;
      ret+=QString::fromUtf8(buffer).lower();
    }
    ret+=")";
  }
  ret+=";\n\n";
  return ret;
}

void toExtract::describeMViewLog(list<QString> &lst,
				 const QString &schema,const QString &owner,
				 const QString &name,const QString &type)
{
  list<QString> result=toReadQuery(Connection,SQLSnapshotInfo(Connection),name,owner);
  if (result.size()!=4)
    throw QString("Couldn't find log %1.%2").arg(owner).arg(name);

  QString table        =toShift(result);
  QString rowIds       =toShift(result);
  QString primaryKey   =toShift(result);
  QString filterColumns=toShift(result);

  list<QString> ctx;
  ctx.insert(ctx.end(),schema);
  ctx.insert(ctx.end(),type);
  ctx.insert(ctx.end(),name.lower());

  describeMViewTable(lst,ctx,schema,owner,table);


  if (rowIds=="YES" && primaryKey=="YES")
    ctx.insert(ctx.end(),"WITH PRIMARY KEY, ROWID");
  else if (rowIds=="YES")
    ctx.insert(ctx.end(),"WITH ROWID");
  else if (primaryKey=="YES")
    ctx.insert(ctx.end(),"WITH PRIMARY KEY");
  addDescription(lst,ctx);

  otl_stream inf(1,
		 SQLSnapshotColumns(Connection),
		 Connection.connection());
  bool first=true;
  if (!inf.eof()) {
    QString col="(";
    while(!inf.eof()) {
      char buffer[100];
      if (first)
	first=false;
      else
	col+=",";
      inf>>buffer;
      col+=QString::fromUtf8(buffer).lower();
    }
    col+=")";
    addDescription(lst,ctx,col);
  }
}

static toSQL SQLTableType("toExtract:TableType",
			  "SELECT partitioned,\n"
			  "      iot_type\n"
			  " FROM all_tables\n"
			  "WHERE table_name = :nam<char[100]>\n"
			  "  AND owner = :own<char[100]>",
			  "Get table type, must have same columns and binds",
			  "8.0");

static toSQL SQLTableType7("toExtract:TableType",
			   "SELECT 'NO',\n"
			   "      'NOT IOT'\n"
			   " FROM all_tables\n"
			   "WHERE table_name = :nam<char[100]>\n"
			   "  AND owner = :own<char[100]>",
			   QString::null,
			   "7.0");

static toSQL SQLTableInfo("toExtract:TableInfo",
			  "SELECT\n"
			  "        -- Table Properties\n"
			  "        DECODE(\n"
			  "                t.monitoring\n"
			  "               ,'NO','NOMONITORING'\n"
			  "               ,     'MONITORING'\n"
			  "              )                       AS monitoring\n"
			  "      , 'N/A'                         AS table_name\n"
			  "        -- Parallel Clause\n"
			  "      , LTRIM(t.degree)\n"
			  "      , LTRIM(t.instances)\n"
			  "        -- Physical Properties\n"
			  "      , DECODE(\n"
			  "                t.iot_type\n"
			  "               ,'IOT','INDEX'\n"
			  "               ,      'HEAP'\n"
			  "              )                       AS organization\n"
			  "        -- Segment Attributes\n"
			  "      , DECODE(\n"
			  "                LTRIM(t.cache)\n"
			  "               ,'Y','CACHE'\n"
			  "               ,    'NOCACHE'\n"
			  "              )\n"
			  "      , t.pct_used\n"
			  "      , t.pct_free\n"
			  "      , DECODE(\n"
			  "                t.ini_trans\n"
			  "               ,0,1\n"
			  "               ,null,1\n"
			  "               ,t.ini_trans\n"
			  "              )                       AS ini_trans\n"
			  "      , DECODE(\n"
			  "                t.max_trans\n"
			  "               ,0,255\n"
			  "               ,null,255\n"
			  "               ,t.max_trans\n"
			  "              )                       AS max_trans\n"
			  "        -- Storage Clause\n"
			  "      , t.initial_extent\n"
			  "      , t.next_extent\n"
			  "      , t.min_extents\n"
			  "      , DECODE(\n"
			  "                t.max_extents\n"
			  "               ,2147483645,'unlimited'\n"
			  "               ,           t.max_extents\n"
			  "              )                       AS max_extents\n"
			  "      , NVL(t.pct_increase,0)\n"
			  "      , NVL(t.freelists,1)\n"
			  "      , NVL(t.freelist_groups,1)\n"
			  "      , LOWER(t.buffer_pool)          AS buffer_pool\n"
			  "      , DECODE(\n"
			  "                t.logging\n"
			  "               ,'NO','NOLOGGING'\n"
			  "               ,     'LOGGING'\n"
			  "              )                       AS logging\n"
			  "      , LOWER(t.tablespace_name)      AS tablespace_name\n"
			  "      , s.blocks - NVL(t.empty_blocks,0)\n"
			  " FROM\n"
			  "        all_tables    t\n"
			  "      , dba_segments  s\n"
			  " WHERE\n"
			  "            t.table_name   = :nam<char[100]>\n"
			  "        AND t.table_name   = s.segment_name\n"
			  "        AND s.owner        = :own<char[100]>\n"
			  "        AND t.owner        = s.owner",
			  "Get information about a vanilla table, must have same binds and columns",
			  "8.1");

static toSQL SQLTableInfo8("toExtract:TableInfo",
			   "SELECT\n"
			   "        -- Table Properties\n"
			   "        'N/A'                         AS monitoring\n"
			   "      , 'N/A'                         AS table_name\n"
			   "        -- Parallel Clause\n"
			   "      , LTRIM(t.degree)\n"
			   "      , LTRIM(t.instances)\n"
			   "        -- Physical Properties\n"
			   "      , DECODE(\n"
			   "                t.iot_type\n"
			   "               ,'IOT','INDEX'\n"
			   "               ,      'HEAP'\n"
			   "              )                       AS organization\n"
			   "        -- Segment Attributes\n"
			   "      , DECODE(\n"
			   "                LTRIM(t.cache)\n"
			   "               ,'Y','CACHE'\n"
			   "               ,    'NOCACHE'\n"
			   "              )\n"
			   "      , t.pct_used\n"
			   "      , t.pct_free\n"
			   "      , DECODE(\n"
			   "                t.ini_trans\n"
			   "               ,0,1\n"
			   "               ,null,1\n"
			   "               ,t.ini_trans\n"
			   "              )                       AS ini_trans\n"
			   "      , DECODE(\n"
			   "                t.max_trans\n"
			   "               ,0,255\n"
			   "               ,null,255\n"
			   "               ,t.max_trans\n"
			   "              )                       AS max_trans\n"
			   "        -- Storage Clause\n"
			   "      , t.initial_extent\n"
			   "      , t.next_extent\n"
			   "      , t.min_extents\n"
			   "      , DECODE(\n"
			   "                t.max_extents\n"
			   "               ,2147483645,'unlimited'\n"
			   "               ,           t.max_extents\n"
			   "              )                       AS max_extents\n"
			   "      , NVL(t.pct_increase,0)\n"
			   "      , NVL(t.freelists,1)\n"
			   "      , NVL(t.freelist_groups,1)\n"
			   "      , 'N/A'                         AS buffer_pool\n"
			   "      , DECODE(\n"
			   "                t.logging\n"
			   "               ,'NO','NOLOGGING'\n"
			   "               ,     'LOGGING'\n"
			   "              )                       AS logging\n"
			   "      , LOWER(t.tablespace_name)      AS tablespace_name\n"
			   "      , s.blocks - NVL(t.empty_blocks,0)\n"
			   " FROM\n"
			   "        all_tables    t\n"
			   "      , dba_segments  s\n"
			   " WHERE\n"
			   "            t.table_name   = :nam<char[100]>\n"
			   "        AND t.table_name   = s.segment_name\n"
			   "        AND s.owner        = :own<char[100]>\n"
			   "        AND t.owner        = s.owner",
			   QString::null,
			   "8.0");

static toSQL SQLTableInfo7("toExtract:TableInfo",
			   "SELECT\n"
			   "        -- Table Properties\n"
			   "        'N/A'                         AS monitoring\n"
			   "      , 'N/A'                         AS table_name\n"
			   "        -- Parallel Clause\n"
			   "      , LTRIM(t.degree)\n"
			   "      , LTRIM(t.instances)\n"
			   "        -- Physical Properties\n"
			   "      , 'N/A'                         AS organization\n"
			   "        -- Segment Attributes\n"
			   "      , DECODE(\n"
			   "                LTRIM(t.cache)\n"
			   "               ,'Y','CACHE'\n"
			   "               ,    'NOCACHE'\n"
			   "              )\n"
			   "      , t.pct_used\n"
			   "      , t.pct_free\n"
			   "      , DECODE(\n"
			   "                t.ini_trans\n"
			   "               ,0,1\n"
			   "               ,null,1\n"
			   "               ,t.ini_trans\n"
			   "              )                       AS ini_trans\n"
			   "      , DECODE(\n"
			   "                t.max_trans\n"
			   "               ,0,255\n"
			   "               ,null,255\n"
			   "               ,t.max_trans\n"
			   "              )                       AS max_trans\n"
			   "        -- Storage Clause\n"
			   "      , t.initial_extent\n"
			   "      , t.next_extent\n"
			   "      , t.min_extents\n"
			   "      , DECODE(\n"
			   "                t.max_extents\n"
			   "               ,2147483645,'unlimited'\n"
			   "               ,           t.max_extents\n"
			   "              )                       AS max_extents\n"
			   "      , NVL(t.pct_increase,0)\n"
			   "      , NVL(t.freelists,1)\n"
			   "      , NVL(t.freelist_groups,1)\n"
			   "      , 'N/A'                         AS buffer_pool\n"
			   "      , 'N/A'                         AS logging\n"
			   "      , LOWER(t.tablespace_name)      AS tablespace_name\n"
			   "      , s.blocks - NVL(t.empty_blocks,0)\n"
			   " FROM\n"
			   "        all_tables    t\n"
			   "      , dba_segments  s\n"
			   " WHERE\n"
			   "            t.table_name   = :nam<char[100]>\n"
			   "        AND t.table_name   = s.segment_name\n"
			   "        AND s.owner        = :own<char[100]>\n"
			   "        AND t.owner        = s.owner",
			   QString::null,
			   "7.0");

QString toExtract::createTable(const QString &schema,const QString &owner,const QString &name)
{
  otl_stream inf(1,
		 SQLTableType(Connection),
		 Connection.connection());
  inf<<name.utf8()<<owner.utf8();
  if (inf.eof())
    throw QString("Couldn't find table %1.%2").arg(owner).arg(name);

  char buffer[100];
  inf>>buffer;
  QString partitioned(QString::fromUtf8(buffer));
  inf>>buffer;
  QString iot_type(QString::fromUtf8(buffer));

  if (iot_type=="IOT") {
    if (partitioned=="YES"&&Partition)
      return createPartitionedIOT(schema,owner,name);
    else
      return createIOT(schema,owner,name);
  } else if (partitioned=="YES"&&Partition)
    return createPartitionedTable(schema,owner,name);

  list<QString> result=toReadQuery(Connection,SQLTableInfo(Connection),name,owner);
  QString ret=createTableText(result,schema,owner,name);
  ret+=";\n\n";
  ret+=createComments(schema,owner,name);
  return ret;
}

void toExtract::describeTable(list<QString> &lst,
			      const QString &schema,const QString &owner,const QString &name)
{
  otl_stream inf(1,
		 SQLTableType(Connection),
		 Connection.connection());
  inf<<name.utf8()<<owner.utf8();
  if (inf.eof())
    throw QString("Couldn't find table %1.%2").arg(owner).arg(name);

  char buffer[100];
  inf>>buffer;
  QString partitioned(QString::fromUtf8(buffer));
  inf>>buffer;
  QString iot_type(QString::fromUtf8(buffer));

  list<QString> ctx;
  ctx.insert(ctx.end(),schema);
  ctx.insert(ctx.end(),"TABLE");
  ctx.insert(ctx.end(),name.lower());

  if (iot_type=="IOT"&&Indexes) {
    if (partitioned=="YES"&&Partition)
      describePartitionedIOT(lst,ctx,schema,owner,name);
    else
      describeIOT(lst,ctx,schema,owner,name);
    return;
  } else if (partitioned=="YES"&&Partition) {
    describePartitionedTable(lst,ctx,schema,owner,name);
    return;
  }

  list<QString> result=toReadQuery(Connection,SQLTableInfo(Connection),name,owner);
  describeTableText(lst,ctx,result,schema,owner,name);
  describeComments(lst,ctx,schema,owner,name);
}

static toSQL SQLOverflowInfo("toExtract:OverflowInfo",
			     "SELECT\n"
			     "        '  '\n"
			     "      , 'N/A'\n"
			     "        -- Segment Attributes\n"
			     "      , DECODE(\n"
			     "                LTRIM(t.cache)\n"
			     "               ,'Y','CACHE'\n"
			     "               ,    'NOCACHE'\n"
			     "              )\n"
			     "      , t.pct_used\n"
			     "      , t.pct_free\n"
			     "      , DECODE(\n"
			     "                t.ini_trans\n"
			     "               ,0,1\n"
			     "               ,null,1\n"
			     "               ,t.ini_trans\n"
			     "              )                       AS ini_trans\n"
			     "      , DECODE(\n"
			     "                t.max_trans\n"
			     "               ,0,255\n"
			     "               ,null,255\n"
			     "               ,t.max_trans\n"
			     "              )                       AS max_trans\n"
			     "        -- Storage Clause\n"
			     "      , t.initial_extent\n"
			     "      , t.next_extent\n"
			     "      , t.min_extents\n"
			     "      , DECODE(\n"
			     "                t.max_extents\n"
			     "               ,2147483645,'unlimited'\n"
			     "               ,           t.max_extents\n"
			     "              )                       AS max_extents\n"
			     "      , NVL(t.pct_increase,0)\n"
			     "      , NVL(t.freelists,1)\n"
			     "      , NVL(t.freelist_groups,1)\n"
			     "      , LOWER(t.buffer_pool)          AS buffer_pool\n"
			     "      , DECODE(\n"
			     "                t.logging\n"
			     "               ,'NO','NOLOGGING'\n"
			     "               ,     'LOGGING'\n"
			     "              )                       AS logging\n"
			     "      , LOWER(t.tablespace_name)      AS tablespace_name\n"
			     "      , s.blocks - NVL(t.empty_blocks,0)\n"
			     " FROM\n"
			     "        all_tables    t\n"
			     "      , dba_segments  s\n"
			     " WHERE\n"
			     "            t.iot_name     = :nam<char[100]>\n"
			     "        AND t.table_name   = s.segment_name\n"
			     "        AND s.owner        = :own<char[100]>\n"
			     "        AND t.owner        = s.owner",
			     "Get information about overflow segment for table, must have same binds and columns",
			     "8.0");

static toSQL SQLPartitionedIOTInfo("toExtract:PartitionedIOTInfo",
				   "SELECT\n"
				   "        -- Table Properties\n"
				   "        DECODE(\n"
				   "                t.monitoring\n"
				   "               ,'NO','NOMONITORING'\n"
				   "               ,     'MONITORING'\n"
				   "              )                       AS monitoring\n"
				   "      , t.table_name\n"
				   "        -- Parallel Clause\n"
				   "      , LTRIM(t.degree)               AS degree\n"
				   "      , LTRIM(t.instances)            AS instances\n"
				   "        -- Physical Properties\n"
				   "      , 'INDEX'                       AS organization\n"
				   "        -- Segment Attributes\n"
				   "      , DECODE(\n"
				   "                LTRIM(t.cache)\n"
				   "               ,'Y','CACHE'\n"
				   "               ,    'NOCACHE'\n"
				   "              )                       AS cache\n"
				   "      , 'N/A'                         AS pct_used\n"
				   "      , p.def_pct_free                AS pct_free\n"
				   "      , p.def_ini_trans               AS ini_trans\n"
				   "      , p.def_max_trans               AS max_trans\n"
				   "        -- Storage Clause\n"
				   "      ,DECODE(\n"
				   "               p.def_initial_extent\n"
				   "              ,'DEFAULT',s.initial_extent\n"
				   "              ,p.def_initial_extent * :bs<char[100]> * 1024\n"
				   "             )                        AS initial_extent\n"
				   "      ,DECODE(\n"
				   "               p.def_next_extent\n"
				   "              ,'DEFAULT',s.next_extent\n"
				   "              ,p.def_next_extent * :bs<char[100]> * 1024\n"
				   "             )                        AS next_extent\n"
				   "      , DECODE(\n"
				   "                p.def_min_extents\n"
				   "               ,'DEFAULT',s.min_extents\n"
				   "               ,p.def_min_extents\n"
				   "              )                       AS min_extents\n"
				   "      , DECODE(\n"
				   "                p.def_max_extents\n"
				   "               ,'DEFAULT',DECODE(\n"
				   "                                  s.max_extents\n"
				   "                                 ,2147483645,'unlimited'\n"
				   "                                 ,s.max_extents\n"
				   "                                )\n"
				   "               ,2147483645,'unlimited'\n"
				   "               ,p.def_max_extents\n"
				   "              )                       AS max_extents\n"
				   "      , DECODE(\n"
				   "                p.def_pct_increase\n"
				   "               ,'DEFAULT',s.pct_increase\n"
				   "               ,p.def_pct_increase\n"
				   "              )                       AS pct_increase\n"
				   "      , DECODE(\n"
				   "                p.def_freelists\n"
				   "               ,0,1\n"
				   "               ,NVL(p.def_freelists,1)\n"
				   "              )                       AS freelists\n"
				   "      , DECODE(\n"
				   "                p.def_freelist_groups\n"
				   "               ,0,1\n"
				   "               ,NVL(p.def_freelist_groups,1)\n"
				   "              )                       AS freelist_groups\n"
				   "      , LOWER(p.def_buffer_pool)      AS buffer_pool\n"
				   "      , DECODE(\n"
				   "                p.def_logging \n"
				   "               ,'NO','NOLOGGING'\n"
				   "               ,     'LOGGING'\n"
				   "              )                       AS logging\n"
				   "      , LOWER(p.def_tablespace_name)  AS tablespace_name\n"
				   "      , t.blocks - NVL(t.empty_blocks,0)\n"
				   " FROM\n"
				   "        all_all_tables    t\n"
				   "      , all_part_indexes  p\n"
				   "      , dba_tablespaces   s\n"
				   " WHERE\n"
				   "            t.table_name      = :name<char[100]>\n"
				   "        AND p.table_name      = t.table_name\n"
				   "        AND s.tablespace_name = p.def_tablespace_name\n"
				   "        AND t.owner           = :own<char[100]>\n"
				   "        AND p.owner           = t.owner",
				   "Get information about a partitioned indexed organized table, "
				   "must have same columns and binds",
				   "8.1");

static toSQL SQLPartitionedIOTInfo8("toExtract:PartitionedIOTInfo",
				    "SELECT\n"
				    "        -- Table Properties\n"
				    "        'N/A'                         AS monitoring\n"
				    "      , t.table_name\n"
				    "        -- Parallel Clause\n"
				    "      , LTRIM(t.degree)               AS degree\n"
				    "      , LTRIM(t.instances)            AS instances\n"
				    "        -- Physical Properties\n"
				    "      , 'INDEX'                       AS organization\n"
				    "        -- Segment Attributes\n"
				    "      , DECODE(\n"
				    "                LTRIM(t.cache)\n"
				    "               ,'Y','CACHE'\n"
				    "               ,    'NOCACHE'\n"
				    "              )                       AS cache\n"
				    "      , 'N/A'                         AS pct_used\n"
				    "      , p.def_pct_free                AS pct_free\n"
				    "      , p.def_ini_trans               AS ini_trans\n"
				    "      , p.def_max_trans               AS max_trans\n"
				    "        -- Storage Clause\n"
				    "      ,DECODE(\n"
				    "               p.def_initial_extent\n"
				    "              ,'DEFAULT',s.initial_extent\n"
				    "              ,p.def_initial_extent * :bs<char[100]> * 1024\n"
				    "             )                        AS initial_extent\n"
				    "      ,DECODE(\n"
				    "               p.def_next_extent\n"
				    "              ,'DEFAULT',s.next_extent\n"
				    "              ,p.def_next_extent * :bs<char[100]> * 1024\n"
				    "             )                        AS next_extent\n"
				    "      , DECODE(\n"
				    "                p.def_min_extents\n"
				    "               ,'DEFAULT',s.min_extents\n"
				    "               ,p.def_min_extents\n"
				    "              )                       AS min_extents\n"
				    "      , DECODE(\n"
				    "                p.def_max_extents\n"
				    "               ,'DEFAULT',DECODE(\n"
				    "                                  s.max_extents\n"
				    "                                 ,2147483645,'unlimited'\n"
				    "                                 ,s.max_extents\n"
				    "                                )\n"
				    "               ,2147483645,'unlimited'\n"
				    "               ,p.def_max_extents\n"
				    "              )                       AS max_extents\n"
				    "      , DECODE(\n"
				    "                p.def_pct_increase\n"
				    "               ,'DEFAULT',s.pct_increase\n"
				    "               ,p.def_pct_increase\n"
				    "              )                       AS pct_increase\n"
				    "      , DECODE(\n"
				    "                p.def_freelists\n"
				    "               ,0,1\n"
				    "               ,NVL(p.def_freelists,1)\n"
				    "              )                       AS freelists\n"
				    "      , DECODE(\n"
				    "                p.def_freelist_groups\n"
				    "               ,0,1\n"
				    "               ,NVL(p.def_freelist_groups,1)\n"
				    "              )                       AS freelist_groups\n"
				    "      , 'N/A'                         AS buffer_pool\n"
				    "      , DECODE(\n"
				    "                p.def_logging \n"
				    "               ,'NO','NOLOGGING'\n"
				    "               ,     'LOGGING'\n"
				    "              )                       AS logging\n"
				    "      , LOWER(p.def_tablespace_name)  AS tablespace_name\n"
				    "      , t.blocks - NVL(t.empty_blocks,0)\n"
				    " FROM\n"
				    "        all_all_tables    t\n"
				    "      , all_part_indexes  p\n"
				    "      , dba_tablespaces   s\n"
				    " WHERE\n"
				    "            t.table_name      = :name<char[100]>\n"
				    "        AND p.table_name      = t.table_name\n"
				    "        AND s.tablespace_name = p.def_tablespace_name\n"
				    "        AND t.owner           = :own<char[100]>\n"
				    "        AND p.owner           = t.owner",
				    QString::null,
				    "8.0");

static toSQL SQLPartitionIndexNames("toExtract:PartitionIndexNames",
				    "SELECT index_name\n"
				    "  FROM all_part_indexes\n"
				    " WHERE table_name = :nam<char[100]>\n"
				    "   AND owner      = :own<char[100]>\n"
				    " ORDER BY index_name",
				    "Index names of partition table, "
				    "must have same binds and columns");

QString toExtract::createPartitionedIOT(const QString &schema,const QString &owner,
					const QString &name)
{
  list<QString> result=toReadQuery(Connection,SQLPartitionedIOTInfo(Connection),
				   QString::number(BlockSize),name,owner);
  QString ret=createTableText(result,schema,owner,name);
  if (Storage) {
    list<QString> overflow=toReadQuery(Connection,SQLOverflowInfo(Connection),name,owner);
    if (overflow.size()==18) {
      ret+="OVERFLOW\n";
      ret+=segmentAttributes(overflow);
    }
  }

  otl_stream inf(1,
		 SQLPartitionIndexNames(Connection),
		 Connection.connection());
  inf<<name.utf8()<<owner.utf8();
  if (!inf.eof())
    throw QString("Couldn't find index partitions for %1.%2").arg(owner).arg(name);
  char buffer[100];
  inf>>buffer;
  QString index(QString::fromUtf8(buffer));
  ret+=
    "PARTITION BY RANGE\n"
    "(\n    ";
  ret+=partitionKeyColumns(owner,name,"TABLE");
  ret+="\n)\n";
  ret+=rangePartitions(owner,index,"NONE","IOT");
  ret+=createComments(schema,owner,name);
  return ret;
}

void toExtract::describePartitionedIOT(list<QString> &lst,list<QString> &ctx,
				       const QString &schema,const QString &owner,
				       const QString &name)
{
  list<QString> result=toReadQuery(Connection,SQLPartitionedIOTInfo(Connection),
				   QString::number(BlockSize),name,owner);
  describeTableText(lst,ctx,result,schema,owner,name);
  otl_stream inf(1,
		 SQLPartitionIndexNames(Connection),
		 Connection.connection());
  inf<<name.utf8()<<owner.utf8();
  if (!inf.eof())
    throw QString("Couldn't find index partitions for %1.%2").arg(owner).arg(name);
  char buffer[100];
  inf>>buffer;
  QString index(QString::fromUtf8(buffer));
  addDescription(lst,ctx,"PARTITION COLUMNS",partitionKeyColumns(owner,name,"TABLE"));
  describePartitions(lst,ctx,owner,index,"NONE","IOT");
  describeComments(lst,ctx,schema,owner,name);
  describeComments(lst,ctx,schema,owner,name);
  if (Storage) {
    list<QString> overflow=toReadQuery(Connection,SQLOverflowInfo(Connection),name,owner);
    if (overflow.size()==18) {
      ctx.insert(ctx.end(),"OVERFLOW");
      describeAttributes(lst,ctx,overflow);
    }
  }
}

static toSQL SQLIOTInfo("toExtract:IOTInfo",
			"SELECT\n"
			"        -- Table Properties\n"
			"        DECODE(\n"
			"                b.monitoring\n"
			"               ,'NO','NOMONITORING'\n"
			"               ,     'MONITORING'\n"
			"              )\n"
			"      , 'N/A'                         AS table_name\n"
			"        -- Parallel Clause\n"
			"      , LTRIM(a.degree)\n"
			"      , LTRIM(a.instances)\n"
			"        -- Physical Properties\n"
			"      , 'INDEX'                       AS organization\n"
			"        -- Segment Attributes\n"
			"      , 'N/A'                         AS cache\n"
			"      , 'N/A'                         AS pct_used\n"
			"      , a.pct_free\n"
			"      , DECODE(\n"
			"                a.ini_trans\n"
			"               ,0,1\n"
			"               ,null,1\n"
			"               ,a.ini_trans\n"
			"              )                       AS ini_trans\n"
			"      , DECODE(\n"
			"                a.max_trans\n"
			"               ,0,255\n"
			"               ,null,255\n"
			"               ,a.max_trans\n"
			"              )                       AS max_trans\n"
			"        -- Storage Clause\n"
			"      , a.initial_extent\n"
			"      , a.next_extent\n"
			"      , a.min_extents\n"
			"      , DECODE(\n"
			"                a.max_extents\n"
			"               ,2147483645,'unlimited'\n"
			"               ,a.max_extents\n"
			"              )                       AS max_extents\n"
			"      , a.pct_increase\n"
			"      , NVL(a.freelists,1)\n"
			"      , NVL(a.freelist_groups,1)\n"
			"      , LOWER(a.buffer_pool)          AS buffer_pool\n"
			"      , DECODE(\n"
			"                b.logging\n"
			"               ,'NO','NOLOGGING'\n"
			"               ,     'LOGGING'\n"
			"              )                       AS logging\n"
			"      , LOWER(a.tablespace_name)      AS tablespace_name\n"
			"      , DECODE(\n"
			"                b.blocks\n"
			"                ,null,GREATEST(a.initial_extent,a.next_extent) \n"
			"                      / (b.blocks * 1024)\n"
			"                ,'0' ,GREATEST(a.initial_extent,a.next_extent)\n"
			"                      / (b.blocks * 1024)\n"
			"                ,b.blocks\n"
			"              )                       AS blocks\n"
			" FROM\n"
			"        all_indexes a,\n"
			"        all_all_tables b,\n"
			"        all_constraints c\n"
			" WHERE  a.table_name  = :nam<char[100]>\n"
			"   AND  b.owner = a.owner\n"
			"   AND  b.table_name = a.table_name\n"
			"   AND  a.owner = :own<char[100]>\n"
			"   AND  c.constraint_name = a.index_name\n"
			"   AND  c.owner = a.owner\n"
			"   AND  c.constraint_type = 'P'",
			"Get storage information about a IOT storage, "
			"same binds and columns");

QString toExtract::createIOT(const QString &schema,const QString &owner,
			     const QString &name)
{
  list<QString> storage=toReadQuery(Connection,SQLIOTInfo(Connection),name,owner);

  QString ret=createTableText(storage,schema,owner,name);
  if (Storage) {
    list<QString> overflow=toReadQuery(Connection,SQLOverflowInfo(Connection),name,owner);
    if (overflow.size()==18) {
      ret+="OVERFLOW\n";
      ret+=segmentAttributes(overflow);
    }
  }
  ret+=";\n\n";
  ret+=createComments(schema,owner,name);
  return ret;
}

void toExtract::describeIOT(list<QString> &lst,list<QString> &ctx,
			    const QString &schema,const QString &owner,const QString &name)
{
  list<QString> storage=toReadQuery(Connection,SQLIOTInfo(Connection),name,owner);

  describeTableText(lst,ctx,storage,schema,owner,name);
  describeComments(lst,ctx,schema,owner,name);
  if (Storage) {
    list<QString> overflow=toReadQuery(Connection,SQLOverflowInfo(Connection),name,owner);
    if (overflow.size()==18) {
      ctx.insert(ctx.end(),"OVERFLOW");
      describeAttributes(lst,ctx,overflow);
    }
  }
}

static toSQL SQLPartitionTableInfo("toExtract:PartitionTableInfo",
				   "SELECT\n"
				   "        -- Table Properties\n"
				   "        DECODE(\n"
				   "                t.monitoring\n"
				   "               ,'NO','NOMONITORING'\n"
				   "               ,     'MONITORING'\n"
				   "              )                       AS monitoring\n"
				   "      , t.table_name\n"
				   "        -- Parallel Clause\n"
				   "      , LTRIM(t.degree)               AS degree\n"
				   "      , LTRIM(t.instances)            AS instances\n"
				   "        -- Physical Properties\n"
				   "      , DECODE(\n"
				   "                t.iot_type\n"
				   "               ,'IOT','INDEX'\n"
				   "               ,      'HEAP'\n"
				   "              )                       AS organization\n"
				   "        -- Segment Attributes\n"
				   "      , DECODE(\n"
				   "                LTRIM(t.cache)\n"
				   "               ,'Y','CACHE'\n"
				   "               ,    'NOCACHE'\n"
				   "              )                       AS cache\n"
				   "      , p.def_pct_used\n"
				   "      , p.def_pct_free                AS pct_free\n"
				   "      , p.def_ini_trans               AS ini_trans\n"
				   "      , p.def_max_trans               AS max_trans\n"
				   "        -- Storage Clause\n"
				   "      ,DECODE(\n"
				   "               p.def_initial_extent\n"
				   "              ,'DEFAULT',s.initial_extent\n"
				   "              ,p.def_initial_extent * :bs<char[100]> * 1024\n"
				   "             )                        AS initial_extent\n"
				   "      ,DECODE(\n"
				   "               p.def_next_extent\n"
				   "              ,'DEFAULT',s.next_extent\n"
				   "              ,p.def_next_extent * :bs<char[100]> * 1024\n"
				   "             )                        AS next_extent\n"
				   "      , DECODE(\n"
				   "                p.def_min_extents\n"
				   "               ,'DEFAULT',s.min_extents\n"
				   "               ,p.def_min_extents\n"
				   "              )                       AS min_extents\n"
				   "      , DECODE(\n"
				   "                p.def_max_extents\n"
				   "               ,'DEFAULT',DECODE(\n"
				   "                                  s.max_extents\n"
				   "                                 ,2147483645,'unlimited'\n"
				   "                                 ,s.max_extents\n"
				   "                                )\n"
				   "               ,2147483645,'unlimited'\n"
				   "               ,p.def_max_extents\n"
				   "              )                       AS max_extents\n"
				   "      , DECODE(\n"
				   "                p.def_pct_increase\n"
				   "               ,'DEFAULT',s.pct_increase\n"
				   "               ,p.def_pct_increase\n"
				   "              )                       AS pct_increase\n"
				   "      , DECODE(\n"
				   "                p.def_freelists\n"
				   "               ,0,1\n"
				   "               ,NVL(p.def_freelists,1)\n"
				   "              )                       AS freelists\n"
				   "      , DECODE(\n"
				   "                p.def_freelist_groups\n"
				   "               ,0,1\n"
				   "               ,NVL(p.def_freelist_groups,1)\n"
				   "              )                       AS freelist_groups\n"
				   "      , LOWER(p.def_buffer_pool)      AS buffer_pool\n"
				   "      , DECODE(\n"
				   "                p.def_logging \n"
				   "               ,'NO','NOLOGGING'\n"
				   "               ,     'LOGGING'\n"
				   "              )                       AS logging\n"
				   "      , LOWER(p.def_tablespace_name)  AS tablespace_name\n"
				   "      , t.blocks - NVL(t.empty_blocks,0)\n"
				   " FROM\n"
				   "        all_all_tables   t\n"
				   "      , all_part_tables  p\n"
				   "      , dba_tablespaces  s\n"
				   " WHERE\n"
				   "            t.table_name      = :nam<char[100]>\n"
				   "        AND p.table_name      = t.table_name\n"
				   "        AND s.tablespace_name = p.def_tablespace_name\n"
				   "        AND t.owner           = :own<char[100]>\n"
				   "        AND p.owner           = t.owner",
				   "Get storage information about a partitioned table, "
				   "same binds and columns",
				   "8.1");

static toSQL SQLPartitionTableInfo8("toExtract:PartitionTableInfo",
				    "SELECT\n"
				    "        -- Table Properties\n"
				    "        'N/A'                         AS monitoring\n"
				    "      , t.table_name\n"
				    "        -- Parallel Clause\n"
				    "      , LTRIM(t.degree)               AS degree\n"
				    "      , LTRIM(t.instances)            AS instances\n"
				    "        -- Physical Properties\n"
				    "      , DECODE(\n"
				    "                t.iot_type\n"
				    "               ,'IOT','INDEX'\n"
				    "               ,      'HEAP'\n"
				    "              )                       AS organization\n"
				    "        -- Segment Attributes\n"
				    "      , DECODE(\n"
				    "                LTRIM(t.cache)\n"
				    "               ,'Y','CACHE'\n"
				    "               ,    'NOCACHE'\n"
				    "              )                       AS cache\n"
				    "      , p.def_pct_used\n"
				    "      , p.def_pct_free                AS pct_free\n"
				    "      , p.def_ini_trans               AS ini_trans\n"
				    "      , p.def_max_trans               AS max_trans\n"
				    "        -- Storage Clause\n"
				    "      ,DECODE(\n"
				    "               p.def_initial_extent\n"
				    "              ,'DEFAULT',s.initial_extent\n"
				    "              ,p.def_initial_extent * :bs<char[100]> * 1024\n"
				    "             )                        AS initial_extent\n"
				    "      ,DECODE(\n"
				    "               p.def_next_extent\n"
				    "              ,'DEFAULT',s.next_extent\n"
				    "              ,p.def_next_extent * :bs<char[100]> * 1024\n"
				    "             )                        AS next_extent\n"
				    "      , DECODE(\n"
				    "                p.def_min_extents\n"
				    "               ,'DEFAULT',s.min_extents\n"
				    "               ,p.def_min_extents\n"
				    "              )                       AS min_extents\n"
				    "      , DECODE(\n"
				    "                p.def_max_extents\n"
				    "               ,'DEFAULT',DECODE(\n"
				    "                                  s.max_extents\n"
				    "                                 ,2147483645,'unlimited'\n"
				    "                                 ,s.max_extents\n"
				    "                                )\n"
				    "               ,2147483645,'unlimited'\n"
				    "               ,p.def_max_extents\n"
				    "              )                       AS max_extents\n"
				    "      , DECODE(\n"
				    "                p.def_pct_increase\n"
				    "               ,'DEFAULT',s.pct_increase\n"
				    "               ,p.def_pct_increase\n"
				    "              )                       AS pct_increase\n"
				    "      , DECODE(\n"
				    "                p.def_freelists\n"
				    "               ,0,1\n"
				    "               ,NVL(p.def_freelists,1)\n"
				    "              )                       AS freelists\n"
				    "      , DECODE(\n"
				    "                p.def_freelist_groups\n"
				    "               ,0,1\n"
				    "               ,NVL(p.def_freelist_groups,1)\n"
				    "              )                       AS freelist_groups\n"
				    "      , 'N/A'                         AS buffer_pool\n"
				    "      , DECODE(\n"
				    "                p.def_logging \n"
				    "               ,'NO','NOLOGGING'\n"
				    "               ,     'LOGGING'\n"
				    "              )                       AS logging\n"
				    "      , LOWER(p.def_tablespace_name)  AS tablespace_name\n"
				    "      , t.blocks - NVL(t.empty_blocks,0)\n"
				    " FROM\n"
				    "        all_all_tables   t\n"
				    "      , all_part_tables  p\n"
				    "      , dba_tablespaces  s\n"
				    " WHERE\n"
				    "            t.table_name      = :nam<char[100]>\n"
				    "        AND p.table_name      = t.table_name\n"
				    "        AND s.tablespace_name = p.def_tablespace_name\n"
				    "        AND t.owner           = :own<char[100]>\n"
				    "        AND p.owner           = t.owner",
				    QString::null,
				    "8.0");

static toSQL SQLPartitionType("toExtract:PartitionType",
			      "SELECT\n"
			      "        partitioning_type\n"
			      "      , partition_count\n"
			      "      , subpartitioning_type\n"
			      "      , def_subpartition_count\n"
			      " FROM\n"
			      "        all_part_tables\n"
			      " WHERE\n"
			      "            table_name = :nam<char[100]>\n"
			      "        AND owner = :own<char[100]>",
			      "Get partition type, must have same binds and columns",
			      "8.1");

static toSQL SQLPartitionType8("toExtract:PartitionType",
			       "SELECT\n"
			       "        partitioning_type\n"
			       "      , partition_count\n"
			       "      , 'N/A'                        AS subpartitioning_type\n"
			       "      , 'N/A'                        AS def_subpartition_count\n"
			       " FROM\n"
			       "        all_part_tables\n"
			       " WHERE\n"
			       "            table_name = :nam<char[100]>\n"
			       "        AND owner = :own<char[100]>",
			       QString::null,
			       "8.0");

static toSQL SQLPartitionSegment("toExtract:PartitionSegment",
				 "SELECT\n"
				 "        partition_name\n"
				 "      , high_value\n"
				 "      , 'N/A'\n"
				 "      , pct_used\n"
				 "      , pct_free\n"
				 "      , ini_trans\n"
				 "      , max_trans\n"
				 "        -- Storage Clause\n"
				 "      , initial_extent\n"
				 "      , next_extent\n"
				 "      , min_extent\n"
				 "      , DECODE(\n"
				 "                max_extent\n"
				 "               ,2147483645,'unlimited'\n"
				 "               ,           max_extent\n"
				 "              )                       AS max_extents\n"
				 "      , pct_increase\n"
				 "      , NVL(freelists,1)\n"
				 "      , NVL(freelist_groups,1)\n"
				 "      , LOWER(buffer_pool)\n"
				 "      , DECODE(\n"
				 "                logging \n"
				 "               ,'NO','NOLOGGING'\n"
				 "               ,     'LOGGING'\n"
				 "              )                       AS logging\n"
				 "      , LOWER(tablespace_name)\n"
				 "      , blocks - NVL(empty_blocks,0)\n"
				 " FROM\n"
				 "        all_tab_partitions\n"
				 " WHERE  table_name = :nam<char[100]>\n"
				 "   AND  owner = :nam<char[100]>\n"
				 " ORDER BY partition_name",
				 "Information about segment storage for partitioned tables, "
				 "must have same binds and columns",
				 "8.1");

static toSQL SQLPartitionSegment8("toExtract:PartitionSegment",
				  "SELECT\n"
				  "        partition_name\n"
				  "      , high_value\n"
				  "      , 'N/A'\n"
				  "      , pct_used\n"
				  "      , pct_free\n"
				  "      , ini_trans\n"
				  "      , max_trans\n"
				  "        -- Storage Clause\n"
				  "      , initial_extent\n"
				  "      , next_extent\n"
				  "      , min_extent\n"
				  "      , DECODE(\n"
				  "                max_extent\n"
				  "               ,2147483645,'unlimited'\n"
				  "               ,           max_extent\n"
				  "              )                       AS max_extents\n"
				  "      , pct_increase\n"
				  "      , NVL(freelists,1)\n"
				  "      , NVL(freelist_groups,1)\n"
				  "      , 'N/A'                         AS buffer_pool\n"
				  "      , DECODE(\n"
				  "                logging \n"
				  "               ,'NO','NOLOGGING'\n"
				  "               ,     'LOGGING'\n"
				  "              )                       AS logging\n"
				  "      , LOWER(tablespace_name)\n"
				  "      , blocks - NVL(empty_blocks,0)\n"
				  " FROM\n"
				  "        all_tab_partitions\n"
				  " WHERE  table_name = :nam<char[100]>\n"
				  "   AND  owner = :nam<char[100]>\n"
				  " ORDER BY partition_name",
				  QString::null,
				  "8.0");

static toSQL SQLSubPartitionName("toExtract:SubPartitionName",
				 "SELECT subpartition_name,\n"
				 "       tablespace_name\n"
				 "  FROM all_tab_subpartitions\n"
				 " WHERE table_name = :nam<char[100]>\n"
				 "   AND partition_name = :prt<char[100]>\n"
				 "   AND table_owner = :own<char[100]>\n"
				 " ORDER BY subpartition_name",
				 "Get information about sub partitions, "
				 "must have same columns and binds");

static toSQL SQLPartitionName("toExtract:PartitionName",
			      "SELECT partition_name,\n"
			      "       tablespace_name\n"
			      "  FROM all_tab_partitions\n"
			      " WHERE table_name = :nam<char[100]>\n"
			      "   AND table_owner = :own<char[100]>\n"
			      " ORDER BY partition_name",
			      "Get information about hash partition names, "
			      "must have same columns and binds");

QString toExtract::createPartitionedTable(const QString &schema,const QString &owner,
					  const QString &name)
{
  list<QString> storage=toReadQuery(Connection,SQLPartitionTableInfo(Connection),
				    QString::number(BlockSize),name,owner);

  QString organization;
  {
    list<QString>::iterator i=storage.begin();
    i++; i++; i++; i++;
    organization=*i;
  }

  QString ret=createTableText(storage,schema,owner,name);
  list<QString> type=toReadQuery(Connection,SQLPartitionType(Connection),name,owner);
  QString partitionType    (toShift(type));
  QString partitionCount   (toShift(type));
  QString subPartitionType (toShift(type));
  QString subPartitionCount(toShift(type));

  ret+=QString("PARTITION BY %1\n(\n    ").arg(partitionType);
  ret+=partitionKeyColumns(owner,name,"TABLE");
  ret+="\n)\n";

  if (partitionType=="RANGE") {
    if (subPartitionType=="HASH") {
      ret+="SUBPARTITIONED BY HASH\n(\n    ";
      ret+=subPartitionKeyColumns(owner,name,"TABLE");
      ret+="\n)\nSUBPARTITIONS ";
      ret+=subPartitionCount;
      ret+="\n";
    }
    ret+="(\n";

    list<QString> segment=toReadQuery(Connection,SQLPartitionSegment(Connection),name,owner);

    QString comma="    ";
    while(segment.size()>0) {
      list<QString> storage;
      QString partition=toShift(segment);
      QString highValue=toShift(segment);
      toPush(storage,"      ");
      toPush(storage,organization);
      for(int i=0;i<16;i++)
	toPush(storage,toShift(segment));

      ret+=comma;
      ret+=QString("PARTITION %1 VALUES LESS THAN\n"
		   "      (\n"
		   "        %2\n"
		   "      )\n").arg(partition.lower()).arg(highValue.lower());
      ret+=segmentAttributes(storage);
      comma="  , ";

      if (subPartitionType=="HASH") {
	list<QString> subs=toReadQuery(Connection,SQLSubPartitionName(Connection),
				       name,partition,owner);
	bool first=true;
	ret+="        (\n            ";
	while(subs.size()>0) {
	  if (first)
	    first=false;
	  else
	    ret+="\n          , ";
	  ret+=QString("SUBPARTITION %2 TABLESPACE %1").
	    arg(toShift(subs).lower()).
	    arg(toShift(subs).lower());
	}
	ret+="\n        )\n";
      }
    }
  } else {
    list<QString> hash=toReadQuery(Connection,SQLSubPartitionName(Connection),
				   name,owner);
    bool first=true;
    ret+="(\n    ";
    while(hash.size()>0) {
      if (first)
	first=false;
      else
	ret+="\n  , ";
      ret+=QString("PARTITION %2 TABLESPACE %1").
	arg(toShift(hash).lower()).
	arg(toShift(hash).lower());
    }
  }

  ret+=";\n\n";
  ret+=createComments(schema,owner,name);
  return ret;
}

void toExtract::describePartitionedTable(list<QString> &lst,list<QString> &ctx,
					 const QString &schema,const QString &owner,
					 const QString &name)
{
  list<QString> storage=toReadQuery(Connection,SQLPartitionTableInfo(Connection),
				    QString::number(BlockSize),name,owner);

  QString organization;
  {
    list<QString>::iterator i=storage.begin();
    i++; i++; i++; i++;
    organization=*i;
  }

  describeTableText(lst,ctx,storage,schema,owner,name);
  list<QString> type=toReadQuery(Connection,SQLPartitionType(Connection),name,owner);
  QString partitionType    (toShift(type));
  QString partitionCount   (toShift(type));
  QString subPartitionType (toShift(type));
  QString subPartitionCount(toShift(type));

  addDescription(lst,ctx,QString("PARTITION BY %1 (%2)").
		 arg(partitionType).
		 arg(partitionKeyColumns(owner,name,"TABLE")));

  if (partitionType=="RANGE") {
    if (subPartitionType=="HASH") {
      addDescription(lst,ctx,QString("SUBPARTITIONED BY HASH (%1) SUBPARTITIONS %2").
		     arg(subPartitionKeyColumns(owner,name,"TABLE")).
		     arg(subPartitionCount));
    }

    list<QString> segment=toReadQuery(Connection,SQLPartitionSegment(Connection),name,owner);

    while(segment.size()>0) {
      list<QString> storage;
      QString partition=toShift(segment);
      QString highValue=toShift(segment);
      toPush(storage,"      ");
      toPush(storage,organization);
      for(int i=0;i<16;i++)
	toPush(storage,toShift(segment));

      list<QString> cctx=ctx;
      cctx.insert(cctx.end(),"PARTITION");
      cctx.insert(cctx.end(),partition.lower());
      addDescription(lst,cctx,"RANGE",QString("VALUES LESS THAN %2").
		     arg(highValue.lower()));
      describeAttributes(lst,cctx,storage);

      if (subPartitionType=="HASH") {
	list<QString> subs=toReadQuery(Connection,SQLSubPartitionName(Connection),
				       name,partition,owner);
	while(subs.size()>0) {
	  QString subpart=toShift(subs).lower();
	  QString tabspac=toShift(subs);
	  addDescription(lst,cctx,"SUBPARTITION",subpart);
	  addDescription(lst,cctx,"SUBPARTITION",subpart,
			 QString("TABLESPACE %1").arg(tabspac));
	}
      }
    }
  } else {
    list<QString> hash=toReadQuery(Connection,SQLSubPartitionName(Connection),
				   name,owner);
    while(hash.size()>0) {
      QString partition=toShift(hash).lower();
      QString tablespac=toShift(hash);
      addDescription(lst,ctx,"PARTITION",partition);
      addDescription(lst,ctx,"PARTITION",partition,
		     QString("TABLESPACE %1").arg(tablespac));
    }
  }

  describeComments(lst,ctx,schema,owner,name);
}

QString toExtract::createPackage(const QString &schema,const QString &owner,const QString &name)
{
  return displaySource(schema,owner,name,"PACKAGE");
}

void toExtract::describePackage(list<QString> &lst,
				const QString &schema,const QString &owner,const QString &name)
{
  describeSource(lst,schema,owner,name,"PACKAGE");
}

QString toExtract::createPackageBody(const QString &schema,const QString &owner,const QString &name)
{
  return displaySource(schema,owner,name,"PACKAGE BODY");
}

void toExtract::describePackageBody(list<QString> &lst,const QString &schema,
				    const QString &owner,const QString &name)
{
  describeSource(lst,schema,owner,name,"PACKAGE BODY");
}

QString toExtract::createProcedure(const QString &schema,const QString &owner,const QString &name)
{
  return displaySource(schema,owner,name,"PROCEDURE");
}

void toExtract::describeProcedure(list<QString> &lst,
				  const QString &schema,const QString &owner,const QString &name)
{
  describeSource(lst,schema,owner,name,"PROCEDURE");
}

QString toExtract::createSnapshot(const QString &schema,const QString &owner,const QString &name)
{
  return createMView(schema,owner,name,"SNAPSHOT");
}

QString toExtract::createSnapshotLog(const QString &schema,const QString &owner,const QString &name)
{
  return createMViewLog(schema,owner,name,"SNAPSHOT");
}

void toExtract::describeSnapshot(list<QString> &lst,const QString &schema,
				 const QString &owner,const QString &name)
{
  describeMView(lst,schema,owner,name,"SNAPSHOT");
}

void toExtract::describeSnapshotLog(list<QString> &lst,const QString &schema,
				    const QString &owner,const QString &name)
{
  describeMViewLog(lst,schema,owner,name,"SNAPSHOT LOG");
}

static toSQL SQLProfileInfo("toExtract:ProfileInfo",
			    "SELECT\n"
			    "        RPAD(resource_name,27)\n"
			    "      , DECODE(\n"
			    "                RESOURCE_NAME\n"
			    "               ,'PASSWORD_VERIFY_FUNCTION',DECODE(\n"
			    "                                                   limit\n"
			    "                                                  ,'UNLIMITED','null'\n"
			    "                                                  ,LOWER(limit)\n"
			    "                                                 )\n"
			    "               ,                           LOWER(limit)\n"
			    "              )\n"
			    " FROM\n"
			    "        dba_profiles\n"
			    " WHERE\n"
			    "        profile = :nam<char[100]>\n"
			    " ORDER\n"
			    "    BY\n"
			    "        DECODE(\n"
			    "               SUBSTR(resource_name,1,8)\n"
			    "              ,'FAILED_L',2\n"
			    "              ,'PASSWORD',2\n"
			    "              ,1\n"
			    "             )\n"
			    "      , resource_name",
			    "Get information about a profile, must have same binds and columns");

QString toExtract::createProfile(const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLProfileInfo(Connection),
				 name);
  if (info.size()==0)
    throw QString("Couldn't find profile %1").arg(name);

  QString ret;
  if (Prompt)
    ret=QString("PROMPT CREATE PROFILE %1\n\n").arg(name.lower());
  ret+=QString("CREATE PROFILE %1\n").arg(name.lower());

  while(info.size()!=0) {
    ret+="   ";
    ret+=toShift(info);
    ret+="\n";
  }
  ret+=";\n\n";
  return ret;
}

void toExtract::describeProfile(list<QString> &lst,
				const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLProfileInfo(Connection),
				 name);
  if (info.size()==0)
    throw QString("Couldn't find profile %1").arg(name);

  list<QString> ctx;
  ctx.insert(ctx.end(),"PROFILE");
  ctx.insert(ctx.end(),name.lower());
  addDescription(lst,ctx);

  while(info.size()!=0)
    addDescription(lst,ctx,toShift(info));
}

static toSQL SQLRoleInfo("toExtract:RoleInfo",
			 "SELECT\n"
			 "        DECODE(\n"
			 "                r.password_required\n"
			 "               ,'YES', DECODE(\n"
			 "                               u.password\n"
			 "                              ,'EXTERNAL','IDENTIFIED EXTERNALLY'\n"
			 "                              ,'IDENTIFIED BY VALUES ''' \n"
			 "                                || u.password || ''''\n"
			 "                             )\n"
			 "               ,'NOT IDENTIFIED'\n"
			 "              )                         AS password\n"
			 " FROM\n"
			 "        dba_roles   r\n"
			 "      , sys.user$  u\n"
			 " WHERE\n"
			 "            r.role = :rol<char[100]>\n"
			 "        AND u.name = r.role",
			 "Get information about a role, must have same binds and columns");

QString toExtract::createRole(const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLRoleInfo(Connection),
				 name);
  if (info.size()==0)
    throw QString("Couldn't find role %1").arg(name);

  QString ret;
  if (Prompt)
    ret=QString("PROMPT CREATE ROLE %1\n\n").arg(name.lower());
  ret+=QString("CREATE ROLE %1 %2;\n\n").arg(name.lower()).arg(toShift(info));
  ret+=grantedPrivs(name.lower(),name,1);
  return ret;
}

void toExtract::describeRole(list<QString> &lst,
			     const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLRoleInfo(Connection),
				 name);
  if (info.size()==0)
    throw QString("Couldn't find role %1").arg(name);

  list<QString> ctx;
  ctx.insert(ctx.end(),"ROLE");
  ctx.insert(ctx.end(),name.lower());
  addDescription(lst,ctx);
  describePrivs(lst,ctx,name);
}

static toSQL SQLRolePrivs("toExtract:RolePrivs",
			  "SELECT\n"
			  "        granted_role\n"
			  "      , DECODE(\n"
			  "                admin_option\n"
			  "               ,'YES','WITH ADMIN OPTION'\n"
			  "               ,null\n"
			  "              )                         AS admin_option\n"
			  "  FROM  dba_role_privs\n"
			  " WHERE  grantee = :nam<char[100]>\n"
			  " ORDER  BY granted_role",
			  "Get roles granted, must have same columns and binds");

static toSQL SQLSystemPrivs("toExtract:SystemPrivs",
			    "SELECT\n"
			    "        privilege\n"
			    "      , DECODE(\n"
			    "                admin_option\n"
			    "               ,'YES','WITH ADMIN OPTION'\n"
			    "               ,null\n"
			    "              )                         AS admin_option\n"
			    "  FROM  dba_sys_privs\n"
			    " WHERE  grantee = :nam<char[100]>\n"
			    " ORDER  BY privilege",
			    "Get system priveleges granted, must have same columns and binds");

static toSQL SQLObjectPrivs("toExtract:ObjectPrivs",
			    "SELECT  privilege\n"
			    "      , owner\n"
			    "      , table_name\n"
			    "      , DECODE(\n"
			    "                grantable\n"
			    "               ,'YES','WITH GRANT OPTION'\n"
			    "               ,null\n"
			    "              )                         AS grantable\n"
			    "  FROM  dba_tab_privs\n"
			    " WHERE  grantee = :nam<char[100]>\n"
			    " ORDER  BY table_name,privilege",
			    "Get object priveleges granted, must have same columns and binds");

QString toExtract::grantedPrivs(const QString &schema,const QString &name,int typ)
{
  if (!Grants)
    return "";

  QString ret;
  if ((typ&1)==1) {
    list<QString> result=toReadQuery(Connection,SQLSystemPrivs(Connection),name);
    while(result.size()>0) {
      QString priv=toShift(result).lower();
      QString sql=QString("GRANT %1 TO %2 %3").
	arg(priv).
	arg(schema).
	arg(toShift(result));
      if (Prompt) {
	ret+="PROMPT ";
	ret+=sql;
	ret+="\n\n";
      }
      ret+=sql;
      ret+=";\n\n";
    }
  }

  if ((typ&2)==2) {
    list<QString> result=toReadQuery(Connection,SQLRolePrivs(Connection),name);
    while(result.size()>0) {
      QString priv=toShift(result).lower();
      QString sql=QString("GRANT %1 TO %2 %3").
	arg(priv).
	arg(name.lower()).
	arg(toShift(result));
      if (Prompt) {
	ret+="PROMPT ";
	ret+=sql;
	ret+="\n\n";
      }
      ret+=sql;
      ret+=";\n\n";
    }
  }

  if ((typ&4)==4) {
    list<QString> result=toReadQuery(Connection,SQLObjectPrivs(Connection),name);
    while(result.size()>0) {
      QString priv=toShift(result);
      QString schema=intSchema(toShift(result));
      QString object=toShift(result).lower();
      QString sql=QString("GRANT %1 ON %2%3 TO %4 %5").
	arg(priv.lower()).
	arg(schema).
	arg(object).
	arg(name.lower()).
	arg(toShift(result));
      if (Prompt) {
	ret+="PROMPT ";
	ret+=sql;
	ret+="\n\n";
      }
      ret+=sql;
      ret+=";\n\n";
    }
  }
  return ret;
}

void toExtract::describePrivs(list<QString> &lst,list<QString> &ctx,const QString &name)
{
  if (!Grants)
    return;

  list<QString> result=toReadQuery(Connection,SQLRolePrivs(Connection),name);
  while(result.size()>0) {
    QString role=toShift(result).lower();
    addDescription(lst,ctx,"GRANT","ROLE",role,toShift(result));
  }

  result=toReadQuery(Connection,SQLSystemPrivs(Connection),name);
  while(result.size()>0) {
    QString priv=toShift(result).lower();
    addDescription(lst,ctx,"GRANT",priv,toShift(result));
  }

  result=toReadQuery(Connection,SQLObjectPrivs(Connection),name);
  while(result.size()>0) {
    QString priv=toShift(result);
    QString schema=intSchema(toShift(result));
    QString res="ON ";
    res+=schema;
    if (!schema.isEmpty())
      res+=".";
    res+=toShift(result).lower();
    addDescription(lst,ctx,"GRANT",priv.lower(),res,toShift(result));
  }
}

static toSQL SQLRollbackSegment("toExtract:RollbackSegment",
				"SELECT  DECODE(\n"
				"                r.owner\n"
				"               ,'PUBLIC',' PUBLIC '\n"
				"               ,         ' '\n"
				"              )                                  AS is_public\n"
				"      , r.tablespace_name\n"
				"      , NVL(r.initial_extent,t.initial_extent)   AS initial_extent\n"
				"      , NVL(r.next_extent,t.next_extent)         AS next_extent\n"
				"      , r.min_extents\n"
				"      , DECODE(\n"
				"                r.max_extents\n"
				"               ,2147483645,'unlimited'\n"
				"               ,           r.max_extents\n"
				"              )                                  AS max_extents\n"
				"  FROM  dba_rollback_segs    r\n"
				"      , all_tablespaces  t\n"
				" WHERE\n"
				"            r.segment_name    = :nam<char[100]>\n"
				"        AND t.tablespace_name = r.tablespace_name",
				"Get information about rollback segment, "
				"must have same binds and columns");

QString toExtract::createRollbackSegment(const QString &schema,const QString &owner,const QString &name)
{
  list<QString> result=toReadQuery(Connection,SQLRollbackSegment(Connection),name);
  QString isPublic      = toShift(result);
  QString tablespaceName= toShift(result);
  QString initialExtent = toShift(result);
  QString nextExtent    = toShift(result);
  QString minExtent     = toShift(result);
  QString maxExtent     = toShift(result);

  QString ret;
  if (Prompt)
    ret+=QString("PROMPT CREATE%1ROLLBACK SEGMENT %2\n\n").arg(isPublic).arg(name.lower());
  ret+=QString("CREATE%1ROLLBACK SEGMENT %2\n\n").arg(isPublic).arg(name.lower());
  if (Storage)
    ret+=QString("STORAGE\n(\n"
		 "  INITIAL      %1\n"
		 "  NEXT         %2\n"
		 "  MINEXTENTS   %3\n"
		 "  MAXEXTENTS   %4\n"
		 ")\n"
		 "TABLESPACE     %5;\n\n").
      arg(initialExtent).arg(nextExtent).arg(minExtent).arg(maxExtent).arg(tablespaceName.lower());
  return ret;
}

void toExtract::describeRollbackSegment(list<QString> &lst,
					const QString &schema,const QString &owner,
					const QString &name)
{
  list<QString> result=toReadQuery(Connection,SQLRollbackSegment(Connection),name);
  QString isPublic      = toShift(result);
  QString tablespaceName= toShift(result);
  QString initialExtent = toShift(result);
  QString nextExtent    = toShift(result);
  QString minExtent     = toShift(result);
  QString maxExtent     = toShift(result);

  list<QString> ctx;
  ctx.insert(ctx.end(),"ROLLBACK SEGMENT");
  ctx.insert(ctx.end(),name.lower());

  addDescription(lst,ctx);
  addDescription(lst,ctx,QString("TABLESPACE %1").arg(tablespaceName.lower()));

  if (Storage) {
    ctx.insert(ctx.end(),"STORAGE");
    addDescription(lst,ctx,QString("INITIAL %1").arg(initialExtent));
    addDescription(lst,ctx,QString("NEXT %1").arg(nextExtent));
    addDescription(lst,ctx,QString("MINEXTENTS %1").arg(minExtent));
    addDescription(lst,ctx,QString("MAXEXTENTS %1").arg(maxExtent));
  }
}

static toSQL SQLSequenceInfo("toExtract:SequenceInfo",
			     "SELECT  'START WITH       '\n"
			     "         || LTRIM(TO_CHAR(last_number,'fm999999999'))\n"
			     "                                         AS start_with\n"
			     "      , 'INCREMENT BY     '\n"
			     "         || LTRIM(TO_CHAR(increment_by,'fm999999999')) AS imcrement_by\n"
			     "      , DECODE(\n"
			     "                min_value\n"
			     "               ,0,'NOMINVALUE'\n"
			     "               ,'MINVALUE         ' || TO_CHAR(min_value)\n"
			     "              )                          AS min_value\n"
			     "      , DECODE(\n"
			     "                TO_CHAR(max_value,'fm999999999999999999999999999')\n"
			     "               ,'999999999999999999999999999','NOMAXVALUE'\n"
			     "               ,'MAXVALUE         ' || TO_CHAR(max_value)\n"
			     "              )                          AS max_value\n"
			     "      , DECODE(\n"
			     "                cache_size\n"
			     "               ,0,'NOCACHE'\n"
			     "               ,'CACHE            ' || TO_CHAR(cache_size)\n"
			     "              )                          AS cache_size\n"
			     "      , DECODE(\n"
			     "                cycle_flag\n"
			     "               ,'Y','CYCLE'\n"
			     "               ,'N', 'NOCYCLE'\n"
			     "              )                          AS cycle_flag\n"
			     "      , DECODE(\n"
			     "                order_flag\n"
			     "               ,'Y','ORDER'\n"
			     "               ,'N', 'NOORDER'\n"
			     "              )                          AS order_flag\n"
			     " FROM\n"
			     "        all_sequences\n"
			     " WHERE\n"
			     "            sequence_name  = :nam<char[100]>\n"
			     "        AND sequence_owner = :own<char[100]>",
			     "Get information about sequences, must have same binds");

QString toExtract::createSequence(const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLSequenceInfo(Connection),
				 name,owner);
  if (info.size()==0)
    throw QString("Couldn't find sequence %1").arg(name);

  QString ret;
  if (Prompt)
    ret=QString("PROMPT CREATE SEQUENCE %1%2\n\n").arg(schema).arg(name.lower());
  ret+=QString("CREATE SEQUENCE %1%2\n").arg(schema).arg(name.lower());

  while(info.size()!=0) {
    ret+="   ";
    ret+=toShift(info);
    ret+="\n";
  }
  ret+=";\n\n";
  return ret; 
}

void toExtract::describeSequence(list<QString> &lst,
				 const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLSequenceInfo(Connection),
				 name,owner);
  if (info.size()==0)
    throw QString("Couldn't find sequence %1").arg(name);

  list<QString> ctx;
  ctx.insert(ctx.end(),schema.lower());
  ctx.insert(ctx.end(),"SEQUENCE");
  ctx.insert(ctx.end(),name.lower());

  addDescription(lst,ctx);

  while(info.size()!=0)
    addDescription(lst,ctx,toShift(info));
}

static toSQL SQLSynonymInfo("toExtract:SynonymInfo",
			    "SELECT  table_owner\n"
			    "      , table_name\n"
			    "      , NVL(db_link,'NULL')\n"
			    " FROM\n"
			    "        all_synonyms\n"
			    " WHERE\n"
			    "            synonym_name = :nam<char[100]>\n"
			    "        AND owner = :own<char[100]>",
			    "Get information about a synonym, must have same binds and columns");

QString toExtract::createSynonym(const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLSynonymInfo(Connection),
				 name,owner);
  if (info.size()==0)
    throw QString("Couldn't find synonym %1.%2").arg(owner).arg(name);
  
  QString tableOwner = toShift(info);
  QString tableName  = toShift(info);
  QString dbLink     = toShift(info);
  if (dbLink=="NULL")
    dbLink="";
  else
    dbLink.prepend("@");
  QString useSchema=(schema=="PUBLIC")?QString(""):schema;
  QString isPublic=(owner=="PUBLIC")?" PUBLIC":"";
  QString tableSchema=intSchema(tableOwner);

  QString sql=QString("CREATE%1 SYNONYM %2%3 FOR %4%5%6").
    arg(isPublic).arg(useSchema).arg(name.lower()).
    arg(tableSchema).arg(tableName.lower()).arg(dbLink.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";
  return ret;
}

void toExtract::describeSynonym(list<QString> &lst,
				const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLSynonymInfo(Connection),
				 name,owner);
  if (info.size()==0)
    throw QString("Couldn't find synonym %1.%2").arg(owner).arg(name);
  
  QString tableOwner = toShift(info);
  QString tableName  = toShift(info);
  QString dbLink     = toShift(info);
  if (dbLink=="NULL")
    dbLink="";
  else
    dbLink.prepend("@");
  QString useSchema=(schema=="PUBLIC")?QString(""):schema;
  QString tableSchema=intSchema(tableOwner);

  list<QString> ctx;
  if (owner=="PUBLIC")
    ctx.insert(ctx.end(),owner);
  addDescription(lst,ctx,"SYNONYM",QString("%1%2%3").
		 arg(tableSchema).arg(tableName.lower()).arg(dbLink.lower()));
}

static toSQL SQLTableConstraints("toExtract:TableConstraints",
				 "SELECT\n"
				 "        constraint_type,\n"
				 "        constraint_name\n"
				 " FROM\n"
				 "        all_constraints cn\n"
				 " WHERE      table_name       = :nam<char[100]>\n"
				 "        AND owner            = :own<char[100]>\n"
				 "        AND constraint_type IN('P','U','C')\n"
				 "        AND generated        != 'GENERATED NAME'\n"
				 " ORDER\n"
				 "    BY\n"
				 "       DECODE(\n"
				 "               constraint_type\n"
				 "              ,'P',1\n"
				 "              ,'U',2\n"
				 "              ,'C',4\n"
				 "             )\n"
				 "     , constraint_name",
				 "Get constraints tied to a table except referential, same binds and columns");

static toSQL SQLTableReferences("toExtract:TableReferences",
				"SELECT\n"
				"        constraint_name\n"
				" FROM\n"
				"        all_constraints cn\n"
				" WHERE      table_name       = :nam<char[100]>\n"
				"        AND owner            = :own<char[100]>\n"
				"        AND constraint_type IN('R')\n"
				"        AND generated        != 'GENERATED NAME'\n"
				" ORDER\n"
				"    BY\n"
				"       DECODE(\n"
				"               constraint_type\n"
				"              ,'R',1\n"
				"             )\n"
				"     , constraint_name",
				"Get foreign constraints from a table, same binds and columns");

static toSQL SQLTableTriggers("toExtract:TableTriggers",
			      "SELECT  trigger_name\n"
			      "  FROM  all_triggers\n"
			      " WHERE      table_name = :nam<char[100]>\n"
			      "        AND owner      = :own<char[100]>\n"
			      " ORDER  BY  trigger_name",
			      "Get triggers for a table, must have same columns and binds");

static toSQL SQLIndexNames("toExtract:IndexNames",
			   "SELECT owner,index_name\n"
			   "  FROM all_indexes a\n"
			   " WHERE table_name = :nam<char[100]>\n"
			   "   AND table_owner = :own<char[100]>\n"
			   "   AND (owner,index_name) NOT IN (SELECT b.owner,\n"
			   "                                         b.constraint_name\n"
			   "                                    FROM all_constraints b\n"
			   "                                   WHERE b.owner = a.table_owner\n"
			   "                                     AND b.table_name = a.table_name)",
			   "Get all indexes not tied to any constriaints, same binds and columns");

QString toExtract::createTableFamily(const QString &schema,const QString &owner,const QString &name)
{
  QString ret=createTable(schema,owner,name);

  list<QString> indexes=toReadQuery(Connection,SQLIndexNames(Connection),name,owner);
  while(indexes.size()>0) {
    QString indOwner(toShift(indexes));
    ret+=createIndex(intSchema(indOwner),indOwner,toShift(indexes));
  }

  otl_stream inf(1,
		 SQLTableType(Connection),
		 Connection.connection());
  inf<<name.utf8()<<owner.utf8();
  if (inf.eof())
    throw QString("Couldn't find table %1.%2").arg(owner).arg(name);

  char buffer[100];
  inf>>buffer;
  inf>>buffer;
  QString iotType(QString::fromUtf8(buffer));

  list<QString> constraints=toReadQuery(Connection,SQLTableConstraints(Connection),name,owner);
  while(constraints.size()>0) {
    if (toShift(constraints)!="P"||iotType!="IOT")
      ret+=createConstraint(schema,owner,toShift(constraints));
    else
      toShift(constraints);
  }

  list<QString> triggers=toReadQuery(Connection,SQLTableTriggers(Connection),name,owner);
  while(triggers.size()>0)
    ret+=createTrigger(schema,owner,toShift(triggers));
  return ret;
}

QString toExtract::createTableContents(const QString &schema,const QString &owner,const QString &name)
{
  QString ret;
  if (Contents) {
    if (Prompt)
      ret+=QString("PROMPT CONTENTS OF %1%2\n\n").arg(schema).arg(name.lower());
    int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();

    otl_stream str;
    str.set_all_column_types(otl_all_num2str);

    str.open(1,QString("SELECT * FROM %1.%2").arg(owner).arg(name).utf8(),Connection.connection());

    int descLen;
    otl_column_desc *desc=str.describe_select(descLen);
    bool first=true;

    QString beg=QString("INSERT INTO %1%2 (").arg(schema).arg(name.lower());
    for (int i=0;i<descLen;i++) {
      if (first)
	first=false;
      else
	beg+=",";
      beg+=QString::fromUtf8((const char *)desc[i].name).lower();
    }
    beg+=") VALUES (";

    QRegExp find("'");
    while(!str.eof()) {
      QString line=beg;
      first=true;
      for (int i=0;i<descLen;i++) {
	if (first)
	  first=false;
	else
	  line+=",";
	switch (desc[i].otl_var_dbtype) {
	case otl_var_timestamp:
	  {
	    otl_datetime tim;
	    str>>tim;
	    if (str.is_null())
	      line+="NULL";
	    else {
	      QString str;
	      str.sprintf("TO_DATE('%04d-%02d-%02d %02d:%02d:%02d','YYYY-MM-DD HH24:MI:SS')",
			  tim.year,tim.month,tim.day,tim.hour,tim.minute,tim.second);
	      line+=str;
	    }
	  }
	  break;
	default:
	  QString val=toReadValue(desc[i],str,MaxColSize);
	  if (val=="{null}")
	    line+="NULL";
	  else {
	    val.replace(find,"''");
	    line+="'";
	    line+=val;
	    line+="'";
	  }
	};
      }
      line+=");\n";
      ret+=line;
    }
    ret+="COMMIT;\n\n";
  }

  return ret;
}

void toExtract::describeTableFamily(list<QString> &lst,
				    const QString &schema,const QString &owner,const QString &name)
{
  describeTable(lst,schema,owner,name);

  list<QString> indexes=toReadQuery(Connection,SQLIndexNames(Connection),name,owner);
  while(indexes.size()>0) {
    QString indOwner(toShift(indexes));
    describeIndex(lst,intSchema(indOwner),indOwner,toShift(indexes));
  }

  otl_stream inf(1,
		 SQLTableType(Connection),
		 Connection.connection());
  inf<<name.utf8()<<owner.utf8();
  if (inf.eof())
    throw QString("Couldn't find table %1.%2").arg(owner).arg(name);

  char buffer[100];
  inf>>buffer;
  inf>>buffer;
  QString iotType(QString::fromUtf8(buffer));

  list<QString> constraints=toReadQuery(Connection,SQLTableConstraints(Connection),name,owner);
  while(constraints.size()>0) {
    if (toShift(constraints)!="P"||iotType!="IOT")
      describeConstraint(lst,schema,owner,toShift(constraints));
    else
      toShift(constraints);
  }

  list<QString> triggers=toReadQuery(Connection,SQLTableTriggers(Connection),name,owner);
  while(triggers.size()>0)
    describeTrigger(lst,schema,owner,toShift(triggers));
}

QString toExtract::createTableReferences(const QString &schema,const QString &owner,const QString &name)
{
  QString ret;
  list<QString> constraints=toReadQuery(Connection,SQLTableReferences(Connection),name,owner);
  while(constraints.size()>0)
    ret+=createConstraint(schema,owner,toShift(constraints));
  return ret;
}

void toExtract::describeTableReferences(list<QString> &lst,
					const QString &schema,const QString &owner,const QString &name)
{
  list<QString> constraints=toReadQuery(Connection,SQLTableReferences(Connection),name,owner);
  while(constraints.size()>0)
    describeConstraint(lst,schema,owner,toShift(constraints));
}

static toSQL SQLTriggerInfo("toExtract:TriggerInfo",
			    "SELECT  trigger_type\n"
			    "      , RTRIM(triggering_event)\n"
			    "      , table_owner\n"
			    "      , table_name\n"
			    "      , base_object_type\n"
			    "      , referencing_names\n"
			    "      , description\n"
			    "      , DECODE(\n"
			    "                when_clause\n"
			    "               ,null,null\n"
			    "               ,'WHEN (' || when_clause || ')' || CHR(10)\n"
			    "              )\n"
			    "      , trigger_body\n"
			    " FROM\n"
			    "        all_triggers\n"
			    " WHERE\n"
			    "            trigger_name = :nam<char[100]>\n"
			    "        AND owner        = :own<char[100]>",
			    "Get information about triggers, must have same binds and columns",
			    "8.1");

static toSQL SQLTriggerInfo8("toExtract:TriggerInfo",
			     "SELECT\n"
			     "        trigger_type\n"
			     "      , RTRIM(triggering_event)\n"
			     "      , table_owner\n"
			     "      , table_name\n"
			     "        -- Only table triggers before 8i\n"
			     "      , 'TABLE'                           AS base_object_type\n"
			     "      , referencing_names\n"
			     "      , description\n"
			     "      , DECODE(\n"
			     "                when_clause\n"
			     "               ,null,null\n"
			     "               ,'WHEN (' || when_clause || ')' || CHR(10)\n"
			     "              )\n"
			     "      , trigger_body\n"
			     " FROM\n"
			     "        all_triggers\n"
			     " WHERE\n"
			     "            trigger_name = :nam<char[100]>\n"
			     "        AND owner        = :own<char[100]>",
			     QString::null,
			     "8.0");

QString toExtract::createTrigger(const QString &schema,const QString &owner,const QString &name)
{
  if (!Code)
    return "";
  list<QString> result=toReadQuery(Connection,SQLTriggerInfo(Connection),name,owner);
  if (result.size()!=9)
    throw QString("Couldn't find trigger %1.%2").arg(owner).arg(name);
  QString triggerType=toShift(result);
  QString event      =toShift(result);
  QString tableOwner =toShift(result);
  QString table      =toShift(result);
  QString baseType   =toShift(result);
  QString refNames   =toShift(result);
  QString description=toShift(result);
  QString when       =toShift(result);
  QString body       =toShift(result);

  QString trgType;
  if (triggerType.find("BEFORE")>=0)
    trgType="BEFORE";
  else if (triggerType.find("AFTER")>=0)
    trgType="AFTER";
  else if (triggerType.find("INSTEAD OF")>=0)
    trgType="INSTEAD OF";

  QString src=trgType;
  src+=" ";
  src+=event;
  int pos=description.find(src);
  QString columns;
  if (pos>=0) {
    pos+=src.length();
    int endPos=description.find(" ON ",pos);
    if (endPos>=0) {
      columns=description.right(description.length()-pos);
      columns.truncate(endPos-pos);
    }
  }
  QString schema2=intSchema(tableOwner);
  QString object;
  if (baseType=="TABLE") {
    object=schema2;
    object+=table;
  } else if (baseType=="SCHEMA") {
    object=schema;
    object+="SCHEMA";
  } else {
    object=baseType;
  }
  QString sql=QString("CREATE OR REPLACE TRIGGER %1%2\n").arg(schema).arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n";
  }
  ret+=sql;
  ret+=QString("%1 %2%3 ON %4\n").arg(trgType).arg(event).arg(columns).arg(object);
  if (baseType.find("TABLE")>=0||baseType.find("VIEW")>=0) {
    ret+=refNames;
    ret+="\n";
  }
  if (triggerType.find("EACH ROW")>=0)
    ret+="FOR EACH ROW\n";
  if (!when.isEmpty())
    ret+=when;
  ret+=body;
  ret+="\n/\n\n";
  return ret;
}

void toExtract::describeTrigger(list<QString> &lst,
				const QString &schema,const QString &owner,const QString &name)
{
  if (!Code)
    return;

  list<QString> result=toReadQuery(Connection,SQLTriggerInfo(Connection),name,owner);
  if (result.size()!=9)
    throw QString("Couldn't find trigger %1.%2").arg(owner).arg(name);
  QString triggerType=toShift(result);
  QString event      =toShift(result);
  QString tableOwner =toShift(result);
  QString table      =toShift(result);
  QString baseType   =toShift(result);
  QString refNames   =toShift(result);
  QString description=toShift(result);
  QString when       =toShift(result);
  QString body       =toShift(result);

  QString trgType;
  if (triggerType.find("BEFORE")>=0)
    trgType="BEFORE";
  else if (triggerType.find("AFTER")>=0)
    trgType="AFTER";
  else if (triggerType.find("INSTEAD OF")>=0)
    trgType="INSTEAD OF";

  QString src=trgType;
  src+=" ";
  src+=event;
  int pos=description.find(src);
  QString columns;
  if (pos>=0) {
    pos+=src.length();
    int endPos=description.find(" ON ",pos);
    if (endPos>=0) {
      columns=description.right(description.length()-pos);
      columns.truncate(endPos-pos);
    }
  }
  QString schema2=intSchema(tableOwner);
  QString object;
  if (baseType=="TABLE") {
    object=schema2;
    if (!schema2.isEmpty())
      object+=".";
    object+=table;
  } else if (baseType=="SCHEMA") {
    object=schema;
    if (!schema.isEmpty())
      object+=".";
    object+="SCHEMA";
  } else {
    object=baseType;
  }

  list<QString> ctx;
  ctx.insert(ctx.end(),schema.lower());
  ctx.insert(ctx.end(),"TRIGGER");
  ctx.insert(ctx.end(),name.lower());
  addDescription(lst,ctx);
  QString tmp=triggerType;
  tmp+=event;
  tmp+=columns;
  addDescription(lst,ctx,tmp);
  tmp="ON ";
  tmp+=object;
  if (!refNames.isEmpty()) {
    tmp+=" ";
    tmp+=refNames;
  }
  addDescription(lst,ctx,object);
  if (!when.isEmpty())
    addDescription(lst,ctx,when);
  addDescription(lst,ctx,"BODY",body);
}

static toSQL SQLTablespaceInfo("toExtract:TablespaceInfo",
			       "SELECT  initial_extent\n"
			       "      , next_extent\n"
			       "      , min_extents\n"
			       "      , DECODE(\n"
			       "                max_extents\n"
			       "               ,2147483645,'unlimited'\n"
			       "               ,null,DECODE(\n"
			       "                              :bs<char[100]>\n"
			       "                            , 1,  57\n"
			       "                            , 2, 121\n"
			       "                            , 4, 249\n"
			       "                            , 8, 505\n"
			       "                            ,16,1017\n"
			       "                            ,32,2041\n"
			       "                            ,'\?\?\?'\n"
			       "                           )\n"
			       "               ,max_extents\n"
			       "              )                       AS max_extents\n"
			       "      , pct_increase\n"
			       "      , min_extlen\n"
			       "      , contents\n"
			       "      , logging\n"
			       "      , extent_management\n"
			       "      , allocation_type\n"
			       " FROM\n"
			       "        dba_tablespaces\n"
			       " WHERE\n"
			       "        tablespace_name = :nam<char[100]>",
			       "Get tablespace information, must have same columns and binds",
			       "8.1");

static toSQL SQLTablespaceInfo8("toExtract:TablespaceInfo",
				"SELECT  initial_extent\n"
				"      , next_extent\n"
				"      , min_extents\n"
				"      , DECODE(\n"
				"                max_extents\n"
				"               ,2147483645,'unlimited'\n"
				"               ,null,DECODE(\n"
				"                             :bs<char[100]>\n"
				"                            , 1,  57\n"
				"                            , 2, 121\n"
				"                            , 4, 249\n"
				"                            , 8, 505\n"
				"                            ,16,1017\n"
				"                            ,32,2041\n"
				"                            ,'\?\?\?'\n"
				"                           )\n"
				"               ,max_extents\n"
				"              )                       AS max_extents\n"
				"      , pct_increase\n"
				"      , min_extlen\n"
				"      , contents\n"
				"      , DECODE(\n"
				"                logging \n"
				"               ,'NO','NOLOGGING'\n"
				"               ,     'LOGGING'\n"
				"              )                       AS logging\n"
				"      , 'N/A'                         AS extent_management\n"
				"      , 'N/A'                         AS allocation_type\n"
				" FROM\n"
				"        dba_tablespaces\n"
				" WHERE\n"
				"        tablespace_name = :nam<char[100]>",
				QString::null,
				"8.0");

static toSQL SQLTablespaceInfo7("toExtract:TablespaceInfo",
				"SELECT  initial_extent\n"
				"      , next_extent\n"
				"      , min_extents\n"
				"      , DECODE(\n"
				"                max_extents\n"
				"               ,2147483645,'unlimited'\n"
				"               ,null,DECODE(\n"
				"                             $block_size\n"
				"                            , 1,  57\n"
				"                            , 2, 121\n"
				"                            , 4, 249\n"
				"                            , 8, 505\n"
				"                            ,16,1017\n"
				"                            ,32,2041\n"
				"                            ,'\?\?\?'\n"
				"                           )\n"
				"               ,max_extents\n"
				"              )                       AS max_extents\n"
				"      , pct_increase\n"
				"      , min_extlen\n"
				"      , contents\n"
				"      , DECODE(\n"
				"                logging \n"
				"               ,'NO','NOLOGGING'\n"
				"               ,     'LOGGING'\n"
				"              )                       AS logging\n"
				"      , 'N/A'                         AS extent_management\n"
				"      , 'N/A'                         AS allocation_type\n"
				" FROM\n"
				"        dba_tablespaces\n"
				" WHERE\n"
				"        tablespace_name = :nam<char[100]>",
				QString::null,
				"7.0");

static toSQL SQLDatafileInfo("toExtract:DatafileInfo",
			     "SELECT\n"
			     "        file_name\n"
			     "      , bytes\n"
			     "      , autoextensible\n"
			     "      , DECODE(\n"
			     "                SIGN(2147483645 - maxbytes)\n"
			     "               ,-1,'unlimited'\n"
			     "               ,maxbytes\n"
			     "              )                               AS maxbytes\n"
			     "      , increment_by * :bs<char[100]> * 1024     AS increment_by\n"
			     " FROM\n"
			     "        (select * from dba_temp_files union select * from dba_data_files)\n"
			     " WHERE\n"
			     "        tablespace_name = :nam<char[100]>\n"
			     " ORDER  BY file_name",
			     "Get information about datafiles in a tablespace, "
			     "same binds and columns",
			     "8.0");

static toSQL SQLDatafileInfo7("toExtract:DatafileInfo",
			      "SELECT\n"
			      "        file_name\n"
			      "      , bytes\n"
			      "      , 'N/A'                                 AS autoextensible\n"
			      "      , 'N/A'                                 AS maxbytes\n"
			      "      , DECODE(:bs<char[100]>,\n"
			      "               NULL,'N/A','N/A')              AS increment_by\n"
			      " FROM\n"
			      "        dba_data_files\n"
			      " WHERE\n"
			      "        tablespace_name = :nam<char[100]>\n"
			      " ORDER  BY file_name",
			      QString::null,
			      "7.0");


QString toExtract::createTablespace(const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLTablespaceInfo(Connection),
				 QString::number(BlockSize),
				 name);

  if (info.size()!=10)
    throw QString("Couldn't find tablespace %1").arg(name);

  QString initial          =toShift(info);
  QString next             =toShift(info);
  QString minExtents       =toShift(info);
  QString maxExtents       =toShift(info);
  QString pctIncrease      =toShift(info);
  QString minExtlen        =toShift(info);
  QString contents         =toShift(info);
  QString logging          =toShift(info);
  QString extentManagement =toShift(info);
  QString allocationType   =toShift(info);

  QString ret;
  QString sql;

  if (extentManagement=="LOCAL"&&contents=="TEMPORARY")
    sql=QString("CREATE TEMPORARY TABLESPACE %1\n").arg(name.lower());
  else
    sql=QString("CREATE TABLESPACE %1\n").arg(name.lower());

  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n";
  }
  ret+=sql;

  list<QString> files=toReadQuery(Connection,
				  SQLDatafileInfo(Connection),
				  QString::number(BlockSize),
				  name);
  if (extentManagement=="LOCAL"&&contents=="TEMPORARY")
    ret+="TEMPFILE\n";
  else
    ret+="DATAFILE\n";
  QString comma="  ";
  while(files.size()>0) {
    QString fileName       =toShift(files);
    QString bytes          =toShift(files);
    QString autoExtensible =toShift(files);
    QString maxBytes       =toShift(files);
    QString incrementBy    =toShift(files);

    ret+=QString("%1 '%2' SIZE %3 REUSE\n").
      arg(comma).
      arg(prepareDB(fileName)).
      arg(bytes);
    if (Connection.version()>="8.0"&&Storage) {
      ret+="       AUTOEXTEND ";
      if (autoExtensible=="YES")
	ret+=QString("ON NEXT %1 MAXSIZE %2\n").
	  arg(incrementBy).
	  arg(maxBytes);
      else
	ret+="OFF\n";
    }
    comma=" ,";
  }

  if (Storage) {
    if (extentManagement=="LOCAL") {
      ret+="EXTENT MANAGEMENT LOCAL ";
      if (allocationType=="SYSTEM")
	ret+="AUTOALLOCATE\n";
      else
	ret+=QString("UNIFORM SIZE %1\n").arg(next);
    } else {
      ret+=QString("DEFAULT STORAGE\n"
		   "(\n"
		   "  INITIAL        %1\n"
		   "  NEXT           %2\n"
		   "  MINEXTENTS     %3\n"
		   "  MAXEXTENTS     %4\n"
		   "  PCTINCREASE    %5\n"
		   ")\n").
	arg(initial).
	arg(next).
	arg(minExtents).
	arg(maxExtents).
	arg(pctIncrease);
      if (minExtlen.toInt()>0)
	ret+=QString("MINIMUM EXTENT   %1\n").arg(minExtlen);
      if (Connection.version()>="8.1")
	ret+="EXTENT MANAGEMENT DICTIONARY\n";
    }
    if (Connection.version()>="8.0"&&(contents!="TEMPORARY"||extentManagement!="LOCAL")) {
      ret+=logging;
      ret+="\n";
    }
  }
  ret+=";\n\n";
  return ret;
}

void toExtract::describeTablespace(list<QString> &lst,
				   const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLTablespaceInfo(Connection),
				 QString::number(BlockSize),
				 name);

  if (info.size()!=10)
    throw QString("Couldn't find tablespace %1").arg(name);

  QString initial          =toShift(info);
  QString next             =toShift(info);
  QString minExtents       =toShift(info);
  QString maxExtents       =toShift(info);
  QString pctIncrease      =toShift(info);
  QString minExtlen        =toShift(info);
  QString contents         =toShift(info);
  QString logging          =toShift(info);
  QString extentManagement =toShift(info);
  QString allocationType   =toShift(info);

  list<QString> ctx;

  if (extentManagement=="LOCAL"&&contents=="TEMPORARY")
    ctx.insert(ctx.end(),"TEMPORARY TABLESPACE");
  else
    ctx.insert(ctx.end(),"TABLESPACE");
  ctx.insert(ctx.end(),name.lower());

  addDescription(lst,ctx);

  list<QString> files=toReadQuery(Connection,
				  SQLDatafileInfo(Connection),
				  QString::number(BlockSize),
				  name);
  while(files.size()>0) {
    QString fileName       =toShift(files);
    QString bytes          =toShift(files);
    QString autoExtensible =toShift(files);
    QString maxBytes       =toShift(files);
    QString incrementBy    =toShift(files);

    list<QString> cctx=ctx;
    if (extentManagement=="LOCAL"&&contents=="TEMPORARY")
      cctx.insert(cctx.end(),"TEMPFILE");
    else
      cctx.insert(cctx.end(),"DATAFILE");
    cctx.insert(cctx.end(),prepareDB(fileName));

    addDescription(lst,cctx);
    addDescription(lst,cctx,QString("SIZE %3").arg(bytes));

    if (Connection.version()>="8.0"&&Storage) {
      QString ret="AUTOEXTEND ";
      if (autoExtensible=="YES")
	ret+=QString("ON NEXT %1 MAXSIZE %2\n").
	  arg(incrementBy).
	  arg(maxBytes);
      else
	ret+="OFF\n";
      addDescription(lst,cctx,ret);
    }
  }

  if (Storage) {
    if (extentManagement=="LOCAL") {
      QString ret="EXTENT MANAGEMENT LOCAL ";
      if (allocationType=="SYSTEM")
	ret+="AUTOALLOCATE";
      else
	ret+=QString("UNIFORM SIZE %1").arg(next);
      addDescription(lst,ctx,ret);
    } else {
      addDescription(lst,ctx,"STORAGE",QString("INITIAL %1").arg(initial));
      addDescription(lst,ctx,"STORAGE",QString("NEXT %1").arg(next));
      addDescription(lst,ctx,"STORAGE",QString("MINEXTENTS %1").arg(minExtents));
      addDescription(lst,ctx,"STORAGE",QString("MAXEXTENTS %1").arg(maxExtents));
      addDescription(lst,ctx,"STORAGE",QString("PCTINCREASE %1").arg(pctIncrease));
      addDescription(lst,ctx,QString("MINIMUM EXTENT %1").arg(minExtlen));
      addDescription(lst,ctx,"EXTENT MANAGEMENT DICTIONARY");
    }
    if (Connection.version()>="8.0"&&(contents!="TEMPORARY"||extentManagement!="LOCAL"))
      addDescription(lst,ctx,logging);
  }
}

QString toExtract::createType(const QString &schema,const QString &owner,const QString &name)
{
  return displaySource(schema,owner,name,"TYPE");
}

void toExtract::describeType(list<QString> &lst,
			     const QString &schema,const QString &owner,const QString &name)
{
  describeSource(lst,schema,owner,name,"TYPE");
}

static toSQL SQLUserInfo("toExtract:UserInfo",
			 "SELECT\n"
			 "        DECODE(\n"
			 "                password\n"
			 "               ,'EXTERNAL','EXTERNALLY'\n"
			 "               ,'BY VALUES ''' || password || ''''\n"
			 "              )                         AS password\n"
			 "      , profile\n"
			 "      , default_tablespace\n"
			 "      , temporary_tablespace\n"
			 " FROM\n"
			 "        dba_users\n"
			 " WHERE\n"
			 "        username = :nam<char[100]>",
			 "Information about authentication for a user, "
			 "same binds and columns");

static toSQL SQLUserQuotas("toExtract:UserQuotas",
			   "SELECT\n"
			   "        DECODE(\n"
			   "                max_bytes\n"
			   "               ,-1,'unlimited'\n"
			   "               ,TO_CHAR(max_bytes,'99999999')\n"
			   "              )                         AS max_bytes\n"
			   "      , tablespace_name\n"
			   " FROM\n"
			   "        dba_ts_quotas\n"
			   " WHERE\n"
			   "        username = :nam<char[100]>\n"
			   " ORDER  BY tablespace_name",
			   "Get information about tablespaces for a user, "
			   "same binds and columns");

QString toExtract::createUser(const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLUserInfo(Connection),
				 name);

  if (info.size()!=4)
    throw QString("Couldn't find user %1").arg(name);

  QString password           =toShift(info);
  QString profile            =toShift(info);
  QString defaultTablespace  =toShift(info);
  QString temporaryTablespace=toShift(info);

  QString ret;
  QString nam;
  if (Schema!="1"&&!Schema.isEmpty())
    nam=Schema.lower();
  else
    nam=name.lower();
  if (Prompt)
    ret=QString("PROMPT CREATE USER %1\n\n").arg(nam);
  ret+=QString("CREATE USER %1 IDENTIFIED %2\n"
	       "   PROFILE              %3\n"
	       "   DEFAULT TABLESPACE   %4\n"
	       "   TEMPORARY TABLESPACE %5\n").
    arg(nam).
    arg(password).
    arg(profile.lower()).
    arg(defaultTablespace.lower()).
    arg(temporaryTablespace.lower());

  if (Storage) {
    list<QString> quota=toReadQuery(Connection,SQLUserQuotas(Connection),name);
    while(quota.size()>0) {
      QString siz=toShift(quota);
      QString tab=toShift(quota);
      ret+=QString("   QUOTA %1 ON %2\n").
	arg(siz).
	arg(tab.lower());
    }
  }
  ret+=";\n\n";
  ret+=grantedPrivs(nam,name,3);
  return ret;
}

void toExtract::describeUser(list<QString> &lst,
			     const QString &schema,const QString &owner,const QString &name)
{
  list<QString> info=toReadQuery(Connection,
				 SQLUserInfo(Connection),
				 name);

  if (info.size()!=4)
    throw QString("Couldn't find user %1").arg(name);

  QString password           =toShift(info);
  QString profile            =toShift(info);
  QString defaultTablespace  =toShift(info);
  QString temporaryTablespace=toShift(info);

  list<QString> ctx;
  ctx.insert(ctx.end(),"USER");
  QString nam;
  if (Schema!="1"&&!Schema.isEmpty())
    nam=Schema.lower();
  else
    nam=name.lower();
  ctx.insert(ctx.end(),nam);

  addDescription(lst,ctx);
  addDescription(lst,ctx,QString("PROFILE %1").arg(profile.lower()));
  addDescription(lst,ctx,QString("DEFAULT TABLESPACE %1").arg(defaultTablespace.lower()));
  addDescription(lst,ctx,QString("TEMPORARY TABLESPACE %1").arg(temporaryTablespace.lower()));

  if (Storage) {
    list<QString> quota=toReadQuery(Connection,SQLUserQuotas(Connection),name);
    while(quota.size()>0) {
      QString siz=toShift(quota);
      QString tab=toShift(quota);
      addDescription(lst,ctx,"QUOTA",QString("%1 ON %2").
		     arg(siz).
		     arg(tab));
    }
  }
  describePrivs(lst,ctx,name);
}

static toSQL SQLViewSource("toExtract:ViewSource",
			   "SELECT  text\n"
			   " FROM\n"
			   "        all_views\n"
			   " WHERE\n"
			   "            view_name = :nam<char[100]>\n"
			   "        AND owner = :own<char[100]>",
			   "Get the source of the view, must have same binds and columns");

QString toExtract::createView(const QString &schema,const QString &owner,const QString &name)
{
  if (!Code)
    return "";
  list<QString> source=toReadQuery(Connection,
				   SQLViewSource(Connection),
				   name,owner);
  if (source.size()==0)
    throw QString("Couldn't find user %1.%2").arg(owner.lower()).arg(name.lower());

  QString text=toShift(source);
  QString ret;
  QString sql=QString("CREATE OR REPLACE VIEW %1%2\n").arg(schema).arg(name.lower());
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n";
  }
  ret+=sql;
  ret+="AS\n";
  ret+=text;
  ret+=";\n\n";
  return ret;
}

void toExtract::describeView(list<QString> &lst,
			     const QString &schema,const QString &owner,const QString &name)
{
  if (!Code)
    return;
  list<QString> source=toReadQuery(Connection,
				   SQLViewSource(Connection),
				   name,owner);
  if (source.size()==0)
    throw QString("Couldn't find user %1.%2").arg(owner.lower()).arg(name.lower());

  list<QString> ctx;
  ctx.insert(ctx.end(),schema);
  ctx.insert(ctx.end(),"VIEW");
  ctx.insert(ctx.end(),name.lower());

  addDescription(lst,ctx);
  QString text=toShift(source);
  addDescription(lst,ctx,"AS",text.simplifyWhiteSpace());
}

QString toExtract::dropConstraint(const QString &schema,const QString &owner,
				  const QString &type,const QString &name)
{
  list<QString> tableName=toReadQuery(Connection,
				      SQLConstraintTable(Connection),
				      owner,name);
  if (tableName.size()==0)
    throw QString("Couldn't find constraint %1.%2").arg(owner.lower()).arg(name.lower());
  QString sql=QString("ALTER TABLE %1%2 DROP CONSTRAINT %3").
    arg(schema).
    arg(toShift(tableName).lower()).
    arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";
  return ret;
}

QString toExtract::dropDatabaseLink(const QString &schema,const QString &owner,
				    const QString &type,const QString &name)
{
  QString sql=QString("DROP%1 DATABASE LINK %2").
    arg((owner=="PUBLIC")?" PUBLIC":"").
    arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";
  return ret;
}

QString toExtract::dropMViewLog(const QString &schema,const QString &owner,
				const QString &type,const QString &name)
{
  QString sql=QString("DROP %1 ON %2%3").arg(type).arg(schema).arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";

  return ret;
}

QString toExtract::dropObject(const QString &schema,const QString &owner,
			      const QString &type,const QString &name)
{
  QString sql=QString("DROP %1 %2").arg(type).arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";
  return ret;
}

QString toExtract::dropSchemaObject(const QString &schema,const QString &owner,
				    const QString &type,const QString &name)
{
  QString sql=QString("DROP %1 %2%3").arg(type).arg(schema).arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";
  return ret;
}

QString toExtract::dropProfile(const QString &schema,const QString &owner,
			       const QString &type,const QString &name)
{
  QString sql=QString("DROP PROFILE %1 CASCADE").arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";
  return ret;
}

QString toExtract::dropSynonym(const QString &schema,const QString &owner,
			       const QString &type,const QString &name)
{
  QString sql=QString("DROP%1 SYNONYM %2%3").
    arg((owner=="PUBLIC")?" PUBLIC":"").
    arg((owner=="PUBLIC")?schema:QString("")).
    arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";
  return ret;
}

QString toExtract::dropTable(const QString &schema,const QString &owner,
			     const QString &type,const QString &name)
{
  QString sql=QString("DROP TABLE %1%2 CASCADE CONSTRAINTS").
    arg(schema).
    arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";
  return ret;
}

QString toExtract::dropTablespace(const QString &schema,const QString &owner,
				  const QString &type,const QString &name)
{
  QString sql=QString("DROP TABLESPACE %1 INCLUDING CONTENTS CASCADE CONSTRAINTS").
    arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";
  return ret;
}

QString toExtract::dropUser(const QString &schema,const QString &owner,
			    const QString &type,const QString &name)
{
  QString sql=QString("DROP USER %1 CASCADE").
    arg(name.lower());
  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  ret+=";\n\n";
  return ret;
}

static toSQL SQLIndexPartitioned("toExtract:IndexPartitioned",
				 "SELECT partitioned\n"
				 "  FROM all_indexes\n"
				 " WHERE index_name = :nam<char[100]>\n"
				 "   AND owner = :own<char[100]>",
				 "Get information about if an index is partitioned or not, "
				 "must use same binds and columns");

static toSQL SQLSegmentInfo("toExtract:SegmentInfo",
			    "SELECT\n"
			    "       s.blocks - NVL(t.empty_blocks,0)\n"
			    "     , s.initial_extent\n"
			    "     , s.next_extent\n"
			    "FROM\n"
			    "       dba_segments s\n"
			    "     , all_tables   t\n"
			    "WHERE\n"
			    "           s.segment_name = :nam<char[100]>\n"
			    "       AND s.segment_type = 'TABLE'\n"
			    "       AND s.owner        = :own<char[100]>",
			    "Get information about a segment, "
			    "must have same binds and columns");

static toSQL SQLObjectPartitions("toExtract:ObjectPartitions",
				 "SELECT\n"
				 "        partition_name\n"
				 "      , SUBSTR(segment_type,7)   -- PARTITION or SUBPARTITION\n"
				 " FROM\n"
				 "        all_segments\n"
				 " WHERE\n"
				 "            segment_name = :nam<char[100]>\n"
				 "        AND owner        = :own<char[100]>",
				 "Get partitions and their type for an object, "
				 "must use same binds and columns");

static toSQL SQLIndexSegmentInfo("toExtract:IndexSegmentInfo",
				 "SELECT  s.blocks\n"
				 "        , s.initial_extent\n"
				 "        , s.next_extent\n"
				 "   FROM\n"
				 "          all_segments s\n"
				 "   WHERE\n"
				 "              s.segment_name = :nam<char[100]>\n"
				 "          AND s.segment_type = 'INDEX'\n"
				 "          AND s.owner        = :own<char[100]>",
				 "Get information about index segment, "
				 "must have same binds and columns");

QString toExtract::resizeIndex(const QString &schema,const QString &owner,const QString &name)
{
  QStringList str=QStringList::split(":",name);
  if (str.count()>2)
    throw QString("When calling resizeIndex name should contain one : maximum");
  QString index=str.first();
  QString partition;
  if (str.count()==2)
    partition=str.last();

  if (!partition.isEmpty()) {
    list<QString> result=toReadQuery(Connection,SQLPartitionSegmentType(Connection),
				     index,partition);

    if (result.size()!=2)
      throw QString("Index partition %1.%2 not found").arg(owner).arg(name);

    return resizeIndexPartition(schema,owner,index,partition,toShift(result));
  }
  list<QString> result=toReadQuery(Connection,SQLIndexPartitioned(Connection),name,owner);
  if (result.size()!=1)
    throw QString("Index %1.%2 not found").arg(owner).arg(name);
  QString partitioned=toShift(result);
  if (partitioned=="NO") {
    list<QString> segment=toReadQuery(Connection,SQLIndexSegmentInfo(Connection),
				      name,"INDEX",owner);
    QString blocks =toShift(segment);
    QString initial=toShift(segment);
    QString next   =toShift(segment);
    if (Resize)
      initialNext(blocks,initial,next);
    QString sql=QString("ALTER INDEX %1%2 REBUILD").arg(schema).arg(name.lower());
    QString ret;
    if (Prompt) {
      ret="PROMPT ";
      ret+=sql;
      ret+="\n\n";
    }
    ret+=sql;
    ret+=QString("\nSTORAGE\n"
		 "(\n"
		 "  INITIAL  %1\n"
		 "  NEXT     %2\n"
		 ");\n\n").arg(initial).arg(next);
    return ret;
  } else {
    list<QString> partitions=toReadQuery(Connection,
					 SQLObjectPartitions(Connection),
					 name,owner);
    QString ret;
    while(partitions.size()>0) {
      QString partition=toShift(partitions);
      QString seqType  =toShift(partitions);
      ret+=resizeIndexPartition(schema,owner,name,partition,seqType);
    }
    return ret;
  }
}

static toSQL SQLIndexPartitionSegment("toExtract:IndexPartitionSegment",
				      "SELECT\n"
				      "       s.blocks\n"
				      "     , s.initial_extent\n"
				      "     , s.next_extent\n"
				      "     , p.partitioning_type\n"
				      "FROM\n"
				      "       dba_segments      s\n"
				      "     , all_part_indexes  p\n"
				      "WHERE\n"
				      "           s.segment_name   = :nam<char[100]>\n"
				      "       AND s.partition_name = :prt<char[100]>\n"
				      "       AND p.index_name     = s.segment_name\n"
				      "       AND s.owner          = :own<char[100]>\n"
				      "       AND p.owner          = s.owner",
				      "Get information about an index partition segment, "
				      "must have have same binds and columns");

QString toExtract::resizeIndexPartition(const QString &schema,const QString &owner,
					const QString &name,const QString &partition,
					const QString &seqType)
{
  list<QString> result=toReadQuery(Connection,SQLIndexPartitionSegment(Connection),
				   name,partition,owner);
  QString blocks          =toShift(result);
  QString initial         =toShift(result);
  QString next            =toShift(result);
  QString partitioningType=toShift(result);
  if (Resize)
    initialNext(blocks,initial,next);
  QString ret;
  QString sql=QString("ALTER INDEX %1%2 REBUILD %3 %4").
    arg(schema).
    arg(name.lower()).
    arg(seqType).
    arg(partition.lower());
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  if (seqType=="PARTITION"&&partitioningType=="RANGE")
    ret+=QString("\nSTORAGE\n"
		 "(\n"
		 "  INITIAL  %1\n"
		 "  NEXT     %2\n"
		 ")\n").
      arg(initial).
      arg(next);
  ret+=";\n\n";
  return ret;
}

static toSQL SQLTablePartitioned("toExtract:TablePartitioned",
				 "SELECT partitioned\n"
				 "  FROM all_tables\n"
				 " WHERE table_name = :nam<char[100]>\n"
				 "   AND owner = :own<char[100]>",
				 "Get information about if a table is partitioned or not, "
				 "must use same binds and columns");

static toSQL SQLTablePartIndexes("toExtract:TablePartIndexes",
				 "SELECT\n"
				 "       owner\n"
				 "     , index_name\n"
				 "FROM\n"
				 "       dba_part_indexes\n"
				 "WHERE\n"
				 "           table_name = :nam<char[100]>\n"
				 "       AND owner      = :own<char[100]>",
				 "Get information about indexes of a partitioned table, "
				 "must have same binds and columns");

static toSQL SQLTablePartIndex("toExtract:TablePartIndex",
			       "SELECT\n"
			       "       owner\n"
			       "     , index_name\n"
			       "FROM\n"
			       "       dba_part_indexes\n"
			       "WHERE\n"
			       "           table_name = :nam<char[100]>\n"
			       "       AND owner      = :own<char[100]>\n"
			       "       AND locality   = 'LOCAL'",
			       "Get information about index of a partition in a table, "
			       "must have same binds and columns");

QString toExtract::resizeTable(const QString &schema,const QString &owner,const QString &name)
{
  QStringList str=QStringList::split(":",name);
  if (str.count()>2)
    throw QString("When calling resizeTable name should contain one : maximum");
  QString table=str.first();
  QString partition;
  if (str.count()==2)
    partition=str.last();

  if (!partition.isEmpty()) {
    list<QString> result=toReadQuery(Connection,SQLPartitionSegmentType(Connection),
				     table,partition);

    if (result.size()!=2)
      throw QString("Table partition %1.%2 not found").arg(owner).arg(name);

    QString ret=resizeTablePartition(schema,owner,table,partition,toShift(result));

    list<QString> indexes=toReadQuery(Connection,SQLTablePartIndex(Connection),
				      table,owner);
    while(indexes.size()>0) {
      QString schema2=intSchema(toShift(indexes));
      QString index=intSchema(toShift(indexes));
      index+=":";
      index+=partition;
      ret+=resizeIndex(schema2,owner,index);
    }
    return ret;
  }
  list<QString> result=toReadQuery(Connection,SQLTablePartitioned(Connection),name,owner);
  if (result.size()!=1)
    throw QString("Table %1.%2 not found").arg(owner).arg(name);
  QString partitioned=toShift(result);
  if (partitioned=="NO") {
    list<QString> segment=toReadQuery(Connection,SQLSegmentInfo(Connection),name,"TABLE",owner);
    QString blocks =toShift(segment);
    QString initial=toShift(segment);
    QString next   =toShift(segment);
    if (Resize)
      initialNext(blocks,initial,next);
    QString sql=QString("ALTER TABLE %1%2 MOVE").arg(schema).arg(name.lower());
    QString ret;
    if (Prompt) {
      ret="PROMPT ";
      ret+=sql;
      ret+="\n\n";
    }
    ret+=sql;
    ret+=QString("\nSTORAGE\n"
		 "(\n"
		 "  INITIAL  %1\n"
		 "  NEXT     %2\n"
		 ");\n\n").arg(initial).arg(next);
    return ret;
  } else {
    list<QString> partitions=toReadQuery(Connection,
					 SQLObjectPartitions(Connection),
					 name,owner);
    QString ret;
    while(partitions.size()>0) {
      QString partition=toShift(partitions);
      QString seqType  =toShift(partitions);
      ret+=resizeTablePartition(schema,owner,name,partition,seqType);
    }

    list<QString> indexes=toReadQuery(Connection,SQLTablePartIndexes(Connection),
				      name,owner);
    while(indexes.size()>0) {
      QString schema2=intSchema(toShift(indexes));
      ret+=resizeIndex(schema2,owner,toShift(indexes));
    }
    return ret;
  }
}
static toSQL SQLTablePartitionSegment("toExtract:TablePartitionSegment",
				      "SELECT\n"
				      "              s.blocks - NVL(t.empty_blocks,0)\n"
				      "            , s.initial_extent\n"
				      "            , s.next_extent\n"
				      "            , p.partitioning_type\n"
				      "       FROM\n"
				      "              dba_segments          s\n"
				      "            , all_tab_%1s  t\n"
				      "            , all_part_tables       p\n"
				      "       WHERE\n"
				      "                  s.segment_name   = :nam<char[100]>\n"
				      "              AND s.partition_name = :prt<char[100]>\n"
				      "              AND t.table_name     = s.segment_name\n"
				      "              AND t.partition_name = s.partition_name\n"
				      "              AND p.table_name     = t.table_name\n"
				      "              AND s.owner          = :own<char[100]>\n"
				      "              AND p.owner          = s.owner\n"
				      "              AND t.table_owner    = s.owner",
				      "Get information about an index partition segment, "
				      "must have have same binds, columns and %");

QString toExtract::resizeTablePartition(const QString &schema,const QString &owner,
					const QString &name,const QString &partition,
					const QString &seqType)
{
  list<QString> result=toReadQuery(Connection,
				   toSQL::string(SQLTablePartitionSegment,Connection).
				   arg(seqType).utf8(),
				   name,partition,owner);
  QString blocks          =toShift(result);
  QString initial         =toShift(result);
  QString next            =toShift(result);
  QString partitioningType=toShift(result);
  if (Resize)
    initialNext(blocks,initial,next);
  QString ret;
  QString sql=QString("ALTER TABLE %1%2 MOVE %3 %4").
    arg(schema).
    arg(name.lower()).
    arg(seqType).
    arg(partition.lower());
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n\n";
  }
  ret+=sql;
  if (seqType=="PARTITION"&&partitioningType=="RANGE")
    ret+=QString("\nSTORAGE\n"
		 "(\n"
		 "  INITIAL  %1\n"
		 "  NEXT     %2\n"
		 ")\n").
      arg(initial).
      arg(next);
  ret+=";\n\n";
  return ret;
}

QString toExtract::compile(list<QString> &objects)
{
  clearFlags();
  QString ret=generateHeading("COMPILE",objects);

  QProgressDialog progress("Creating script","&Cancel",objects.size(),Parent,"progress",true);
  progress.setCaption("Creating script");
  QLabel *label=new QLabel(&progress);
  progress.setLabel(label);

  int num=1;
  for (list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
    progress.setProgress(num);
    label->setText(*i);
    qApp->processEvents();
    if (progress.wasCancelled())
      throw QString("Creating script was cancelled");
    num++;

    QString type=*i;
    QString owner;
    QString name;
    int pos=type.find(":");
    if (pos<0)
      throw QString("Internal error, missing : in object description");
    parseObject(type.right(type.length()-pos-1),owner,name);
    type.truncate(pos);
    QString utype=type.upper();
    QString schema=intSchema(owner);

    try {
      if (utype=="FUNCTION"||
	  utype=="PROCEDURE"||
	  utype=="TRIGGER"||
	  utype=="VIEW") {
	ret+=compile(schema,owner,name,utype);
      } else if (utype=="PACKAGE") {
	ret+=compilePackage(schema,owner,name,utype);
      } else {
	QString str="Invalid type ";
	str+=type;
	str+=" to compile";
	throw str;
      }
    } catch (const otl_exception &exc) {
      rethrow("Compile",*i,exc);
    }
  }
  return ret;
}

QString toExtract::create(list<QString> &objects)
{
  clearFlags();
  QString ret=generateHeading("CREATE",objects);

  QProgressDialog progress("Creating script","&Cancel",objects.size(),Parent,"progress",true);
  progress.setCaption("Creating script");
  QLabel *label=new QLabel(&progress);
  progress.setLabel(label);

  int num=1;
  for (list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
    progress.setProgress(num);
    label->setText(*i);
    qApp->processEvents();
    if (progress.wasCancelled())
      throw QString("Creating script was cancelled");
    num++;

    QString type=*i;
    QString owner;
    QString name;
    int pos=type.find(":");
    if (pos<0)
      throw QString("Internal error, missing : in object description");
    parseObject(type.right(type.length()-pos-1),owner,name);
    type.truncate(pos);
    QString utype=type.upper();
    QString schema=intSchema(owner);

    try {
      if (utype=="CONSTRAINT")
	ret+=createConstraint(schema,owner,name);
      else if (utype=="DATABASE LINK")
	ret+=createDBLink(schema,owner,name);
      else if (utype=="EXCHANGE INDEX")
	ret+=createExchangeIndex(schema,owner,name);
      else if (utype=="EXCHANGE TABLE")
	ret+=createExchangeTable(schema,owner,name);
      else if (utype=="FUNCTION")
	ret+=createFunction(schema,owner,name);
      else if (utype=="INDEX")
	ret+=createIndex(schema,owner,name);
      else if (utype=="MATERIALIZED VIEW")
	ret+=createMaterializedView(schema,owner,name);
      else if (utype=="MATERIALIZED VIEW LOG")
	ret+=createMaterializedViewLog(schema,owner,name);
      else if (utype=="PACKAGE")
	ret+=createPackage(schema,owner,name);
      else if (utype=="PACKAGE BODY")
	ret+=createPackageBody(schema,owner,name);
      else if (utype=="PROCEDURE")
	ret+=createProcedure(schema,owner,name);
      else if (utype=="PROFILE")
	ret+=createProfile(schema,owner,name);
      else if (utype=="ROLE")
	ret+=createRole(schema,owner,name);
      else if (utype=="ROLE GRANTS")
	ret+=grantedPrivs(name.lower(),name,6);
      else if (utype=="ROLLBACK SEGMENT")
	ret+=createRollbackSegment(schema,owner,name);
      else if (utype=="SEQUENCE")
	ret+=createSequence(schema,owner,name);
      else if (utype=="SNAPSHOT")
	ret+=createSnapshot(schema,owner,name);
      else if (utype=="SNAPSHOT LOG")
	ret+=createSnapshotLog(schema,owner,name);
      else if (utype=="SYNONYM")
	ret+=createSynonym(schema,owner,name);
      else if (utype=="TABLE")
	ret+=createTable(schema,owner,name);
      else if (utype=="TABLE FAMILY")
	ret+=createTableFamily(schema,owner,name);
      else if (utype=="TABLE CONTENTS")
	ret+=createTableContents(schema,owner,name);
      else if (utype=="TABLE REFERENCES")
	ret+=createTableReferences(schema,owner,name);
      else if (utype=="TABLESPACE")
	ret+=createTablespace(schema,owner,name);
      else if (utype=="TRIGGER")
	ret+=createTrigger(schema,owner,name);
      else if (utype=="TYPE")
	ret+=createType(schema,owner,name);
      else if (utype=="USER")
	ret+=createUser(schema,owner,name);
      else if (utype=="USER GRANTS") {
	QString nam;
	if (Schema!="1"&&!Schema.isEmpty())
	  nam=Schema.lower();
	else
	  nam=name.lower();
	ret+=grantedPrivs(nam,name,4);
      } else if (utype=="VIEW")
	ret+=createView(schema,owner,name);
      else {
	QString str="Invalid type ";
	str+=type;
	str+=" to create";
	throw str;
      }
    } catch (const otl_exception &exc) {
      rethrow("Create",*i,exc);
    }
  }
  return ret;
}

void toExtract::rethrow(const QString &what,const QString &object,const otl_exception &exc)
{
  throw QString("Encountered error in toExtract\n\n"
		"Operation:      %1\n"
		"Object:         %2\n"
		"TOra Version:   %3\n"
		"Oracle Version: %4\n"
		"Error:          %5\n"
		"SQL:\n%6\n").
    arg(what).
    arg(object).
    arg(TOVERSION).
    arg(Connection.version()).
    arg((const char *)exc.msg).
    arg((const char *)exc.stm_text);
}

list<QString> toExtract::describe(list<QString> &objects)
{
  clearFlags();
  Describe=true;

  list<QString> ret;

  QProgressDialog progress("Creating description","&Cancel",objects.size(),Parent,"progress",true);
  progress.setCaption("Creating description");
  QLabel *label=new QLabel(&progress);
  progress.setLabel(label);

  int num=1;
  for (list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
    progress.setProgress(num);
    label->setText(*i);
    qApp->processEvents();
    if (progress.wasCancelled())
      throw QString("Creating script was cancelled");
    num++;

    QString type=*i;
    QString owner;
    QString name;
    int pos=type.find(":");
    if (pos<0)
      throw QString("Internal error, missing : in object description");
    parseObject(type.right(type.length()-pos-1),owner,name);
    type.truncate(pos);
    QString utype=type.upper();
    QString schema=intSchema(owner);

    list<QString> cur;

    try {
      if (utype=="CONSTRAINT")
	describeConstraint(cur,schema,owner,name);
      else if (utype=="DATABASE LINK")
	describeDBLink(cur,schema,owner,name);
      else if (utype=="EXCHANGE INDEX")
	describeExchangeIndex(cur,schema,owner,name);
      else if (utype=="EXCHANGE TABLE")
	describeExchangeTable(cur,schema,owner,name);
      else if (utype=="FUNCTION")
	describeFunction(cur,schema,owner,name);
      else if (utype=="INDEX")
	describeIndex(cur,schema,owner,name);
      else if (utype=="MATERIALIZED VIEW")
	describeMaterializedView(cur,schema,owner,name);
      else if (utype=="MATERIALIZED VIEW LOG")
	describeMaterializedViewLog(cur,schema,owner,name);
      else if (utype=="PACKAGE")
	describePackage(cur,schema,owner,name);
      else if (utype=="PACKAGE BODY")
	describePackageBody(cur,schema,owner,name);
      else if (utype=="PROCEDURE")
	describeProcedure(cur,schema,owner,name);
      else if (utype=="PROFILE")
	describeProfile(cur,schema,owner,name);
      else if (utype=="ROLE")
	describeRole(cur,schema,owner,name);
      else if (utype=="ROLE GRANTS") {
	// A nop, everything is done in describe role
      } else if (utype=="ROLLBACK SEGMENT")
	describeRollbackSegment(cur,schema,owner,name);
      else if (utype=="SEQUENCE")
	describeSequence(cur,schema,owner,name);
      else if (utype=="SNAPSHOT")
	describeSnapshot(cur,schema,owner,name);
      else if (utype=="SNAPSHOT LOG")
	describeSnapshotLog(cur,schema,owner,name);
      else if (utype=="SYNONYM")
	describeSynonym(cur,schema,owner,name);
      else if (utype=="TABLE")
	describeTable(cur,schema,owner,name);
      else if (utype=="TABLE FAMILY")
	describeTableFamily(cur,schema,owner,name);
      else if (utype=="TABLE REFERENCES")
	describeTableReferences(cur,schema,owner,name);
      else if (utype=="TABLE CONTENTS") {
	// A nop, nothing is described of contents
      } else if (utype=="TABLESPACE")
	describeTablespace(cur,schema,owner,name);
      else if (utype=="TRIGGER")
	describeTrigger(cur,schema,owner,name);
      else if (utype=="TYPE")
	describeType(cur,schema,owner,name);
      else if (utype=="USER")
	describeUser(cur,schema,owner,name);
      else if (utype=="USER GRANTS") {
	// A nop, everything is done in describe user
      } else if (utype=="VIEW")
	describeView(cur,schema,owner,name);
      else {
	QString str="Invalid type ";
	str+=type;
	str+=" to describe";
	throw str;
      }
    } catch (const otl_exception &exc) {
      rethrow("Describe",*i,exc);
    }
    cur.sort();
    ret.merge(cur);
  }
  return ret;
}

QString toExtract::drop(list<QString> &objects)
{
  clearFlags();
  QString ret=generateHeading("CREATE",objects);

  QProgressDialog progress("Creating script","&Cancel",objects.size(),Parent,"progress",true);
  progress.setCaption("Creating script");
  QLabel *label=new QLabel(&progress);
  progress.setLabel(label);

  int num=1;
  for (list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
    progress.setProgress(num);
    label->setText(*i);
    qApp->processEvents();
    if (progress.wasCancelled())
      throw QString("Creating script was cancelled");
    num++;

    QString type=*i;
    QString owner;
    QString name;
    int pos=type.find(":");
    if (pos<0)
      throw QString("Internal error, missing : in object description");
    parseObject(type.right(type.length()-pos-1),owner,name);
    type.truncate(pos);
    QString utype=type.upper();
    QString schema=intSchema(owner);

    try {
      if (utype=="CONSTRAINT")
	ret+=dropConstraint(schema,owner,utype,name);
      else if (utype=="DATABASE LINK")
	ret+=dropDatabaseLink(schema,owner,utype,name);
      else if (utype=="DIMENSION")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else if (utype=="DIRECTORY")
	ret+=dropObject(schema,owner,utype,name);
      else if (utype=="FUNCTION")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else if (utype=="INDEX")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else if (utype=="MATERIALIZED VIEW")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else if (utype=="MATERIALIZED VIEW LOG")
	ret+=dropMViewLog(schema,owner,utype,name);
      else if (utype=="PACKAGE")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else if (utype=="PROCEDURE")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else if (utype=="PROFILE")
	ret+=dropProfile(schema,owner,utype,name);
      else if (utype=="ROLE")
	ret+=dropObject(schema,owner,utype,name);
      else if (utype=="ROLLBACK SEGMENT")
	ret+=dropObject(schema,owner,utype,name);
      else if (utype=="SEQUENCE")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else if (utype=="SNAPSHOT")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else if (utype=="SNAPSHOT LOG")
	ret+=dropMViewLog(schema,owner,utype,name);
      else if (utype=="SYNONYM")
	ret+=dropSynonym(schema,owner,utype,name);
      else if (utype=="TABLE")
	ret+=dropTable(schema,owner,utype,name);
      else if (utype=="TABLESPACE")
	ret+=dropTablespace(schema,owner,utype,name);
      else if (utype=="TRIGGER")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else if (utype=="TYPE")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else if (utype=="USER")
	ret+=dropUser(schema,owner,utype,name);
      else if (utype=="VIEW")
	ret+=dropSchemaObject(schema,owner,utype,name);
      else {
	QString str="Invalid type ";
	str+=type;
	str+=" to drop";
	throw str;
      }
    } catch (const otl_exception &exc) {
      rethrow("Drop",*i,exc);
    }
  }
  return ret;
}

QString toExtract::resize(list<QString> &objects)
{
  throw QString("Migration not implemented yet");
  clearFlags();
  QString ret=generateHeading("RESIZE",objects);

  QProgressDialog progress("Resize script","&Cancel",objects.size(),Parent,"progress",true);
  progress.setCaption("Resize script");
  QLabel *label=new QLabel(&progress);
  progress.setLabel(label);

  int num=1;
  for (list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
    progress.setProgress(num);
    label->setText(*i);
    qApp->processEvents();
    if (progress.wasCancelled())
      throw QString("Creating script was cancelled");
    num++;

    QString type=*i;
    QString owner;
    QString name;
    int pos=type.find(":");
    if (pos<0)
      throw QString("Internal error, missing : in object description");
    parseObject(type.right(type.length()-pos-1),owner,name);
    type.truncate(pos);
    QString utype=type.upper();
    QString schema=intSchema(owner);

    try {
      if (utype=="INDEX")
	ret+=resizeIndex(schema,owner,name);
      else if (utype=="TABLE")
	ret+=resizeTable(schema,owner,name);
      else {
	QString str="Invalid type ";
	str+=type;
	str+=" to resize";
	throw str;
      }
    } catch (const otl_exception &exc) {
      rethrow("Resize",*i,exc);
    }
  }
  return ret;
}

QString toExtract::migrate(list<QString> &drpLst,list<QString> &crtLst)
{
  throw QString("Migration not implemented yet");
}
