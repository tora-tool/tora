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

#include "tomain.h"
#include "tosql.h"
#include "toconnection.h"

toSQL::sqlMap *toSQL::Definitions;
const char * const toSQL::TOSQL_USERLIST= "Global:UserList";
const char * const toSQL::TOSQL_CREATEPLAN= "Global:CreatePlan";

toSQL::toSQL(const QString &name,
	     const QString &sql,
	     const QString &description,
	     const QString &ver,
	     const QString &provider)
  : Name(name)
{
  updateSQL(name,sql,description,ver,provider,false);
}

toSQL::toSQL(const QString &name)
  : Name(name)
{
}

void toSQL::allocCheck(void)
{
  if (!Definitions) Definitions=new sqlMap;
}

bool toSQL::updateSQL(const QString &name,
		      const QString &sql,
		      const QString &description,
		      const QString &ver,
		      const QString &provider,
		      bool modified)
{
  version def(provider,ver,sql,modified);

  allocCheck();
  sqlMap::iterator i=Definitions->find(name);
  if (i==Definitions->end()) {
    if (description.isEmpty()) {
      fprintf(stderr,"ERROR:Tried add new version to unknown SQL (%s)\n",(const char *)name);
      return false;
    }
    definition newDef;
    newDef.Modified=modified;
    newDef.Description=description;
    if (!sql.isNull()) {
      std::list<version> &cl=newDef.Versions;
      cl.insert(cl.end(),def);
    }
    (*Definitions)[name]=newDef;
    return true;
  } else {
    if (!description.isEmpty()) {
      if ((*i).second.Description!=description) {
	(*i).second.Description=description;
	(*i).second.Modified=modified;
      }
      if (!modified)
	fprintf(stderr,"ERROR:Overwrite description of nonmodified (%s)\n",(const char *)name);
    }
    std::list<version> &cl=(*i).second.Versions;
    for (std::list<version>::iterator j=cl.begin();j!=cl.end();j++) {
      if ((*j).Provider==provider&&(*j).Version==ver) {
	if (!sql.isNull()) {
	  (*j)=def;
	  if (def.SQL!=(*j).SQL)
	    (*j).Modified=modified;
	}
	return false;
      } else if ((*j).Provider>provider||
		 ((*j).Provider==provider&&(*j).Version>ver)) {
	if (!sql.isNull())
	  cl.insert(j,def);
	return true;
      }
    }
    cl.insert(cl.end(),def);
    return true;
  }
}

bool toSQL::deleteSQL(const QString &name,
		      const QString &ver,
		      const QString &provider)
{
  allocCheck();
  sqlMap::iterator i=Definitions->find(name);
  if (i==Definitions->end()) {
    return false;
  } else {
    std::list<version> &cl=(*i).second.Versions;
    for (std::list<version>::iterator j=cl.begin();j!=cl.end();j++) {
      if ((*j).Version==ver&&(*j).Provider==provider) {
	cl.erase(j);
	if (cl.begin()==cl.end())
	  Definitions->erase(i);
	return true;
      } else if ((*j).Provider>provider||
		 ((*j).Provider==provider&&(*j).Version>ver)) {
	return false;
      }
    }
    return false;
  }
}

toSQL toSQL::sql(const QString &name)
{
  allocCheck();
  sqlMap::iterator i=Definitions->find(name);
  if (i!=Definitions->end())
    return name;
  QString str="Tried to get unknown SQL (";
  str+=name;
  str+=")";
  throw str;
}

QString toSQL::string(const QString &name,
		      const toConnection &conn)
{
  allocCheck();
  const QString &ver=conn.version();
  const QString &prov=conn.provider();
  sqlMap::iterator i=Definitions->find(name);
  if (i!=Definitions->end()) {
    QString *sql=NULL;
    std::list<version> &cl=(*i).second.Versions;
    for (std::list<version>::iterator j=cl.begin();j!=cl.end();j++) {
      if ((*j).Provider==prov) {
	if ((*j).Version<=ver||!sql) {
	  sql=&(*j).SQL;
	}
	if ((*j).Version>=ver)
	  return *sql;
      }
    }
    if (sql)
      return *sql;
  }
  QString str="Tried to get unknown SQL (";
  str+=name;
  str+=")";
  throw str;
}

