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
#include "toextract.h"
#include "tofilesize.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "toreport.h"
#include "toresultview.h"
#include "toscript.h"
#include "toscriptui.h"
#include "tosql.h"
#include "totextview.h"
#include "totool.h"
#include "toworksheet.h"

#ifdef TO_HAS_KPRINT
#include <kfiledialog.h>
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtextview.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include "toscript.moc"
#include "toscriptui.moc"

#include "icons/execute.xpm"
#include "icons/toscript.xpm"

class toScriptTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return toscript_xpm; }
public:
  toScriptTool()
    : toTool(310,"DB Extraction/Compare/Search")
  { }
  virtual const char *menuItem()
  { return "DB Extraction/Compare/Search"; }
  virtual const char *toolbarTip()
  { return "DB or schema extraction, compare and search"; }
  virtual QWidget *toolWindow(QWidget *main,toConnection &connection)
  {
    return new toScript(main,connection);
  }
};

static toScriptTool ScriptTool;

static toSQL SQLObjectList("toScript:ExtractObject",
			   "SELECT *\n"
			   "  FROM (SELECT 'TABLESPACE',tablespace_name,NULL\n"
			   "	  FROM sys.dba_tablespaces\n"
			   "	UNION\n"
			   "	SELECT 'ROLE',role,NULL\n"
			   "	  FROM sys.dba_roles\n"
			   "	UNION\n"
			   "	SELECT 'PUBLIC',NULL,NULL\n"
			   "	  FROM dual\n"
			   "	UNION\n"
			   "	SELECT username,NULL,NULL\n"
			   "	  FROM sys.all_users)\n"
			   "  ORDER BY 1,2,3",
			   "Extract objects available to extract from the database, "
			   "should have same columns",
			   "8.1");

static toSQL SQLPublicSynonymList("toScript:PublicSynonyms",
				  "SELECT synonym_name\n"
				  "  FROM sys.all_synonyms WHERE owner = 'PUBLIC'\n"
				  " ORDER BY 1",
				  "Extract all public synonyms from database");

static toSQL SQLUserObjects("toScript:UserObjects",
			    "SELECT *\n"
			    "  FROM (SELECT 'DATABASE LINK',db_link\n"
			    "          FROM sys.all_db_links\n"
			    "         WHERE owner = :own<char[101]>\n"
			    "        UNION\n"
			    "        SELECT object_type,object_name\n"
			    "          FROM sys.all_objects\n"
			    "         WHERE object_type IN ('VIEW','TYPE','SEQUENCE','PACKAGE',\n"
			    "                               'PACKAGE BODY','FUNCTION','PROCEDURE')\n"
			    "           AND owner = :own<char[101]>\n"
			    "         UNION\n"
			    "        SELECT 'TABLE',table_name\n"
			    "          FROM sys.all_tables\n"
			    "         WHERE temporary != 'Y' AND secondary = 'N' AND iot_name IS NULL\n"
			    "           AND owner = :own<char[101]>\n"
			    "        UNION\n"
			    "        SELECT 'MATERIALIZED TABLE',mview_name AS object\n"
			    "          FROM sys.all_mviews\n"
			    "         WHERE owner = :own<char[101]>)\n"
			    " ORDER BY 1,2",
			    "Get the objects available for a user, must have same columns and binds");

static toSQL SQLUserObjects7("toScript:UserObjects",
			     "SELECT *\n"
			     "  FROM (SELECT 'DATABASE LINK',db_link\n"
			     "          FROM sys.all_db_links\n"
			     "         WHERE owner = :own<char[101]>\n"
			     "        UNION\n"
			     "        SELECT object_type,object_name\n"
			     "          FROM sys.all_objects\n"
			     "         WHERE object_type IN ('VIEW','TYPE','SEQUENCE','PACKAGE',\n"
			     "                               'PACKAGE BODY','FUNCTION','PROCEDURE')\n"
			     "           AND owner = :own<char[101]>\n"
			     "         UNION\n"
			     "        SELECT 'TABLE',table_name\n"
			     "          FROM sys.all_tables\n"
			     "         WHERE temporary != 'Y' AND secondary = 'N'\n"
			     "           AND owner = :own<char[101]>\n"
			     "        UNION\n"
			     "        SELECT 'MATERIALIZED TABLE',mview_name AS object\n"
			     "          FROM sys.all_mviews\n"
			     "         WHERE owner = :own<char[101]>)\n"
			     " ORDER BY 1,2",
			     "",
			     "7.3");

static toSQL SQLUserObjectList("toScript:UserExtractObject",
			       "SELECT owner,object_type,object_name\n"
			       "  FROM sys.all_objects\n"
			       " WHERE object_type IN ('VIEW','TABLE','TYPE','SEQUENCE','PACKAGE',\n"
			       "	               'PACKAGE BODY','FUNCTION','PROCEDURE')\n"
			       " ORDER BY 1,2,3",
			       "Extract objects available to extract from the database if you "
			       "don't have admin access, should have same columns");

