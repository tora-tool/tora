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
#include <ctype.h>

#include "toresultfield.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"

#include "toresultfield.moc"

toResultField::toResultField(toConnection &conn,QWidget *parent,const char *name)
  : toHighlightedText(parent,name), Connection(conn)
{
  setReadOnly(true);
}

void toResultField::query(const QString &sql,const list<QString> &param)
{
  SQL=sql;

  try {
    otl_stream Query;

    int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();

    Query.set_all_column_types(otl_all_num2str|otl_all_date2str);
    Query.open(1,
	       (const char *)sql,
	       Connection.connection());

    for (list<QString>::iterator i=((list<QString> &)param).begin();i!=((list<QString> &)param).end();i++)
      Query<<(const char *)(*i);

    QString text;

    int DescriptionLen,col;
    otl_column_desc *Description=Query.describe_select(DescriptionLen);

    col=0;
    while(!Query.eof()) {
      char buffer[MaxColSize+1];
      buffer[MaxColSize]=0;
      text.append(toReadValue(Description[col],Query,MaxColSize));
      col++;
      col%=DescriptionLen;
    }
    setText(text);
  } catch (const QString &str) {
    toStatusMessage((const char *)str);
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
  }
}
