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


#include "toresultindexes.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"

toResultIndexes::toResultIndexes(toConnection &conn,QWidget *parent,const char *name=NULL)
  : toResultView(false,false,conn,parent,name)
{
  setReadAll(true);
  addColumn("Index Name");
  addColumn("Columns");
  addColumn("Type");
  addColumn("Unique");
}

QString toResultIndexes::indexCols(const QString &indOwner,const QString &indName)
{
  otl_stream Query(1,
		   "SELECT Column_Name FROM All_Ind_Columns"
		   " WHERE Index_Owner = :f1<char[31]> AND Index_Name = :f2<char[31]>"
		   " ORDER BY Column_Position",
		   Connection.connection());

  Query<<(const char *)indOwner;
  Query<<(const char *)indName;

  QString ret;
  while(!Query.eof()) {
    char buffer[31];
    Query>>buffer;
    if (!ret.isEmpty())
      ret.append(",");
    ret.append(buffer);
  }
  return ret;
}

QString toResultIndexes::query(const QString &sql,const list<QString> &param)
{
  QString Owner;
  QString TableName;
  list<QString>::iterator cp=((list<QString> &)param).begin();
  if (cp!=((list<QString> &)param).end())
    Owner=*cp;
  cp++;
  if (cp!=((list<QString> &)param).end())
    TableName=(*cp);

  LastItem=NULL;
  RowNumber=0;

  clear();

  try {
    otl_stream Query(1,
		     "SELECT Owner,"
		     "       Index_Name,"
		     "       Index_Type,"
		     "       Uniqueness"
		     "  FROM All_Indexes"
		     " WHERE Table_Owner = :f1<char[31]>"
		     "   AND Table_Name = :f2<char[31]>"
		     " ORDER BY Index_Name",
		     Connection.connection());

    Description=Query.describe_select(DescriptionLen);

    Query<<Owner;
    Query<<TableName;

    QListViewItem *item;
    while(!Query.eof()) {
      item=new QListViewItem(this,LastItem,NULL);
      LastItem=item;

      char buffer[101];
      buffer[100]=0;
      Query>>buffer;
      QString indexOwner(buffer);
      Query>>buffer;
      item->setText(0,buffer);
      item->setText(1,indexCols(indexOwner,buffer));
      Query>>buffer;
      item->setText(2,buffer);
      Query>>buffer;
      item->setText(3,buffer);
    }
  } catch (const QString &str) {
    toStatusMessage((const char *)str);
    updateContents();
    return str;
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
    updateContents();
    return QString((const char *)exc.msg);
  }
  updateContents();
  return "";
}
