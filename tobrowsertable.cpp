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

#include "tobrowsertable.h"
#include "toconnection.h"
#include "todatatype.h"
#include "toextract.h"
#include "tohighlightedtext.h"
#include "tomemoeditor.h"
#include "toresultcombo.h"
#include "tosql.h"

#include <qapplication.h>
#include <qcombobox.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qprogressdialog.h>
#include <qscrollview.h>
#include <qtimer.h>

#include "tobrowsertable.moc"
#include "tobrowsertableui.moc"

static toSQL SQLListTablespaces("toBrowserTable:ListTablespaces",
				"SELECT Tablespace_Name FROM sys.DBA_TABLESPACES\n"
				" ORDER BY Tablespace_Name",
				"List the available tablespaces in a database.");

void toBrowserTable::editTable(toConnection &conn,const QString &owner,const QString &table,QWidget *parent)
{
  toBrowserTable dialog(conn,owner,table,parent);
  if (dialog.exec()) {
    std::list<toSQLParse::statement> statements=toSQLParse::parse(dialog.sql(),conn);
    try {
      QProgressDialog prog(tr("Performing table changes"),
			   tr("Executing table change script"),
			   statements.size(),
			   &dialog,"progress",true);
      prog.setCaption(tr("Performing table changes"));
      for(std::list<toSQLParse::statement>::iterator i=statements.begin();i!=statements.end();i++) {
	QString sql=toSQLParse::indentStatement(*i,conn);
	int i=sql.length()-1;
	while(i>=0&&(sql.at(i)==';'||sql.at(i).isSpace()))
	  i--;
	if (i>=0)
	  conn.execute(sql.mid(0,i+1));
	qApp->processEvents();
	if (prog.wasCancelled())
	  throw tr("Cancelled ongoing table modification, table might be corrupt");
      }
    } TOCATCH
  }
}

toBrowserTable::toBrowserTable(toConnection &conn,const QString &owner,const QString &table,QWidget *parent,const char *name)
  : toBrowserTableUI(parent,name,true),toConnectionWidget(conn,this),Extractor(conn,NULL)
{
  ColumnNumber=0;
  ColumnGrid=new QGrid(3,ColumnList->viewport());
  ColumnList->addChild(ColumnGrid);
  ColumnList->viewport()->setBackgroundColor(qApp->palette().active().background());

  new QLabel(tr("Name"),ColumnGrid);
  new QLabel(tr("Datatype"),ColumnGrid);
  new QLabel(tr("Extra parameters"),ColumnGrid);

  Extractor.setIndexes(false);
  Extractor.setConstraints(false);
  Extractor.setPrompt(false);
  Extractor.setHeading(false);

  UglyFlag=false; // Indicates wether the correct size has been retreived at least once.

  try {
    Owner=owner;
    Table=table;

    QString tablespace;
    Schema->query(toSQL::sql(toSQL::TOSQL_USERLIST));
    Schema->setSelected(Owner);

    if (!Table.isEmpty()) {
      std::list<QString> Objects;
      Objects.insert(Objects.end(),"TABLE:"+Owner+"."+Table);

      OriginalDescription=Extractor.describe(Objects);
      Schema->setEnabled(false);

      bool invalid=false;

      Name->setText(table);

      QString declarations;
      QString storage;
      QString parallel;

      for(std::list<QString>::iterator i=OriginalDescription.begin();i!=OriginalDescription.end();i++) {
	std::list<QString> row=toExtract::splitDescribe(*i);
	if (toShift(row)!=connection().quote(owner)) {
	  invalid=true;
	  break;
	}
	if (toShift(row)!="TABLE")
	  continue;
	if (toShift(row)!=connection().quote(table)) {
	  invalid=true;
	  break;
	}
	QString type=toShift(row);
	if (type=="COLUMN") {
	  // Nop, handled by the parseColumnDescription call below
	} else if (type=="COMMENT")
	  Comment->setText(toShift(row));
	else if (type=="STORAGE")
	  storage+=toShift(row)+" ";
	else if (type=="PARALLEL")
	  parallel+=toShift(row)+" ";
	else if (type=="PARAMETERS") {
	  QString t=toShift(row);
	  if (t.startsWith("TABLESPACE")) {
	    tablespace=connection().unQuote(t.mid(10).stripWhiteSpace());
	  } else
	    declarations+=t+" ";
	} else if (!type.isEmpty())
	  invalid=true;
      }

      Columns=toExtract::parseColumnDescription(OriginalDescription);

      ExtraDeclarations->setText(declarations.stripWhiteSpace());
      StorageDeclarations->setText(storage.stripWhiteSpace());
      ParallelDeclarations->setText(parallel.stripWhiteSpace());
      {
	for(unsigned int i=0;i<Columns.size();i++)
	  addColumn();
      }
      std::list<QLineEdit *>::iterator name=ColumnNames.begin();
      std::list<toDatatype *>::iterator datatype=Datatypes.begin();
      std::list<QLineEdit *>::iterator extra=Extra.begin();
      for(std::list<toExtract::columnInfo>::iterator column=Columns.begin();
	  name!=ColumnNames.end()&&datatype!=Datatypes.end()&&extra!=Extra.end()&&column!=Columns.end();
	  name++,datatype++,extra++,column++) {
	if ((*column).Order==0)
	  invalid=true;
	(*name)->setText((*column).Name);
	(*datatype)->setType((*column).Definition);
	(*extra)->setText((*column).Data["EXTRA"]);
      }
      if (invalid) {
	reject();
	toStatusMessage("Invalid output from extraction layer prevent this dialog to be properl filled in");
	return;
      }
    } else
      addColumn();
    if (toIsMySQL(connection())) {
      ParallelLabel->hide();
      ParallelDeclarations->hide();
      SchemaLabel->setText(tr("&Database"));
    }
    try {
      toQuery query(connection(),SQLListTablespaces);
      while(!query.eof()) {
	QString t=query.readValueNull();
	Tablespace->insertItem(t);
	if (t==tablespace)
	  Tablespace->setCurrentItem(Tablespace->count()-1);
      }
    } catch(...) {
      TablespaceLabel->hide();
      Tablespace->hide();
    }
  } catch(const QString &exc) {
    toStatusMessage(exc);
    reject();
  }
  uglyWorkaround();
}