static toSQL SQLSchemas("toScript:ExtractSchema",
			"SELECT username FROM sys.all_users ORDER BY username",
			"Get usernames available in database, must have same columns");

toScript::toScript(QWidget *parent,toConnection &connection)
  : toToolWidget(ScriptTool,"script.html",parent,connection)
{
  QToolBar *toolbar=toAllocBar(this,tr("Extraction and Compare"));

  new QToolButton(QPixmap((const char **)execute_xpm),
		  tr("Perform defined extraction"),
		  tr("Perform defined extraction"),
		  this,SLOT(execute(void)),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel(toolbar,TO_KDE_TOOLBAR_WIDGET));

  ScriptUI=new toScriptUI(this);

  QSplitter *hsplitter=new QSplitter(Horizontal,ScriptUI->DifferenceTab);
  QVBox *box=new QVBox(ScriptUI->ResultTab);
  Worksheet=new toWorksheet(box,connection);
  SearchList=new toListView(box);
  SearchList->addColumn(tr("Search result"));
  SearchList->setRootIsDecorated(true);
  SearchList->setSorting(0);
  SearchList->hide();
  Report=new toTextView(box);
  Report->hide();

  DropList=new toListView(hsplitter);
  DropList->addColumn(tr("Dropped"));
  DropList->setRootIsDecorated(true);
  DropList->setSorting(0);
  CreateList=new toListView(hsplitter);
  CreateList->addColumn(tr("Created"));
  CreateList->setRootIsDecorated(true);
  CreateList->setSorting(0);
  ScriptUI->Tabs->setTabEnabled(ScriptUI->ResultTab,false);
  ScriptUI->Tabs->setTabEnabled(ScriptUI->DifferenceTab,false);

  connect(SearchList,SIGNAL(clicked(QListViewItem *)),this,SLOT(keepOn(QListViewItem *)));
  connect(DropList,SIGNAL(clicked(QListViewItem *)),this,SLOT(keepOn(QListViewItem *)));
  connect(CreateList,SIGNAL(clicked(QListViewItem *)),this,SLOT(keepOn(QListViewItem *)));

  QGridLayout *layout=new QGridLayout(ScriptUI->ResultTab);
  layout->addWidget(box,0,0);
  layout=new QGridLayout(ScriptUI->DifferenceTab);
  layout->addWidget(hsplitter,0,0);

  ScriptUI->Initial->setTitle(tr("&Initial"));
  ScriptUI->Limit->setTitle(tr("&Limit"));
  ScriptUI->Next->setTitle(tr("&Next"));
  connect(ScriptUI->ModeGroup,SIGNAL(clicked(int)),this,SLOT(changeMode(int)));
  ScriptUI->Tabs->setTabEnabled(ScriptUI->ResizeTab,false);
  ScriptUI->SourceObjects->setSorting(0);
  ScriptUI->DestinationObjects->setSorting(0);

  // Remove when migrate and resize is implemented
#if 1
  ScriptUI->Migrate->hide();
#endif

  int def=0;
  std::list<QString> cons=toMainWidget()->connections();
  int i=0;
  while(cons.size()>0) {
    QString str=toShift(cons);
    if(str==connection.description()&&def==0)
      def=i;
    i++;
    ScriptUI->SourceConnection->insertItem(str);
    ScriptUI->DestinationConnection->insertItem(str);
  }
  ScriptUI->SourceConnection->setCurrentItem(def);
  changeSource(def);
  changeDestination(def);
  ScriptUI->DestinationConnection->setCurrentItem(def);

  connect(ScriptUI->AddButton,SIGNAL(clicked()),this,SLOT(newSize()));
  connect(ScriptUI->Remove,SIGNAL(clicked()),this,SLOT(removeSize()));

  connect(ScriptUI->SourceConnection,SIGNAL(activated(int)),this,SLOT(changeSource(int)));
  connect(ScriptUI->DestinationConnection,SIGNAL(activated(int)),this,SLOT(changeDestination(int)));
  connect(ScriptUI->SourceSchema,SIGNAL(activated(int)),this,SLOT(changeSourceSchema(int)));
  connect(ScriptUI->DestinationSchema,SIGNAL(activated(int)),this,SLOT(changeDestinationSchema(int)));
  connect(ScriptUI->SourceObjects,SIGNAL(clicked(QListViewItem *)),this,SLOT(objectClicked(QListViewItem *)));
  connect(ScriptUI->DestinationObjects,SIGNAL(clicked(QListViewItem *)),this,SLOT(objectClicked(QListViewItem *)));
  connect(ScriptUI->Browse,SIGNAL(clicked()),this,SLOT(browseFile()));

  connect(ScriptUI->SourceObjects,SIGNAL(expanded(QListViewItem *)),
	  this,SLOT(expandSource(QListViewItem *)));
  connect(ScriptUI->DestinationObjects,SIGNAL(clicked(QListViewItem *)),
	  this,SLOT(expandDestination(QListViewItem *)));

  connect(toMainWidget(),SIGNAL(addedConnection(const QString &)),
	  this,SLOT(addConnection(const QString &)));
  connect(toMainWidget(),SIGNAL(removedConnection(const QString &)),
	  this,SLOT(delConnection(const QString &)));

  ScriptUI->Schema->setCurrentItem(0);
  setFocusProxy(ScriptUI->Tabs);
}

