#include <unistd.h>

#include <qregexp.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include "toextract.h"
#include "tosql.h"
#include "tomain.h"

static toSQL SQLSetSizing("toExtract:SetSizing",
			  "SELECT block_size
  FROM (SELECT bytes/blocks/1024   AS block_size
          FROM user_segments
         WHERE bytes  IS NOT NULL
           AND blocks IS NOT NULL
         UNION
        SELECT bytes/blocks/1024   AS block_size
          FROM user_free_space
         WHERE bytes  IS NOT NULL
           AND blocks IS NOT NULL)
 WHERE rownum < 2",
			  "Get information about block sizes, same columns");

toExtract::toExtract(toConnection &conn)
  : Connection(conn)
{
  SQL=true;
  Heading=true;
  Resize=true;
  Heading=true;
  Prompt=true;

  IsASnapIndex=false;
  IsASnapTable=false;

  Schema="1";

  otl_stream inf(1,SQLSetSizing(Connection),Connection.connection());
  inf>>BlockSize;
  setSizes();
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
			     "SELECT
       'X'
  FROM all_objects
 WHERE object_name = :nam<char[100]>
   AND owner = :own<char[100]>
   AND object_type = :typ<char[100]>",
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
			       "SELECT LOWER(column_name)
  FROM all_cons_columns
 WHERE owner = :own<char[100]>
   AND constrant_name = :con<char[100]>
 ORDER BY position",
			       "List columns in a constraint, must have same binds and columns");

QString toExtract::constraintColumns(const QString &owner,const QString &name)
{
  otl_stream str(1,
		 SQLConstraintCols(Connection),
		 Connection.connection());
  str<<owner.utf8();
  str<<name.utf8();

  QString ret="(\n    ";
  bool any=false;
  while(!str.eof()) {
    char buffer[100];
    str>>buffer;
    if (any)
      ret+=",\n    ";
    ret+=QString::fromUtf8(buffer);
  }
  ret+="\n)\n";
  return ret;
}

static toSQL SQLTableComments("toExtract:TableComment",
			      "SELECT comments,
  FROM all_tab_comments
 WHERE table_name = :nam<char[100]>
   AND comment IS NOT NULL
   AND owner = :own<char[100]>",
			      "Extract comments about a table, must have same columns and binds");
static toSQL SQLColumnComments("toExtract:ColumnComment",
			       "SELECT column_name,
       comments
  FROM all_col_comments
 WHERE table_name = :nam<char[100]>
   AND comment IS NOT NULL
   AND owner = :own<char[100]>",
			       "Extract comments about a columns, must have same columns and binds");

