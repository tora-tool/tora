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


#ifndef __TOSQL_H
#define __TOSQL_H

#include <map>
#include <list>
#include <qstring.h>

class toConnection;

class toSQL {
public:
  struct version {
    QString Version;
    QString SQL;
    bool Modified;
    version(const QString &ver,const QString &sql,bool modified=false)
      : Version(ver),SQL(sql),Modified(modified)
    { }
  };

  struct definition {
    QString Description;
    bool Modified;
    list<version> Versions;
  };

  typedef map<QString,definition> sqlMap;
private:
  static sqlMap *Definitions;

  QString Name;

  toSQL(const toSQL &)
  { }

  static void allocCheck(void)
  { if (!Definitions) Definitions=new sqlMap; }

  static QString expandFile(const QString &file);
public:
  static bool updateSQL(const QString &name,
		        const QString &sql,
		        const QString &description,
		        const QString &ver="8.1.5",
			bool modified=true);

  static bool deleteSQL(const QString &name,
		        const QString &ver);

  static const QString &sql(const QString &name,const toConnection &conn);

  static const QString &sql(const toSQL &sqldef,const toConnection &conn)
  { return sql(sqldef.Name,conn); }

  static bool saveSQL(const QString &file);
  static bool loadSQL(const QString &file);

  static const sqlMap &definitions(void)
  { allocCheck(); return *Definitions; }

  const QString &operator () (const toConnection &conn)
  { return sql(Name,conn); }

  const QString &name(void)
  { return Name; }

  toSQL(const QString &name,
	const QString &sql,
	const QString &description=QString::null,
	const QString &ver="8.1.5")
    : Name(name)
  { updateSQL(name,sql,description,ver,false); }
};

#endif
