//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003 Quest Software, Inc
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "toextract.h"
#include "tonoblockquery.h"
#include "toresultconstraint.h"
#include "tosql.h"
#include "totool.h"

#include "toresultconstraint.moc"

toResultConstraint::toResultConstraint(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
{
  setReadAll(true);
  addColumn(tr("Constraint Name"));
  addColumn(tr("Condition"));
#ifndef TOAD
  addColumn(tr("Enabled"));
  addColumn(tr("Delete Rule"));
  addColumn(tr("Generated"));
#endif
  setSQLName(QString::fromLatin1("toResultConstraint"));

  setSorting(0);

  Query=NULL;
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
}

bool toResultConstraint::canHandle(toConnection &conn)
{
  return toIsOracle(conn)||toExtract::canHandle(conn);
}

static toSQL SQLConsColumns("toResultConstraint:ForeignColumns",
			    "SELECT Column_Name,Table_Name FROM sys.All_Cons_Columns\n"
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
      ret.append(QString::fromLatin1(","));
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
			    "  FROM sys.All_Constraints\n"
			    " WHERE Owner = :f1<char[101]>\n"
			    "   AND Table_Name = :f2<char[101]>\n"
			    " ORDER BY Constraint_Name",
			    "List constraints on a table. Must have same column order",
			    "8.0");
static toSQL SQLConstraints7("toResultConstraint:ListConstraints",
			     "SELECT Constraint_Name,\n"
			     "       Search_Condition,\n"
			     "       R_Owner,\n"
			     "       R_Constraint_Name,\n"
			     "       Status,\n"
			     "       Constraint_Type,\n"
			     "       Delete_Rule,\n"
			     "       'N/A'\n"
			     "  FROM sys.All_Constraints\n"
			     " WHERE Owner = :f1<char[101]>\n"
			     "   AND Table_Name = :f2<char[101]>\n"
			     " ORDER BY Constraint_Name",
			     "",
			     "7.3");

void toResultConstraint::addConstraint(const QString &name,const QString &definition,const QString &status)
{
  QListViewItem *item=new toResultViewItem(this,NULL);
  item->setText(0,name);
  item->setText(1,definition);
  item->setText(2,status);
}

void toResultConstraint::query(const QString &sql,const toQList &param)
{
  if (!handled())
    return;

  toQList::iterator cp=((toQList &)param).begin();
  if (cp!=((toQList &)param).end())
    Owner=*cp;
  cp++;
  if (cp!=((toQList &)param).end())
    TableName=(*cp);

  if (!setSQLParams(sql,param))
    return;

  clear();

  if (toIsOracle(connection())) {
    if (Query)
      delete Query;
    Query=NULL;

    try {
      toQList par;
      par.insert(par.end(),Owner);
      par.insert(par.end(),TableName);
      Query=new toNoBlockQuery(connection(),toQuery::Background,
			       toSQL::string(SQLConstraints,connection()),
			       par);
      Poll.start(100);
    } TOCATCH
  } else {
    std::list<QString> objects;
    objects.insert(objects.end(),"TABLE FAMILY:"+Owner+"."+TableName);
    objects.insert(objects.end(),"TABLE REFERENCES:"+Owner+"."+TableName);

    toExtract extract(connection(),NULL);
    extract.setCode(true);
    extract.setHeading(false);
    extract.setConstraints(true);
    std::list<QString> dsc=extract.describe(objects);

    QString name;
    QString definition;
    QString status;
    
    for(std::list<QString>::iterator i=dsc.begin();i!=dsc.end();i++) {
      std::list<QString> ctx=toExtract::splitDescribe(*i);
      toShift(ctx);	// Schema;
      toShift(ctx);	// Table;
      toShift(ctx);	// Name;
      if (toShift(ctx)=="CONSTRAINT") {
	QString nname=toShift(ctx);
	if (nname!=name&&!name.isEmpty())
	  addConstraint(name,definition,status);
	name=nname;
	QString extra=toShift(ctx);
	if (extra=="DEFINITION")
	  definition=toShift(ctx);
	else if (extra=="STATUS")
	  status=toShift(ctx);
      }
    }
    if (!name.isEmpty())
      addConstraint(name,definition,status);
  }
}

toResultConstraint::~toResultConstraint()
{
  delete Query;
}

void toResultConstraint::poll()
{
  try {
    if (Query&&Query->poll()) {
      while(Query->poll()&&!Query->eof()) {
	QListViewItem *item=new toResultViewItem(this,NULL);
      
	QString consName=Query->readValue();
	QString check=Query->readValue();
	QString colNames=constraintCols(Owner,consName);
	item->setText(0,consName);
	QString rConsOwner=Query->readValue();
	QString rConsName=Query->readValue();
	item->setText(2,Query->readValue());
	QString type=Query->readValue();
	QString Condition;
	char t=(type.latin1())[0];
	switch(t) {
	case 'U':
	  Condition=QString::fromLatin1("unique (");
	  Condition.append(colNames);
	  Condition.append(QString::fromLatin1(")"));
	  break;
	case 'P':
	  Condition=QString::fromLatin1("primary key (");
	  Condition.append(colNames);
	  Condition.append(QString::fromLatin1(")"));
	  break;
	case 'C':
	case 'V':
	case 'O':
	  Condition=QString::fromLatin1("check (");
	  Condition.append(check);
	  Condition.append(QString::fromLatin1(")"));
	  break;
	case 'R':
	  Condition=QString::fromLatin1("foreign key (");
	  Condition.append(colNames);
	  Condition.append(QString::fromLatin1(") references "));
	  Condition.append(rConsOwner);
	  Condition.append(QString::fromLatin1("."));
	  QString cols(constraintCols(rConsOwner,rConsName));
	
	  Condition.append(LastTable);
	  Condition.append(QString::fromLatin1("("));
	  Condition.append(cols);
	  Condition.append(QString::fromLatin1(")"));
	  break;
	}
	item->setText(1,Condition);
	item->setText(3,Query->readValueNull());
	item->setText(4,Query->readValue());
      }
      if (Query->eof()) {
	delete Query;
	Query=NULL;
	Poll.stop();
      }
    }
  } catch(const QString &exc) {
    delete Query;
    Query=NULL;
    Poll.stop();
    toStatusMessage(exc);
  }
}