QString toExtract::createComments(const QString &schema,
				  const QString &owner,
				  const QString &name)
{
  QString ret;
  QString sql;
  otl_stream str(1,
		 SQLTableComments(Connection),
		 Connection.connection());
  while(!str.eof()) {
    char buffer[10000];
    str>>buffer;
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
  while(!col.eof()) {
    char buffer[10000];
    str>>buffer;
    QString col=QString::fromUtf8(buffer);
    str>>buffer;
    sql=QString("COMMENT ON COLUMN %1%2.%3 IS '%4'").
      arg(schema.lower()).
      arg(name.lower()).
      arg(col.lower()).
      arg(prepareDB(QString::fromUtf8(buffer)));
    if (Prompt) {
      ret+="PROMPT ";
      ret+=sql;
      ret+="\n\n";
    }
    ret+=sql;
    ret+=";\n\n";
  }
  return ret;
}

QString toExtract::prepareDB(const QString &db)
{
  static QRegExp quote("'");
  QString ret=db;
  ret.replace(quote,"''");
  return ret;
}

QString toExtract::setSchema(const QString &owner)
{
  if (owner.upper()=="PUBLIC")
    return "PUBLIC";
  if (Schema=="1") {
    QString ret=owner.lower();
    ret+=".";
    return ret;
  } else if (Schema.isEmpty())
    return "";
  QString ret=Schema.lower();
  ret+=".";
  return ret;
}

QString toExtract::generateHeading(const QString &action,
				   const QString &type,
				   list<QString> &lst)
{
  if (!Heading)
    return "";
  QString str="REM This DDL was reverse engineered by
REM TOra, Version %1
REM
REM at:   %2
REM from: %3, an Oracle Release %4 instance
REM
REM on:   %4
REM
";
  char host[1024];
  gethostname(host,1024);
  str.
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
    str+=" ";
    str+=type;
    str+=" statement";
  }
  if (lst.size()!=1)
    str+="s";
  str+=" for:\nREM\n";
  for(list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
    str+="REM ";
    str+=(*i);
    str+="\n";
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
    } else if (owner[search]!='.')
      throw 2;
    search++;
    if (object[search]=='\"') {
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
			       "SELECT table_name,
       constraint_type,
       search_condition,
       r_owner,
       r_constraint_name,
       delete_rule,
       DECODE(status,'ENABLED','ENABLE NOVALIDATE','DISABLE'),
       deferrable,
       deferred
  FROM all_constraints
 WHERE owner = :own<char[100]>
   AND constraint_name = :nam<char[100]>",
			       "Get information about a constraint, same binds and columns",
			       "8.0");

static toSQL SQLListConstraint7("toExtract:ListConstraint",
			       "SELECT table_name,
       constraint_type,
       search_condition,
       r_owner,
       r_constraint_name,
       delete_rule,
       DECODE(status,'ENABLED','ENABLE','DISABLE'),
       NULL,
       NULL
  FROM all_constraints
 WHERE owner = :own<char[100]>
   AND constraint_name = :nam<char[100]>",
			       QString::null,
			       "7.0");

static toSQL SQLConstraintTable("toExtract:ConstraintTable",
			     "SELECT table_name
  FROM all_constraint
 WHERE owner = :own<char[100]>
   AND constraint_name = :nam<char[100]>",
			     "Get tablename from constraint name, same binds and columns");

QString toExtract::createConstraint(const QString &schema,const QString &owner,const QString &name)
{
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

  QString ret;

  QString sql("ALTER TABLE %1%2 ADD CONSTRAINT %3 %4\n");
  sql.arg(schema).arg(table.lower()).arg(name.lower()).arg(type);
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n";
  }
  ret+=sql;
  if (tchr=="C") {
    ret+="    CHECK (";
    ret+=search;
    ret+="\n";
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
      ret+=QString::fromUtf8(buffer);
      ret+=constraintColumns(rOwner,rName);

      if (delRule=="CASCADE")
	ret+="ON DELETE CASCADE\n";
      ret+=status;
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
  return ret;
}

static toSQL SQLDBLink("toExtract:ExtractDBLink",
		       "SELECT l.userid,
       l.password,
       l.host
  FROM sys.link$ l,
       sys.user$ u
 WHERE u.name    = :own<char[100]>
   AND l.owner# = u.user#
   AND l.name LIKE :nam<char[100]>||'%'",
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

static toSQL SQLPartitionSegmentType("toExtract:PartitionSegment type",
				     "SELECT SUBSTR(segment_type,7),
       TO_CHAR(blocks)
  FROM dba_segments
 WHERE segment_name = :nam<char[100]>
   AND partition_name = :prt<char[100]>
   AND owner = :own<char[100]>",
				 "Get segment type for object partition, same binds and columns");

static toSQL SQLExchangeIndex("toExtract:ExchangeIndex",
			      "SELECT
        LTRIM(i.degree)
      , LTRIM(i.instances)
      , i.table_name
      , DECODE(
                i.uniqueness
               ,'UNIQUE',' UNIQUE'
               ,null
              )                       AS uniqueness
      , DECODE(
                i.index_type
               ,'BITMAP',' BITMAP'
               ,null
              )                       AS index_type
        -- Physical Properties
      , 'INDEX'                       AS organization
        -- Segment Attributes
      , 'N/A'                         AS cache
      , 'N/A'                         AS pct_used
      , p.pct_free
      , DECODE(
                p.ini_trans
               ,0,1
               ,null,1
               ,p.ini_trans
              )                       AS ini_trans
      , DECODE(
                p.max_trans
               ,0,255
               ,null,255
               ,p.max_trans
              )                       AS max_trans
        -- Storage Clause
      , p.initial_extent
      , p.next_extent
      , p.min_extent
      , DECODE(
                p.max_extent
               ,2147483645,'unlimited'
               ,           p.max_extent
              )                       AS max_extent
      , p.pct_increase
      , NVL(p.freelists,1)
      , NVL(p.freelist_groups,1)
      , LOWER(p.buffer_pool)          AS buffer_pool
      , DECODE(
                p.logging
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(p.tablespace_name)      AS tablespace_name
      , %2                            AS blocks
 FROM
        dba_indexes  i
      , dba_ind_%1s  p
 WHERE
            p.index_name   = :nam<char[100]>
        AND p.%1_name      = :typ<char[100]>
        AND i.index_name   = p.index_name
        AND p.index_owner  = :own<char[100]>
        AND i.owner        = p.index_owner",
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

  QString sql=toSQL::string(SQLExchangeIndex,Connection);
  sql.arg(type).arg(blocks);
  list<QString> result=toReadQuery(Connection,sql.utf8(),segment,partition,owner);
  QString degree=toShift(result);
  QString instances=toShift(result);
  QString table=toShift(result);
  QString unique=toShift(result);
  QString bitmap=toShift(result);

  toUnShift(result,"");

  sql=QString("CREATE%1%2 INDEX %3%4 ON %3%5\n");
  sql.arg(unique).arg(bitmap).arg(schema).arg(segment).arg(table);

  QString ret;
  if (Prompt) {
    ret="PROMPT ";
    ret+=sql;
    ret+="\n";
  }
  ret+=sql;
  ret+=indexColumns("",owner,segment);
  ret+="PARALLEL\n(\n  DEGREE            ";
  ret+=degree;
  ret+="\n  INSTANCES         ";
  ret+=instances;
  ret+=")\n";
  ret+=segmentAttributes(result);
  ret+=";\n\n";
  return ret;
}

static toSQL SQLIndexColumns("toExtract:IndexColumns",
			     "SELECT column_name,
       descend
  FROM all_ind_columns
 WHERE index_name = :nam<char[100]>
   AND index_owner = :own<char[100]>
 ORDER BY column_position",
			     "Get column names from index, same binds and columns",
			     "8.1");
static toSQL SQLIndexColumns7("toExtract:IndexColumns",
			      "SELECT column_name,
       'ASC'
  FROM all_ind_columns
 WHERE index_name = :nam<char[100]>
   AND index_owner = :own<char[100]>
 ORDER BY column_position",
			      QString::null,
			      "7");
static toSQL SQLIndexFunction("toExtract:IndexFunction",
			      "SELECT c.default$
  FROM sys.col$ c,
       all_indexes i,
       all_objects o
 WHERE i.index_name = :ind<char[100]>
   AND o.object_name = i.table_name
   AND c.obj# = o.object_id
   AND c.name = :tab<char[100]>
   AND i.owner = :own<char[100]>
   AND o.owner = t.table_owner",
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
    if (func.match(col)) {
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
      ret+="  , ";
      first=false;
    } else
      ret+="    ";
    ret+=row;
  }
  ret+=indent;
  ret+=")\n";
  return ret;
}

static toSQL SQLExchangeTable("toExtract:ExchangeTable",
			      "SELECT
        DECODE(
                t.monitoring
               ,'NO','NOMONITORING'
               ,     'MONITORING'
              )                              AS monitoring
      , t.table_name
      , LTRIM(t.degree)                      AS degree
      , LTRIM(t.instances)                   AS instances
      , 'HEAP'                               AS organization
      , DECODE(
                t.cache
               ,'y','CACHE'
               ,    'NOCACHE'
              )                              AS cache
      , p.pct_used
      , p.pct_free
      , p.ini_trans
      , p.max_trans
      , p.initial_extent
      , p.next_extent
      , p.min_extent
      , DECODE(
                p.max_extent
               ,2147483645,'unlimited'
               ,p.max_extent
              )                              AS max_extents
      , p.pct_increase
      , p.freelists
      , p.freelist_groups
      , LOWER(p.buffer_pool)                 AS buffer_pool
      , DECODE(
                p.logging
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                              AS logging
      , LOWER(p.tablespace_name)             AS tablespace_name
      , %2 - NVL(p.empty_blocks,0)           AS blocks
 FROM
        dba_tables        t
      , dba_tab_%1s       p
 WHERE
            p.table_name   = :nam<char[100]>
        AND p.%1_name      = :sgm<char[100]>
        AND t.table_name   = p.table_name
        AND p.table_owner  = :own<char[100]>
        AND t.owner        = p.table_owner",
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

  QString sql=toSQL::string(SQLExchangeTable,Connection);
  sql.arg(type).arg(blocks);
  list<QString> result=toReadQuery(Connection,sql.utf8(),segment,partition,owner);
  QString ret=createTableText(result,schema,owner,segment);
  ret+=";\n\n";
  return ret;
}

void toExtract::initialNext(const QString &blocks,QString &initial,QString &next)
{
  list<QString>::iterator iinit=Initial.begin();
  list<QString>::iterator inext=Next.begin();
  list<QString>::iterator ilimit=Limit.begin();
  while(ilimit!=Initial.end()) {
    if (*ilimit=="UNLIMITED"||
	(*ilimit).toFloat()>blocks) {
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
  if (result.size()!=18)
    throw QString("Internal error, result should be 17 in segment attributes");

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
    if (cache!="N/A") {
      ret+=indent;
      ret+=cache;
      ret+="\n";
    }
    if (!IsASnapIndex)
      ret+=QString("%1PCTFREE             %2\n").arg(indent).arg(pctUsed);
  }
  if (!IsASnapIndex) {
    ret+=QString("%1PCTFREE             %2\n").arg(indent).arg(pctFree);
    ret+=QString("%1INITRANS            %2\n").arg(indent).arg(iniTrans);
  }
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

  return ret;
}

static toSQL SQLPrimaryKey("toExtract:PrimaryKey",
			   "SELECT constraint_name
  FROM all_constraints
 WHERE table_name = :nam<char[100]>
   AND constraint_type = 'P'
   AND owner = :own<char[100]>",
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
  if (organization=="INDEX") {
    list<QString> res=toReadQuery(Connection,SQLPrimaryKey(Connection),name,owner);
    if (res.size()!=1)
      throw QString("Couldn't find primary key of %1.%2").arg(owner).arg(name);
    QString primary=*(res.begin());
    ret+=QString("  , CONSTRAINT %1 PRIMARY KEY\n").arg(primary.lower());
    ret+=indexColumns("      ",owner,primary);
  }
  ret+=")\n";
  if (Connection.version()>="8.0") {
    ret+="ORGANIZATION        ";
    ret+=organization;
    ret+="\n";
  }
  if (Connection.version()>="8.1") {
    ret+=monitoring;
    ret+="\n";
  }
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

static toSQL SQLTableColumns("toExtract:TableColumns",
			     "SELECT  RPAD(LOWER(column_name),32)
     || RPAD(
             DECODE(
                     data_type
                    ,'NUMBER',DECODE(
                                      data_precision
                                     ,null,DECODE(
                                                   data_scale
                                                  ,0,'INTEGER'
                                                  ,  'NUMBER   '
                                                 )
                                     ,'NUMBER   '
                                    )
                    ,'RAW'     ,'RAW      '
                    ,'CHAR'    ,'CHAR     '
                    ,'NCHAR'   ,'NCHAR    '
                    ,'UROWID'  ,'UROWID   '
                    ,'VARCHAR2','VARCHAR2 '
                    ,data_type
                   )
               || DECODE(
                          data_type
                         ,'DATE',null
                         ,'LONG',null
                         ,'NUMBER',DECODE(
                                           data_precision
                                          ,null,null
                                          ,'('
                                         )
                         ,'RAW'      ,'('
                         ,'CHAR'     ,'('
                         ,'NCHAR'    ,'('
                         ,'UROWID'   ,'('
                         ,'VARCHAR2' ,'('
                         ,'NVARCHAR2','('
                         ,null
                        )
               || DECODE(
                          data_type
                         ,'RAW'      ,data_length
                         ,'CHAR'     ,data_length
                         ,'NCHAR'    ,data_length
                         ,'UROWID'   ,data_length
                         ,'VARCHAR2' ,data_length
                         ,'NVARCHAR2',data_length
                         ,'NUMBER'   ,data_precision
                         , null
                        )
               || DECODE(
                          data_type
                         ,'NUMBER',DECODE(
                           data_precision
                          ,null,null
                          ,DECODE(
                                   data_scale
                                  ,null,null
                                  ,0   ,null
                                  ,',' || data_scale
                                 )
                              )
                        )
               || DECODE(
                          data_type
                         ,'DATE',null
                         ,'LONG',null
                         ,'NUMBER',DECODE(
                                           data_precision
                                          ,null,null
                                          ,')'
                                         )
                         ,'RAW'      ,')'
                         ,'CHAR'     ,')'
                         ,'NCHAR'    ,')'
                         ,'UROWID'   ,')'
                         ,'VARCHAR2' ,')'
                         ,'NVARCHAR2',')'
                         ,null
                        )
             ,32
            )
     || DECODE(
                nullable
               ,'N','NOT NULL'
               ,     null
              )
  FROM all_tab_columns
 WHERE table_name = :nam<char[100]>
   AND owner = :own<char[100]>
 ORDER BY column_id",
			     "Extract column definitions from table",
			     "8.0");
static toSQL SQLTableColumns7("toExtract:TableColumns",
			      "       SELECT
        RPAD(LOWER(column_name),32)
     || RPAD(
             DECODE(
                     data_type
                    ,'NUMBER',DECODE(
                                      data_precision
                                     ,null,DECODE(
                                                   data_scale
                                                  ,0,'INTEGER'
                                                  ,  'NUMBER   '
                                                 )
                                     ,'NUMBER   '
                                    )
                    ,'RAW'     ,'RAW      '
                    ,'CHAR'    ,'CHAR     '
                    ,'NCHAR'   ,'NCHAR    '
                    ,'UROWID'  ,'UROWID   '
                    ,'VARCHAR2','VARCHAR2 '
                    ,data_type
                   )
               || DECODE(
                          data_type
                         ,'DATE',null
                         ,'LONG',null
                         ,'NUMBER',DECODE(
                                           data_precision
                                          ,null,null
                                          ,'('
                                         )
                         ,'RAW'      ,'('
                         ,'CHAR'     ,'('
                         ,'NCHAR'    ,'('
                         ,'UROWID'   ,'('
                         ,'VARCHAR2' ,'('
                         ,'NVARCHAR2','('
                         ,null
                        )
               || DECODE(
                          data_type
                         ,'RAW'      ,data_length
                         ,'CHAR'     ,data_length
                         ,'NCHAR'    ,data_length
                         ,'UROWID'   ,data_length
                         ,'VARCHAR2' ,data_length
                         ,'NVARCHAR2',data_length
                         ,'NUMBER'   ,data_precision
                         , null
                        )
               || DECODE(
                          data_type
                         ,'NUMBER',DECODE(
                           data_precision
                          ,null,null
                          ,DECODE(
                                   data_scale
                                  ,null,null
                                  ,0   ,null
                                  ,',' || data_scale
                                 )
                              )
                        )
               || DECODE(
                          data_type
                         ,'DATE',null
                         ,'LONG',null
                         ,'NUMBER',DECODE(
                                           data_precision
                                          ,null,null
                                          ,')'
                                         )
                         ,'RAW'      ,')'
                         ,'CHAR'     ,')'
                         ,'NCHAR'    ,')'
                         ,'UROWID'   ,')'
                         ,'VARCHAR2' ,')'
                         ,'NVARCHAR2',')'
                         ,null
                        )
             ,33
            )
     || DECODE(
                nullable
               ,'N','NOT NULL'
               ,     null
              )
  FROM all_tab_columns
 WHERE table_name = :nam<char[100]>
   AND owner = :own<char[100]>
 ORDER BY column_id",
			      QString::null,
			      "7.0");

QString toExtract::tableColumns(const QString &owner,const QString &name)
{
  otl_stream inf(1,
		 SQLTableColumns(Connection),
		 Connection.connection());
  inf<<name.utf8();
  inf<<owner.utf8();
  bool first=true;
  QString ret;
  while(!inf.eof()) {
    char buffer[1024];
    if (first)
      first=false;
    else
      ret+="\n  , ";
    inf>>buffer;
    ret+=QString::fromUtf8(buffer);
  }
  ret+="\n";
  return ret;
}

static toSQL SQLDisplaySource("toExtract:ListSource",
			      "SELECT text
  FROM all_source
 WHERE type = :typ<char[100]>
   AND name = :nam<char[100]>
   AND owner = :own<char[100]>
 ORDER BY line",
			      "Get source of an object from the database, "
			      "must have same columns and binds");

QString toExtract::displaySource(const QString &schema,const QString &owner,
				 const QString &name,const QString &type)
{
  static QRegExp StripType("^[a-zA-Z]+[ \t]+[^ \t]");
  otl_stream inf(1,
		 SQLDisplaySource(Connection),
		 Connection.connection());
  inf<<type.utf8()<<name.utf8()<<owner.utf8();
  if (inf.eof())
    throw QString("Couldn't find source for of %1.%2").arg(owner).arg(name);
  
  QString ret;
  if (Prompt)
    ret=QString("PROMPT CREATE OR REPLACE %1 %2%3\n\n").
      arg(type).
      arg(schema).
      arg(name.lower());
  ret+="CREATE OR REPLACE ";
  bool first=true;
  while(!inf.eof()) {
    char buffer[1024];
    inf>>buffer;
    QString line=QString::fromUtf8(buffer);
    if (first) {
      line.replace(StripType,"");
      line.prepend(QString("%1 %2%3").arg(type).arg(schema).arg(name.lower()));
      first=false;
    }
    ret+=line;
  }
  ret+="\n/\n\n";
  return ret;
}

QString toExtract::createFunction(const QString &schema,const QString &owner,const QString &name)
{
  return displaySource(schema,owner,name,"FUNCTION");
}

static toSQL SQLIndexInfo7("toExtract:IndexInfo",
			   "SELECT  'N/A'                           AS partitioned
      , table_name
      , table_owner
      , DECODE(
                uniqueness
               ,'UNIQUE',' UNIQUE'
               ,null
              )
      , null                            AS bitmap
      , null                            AS domain
      , null
      , null
      , null
  FROM all_indexes
 WHERE index_name = :nam<char[100]>
   AND owner = :own<char[100]>",
			   "Initial information about an index, same binds and columns",
			   "7.0");
static toSQL SQLIndexInfo("toExtract:IndexInfo",
			  "SELECT partitioned
      , table_name
      , table_owner
      , DECODE(
                uniqueness
               ,'UNIQUE',' UNIQUE'
               ,null
              )
      , DECODE(
                index_type
               ,'BITMAP',' BITMAP'
               ,null
              )
      , DECODE(
                index_type
               ,'DOMAIN','DOMAIN'
               ,null
              )
      , ityp_owner
      , ityp_name
      , parameters
  FROM all_indexes
 WHERE index_name = :nam<char[100]>
   AND owner = :own<char[100]>",
			  QString::null,
			  "8.0");

static toSQL SQLIndexSegment("toExtract:IndexSegment",
			     "SELECT  LTRIM(i.degree)
      , LTRIM(i.instances)
      , DECODE(
                i.compression
               ,'ENABLED',i.prefix_length
               ,0
              )                             AS compressed
        -- Physical Properties
      , 'INDEX'                       AS organization
        -- Segment Attributes
      , 'N/A'                         AS cache
      , 'N/A'                         AS pct_used
      , i.pct_free
      , DECODE(
                i.ini_trans
               ,0,1
               ,null,1
               ,i.ini_trans
              )                       AS ini_trans
      , DECODE(
                i.max_trans
               ,0,255
               ,null,255
               ,i.max_trans
              )                       AS max_trans
        -- Storage Clause
      , i.initial_extent
      , i.next_extent
      , i.min_extents
      , DECODE(
                i.max_extents
               ,2147483645,'unlimited'
               ,           i.max_extents
              )                       AS max_extents
      , i.pct_increase
      , NVL(i.freelists,1)
      , NVL(i.freelist_groups,1)
      , LOWER(i.buffer_pool)          AS buffer_pool
      , DECODE(
                i.logging
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(i.tablespace_name)      AS tablespace_name
      , s.blocks
  FROM  all_indexes   i
      , dba_segments  s
 WHERE  i.index_name   = :nam<char[100]>
   AND  s.segment_name = i.index_name
   AND  i.owner        = :own<char[100]>
   AND  s.owner        = i.owner",
			     "Get information about how index is stored",
			     "8.1");

static toSQL SQLIndexSegment8("toExtract:IndexSegment",
			      "SELECT  LTRIM(i.degree)
      , LTRIM(i.instances)
      , 0                             AS compressed
        -- Physical Properties
      , 'INDEX'                       AS organization
        -- Segment Attributes
      , 'N/A'                         AS cache
      , 'N/A'                         AS pct_used
      , i.pct_free
      , DECODE(
                i.ini_trans
               ,0,1
               ,null,1
               ,i.ini_trans
              )                       AS ini_trans
      , DECODE(
                i.max_trans
               ,0,255
               ,null,255
               ,i.max_trans
              )                       AS max_trans
        -- Storage Clause
      , i.initial_extent
      , i.next_extent
      , i.min_extents
      , DECODE(
                i.max_extents
               ,2147483645,'unlimited'
               ,           i.max_extents
              )                       AS max_extents
      , i.pct_increase
      , NVL(i.freelists,1)
      , NVL(i.freelist_groups,1)
      , LOWER(i.buffer_pool)          AS buffer_pool
      , DECODE(
                i.logging
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(i.tablespace_name)      AS tablespace_name
      , s.blocks
  FROM  all_indexes   i
      , dba_segments  s
 WHERE  i.index_name   = :nam<char[100]>
   AND  s.segment_name = i.index_name
   AND  i.owner        = :own<char[100]>
   AND  s.owner        = i.owner",
			      QString::null,
			      "8.0");

static toSQL SQLIndexSegment7("toExtract:IndexSegment",
			      "SELECT  'N/A'                         AS degree
      , 'N/A'                         AS instances
      , 0                             AS compressed
        -- Physical Properties
      , 'INDEX'                       AS organization
        -- Segment Attributes
      , 'N/A'                         AS cache
      , 'N/A'                         AS pct_used
      , i.pct_free
      , DECODE(
                i.ini_trans
               ,0,1
               ,null,1
               ,i.ini_trans
              )                       AS ini_trans
      , DECODE(
                i.max_trans
               ,0,255
               ,null,255
               ,i.max_trans
              )                       AS max_trans
        -- Storage Clause
      , i.initial_extent
      , i.next_extent
      , i.min_extents
      , DECODE(
                i.max_extents
               ,2147483645,'unlimited'
               ,           i.max_extents
              )                       AS max_extents
      , i.pct_increase
      , NVL(i.freelists,1)
      , NVL(i.freelist_groups,1)
      , 'N/A'                         AS buffer_pool
      , 'N/A'                         AS logging
      , LOWER(i.tablespace_name)      AS tablespace_name
      , s.blocks
  FROM  all_indexes   i
      , dba_segments  s
 WHERE  i.index_name   = :nam<char[100]>
   AND  s.segment_name = i.index_name
   AND  i.owner        = :own<char[100]>
   AND  s.owner        = i.owner",
			      QString::null,
			      "7.0");

QString toExtract::createIndex(const QString &schema,const QString &owner,const QString &name)
{
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

  QString schema2=setSchema(tableOwner);

  QString ret;
  QString sql=QString("CREATE%1%2 INDEX %3%4 ON %5%6\n").
    arg(unique).
    arg(bitmap).
    arg(schema).
    arg(name).
    arg(schema2).
    arg(table);
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
  if (Connection.version()>="8.0") {
    ret+=QString("PARALLEL\n"
		 "(\n"
		 "  DEGREE            %1\n"
		 "  INSTANCES         %2\n"
		 ")\n").
      arg(degree).
      arg(instances);
  }
  if (partitioned == "YES")
    return createPartitionedIndex(schema,owner,name,ret);

  toUnShift(storage,"");
  ret+=segmentAttributes(storage);
  if (!compressed.isEmpty()) {
    ret+="COMPRESS            ";
    ret+=compressed;
    ret+="\n";
  }
  ret+=";\n\n";
  return ret;
}

static toSQL SQLIndexPartition8("toExtract:IndexPartition",
				"SELECT  -- 8.0 Indexes may partition only by RANGE
        i.partitioning_type
      , 'N/A'                         AS subpartitioning_type
      , i.locality
      , 0                             AS compressed
        -- Physical Properties
      , 'INDEX'                       AS organization
        -- Segment Attributes
      , 'N/A'                         AS cache
      , 'N/A'                         AS pct_used
      , i.def_pct_free
      , DECODE(
                i.def_ini_trans
               ,0,1
               ,null,1
               ,i.def_ini_trans
              )                       AS ini_trans
      , DECODE(
                i.def_max_trans
               ,0,255
               ,null,255
               ,i.def_max_trans
              )                       AS max_trans
        -- Storage Clause
      ,DECODE(
               i.def_initial_extent
              ,'DEFAULT',s.initial_extent
              ,i.def_initial_extent * :bs<char[100]> * 1024
             )                        AS initial_extent
      ,DECODE(
               i.def_next_extent
              ,'DEFAULT',s.next_extent
              ,i.def_next_extent * :bs<char[100]> * 1024
             )                        AS next_extent
      , DECODE(
                i.def_min_extents
               ,'DEFAULT',s.min_extents
               ,i.def_min_extents
              )                       AS min_extents
      , DECODE(
                i.def_max_extents
               ,'DEFAULT',DECODE(
                                  s.max_extents
                                 ,2147483645,'unlimited'
                                 ,s.max_extents
                                )
               ,2147483645,'unlimited'
               ,i.def_max_extents
              )                       AS max_extents
      , DECODE(
                i.def_pct_increase
               ,'DEFAULT',s.pct_increase
               ,i.def_pct_increase
              )                       AS pct_increase
      , DECODE(
                i.def_freelists
               ,0,1
               ,null,1
               ,i.def_freelists
              )                       AS freelists
      , DECODE(
                i.def_freelist_groups
               ,0,1
               ,null,1
               ,i.def_freelist_groups
              )                       AS freelist_groups
      , 'N/A'                         AS buffer_pool
      , DECODE(
                i.def_logging
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(NVL(i.def_tablespace_name,s.tablespace_name))
        -- Don't have default blocks, so use larger of initial/next
      , GREATEST(
                  DECODE(
                          i.def_initial_extent
                         ,'DEFAULT',s.initial_extent / :bs<char[100]> / 1024
                         ,i.def_initial_extent
                        )
                 ,DECODE(
                          i.def_next_extent
                         ,'DEFAULT',s.next_extent / :bs<char[100]> / 1024
                         ,i.def_next_extent
                        )
                )                     AS blocks
 FROM
        all_part_indexes  i
      , dba_tablespaces   s
      , all_part_tables   t
 WHERE
            -- def_tablspace is sometimes NULL in PART_INDEXES,
            -- we'll have to go over to the table for the defaults
            i.index_name      = :nam<char[100]>
        AND t.table_name      = i.table_name
        AND s.tablespace_name = t.def_tablespace_name
        AND i.owner           = :own<char[100]>
        AND n.owner           = i.owner
        AND t.owner           = i.owner",
				"Get information about index partitions, "
				"must have same columns and same columns",
				"8.0");

static toSQL SQLIndexPartition("toExtract:IndexPartition",
			       "SELECT  -- Indexes may partition only by RANGE or RANGE/HASH
        i.partitioning_type
      , i.subpartitioning_type
      , i.locality
      , DECODE(
                n.compression
               ,'ENABLED',n.prefix_length
               ,0
              )                             AS compressed
        -- Physical Properties
      , 'INDEX'                       AS organization
        -- Segment Attributes
      , 'N/A'                         AS cache
      , 'N/A'                         AS pct_used
      , i.def_pct_free
      , DECODE(
                i.def_ini_trans
               ,0,1
               ,null,1
               ,i.def_ini_trans
              )                       AS ini_trans
      , DECODE(
                i.def_max_trans
               ,0,255
               ,null,255
               ,i.def_max_trans
              )                       AS max_trans
        -- Storage Clause
      ,DECODE(
               i.def_initial_extent
              ,'DEFAULT',s.initial_extent
              ,i.def_initial_extent * :bs<char[100]> * 1024
             )                        AS initial_extent
      ,DECODE(
               i.def_next_extent
              ,'DEFAULT',s.next_extent
              ,i.def_next_extent * :bs<char[100]> * 1024
             )                        AS next_extent
      , DECODE(
                i.def_min_extents
               ,'DEFAULT',s.min_extents
               ,i.def_min_extents
              )                       AS min_extents
      , DECODE(
                i.def_max_extents
               ,'DEFAULT',DECODE(
                                  s.max_extents
                                 ,2147483645,'unlimited'
                                 ,s.max_extents
                                )
               ,2147483645,'unlimited'
               ,i.def_max_extents
              )                       AS max_extents
      , DECODE(
                i.def_pct_increase
               ,'DEFAULT',s.pct_increase
               ,i.def_pct_increase
              )                       AS pct_increase
      , DECODE(
                i.def_freelists
               ,0,1
               ,null,1
               ,i.def_freelists
              )                       AS freelists
      , DECODE(
                i.def_freelist_groups
               ,0,1
               ,null,1
               ,i.def_freelist_groups
              )                       AS freelist_groups
      , LOWER(i.def_buffer_pool)        AS buffer_pool
      , DECODE(
                i.def_logging
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(NVL(i.def_tablespace_name,s.tablespace_name))
        -- Don't have default blocks, so use larger of initial/next
      , GREATEST(
                  DECODE(
                          i.def_initial_extent
                         ,'DEFAULT',s.initial_extent / :bs<char[100]> / 1024
                         ,i.def_initial_extent
                        )
                 ,DECODE(
                          i.def_next_extent
                         ,'DEFAULT',s.next_extent / :bs<char[100]> / 1024
                         ,i.def_next_extent
                        )
                )                     AS blocks
 FROM
        all_part_indexes  i
      , all_indexes       n
      , dba_tablespaces   s
      , all_part_tables   t
 WHERE
            -- def_tablspace is sometimes NULL in PART_INDEXES,
            -- we'll have to go over to the table for the defaults
            i.index_name      = :nam<char[100]>
        AND n.index_name      = i.index_name
        AND t.table_name      = i.table_name
        AND s.tablespace_name = t.def_tablespace_name
        AND i.owner           = :own<char[100]>
        AND n.owner           = i.owner
        AND t.owner           = i.owner",
			       QString::null,
			       "8.1");

QString toExtract::createPartitionedIndex(const QString &schema,const QString &owner,
					  const QString &name,const QString &sql)
{
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
			   "SELECT  column_name
  FROM all_%1_key_columns
 WHERE name           = :nam<char[100]>
   AND owner          = :owner<char[100]>
   AND object_type LIKE :typ<char[100]>||'%'",
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
				"SELECT  partition_name
      , high_value
      , 'N/A'                         AS cache
      , 'N/A'                         AS pct_used
      , pct_free
      , ini_trans
      , max_trans
        -- Storage Clause
      , initial_extent
      , next_extent
      , min_extent
      , DECODE(
                max_extent
               ,2147483645,'unlimited'
               ,           max_extent
              )                       AS max_extents
      , pct_increase
      , NVL(freelists,1)
      , NVL(freelist_groups,1)
      , LOWER(buffer_pool)
      , DECODE(
                logging 
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , tablespace_name
      , leaf_blocks                   AS blocks
 FROM   all_ind_partitions
 WHERE      index_name  =  :nam<char[100]>
        AND index_owner =  :own<char[100]>
 ORDER BY partition_name",
				"Get information about partition ranges, must have same binds "
				"and columns",
				"8.1");

static toSQL SQLRangePartitions8("toExtract:RangePartitions",
				 "SELECT  partition_name
      , high_value
      , 'N/A'                         AS cache
      , 'N/A'                         AS pct_used
      , pct_free
      , ini_trans
      , max_trans
        -- Storage Clause
      , initial_extent
      , next_extent
      , min_extent
      , DECODE(
                max_extent
               ,2147483645,'unlimited'
               ,           max_extent
              )                       AS max_extents
      , pct_increase
      , NVL(freelists,1)
      , NVL(freelist_groups,1)
      , 'N/A'                         AS buffer_pool
      , DECODE(
                logging 
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , tablespace_name
      , leaf_blocks                   AS blocks
 FROM   all_ind_partitions
 WHERE      index_name  =  :nam<char[100]>
        AND index_owner =  :own<char[100]>
 ORDER BY partition_name",
				 QString::null,
				 "8.1");

static toSQL SQLIndexSubPartitionName("toExtract:IndexSubPartitionName",
				      "SELECT subpartition_name,
       tablespace_name
  FROM all_ind_subpartitions
 WHERE index_name     = :ind<char[100]>
   AND partition_name = :prt<char[100]>
   AND index_owner    = :own<char[100]>
 ORDER BY subpartition_name",
				      "Get information about a subpartition, "
				      "must have same binds and columns");

QString toExtract::rangePartitions(const QString &owner,const QString &name,
				   const QString &subPartitionType,const QString &caller)
{
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
    if (caller=="LOCAL")
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

QString toExtract::createMaterializedView(const QString &schema,const QString &owner,
					   const QString &name)
{
  return createMView(schema,owner,name,"MATERIALIZED VIEW");
}

QString toExtract::createMaterializedViewLog(const QString &schema,const QString &owner,
					     const QString &name)
{
  return createMViewLog(schema,owner,name,"MATERIALIZED VIEW LOG");
}

static toSQL SQLMViewInfo("toExtract:MaterializedViewInfo",
			  "       SELECT
              m.container_name
            , DECODE(
                      m.build_mode
                     ,'YES','USING PREBUILT TABLE'
                     ,DECODE(
                              m.last_refresh_date
                             ,null,'BUILD DEFERRED'
                             ,'BUILD IMMEDIATE'
                            )
                    )                                  AS build_mode
            , DECODE(
                      m.refresh_method
                     ,'NEVER','NEVER REFRESH'
                     ,'REFRESH ' || m.refresh_method
                    )                                  AS refresh_method
            , DECODE(
                      m.refresh_mode
                     ,'NEVER',null
                     ,'ON ' || m.refresh_mode
                    )                                  AS refresh_mode
            , TO_CHAR(s.start_with, 'DD-MON-YYYY HH24:MI:SS')
                                                       AS start_with
            , s.next
            , DECODE(
                      s.refresh_method
                     ,'PRIMARY KEY','WITH  PRIMARY KEY'
                     ,'ROWID'      ,'WITH  ROWID'
                     ,null
                    )                                  AS using_pk
            , s.master_rollback_seg
            , DECODE(
                      m.updatable
                     ,'N',null
                     ,DECODE(
                              m.rewrite_enabled
                             ,'Y','FOR UPDATE ENABLE QUERY REWRITE'
                             ,'N','FOR UPDATE DISABLE QUERY REWRITE'
                            )
                    )                                  AS updatable
            , s.query
       FROM
              all_mviews     m
            , all_snapshots  s
       WHERE
                  m.mview_name  = :nam<char[100]>
              AND s.name        = m.mview_name
              AND m.owner       = :own<char[100]>
              AND s.owner       = m.owner",
			  "Get information about materialized view, must have same columns and binds");

static toSQL SQLIndexName("toExtract:TableIndexes",
			  "SELECT index_name
  FROM all_indexes
 WHERE table_name = :nam<char[100]>
   AND owner = own<char[100]>",
			  "Get indexes available to a table, must have same binds and columns");

QString toExtract::createMView(const QString &schema,const QString &owner,
			       const QString &name,const QString &type)
{
  list<QString> result=toReadQuery(Connection,SQLMViewInfo(Connection),name,owner);
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
    if (!masterRBSeg.isEmpty())
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

QString toExtract::createMViewTable(const QString &schema,const QString &owner,
				    const QString &name)
{
  IsASnapTable=true;

  static QRegExp parallel("^PARALLEL");

  bool started=false;
  bool done=false;

  QString initial=createTable(schema,owner,name);
  QStringList linesIn=QStringList::split("\n",initial,true);
  QString ret;

  for(QStringList::Iterator i=linesIn.begin();i!=linesIn.end()&&!done;i++) {
    if(parallel.match(*i))
      started=true;
    if (started) {
      QString line=*i;
      if (line.length()>0&&line[line.length()-1]==';') {
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

QString toExtract::createMViewIndex(const QString &schema,const QString &owner,
				    const QString &name)
{
  IsASnapIndex=true;

  static QRegExp start("^INITRANS");
  static QRegExp ignore("LOGGING");

  bool started=false;
  bool done=false;

  QString initial=createTable(schema,owner,name);
  QStringList linesIn=QStringList::split("\n",initial,true);
  QString ret;

  for(QStringList::Iterator i=linesIn.begin();i!=linesIn.end()&&!done;i++) {
    if(start.match(*i))
      started=true;
    if (started) {
      QString line=*i;
      if (line.length()>0&&line[line.length()-1]==';') {
	line.truncate(line.length()-1);
	done=true;
      }
      if (!ignore.match(line)&&line.length()) {
	ret+=line;
	ret+="\n";
      }
    }
  }

  IsASnapIndex=false;
  return ret;
}

static toSQL SQLSnapshotInfo("toExtract:SnapshotInfo",
			     "SELECT log_table,
       rowids,
       primary_key,
       filter_columns
  FROM all_snapshot_logs
   AND master = :nam<char[100]>
   AND log_owner = :own<char[100]>",
			     "Get information about snapshot or materialized view log, "
			     "must have same binds and columns");

static toSQL SQLSnapshotColumns("toExtract:SnapshotColumns",
				"SELECT
        column_name
 FROM
        dba_snapshot_log_filter_cols
 WHERE
            name  = :nam<char[100]>
        AND owner = :own<char[100]>
 MINUS
 SELECT
        column_name
 FROM
        all_cons_columns  c
      , all_constraints   d
 WHERE
            d.table_name      = :nam<char[100]>
        AND d.constraint_type = 'P'
        AND c.table_name      = d.table_name
        AND c.constraint_name = d.constraint_name
        AND d.owner           = :own<char[100]>
        AND c.owner           = d.owner",
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

static toSQL SQLTableType("toExtract:TableType",
			  "SELECT partitioned,
      iot_type
 FROM all_tables
WHERE table_name = :nam<char[100]>
  AND owner = :own<char[100]>",
			  "Get table type, must have same columns and binds",
			  "8.0");

static toSQL SQLTableType7("toExtract:TableType",
			   "SELECT 'NO',
      'NOT IOT'
 FROM all_tables
WHERE table_name = :nam<char[100]>
  AND owner = :own<char[100]>",
			   QString::null,
			   "7.0");

static toSQL SQLTableInfo("toExtract:TableInfo",
			  "SELECT
        -- Table Properties
        DECODE(
                t.monitoring
               ,'NO','NOMONITORING'
               ,     'MONITORING'
              )                       AS monitoring
      , 'N/A'                         AS table_name
        -- Parallel Clause
      , LTRIM(t.degree)
      , LTRIM(t.instances)
        -- Physical Properties
      , DECODE(
                t.iot_type
               ,'IOT','INDEX'
               ,      'HEAP'
              )                       AS organization
        -- Segment Attributes
      , DECODE(
                LTRIM(t.cache)
               ,'Y','CACHE'
               ,    'NOCACHE'
              )
      , t.pct_used
      , t.pct_free
      , DECODE(
                t.ini_trans
               ,0,1
               ,null,1
               ,t.ini_trans
              )                       AS ini_trans
      , DECODE(
                t.max_trans
               ,0,255
               ,null,255
               ,t.max_trans
              )                       AS max_trans
        -- Storage Clause
      , t.initial_extent
      , t.next_extent
      , t.min_extents
      , DECODE(
                t.max_extents
               ,2147483645,'unlimited'
               ,           t.max_extents
              )                       AS max_extents
      , NVL(t.pct_increase,0)
      , NVL(t.freelists,1)
      , NVL(t.freelist_groups,1)
      , LOWER(t.buffer_pool)          AS buffer_pool
      , DECODE(
                t.logging
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(t.tablespace_name)      AS tablespace_name
      , s.blocks - NVL(t.empty_blocks,0)
 FROM
        all_tables    t
      , dba_segments  s
 WHERE
            t.table_name   = :nam<char[100]>
        AND t.table_name   = s.segment_name
        AND s.owner        = :own<char[100]>
        AND t.owner        = s.owner",
			  "Get information about a vanilla table, must have same binds and columns",
			  "8.1");

static toSQL SQLTableInfo8("toExtract:TableInfo",
			   "SELECT
        -- Table Properties
        'N/A'                         AS monitoring
      , 'N/A'                         AS table_name
        -- Parallel Clause
      , LTRIM(t.degree)
      , LTRIM(t.instances)
        -- Physical Properties
      , DECODE(
                t.iot_type
               ,'IOT','INDEX'
               ,      'HEAP'
              )                       AS organization
        -- Segment Attributes
      , DECODE(
                LTRIM(t.cache)
               ,'Y','CACHE'
               ,    'NOCACHE'
              )
      , t.pct_used
      , t.pct_free
      , DECODE(
                t.ini_trans
               ,0,1
               ,null,1
               ,t.ini_trans
              )                       AS ini_trans
      , DECODE(
                t.max_trans
               ,0,255
               ,null,255
               ,t.max_trans
              )                       AS max_trans
        -- Storage Clause
      , t.initial_extent
      , t.next_extent
      , t.min_extents
      , DECODE(
                t.max_extents
               ,2147483645,'unlimited'
               ,           t.max_extents
              )                       AS max_extents
      , NVL(t.pct_increase,0)
      , NVL(t.freelists,1)
      , NVL(t.freelist_groups,1)
      , 'N/A'                         AS buffer_pool
      , DECODE(
                t.logging
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(t.tablespace_name)      AS tablespace_name
      , s.blocks - NVL(t.empty_blocks,0)
 FROM
        all_tables    t
      , dba_segments  s
 WHERE
            t.table_name   = :nam<char[100]>
        AND t.table_name   = s.segment_name
        AND s.owner        = :own<char[100]>
        AND t.owner        = s.owner",
			   QString::null,
			   "8.0");

static toSQL SQLTableInfo7("toExtract:TableInfo",
			   "SELECT
        -- Table Properties
        'N/A'                         AS monitoring
      , 'N/A'                         AS table_name
        -- Parallel Clause
      , LTRIM(t.degree)
      , LTRIM(t.instances)
        -- Physical Properties
      , 'N/A'                         AS organization
        -- Segment Attributes
      , DECODE(
                LTRIM(t.cache)
               ,'Y','CACHE'
               ,    'NOCACHE'
              )
      , t.pct_used
      , t.pct_free
      , DECODE(
                t.ini_trans
               ,0,1
               ,null,1
               ,t.ini_trans
              )                       AS ini_trans
      , DECODE(
                t.max_trans
               ,0,255
               ,null,255
               ,t.max_trans
              )                       AS max_trans
        -- Storage Clause
      , t.initial_extent
      , t.next_extent
      , t.min_extents
      , DECODE(
                t.max_extents
               ,2147483645,'unlimited'
               ,           t.max_extents
              )                       AS max_extents
      , NVL(t.pct_increase,0)
      , NVL(t.freelists,1)
      , NVL(t.freelist_groups,1)
      , 'N/A'                         AS buffer_pool
      , 'N/A'                         AS logging
      , LOWER(t.tablespace_name)      AS tablespace_name
      , s.blocks - NVL(t.empty_blocks,0)
 FROM
        all_tables    t
      , dba_segments  s
 WHERE
            t.table_name   = :nam<char[100]>
        AND t.table_name   = s.segment_name
        AND s.owner        = :own<char[100]>
        AND t.owner        = s.owner",
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
    if (partitioned=="YES")
      return createPartitionedIOT(schema,owner,name);
    else
      return createIOT(schema,owner,name);
  } else if (partitioned=="YES")
    return createPartitionedTable(schema,owner,name);

  list<QString> result=toReadQuery(Connection,SQLTableInfo(Connection),name,owner);
  QString ret=createTableText(result,schema,owner,name);
  ret+=";\n\n";
  ret+=createComments(schema,owner,name);
  return ret;
}

static toSQL SQLPartitionedIOTInfo("toExtract:PartitionedIOTInfo",
				   "SELECT
        -- Table Properties
        DECODE(
                t.monitoring
               ,'NO','NOMONITORING'
               ,     'MONITORING'
              )                       AS monitoring
      , t.table_name
        -- Parallel Clause
      , LTRIM(t.degree)               AS degree
      , LTRIM(t.instances)            AS instances
        -- Physical Properties
      , 'INDEX'                       AS organization
        -- Segment Attributes
      , DECODE(
                LTRIM(t.cache)
               ,'Y','CACHE'
               ,    'NOCACHE'
              )                       AS cache
      , 'N/A'                         AS pct_used
      , p.def_pct_free                AS pct_free
      , p.def_ini_trans               AS ini_trans
      , p.def_max_trans               AS max_trans
        -- Storage Clause
      ,DECODE(
               p.def_initial_extent
              ,'DEFAULT',s.initial_extent
              ,p.def_initial_extent * :bs<char[100]> * 1024
             )                        AS initial_extent
      ,DECODE(
               p.def_next_extent
              ,'DEFAULT',s.next_extent
              ,p.def_next_extent * :bs<char[100]> * 1024
             )                        AS next_extent
      , DECODE(
                p.def_min_extents
               ,'DEFAULT',s.min_extents
               ,p.def_min_extents
              )                       AS min_extents
      , DECODE(
                p.def_max_extents
               ,'DEFAULT',DECODE(
                                  s.max_extents
                                 ,2147483645,'unlimited'
                                 ,s.max_extents
                                )
               ,2147483645,'unlimited'
               ,p.def_max_extents
              )                       AS max_extents
      , DECODE(
                p.def_pct_increase
               ,'DEFAULT',s.pct_increase
               ,p.def_pct_increase
              )                       AS pct_increase
      , DECODE(
                p.def_freelists
               ,0,1
               ,NVL(p.def_freelists,1)
              )                       AS freelists
      , DECODE(
                p.def_freelist_groups
               ,0,1
               ,NVL(p.def_freelist_groups,1)
              )                       AS freelist_groups
      , LOWER(p.def_buffer_pool)      AS buffer_pool
      , DECODE(
                p.def_logging 
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(p.def_tablespace_name)  AS tablespace_name
      , t.blocks - NVL(t.empty_blocks,0)
 FROM
        all_all_tables    t
      , all_part_indexes  p
      , dba_tablespaces   s
 WHERE
            t.table_name      = :name<char[100]>
        AND p.table_name      = t.table_name
        AND s.tablespace_name = p.def_tablespace_name
        AND t.owner           = :own<char[100]>
        AND p.owner           = t.owner",
				   "Get information about a partitioned indexed organized table, "
				   "must have same columns and binds",
				   "8.1");

static toSQL SQLPartitionedIOTInfo8("toExtract:PartitionedIOTInfo",
				    "SELECT
        -- Table Properties
        'N/A'                         AS monitoring
      , t.table_name
        -- Parallel Clause
      , LTRIM(t.degree)               AS degree
      , LTRIM(t.instances)            AS instances
        -- Physical Properties
      , 'INDEX'                       AS organization
        -- Segment Attributes
      , DECODE(
                LTRIM(t.cache)
               ,'Y','CACHE'
               ,    'NOCACHE'
              )                       AS cache
      , 'N/A'                         AS pct_used
      , p.def_pct_free                AS pct_free
      , p.def_ini_trans               AS ini_trans
      , p.def_max_trans               AS max_trans
        -- Storage Clause
      ,DECODE(
               p.def_initial_extent
              ,'DEFAULT',s.initial_extent
              ,p.def_initial_extent * :bs<char[100]> * 1024
             )                        AS initial_extent
      ,DECODE(
               p.def_next_extent
              ,'DEFAULT',s.next_extent
              ,p.def_next_extent * :bs<char[100]> * 1024
             )                        AS next_extent
      , DECODE(
                p.def_min_extents
               ,'DEFAULT',s.min_extents
               ,p.def_min_extents
              )                       AS min_extents
      , DECODE(
                p.def_max_extents
               ,'DEFAULT',DECODE(
                                  s.max_extents
                                 ,2147483645,'unlimited'
                                 ,s.max_extents
                                )
               ,2147483645,'unlimited'
               ,p.def_max_extents
              )                       AS max_extents
      , DECODE(
                p.def_pct_increase
               ,'DEFAULT',s.pct_increase
               ,p.def_pct_increase
              )                       AS pct_increase
      , DECODE(
                p.def_freelists
               ,0,1
               ,NVL(p.def_freelists,1)
              )                       AS freelists
      , DECODE(
                p.def_freelist_groups
               ,0,1
               ,NVL(p.def_freelist_groups,1)
              )                       AS freelist_groups
      , 'N/A'                         AS buffer_pool
      , DECODE(
                p.def_logging 
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(p.def_tablespace_name)  AS tablespace_name
      , t.blocks - NVL(t.empty_blocks,0)
 FROM
        all_all_tables    t
      , all_part_indexes  p
      , dba_tablespaces   s
 WHERE
            t.table_name      = :name<char[100]>
        AND p.table_name      = t.table_name
        AND s.tablespace_name = p.def_tablespace_name
        AND t.owner           = :own<char[100]>
        AND p.owner           = t.owner",
				    QString::null,
				    "8.0");

static toSQL SQLIndexNames("toExtract:IndexNames",
			   "SELECT index_name
  FROM all_part_indexes
 WHERE table_name = :nam<char[100]>
   AND owner      = :own<char[100]>
 ORDER BY index_name",
			   "Index names of table, "
			   "must have same binds and columns");

QString toExtract::createPartitionedIOT(const QString &schema,const QString &owner,
					const QString &name)
{
  list<QString> result=toReadQuery(Connection,SQLPartitionedIOTInfo(Connection),
				   QString::number(BlockSize),name,owner);
  QString ret=createTableText(result,schema,owner,name);
  ret+=createComments(schema,owner,name);
  otl_stream inf(1,
		 SQLIndexNames(Connection),
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
  return ret;
}

static toSQL SQLIOTInfo("toExtract:IOTInfo",
			"SELECT
        -- Table Properties
        DECODE(
                b.monitoring
               ,'NO','NOMONITORING'
               ,     'MONITORING'
              )
      , 'N/A'                         AS table_name
        -- Parallel Clause
      , LTRIM(a.degree)
      , LTRIM(a.instances)
        -- Physical Properties
      , 'INDEX'                       AS organization
        -- Segment Attributes
      , 'N/A'                         AS cache
      , 'N/A'                         AS pct_used
      , a.pct_free
      , DECODE(
                a.ini_trans
               ,0,1
               ,null,1
               ,a.ini_trans
              )                       AS ini_trans
      , DECODE(
                a.max_trans
               ,0,255
               ,null,255
               ,a.max_trans
              )                       AS max_trans
        -- Storage Clause
      , a.initial_extent
      , a.next_extent
      , a.min_extents
      , DECODE(
                a.max_extents
               ,2147483645,'unlimited'
               ,a.max_extents
              )                       AS max_extents
      , a.pct_increase
      , NVL(a.freelists,1)
      , NVL(a.freelist_groups,1)
      , LOWER(a.buffer_pool)          AS buffer_pool
      , DECODE(
                b.logging
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(a.tablespace_name)      AS tablespace_name
      , DECODE(
                b.blocks
                ,null,GREATEST(a.initial_extent,a.next_extent) 
                      / (b.blocks * 1024)
                ,'0' ,GREATEST(a.initial_extent,a.next_extent)
                      / (b.blocks * 1024)
                ,b.blocks
              )                       AS blocks
 FROM
        all_indexes a,
        all_all_tables b
 WHERE  a.table_name  = :nam<char[100]>
   AND  b.owner = a.owner
   AND  b.table_name = a.table_name
   AND  a.owner = :own<char[100]>",
			"Get storage information about a IOT storage, "
			"same binds and columns");



QString toExtract::createIOT(const QString &schema,const QString &owner,
			     const QString &name)
{
  list<QString> storage=toReadQuery(Connection,SQLIOTInfo(Connection),name,owner);

  QString ret=createTableText(storage,schema,owner,name);
  ret+=";\n\n";
  ret+=createComments(schema,owner,name);
  return ret;
}

static toSQL SQLPartitionTableInfo("toExtract:PartitionTableInfo",
				   "SELECT
        -- Table Properties
        DECODE(
                t.monitoring
               ,'NO','NOMONITORING'
               ,     'MONITORING'
              )                       AS monitoring
      , t.table_name
        -- Parallel Clause
      , LTRIM(t.degree)               AS degree
      , LTRIM(t.instances)            AS instances
        -- Physical Properties
      , DECODE(
                t.iot_type
               ,'IOT','INDEX'
               ,      'HEAP'
              )                       AS organization
        -- Segment Attributes
      , DECODE(
                LTRIM(t.cache)
               ,'Y','CACHE'
               ,    'NOCACHE'
              )                       AS cache
      , p.def_pct_used
      , p.def_pct_free                AS pct_free
      , p.def_ini_trans               AS ini_trans
      , p.def_max_trans               AS max_trans
        -- Storage Clause
      ,DECODE(
               p.def_initial_extent
              ,'DEFAULT',s.initial_extent
              ,p.def_initial_extent * :bs<char[100]> * 1024
             )                        AS initial_extent
      ,DECODE(
               p.def_next_extent
              ,'DEFAULT',s.next_extent
              ,p.def_next_extent * :bs<char[100]> * 1024
             )                        AS next_extent
      , DECODE(
                p.def_min_extents
               ,'DEFAULT',s.min_extents
               ,p.def_min_extents
              )                       AS min_extents
      , DECODE(
                p.def_max_extents
               ,'DEFAULT',DECODE(
                                  s.max_extents
                                 ,2147483645,'unlimited'
                                 ,s.max_extents
                                )
               ,2147483645,'unlimited'
               ,p.def_max_extents
              )                       AS max_extents
      , DECODE(
                p.def_pct_increase
               ,'DEFAULT',s.pct_increase
               ,p.def_pct_increase
              )                       AS pct_increase
      , DECODE(
                p.def_freelists
               ,0,1
               ,NVL(p.def_freelists,1)
              )                       AS freelists
      , DECODE(
                p.def_freelist_groups
               ,0,1
               ,NVL(p.def_freelist_groups,1)
              )                       AS freelist_groups
      , LOWER(p.def_buffer_pool)      AS buffer_pool
      , DECODE(
                p.def_logging 
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(p.def_tablespace_name)  AS tablespace_name
      , t.blocks - NVL(t.empty_blocks,0)
 FROM
        all_all_tables   t
      , all_part_tables  p
      , dba_tablespaces  s
 WHERE
            t.table_name      = :nam<char[100]>
        AND p.table_name      = t.table_name
        AND s.tablespace_name = p.def_tablespace_name
        AND t.owner           = :own<char[100]>
        AND p.owner           = t.owner",
				   "Get storage information about a partitioned table, "
				   "same binds and columns",
				   "8.1");

static toSQL SQLPartitionTableInfo8("toExtract:PartitionTableInfo",
				    "SELECT
        -- Table Properties
        'N/A'                         AS monitoring
      , t.table_name
        -- Parallel Clause
      , LTRIM(t.degree)               AS degree
      , LTRIM(t.instances)            AS instances
        -- Physical Properties
      , DECODE(
                t.iot_type
               ,'IOT','INDEX'
               ,      'HEAP'
              )                       AS organization
        -- Segment Attributes
      , DECODE(
                LTRIM(t.cache)
               ,'Y','CACHE'
               ,    'NOCACHE'
              )                       AS cache
      , p.def_pct_used
      , p.def_pct_free                AS pct_free
      , p.def_ini_trans               AS ini_trans
      , p.def_max_trans               AS max_trans
        -- Storage Clause
      ,DECODE(
               p.def_initial_extent
              ,'DEFAULT',s.initial_extent
              ,p.def_initial_extent * :bs<char[100]> * 1024
             )                        AS initial_extent
      ,DECODE(
               p.def_next_extent
              ,'DEFAULT',s.next_extent
              ,p.def_next_extent * :bs<char[100]> * 1024
             )                        AS next_extent
      , DECODE(
                p.def_min_extents
               ,'DEFAULT',s.min_extents
               ,p.def_min_extents
              )                       AS min_extents
      , DECODE(
                p.def_max_extents
               ,'DEFAULT',DECODE(
                                  s.max_extents
                                 ,2147483645,'unlimited'
                                 ,s.max_extents
                                )
               ,2147483645,'unlimited'
               ,p.def_max_extents
              )                       AS max_extents
      , DECODE(
                p.def_pct_increase
               ,'DEFAULT',s.pct_increase
               ,p.def_pct_increase
              )                       AS pct_increase
      , DECODE(
                p.def_freelists
               ,0,1
               ,NVL(p.def_freelists,1)
              )                       AS freelists
      , DECODE(
                p.def_freelist_groups
               ,0,1
               ,NVL(p.def_freelist_groups,1)
              )                       AS freelist_groups
      , 'N/A'                         AS buffer_pool
      , DECODE(
                p.def_logging 
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(p.def_tablespace_name)  AS tablespace_name
      , t.blocks - NVL(t.empty_blocks,0)
 FROM
        all_all_tables   t
      , all_part_tables  p
      , dba_tablespaces  s
 WHERE
            t.table_name      = :nam<char[100]>
        AND p.table_name      = t.table_name
        AND s.tablespace_name = p.def_tablespace_name
        AND t.owner           = :own<char[100]>
        AND p.owner           = t.owner",
				    QString::null,
				    "8.0");

static toSQL SQLPartitionType("toExtract:PartitionType",
			      "SELECT
        partitioning_type
      , partition_count
      , subpartitioning_type
      , def_subpartition_count
 FROM
        all_part_tables
 WHERE
            table_name = :nam<char[100]>
        AND owner = :own<char[100]>",
			      "Get partition type, must have same binds and columns",
			      "8.1");

static toSQL SQLPartitionType8("toExtract:PartitionType",
			       "SELECT
        partitioning_type
      , partition_count
      , 'N/A'                        AS subpartitioning_type
      , 'N/A'                        AS def_subpartition_count
 FROM
        all_part_tables
 WHERE
            table_name = :nam<char[100]>
        AND owner = :own<char[100]>",
			       QString::null,
			       "8.0");

static toSQL SQLPartitionSegment("toExtract:PartitionSegment",
				 "SELECT
        partition_name
      , high_value
      , 'N/A'
      , pct_used
      , pct_free
      , ini_trans
      , max_trans
        -- Storage Clause
      , initial_extent
      , next_extent
      , min_extent
      , DECODE(
                max_extent
               ,2147483645,'unlimited'
               ,           max_extent
              )                       AS max_extents
      , pct_increase
      , NVL(freelists,1)
      , NVL(freelist_groups,1)
      , LOWER(buffer_pool)
      , DECODE(
                logging 
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(tablespace_name)
      , blocks - NVL(empty_blocks,0)
 FROM
        all_tab_partitions
 WHERE  table_name = :nam<char[100]>
   AND  owner = :nam<char[100]>
 ORDER BY partition_name",
				 "Information about segment storage for partitioned tables, "
				 "must have same binds and columns",
				 "8.1");

static toSQL SQLPartitionSegment8("toExtract:PartitionSegment",
				 "SELECT
        partition_name
      , high_value
      , 'N/A'
      , pct_used
      , pct_free
      , ini_trans
      , max_trans
        -- Storage Clause
      , initial_extent
      , next_extent
      , min_extent
      , DECODE(
                max_extent
               ,2147483645,'unlimited'
               ,           max_extent
              )                       AS max_extents
      , pct_increase
      , NVL(freelists,1)
      , NVL(freelist_groups,1)
      , 'N/A'                         AS buffer_pool
      , DECODE(
                logging 
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , LOWER(tablespace_name)
      , blocks - NVL(empty_blocks,0)
 FROM
        all_tab_partitions
 WHERE  table_name = :nam<char[100]>
   AND  owner = :nam<char[100]>
 ORDER BY partition_name",
				  QString::null,
				  "8.0");

static toSQL SQLSubPartitionName("toExtract:SubPartitionName",
				 "SELECT subpartition_name,
       tablespace_name
  FROM all_tab_subpartitions
 WHERE table_name = :nam<char[100]>
   AND partition_name = :prt<char[100]>
   AND table_owner = :own<char[100]>
 ORDER BY subpartition_name",
				 "Get information about sub partitions, "
				 "must have same columns and binds");

static toSQL SQLPartitionName("toExtract:PartitionName",
				 "SELECT partition_name,
       tablespace_name
  FROM all_tab_partitions
 WHERE table_name = :nam<char[100]>
   AND table_owner = :own<char[100]>
 ORDER BY partition_name",
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
	  ret+=QString("SUBPARTITION %1 TABLESPACE %2").
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
      ret+=QString("PARTITION %1 TABLESPACE %2").
	arg(toShift(hash).lower()).
	arg(toShift(hash).lower());
    }
  }

  ret+=";\n\n";
  ret+=createComments(schema,owner,name);
  return ret;
}

QString toExtract::createPackage(const QString &schema,const QString &owner,const QString &name)
{
  return displaySource(schema,owner,name,"PACKAGE");
}

QString toExtract::createPackageBody(const QString &schema,const QString &owner,const QString &name)
{
  return displaySource(schema,owner,name,"PACKAGE BODY");
}

QString toExtract::createProcedure(const QString &schema,const QString &owner,const QString &name)
{
  return displaySource(schema,owner,name,"PROCEDURE");
}

QString toExtract::createSnapshot(const QString &schema,const QString &owner,const QString &name)
{
  return createMView(schema,owner,name,"SNAPSHOT");
}

QString toExtract::createSnapshotLog(const QString &schema,const QString &owner,const QString &name)
{
  return createMViewLog(schema,owner,name,"SNAPSHOT");
}

static toSQL SQLProfileInfo("toExtract:ProfileInfo",
			    "SELECT
        RPAD(resource_name,27)
      , DECODE(
                RESOURCE_NAME
               ,'PASSWORD_VERIFY_FUNCTION',DECODE(
                                                   limit
                                                  ,'UNLIMITED','null'
                                                  ,LOWER(limit)
                                                 )
               ,                           LOWER(limit)
              )
 FROM
        dba_profiles
 WHERE
        profile = :nam<char[100]>
 ORDER
    BY
        resource_type
      , resource_name",
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

static toSQL SQLRoleInfo("toExtract:RoleInfo",
			 "SELECT
        DECODE(
                r.password_required
               ,'YES', DECODE(
                               u.password
                              ,'EXTERNAL','IDENTIFIED EXTERNALLY'
                              ,'IDENTIFIED BY VALUES ''' 
                                || u.password || ''''
                             )
               ,'NOT IDENTIFIED'
              )                         AS password
 FROM
        dba_roles   r
      , sys.user$  u
 WHERE
            r.role = :rol<char[100]>
        AND u.name = r.role",
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
  ret+=grantedPrivs(name);
  return ret;
}

static toSQL SQLRolePrivs("toExtract:RolePrivs",
			  "SELECT
        granted_role
      , DECODE(
                admin_option
               ,'YES','WITH ADMIN OPTION'
               ,null
              )                         AS admin_option
  FROM  dba_role_privs
 WHERE  grantee = :nam<char[100]>
 ORDER  BY granted_role",
			  "Get roles granted, must have same columns and binds");

static toSQL SQLSystemPrivs("toExtract:SystemPrivs",
			    "SELECT
        privilege
      , DECODE(
                admin_option
               ,'YES','WITH ADMIN OPTION'
               ,null
              )                         AS admin_option
  FROM  dba_sys_privs
 WHERE  grantee = :nam<char[100]>
 ORDER  BY privilege",
			    "Get system priveleges granted, must have same columns and binds");

static toSQL SQLObjectPrivs("toExtract:ObjectPrivs",
			    "SELECT  privilege
      , owner
      , table_name
      , DECODE(
                grantable
               ,'YES','WITH GRANT OPTION'
               ,null
              )                         AS grantable
  FROM  dba_tab_privs
 WHERE  grantee = :nam<char[100]>
 ORDER  BY table_name,privilege",
			    "Get object priveleges granted, must have same columns and binds");

QString toExtract::grantedPrivs(const QString &name)
{
  list<QString> result=toReadQuery(Connection,SQLRolePrivs(Connection),name);
  QString ret;
  while(result.size()>0) {
    QString sql=QString("GRANT %1 TO %2 %3").
      arg(toShift(result).lower()).
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

  result=toReadQuery(Connection,SQLSystemPrivs(Connection),name);
  while(result.size()>0) {
    QString sql=QString("GRANT %1 TO %2 %3").
      arg(toShift(result).lower()).
      arg(name.lower()).
      arg(toShift(result));
    if (Prompt) {
      ret+="PROMPT ";
      ret+=sql;
      ret+="\n\n";
    }
    ret+="\n\n";
    ret+=sql;
    ret+=";\n\n";
  }

  result=toReadQuery(Connection,SQLObjectPrivs(Connection),name);
  while(result.size()>0) {
    QString priv=toShift(result);
    QString schema=setSchema(toShift(result));
    QString sql=QString("GRANT %1 ON %2%3 TO %4 %5").
      arg(priv.lower()).
      arg(schema).
      arg(toShift(result).lower()).
      arg(name).
      arg(toShift(result));
    if (Prompt) {
      ret+="PROMPT ";
      ret+=sql;
      ret+="\n\n";
    }
    ret+="\n\n";
    ret+=sql;
    ret+=";\n\n";
  }
  return ret;
}

static toSQL SQLRollbackSegment("toExtract:RollbackSegment",
				"SELECT  DECODE(
                r.owner
               ,'PUBLIC',' PUBLIC '
               ,         ' '
              )                                  AS is_public
      , r.tablespace_name
      , NVL(r.initial_extent,t.initial_extent)   AS initial_extent
      , NVL(r.next_extent,t.next_extent)         AS next_extent
      , r.min_extents
      , DECODE(
                r.max_extents
               ,2147483645,'unlimited'
               ,           r.max_extents
              )                                  AS max_extents
  FROM  dba_rollback_segs    r
      , all_tablespaces  t
 WHERE
            r.segment_name    = :nam<char[100]>
        AND t.tablespace_name = r.tablespace_name",
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

static toSQL SQLSequenceInfo("toExtract:SequenceInfo",
			     "SELECT  'START WITH       '
         || LTRIM(TO_CHAR(last_number + cache_size,'fm999999999'))
                                         AS start_with
      , 'INCREMENT BY     '
         || LTRIM(TO_CHAR(increment_by,'fm999999999')) AS imcrement_by
      , DECODE(
                min_value
               ,0,'NOMINVALUE'
               ,'MINVALUE         ' || TO_CHAR(min_value)
              )                          AS min_value
      , DECODE(
                TO_CHAR(max_value,'fm999999999999999999999999999')
               ,'999999999999999999999999999','NOMAXVALUE'
               ,'MAXVALUE         ' || TO_CHAR(max_value)
              )                          AS max_value
      , DECODE(
                cache_size
               ,0,'NOCACHE'
               ,'CACHE            ' || TO_CHAR(cache_size)
              )                          AS cache_size
      , DECODE(
                cycle_flag
               ,'Y','CYCLE'
               ,'N', 'NOCYCLE'
              )                          AS cycle_flag
      , DECODE(
                order_flag
               ,'Y','ORDER'
               ,'N', 'NOORDER'
              )                          AS order_flag
 FROM
        all_sequences
 WHERE
            sequence_name  = :nam<char[100]>
        AND sequence_owner = :own<char[100]>",
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

static toSQL SQLSynonymInfo("toExtract:SynonymInfo",
			    "SELECT  table_owner
      , table_name
      , NVL(db_link,'NULL')
 FROM
        all_synonyms
 WHERE
            synonym_name = :nam<char[100]>
        AND owner = :own<char[100]>",
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
  QString tableSchema=setSchema(tableOwner);

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

static toSQL SQLTableConstraints("toExtract:TableConstraints",
				 "SELECT
        constraint_name
 FROM
        all_constraints cn
 WHERE      table_name       = :nam<char[100]>
        AND owner            = :own<char[100]>
        AND constraint_type IN('P','U','R','C')
        AND generated        != 'GENERATED NAME'
 ORDER
    BY
       DECODE(
               constraint_type
              ,'P',1
              ,'U',2
              ,'R',3
              ,'C',4
             )
     , constraint_name",
				 "Get constraints tied to a table, same binds and columns");

static toSQL SQLTableTriggers("toExtract:TableTriggers",
			      "SELECT  trigger_name
  FROM  all_triggers
 WHERE      table_name = :nam<char[100]>
        AND owner      = :own<char[100]>
 ORDER  BY  trigger_name",
			      "Get triggers for a table, must have same columns and binds");

QString toExtract::createTableFamily(const QString &schema,const QString &owner,const QString &name)
{
  list<QString> indexes=toReadQuery(Connection,SQLIndexNames(Connection),name,owner);
  QString ret=createTable(schema,owner,name);
  while(indexes.size()>0)
    ret+=createIndex(schema,owner,toShift(indexes));
  list<QString> constraints=toReadQuery(Connection,SQLTableConstraints(Connection),name,owner);
  while(constraints.size()>0)
    ret+=createConstraint(schema,owner,toShift(constraints));
  list<QString> triggers=toReadQuery(Connection,SQLTableTriggers(Connection),name,owner);
  while(triggers.size()>0)
    ret+=createTrigger(schema,owner,toShift(triggers));
  return ret;
}

static toSQL SQLTriggerInfo("toExtract:TriggerInfo",
			    "SELECT  trigger_type
      , RTRIM(triggering_event)
      , table_owner
      , table_name
      , base_object_type
      , referencing_names
      , description
      , DECODE(
                when_clause
               ,null,null
               ,'WHEN (' || when_clause || ')' || CHR(10)
              )
      , trigger_body
 FROM
        all_triggers
 WHERE
            trigger_name = :nam<char[100]>
        AND owner        = :own<char[100]>",
			    "Get information about triggers, must have same binds and columns",
			    "8.1");

static toSQL SQLTriggerInfo8("toExtract:TriggerInfo",
			     "SELECT
        trigger_type
      , RTRIM(triggering_event)
      , table_owner
      , table_name
        -- Only table triggers before 8i
      , 'TABLE'                           AS base_object_type
      , referencing_names
      , description
      , DECODE(
                when_clause
               ,null,null
               ,'WHEN (' || when_clause || ')' || CHR(10)
              )
      , trigger_body
 FROM
        all_triggers
 WHERE
            trigger_name = :nam<char[100]>
        AND owner        = :own<char[100]>",
			     QString::null,
			     "8.0");

QString toExtract::createTrigger(const QString &schema,const QString &owner,const QString &name)
{
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
  QString schema2=setSchema(tableOwner);
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

static toSQL SQLTablespaceInfo("toExtract:TablespaceInfo",
			       "SELECT  initial_extent
      , next_extent
      , min_extents
      , DECODE(
                max_extents
               ,2147483645,'unlimited'
               ,null,DECODE(
                              :bs<char[100]>
                            , 1,  57
                            , 2, 121
                            , 4, 249
                            , 8, 505
                            ,16,1017
                            ,32,2041
                            ,'???'
                           )
               ,max_extents
              )                       AS max_extents
      , pct_increase
      , min_extlen
      , contents
      , logging
      , extent_management
      , allocation_type
 FROM
        dba_tablespaces
 WHERE
        tablespace_name = :nam<char[100]>",
			       "Get tablespace information, must have same columns and binds",
			       "8.1");

static toSQL SQLTablespaceInfo8("toExtract:TablespaceInfo",
				"SELECT  initial_extent
      , next_extent
      , min_extents
      , DECODE(
                max_extents
               ,2147483645,'unlimited'
               ,null,DECODE(
                             :bs<char[100]>
                            , 1,  57
                            , 2, 121
                            , 4, 249
                            , 8, 505
                            ,16,1017
                            ,32,2041
                            ,'???'
                           )
               ,max_extents
              )                       AS max_extents
      , pct_increase
      , min_extlen
      , contents
      , DECODE(
                logging 
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , 'N/A'                         AS extent_management
      , 'N/A'                         AS allocation_type
 FROM
        dba_tablespaces
 WHERE
        tablespace_name = :nam<char[100]>",
				QString::null,
				"8.0");

static toSQL SQLTablespaceInfo7("toExtract:TablespaceInfo",
				"SELECT  initial_extent
      , next_extent
      , min_extents
      , DECODE(
                max_extents
               ,2147483645,'unlimited'
               ,null,DECODE(
                             $block_size
                            , 1,  57
                            , 2, 121
                            , 4, 249
                            , 8, 505
                            ,16,1017
                            ,32,2041
                            ,'???'
                           )
               ,max_extents
              )                       AS max_extents
      , pct_increase
      , min_extlen
      , contents
      , DECODE(
                logging 
               ,'NO','NOLOGGING'
               ,     'LOGGING'
              )                       AS logging
      , 'N/A'                         AS extent_management
      , 'N/A'                         AS allocation_type
 FROM
        dba_tablespaces
 WHERE
        tablespace_name = :nam<char[100]>",
				QString::null,
				"7.0");

static toSQL SQLDatafileInfo("toExtract:DatafileInfo",
			     "SELECT
        file_name
      , bytes
      , autoextensible
      , DECODE(
                SIGN(2147483645 - maxbytes)
               ,-1,'unlimited'
               ,maxbytes
              )                               AS maxbytes
      , increment_by * :bs<char[100]> * 1024     AS increment_by
 FROM
        (select * from dba_temp_files union select * from dba_data_files)
 WHERE
        tablespace_name = :nam<char[100]>
 ORDER  BY file_name",
			     "Get information about datafiles in a tablespace, "
			     "same binds and columns",
			     "8.0");

static toSQL SQLDatafileInfo7("toExtract:DatafileInfo",
			      "SELECT
        file_name
      , bytes
      , 'N/A'                                 AS autoextensible
      , 'N/A'                                 AS maxbytes
      , DECODE(:bs<char[100]>,
               NULL,'N/A','N/A')              AS increment_by
 FROM
        dba_data_files
 WHERE
        tablespace_name = :nam<char[100]>
 ORDER  BY file_name",
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
    if (Connection.version()>="8.0") {
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
  ret+=";\n\n";
  return ret;
}

QString toExtract::createType(const QString &schema,const QString &owner,const QString &name)
{
  return displaySource(schema,owner,name,"TYPE");
}

static toSQL SQLUserInfo("toExtract:UserInfo",
			 "SELECT
        DECODE(
                password
               ,'EXTERNAL','EXTERNALLY'
               ,'BY VALUES ''' || password || ''''
              )                         AS password
      , profile
      , default_tablespace
      , temporary_tablespace
 FROM
        dba_users
 WHERE
        username = :nam<char[100]>",
			 "Information about authentication for a user, "
			 "same binds and columns");

static toSQL SQLUserQuotas("toExtract:UserQuotas",
			   "SELECT
        DECODE(
                max_bytes
               ,-1,'unlimited'
               ,TO_CHAR(max_bytes,'99999999')
              )                         AS max_bytes
      , tablespace_name
 FROM
        dba_ts_quotas
 WHERE
        username = :nam<char[100]>
 ORDER  BY tablespace_name",
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
  if (Prompt)
    ret=QString("PROMPT CREATE USER %1\n\n").arg(name.lower());
  ret+=QString("CREATE USER %1 IDENTIFIED %2\n"
	       "   PROFILE              %3\n"
	       "   DEFAULT TABLESPACE   %4\n"
	       "   TEMPORARY TABLESPACE %5\n").
    arg(name.lower()).
    arg(password).
    arg(profile.lower()).
    arg(defaultTablespace.lower()).
    arg(temporaryTablespace.lower());

  list<QString> quota=toReadQuery(Connection,SQLUserQuotas(Connection),name);
  while(quota.size()>0) {
    ret+=QString("   QUOTA %1 ON %2\n").
      arg(toShift(quota).lower()).
      arg(toShift(quota));
  }
  ret+=";\n\n";
  ret+=grantedPrivs(name);
  return ret;
}

static toSQL SQLViewSource("toExtract:ViewSource",
			   "SELECT  text
 FROM
        all_views
 WHERE
            view_name = :nam<char[100]>
        AND owner = :own<char[100]>",
			   "Get the source of the view, must have same binds and columns");

QString toExtract::createView(const QString &schema,const QString &owner,const QString &name)
{
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

QString toExtract::compile(const QString &type,list<QString> &objects)
{
  QString ret=generateHeading("COMPILE",type,objects);

  QString utype=type.upper();
  for (list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
    QString owner;
    QString name;
    parseObject(*i,owner,name);
    QString schema=setSchema(owner);

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
  }
  return ret;
}

QString toExtract::create(const QString &type,list<QString> &objects)
{
  QString ret=generateHeading("CREATE",type,objects);

  QString utype=type.upper();
  for (list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
    QString owner;
    QString name;
    parseObject(*i,owner,name);
    QString schema=setSchema(owner);

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
    else if (utype=="TABLESPACE")
      ret+=createTablespace(schema,owner,name);
    else if (utype=="TRIGGER")
      ret+=createTrigger(schema,owner,name);
    else if (utype=="TYPE")
      ret+=createType(schema,owner,name);
    else if (utype=="USER")
      ret+=createUser(schema,owner,name);
    else if (utype=="VIEW")
      ret+=createView(schema,owner,name);
    else {
      QString str="Invalid type ";
      str+=type;
      str+=" to create";
      throw str;
    }
  }
  return ret;
}

QString toExtract::drop(const QString &type,list<QString> &objects)
{
  QString ret=generateHeading("CREATE",type,objects);

  QString utype=type.upper();
  for (list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
    QString owner;
    QString name;
    parseObject(*i,owner,name);
    QString schema=setSchema(owner);

    if (utype=="CONSTRAINT")
      ret+=dropConstraint(schema,owner,type,name);
    else if (utype=="DATABASE LINK")
      ret+=dropDatabaseLink(schema,owner,type,name);
    else if (utype=="DIMENSION")
      ret+=dropSchemaObject(schema,owner,type,name);
    else if (utype=="DIRECTORY")
      ret+=dropObject(schema,owner,type,name);
    else if (utype=="FUNCTION")
      ret+=dropSchemaObject(schema,owner,type,name);
    else if (utype=="INDEX")
      ret+=dropSchemaObject(schema,owner,type,name);
    else if (utype=="MATERIALIZED VIEW")
      ret+=dropSchemaObject(schema,owner,type,name);
    else if (utype=="MATERIALIZED VIEW LOG")
      ret+=dropMViewLog(schema,owner,type,name);
    else if (utype=="PACKAGE")
      ret+=dropSchemaObject(schema,owner,type,name);
    else if (utype=="PROCEDURE")
      ret+=dropSchemaObject(schema,owner,type,name);
    else if (utype=="PROFILE")
      ret+=dropProfile(schema,owner,type,name);
    else if (utype=="ROLE")
      ret+=dropObject(schema,owner,type,name);
    else if (utype=="ROLLBACK SEGMENT")
      ret+=dropObject(schema,owner,type,name);
    else if (utype=="SEQUENCE")
      ret+=dropSchemaObject(schema,owner,type,name);
    else if (utype=="SNAPSHOT")
      ret+=dropSchemaObject(schema,owner,type,name);
    else if (utype=="SNAPSHOT LOG")
      ret+=dropMViewLog(schema,owner,type,name);
    else if (utype=="SYNONYM")
      ret+=dropSynonym(schema,owner,type,name);
    else if (utype=="TABLE")
      ret+=dropTable(schema,owner,type,name);
    else if (utype=="TABLESPACE")
      ret+=dropTablespace(schema,owner,type,name);
    else if (utype=="TRIGGER")
      ret+=dropSchemaObject(schema,owner,type,name);
    else if (utype=="TYPE")
      ret+=dropSchemaObject(schema,owner,type,name);
    else if (utype=="USER")
      ret+=dropUser(schema,owner,type,name);
    else if (utype=="VIEW")
      ret+=dropSchemaObject(schema,owner,type,name);
    else {
      QString str="Invalid type ";
      str+=type;
      str+=" to drop";
      throw str;
    }
  }
  return ret;
}