void toBrowserTable::addParameters(std::list<QString> &migrateTable,
				   const std::list<QString> &ctx,
				   const QString &type,
				   const QString &data)
{
  toSQLParse::statement statement=toSQLParse::parseStatement(data,connection());
  std::list<toSQLParse::statement>::iterator beg=statement.subTokens().begin();
  std::list<toSQLParse::statement>::iterator end=beg;
  while(end!=statement.subTokens().end()) {
    if (beg!=end) {
      if ((*end).String!="=") {
	if ((*end).String=="(") {
	  do {
	    end++;
	  } while(end!=statement.subTokens().end()&&(*end).String!=")");
	}
	end++;
	toExtract::addDescription(migrateTable,ctx,type,Extractor.createFromParse(beg,end));
	beg=end;
      } else
	end++;
    } else
      end++;
  }
  if (beg!=end)
    toExtract::addDescription(migrateTable,ctx,type,Extractor.createFromParse(beg,end).stripWhiteSpace());
}

QString toBrowserTable::sql()
{
  std::list<QString> migrateTable;

  std::list<QString> ctx;
  toPush(ctx,Owner);
  toPush(ctx,QString("TABLE"));
  if (Table.isEmpty())
    toPush(ctx,Name->text());
  else
    toPush(ctx,Table);
  toExtract::addDescription(migrateTable,ctx);
  if (Name->text()!=Table&&!Table.isEmpty())
    toExtract::addDescription(migrateTable,ctx,"RENAME",Name->text());

  std::list<toExtract::columnInfo>::const_iterator column=Columns.begin();
  std::list<QLineEdit *>::const_iterator name=ColumnNames.begin();
  std::list<toDatatype *>::const_iterator datatype=Datatypes.begin();
  std::list<QLineEdit *>::const_iterator extra=Extra.begin();
  int num=1;
  while(name!=ColumnNames.end()&&datatype!=Datatypes.end()&&extra!=Extra.end()) {
    QString cname;
    if (column!=Columns.end()) {
      cname=(*column).Name;
      if ((*name)->text()!=(*column).Name)
	toExtract::addDescription(migrateTable,ctx,"COLUMN",cname,"RENAME",(*name)->text());
      column++;
    } else
      cname=(*name)->text();

    toExtract::addDescription(migrateTable,ctx,"COLUMN",cname);
    toExtract::addDescription(migrateTable,ctx,"COLUMN",cname,(*datatype)->type());
    if (!(*extra)->text().isEmpty())
      toExtract::addDescription(migrateTable,ctx,"COLUMN",cname,"EXTRA",(*extra)->text());
    toExtract::addDescription(migrateTable,ctx,"COLUMN",cname,"ORDER",QString::number(num++));
    name++;
    datatype++;
    extra++;
  }

  if (!Comment->text().isEmpty())
    toExtract::addDescription(migrateTable,ctx,"COMMENT",Comment->text());
  addParameters(migrateTable,ctx,"STORAGE",StorageDeclarations->text());
  addParameters(migrateTable,ctx,"PARALLEL",ParallelDeclarations->text());
  addParameters(migrateTable,ctx,"PARAMETERS",ExtraDeclarations->text());

  migrateTable.sort();

  return Extractor.migrate(OriginalDescription,migrateTable);
}

void toBrowserTable::displaySQL()
{
  toMemoEditor memo(this,sql(),-1,-1,true,true);
  memo.exec();
}

void toBrowserTable::addColumn()
{
  ColumnNumber++;
  QLineEdit *tl;
  toDatatype *td;
  tl=new QLineEdit(ColumnGrid,QString::number(ColumnNumber));
  tl->show();
  ColumnNames.insert(ColumnNames.end(),tl);
  td=new toDatatype(connection(),ColumnGrid);
  td->show();
  Datatypes.insert(Datatypes.end(),td);
  tl=new QLineEdit(ColumnGrid);
  tl->show();
  Extra.insert(Extra.end(),tl);
}

void toBrowserTable::removeColumn()
{
  // Not implemented yet
}

void toBrowserTable::toggleCustom(bool val)
{
  for(std::list<toDatatype *>::iterator i=Datatypes.begin();i!=Datatypes.end();i++)
    (*i)->setCustom(val);
}

void toBrowserTable::uglyWorkaround()
{
  // Somehome the size doesn't get updated until way later so just keep calling until it gets set.
  printf("%d\n",ColumnList->width());
  if (ColumnList->width()>220||UglyFlag) {
    ColumnGrid->setFixedWidth(ColumnList->width()-30);
    UglyFlag=true;
  } else
    QTimer::singleShot(100,this,SLOT(uglyWorkaround()));
}

void toBrowserTable::resizeEvent(QResizeEvent *e)
{
  toBrowserTableUI::resizeEvent(e);
  uglyWorkaround();
}
