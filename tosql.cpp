//***************************************************************************
/*
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

TO_NAMESPACE;

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
	     const QString &ver)
  : Name(name)
{
  updateSQL(name,sql,description,ver,false);
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
		      bool modified)
{
  version def(ver,sql,modified);

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
      list<version> &cl=newDef.Versions;
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
    list<version> &cl=(*i).second.Versions;
    for (list<version>::iterator j=cl.begin();j!=cl.end();j++) {
      if ((*j).Version==ver) {
	if (!sql.isNull()) {
	  (*j)=def;
	  if (def.SQL!=(*j).SQL)
	    (*j).Modified=modified;
	}
	return false;
      } else if ((*j).Version>ver) {
	if (!sql.isNull()) {
	  cl.insert(j,def);
	}
	return true;
      }
    }
    cl.insert(cl.end(),def);
    return true;
  }
}

bool toSQL::deleteSQL(const QString &name,
		      const QString &ver)
{
  allocCheck();
  sqlMap::iterator i=Definitions->find(name);
  if (i==Definitions->end()) {
    return false;
  } else {
    list<version> &cl=(*i).second.Versions;
    for (list<version>::iterator j=cl.begin();j!=cl.end();j++) {
      if ((*j).Version==ver) {
	cl.erase(j);
	if (cl.begin()==cl.end())
	  Definitions->erase(i);
	return true;
      } else if ((*j).Version>ver) {
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
  sqlMap::iterator i=Definitions->find(name);
  if (i!=Definitions->end()) {
    QString *sql=NULL;
    list<version> &cl=(*i).second.Versions;
    for (list<version>::iterator j=cl.begin();j!=cl.end();j++) {
      if ((*j).Version<=ver||!sql) {
	sql=&(*j).SQL;
      }
      if ((*j).Version>=ver)
	return *sql;
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
    for (list<version>::iterator j=def.Versions.begin();j!=def.Versions.end();j++) {
      version &ver=(*j);
      if (ver.Modified||all) {
	QString line=name;
	line+="[";
	line+=ver.Version;
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
	if (vertag==-1)
	  updateSQL(nam,QString::null,val,QString::null,true);
	else
	  updateSQL(nam,val,QString::null,QString::fromUtf8(((const char *)data)+vertag+1),true);
      }
      bol=pos+1;
      vertag=endtag=-1;
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
	if (vertag>=0)
	  throw QString("Malformed line in SQL dictionary file. Two '[' before '='");
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

list<QString> toSQL::range(const QString &startWith)
{
  list<QString> ret;
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
