/*****
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003-2005 Quest Software, Inc
 * Portions Copyright (C) 2005 Other Contributors
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
 *      You may link this product with any GPL'd Qt library such as Qt/Free
 *
 * All trademarks belong to their respective owners.
 *
 *****/

#include "utils.h"

#include "tobrowserconstraint.h"
#include "toconnection.h"
#include "toresultcols.h"
#include "toextract.h"
#include "tomemoeditor.h"
#include "tosql.h"
#include "totableselect.h"

#include <qapplication.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qprogressdialog.h>
#include <qradiobutton.h>
#include <qtimer.h>
#include <qwidgetstack.h>

#include "tobrowserconstraint.moc"
#include "tobrowserconstraintui.moc"

void toBrowserConstraint::modifyConstraint(toConnection &conn,const QString &owner,const QString &table,QWidget *parent)
{
  toBrowserConstraint dialog(conn,owner,table,parent);
  if (dialog.exec()) {
    dialog.changeConstraint();
    std::list<toSQLParse::statement> statements=toSQLParse::parse(dialog.sql(),conn);
    try {
      QProgressDialog prog(tr("Performing constraint changes"),
			   tr("Executing constraint change script"),
			   statements.size(),
			   &dialog,"progress",true);
      prog.setCaption(tr("Performing constraint changes"));
      for(std::list<toSQLParse::statement>::iterator i=statements.begin();i!=statements.end();i++) {
	QString sql=toSQLParse::indentStatement(*i,conn);
	int i=sql.length()-1;
	while(i>=0&&(sql.at(i)==';'||sql.at(i).isSpace()))
	  i--;
	if (i>=0)
	  conn.execute(sql.mid(0,i+1));
	qApp->processEvents();
	if (prog.wasCancelled())
	  throw tr("Cancelled ongoing constraint modification, constraints might be corrupt");
      }
    } TOCATCH
  }
}

toBrowserConstraint::toBrowserConstraint(toConnection &conn,const QString &owner,const QString &table,QWidget *parent,const char *name)
  : toBrowserConstraintUI(parent,name,true),toConnectionWidget(conn,this),Extractor(conn,NULL)
{
  Extractor.setIndexes(false);
  Extractor.setConstraints(true);
  Extractor.setPrompt(false);
  Extractor.setHeading(false);

  if (toIsMySQL(connection()))
    UniqueType->hide();

  SourceColList->displayHeader(false);
  UniqueColList->displayHeader(false);
  CheckColumn->displayHeader(false);
  ReferColList->displayHeader(false);

  if (!owner.isEmpty()&&!table.isEmpty()) {
    QString fulltable=conn.quote(owner)+"."+conn.quote(table);
    TableSelect->setTable(fulltable);
    describeTable(fulltable);
  } else if (!owner.isEmpty())
    TableSelect->setTable(conn.quote(owner));

  connect(TableSelect,SIGNAL(selectTable(const QString &)),this,SLOT(changeTable(const QString &)));
}

void toBrowserConstraint::describeTable(const QString &table)
{
  try {
    QStringList parts=QStringList::split(".",table);
    if (parts.size()>1) {
      Owner=connection().unQuote(parts[0]);
      Table=connection().unQuote(parts[1]);
    } else {
      Table=connection().unQuote(table);
      if (toIsOracle(connection()))
	Owner=connection().user();
      else
	Owner=connection().database();
    }

    Constraints.clear();
    Current=QString::null;

    if (!Table.isEmpty()) {
      if (OriginalDescription[table].begin()==OriginalDescription[table].end()) {
	std::list<QString> Objects;
	Objects.insert(Objects.end(),"TABLE:"+Owner+"."+Table);
	Objects.insert(Objects.end(),"TABLE REFERENCES:"+Owner+"."+Table);

	NewDescription[table]=Extractor.describe(Objects);
      }
      std::list<QString> &origDescription=NewDescription[table];

      for(std::list<QString>::iterator i=origDescription.begin();i!=origDescription.end();) {
	std::list<QString> ctx=toExtract::splitDescribe(*i);
	toShift(ctx);
	toShift(ctx);
	toShift(ctx);
	if (toShift(ctx)=="CONSTRAINT") {
	  QString name=toShift(ctx);
	  if (toShift(ctx)=="DEFINITION")
	    Constraints[name]=toShift(ctx);
	  i++;
	} else
	  i=origDescription.erase(i);		// Remove everything that isn't constraints
      }
      if (OriginalDescription[table].begin()==OriginalDescription[table].end())
	OriginalDescription[table]=NewDescription[table];
    }
    Name->clear();
    for(std::map<QString,QString>::iterator i=Constraints.begin();i!=Constraints.end();i++)
      Name->insertItem((*i).first);

    SourceColList->changeParams(Owner,Table);
    UniqueColList->changeParams(Owner,Table);
    CheckColumn->changeParams(Owner,Table);

    ReferTable->clear();
    std::list<toConnection::objectName> objects=connection().objects(false);
    {
      for(std::list<toConnection::objectName>::iterator i=objects.begin();i!=objects.end();i++) {
	if ((*i).Type=="TABLE"&&(*i).Owner==Owner)
	  ReferTable->insertItem((*i).Name);
      }
    }

    changeConstraint();
  } catch(const QString &exc) {
    toStatusMessage(exc);
    reject();
  }
}

