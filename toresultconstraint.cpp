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
 *      software in the executable aside from Oracle client libraries.
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

#include "toresultconstraint.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"
#include "tosql.h"

toResultConstraint::toResultConstraint(toConnection &conn,QWidget *parent,const char *name)
  : toResultView(false,false,conn,parent,name)
{
  setReadAll(true);
  addColumn("Constraint Name");
  addColumn("Condition");
  addColumn("Enabled");
  addColumn("Delete Rule");
  addColumn("Generated");
  setSQLName("toResultConstraint");
}

static toSQL SQLConsColumns("toResultConstraint:ForeignColumns",
			    "SELECT Column_Name,Table_Name FROM All_Cons_Columns\n"
			    " WHERE Owner = :f1<char[31]> AND Constraint_Name = :f2<char[31]>\n"
			    " ORDER BY Position",
			    "Get columns of foreign constraint, must return same number of cols");

QString toResultConstraint::constraintCols(const QString &conOwner,const QString &conName)
{
  otl_stream Query(1,
		   SQLConsColumns(Connection),
		   Connection.connection());

  Query<<conOwner.utf8();
  Query<<conName.utf8();

  QString ret;
  while(!Query.eof()) {
    char buffer[31];
    Query>>buffer;
    if (!ret.isEmpty())
      ret.append(",");
    ret.append(QString::fromUtf8(buffer));
    Query>>buffer;
    LastTable=QString::fromUtf8(buffer);
  }
  return ret;
}

static toSQL SQLConstraints("toResultConstraint:ListConstraints",
			    "SELECT Constraint_Name,\n"
			    "       Search_Condition,\n"
			    "       R_Owner,\n"
			    "       R_Constraint_Name,\n"
			    "       Status,\n"
			    "       Constraint_Type,\n"
			    "       Delete_Rule,\n"
			    "       Generated\n"
			    "  FROM All_Constraints\n"
			    " WHERE Owner = :f1<char[31]>\n"
			    "   AND Table_Name = :f2<char[31]>\n"
			    " ORDER BY Constraint_Name",
			    "List constraints on a table. Must have same column order");

void toResultConstraint::query(const QString &sql,const list<QString> &param)
{
  QString Owner;
  QString TableName;
  list<QString>::iterator cp=((list<QString> &)param).begin();
  if (cp!=((list<QString> &)param).end())
    Owner=*cp;
  cp++;
  if (cp!=((list<QString> &)param).end())
    TableName=(*cp);

  Query=NULL;
  LastItem=NULL;
  RowNumber=0;

  clear();

  try {
    otl_stream Query;

    int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();

    Query.set_column_type(2,otl_var_char,MaxColSize);
    Query.open(1,
	       SQLConstraints(Connection),
	       Connection.connection());

    Description=Query.describe_select(DescriptionLen);

    Query<<Owner.utf8();
    Query<<TableName.utf8();

    QListViewItem *item;
    while(!Query.eof()) {
      item=new QListViewItem(this,LastItem,NULL);
      LastItem=item;

      char *buffer=new char[MaxColSize+1];
      try {
        buffer[MaxColSize]=0;
        Query>>buffer;
        QString consName(QString::fromUtf8(buffer));
        QString colNames(constraintCols(Owner,QString::fromUtf8(buffer)));
        item->setText(0,consName);
        Query>>buffer;
        QString Check(QString::fromUtf8(buffer));
        Query>>buffer;
        QString rConsOwner(QString::fromUtf8(buffer));
        Query>>buffer;
        QString rConsName(QString::fromUtf8(buffer));
        Query>>buffer;
        item->setText(2,QString::fromUtf8(buffer));
        Query>>buffer;
        QString Condition;
        switch(buffer[0]) {
        case 'U':
	  Condition="unique (";
	  Condition.append(colNames);
	  Condition.append(")");
	  break;
        case 'P':
	  Condition="primary key (";
	  Condition.append(colNames);
	  Condition.append(")");
	  break;
        case 'C':
        case 'V':
        case 'O':
	  Condition="check (";
	  Condition.append(Check);
	  Condition.append(")");
	  break;
        case 'R':
	  Condition="foreign key (";
	  Condition.append(colNames);
	  Condition.append(") references ");
	  Condition.append(rConsOwner);
	  Condition.append(".");
	  QString cols(constraintCols(rConsOwner,rConsName));

	  Condition.append(LastTable);
	  Condition.append("(");
	  Condition.append(cols);
	  Condition.append(")");
	  break;
	}
        item->setText(1,Condition);
        Query>>buffer;
        item->setText(3,QString::fromUtf8(buffer));
        Query>>buffer;
        item->setText(4,QString::fromUtf8(buffer));
      } catch (...) {
	delete buffer;
	throw;
      }
      delete buffer;
    }
  } TOCATCH
  updateContents();
}
