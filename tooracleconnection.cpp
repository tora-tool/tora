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

#include <qfile.h>
#include <qregexp.h>

#include "tomain.h"

#ifdef WIN32
#  include "windows/cregistry.h"
#endif

#include "otlv32.h"
#include "toconnection.h"
#include "toconf.h"
#include "totool.h"

// Must be larger than max long size in otl.

#ifndef TO_ORACLE_LONG_SIZE
#define TO_ORACLE_LONG_SIZE 33000 
#endif

static void ThrowException(const otl_exception &exc)
{
  QString ret=QString::fromUtf8((const char *)exc.msg);
#if 1
  if (strlen(exc.stm_text)) {
    ret+="\n";
    ret+=QString::fromUtf8((const char *)exc.stm_text);
  }
#endif
  throw ret;
}

class toOracleProvider : public toConnectionProvider {
public:
  class oracleSub : public toConnectionSub {
  public:
    otl_connect *Connection;
    oracleSub(otl_connect *conn)
    { Connection=conn; }
    ~oracleSub()
    { delete Connection; }
  };

  class oracleQuery : public toQuery::queryImpl {
    otl_stream *Query;
  public:
    oracleQuery(toQuery *query,oracleSub *conn)
      : toQuery::queryImpl(query)
    {
      Query=NULL;
    }
    virtual ~oracleQuery()
    { delete Query; }
    virtual void execute(void);

    virtual toQValue readValue(void)
    {
      char *buffer=NULL;
      otl_var_desc *dsc=Query->describe_next_out_var();
      if (!dsc)
	throw QString("Couldn't get description of next column to read");

      try {
	toQValue null;
	switch (dsc->ftype) {
	case otl_var_double:
	case otl_var_float:
	  {
	    double d;
	    (*Query)>>d;
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
	    if (Query->is_null())
	      return null;
	    return toQValue(i);
	  }
	  break;
	case otl_var_varchar_long:
	case otl_var_raw_long:
	  {
	    buffer=new char[TO_ORACLE_LONG_SIZE+1];
	    buffer[TO_ORACLE_LONG_SIZE]=0;
	    otl_long_string str(buffer,TO_ORACLE_LONG_SIZE);
	    (*Query)>>str;
	    if (!str.len())
	      return null;
	    QString buf(QString::fromUtf8(buffer));
	    delete buffer;
	    return buf;
	  }
	case otl_var_clob:
	case otl_var_blob:
	  {
	    otl_lob_stream lob;
	    (*Query)>>lob;
	    if (lob.len()==0)
	      return null;
	    buffer=new char[lob.len()+1];
	    buffer[0]=0;
	    otl_long_string data(buffer,lob.len());
	    lob>>data;
	    if (!lob.eof()) {
	      otl_long_string sink(10000);
	      while(!lob.eof()) {
		lob>>sink;
	      }
	      toStatusMessage("Weird data exists past length of LOB");
	    }
	    buffer[data.len()]=0; // Not sure if this is needed
	    QString buf(QString::fromUtf8(buffer));
	    delete buffer;
	    return buf;
	  }
	  break;
	default:  // Try using char if all else fails
	  {
	    // The *2 is for raw columns, also dates and numbers are a bit tricky
	    // but if someone specifies a dateformat longer than 100 bytes he
	    // deserves everything he gets!
	    buffer=new char[max(dsc->elem_size*2+1,100)];
	    buffer[0]=0;
	    (*Query)>>buffer;
	    if (Query->is_null()) {
	      delete buffer;
	      return null;
	    }
	    QString buf(QString::fromUtf8(buffer));
	    delete buffer;
	    return buf;
	  }
	  break;
	}
      } catch (const otl_exception &exc) {
	delete buffer;
	ThrowException(exc);
      } catch (...) {
	delete buffer;
	throw;
      }
      // Never get here
      return QString::null;
    }
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

	desc.Name=QString::fromUtf8(description[i].name);