void toScript::delConnection(const QString &name)
{
  for(int i=0;i<ScriptUI->SourceConnection->count();i++) {
    if (ScriptUI->SourceConnection->text(i)==name) {
      ScriptUI->SourceConnection->removeItem(i);
      break;
    }
  }
  for(int j=0;j<ScriptUI->DestinationConnection->count();j++) {
    if (ScriptUI->DestinationConnection->text(j)==name) {
      ScriptUI->DestinationConnection->removeItem(j);
      break;
    }
  }
}

void toScript::addConnection(const QString &name)
{
  ScriptUI->SourceConnection->insertItem(name);
  ScriptUI->DestinationConnection->insertItem(name);
}

toScript::~toScript()
{
}

std::list<QString> toScript::createObjectList(QListView *source)
{
  std::list<QString> lst;

  std::list<QString> otherGlobal;
  std::list<QString> profiles;
  std::list<QString> roles;
  std::list<QString> tableSpace;
  std::list<QString> tables;
  std::list<QString> userOther;
  std::list<QString> userViews;
  std::list<QString> users;

  QListViewItem *next=NULL;
  for (QListViewItem *item=source->firstChild();item;item=next) {
    toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(item);
    
    if (chk&&chk->isEnabled()) {
      QString name=chk->text(0);
      QString type=chk->text(1);
      QString user=chk->text(2);
      if (!user.isEmpty()) {
	if (chk->isOn()&&chk->isEnabled()) {
	  QString line;
	  if (type==QString::fromLatin1("TABLE")) {
	    line=user;
	    line+=QString::fromLatin1(".");
	    line+=name;
	    toPush(tables,line);
	  } else {
	    line=type;
	    line+=QString::fromLatin1(":");
	    line+=user;
	    line+=QString::fromLatin1(".");
	    line+=name;
	    if (type==QString::fromLatin1("VIEW"))
	      toPush(userViews,line);
	    else
	      toPush(userOther,line);
	  }
	}
      } else if (!type.isEmpty()) {
	if (chk->isOn()&&chk->isEnabled()) {
	  QString line=type;
	  line+=QString::fromLatin1(":");
	  line+=name;
	  if (type==QString::fromLatin1("TABLESPACE"))
	    toPush(tableSpace,line);
	  else if (type==QString::fromLatin1("PROFILE"))
	    toPush(profiles,line);
	  else if (type==QString::fromLatin1("ROLE"))
	    toPush(roles,name);
	  else if (type==QString::fromLatin1("USER"))
	    toPush(users,name);
	  else
	    toPush(otherGlobal,line);
	}
      }
    }
    
    if (item->firstChild()&&chk&&chk->isEnabled())
      next=item->firstChild();
    else if (item->nextSibling())
      next=item->nextSibling();
    else {
      next=item;
      do {
	next=next->parent();
      } while(next&&!next->nextSibling());
      if (next)
	next=next->nextSibling();
    }
  }
  
  if (ScriptUI->IncludeDDL->isChecked()) {
    {
      for(std::list<QString>::iterator i=tableSpace.begin();i!=tableSpace.end();i++)
	toPush(lst,*i);
    }
    {
      for(std::list<QString>::iterator i=profiles.begin();i!=profiles.end();i++)
	toPush(lst,*i);
    }
    {
      for(std::list<QString>::iterator i=otherGlobal.begin();i!=otherGlobal.end();i++)
	toPush(lst,*i);
    }
    {
      for(std::list<QString>::iterator i=roles.begin();i!=roles.end();i++) {
	QString line=QString::fromLatin1("ROLE:");
	line+=*i;
	toPush(lst,line);
      }
    }
    {
      for(std::list<QString>::iterator i=users.begin();i!=users.end();i++) {
	QString line=QString::fromLatin1("USER:");
	line+=*i;
	toPush(lst,line);
      }
    }
    {
      for(std::list<QString>::iterator i=tables.begin();i!=tables.end();i++) {
	QString line=QString::fromLatin1("TABLE FAMILY:");
	line+=*i;
	toPush(lst,line);
      }
    }
    {
      for(std::list<QString>::iterator i=userViews.begin();i!=userViews.end();i++)
	toPush(lst,*i);
    }
    {
      for(std::list<QString>::iterator i=userOther.begin();i!=userOther.end();i++)
	toPush(lst,*i);
    }
  }
  for(std::list<QString>::iterator i=tables.begin();i!=tables.end();i++) {
    QString line=QString::fromLatin1("TABLE CONTENTS:");
    line+=*i;
    toPush(lst,line);
  }
  if (ScriptUI->IncludeDDL->isChecked()) {
    {
      for(std::list<QString>::iterator i=tables.begin();i!=tables.end();i++) {
	QString line=QString::fromLatin1("TABLE REFERENCES:");
	line+=*i;
	toPush(lst,line);
      }
    }
    {
      for(std::list<QString>::iterator i=roles.begin();i!=roles.end();i++) {
	QString line=QString::fromLatin1("ROLE GRANTS:");
	line+=*i;
	toPush(lst,line);
      }
    }
    {
      for(std::list<QString>::iterator i=users.begin();i!=users.end();i++) {
	QString line=QString::fromLatin1("USER GRANTS:");
	line+=*i;
	toPush(lst,line);
      }
    }
  }
  return lst;
}

