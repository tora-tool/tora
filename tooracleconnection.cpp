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

#ifdef WIN32
#  include "windows/cregistry.h"
#endif

#define OTL_STL
#define OTL_STREAM_POOLING_ON

#include "otlv4.h"

#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tosql.h"
#include "totool.h"

#include <stdio.h>

#include <qcheckbox.h>
#include <qfile.h>
#include <qinputdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qvalidator.h>

#include "tooraclesettingui.h"
#include "tooraclesettingui.moc"

#define CONF_OPEN_CURSORS	"OpenCursors"
#define DEFAULT_OPEN_CURSORS	"40"  // Defined to be able to update tuning view
#define CONF_MAX_LONG		"MaxLong"

// Must be larger than max long size in otl.

#ifndef DEFAULT_MAX_LONG
#define DEFAULT_MAX_LONG 30000 
#endif

static int toMaxLong=DEFAULT_MAX_LONG;

static toSQL SQLComment("toOracleConnection:Comments",
			"SELECT Column_name,Comments FROM sys.All_Col_Comments\n"
			" WHERE Owner = :f1<char[100]>\n"
			"   AND Table_Name = :f2<char[100]>",
			"Display column comments");

static toSQL SQLMembers("toOracleConnection:Members",
			"SELECT object_name,overload,argument_name,data_type\n"
			"  FROM sys.All_Arguments\n"
			" WHERE Owner = :f1<char[100]>\n"
			"   AND Package_Name = :f2<char[100]>\n"
			" ORDER BY object_name,overload,DECODE(argument_name,NULL,9999,sequence)",
			"Get list of package members");

static toSQL SQLListObjects("toOracleConnection:ListObjects",
			    "select a.owner,a.object_name,a.object_type,b.comments\n"
			    "  from sys.all_objects a,\n"
			    "       sys.all_tab_comments b\n"
			    " where a.owner = b.owner(+) and a.object_name = b.table_name(+)\n"
			    "   and a.object_type = b.table_type(+) and a.object_type != 'SYNONYM'",
			    "List the objects to cache for a connection, should have same "
			    "columns and binds");

static toSQL SQLListSynonyms("toOracleConnection:ListSynonyms",
			     "select synonym_name,table_owner,table_name\n"
			     "  from sys.all_synonyms\n"
			     " where owner = :usr<char[101]> or owner = 'PUBLIC'\n"
			     " order by table_owner,table_name",
			     "List the synonyms available to a user, should have same columns and binds");

static void ThrowException(const otl_exception &exc)
{
  if (exc.code==24344)
    throw toConnection::exception(QString::fromLatin1("ORA-24344 success with compilation error"));
  else {
    toConnection::exception ret(QString::fromUtf8((const char *)exc.msg));

    if (ret.isEmpty()) {
      if (exc.code!=0)
	ret=QString::fromLatin1("ORA-")+QString::number(exc.code)+QString::fromLatin1(" missing error description");
      else if (ret.isEmpty())
	ret=QString::fromLatin1("Missing error description, could occur if you have several ORACLE_HOME and the binary directory\n"
				"of the one which is not active is before the active ORACLE_HOME in your path");
    }

    if (exc.stm_text&&strlen(exc.stm_text)) {
      ret+=QString::fromLatin1("\n");
      QString sql=QString::fromUtf8((const char *)exc.stm_text);
      if (exc.errorofs>=0) {
	QString t=QString::fromUtf8((const char *)exc.stm_text,exc.errorofs);
	ret.setOffset(t.length());
	sql.insert(t.length(),QString::fromLatin1("<ERROR>"));
      }
      ret+=sql;
    }
    throw ret;
  }
}

