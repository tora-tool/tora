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



#include <stdio.h>
#include <ctype.h>

#include "toresultfield.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"

toResultField::toResultField(toConnection &conn,QWidget *parent,const char *name)
  : toHighlightedText(parent,name), Connection(conn)
{
  setReadOnly(true);
}

QString toResultField::query(const QString &sql,const QString *Param1,const QString *Param2,const QString *Param3)
{
  SQL=sql;

  try {
    otl_stream Query;

    int MaxColNum=toTool::globalConfig(CONF_MAX_COL_NUM,DEFAULT_MAX_COL_NUM).toInt();
    int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();

    for (int i=0;i<MaxColNum;i++)
      Query.set_column_type(i+1,otl_var_char,MaxColSize);

    Query.open(1,
	       (const char *)sql,
	       Connection.connection());

    if (Param1)
      Query<<(const char *)(*Param1);
    if (Param2)
      Query<<(const char *)(*Param2);
    if (Param3)
      Query<<(const char *)(*Param3);

    QString text;

    while(!Query.eof()) {
      char buffer[MaxColSize+1];
      buffer[MaxColSize]=0;
      Query>>buffer;
      text.append(buffer);
    }
    setText(text);
    return "";
  } catch (const QString &str) {
    toStatusMessage((const char *)str);
    return str;
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
    return QString((const char *)exc.msg);
  }
}
