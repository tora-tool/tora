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
#include "tosql.h"

#include <qfile.h>
#include <qregexp.h>

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
  toStatusMessage(str);
  toSQL empty("Unknown");
  return empty;
}

QString toSQL::string(const QString &name,
		      const toConnection &conn)
{
  allocCheck();
  QString ver=conn.version();
  QString prov=conn.provider();

  bool quit=false;

  sqlMap::iterator i=Definitions->find(name);
  if (i!=Definitions->end()) {
    do {
      if (prov=="Any")
	quit=true;
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

      prov="Any";
    } while(!quit);
  }

  QString str="Tried to get unknown SQL (";
  str+=name;
  str+=")";
  toStatusMessage(str);
  return "";
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
  QCString data=toReadFile(filename);
  
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