class toOracleProvider : public toConnectionProvider {
public:
  class connectionDeleter : public toTask {
    otl_connect *Connection;
  public:
    connectionDeleter(otl_connect *connect)
      : Connection(connect)
    { }
    virtual void run(void)
    {
      delete Connection;
    }
  };
  class oracleSub : public toConnectionSub {
  public:
    toSemaphore Lock;
    otl_connect *Connection;
    oracleSub(otl_connect *conn)
      : Lock(1)
    { Connection=conn; }
    ~oracleSub()
    { toThread *thread=new toThread(new connectionDeleter(Connection)); thread->start(); }
    virtual void cancel(void)
    { Connection->cancel(); }
    virtual void throwExtendedException(toConnection &conn,const otl_exception &exc)
    {
      if (conn.version()<"8.0"&&exc.errorofs==0) {
	// Serious OCI voodoo to get the Parse error location on Oracle7 servers

	Lda_Def lda;
	Cda_Def cda;
	if (OCISvcCtxToLda(Connection->connect_struct.svchp,
			   Connection->connect_struct.errhp,
			   &lda)==OCI_SUCCESS) {
	  if (oopen(&cda,
		    &lda,
		    (OraText *)0,-1,
		    -1,
		    (OraText *)0,-1)==OCI_SUCCESS) {
	    cda.peo=0;
	    oparse(&cda,(OraText *)exc.stm_text,-1,FALSE,1);
	    ((otl_exception &)exc).errorofs=cda.peo;
	    oclose(&cda);
	  }
	  OCILdaToSvcCtx(&Connection->connect_struct.svchp,
			 Connection->connect_struct.errhp,
			 &lda);
	}
      }
      ThrowException(exc);
    }
  };

  class oracleQuery : public toQuery::queryImpl {
    bool Cancel;
    bool Running;
    bool SaveInPool;
    otl_stream *Query;
  public:
    oracleQuery(toQuery *query,oracleSub *)
      : toQuery::queryImpl(query)
    {
      Running=Cancel=false;
      SaveInPool=false;
      Query=NULL;
    }
    virtual ~oracleQuery()
    {
#ifdef OTL_STREAM_POOLING_ON
      if (!SaveInPool&&Query)
	Query->close(false);
#endif
      delete Query;
    }
    virtual void execute(void);