	switch(description[i].dbtype) {
	case 1:
	case 5:
	case 9:
	case 155:
	  desc.Datatype="VARCHAR2";
	  break;
	case 2:
	case 3:
	case 4:
	case 6:
	case 68:
	  desc.Datatype="NUMBER";
	  break;
	case 8:
	case 94:
	case 95:
	  desc.Datatype="LONG";
	  break;
	case 11:
	case 104:
	  desc.Datatype="ROWID";
	  break;
	case 12:
	case 156:
	  desc.Datatype="DATE";
	  break;
	case 15:
	case 23:
	case 24:
	  desc.Datatype="RAW";
	  break;
	case 96:
	case 97:
	  desc.Datatype="CHAR";
	  break;
	case 108:
	  desc.Datatype="NAMED DATA TYPE";
	  break;
	case 110:
	  desc.Datatype="REF";
	  break;
	case 112:
	  desc.Datatype="CLOB";
	  break;
	case 113:
	case 114:
	  desc.Datatype="BLOB";
	  break;
	}

	if (desc.Datatype=="NUMBER") {
	  if (description[i].prec) {
	    desc.Datatype.append(" (");
	    desc.Datatype.append(QString::number(description[i].prec));
	    if (description[i].scale!=0) {
	      desc.Datatype.append(",");
	      desc.Datatype.append(QString::number(description[i].scale));
	    }
	    desc.Datatype.append(")");
	  }
	} else {
	  desc.Datatype.append(" (");
	  desc.Datatype.append(QString::number(description[i].dbsize));
	  desc.Datatype.append(")");
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
      ret+="/";
      ret+=connection().password().utf8();
      if (!connection().host().isEmpty()) {
	ret+="@";
	ret+=connection().database().utf8();
      }
      return ret;
    }
    oracleSub *oracleConv(toConnectionSub *sub)
    {
      oracleSub *conn=dynamic_cast<oracleSub *>(sub);
      if (!conn)
	throw QString("Internal error, not oracle sub connection");
      return conn;
    }
  public:
    oracleConnection(toConnection *conn)
      : toConnection::connectionImpl(conn)
    { }

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

    virtual toConnectionSub *createConnection(void)
    {
      QString oldSid;
      bool sqlNet=!connection().host().isEmpty();
      if (!sqlNet) {
	oldSid=getenv("ORACLE_SID");
	toSetEnv("ORACLE_SID",connection().database().latin1());
      }
      otl_connect *conn;
      try {
	QString mode=connection().mode();
	int oper=0;
	int dba=0;
	if (mode=="SYS_OPER")
	  oper=1;
	else if (mode=="SYS_DBA")
	  dba=1;
	conn=new otl_connect(connectString(),0,oper,dba);
      } catch (const otl_exception &exc) {
	if (!sqlNet) {
	  if (oldSid.isNull())
	    toUnSetEnv("ORACLE_SID");
	  else
	    toSetEnv("ORACLE_SID",oldSid.latin1());
	}
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
	  QString str="ALTER SESSION SET NLS_DATE_FORMAT = '";
	  str+=toTool::globalConfig(CONF_DATE_FORMAT,DEFAULT_DATE_FORMAT);
	  str+="'";
	  otl_stream date(1,str.utf8(),*conn);
	}
	{
	  otl_stream info(1,
			  "BEGIN\n"
			  "  DBMS_APPLICATION_INFO.SET_CLIENT_INFO('TOra (http://www.globecom.net/tora)');\n"
			  "END;",
			  *conn);
	}
      } catch(...) {
	toStatusMessage("Failed to set new default date format for session");
      }
      return new oracleSub(conn);
    }
    void closeConnection(toConnectionSub *conn)
    {
      delete conn;
    }

    virtual QString version(toConnectionSub *sub)
    {
      oracleSub *conn=oracleConv(sub);
      try {
	otl_stream version(1,
			   "SELECT banner FROM v$version",
			   *(conn->Connection));
	QRegExp verre("[0-9]\\.[0-9\\.]+[0-9]");
	QRegExp orare("^oracle",false);
	while(!version.eof()) {
	  char buffer[1024];
	  version>>buffer;
	  QString ver=QString::fromUtf8(buffer);
	  if (orare.match(ver)>=0) {
	    int pos;
	    int len;
	    pos=verre.match(ver,0,&len);
	    if (pos>=0)
	      return ver.mid(pos,len);
	  }
	}
      } catch (...) {
	// Ignore any errors here
      }
      return QString::null;
    }
    virtual toConnection::connectionImpl *clone(toConnection *newConn) const
    { return new oracleConnection(newConn); }