bool toSQL::saveSQL(const QString &filename,bool all)
{
  allocCheck();
  QCString data;

  QRegExp backslash("\\");
  QRegExp newline("\n");
  for (sqlMap::iterator i=Definitions->begin();i!=Definitions->end();i++) {
    QCString str;
    definition &def=(*i).second;
    QString name=(*i).first;
    if (def.Modified||all) {
      QString line=name;
      line+="=";
      line+=def.Description;
      line.replace(backslash,"\\\\");
      line.replace(newline,"\\n");
      str=line.latin1();
      str+="\n";
    }
    for (std::list<version>::iterator j=def.Versions.begin();j!=def.Versions.end();j++) {
      version &ver=(*j);
      if (ver.Modified||all) {
	QString line=name;
	line+="[";
	line+=ver.Version;
	line+="][";
	line+=ver.Provider;
	line+="]=";
	line.replace(backslash,"\\\\");
	line.replace(newline,"\\n");
	str+=line.utf8();
	line=ver.SQL;
	line.replace(backslash,"\\\\");
	line.replace(newline,"\\n");
	str+=line.utf8();
	str+="\n";
      }
    }
    data+=str;
  }
  return toWriteFile(filename,data);
}

void toSQL::loadSQL(const QString &filename)
{
  allocCheck();
  QCString data;
  try {
    data=toReadFile(filename);
  } catch(...) {
    return;
  }
  
  int size=data.length();
  int pos=0;
  int bol=0;
  int endtag=-1;
  int provtag=-1;
  int vertag=-1;
  int wpos=0;
  while(pos<size) {
    switch(data[pos]) {
    case '\n':
      if (endtag==-1)
	throw QString("Malformed tag in config file. Missing = on row.");
      data[wpos]=0;
      {
	QString nam=((const char *)data)+bol;
	QString val(QString::fromUtf8(((const char *)data)+endtag+1));
	QString ver;
	QString prov;
	if (vertag>=0) {
	  ver=QString::fromUtf8(((const char *)data)+vertag+1);
	  if (provtag>=0)
	    prov=QString::fromUtf8(((const char *)data)+provtag+1);
	  updateSQL(nam,val,QString::null,ver,prov,true);
	} else
	  updateSQL(nam,QString::null,val,QString::null,QString::null,true);
      }
      bol=pos+1;
      provtag=vertag=endtag=-1;
      wpos=pos;
      break;
    case '=':
      if (endtag==-1) {
	endtag=pos;
	data[wpos]=0;
	wpos=pos;
      } else
	data[wpos]=data[pos];
      break;
    case '[':
      if (endtag==-1) {
	if (vertag>=0) {
	  if (provtag>=0)
	    throw QString("Malformed line in SQL dictionary file. Two '[' before '='");
	  provtag=pos;
	} else
	  vertag=pos;
	data[wpos]=0;
	wpos=pos;
      } else
	data[wpos]=data[pos];
      break;
    case ']':
      if (endtag==-1) {
	data[wpos]=0;
	wpos=pos;
      } else
	data[wpos]=data[pos];
      break;
    case '\\':
      pos++;
      switch(data[pos]) {
      case 'n':
	data[wpos]='\n';
	break;
      case '\\':
	if (endtag>=0)
	  data[wpos]='\\';
	else
	  data[wpos]=':';
	break;
      default:
	throw QString("Unknown escape character in string (Only \\\\ and \\n recognised)");
      }
      break;
    default:
      data[wpos]=data[pos];
    }
    wpos++;
    pos++;
  }
}

std::list<QString> toSQL::range(const QString &startWith)
{
  std::list<QString> ret;
  for(sqlMap::iterator i=Definitions->begin();i!=Definitions->end();i++) {
    if ((*i).first>startWith) {
      if ((*i).first.startsWith(startWith))
	ret.insert(ret.end(),(*i).first);
      else
	return ret;
    }
  }
  return ret;
}

QString toSQL::description(const QString &name)
{
  allocCheck();
  sqlMap::iterator i=Definitions->find(name);
  if (i!=Definitions->end())
    return (*i).second.Description;
  QString str="Tried to get unknown SQL (";
  str+=name;
  str+=")";
  throw str;
}