    virtual toQValue readValue(void)
    {
      char *buffer=NULL;
      otl_var_desc *dsc=Query->describe_next_out_var();
      if (!dsc)
	throw QString::fromLatin1("Couldn't get description of next column to read");

      oracleSub *conn=dynamic_cast<oracleSub *>(query()->connectionSub());
      if (!conn)
	throw QString::fromLatin1("Internal error, not oracle sub connection");
      conn->Lock.down();
      if (Cancel)
	throw QString::fromLatin1("Cancelled while waiting to read value");
      Running=true;
      SaveInPool=true;
      try {
	toQValue null;
	switch (dsc->ftype) {
	case otl_var_double:
	case otl_var_float:
	  {
	    double d;
	    (*Query)>>d;
	    Running=false;
	    conn->Lock.up();
	    if (Query->is_null())
	      return null;
	    return toQValue(d);
	  }
	  break;
	case otl_var_int:
	case otl_var_unsigned_int:
	case otl_var_short:
	case otl_var_long_int:
	  {
	    int i;
	    (*Query)>>i;
	    Running=false;
	    conn->Lock.up();
	    if (Query->is_null())
	      return null;
	    return toQValue(i);
	  }
	  break;
	case otl_var_varchar_long:
	case otl_var_raw_long:
	  {
	    int len=toMaxLong;
	    if (toMaxLong<0)
	      len=DEFAULT_MAX_LONG;
	    buffer=new char[len+1];
	    buffer[len]=0;
	    otl_long_string str(buffer,len);
	    (*Query)>>str;
	    Running=false;
	    conn->Lock.up();
	    if (!str.len())
	      return null;
	    QString buf(QString::fromUtf8(buffer));
	    delete[] buffer;
	    return buf;
	  }
	case otl_var_clob:
	case otl_var_blob:
	  {
	    otl_lob_stream lob;
	    (*Query)>>lob;
	    if (lob.len()==0) {
	      Running=false;
	      conn->Lock.up();
	      return null;
	    }
	    int len=lob.len();
	    if (toMaxLong>=0&&len>toMaxLong)
	      len=toMaxLong;
	    if (dsc->ftype==otl_var_clob)
	      len*=5;
	    else
	      len*=2;
	    buffer=new char[len+1];
	    buffer[0]=0;
	    otl_long_string data(buffer,len);
	    lob>>data;
	    if (!lob.eof()) {
	      otl_long_string sink(10000);
	      while(!lob.eof())
		lob>>sink;
	      if (toThread::mainThread())
		toStatusMessage(QString::fromLatin1("Data exists past length of LOB"));
	      else
		printf("Data exists past length of LOB in thread\n");
	    }
	    buffer[data.len()]=0;
	    QString buf;
	    if (dsc->ftype==otl_var_clob)
	      buf=QString::fromUtf8(buffer);
	    else {
	      buf.fill('0',data.len()*2);
	      QString t;
	      for(int i=0;i<data.len();i++) {
		t.sprintf("%02x",((unsigned int)buffer[i])%0xff);
		buf.replace(i*2,2,t);
	      }
	    }
	    delete[] buffer;
	    Running=false;
	    conn->Lock.up();
	    return buf;
	  }
	  break;
	default:  // Try using char if all else fails
	  {
	    // The *5 is for raw columns or UTF expanded data, also dates and numbers
	    // are a bit tricky but if someone specifies a dateformat longer than 100 bytes he
	    // deserves everything he gets!
	    buffer=new char[max(dsc->elem_size*5+1,100)];
	    buffer[0]=0;
	    (*Query)>>buffer;
	    Running=false;
	    conn->Lock.up();
	    if (Query->is_null()) {
	      delete[] buffer;
	      return null;
	    }
	    QString buf(QString::fromUtf8(buffer));
	    delete[] buffer;
	    return buf;
	  }
	  break;
	}
      } catch (const otl_exception &exc) {
	Running=false;
	conn->Lock.up();
	delete[] buffer;
	conn->throwExtendedException(query()->connection(),exc);
      } catch (...) {
	Running=false;
	conn->Lock.up();
	delete[] buffer;
	throw;
      }
      // Never get here
      return QString::null;
    }
    virtual void cancel(void);
    virtual bool eof(void)
    {
      if (!Query)
	return true;
      return Query->eof();
    }
    virtual int rowsProcessed(void)
    {
      if (!Query)
	return 0;
      return Query->get_rpc();
    }
    virtual int columns(void)
    {
      int descriptionLen;
      Query->describe_select(descriptionLen);
      return descriptionLen;
    }
    virtual std::list<toQuery::queryDescribe> describe(void)
    {
      std::list<toQuery::queryDescribe> ret;
      int descriptionLen;
      otl_column_desc *description=Query->describe_select(descriptionLen);

      for (int i=0;i<descriptionLen;i++) {
	toQuery::queryDescribe desc;
	desc.AlignRight=false;
	desc.Name=QString::fromUtf8(description[i].name);

	switch(description[i].dbtype) {
	case 1:
	case 5:
	case 9:
	case 155:
	  desc.Datatype=QString::fromLatin1("VARCHAR2");
	  break;
	case 2:
	case 3:
	case 4:
	case 6:
	case 68:
	  desc.AlignRight=true;
	  desc.Datatype=QString::fromLatin1("NUMBER");
	  break;
	case 8:
	case 94:
	case 95:
	  desc.Datatype=QString::fromLatin1("LONG");
	  break;
	case 11:
	case 104:
	  desc.Datatype=QString::fromLatin1("ROWID");
	  break;
	case 12:
	case 156:
	  desc.AlignRight=true;
	  desc.Datatype=QString::fromLatin1("DATE");
	  break;
	case 15:
	case 23:
	case 24:
	  desc.Datatype=QString::fromLatin1("RAW");
	  break;
	case 96:
	case 97:
	  desc.Datatype=QString::fromLatin1("CHAR");
	  break;
	case 108:
	  desc.Datatype=QString::fromLatin1("NAMED DATA TYPE");
	  break;
	case 110:
	  desc.Datatype=QString::fromLatin1("REF");
	  break;
	case 112:
	  desc.Datatype=QString::fromLatin1("CLOB");
	  break;
	case 113:
	case 114:
	  desc.Datatype=QString::fromLatin1("BLOB");
	  break;
	default:
	  desc.Datatype=QString::fromLatin1("UNKNOWN");
          break;
	}

	if (desc.Datatype==QString::fromLatin1("NUMBER")) {
	  if (description[i].prec) {
	    desc.Datatype.append(QString::fromLatin1(" ("));
	    desc.Datatype.append(QString::number(description[i].prec));
	    if (description[i].scale!=0) {
	      desc.Datatype.append(QString::fromLatin1(","));
	      desc.Datatype.append(QString::number(description[i].scale));
	    }
	    desc.Datatype.append(QString::fromLatin1(")"));
	  }
	} else {
	  desc.Datatype.append(QString::fromLatin1(" ("));
	  desc.Datatype.append(QString::number(description[i].dbsize));
	  desc.Datatype.append(QString::fromLatin1(")"));
	}
	desc.Null=description[i].nullok;

	ret.insert(ret.end(),desc);
      }
      return ret;
    }
  };

