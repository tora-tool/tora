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

#include "toresultreferences.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"
#include "tosql.h"
#include "toconnection.h"

toResultReferences::toResultReferences(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
{
  setReadAll(true);
  addColumn("Owner");
  addColumn("Object");
  addColumn("Constraint");
  addColumn("Condition");
  addColumn("Enabled");
  addColumn("Delete Rule");
  setSQLName("toResultReferences");
}

static toSQL SQLConsColumns("toResultReferences:ForeignColumns",
			    "SELECT Column_Name FROM All_Cons_Columns\n"
			    " WHERE Owner = :f1<char[101]> AND Constraint_Name = :f2<char[101]>\n"
			    " ORDER BY Position",
			    "Get columns of foreign constraint, must return same number of cols");

QString toResultReferences::constraintCols(const QString &conOwner,const QString &conName)
{
  toQuery query(connection(),SQLConsColumns,conOwner,conName);

  QString ret;
  while(!query.eof()) {
    QString value=query.readValue();
    if (!ret.isEmpty())
      ret.append(",");
    ret.append(value);
  }
  return ret;
}

static toSQL SQLConstraints("toResultReferences:References",
			    "SELECT Owner,\n"
			    "       Table_Name,\n"
			    "       Constraint_Name,\n"
			    "       R_Owner,\n"
			    "       R_Constraint_Name,\n"
			    "       Status,\n"
			    "       Delete_Rule\n"
			    "  FROM all_constraints a\n"
			    " WHERE constraint_type = 'R'\n"
			    "   AND (r_owner,r_constraint_name) IN (SELECT b.owner,b.constraint_name\n"
			    "                                         FROM all_constraints b\n"
			    "                                        WHERE b.OWNER = :owner<char[101]>\n"
			    "                                          AND b.TABLE_NAME = :tab<char[101]>)\n"
			    " ORDER BY Constraint_Name",
			    "List the references from foreign constraints to specified table, must return same columns");
static toSQL SQLDependencies("toResultReferences:Dependencies",
			     "SELECT owner,name,type||' '||dependency_type\n"
			     "  FROM dba_dependencies\n"
			     " WHERE referenced_owner = :owner<char[101]>\n"
			     "   AND referenced_name = :tab<char[101]>\n"
			     " ORDER BY owner,type,name",
			     "List the dependencies from other objects to this object, must return same number of columns");

void toResultReferences::query(const QString &sql,const toQList &param)
{
  QString Owner;
  QString TableName;
  toQList::iterator cp=((toQList &)param).begin();
  if (cp!=((toQList &)param).end())
    Owner=*cp;
  cp++;
  if (cp!=((toQList &)param).end())
    TableName=(*cp);

  Query=NULL;
  RowNumber=0;

  clear();

  try {
    toQuery query(connection(),SQLConstraints,Owner,TableName);

    QListViewItem *item=NULL;
    while(!query.eof()) {
      item=new QListViewItem(this,item,NULL);

      QString consOwner(query.readValue());
      item->setText(1,query.readValue());
      QString consName(query.readValue());
      QString colNames(constraintCols(Owner,consName));
      item->setText(0,consOwner);
      item->setText(2,consName);
      QString rConsOwner(query.readValue());
      QString rConsName(query.readValue());
      item->setText(4,query.readValue());
      QString Condition;

      Condition="foreign key (";
      Condition.append(colNames);
      Condition.append(") references ");
      Condition.append(rConsOwner);
      Condition.append(".");
      QString cols(constraintCols(rConsOwner,rConsName));
      
      Condition.append(TableName);
      Condition.append("(");
      Condition.append(cols);
      Condition.append(")");

      item->setText(3,Condition);
      item->setText(5,query.readValue());
    }
    
    toQuery deps(connection(),SQLDependencies,Owner,TableName);
    while(!deps.eof()) {
      item=new QListViewItem(this,item,NULL);
      item->setText(0,deps.readValue());
      item->setText(1,deps.readValue());
      item->setText(3,deps.readValue());
      item->setText(4,"DEPENDENCY");
    }
  } TOCATCH
  updateContents();
}