void toScript::execute(void)
{
  try {
    int mode;
    if (ScriptUI->Compare->isChecked())
      mode=0;
    else if (ScriptUI->Extract->isChecked())
      mode=1;
    else if (ScriptUI->Migrate->isChecked())
      mode=2;
    else if (ScriptUI->Search->isChecked())
      mode=3;
    else if (ScriptUI->Report->isChecked())
      mode=4;
    else {
      toStatusMessage(tr("No mode selected"));
      return;
    }

    std::list<QString> sourceObjects=createObjectList(ScriptUI->SourceObjects);

    std::list<QString> sourceDescription;
    std::list<QString> destinationDescription;
    QString script;

    toExtract source(toMainWidget()->connection(ScriptUI->SourceConnection->currentText()),this);
    setupExtract(source);
    switch(mode) {
    case 1:
      if (ScriptUI->OutputTab->isChecked())
	script+=source.create(sourceObjects);
      else if (ScriptUI->OutputFile->isChecked()) {
	if (ScriptUI->Filename->text().isEmpty())
	  throw tr("No filename specified");

	QFile file(ScriptUI->Filename->text());
	file.open(IO_WriteOnly);

	if (file.status()!=IO_Ok)
	  throw tr("Couldn't open file %1").arg(file.name());

	QTextStream stream(&file);
	source.create(stream,sourceObjects);

	if (file.status()!=IO_Ok)
	  throw tr("Error writing to file %1").arg(file.name());

	script=tr("-- Script generated to file %1 successfully").arg(ScriptUI->Filename->text());
      } else if (ScriptUI->OutputDir->isChecked()) {
	if (ScriptUI->Filename->text().isEmpty())
	  throw tr("No filename specified");

	QFile file(ScriptUI->Filename->text()+QDir::separator()+"script.sql");
	file.open(IO_WriteOnly);

	if (file.status()!=IO_Ok)
	  throw QString(tr("Couldn't open file %1")).arg(file.name());

	QTextStream stream(&file);
	stream<<tr("rem Master script for DDL reverse engineering by TOra\n"
		   "\n");
	QRegExp repl("\\W+");
	for(std::list<QString>::iterator i=sourceObjects.begin();i!=sourceObjects.end();i++) {
	  std::list<QString> t;
	  t.insert(t.end(),*i);
	  QString fn=*i;
	  fn.replace(repl,"_");
	  fn+=".sql";
	  stream<<"@"<<fn<<"\n";

	  QFile tf(ScriptUI->Filename->text()+QDir::separator()+fn);
	  tf.open(IO_WriteOnly);

	  if (tf.status()!=IO_Ok)
	    throw QString(tr("Couldn't open file %1")).arg(tf.name());

	  QTextStream ts(&tf);
	  source.create(ts,t);

	  if (tf.status()!=IO_Ok)
	    throw QString(tr("Error writing to file %1")).arg(tf.name());


	  script=tr("-- Scripts generate to directory %1 successfully").arg(ScriptUI->Filename->text());;
	}

	if (file.status()!=IO_Ok)
	  throw QString(tr("Error writing to file %1")).arg(file.name());
      }
      break;
    case 0:
    case 2:
    case 3:
    case 4:
      sourceDescription=source.describe(sourceObjects);
      break;
    }

    if (ScriptUI->Destination->isEnabled()) {
      std::list<QString> destinationObjects=createObjectList(ScriptUI->DestinationObjects);
      toExtract destination(toMainWidget()->connection(ScriptUI->
						       DestinationConnection->
						       currentText()),this);
      setupExtract(destination);
      switch(mode) {
      case 0:
      case 2:
	destinationDescription=destination.describe(destinationObjects);
	break;
      case 1:
      case 3:
	throw tr("Destination shouldn't be enabled now, internal error");
      }

      std::list<QString> drop;
      std::list<QString> create;

      toExtract::srcDst2DropCreate(sourceDescription,destinationDescription,
				   drop,create);
      sourceDescription=drop;
      destinationDescription=create;
    }
    ScriptUI->Tabs->setTabEnabled(ScriptUI->ResultTab,mode==1||mode==2||mode==3||mode==4);
    ScriptUI->Tabs->setTabEnabled(ScriptUI->DifferenceTab,mode==0||mode==2);
    if (!script.isEmpty()) {
      Worksheet->editor()->setText(script);
      Worksheet->editor()->setFilename(QString::null);
      Worksheet->editor()->setEdited(true);
    }
    if (mode==3) {
      Worksheet->hide();
      Report->hide();
      SearchList->show();
      QRegExp re(ScriptUI->SearchWord->text(),false);
      QStringList words(QStringList::split(QRegExp(QString::fromLatin1(" ")),
						   ScriptUI->SearchWord->text().
						   upper().simplifyWhiteSpace()));
      QString word=ScriptUI->SearchWord->text().upper();
      int searchMode=0;
      if (ScriptUI->AllWords->isChecked())
	searchMode=1;
      else if (ScriptUI->AnyWords->isChecked())
	searchMode=2;
      else if (ScriptUI->RegExp->isChecked())
	searchMode=3;
      else if (ScriptUI->ExactMatch->isChecked())
	searchMode=4;
      std::list<QString> result;
      for(std::list<QString>::iterator i=sourceDescription.begin();
	  i!=sourceDescription.end();
	  i++) {
	QStringList ctx=QStringList::split(QString::fromLatin1("\01"),(*i).upper());
	switch(searchMode) {
	case 1:
	case 2:
	  {
	    unsigned int count=0;
	    for(unsigned int k=0;k<words.count();k++) {
	      QString s=words[k];
	      if (ctx.last().contains(s))
		count++;
	    }
	    if ((searchMode==2&&count>0)||(searchMode==1&&count==words.count()))
	      result.insert(result.end(),*i);
	  }
	  break;
	case 4:
	  if (ctx.last()==word)
	    result.insert(result.end(),*i);
	  break;
	case 3:
	  if (re.match(ctx.last())>=0)
	    result.insert(result.end(),*i);
	  break;
	}
      }
      fillDifference(result,SearchList);
    } else if (mode==4) {
      Worksheet->hide();
      SearchList->hide();
      Report->show();
      QString res=toGenerateReport(source.connection(),sourceDescription);
      Report->setText(res);
      if (ScriptUI->OutputFile->isChecked()) {
	if (ScriptUI->Filename->text().isEmpty())
	  toStatusMessage(tr("No filename specified"));
	else {
	  QFile file(ScriptUI->Filename->text());
	  file.open(IO_WriteOnly);

	  if (file.status()!=IO_Ok)
	    toStatusMessage(tr("Couldn't open file %1").arg(file.name()));
	  else {
	    QTextStream stream(&file);
	    stream<<res;

	    if (file.status()!=IO_Ok)
	      toStatusMessage(tr("Error writing to file %1").arg(file.name()));
	  }
	}
      }
    } else {
      Worksheet->show();
      SearchList->hide();
      Report->hide();
      fillDifference(sourceDescription,DropList);
      fillDifference(destinationDescription,CreateList);
    }
    if (mode==0)
      ScriptUI->Tabs->showPage(ScriptUI->DifferenceTab);
    else
      ScriptUI->Tabs->showPage(ScriptUI->ResultTab);
  } TOCATCH
}