  class oracleConnection : public toConnection::connectionImpl {
    QCString connectString(void)
    {
      QCString ret;
      ret=connection().user().utf8();
      ret+=QString::fromLatin1("/");
      ret+=connection().password().utf8();
      if (!connection().host().isEmpty()) {
	ret+=QString::fromLatin1("@");
	ret+=connection().database().utf8();
      }
      return ret;
    }
    oracleSub *oracleConv(toConnectionSub *sub)
    {
      oracleSub *conn=dynamic_cast<oracleSub *>(sub);
      if (!conn)
	throw QString::fromLatin1("Internal error, not oracle sub connection");
      return conn;
    }
  public:
    oracleConnection(toConnection *conn)
      : toConnection::connectionImpl(conn)
    { }

    /** Return a string representation to address an object.
     * @param name The name to be quoted.
     * @return String addressing table.
     */
    virtual QString quote(const QString &name)
    {
      bool ok=true;
      for(unsigned int i=0;i<name.length();i++) {
	if (name.at(i).upper()!=name.at(i)||!toIsIdent(name.at(i)))
	  ok=false;
      }
      if (ok)
	return name.lower();
      else
	return QString::fromLatin1("\"")+name+QString::fromLatin1("\"");
    }
    virtual QString unQuote(const QString &str)
    {
      if (str.at(0).latin1()=='\"'&&str.at(str.length()-1).latin1()=='\"')
	return str.left(str.length()-1).right(str.length()-2);
      return str.upper();
    }

    virtual std::list<toConnection::objectName> objectNames(void)
    {
      std::list<toConnection::objectName> ret;

      std::list<toQValue> par;
      toQuery objects(connection(),toQuery::Long,
		      SQLListObjects,par);
      toConnection::objectName cur;
      while(!objects.eof()) {
	cur.Owner=objects.readValueNull();
	cur.Name=objects.readValueNull();
	cur.Type=objects.readValueNull();
	cur.Comment=objects.readValueNull();
	ret.insert(ret.end(),cur);
      }

      return ret;
    }
    virtual std::map<QString,toConnection::objectName> synonymMap(std::list<toConnection::objectName> &objects)
    {
      std::map<QString,toConnection::objectName> ret;

      toConnection::objectName cur;
      cur.Type=QString::fromLatin1("A");
      std::list<toQValue> par;
      par.insert(par.end(),toQValue(connection().user().upper()));
      toQuery synonyms(connection(),toQuery::Long,
		       SQLListSynonyms,par);
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

      return ret;
    }
    virtual toQDescList columnDesc(const toConnection::objectName &table)
    {
      toBusy busy;
      if(table.Type==QString::fromLatin1("PACKAGE")) {
	toQDescList ret;
	try {
	  toQuery::queryDescribe desc;
	  desc.Datatype=("MEMBER");
	  desc.Null=false;
	  QString lastName;
	  QString lastOver;
	  toQuery member(connection(),SQLMembers,table.Owner,table.Name);
	  bool hasArgs=false;
	  while(!member.eof()) {
	    QString name = member.readValue();
	    QString overld = member.readValue();
	    QString arg = member.readValueNull();
	    QString type = member.readValueNull();
	    if (lastName!=name||overld!=lastOver) {
	      if (hasArgs)
		desc.Name+=")";
	      if (!desc.Name.isEmpty())
		ret.insert(ret.end(),desc);
	      desc.Name=name;
	      lastName=name;
	      lastOver=overld;
	      hasArgs=false;
	    }
	    if (arg.isEmpty()) {
	      if (hasArgs) {
		desc.Name+=")";
		hasArgs=false;
	      }
	      desc.Name+=" RETURNING ";
	    } else {
	      if (hasArgs)
		desc.Name+=", ";
	      else {
		desc.Name+="(";
		hasArgs=true;
	      }
	      desc.Name+=arg;
	      desc.Name+=" ";
	    }
	    desc.Name+=type;
	  }
	  if (desc.Name.contains("("))
	    desc.Name+=")";
	  if (!desc.Name.isEmpty())
	    ret.insert(ret.end(),desc);
	} catch (...) {
	}
	return ret;
      }

      std::map<QString,QString> comments;
      try {
	toQuery comment(connection(),SQLComment,table.Owner,table.Name);
	while(!comment.eof()) {
	  QString col=comment.readValue();
	  comments[col]=comment.readValueNull();
	}
      } catch (...) {
      }

      try {
	toQuery query(connection());
#ifdef OTL_STREAM_POOLING_ON
	// Need to clear the stream cache first.
	oracleSub *sub=dynamic_cast<oracleSub *>(query.connectionSub());
	sub->Lock.down();
	sub->Connection->set_stream_pool_size(max(toTool::globalConfig(CONF_OPEN_CURSORS,
								       DEFAULT_OPEN_CURSORS).toInt(),1));
	sub->Lock.up();
#endif
	QString SQL=QString::fromLatin1("SELECT * FROM \"");
	SQL+=table.Owner;
	SQL+=QString::fromLatin1("\".\"");
	SQL+=table.Name;
	SQL+=QString::fromLatin1("\" WHERE NULL=NULL");
	toQList par;
	query.execute(SQL,par);
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
      oracleSub *conn=oracleConv(sub);
      try {
	conn->Connection->commit();
      } catch (const otl_exception &exc) {
	ThrowException(exc);
      }
    }
    virtual void rollback(toConnectionSub *sub)
    {
      oracleSub *conn=oracleConv(sub);
      try {
	conn->Connection->rollback();
      } catch (const otl_exception &exc) {
	ThrowException(exc);
      }
    }

    virtual toConnectionSub *createConnection(void);

    void closeConnection(toConnectionSub *conn)
    {
      delete conn;
    }

    virtual QCString version(toConnectionSub *sub)
    {
      oracleSub *conn=oracleConv(sub);
      try {
	otl_stream version(1,
			   "SELECT banner FROM v$version",
			   *(conn->Connection));
	QRegExp verre(QString::fromLatin1("[0-9]\\.[0-9\\.]+[0-9]"));
	QRegExp orare(QString::fromLatin1("^(\\S+ )?oracle"),false);
	while(!version.eof()) {
	  char buffer[1024];
	  version>>buffer;
	  QString ver=QString::fromUtf8(buffer);
	  if (orare.match(ver)>=0) {
	    int pos;
	    int len;
	    pos=verre.match(ver,0,&len);
	    if (pos>=0)
	      return ver.mid(pos,len).latin1();
	  }
	}
      } catch (...) {
	// Ignore any errors here
      }
      return "";
    }

    virtual toQuery::queryImpl *createQuery(toQuery *query,toConnectionSub *sub)
    { return new oracleQuery(query,oracleConv(sub)); }
    virtual void execute(toConnectionSub *sub,const QCString &sql,toQList &params)
    {
      oracleSub *conn=oracleConv(sub);

      if (params.size()==0) {
	try {
	  otl_cursor::direct_exec(*(conn->Connection),sql);
	} catch (const otl_exception &exc) {
	  conn->throwExtendedException(connection(),exc);
	}
      } else
	toQuery query(connection(),QString::fromUtf8(sql),params);
    }
    virtual void parse(toConnectionSub *sub,const QCString &sql)
    {
      oracleSub *conn=oracleConv(sub);
      try {
	otl_cursor::parse(*(conn->Connection),sql);
      } catch (const otl_exception &exc) {
	ThrowException(exc);
      }
    }
  };

