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

#include <stdio.h>

#include "tosql.h"
#include "toconnection.h"

toSQL::sqlMap *toSQL::Definitions;

bool toSQL::updateSQL(const QString &name,
		      const QString &sql,
		      const QString &description,
		      const QString &version)
{
  versions def(version,sql);

  allocCheck();
  sqlMap::iterator i=Definitions->find(name);
  if (i==Definitions->end()) {
    if (description.isNull()) {
      fprintf(stderr,"ERROR:Tried add new version to unknown SQL (%s)\n",(const char *)name);
      return false;
    }
    definition newDef;
    newDef.Description=description;
    list<versions> &cl=newDef.Versions;
    cl.insert(cl.end(),def);
    (*Definitions)[name]=newDef;
    return true;
  } else {
    (*i).second.Description=description;
    list<versions> &cl=(*i).second.Versions;
    for (list<versions>::iterator j=cl.begin();j!=cl.end();j++) {
      if ((*j).Version==version) {
	(*j)=def;
	return false;
      } else if ((*j).Version>version) {
	cl.insert(j,def);
	return true;
      }
    }
    cl.insert(cl.end(),def);
    return true;
  }
}

bool toSQL::deleteSQL(const QString &name,
		      const QString &version)
{
  allocCheck();
  sqlMap::iterator i=Definitions->find(name);
  if (i==Definitions->end()) {
    return false;
  } else {
    list<versions> &cl=(*i).second.Versions;
    for (list<versions>::iterator j=cl.begin();j!=cl.end();j++) {
      if ((*j).Version==version) {
	cl.erase(j);
	if (cl.begin()==cl.end())
	  Definitions->erase(i);
	return true;
      } else if ((*j).Version>version) {
	return false;
      }
    }
    return false;
  }
}

const QString &toSQL::sql(const QString &name,
			  const toConnection &conn)
{
  allocCheck();
  const QString &version=conn.version();
  sqlMap::iterator i=Definitions->find(name);
  if (i!=Definitions->end()) {
    QString *sql=NULL;
    list<versions> &cl=(*i).second.Versions;
    for (list<versions>::iterator j=cl.begin();j!=cl.end();j++) {
      if ((*j).Version<=version||!sql) {
	sql=&(*j).SQL;
      }
      if ((*j).Version>=version)
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