void toScript::fillDifference(std::list<QString> &objects,QListView *view)
{
  view->clear();
  QListViewItem *last=NULL;
  int lastLevel=0;
  QStringList lstCtx;
  for(std::list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
    //    printf("Adding %s\n",(const char *)*i);
    QStringList ctx=QStringList::split(QString::fromLatin1("\01"),*i);
    if (last) {
      while(last&&lastLevel>=int(ctx.count())) {
	last=last->parent();
	lastLevel--;
      }
      while(last&&lastLevel>=0&&!toCompareLists(lstCtx,ctx,(unsigned int)lastLevel)) {
	last=last->parent();
	lastLevel--;
      }
    }
    if (lastLevel<0)
      throw tr("Internal error, lastLevel < 0");
    while(lastLevel<int(ctx.count())-1) {
      if (last)
	last=new toResultViewMLine(last,NULL,ctx[lastLevel]);
      else
	last=new toResultViewMLine(view,NULL,ctx[lastLevel]);
      lastLevel++;
    }
    QCheckListItem *item;
    if (last)
      item=new toResultViewMLCheck(last,ctx[lastLevel],QCheckListItem::CheckBox);
    else
      item=new toResultViewMLCheck(view,ctx[lastLevel],QCheckListItem::CheckBox);
    last=item;
    item->setOn(true);
    lstCtx=ctx;
    lastLevel++;
  }
}

