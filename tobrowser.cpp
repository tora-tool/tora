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

#include <qheader.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qworkspace.h>
#include <qvaluelist.h>

#include "totool.h"
#include "tomain.h"
#include "tobrowser.h"
#include "toresultview.h"
#include "toresultcols.h"
#include "toresultconstraint.h"
#include "toresultreferences.h"
#include "toresultitem.h"
#include "toresultfield.h"
#include "toresultindexes.h"
#include "toresultcontent.h"
#include "toresultcontent.h"
#include "toresultdepend.h"
#include "tosql.h"

#include "tobrowser.moc"

#include "icons/tobrowser.xpm"
#include "icons/refresh.xpm"

class toBrowserTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return tobrowser_xpm; }
public:
  toBrowserTool()
    : toTool(2,"Schema Browser")
  { }
  virtual const char *menuItem()
  { return "Schema Browser"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    QWidget *window=new toBrowser(parent,connection);
    window->setIcon(*toolbarImage());
    return window;
  }
};

static toBrowserTool BrowserTool;

static QPixmap *toRefreshPixmap;

#define FIRST_WIDTH 150

#define TAB_TABLES		"Tables"
#define TAB_TABLE_COLUMNS	"TablesColumns"
#define TAB_TABLE_CONS		"TablesConstraint"
#define TAB_TABLE_DEPEND	"TablesDepend"
#define TAB_TABLE_INDEXES	"TablesIndexes"
#define TAB_TABLE_DATA		"TablesData"
#define TAB_TABLE_GRANTS	"TablesGrants"
#define TAB_TABLE_TRIGGERS	"TablesTriggers"
#define TAB_TABLE_INFO		"TablesInfo"
#define TAB_TABLE_COMMENT	"TablesComment"
#define TAB_VIEWS		"Views"
#define TAB_VIEW_COLUMNS	"ViewColumns"
#define TAB_VIEW_SQL		"ViewSQL"
#define TAB_VIEW_DATA		"ViewData"
#define TAB_VIEW_GRANTS		"ViewGrants"
#define TAB_VIEW_DEPEND		"ViewDepend"
#define TAB_VIEW_COMMENT	"ViewComment"
#define TAB_SEQUENCES		"Sequences"
#define TAB_SEQUENCES_INFO	"SequencesInfo"
#define TAB_INDEX		"Index"
#define TAB_INDEX_COLS		"IndexCols"
#define TAB_INDEX_INFO		"IndexInfo"
#define TAB_SYNONYM		"Synonym"
#define TAB_SYNONYM_INFO	"SynonymInfo"
#define TAB_PLSQL		"PLSQL"
#define TAB_PLSQL_SOURCE	"PLSQLSource"
#define TAB_PLSQL_BODY		"PLSQLBody"
#define TAB_PLSQL_DEPEND	"PLSQLDepend"
#define TAB_TRIGGER		"Trigger"
#define TAB_TRIGGER_INFO	"TriggerInfo"
#define TAB_TRIGGER_SOURCE	"TriggerSource"
#define TAB_TRIGGER_COLS	"TriggerCols"
#define TAB_TRIGGER_DEPEND	"TriggerDepend"

static toSQL SQLListTables("toBrowser:ListTables",
			   "SELECT Table_Name FROM ALL_TABLES WHERE OWNER = :f1<char[31]>\n"
			   " ORDER BY Table_Name",
			   "List the available tables in a schema.");
static toSQL SQLTableGrants("toBrowser:TableGrants",
			    "SELECT Privilege,Grantee,Grantor,Grantable FROM ALL_TAB_PRIVS\n"
			    " WHERE Table_Schema = :f1<char[31]> AND Table_Name = :f2<char[31]>\n"
			    " ORDER BY Privilege,Grantee",
			    "Display the grants on a table");