  toOracleProvider(void)
    : toConnectionProvider("Oracle",false)
  {
  }

  virtual void initialize(void)
  {
    toMaxLong=toTool::globalConfig(CONF_MAX_LONG,
				   QString::number(DEFAULT_MAX_LONG).latin1()).toInt();
    if (otl_connect::otl_initialize(1))
      addProvider("Oracle");
  }

  virtual toConnection::connectionImpl *provideConnection(const QCString &,toConnection *conn)
  { return new oracleConnection(conn); }
  virtual std::list<QString> providedModes(const QCString &)
  {
    std::list<QString> ret;
    ret.insert(ret.end(),QString::fromLatin1("Normal"));
    ret.insert(ret.end(),QString::fromLatin1("SYS_OPER"));
    ret.insert(ret.end(),QString::fromLatin1("SYS_DBA"));
    return ret;
  }
  virtual std::list<QString> providedHosts(const QCString &)
  {
    std::list<QString> ret;
    ret.insert(ret.end(),QString::null);
    ret.insert(ret.end(),QString::fromLatin1("SQL*Net"));
    return ret;
  }
  virtual std::list<QString> providedDatabases(const QCString &,const QString &host,const QString &,const QString &)
  {
    QString str;
#ifdef WIN32
    CRegistry registry;
    DWORD siz=1024;
    char buffer[1024];
    try {
      if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
				  "SOFTWARE\\ORACLE\\HOME0",
				  "TNS_ADMIN",
				  buffer,siz)) {
	if (siz>0)
	  str=buffer;
	else
	  throw 0;
      } else
	throw 0;
    } catch(...) {
      try {
	if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
				    "SOFTWARE\\ORACLE\\HOME0",
				    "ORACLE_HOME",
				    buffer,siz)) {
	  if (siz>0) {
	    str=buffer;
	    str+="\\network\\admin";
	  }
	}
      } catch(...) {
      }
    }
