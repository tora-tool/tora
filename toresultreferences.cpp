//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
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
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tonoblockquery.h"
#include "toresultreferences.h"
#include "tosql.h"
#include "totool.h"

#include "toresultreferences.moc"

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

  Query=NULL;
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
}

toResultReferences::~toResultReferences()
{
  delete Query;
}

static toSQL SQLConsColumns("toResultReferences:ForeignColumns",
			    "SELECT Column_Name FROM sys.All_Cons_Columns\n"
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
			    "  FROM sys.all_constraints a\n"
			    " WHERE constraint_type = 'R'\n"
			    "   AND (r_owner,r_constraint_name) IN (SELECT b.owner,b.constraint_name\n"
			    "                                         FROM sys.all_constraints b\n"
			    "                                        WHERE b.OWNER = :owner<char[101]>\n"
			    "                                          AND b.TABLE_NAME = :tab<char[101]>)\n"
			    " ORDER BY Constraint_Name",
			    "List the references from foreign constraints to specified table, must return same columns");
static toSQL SQLDependencies("toResultReferences:Dependencies",
			     "SELECT owner,name,type||' '||dependency_type\n"
			     "  FROM sys.all_dependencies\n"
			     " WHERE referenced_owner = :owner<char[101]>\n"
			     "   AND referenced_name = :tab<char[101]>\n"
			     " ORDER BY owner,type,name",
			     "List the dependencies from other objects to this object, must return same number of columns",
			     "8.0");
static toSQL SQLDependencies7("toResultReferences:Dependencies",
			      "SELECT owner,name,type\n"
			      "  FROM sys.all_dependencies\n"
			      " WHERE referenced_owner = :owner<char[101]>\n"
			      "   AND referenced_name = :tab<char[101]>\n"
			      " ORDER BY owner,type,name",
			      QString::null,
			      "7.3");

void toResultReferences::query(const QString &sql,const toQList &param)
{
  if (!handled())
    return;

  toQList::iterator cp=((toQList &)param).begin();
  if (cp!=((toQList &)param).end())
    Owner=*cp;
  cp++;
  if (cp!=((toQList &)param).end())
    TableName=(*cp);

  if (Query) {
    delete Query;
    Query=NULL;
  }

  clear();

  try {
    toQList par;
    par.insert(par.end(),Owner);
    par.insert(par.end(),TableName);
    Query=new toNoBlockQuery(connection(),toQuery::Background,
			     toSQL::string(SQLConstraints,connection()),par);
    Dependencies=false;
    LastItem=NULL;
    Poll.start(100);
  } TOCATCH
}

void toResultReferences::poll(void)
{
  try {
    if (!toCheckModal(this))
      return;
    if (Query&&Query->poll()) {
      if (!Dependencies) {
	while(Query->poll()&&!Query->eof()) {
	  LastItem=new toResultViewItem(this,LastItem,NULL);
	
	  QString consOwner(Query->readValue());
	  LastItem->setText(1,Query->readValue());
	  QString consName(Query->readValue());
	  QString colNames(constraintCols(Owner,consName));
	  LastItem->setText(0,consOwner);
	  LastItem->setText(2,consName);
	  QString rConsOwner(Query->readValue());
	  QString rConsName(Query->readValue());
	  LastItem->setText(4,Query->readValue());
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

	  LastItem->setText(3,Condition);
	  LastItem->setText(5,Query->readValue());
	}
	if (Query->eof()) {
	  delete Query;
	  Query=NULL;
	  toQList par;
	  par.insert(par.end(),Owner);
	  par.insert(par.end(),TableName);
	  Query=new toNoBlockQuery(connection(),toQuery::Background,
				   toSQL::string(SQLDependencies,connection()),
				   par);
	  Dependencies=true;
	}
      } else {
	while(Query->poll()&&!Query->eof()) {
	  LastItem=new QListViewItem(this,LastItem,NULL);
	  LastItem->setText(0,Query->readValue());
	  LastItem->setText(1,Query->readValue());
	  LastItem->setText(3,Query->readValue());
	  LastItem->setText(4,"DEPENDENCY");
	}
	if (Query->eof()) {
	  delete Query;
	  Query=NULL;
	  Poll.stop();
	}
      }
    }
  } catch(const QString &exc) {
    delete Query;
    Query=NULL;
    Poll.stop();
    toStatusMessage(exc);
  }
}