static toSQL SQLTableTrigger("toBrowser:TableTrigger",
			     "SELECT Trigger_Name,Triggering_Event,Column_Name,Status,Description \n"
			     "  FROM ALL_TRIGGERS\n"
			     " WHERE Table_Owner = :f1<char[31]> AND Table_Name = :f2<char[31]>",
			     "Display the triggers operating on a table");
static toSQL SQLTableInfo("toBrowser:TableInformation",
			  "SELECT *\n"
			  "  FROM ALL_TABLES\n"
			  " WHERE OWNER = :f1<char[31]> AND Table_Name = :f2<char[31]>",
			  "Display information about a table");
static toSQL SQLTableComment("toBrowser:TableComment",
			     "SELECT Comments FROM ALL_TAB_COMMENTS\n"
			     " WHERE Owner = :f1<char[31]> AND Table_Name = :f2<char[31]>",
			     "Display comment on a table");

static toSQL SQLListView("toBrowser:ListView",
			 "SELECT View_Name FROM ALL_VIEWS WHERE OWNER = :f1<char[31]>\n"
			 " ORDER BY View_Name",
			 "List the available views in a schema");
static toSQL SQLViewSQL("toBrowser:ViewSQL",	
			"SELECT Text SQL\n"
			"  FROM ALL_Views\n"
			" WHERE Owner = :f1<char[31]> AND View_Name = :f2<char[31]>",
			"Display SQL of a specified view");
static toSQL SQLViewGrants("toBrowser:ViewGrants",
			   "SELECT Privilege,Grantee,Grantor,Grantable FROM ALL_TAB_PRIVS\n"
			   " WHERE Table_Schema = :f1<char[31]> AND Table_Name = :f2<char[31]>\n"
			   " ORDER BY Privilege,Grantee",
			   "Display grants on a view");
static toSQL SQLViewComment("toBrowser:ViewComment",
			    "SELECT Comments FROM ALL_TAB_COMMENTS\n"
			    " WHERE Owner = :f1<char[31]> AND Table_Name = :f2<char[31]>",
			    "Display comment on a view");

static toSQL SQLListIndex("toBrowser:ListIndex",
			  "SELECT Index_Name\n"
			  "  FROM ALL_INDEXES\n"
			  " WHERE OWNER = :f1<char[31]>\n"
			  " ORDER BY Index_Name\n",
			  "List the available indexes in a schema");
static toSQL SQLIndexCols("toBrowser:IndexCols",
			  "SELECT Table_Name,Column_Name,Column_Length,Descend\n"
			  "  FROM ALL_IND_COLUMNS\n"
			  " WHERE Index_Owner = :f1<char[31]> AND Index_Name = :f2<char[31]>\n"
			  " ORDER BY Column_Position",
			  "Display columns on which an index is built");
static toSQL SQLIndexInfo("toBrowser:IndexInformation",
			  "SELECT * FROM ALL_INDEXES\n"
			  " WHERE Owner = :f1<char[31]> AND Index_Name = :f2<char[31]>",
			  "Display information about an index");

static toSQL SQLListSequence("toBrowser:ListSequence",
			     "SELECT Sequence_Name FROM ALL_SEQUENCES\n"
			     " WHERE SEQUENCE_OWNER = :f1<char[31]>\n"
			     " ORDER BY Sequence_Name",
			     "List the available sequences in a schema");
static toSQL SQLSequenceInfo("toBrowser:SequenceInformation",
			     "SELECT * FROM ALL_SEQUENCES\n"
			     " WHERE Sequence_Owner = :f1<char[31]>\n"
			     "   AND Sequence_Name = :f2<char[31]>",
			     "Display information about a sequence");

static toSQL SQLListSynonym("toBrowser:ListSynonym",
			    "SELECT Synonym_Name FROM ALL_SYNONYMS\n"
			    " WHERE Table_Owner = :f1<char[31]>"
			    " ORDER BY Synonym_Name",
			    "List the available synonyms in a schema");