#else
    if (!getenv("ORACLE_HOME"))
      throw QString::fromLatin1("ORACLE_HOME environment variable not set");
    if (getenv("TNS_ADMIN")) {
      str=getenv("TNS_ADMIN");
    } else {
      str=getenv("ORACLE_HOME");
      str.append(QString::fromLatin1("/network/admin"));
    }
#endif
    str.append(QString::fromLatin1("/tnsnames.ora"));


    std::list<QString> ret;

    QFile file(str);
    if (!file.open(IO_ReadOnly))
      return ret;
	    
    int size=file.size();
	    
    char *buf=new char[size+1];
    if (file.readBlock(buf,size)==-1) {
      delete[] buf;
      return ret;
    }

    buf[size]=0;

    int begname=-1;
    int parambeg=-1;
    int pos=0;
    int param=0;
    while(pos<size) {
      if (buf[pos]=='#') {
	while(pos<size&&buf[pos]!='\n')
	  pos++;
      } else if (buf[pos]=='=') {
	if (param==0) {
	  if (begname>=0&&!host.isEmpty())
	    ret.insert(ret.end(),QString::fromLatin1(buf+begname,pos-begname));
	}
      } else if (buf[pos]=='(') {
	begname=-1;
	parambeg=pos+1;
	param++;
      } else if (buf[pos]==')') {
	if (parambeg>=0&&host.isEmpty()) {
	  QString tmp=QString::fromLatin1(buf+parambeg,pos-parambeg);
	  tmp.replace(QRegExp(QString::fromLatin1("\\s+")),QString::null);
	  if (tmp.lower().startsWith(QString::fromLatin1("sid=")))
	    ret.insert(ret.end(),tmp.mid(4));
	}
	begname=-1;
	parambeg=-1;
	param--;
      } else if (!isspace(buf[pos])&&begname<0) {
	begname=pos;
      }
      pos++;
    }
    delete[] buf;
    return ret;
  }
  virtual QWidget *providerConfigurationTab(const QCString &provider,QWidget *parent);
};

static toOracleProvider OracleProvider;

void toOracleProvider::oracleQuery::execute(void)
{
  oracleSub *conn=dynamic_cast<oracleSub *>(query()->connectionSub());
  if (!conn)
    throw QString::fromLatin1("Internal error, not oracle sub connection");
  try {
    delete Query;
    Query=NULL;

    while (conn->Lock.getValue()>1) {
      conn->Lock.down();
      toStatusMessage(QString::fromLatin1("Too high value on connection lock semaphore"));
    }

    conn->Lock.down();
    if (Cancel)
      throw QString::fromLatin1("Query aborted before started");
    Running=true;
    try {
      static QRegExp stripnl("\r");
      Query=new otl_stream;
      Query->set_commit(0);
      if (toQValue::numberFormat()==0)
	Query->set_all_column_types(otl_all_num2str|otl_all_date2str);
      else
	Query->set_all_column_types(otl_all_date2str);

      QCString sql=query()->sql();
      sql.replace(stripnl,"");
      Query->open(1,
		  sql,
		  *(conn->Connection));
    } catch(...) {
      conn->Lock.up();
      throw;
    }
  } catch (const otl_exception &exc) {
    Running=false;
    conn->throwExtendedException(query()->connection(),exc);
  }
  try {
    otl_null null;
    for(toQList::iterator i=query()->params().begin();i!=query()->params().end();i++) {
      if ((*i).isNull())
	(*Query)<<null;
      else {
	otl_var_desc *next=Query->describe_next_in_var();
	if (next) {
	  switch(next->ftype) {
	  case otl_var_double:
	  case otl_var_float:
	    (*Query)<<(*i).toDouble();
	    break;
	  case otl_var_int:
	  case otl_var_unsigned_int:
	  case otl_var_short:
	  case otl_var_long_int:
	    (*Query)<<(*i).toInt();
	    break;
	  default:
	    (*Query)<<QString(*i).utf8();
	    break;
	  }
	} else {
	  printf("ERROR: More parameters than binds\n");
	}
      }
    }
    Running=false;
    conn->Lock.up();
  } catch (const otl_exception &exc) {
    Running=false;
    conn->Lock.up();
    conn->throwExtendedException(query()->connection(),exc);
  }
}

