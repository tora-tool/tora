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

#include "toresultconstraint.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"
#include "tosql.h"
#include "toconnection.h"

toResultConstraint::toResultConstraint(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
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
			    " WHERE Owner = :f1<char[101]> AND Constraint_Name = :f2<char[101]>\n"
			    " ORDER BY Position",
			    "Get columns of foreign constraint, must return same number of cols");

QString toResultConstraint::constraintCols(const QString &conOwner,const QString &conName)
{
  toQuery query(connection(),SQLConsColumns,conOwner,conName);

  QString ret;
  while(!query.eof()) {
    QString value=query.readValue();
    if (!ret.isEmpty())
      ret.append(",");
    ret.append(value);
    LastTable=query.readValue();
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
			    " WHERE Owner = :f1<char[101]>\n"
			    "   AND Table_Name = :f2<char[101]>\n"
			    " ORDER BY Constraint_Name",
			    "List constraints on a table. Must have same column order");

void toResultConstraint::query(const QString &sql,const toQList &param)
{
  if (!handled())
    return;
  QString Owner;
  QString TableName;
  toQList::iterator cp=((toQList &)param).begin();
  if (cp!=((toQList &)param).end())
    Owner=*cp;
  cp++;
  if (cp!=((toQList &)param).end())
    TableName=(*cp);

  clear();

  try {
    QListViewItem *item=NULL;

    toQuery query(connection(),SQLConstraints,Owner,TableName);

    while(!query.eof()) {
      item=new QListViewItem(this,item,NULL);

      QString consName=query.readValue();
      QString check=query.readValue();
      QString colNames=constraintCols(Owner,consName);
      item->setText(0,consName);
      QString rConsOwner=query.readValue();
      QString rConsName=query.readValue();
      item->setText(2,query.readValue());
      QString type=query.readValue();
      QString Condition;
      char t=((const char *)type)[0];
      switch(t) {
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
	Condition.append(check);
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
      item->setText(3,query.readValueNull());
      item->setText(4,query.readValue());
    }
  } TOCATCH
  updateContents();
}

