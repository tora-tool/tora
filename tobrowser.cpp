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
#include "toresultitem.h"
#include "toresultfield.h"
#include "toresultindexes.h"
#include "toresultcontent.h"
#include "toresultcontent.h"

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
#define TAB_TRIGGER		"Trigger"
#define TAB_TRIGGER_INFO	"TriggerInfo"
#define TAB_TRIGGER_SOURCE	"TriggerSource"
#define TAB_TRIGGER_COLS	"TriggerCols"

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
  resultView->setSQL("SELECT Table_Name FROM ALL_TABLES WHERE OWNER = :f1<char[31]>"
		     " ORDER BY Table_Name");
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

  resultView=new toResultView(true,false,Connection,curr,TAB_TABLE_GRANTS);
  resultView->setReadAll(true);
  resultView->setSQL("SELECT Privilege,Grantee,Grantor,Grantable FROM ALL_TAB_PRIVS "
		     " WHERE Table_Schema = :f1<char[31]> AND Table_Name = :f2<char[31]>"
		     " ORDER BY Privilege,Grantee");
  curr->addTab(resultView,"Grants");
  SecondMap[TAB_TABLE_GRANTS]=resultView;

  resultView=new toResultView(true,false,Connection,curr,TAB_TABLE_TRIGGERS);
  resultView->setReadAll(true);
  resultView->setSQL("SELECT Trigger_Name,Triggering_Event,Column_Name,Status,Description "
		     "  FROM ALL_TRIGGERS "
		     "WHERE Table_Owner = :f1<char[31]> AND Table_Name = :f2<char[31]>");
  curr->addTab(resultView,"Triggers");
  SecondMap[TAB_TABLE_TRIGGERS]=resultView;

  resultView=new toResultContent(Connection,curr,TAB_TABLE_DATA);
  curr->addTab(resultView,"Data");
  SecondMap[TAB_TABLE_DATA]=resultView;

  toResultItem *resultItem=new toResultItem(2,true,Connection,curr,TAB_TABLE_INFO);
  resultItem->setSQL("SELECT * "
		     "  FROM ALL_TABLES "
		     " WHERE OWNER = :f1<char[31]> AND Table_Name = :f2<char[31]>");
  curr->addTab(resultItem,"Information");
  SecondMap[TAB_TABLE_INFO]=resultItem;

  resultItem=new toResultItem(1,true,Connection,curr,TAB_TABLE_COMMENT);
  resultItem->showTitle(false);
  resultItem->setSQL("SELECT Comments FROM ALL_TAB_COMMENTS "
		     " WHERE Owner = :f1<char[31]> AND Table_Name = :f2<char[31]>");
  curr->addTab(resultItem,"Comment");
  SecondMap[TAB_TABLE_COMMENT]=resultItem;

  splitter=new QSplitter(Horizontal,TopTab,TAB_VIEWS);
  TopTab->addTab(splitter,"Views");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_VIEWS]=resultView;
  resultView->setSQL("SELECT View_Name FROM ALL_VIEWS WHERE OWNER = :f1<char[31]>"
		     " ORDER BY View_Name");
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
  resultField->setSQL("SELECT Text SQL"
		     "  FROM ALL_Views"
		     " WHERE Owner = :f1<char[31]> AND View_Name = :f2<char[31]>");
  curr->addTab(resultField,"SQL");
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  SecondMap[TAB_VIEW_SQL]=resultField;

  resultView=new toResultContent(Connection,curr,TAB_VIEW_DATA);
  curr->addTab(resultView,"Data");
  SecondMap[TAB_VIEW_DATA]=resultView;

  resultView=new toResultView(true,false,Connection,curr,TAB_VIEW_GRANTS);
  resultView->setReadAll(true);
  resultView->setSQL("SELECT Privilege,Grantee,Grantor,Grantable FROM ALL_TAB_PRIVS "
		     " WHERE Table_Schema = :f1<char[31]> AND Table_Name = :f2<char[31]>"
		     " ORDER BY Privilege,Grantee");
  curr->addTab(resultView,"Grants");
  SecondMap[TAB_VIEW_GRANTS]=resultView;

  resultItem=new toResultItem(1,true,Connection,curr,TAB_VIEW_COMMENT);
  resultItem->showTitle(false);
  resultItem->setSQL("SELECT Comments FROM ALL_TAB_COMMENTS "
		     " WHERE Owner = :f1<char[31]> AND Table_Name = :f2<char[31]>");
  curr->addTab(resultItem,"Comment");
  SecondMap[TAB_VIEW_COMMENT]=resultItem;

  splitter=new QSplitter(Horizontal,TopTab,TAB_INDEX);
  TopTab->addTab(splitter,"Indexes");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_INDEX]=resultView;
  resultView->setSQL("SELECT Index_Name"
		     "  FROM ALL_INDEXES"
		     " WHERE OWNER = :f1<char[31]>"
		     " ORDER BY Index_Name");
  resultView->resize(FIRST_WIDTH,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  splitter->setResizeMode(curr,QSplitter::Stretch);

  resultView=new toResultView(true,false,Connection,curr,TAB_INDEX_COLS);
  resultView->setSQL("SELECT Table_Name,Column_Name,Column_Length,Descend"
		     "  FROM ALL_IND_COLUMNS"
		     " WHERE Index_Owner = :f1<char[31]> AND Index_Name = :f2<char[31]>"
		     " ORDER BY Column_Position");
  curr->addTab(resultView,"Columns");
  SecondMap[TAB_INDEX]=resultView;
  SecondMap[TAB_INDEX_COLS]=resultView;

  resultItem=new toResultItem(2,true,Connection,curr,TAB_INDEX_INFO);
  resultItem->setSQL("SELECT * FROM ALL_INDEXES "
		     "WHERE Owner = :f1<char[31]> AND Index_Name = :f2<char[31]>");
  curr->addTab(resultItem,"Info");
  SecondMap[TAB_INDEX_INFO]=resultItem;

  splitter=new QSplitter(Horizontal,TopTab,TAB_SEQUENCES);
  TopTab->addTab(splitter,"Sequences");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_SEQUENCES]=resultView;
  resultView->setSQL("SELECT Sequence_Name FROM ALL_SEQUENCES"
		     " WHERE SEQUENCE_OWNER = :f1<char[31]>"
		     " ORDER BY Sequence_Name");
  resultView->resize(FIRST_WIDTH,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  splitter->setResizeMode(curr,QSplitter::Stretch);
  resultItem=new toResultItem(2,true,Connection,curr,TAB_SEQUENCES_INFO);
  resultItem->setSQL("SELECT * FROM ALL_SEQUENCES "
		     "WHERE Sequence_Owner = :f1<char[31]> AND Sequence_Name = :f2<char[31]>");
  curr->addTab(resultItem,"Info");
  SecondMap[TAB_SEQUENCES]=resultItem;
  SecondMap[TAB_SEQUENCES_INFO]=resultItem;

  splitter=new QSplitter(Horizontal,TopTab,TAB_SYNONYM);
  TopTab->addTab(splitter,"Synonyms");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_SYNONYM]=resultView;
  resultView->setSQL("SELECT Synonym_Name FROM ALL_SYNONYMS WHERE TABLE_OWNER = :f1<char[31]>"
		     " ORDER BY Synonym_Name");
  resultView->resize(FIRST_WIDTH,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  splitter->setResizeMode(curr,QSplitter::Stretch);
  resultItem=new toResultItem(2,true,Connection,curr,TAB_SYNONYM_INFO);
  resultItem->setSQL("SELECT * FROM ALL_SYNONYMS "
		     "WHERE Table_Owner = :f1<char[31]> AND Synonym_Name = :f2<char[31]>");
  curr->addTab(resultItem,"Info");
  SecondMap[TAB_SYNONYM]=resultItem;
  SecondMap[TAB_SYNONYM_INFO]=resultItem;

  splitter=new QSplitter(Horizontal,TopTab,TAB_PLSQL);
  TopTab->addTab(splitter,"PL/SQL");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_PLSQL]=resultView;
  resultView->setSQL("SELECT Object_Name,Object_Type Type FROM ALL_OBJECTS"
		     " WHERE OWNER = :f1<char[31]>"
		     "   AND Object_Type IN ('FUNCTION','PACKAGE',"
		     "                       'PROCEDURE','TYPE')"
		     " ORDER BY Object_Name");
  resultView->resize(FIRST_WIDTH*2,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  splitter->setResizeMode(curr,QSplitter::Stretch);

  resultField=new toResultField(Connection,curr,TAB_PLSQL_SOURCE);
  resultField->setSQL("SELECT Text FROM ALL_SOURCE "
		      "WHERE Owner = :f1<char[31]> AND Name = :f2<char[31]>"
		      "AND Type NOT LIKE '% BODY'");
  curr->addTab(resultField,"Declaration");
  SecondMap[TAB_PLSQL]=resultField;
  SecondMap[TAB_PLSQL_SOURCE]=resultField;

  resultField=new toResultField(Connection,curr,TAB_PLSQL_BODY);
  resultField->setSQL("SELECT Text FROM ALL_SOURCE "
		      "WHERE Owner = :f1<char[31]> AND Name = :f2<char[31]> "
		      "AND Type LIKE '% BODY'");
  curr->addTab(resultField,"Body");
  SecondMap[TAB_PLSQL_BODY]=resultField;

  splitter=new QSplitter(Horizontal,TopTab,TAB_TRIGGER);
  TopTab->addTab(splitter,"Triggers");
  resultView=new toResultView(true,false,Connection,splitter);
  resultView->setReadAll(true);
  Map[TAB_TRIGGER]=resultView;
  resultView->setSQL("SELECT Trigger_Name FROM ALL_TRIGGERS"
		     " WHERE OWNER = :f1<char[31]>"
		     " ORDER BY Trigger_Name");
  resultView->resize(FIRST_WIDTH,resultView->height());
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));
  splitter->setResizeMode(curr,QSplitter::Stretch);

  resultItem=new toResultItem(2,true,Connection,curr,TAB_TRIGGER_INFO);
  resultItem->setSQL("SELECT Owner,Trigger_Name,"
		     "       Trigger_Type,Triggering_Event,"
		     "       Table_Owner,Base_Object_Type,Table_Name,Column_Name,"
		     "       Referencing_Names,When_Clause,Status,"
		     "       Description,Action_Type"
		     "  FROM ALL_TRIGGERS "
		     "WHERE Owner = :f1<char[31]> AND Trigger_Name = :f2<char[31]>");
  curr->addTab(resultItem,"Info");
  SecondMap[TAB_TRIGGER]=resultItem;
  SecondMap[TAB_TRIGGER_INFO]=resultItem;

  resultField=new toResultField(Connection,curr,TAB_TRIGGER_SOURCE);
  resultField->setSQL("SELECT Trigger_Body FROM ALL_TRIGGERS "
		      "WHERE Owner = :f1<char[31]> AND Trigger_Name = :f2<char[31]>");
  curr->addTab(resultField,"Code");
  SecondMap[TAB_TRIGGER_SOURCE]=resultField;

  resultView=new toResultView(true,false,Connection,curr,TAB_TRIGGER_COLS);
  resultView->setSQL("SELECT Column_Name,Column_List \"In Update\",Column_Usage Usage"
		     "  FROM ALL_TRIGGER_COLS "
		     "WHERE Trigger_Owner = :f1<char[31]> AND Trigger_Name = :f2<char[31]>");
  curr->addTab(resultView,"Columns");
  SecondMap[TAB_TRIGGER_COLS]=resultView;

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
		       "SELECT Username FROM ALL_Users ORDER BY Username",
		       Connection.connection());
      for(int i=0;!users.eof();i++) {
	char buffer[31];
	users>>buffer;
	Schema->insertItem(buffer);
	if (selected==buffer)
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
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
  } catch (const QString &str) {
    toStatusMessage((const char *)str);
  }
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