QString toBrowserConstraint::sql()
{
  saveChanges();

  std::list<QString> origTable;
  std::list<QString> migrateTable;

  {
    for(std::map<QString,std::list<QString> >::iterator i=OriginalDescription.begin();i!=OriginalDescription.end();i++) {
      std::list<QString> &cur=(*i).second;
      for(std::list<QString>::iterator j=cur.begin();j!=cur.end();j++)
	toPush(origTable,*j);
    }
  }
  {
    for(std::map<QString,std::list<QString> >::iterator i=NewDescription.begin();i!=NewDescription.end();i++) {
      std::list<QString> &cur=(*i).second;
      for(std::list<QString>::iterator j=cur.begin();j!=cur.end();j++)
	toPush(migrateTable,*j);
    }
  }

  origTable.sort();
  migrateTable.sort();

  return Extractor.migrate(origTable,migrateTable);
}

QString toBrowserConstraint::table(void)
{
  if (Owner.isEmpty()&&Table.isEmpty())
    return QString::null;
  return connection().quote(Owner)+"."+connection().quote(Table);
}

void toBrowserConstraint::saveChanges(void)
{
  if (table().isEmpty())
    return;

  changeConstraint();
  std::list<QString> migrateTable;

  std::list<QString> ctx;
  toPush(ctx,Owner);
  toPush(ctx,QString("TABLE"));
  toPush(ctx,Table);
  toPush(ctx,QString("CONSTRAINT"));

  for(std::map<QString,QString>::iterator i=Constraints.begin();i!=Constraints.end();i++)
    toExtract::addDescription(migrateTable,ctx,(*i).first,"DEFINITION",(*i).second);

  migrateTable.sort();
  NewDescription[table()]=migrateTable;
}

void toBrowserConstraint::displaySQL()
{
  toMemoEditor memo(this,sql(),-1,-1,true,true);
  memo.exec();
}

void toBrowserConstraint::addConstraint()
{
  if (Table.isEmpty())
    return;
  bool ok=false;
  QString name=QInputDialog::getText(tr("Enter new constraint name"),
				     tr("Enter name of new constraint."),
				     QLineEdit::Normal,QString::null,&ok,this);
  if (ok) {
    Name->insertItem(name);
    Name->setCurrentItem(Name->count()-1);
    changeConstraint();
  }
}

void toBrowserConstraint::delConstraint()
{
  if (Name->count()>0) {
    Constraints.erase(Current);
    Current=QString::null;
    Name->removeItem(Name->currentItem());
    changeConstraint();
  }
}