static toSQL SQLSynonymInfo("toBrowser:SynonymInformation",
			    "SELECT * FROM ALL_SYNONYMS\n"
			    " WHERE Table_Owner = :f1<char[31]> AND Synonym_Name = :f2<char[31]>",
			    "Display information about a synonym");

static toSQL SQLListSQL("toBrowser:ListPL/SQL",
			"SELECT Object_Name,Object_Type Type FROM ALL_OBJECTS\n"
			" WHERE OWNER = :f1<char[31]>\n"
			"   AND Object_Type IN ('FUNCTION','PACKAGE',\n"
			"                       'PROCEDURE','TYPE')\n"
			" ORDER BY Object_Name",
			"List the available PL/SQL objects in a schema");
static toSQL SQLSQLHead("toBrowser:PL/SQLHead",
			"SELECT Text FROM ALL_SOURCE\n"
			" WHERE Owner = :f1<char[31]> AND Name = :f2<char[31]>\n"
			"   AND Type IN ('PACKAGE','TYPE')",
			"Declaration of object");
static toSQL SQLSQLBody("toBrowser:PL/SQLBody",
			"SELECT Text FROM ALL_SOURCE\n"
			" WHERE Owner = :f1<char[31]> AND Name = :f2<char[31]>\n"
			"   AND Type IN ('PACKAGE','PROCEDURE','PACKAGE BODY','TYPE BODY')",
			"Implementation of object");

static toSQL SQLListTrigger("toBrowser:ListTrigger",
			    "SELECT Trigger_Name FROM ALL_TRIGGERS\n"
			    " WHERE OWNER = :f1<char[31]>\n"
			    " ORDER BY Trigger_Name",
			    "List the available triggers in a schema");
static toSQL SQLTriggerInfo("toBrowser:TriggerInfo",
			    "SELECT Owner,Trigger_Name,\n"
			    "       Trigger_Type,Triggering_Event,\n"
			    "       Table_Owner,Base_Object_Type,Table_Name,Column_Name,\n"
			    "       Referencing_Names,When_Clause,Status,\n"
			    "       Description,Action_Type\n"
			    "  FROM ALL_TRIGGERS\n"
			    "WHERE Owner = :f1<char[31]> AND Trigger_Name = :f2<char[31]>",
			    "Display information about a trigger");
static toSQL SQLTriggerBody("toBrowser:TriggerBody",
			    "SELECT Trigger_Body FROM ALL_TRIGGERS\n"
			    " WHERE Owner = :f1<char[31]> AND Trigger_Name = :f2<char[31]>",
			    "Implementation of trigger");
static toSQL SQLTriggerCols("toBrowser:TriggerCols",
			    "SELECT Column_Name,Column_List \"In Update\",Column_Usage Usage\n"
			    "  FROM ALL_TRIGGER_COLS\n"
			    " WHERE Trigger_Owner = :f1<char[31]> AND Trigger_Name = :f2<char[31]>",
			    "Columns used by trigger");