    virtual toQuery::queryImpl *createQuery(toQuery *query,toConnectionSub *sub)
    { return new oracleQuery(query,oracleConv(sub)); }
    virtual void execute(toConnectionSub *sub,const QCString &sql,toQList &params)
    {
      oracleSub *conn=oracleConv(sub);

      if (params.size()==0) {
	try {
	  otl_cursor::direct_exec(*(conn->Connection),sql);
	} catch (const otl_exception &exc) {
	  ThrowException(exc);
	}
      } else
	toQuery query(connection(),sql,params);
    }
  };

  toOracleProvider(void)
    : toConnectionProvider("Oracle")
  {
    otl_connect::otl_initialize(1);
  }

  virtual toConnection::connectionImpl *connection(toConnection *conn)
  { return new oracleConnection(conn); }
  virtual std::list<QString> modes(void)
  {
    std::list<QString> ret;
    ret.insert(ret.end(),"Normal");
    ret.insert(ret.end(),"SYS_OPER");
    ret.insert(ret.end(),"SYS_DBA");
    return ret;
  }
  virtual std::list<QString> hosts(void)
  {
    std::list<QString> ret;
    ret.insert(ret.end(),QString::null);
    ret.insert(ret.end(),"SQL*Net");
    return ret;
  }
  virtual std::list<QString> databases(const QString &host,const QString &,const QString &)
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
      throw QString("ORACLE_HOME environment variable not set");
    if (getenv("TNS_ADMIN")) {
      str=getenv("TNS_ADMIN");
    } else {
      str=getenv("ORACLE_HOME");
      str.append("/network/admin");
    }
#endif
    str.append("/tnsnames.ora");


    std::list<QString> ret;

    QFile file(str);
    if (!file.open(IO_ReadOnly))
      return ret;
	    
    int size=file.size();
	    
    char *buf=new char[size+1];
    if (file.readBlock(buf,size)==-1) {
      delete buf;
      return ret;
    }

    buf[size]=0;

    char *begname=NULL;
    int pos=0;
    int param=0;
    while(pos<size) {
      if (buf[pos]=='#') {
	buf[pos]=0;
	while(pos<size&&buf[pos]!='\n')
	  pos++;
      } else if (isspace(buf[pos])) {
	buf[pos]=0;
      } else if (buf[pos]=='=') {
	if (param==0) {
	  buf[pos]=0;
	  if (begname)
	    ret.insert(ret.end(),begname);
	}
      } else if (buf[pos]=='(') {
	begname=NULL;
	param++;
      } else if (buf[pos]==')') {
	begname=NULL;
	param--;
      } else if (!begname) {
	begname=buf+pos;
      }
      pos++;
    }
    delete buf;
    return ret;
  }
};

static toOracleProvider OracleProvider;

void toOracleProvider::oracleQuery::execute(void)
{
  oracleSub *conn=dynamic_cast<oracleSub *>(query()->connectionSub());
  if (!conn)
    throw QString("Internal error, not oracle sub connection");
  try {
    delete Query;
    Query=new otl_stream;
    Query->set_commit(0);
    Query->set_all_column_types(otl_all_num2str|otl_all_date2str);
    Query->open(1,
		query()->sql(),
		*(conn->Connection));
    otl_null null;
    for(toQList::iterator i=query()->params().begin();i!=query()->params().end();i++) {
      if ((*i).isNull())
	(*Query)<<null;
      else if ((*i).isString())
	(*Query)<<QString(*i).utf8();
      else if ((*i).isInt())
	(*Query)<<(*i).toInt();
      else if ((*i).isDouble())
	(*Query)<<(*i).toDouble();
      else
	throw QString("Unknown input argument type");
    }
  } catch (const otl_exception &exc) {
    ThrowException(exc);
  }
}