void toOracleProvider::oracleQuery::cancel(void)
{
  oracleSub *conn=dynamic_cast<oracleSub *>(query()->connectionSub());
  if (!conn)
    throw QString::fromLatin1("Internal error, not oracle sub connection");
  if (Running)
    conn->Connection->cancel();
  else {
    Cancel=true;
    conn->Lock.up();
  }
}

toConnectionSub *toOracleProvider::oracleConnection::createConnection(void)
{
  QString oldSid;
  bool sqlNet=!connection().host().isEmpty();
  if (!sqlNet) {
    oldSid=getenv("ORACLE_SID");
    toSetEnv("ORACLE_SID",connection().database().utf8());
  }
  otl_connect *conn=NULL;
  try {
    QString mode=connection().mode();
    int session_mode=OCI_DEFAULT;
    if (mode==QString::fromLatin1("SYS_OPER"))
      session_mode=OCI_SYSOPER;
    else if (mode==QString::fromLatin1("SYS_DBA"))
      session_mode=OCI_SYSDBA;
    do {
      conn=new otl_connect;
#ifdef OTL_STREAM_POOLING_ON
      conn->set_stream_pool_size(max(toTool::globalConfig(CONF_OPEN_CURSORS,
							  DEFAULT_OPEN_CURSORS).toInt(),1));
#endif
      if(!sqlNet)
	conn->server_attach();
      else
	conn->server_attach(connection().database().utf8());
      QCString user=connection().user().utf8();
      QCString pass=connection().password().utf8();
      try {
	conn->session_begin(user.isEmpty()?"":(const char *)user,pass.isEmpty()?"":(const char *)pass,0,session_mode);
      } catch(const otl_exception &exc) {
	if (toThread::mainThread()&&exc.code==28001) {
	  bool ok=false;
	  QString newpass=QInputDialog::getText(qApp->translate("toOracleConnection","Password expired"),
						qApp->translate("toOracleConnection","Enter new password"),
						QLineEdit::Password,
						QString::null,
						&ok,
						toMainWidget());
	  if (ok) {
	    QString newpass2=QInputDialog::getText(qApp->translate("toOracleConnection","Password expired"),
						   qApp->translate("toOracleConnection","Enter password again for confirmation"),
						   QLineEdit::Password,
						   QString::null,
						   &ok,
						   toMainWidget());
	    if (ok) {
	      if (newpass2!=newpass)
		throw qApp->translate("toOracleConnection","The two passwords doesn't match");
	      QCString nputf=newpass.utf8();
	      conn->change_password(user.isEmpty()?"":(const char *)user,
				    pass.isEmpty()?"":(const char *)pass,
				    newpass.isEmpty()?"":(const char *)nputf);
	      connection().setPassword(newpass);
	      delete conn;
	      conn=NULL;
	    } else
	      throw exc;
	  } else {
	    throw exc;
	  }
	} else
	  throw exc;
      }
    } while(!conn);
  } catch (const otl_exception &exc) {
    if (!sqlNet) {
      if (oldSid.isNull())
	toUnSetEnv("ORACLE_SID");
      else
	toSetEnv("ORACLE_SID",oldSid.latin1());
    }
    delete conn;
    ThrowException(exc);
  }
  if (!sqlNet) {
    if (oldSid.isNull())
      toUnSetEnv("ORACLE_SID");
    else {
      toSetEnv("ORACLE_SID",oldSid.latin1());
    }
  }
  
  try {
    {
      QString str=QString::fromLatin1("ALTER SESSION SET NLS_DATE_FORMAT = '");
      str+=toTool::globalConfig(CONF_DATE_FORMAT,DEFAULT_DATE_FORMAT);
      str+=QString::fromLatin1("'");
      otl_stream date(1,str.utf8(),*conn);
    }
    {
      otl_stream info(1,
		      "BEGIN\n"
		      "  SYS.DBMS_APPLICATION_INFO.SET_CLIENT_INFO('" TOAPPNAME
#ifndef OAS
		      " (http://www.globecom.se/tora)"
#endif
		      "');\n"
		      "  SYS.DBMS_APPLICATION_INFO.SET_MODULE('" TOAPPNAME "','Access Database');\n"
		      "END;",
		      *conn);
    }
  } catch(...) {
    toStatusMessage(QString::fromLatin1("Failed to set new default date format for session"));
  }
  return new oracleSub(conn);
}