toBrowser::toBrowser(QWidget *parent,toConnection &connection)
  : QVBox(parent,NULL,WDestructiveClose),Connection(connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);
  Connection.addWidget(this);

  QToolBar *toolbar=new QToolBar("Schema Browser",toMainWidget(),this);
  new QToolButton(*toRefreshPixmap,
		  "Update from DB",
		  "Update from DB",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  Schema=new QComboBox(toolbar);
  connect(Schema,SIGNAL(activated(int)),
	  this,SLOT(changeSchema(int)));
  toolbar->setStretchableWidget(new QLabel("",toolbar));
  
  TopTab=new QTabWidget(this);
  QSplitter *splitter=new QSplitter(Horizontal,TopTab,TAB_TABLES);
  TopTab->addTab(splitter,"Tables");
  toResultView *resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  resultView->setSQL(SQLListTables);
  resultView->resize(FIRST_WIDTH,resultView->height());
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  FirstTab=resultView;
  Map[TAB_TABLES]=resultView;
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));

  QTabWidget *curr=new QTabWidget(splitter);
  splitter->setResizeMode(curr,QSplitter::Stretch);
  resultView=new toResultCols(Connection,curr,TAB_TABLE_COLUMNS);
  curr->addTab(resultView,"Columns");
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  SecondTab=resultView;
  SecondMap[TAB_TABLES]=resultView;
  SecondMap[TAB_TABLE_COLUMNS]=resultView;

  resultView=new toResultIndexes(Connection,curr,TAB_TABLE_INDEXES);
  curr->addTab(resultView,"Indexes");
  SecondMap[TAB_TABLE_INDEXES]=resultView;

  toResultConstraint *resultConstraint=new toResultConstraint(Connection,curr,
							      TAB_TABLE_CONS);
  curr->addTab(resultConstraint,"Constraints");
  SecondMap[TAB_TABLE_CONS]=resultConstraint;

  toResultReferences *resultReferences=new toResultReferences(Connection,curr,
							      TAB_TABLE_DEPEND);
  curr->addTab(resultReferences,"References");
  SecondMap[TAB_TABLE_DEPEND]=resultReferences;

  resultView=new toResultView(true,false,Connection,curr,TAB_TABLE_GRANTS);
  resultView->setReadAll(true);
  resultView->setSQL(SQLTableGrants);
  curr->addTab(resultView,"Grants");
  SecondMap[TAB_TABLE_GRANTS]=resultView;

  resultView=new toResultView(true,false,Connection,curr,TAB_TABLE_TRIGGERS);
  resultView->setReadAll(true);
  resultView->setSQL(SQLTableTrigger);
  curr->addTab(resultView,"Triggers");
  SecondMap[TAB_TABLE_TRIGGERS]=resultView;

  resultView=new toResultContent(Connection,curr,TAB_TABLE_DATA);
  curr->addTab(resultView,"Data");
  SecondMap[TAB_TABLE_DATA]=resultView;

  toResultItem *resultItem=new toResultItem(2,true,Connection,curr,TAB_TABLE_INFO);
  resultItem->setSQL(SQLTableInfo(Connection));
  curr->addTab(resultItem,"Information");
  SecondMap[TAB_TABLE_INFO]=resultItem;

  resultItem=new toResultItem(1,true,Connection,curr,TAB_TABLE_COMMENT);
  resultItem->showTitle(false);
  resultItem->setSQL(SQLTableComment(Connection));
  curr->addTab(resultItem,"Comment");
  SecondMap[TAB_TABLE_COMMENT]=resultItem;

  splitter=new QSplitter(Horizontal,TopTab,TAB_VIEWS);
  TopTab->addTab(splitter,"Views");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_VIEWS]=resultView;
  resultView->setSQL(SQLListView);
  resultView->resize(FIRST_WIDTH,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  splitter->setResizeMode(curr,QSplitter::Stretch);
  resultView=new toResultCols(Connection,curr,TAB_VIEW_COLUMNS);
  curr->addTab(resultView,"Columns");
  SecondMap[TAB_VIEWS]=resultView;
  SecondMap[TAB_VIEW_COLUMNS]=resultView;

  toResultField *resultField=new toResultField(Connection,curr,TAB_VIEW_SQL);
  resultField->setSQL(SQLViewSQL);
  curr->addTab(resultField,"SQL");
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  SecondMap[TAB_VIEW_SQL]=resultField;

  resultView=new toResultContent(Connection,curr,TAB_VIEW_DATA);
  curr->addTab(resultView,"Data");
  SecondMap[TAB_VIEW_DATA]=resultView;

  resultView=new toResultView(true,false,Connection,curr,TAB_VIEW_GRANTS);
  resultView->setReadAll(true);
  resultView->setSQL(SQLViewGrants);
  curr->addTab(resultView,"Grants");
  SecondMap[TAB_VIEW_GRANTS]=resultView;

  toResultDepend *resultDepend=new toResultDepend(Connection,curr,TAB_VIEW_DEPEND);
  curr->addTab(resultDepend,"Dependencies");
  SecondMap[TAB_VIEW_DEPEND]=resultDepend;

  resultItem=new toResultItem(1,true,Connection,curr,TAB_VIEW_COMMENT);
  resultItem->showTitle(false);
  resultItem->setSQL(SQLViewComment(Connection));
  curr->addTab(resultItem,"Comment");
  SecondMap[TAB_VIEW_COMMENT]=resultItem;

  splitter=new QSplitter(Horizontal,TopTab,TAB_INDEX);
  TopTab->addTab(splitter,"Indexes");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_INDEX]=resultView;
  resultView->setSQL(SQLListIndex);
  resultView->resize(FIRST_WIDTH,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  splitter->setResizeMode(curr,QSplitter::Stretch);

  resultView=new toResultView(true,false,Connection,curr,TAB_INDEX_COLS);
  resultView->setSQL(SQLIndexCols);
  curr->addTab(resultView,"Columns");
  SecondMap[TAB_INDEX]=resultView;
  SecondMap[TAB_INDEX_COLS]=resultView;

  resultItem=new toResultItem(2,true,Connection,curr,TAB_INDEX_INFO);
  resultItem->setSQL(SQLIndexInfo(Connection));
  curr->addTab(resultItem,"Info");
  SecondMap[TAB_INDEX_INFO]=resultItem;

  splitter=new QSplitter(Horizontal,TopTab,TAB_SEQUENCES);
  TopTab->addTab(splitter,"Sequences");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_SEQUENCES]=resultView;
  resultView->setSQL(SQLListSequence);
  resultView->resize(FIRST_WIDTH,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  splitter->setResizeMode(curr,QSplitter::Stretch);
  resultItem=new toResultItem(2,true,Connection,curr,TAB_SEQUENCES_INFO);
  resultItem->setSQL(SQLSequenceInfo(Connection));
  curr->addTab(resultItem,"Info");
  SecondMap[TAB_SEQUENCES]=resultItem;
  SecondMap[TAB_SEQUENCES_INFO]=resultItem;

  splitter=new QSplitter(Horizontal,TopTab,TAB_SYNONYM);
  TopTab->addTab(splitter,"Synonyms");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_SYNONYM]=resultView;
  resultView->setSQL(SQLListSynonym);
  resultView->resize(FIRST_WIDTH,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  splitter->setResizeMode(curr,QSplitter::Stretch);
  resultItem=new toResultItem(2,true,Connection,curr,TAB_SYNONYM_INFO);
  resultItem->setSQL(SQLSynonymInfo(Connection));
  curr->addTab(resultItem,"Info");
  SecondMap[TAB_SYNONYM]=resultItem;
  SecondMap[TAB_SYNONYM_INFO]=resultItem;

  splitter=new QSplitter(Horizontal,TopTab,TAB_PLSQL);
  TopTab->addTab(splitter,"PL/SQL");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_PLSQL]=resultView;
  resultView->setSQL(SQLListSQL);
  resultView->resize(FIRST_WIDTH*2,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  splitter->setResizeMode(curr,QSplitter::Stretch);

  resultField=new toResultField(Connection,curr,TAB_PLSQL_SOURCE);
  resultField->setSQL(SQLSQLHead);
  curr->addTab(resultField,"Declaration");
  SecondMap[TAB_PLSQL]=resultField;
  SecondMap[TAB_PLSQL_SOURCE]=resultField;

  resultField=new toResultField(Connection,curr,TAB_PLSQL_BODY);
  resultField->setSQL(SQLSQLBody);
  curr->addTab(resultField,"Body");
  SecondMap[TAB_PLSQL_BODY]=resultField;

  resultDepend=new toResultDepend(Connection,curr,TAB_PLSQL_DEPEND);
  curr->addTab(resultDepend,"Dependencies");
  SecondMap[TAB_PLSQL_DEPEND]=resultDepend;

  splitter=new QSplitter(Horizontal,TopTab,TAB_TRIGGER);
  TopTab->addTab(splitter,"Triggers");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_TRIGGER]=resultView;
  resultView->setSQL(SQLListTrigger);
  resultView->resize(FIRST_WIDTH,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  splitter->setResizeMode(curr,QSplitter::Stretch);

  resultItem=new toResultItem(2,true,Connection,curr,TAB_TRIGGER_INFO);
  resultItem->setSQL(SQLTriggerInfo(Connection));
  curr->addTab(resultItem,"Info");
  SecondMap[TAB_TRIGGER]=resultItem;
  SecondMap[TAB_TRIGGER_INFO]=resultItem;

  resultField=new toResultField(Connection,curr,TAB_TRIGGER_SOURCE);
  resultField->setSQL(SQLTriggerBody);
  curr->addTab(resultField,"Code");
  SecondMap[TAB_TRIGGER_SOURCE]=resultField;

  resultView=new toResultView(true,false,Connection,curr,TAB_TRIGGER_COLS);
  resultView->setSQL(SQLTriggerCols);
  curr->addTab(resultView,"Columns");
  SecondMap[TAB_TRIGGER_COLS]=resultView;

  resultDepend=new toResultDepend(Connection,curr,TAB_TRIGGER_DEPEND);
  curr->addTab(resultDepend,"Dependencies");
  SecondMap[TAB_TRIGGER_DEPEND]=resultDepend;

  refresh();

  connect(TopTab,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeTab(QWidget *)));
}

