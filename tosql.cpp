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
 ****************************************************************************/

TO_NAMESPACE;

#include <qfile.h>
#include <qregexp.h>

#include "tosql.h"
#include "toconnection.h"

toSQL::sqlMap *toSQL::Definitions;

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
    if (description.isNull()) {
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
    if (!description.isNull()) {
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
  QFile file(expandFile(filename));
  if (!file.open(IO_WriteOnly))
    return false;

  QRegExp backslash("\\");
  QRegExp newline("\n");
  for (sqlMap::iterator i=Definitions->begin();i!=Definitions->end();i++) {
    definition &def=(*i).second;
    QString name=(*i).first;
    if (def.Modified||all) {
      QString line=name;
      line+="=";
      line+=def.Description;
      line.replace(backslash,"\\\\");
      line.replace(newline,"\\n");
      file.writeBlock(line,line.length());
      file.writeBlock("\n",1);
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
	file.writeBlock(line,line.length());
	line=ver.SQL;
	line.replace(backslash,"\\\\");
	line.replace(newline,"\\n");
	QCString str=line.utf8();
	file.writeBlock(str,str.length());
	file.writeBlock("\n",1);
      }
    }
  }
  return true;
}

QString toSQL::expandFile(const QString &file)
{
  QString ret(file);
  ret.replace(QRegExp("$HOME"),getenv("HOME"));
  return ret;
}

bool toSQL::loadSQL(const QString &filename)
{
  allocCheck();
  QFile file(expandFile(filename));
  if (!file.open(IO_ReadOnly))
    return false;
  
  int size=file.size();
  
  char *buf=new char[size+1];
  try {
    if (file.readBlock(buf,size)==-1) {
      throw QString("Encountered problems read configuration");
    }
    buf[size]=0;
    int pos=0;
    int bol=0;
    int endtag=-1;
    int vertag=-1;
    int wpos=0;
    while(pos<size) {
      switch(buf[pos]) {
      case '\n':
	if (endtag==-1)
	  throw QString("Malformed tag in config file. Missing = on row.");
	buf[wpos]=0;
	{
	  QString nam=buf+bol;
	  QString val(QString::fromUtf8(buf+endtag+1));
	  if (vertag==-1)
	    updateSQL(nam,QString::null,val,QString::null,true);
	  else
	    updateSQL(nam,val,QString::null,buf+vertag+1,true);
	}
	bol=pos+1;
	vertag=endtag=-1;
	wpos=pos;
	break;
      case '=':
	if (endtag==-1) {
	  endtag=pos;
	  buf[wpos]=0;
	  wpos=pos;
	} else
	  buf[wpos]=buf[pos];
	break;
      case '[':
	if (endtag==-1) {
	  if (vertag>=0)
	    throw QString("Malformed line in SQL dictionary file. Two '[' before '='");
	  vertag=pos;
	  buf[wpos]=0;
	  wpos=pos;
	} else
	  buf[wpos]=buf[pos];
	break;
      case ']':
	if (endtag==-1) {
	  buf[wpos]=0;
	  wpos=pos;
	} else
	  buf[wpos]=buf[pos];
	break;
      case '\\':
	pos++;
	switch(buf[pos]) {
	case 'n':
	  buf[wpos]='\n';
	  break;
	case '\\':
	  if (endtag>=0)
	    buf[wpos]='\\';
	  else
	    buf[wpos]=':';
	  break;
	default:
	  throw QString("Unknown escape character in string (Only \\\\ and \\n recognised)");
	}
	break;
      default:
	buf[wpos]=buf[pos];
      }
      wpos++;
      pos++;
    }
  } catch(...) {
    delete buf;
    throw;
  }
  delete buf;
  return true;
}