static toSQL SQLCreatePlanTable(toSQL::TOSQL_CREATEPLAN,
				"CREATE TABLE %1 (\n"
				"    STATEMENT_ID    VARCHAR2(30),\n"
				"    TIMESTAMP       DATE,\n"
				"    REMARKS         VARCHAR2(80),\n"
				"    OPERATION       VARCHAR2(30),\n"
				"    OPTIONS         VARCHAR2(30),\n"
				"    OBJECT_NODE     VARCHAR2(128),\n"
				"    OBJECT_OWNER    VARCHAR2(30),\n"
				"    OBJECT_NAME     VARCHAR2(30),\n"
				"    OBJECT_INSTANCE NUMERIC,\n"
				"    OBJECT_TYPE     VARCHAR2(30),\n"
				"    OPTIMIZER       VARCHAR2(255),\n"
				"    SEARCH_COLUMNS  NUMBER,\n"
				"    ID              NUMERIC,\n"
				"    PARENT_ID       NUMERIC,\n"
				"    POSITION        NUMERIC,\n"
				"    COST            NUMERIC,\n"
				"    CARDINALITY     NUMERIC,\n"
				"    BYTES           NUMERIC,\n"
				"    OTHER_TAG       VARCHAR2(255),\n"
				"    PARTITION_START VARCHAR2(255),\n"
				"    PARTITION_STOP  VARCHAR2(255),\n"
				"    PARTITION_ID    NUMERIC,\n"
				"    OTHER           LONG,\n"
				"    DISTRIBUTION    VARCHAR2(30)\n"
				")",
				"Create plan table, must have same % signs");

class toOracleSetting : public toOracleSettingUI, public toSettingTab
{
public:
  toOracleSetting(QWidget *parent)
    : toOracleSettingUI(parent),toSettingTab("database.html#oracle")
  {
    DefaultDate->setText(toTool::globalConfig(CONF_DATE_FORMAT,
					      DEFAULT_DATE_FORMAT));
    CheckPoint->setText(toTool::globalConfig(CONF_PLAN_CHECKPOINT,
					     DEFAULT_PLAN_CHECKPOINT));
    ExplainPlan->setText(toTool::globalConfig(CONF_PLAN_TABLE,
					      DEFAULT_PLAN_TABLE));
    OpenCursors->setValue(toTool::globalConfig(CONF_OPEN_CURSORS,
					       DEFAULT_OPEN_CURSORS).toInt());
    KeepPlans->setChecked(!toTool::globalConfig(CONF_KEEP_PLANS,"").isEmpty());
    int len=toTool::globalConfig(CONF_MAX_LONG,
				 QString::number(DEFAULT_MAX_LONG).latin1()).toInt();
    if (len>=0) {
      MaxLong->setText(QString::number(len));
      MaxLong->setValidator(new QIntValidator(MaxLong));
      Unlimited->setChecked(false);
    }
    try {
      // Check if connection exists
      toMainWidget()->currentConnection();
      CreatePlanTable->setEnabled(true);
    } catch (...) {
    }
  }
  virtual void saveSetting(void)
  {
    toTool::globalSetConfig(CONF_KEEP_PLANS,KeepPlans->isChecked()?"Yes":"");
    toTool::globalSetConfig(CONF_DATE_FORMAT,DefaultDate->text());
    toTool::globalSetConfig(CONF_PLAN_CHECKPOINT,CheckPoint->text());
    toTool::globalSetConfig(CONF_PLAN_TABLE,ExplainPlan->text());
    toTool::globalSetConfig(CONF_OPEN_CURSORS,QString::number(OpenCursors->value()));
    if (Unlimited->isChecked()) {
      toMaxLong=-1;
      toTool::globalSetConfig(CONF_MAX_LONG,QString::fromLatin1("-1"));
    } else {
      toTool::globalSetConfig(CONF_MAX_LONG,MaxLong->text());
      toMaxLong=MaxLong->text().toInt();
    }
  }
  virtual void createPlanTable(void)
  {
    try {
      toConnection &conn=toMainWidget()->currentConnection();
      conn.execute(toSQL::string(SQLCreatePlanTable,conn).
		   arg(ExplainPlan->text()));
    } TOCATCH
  }
};

QWidget *toOracleProvider::providerConfigurationTab(const QCString &,QWidget *parent)
{
  return new toOracleSetting(parent);
}