void toBrowserConstraint::parseConstraint(const QString &definition)
{
  if (definition.isEmpty()) {
    Type->setEnabled(true);
    SourceColumns->clear();
    ReferedColumns->clear();
    UniqueCols->clear();
    CheckCondition->clear();
    return;
  }

  toSQLParse::statement statement=toSQLParse::parseStatement(definition,connection());
  std::list<toSQLParse::statement>::iterator i=statement.subTokens().begin();
  if (i==statement.subTokens().end())
    return;
  Type->setEnabled(false);

  QString type=(*i).String.upper();
  i++;
  if (type=="FOREIGN") {
    WidgetStack->raiseWidget(ReferentialPage);
    Type->setCurrentItem(0);
    if (i==statement.subTokens().end())
      return;

    while(i!=statement.subTokens().end()&&(*i).Type!=toSQLParse::statement::List)
      i++;
    if (i!=statement.subTokens().end())
      SourceColumns->setText(toSQLParse::indentStatement(*i,connection()));
    else
      return;

    i++;		// Ending )
    if (i==statement.subTokens().end())
      return;
    i++;		// REFERENCES
    if (i==statement.subTokens().end())
      return;
    i++;		// Refered table finally
    if (i==statement.subTokens().end())
      return;
    ReferTable->setCurrentText((*i).String);
    changeRefered();
    while(i!=statement.subTokens().end()&&(*i).Type!=toSQLParse::statement::List)
      i++;
    if (i!=statement.subTokens().end())
      ReferedColumns->setText(toSQLParse::indentStatement(*i,connection()));
  } else if (type=="PRIMARY"||type=="UNIQUE") {
    WidgetStack->raiseWidget(PrimaryPage);
    Type->setCurrentItem(2);
    Primary->setChecked(type=="PRIMARY");
    Unique->setChecked(type=="UNIQUE");
    while(i!=statement.subTokens().end()&&(*i).Type!=toSQLParse::statement::List)
      i++;
    if (i!=statement.subTokens().end())
      UniqueCols->setText(toSQLParse::indentStatement(*i,connection()));
  } else if (type=="CHECK") {
    WidgetStack->raiseWidget(CheckPage);
    Type->setCurrentItem(1);
    while(i!=statement.subTokens().end()&&(*i).Type!=toSQLParse::statement::List)
      i++;
    if (i!=statement.subTokens().end())
      CheckCondition->setText(toSQLParse::indentStatement(*i,connection()));
  }
}

void toBrowserConstraint::changeConstraint()
{
  if (!Current.isEmpty()) {
    QString newDefinition;
    if (WidgetStack->visibleWidget()==ReferentialPage) {
      QString table=ReferTable->currentText();
      QString sourceCols=SourceColumns->text();
      QString destinCols=ReferedColumns->text();
      if (!table.isEmpty()&&!sourceCols.isEmpty()&&!destinCols.isEmpty())
	newDefinition="FOREIGN KEY ("+sourceCols+") REFERENCES "+table+" ("+destinCols+")";
    } else if (WidgetStack->visibleWidget()==PrimaryPage) {
      QString columns=UniqueCols->text();
      if (!columns.isEmpty())
	newDefinition=(Primary->isChecked()?"PRIMARY KEY (":"UNIQUE (")+columns+")";
    } else if (WidgetStack->visibleWidget()==CheckPage) {
      QString checkCondition=CheckCondition->text();
      if (!checkCondition.isEmpty())
	newDefinition="CHECK ("+checkCondition+")";
    }
    if (!newDefinition.isEmpty()&&!Current.isEmpty())
      Constraints[Current]=newDefinition;
  }
  Current=Name->currentText();
  if (Current.isEmpty()) {
    parseConstraint(QString::null);
    Type->setEnabled(!Current.isEmpty());
  } else
    parseConstraint(Constraints[Current]);
  WidgetStack->setEnabled(!Current.isEmpty());
}

void toBrowserConstraint::changeRefered(void)
{
  QStringList parts=QStringList::split(".",ReferTable->currentText());
  if (parts.size()>1)
    ReferColList->changeParams(parts[0],parts[1]);
  else
    ReferColList->changeParams(ReferTable->currentText());
}

void toBrowserConstraint::changeType(int type)
{
  switch(type) {
  case 0:
    WidgetStack->raiseWidget(ReferentialPage);
    break;
  case 1:
    WidgetStack->raiseWidget(CheckPage);
    break;
  case 2:
    WidgetStack->raiseWidget(PrimaryPage);
    break;
  }
}

void toBrowserConstraint::changeTable(const QString &table)
{
  saveChanges();
  describeTable(table);
}