toBrowser::~toBrowser()
{
  Connection.delWidget(this);
}

void toBrowser::refresh(void)
{
  try {
    QString selected=Schema->currentText();
    if (selected.isEmpty()) {
      selected=Connection.user().upper();
      Schema->clear();
      otl_stream users(1,
		       toSQL::sql(TOSQL_USERLIST,Connection),
		       Connection.connection());
      for(int i=0;!users.eof();i++) {
	char buffer[31];
	users>>buffer;
	Schema->insertItem(QString::fromUtf8(buffer));
	if (selected==QString::fromUtf8(buffer))
	  Schema->setCurrentItem(i);
      }
    }
    if (!Schema->currentText().isEmpty()) {
      QListViewItem *item=FirstTab->currentItem();
      QString str;
      if (item)
	str=item->text(0);
      FirstTab->changeParams(Schema->currentText());
      if (!str.isEmpty()) {
	for (item=FirstTab->firstChild();item;item=item->nextSibling()) {
	  if (item->text(0)==str) {
	    FirstTab->setSelected(item,true);
	    break;
	  }
	}
      }
    }
    if (SecondTab&&!SecondText.isEmpty())
      SecondTab->changeParams(Schema->currentText(),SecondText);
  } TOCATCH
}

void toBrowser::changeItem(QListViewItem *item)
{
  if (item) {
    SecondText=item->text(0);
    if (SecondTab&&!SecondText.isEmpty())
      SecondTab->changeParams(Schema->currentText(),
			      SecondText);
  }
}

void toBrowser::changeSecondTab(QWidget *tab)
{
  if (tab) {
    SecondTab=SecondMap[tab->name()];
    SecondMap[TopTab->currentPage()->name()]=SecondTab;
    if (SecondTab&&!SecondText.isEmpty())
      SecondTab->changeParams(Schema->currentText(),
			      SecondText);
  }
}

void toBrowser::changeTab(QWidget *tab)
{
  if (tab) {
    FirstTab=Map[tab->name()];
    SecondTab=SecondMap[tab->name()];
    SecondText="";
    if (FirstTab&&SecondTab)
      refresh();
  }
}