void toScript::changeConnection(int,bool source)
{
  try {
    QListView *sourceL=NULL;
    QListView *destinationL=NULL;
    if (ScriptUI->SourceConnection->currentText()==
	ScriptUI->DestinationConnection->currentText()) {
      if (source) {
	destinationL=ScriptUI->SourceObjects;
	sourceL=ScriptUI->DestinationObjects;
      } else {
	sourceL=ScriptUI->SourceObjects;
	destinationL=ScriptUI->DestinationObjects;
      }
    }
    if (sourceL&&destinationL&&sourceL->firstChild()) {
      destinationL->clear();
      QListViewItem *next=NULL;
      QListViewItem *parent=NULL;
      for (QListViewItem *item=sourceL->firstChild();item;item=next) {
	QListViewItem *lastParent=parent;
	if (!parent) {
	  parent=new toResultViewCheck(destinationL,item->text(0),
				       QCheckListItem::CheckBox);
	  parent->setExpandable(true);
	} else
	  parent=new toResultViewCheck(parent,item->text(0),
				       QCheckListItem::CheckBox);
	parent->setText(1,item->text(1));
	parent->setText(2,item->text(2));
	if (item->firstChild())
	  next=item->firstChild();
	else if (item->nextSibling()) {
	  next=item->nextSibling();
	  parent=lastParent;
	} else {
	  next=item;
	  parent=lastParent;
	  do {
	    next=next->parent();
	    if (parent)
	      parent=parent->parent();
	  } while(next&&!next->nextSibling());
	  if (next)
	    next=next->nextSibling();
	}
      }
      return;
    }
    (source?ScriptUI->SourceObjects:ScriptUI->DestinationObjects)->clear();
    (source?ScriptUI->SourceSchema:ScriptUI->DestinationSchema)->clear();
    (source?ScriptUI->SourceSchema:ScriptUI->DestinationSchema)->insertItem(tr("All"));
    toConnection &conn=toMainWidget()->connection((source?
						   ScriptUI->SourceConnection:
						   ScriptUI->DestinationConnection)
						  ->currentText());
    toQList object;
    try {
      object=toQuery::readQueryNull(conn,SQLObjectList);
    } catch(...) {
      object=toQuery::readQueryNull(conn,SQLUserObjectList);
    }
    toQList schema=toQuery::readQuery(conn,SQLSchemas);
    while(schema.size()>0) {
      QString str=toShift(schema);
      (source?ScriptUI->SourceSchema:ScriptUI->DestinationSchema)->insertItem(str);
    }
    QListViewItem *lastTop=NULL;
    QListViewItem *lastFirst=NULL;
    while(object.size()>0) {
      QString top=toShift(object);
      QString first=toShift(object);
      QString second=toShift(object);

      if (top!=(lastTop?lastTop->text(0):QString::null)) {
	lastFirst=NULL;
	lastTop=new toResultViewCheck((source?
				       ScriptUI->SourceObjects:
				       ScriptUI->DestinationObjects),
				      top,QCheckListItem::CheckBox);
	lastTop->setExpandable(true);
	if (!second.isEmpty()||first.isEmpty())
	  lastTop->setText(1,QString::fromLatin1("USER"));
      }
      if (first!=(lastFirst?lastFirst->text(0):QString::null)&&!first.isEmpty()) {
	lastFirst=new toResultViewCheck(lastTop,first,QCheckListItem::CheckBox);
	if (second.isEmpty())
	  lastFirst->setText(1,top);
      }
      if (!second.isEmpty()&&lastFirst) {
	QListViewItem *item=new toResultViewCheck(lastFirst,second,QCheckListItem::CheckBox);
	item->setText(1,first);
	item->setText(2,top);
      }
    }
  } TOCATCH
}

void toScript::readOwnerObjects(QListViewItem *item,toConnection &conn)
{
  if (!item->parent()&&!item->firstChild()) {
    try {
      QListViewItem *lastFirst=NULL;
      QString top=item->text(0);
      toQList object=toQuery::readQueryNull(conn,SQLUserObjects,top);

      while(object.size()>0) {
	QString first=toShift(object);
	QString second=toShift(object);

	if (first!=(lastFirst?lastFirst->text(0):QString::null)&&!first.isEmpty()) {
	  lastFirst=new toResultViewCheck(item,first,QCheckListItem::CheckBox);
	  if (second.isEmpty())
	    lastFirst->setText(1,top);
	}
	if (!second.isEmpty()&&lastFirst) {
	  QListViewItem *item=new toResultViewCheck(lastFirst,second,QCheckListItem::CheckBox);
	  item->setText(1,first);
	  item->setText(2,top);
	}
      }

      if (top==QString::fromLatin1("PUBLIC")) {
	object=toQuery::readQueryNull(conn,SQLPublicSynonymList);
	QListViewItem *topItem=new toResultViewCheck(item,QString::fromLatin1("SYNONYM"),QCheckListItem::CheckBox);
	while(object.size()>0) {
	  QListViewItem *item=new toResultViewCheck(topItem,toShift(object),
						    QCheckListItem::CheckBox);
	  item->setText(1,QString::fromLatin1("SYNONYM"));
	  item->setText(2,top);
	}
      }
    } TOCATCH
  }
}

void toScript::changeMode(int mode)
{
  if (mode<0||mode>4)
    return;

  if (mode==0||mode==2)
    ScriptUI->Destination->setEnabled(true);
  else if (mode==1||mode==3||mode==4)
    ScriptUI->Destination->setEnabled(false);

  if (mode==1||mode==2)
    ScriptUI->Tabs->setTabEnabled(ScriptUI->ResizeTab,true);
  else if (mode==0||mode==3||mode==4)
    ScriptUI->Tabs->setTabEnabled(ScriptUI->ResizeTab,false);

  if (mode==1)
    ScriptUI->IncludeContent->setEnabled(true);
  else if (mode==0||mode==2||mode==3||mode==4)
    ScriptUI->IncludeContent->setEnabled(false);

  if (mode==1||mode==2) {
    ScriptUI->IncludeHeader->setEnabled(true);
    ScriptUI->IncludePrompt->setEnabled(true);
  } else if (mode==0||mode==3||mode==4) {
    ScriptUI->IncludeHeader->setEnabled(false);
    ScriptUI->IncludePrompt->setEnabled(false);
  }

  if (mode==0||mode==2||mode==3||mode==4) {
    ScriptUI->IncludeDDL->setEnabled(false);
    ScriptUI->IncludeDDL->setChecked(true);
  } else if (mode==1)
    ScriptUI->IncludeDDL->setEnabled(true);

  ScriptUI->OutputGroup->setEnabled(mode==1||mode==2||mode==4);

  ScriptUI->IncludeConstraints->setEnabled(ScriptUI->IncludeDDL->isChecked());
  ScriptUI->IncludeIndexes->setEnabled(ScriptUI->IncludeDDL->isChecked());
  ScriptUI->IncludeGrants->setEnabled(ScriptUI->IncludeDDL->isChecked());
  ScriptUI->IncludeStorage->setEnabled(ScriptUI->IncludeDDL->isChecked()&&mode!=3);
  ScriptUI->IncludeParallell->setEnabled(ScriptUI->IncludeDDL->isChecked()&&mode!=3);
  ScriptUI->IncludePartition->setEnabled(ScriptUI->IncludeDDL->isChecked()&&mode!=3);
  ScriptUI->IncludeCode->setEnabled(ScriptUI->IncludeDDL->isChecked());
  ScriptUI->IncludeComment->setEnabled(ScriptUI->IncludeDDL->isChecked());
}

void toScript::keepOn(QListViewItem *parent)
{
  if (!parent)
    return;
  toResultViewCheck *pchk=dynamic_cast<toResultViewCheck *>(parent);
  if (!pchk)
    return;
  pchk->setOn(true);
}

void toScript::objectClicked(QListViewItem *parent)
{
  if (!parent)
    return;
  toResultViewCheck *pchk=dynamic_cast<toResultViewCheck *>(parent);
  if (!pchk)
    return;
  if (!parent->parent()&&!parent->firstChild()) {
    if (parent->listView()==ScriptUI->SourceObjects)
      expandSource(parent);
    else if (parent->listView()==ScriptUI->DestinationObjects)
      expandDestination(parent);
  }
  bool on=pchk->isOn();
  QListViewItem *next=NULL;
  for (QListViewItem *item=parent->firstChild();item;item=next) {
    toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(item);
    if (chk)
      chk->setOn(on);

    if (item->firstChild())
      next=item->firstChild();
    else if (item->nextSibling())
      next=item->nextSibling();
    else {
      next=item;
      do {
	next=next->parent();
	if(next==parent)
	  return;
      } while(next&&!next->nextSibling());
      if (next)
	next=next->nextSibling();
    }
  }
}

void toScript::changeSchema(int,bool source)
{
  QString src=(source?ScriptUI->SourceSchema:ScriptUI->DestinationSchema)->currentText();
  for(QListViewItem *parent=(source?
			     ScriptUI->SourceObjects:
			     ScriptUI->DestinationObjects)->firstChild();
      parent;
      parent=parent->nextSibling()) {
    toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(parent);
    if (chk) {
      bool ena=((src==chk->text(0))||(src==QString::fromLatin1("All")));

      QListViewItem *next=NULL;
      for (QListViewItem *item=parent;item;item=next) {
	chk=dynamic_cast<toResultViewCheck *>(item);
	if (chk) {
	  chk->setEnabled(ena);
	  (source?ScriptUI->SourceObjects:ScriptUI->DestinationObjects)->repaintItem(chk);
	}

	if (item->firstChild())
	  next=item->firstChild();
	else if (item->nextSibling())
	  next=item->nextSibling();
	else {
	  next=item;
	  do {
	    next=next->parent();
	    if(next==parent)
	      break;
	  } while(next&&!next->nextSibling());
	  if (next==parent)
	    break;
	  if (next)
	    next=next->nextSibling();
	}
      }

    }
  }
}

void toScript::newSize(void)
{
  QString init=ScriptUI->Initial->sizeString();
  QString next=ScriptUI->Next->sizeString();
  QString max=ScriptUI->Limit->sizeString();
  QString maxNum;
  maxNum.sprintf("%010d",ScriptUI->Limit->value());

  for (QListViewItem *item=ScriptUI->Sizes->firstChild();item;item=item->nextSibling())
    if (max==item->text(0)) {
      toStatusMessage(tr("Replacing existing size with new"),false,false);
      delete item;
      break;
    }
  
  new QListViewItem(ScriptUI->Sizes,max,init,next,maxNum);
  ScriptUI->Sizes->setSorting(3);
}

void toScript::removeSize(void)
{
  QListViewItem *item=ScriptUI->Sizes->selectedItem();
  if (item)
    delete item;
}

void toScript::setupExtract(toExtract &extr)
{
  extr.setCode       (ScriptUI->IncludeCode->isEnabled()       &&
		      ScriptUI->IncludeCode->isChecked()       );
  extr.setComments   (ScriptUI->IncludeComment->isEnabled()    &&
		      ScriptUI->IncludeComment->isChecked()    );
  extr.setConstraints(ScriptUI->IncludeConstraints->isEnabled()&&
		      ScriptUI->IncludeConstraints->isChecked());
  extr.setContents   (ScriptUI->IncludeContent->isEnabled()    &&
		      ScriptUI->IncludeContent->isChecked()    );
  extr.setGrants     (ScriptUI->IncludeGrants->isEnabled()     &&
		      ScriptUI->IncludeGrants->isChecked()     );
  extr.setHeading    (ScriptUI->IncludeHeader->isEnabled()     &&
		      ScriptUI->IncludeHeader->isChecked()     );
  extr.setIndexes    (ScriptUI->IncludeIndexes->isEnabled()    &&
		      ScriptUI->IncludeIndexes->isChecked()    );
  extr.setParallel   (ScriptUI->IncludeParallell->isEnabled()  &&
		      ScriptUI->IncludeParallell->isChecked()  );
  extr.setPartition  (ScriptUI->IncludePartition->isEnabled()  &&
		      ScriptUI->IncludePartition->isChecked()  );
  extr.setPrompt     (ScriptUI->IncludePrompt->isEnabled()     &&
		      ScriptUI->IncludePrompt->isChecked()     );
  extr.setStorage    (ScriptUI->IncludeStorage->isEnabled()    &&
		      ScriptUI->IncludeStorage->isChecked()    );

  if (ScriptUI->Schema->currentText()==tr("Same"))
    extr.setSchema(QString::fromLatin1("1"));
  else if (ScriptUI->Schema->currentText()==tr("None"))
    extr.setSchema(QString::null);
  else
    extr.setSchema(ScriptUI->Schema->currentText());

  if (ScriptUI->DontResize->isChecked())
    extr.setResize(QString::null);
  else if (ScriptUI->AutoResize->isChecked())
    extr.setResize(QString::fromLatin1("1"));
  else {
    QString siz;
    for (QListViewItem *item=ScriptUI->Sizes->firstChild();item;item=item->nextSibling()) {
      siz+=item->text(0);
      siz+=QString::fromLatin1(":");
      siz+=item->text(1);
      siz+=QString::fromLatin1(":");
      siz+=item->text(2);
      if (item->nextSibling())
	siz+=QString::fromLatin1(":");
    }
    extr.setResize(siz);
  }
}

void toScript::expandSource(QListViewItem *item)
{
  try {
    if (item)
      readOwnerObjects(item,
		       toMainWidget()->connection(ScriptUI->SourceConnection->currentText()));
  } TOCATCH
}

void toScript::expandDestination(QListViewItem *item)
{
  try {
    if (item)
      readOwnerObjects(item,
		       toMainWidget()->connection(ScriptUI->DestinationConnection->currentText()));
  } TOCATCH
}

void toScript::browseFile(void)
{
  if (ScriptUI->OutputFile->isChecked()) {
    QString f=toOpenFilename(QString::null,QString::null,this);
    if (!f.isEmpty())
      ScriptUI->Filename->setText(f);
  } else if (ScriptUI->OutputDir->isChecked()) {
    QString f=TOFileDialog::getExistingDirectory(QString::null,this);
    if (!f.isEmpty())
      ScriptUI->Filename->setText(f);
  }
}
