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
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include "tofilesize.h"
#include "toresultview.h"
#include "toscript.h"
#include "toscriptui.h"
#include "totool.h"
#include "toworksheet.h"
#include "toextract.h"
#include "tosql.h"

#include "toscript.moc"
#include "toscriptui.moc"

#include "icons/toscript.xpm"
#include "icons/execute.xpm"

static QPixmap *ExecutePixmap;

class toScriptTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return toscript_xpm; }
public:
  toScriptTool()
    : toTool(100,"Schema extraction")
  { }
  virtual const char *menuItem()
  { return "DB Extraction/Compare"; }
  virtual const char *toolbarTip()
  { return "DB or schema extraction and compare"; }
  virtual QWidget *toolWindow(QWidget *main,toConnection &connection)
  {
    QVBox *window=new QVBox(main);
    window->setIcon(*toolbarImage());

    if (!ExecutePixmap)
      ExecutePixmap=new QPixmap((const char **)execute_xpm);
    QToolBar *toolbar=toAllocBar(window,"SQL worksheet",connection.connectString());
    toScript *script=new toScript(window,connection);

    new QToolButton(*ExecutePixmap,
		    "Perform defined extraction",
		    "Perform defined extraction",
		    script,SLOT(execute(void)),
		    toolbar);
    toolbar->setStretchableWidget(new QLabel("",toolbar));

    return window;
  }
};

static toScriptTool ScriptTool;

static toSQL SQLObjectList("toScript:ExtractObject",
			   "SELECT *\n"
			   "  FROM (SELECT 'TABLESPACE',tablespace_name,NULL\n"
			   "	  FROM dba_tablespaces\n"
			   "	UNION\n"
			   "	SELECT 'ROLE',role,NULL\n"
			   "	  FROM dba_roles\n"
			   "	UNION\n"
			   "	SELECT 'PUBLIC','SYNONYM',synonym_name\n"
			   "	  FROM all_synonyms WHERE owner = 'PUBLIC'\n"
			   "	UNION\n"
			   "	SELECT owner,'DATABASE LINK',db_link\n"
			   "	  FROM all_db_links\n"
			   "	UNION\n"
			   "	SELECT owner,object_type,object_name\n"
			   "	  FROM all_objects\n"
			   "	 WHERE object_type IN ('VIEW','TYPE','SEQUENCE','PACKAGE',\n"
			   "			       'PACKAGE BODY','FUNCTION','PROCEDURE')\n"
			   "	UNION\n"
			   "	SELECT owner,'TABLE',table_name\n"
			   "	  FROM all_tables\n"
			   "	 WHERE temporary != 'Y' AND secondary = 'N' AND iot_name IS NULL\n"
			   "	UNION\n"
			   "	SELECT owner,'MATERIALIZED TABLE',mview_name AS object\n"
			   "	  FROM all_mviews\n"
			   "	UNION\n"
			   "	SELECT username,NULL,NULL\n"
			   "	  FROM all_users)\n"
			   "  ORDER BY 1,2,3",
			   "Extract objects available to extract from the database, "
			   "should have same columns",
			   "8.1");

static toSQL SQLObjectList7("toScript:ExtractObject",
			    "SELECT *\n"
			    "  FROM (SELECT 'TABLESPACE',tablespace_name,NULL\n"
			    "	  FROM dba_tablespaces\n"
			    "	UNION\n"
			    "	SELECT 'ROLE',role,NULL\n"
			    "	  FROM dba_roles\n"
			    "	UNION\n"
			    "	SELECT 'PUBLIC','SYNONYM',synonym_name\n"
			    "	  FROM all_synonyms WHERE owner = 'PUBLIC'\n"
			    "	UNION\n"
			    "	SELECT owner,'DATABASE LINK',db_link\n"
			    "	  FROM all_db_links\n"
			    "	UNION\n"
			    "	SELECT owner,object_type,object_name\n"
			    "	  FROM all_objects\n"
			    "	 WHERE object_type IN ('VIEW','TYPE','SEQUENCE','PACKAGE',\n"
			    "			       'PACKAGE BODY','FUNCTION','PROCEDURE')\n"
			    "	UNION\n"
			    "	SELECT owner,'TABLE',table_name\n"
			    "	  FROM all_tables\n"
			    "	 WHERE temporary != 'Y' AND secondary = 'N'\n"
			    "	UNION\n"
			    "	SELECT owner,'MATERIALIZED TABLE',mview_name AS object\n"
			    "	  FROM all_mviews\n"
			    "	UNION\n"
			    "	SELECT username,NULL,NULL\n"
			    "	  FROM all_users)\n"
			    "  ORDER BY 1,2,3",
			    QString::null,
			    "7.0");

static toSQL SQLUserObjectList("toScript:UserExtractObject",
			       "SELECT owner,object_type,object_name\n"
			       "  FROM all_objects\n"
			       " WHERE object_type IN ('VIEW','TABLE','TYPE','SEQUENCE','PACKAGE',\n"
			       "	               'PACKAGE BODY','FUNCTION','PROCEDURE')\n"
			       " ORDER BY 1,2,3",
			       "Extract objects available to extract from the database if you "
			       "don't have admin access, should have same columns");

static toSQL SQLSchemas("toScript:ExtractSchema",
			"SELECT username FROM all_users ORDER BY username",
			"Get usernames available in database, must have same columns");

toScript::toScript(QWidget *parent,toConnection &connection)
  : toScriptUI(parent), Connection(connection)
{
  QSplitter *hsplitter=new QSplitter(Horizontal,DifferenceTab);
  Worksheet=new toWorksheet(ResultTab,Connection);
  DropList=new toResultView(false,false,Connection,hsplitter);
  DropList->addColumn("Dropped");
  DropList->setRootIsDecorated(true);
  DropList->setSorting(0);
  CreateList=new toResultView(false,false,Connection,hsplitter);
  CreateList->addColumn("Created");
  CreateList->setRootIsDecorated(true);
  CreateList->setSorting(0);
  Tabs->setTabEnabled(ResultTab,false);
  Tabs->setTabEnabled(DifferenceTab,false);

  QGridLayout *layout=new QGridLayout(ResultTab);
  layout->addWidget(Worksheet,0,0);
  layout=new QGridLayout(DifferenceTab);
  layout->addWidget(hsplitter,0,0);

  Initial->setTitle("&Initial");
  Limit->setTitle("&Limit");
  Next->setTitle("&Next");
  connect(ModeGroup,SIGNAL(clicked(int)),this,SLOT(changeMode(int)));
  Tabs->setTabEnabled(ResizeTab,false);
  SourceObjects->setSorting(0);
  DestinationObjects->setSorting(0);

  int def=0;
  list<QString> cons=toMainWidget()->connections();
  int i=0;
  while(cons.size()>0) {
    QString str=toShift(cons);
    if(str==Connection.connectString()&&def==0)
      def=i;
    i++;
    SourceConnection->insertItem(str);
    DestinationConnection->insertItem(str);
  }
  SourceConnection->setCurrentItem(def);
  changeSource(def);
  changeDestination(def);
  DestinationConnection->setCurrentItem(def);

  connect(SourceConnection,SIGNAL(activated(int)),this,SLOT(changeSource(int)));
  connect(DestinationConnection,SIGNAL(activated(int)),this,SLOT(changeDestination(int)));
  connect(SourceSchema,SIGNAL(activated(int)),this,SLOT(changeSourceSchema(int)));
  connect(DestinationSchema,SIGNAL(activated(int)),this,SLOT(changeDestinationSchema(int)));
  connect(SourceObjects,SIGNAL(clicked(QListViewItem *)),this,SLOT(objectClicked(QListViewItem *)));
  connect(DestinationObjects,SIGNAL(clicked(QListViewItem *)),this,SLOT(objectClicked(QListViewItem *)));

  Schema->setCurrentItem(0);

  Connection.addWidget(this);
}

toScript::~toScript()
{
  Connection.delWidget(this);
}

list<QString> toScript::createObjectList(QListView *source)
{
  list<QString> lst;

  list<QString> otherGlobal;
  list<QString> profiles;
  list<QString> roles;
  list<QString> tableSpace;
  list<QString> tables;
  list<QString> userOther;
  list<QString> userViews;
  list<QString> users;

  QListViewItem *next=NULL;
  for (QListViewItem *item=source->firstChild();item;item=next) {
    QCheckListItem *chk=dynamic_cast<QCheckListItem *>(item);
    
    if (chk&&chk->isEnabled()) {
      QString name=chk->text(0);
      QString type=chk->text(1);
      QString user=chk->text(2);
      if (!user.isEmpty()) {
	if (chk->isOn()&&chk->isEnabled()) {
	  QString line;
	  if (type=="TABLE") {
	    line=user;
	    line+=".";
	    line+=name;
	    toPush(tables,line);
	  } else {
	    line=type;
	    line+=":";
	    line+=user;
	    line+=".";
	    line+=name;
	    if (type=="VIEW")
	      toPush(userViews,line);
	    else
	      toPush(userOther,line);
	  }
	}
      } else if (!type.isEmpty()) {
	if (chk->isOn()&&chk->isEnabled()) {
	  QString line=type;
	  line+=":";
	  line+=name;
	  if (type=="TABLESPACE")
	    toPush(tableSpace,line);
	  else if (type=="PROFILE")
	    toPush(profiles,line);
	  else if (type=="ROLE")
	    toPush(roles,name);
	  else if (type=="USER")
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
  
  if (IncludeDDL->isChecked()) {
    {
      for(list<QString>::iterator i=tableSpace.begin();i!=tableSpace.end();i++)
	toPush(lst,*i);
    }
    {
      for(list<QString>::iterator i=profiles.begin();i!=profiles.end();i++)
	toPush(lst,*i);
    }
    {
      for(list<QString>::iterator i=otherGlobal.begin();i!=otherGlobal.end();i++)
	toPush(lst,*i);
    }
    {
      for(list<QString>::iterator i=roles.begin();i!=roles.end();i++) {
	QString line="ROLE:";
	line+=*i;
	toPush(lst,line);
      }
    }
    {
      for(list<QString>::iterator i=users.begin();i!=users.end();i++) {
	QString line="USER:";
	line+=*i;
	toPush(lst,line);
      }
    }
    {
      for(list<QString>::iterator i=tables.begin();i!=tables.end();i++) {
	QString line="TABLE FAMILY:";
	line+=*i;
	toPush(lst,line);
      }
    }
    {
      for(list<QString>::iterator i=userViews.begin();i!=userViews.end();i++)
	toPush(lst,*i);
    }
    {
      for(list<QString>::iterator i=userOther.begin();i!=userOther.end();i++)
	toPush(lst,*i);
    }
  }
  for(list<QString>::iterator i=tables.begin();i!=tables.end();i++) {
    QString line="TABLE CONTENTS:";
    line+=*i;
    toPush(lst,line);
  }
  if (IncludeDDL->isChecked()) {
    {
      for(list<QString>::iterator i=tables.begin();i!=tables.end();i++) {
	QString line="TABLE REFERENCES:";
	line+=*i;
	toPush(lst,line);
      }
    }
    {
      for(list<QString>::iterator i=roles.begin();i!=roles.end();i++) {
	QString line="ROLE GRANTS:";
	line+=*i;
	toPush(lst,line);
      }
    }
    {
      for(list<QString>::iterator i=users.begin();i!=users.end();i++) {
	QString line="USER GRANTS:";
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
    if (Compare->isChecked())
      mode=0;
    else if (Extract->isChecked())
      mode=1;
    else if (Migrate->isChecked())
      mode=2;
    else if (Resize->isChecked())
      mode=3;
    else {
      toStatusMessage("No mode selected");
      return;
    }

    list<QString> sourceObjects=createObjectList(SourceObjects);

    list<QString> sourceDescription;
    list<QString> destinationDescription;
    QString script;

    toExtract source(toMainWidget()->connection(SourceConnection->currentText()),this);
    setupExtract(source);
    switch(mode) {
    case 1:
      script+=source.create(sourceObjects);
      break;
    case 0:
    case 2:
      sourceDescription=source.describe(sourceObjects);
      break;
    case 3:
      break;
    }

    if (Destination->isEnabled()) {
      list<QString> destinationObjects=createObjectList(DestinationObjects);
      toExtract destination(toMainWidget()->connection(DestinationConnection->currentText()),this);
      setupExtract(destination);
      switch(mode) {
      case 0:
      case 2:
	destinationDescription=destination.describe(destinationObjects);
	break;
      case 1:
      case 3:
	throw QString ("Destination shouldn't be enabled now, internal error");
      }

      // Remove entries existing in both source and destination
      list<QString>::iterator i=sourceDescription.begin();
      list<QString>::iterator j=destinationDescription.begin();
      while(i!=sourceDescription.end()&&j!=destinationDescription.end()) {
	if (*i==*j) {
	  sourceDescription.erase(i);
	  destinationDescription.erase(j);
	  i=sourceDescription.begin();
	  j=destinationDescription.begin();
	} else if (*i<*j)
	  i++;
	else
	  j++;
      }
    }
    Tabs->setTabEnabled(ResultTab,mode==1||mode==2||mode==3);
    Tabs->setTabEnabled(DifferenceTab,mode==0||mode==2);
    if (!script.isEmpty()) {
      Worksheet->editor()->setText(script);
      Worksheet->editor()->setFilename(QString::null);
      Worksheet->editor()->setEdited(true);
    }
    fillDifference(sourceDescription,DropList);
    fillDifference(destinationDescription,CreateList);
  } TOCATCH
}

static bool CompareLists(QStringList &lst1,QStringList &lst2,unsigned int len)
{
  if (lst1.count()<len||lst2.count()<len)
    return false;
  for (unsigned int i=0;i<len;i++)
    if (lst1[i]!=lst2[i])
      return false;
  return true;
}

void toScript::fillDifference(list<QString> &objects,QListView *view)
{
  view->clear();
  QListViewItem *last=NULL;
  int lastLevel=0;
  QStringList lstCtx;
  for(list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
    //    printf("Adding %s\n",(const char *)*i);
    QStringList ctx=QStringList::split("\01",*i);
    if (last) {
      while(last&&lastLevel>=int(ctx.count())) {
	last=last->parent();
	lastLevel--;
      }
      while(last&&lastLevel>=0&&!CompareLists(lstCtx,ctx,(unsigned int)lastLevel)) {
	last=last->parent();
	lastLevel--;
      }
    }
    if (lastLevel<0)
      throw QString("Internal error, lastLevel < 0");
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
    item->setEnabled(false);
    lstCtx=ctx;
    lastLevel++;
  }
}

void toScript::changeConnection(int,bool source)
{
  try {
    QListView *sourceL=NULL;
    QListView *destinationL=NULL;
    if (SourceConnection->currentText()==DestinationConnection->currentText()) {
      if (source) {
	destinationL=SourceObjects;
	sourceL=DestinationObjects;
      } else {
	sourceL=SourceObjects;
	destinationL=DestinationObjects;
      }
    }
    if (sourceL&&destinationL&&sourceL->firstChild()) {
      destinationL->clear();
      QListViewItem *next=NULL;
      QListViewItem *parent=NULL;
      for (QListViewItem *item=sourceL->firstChild();item;item=next) {
	QListViewItem *lastParent=parent;
	if (!parent)
	  parent=new QCheckListItem(destinationL,item->text(0),
				    QCheckListItem::CheckBox);
	else
	  parent=new QCheckListItem(parent,item->text(0),
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
    (source?SourceObjects:DestinationObjects)->clear();
    (source?SourceSchema:DestinationSchema)->clear();
    (source?SourceSchema:DestinationSchema)->insertItem("All");
    toConnection &conn=toMainWidget()->connection((source?SourceConnection:DestinationConnection)
						  ->currentText());
    list<QString> object;
    try {
      object=toReadQuery(conn,SQLObjectList(conn));
    } catch(...) {
      object=toReadQuery(conn,SQLUserObjectList(conn));
    }
    list<QString> schema=toReadQuery(conn,SQLSchemas(conn));
    while(schema.size()>0) {
      QString str=toShift(schema);
      (source?SourceSchema:DestinationSchema)->insertItem(str);
    }
    QListViewItem *lastTop=NULL;
    QListViewItem *lastFirst=NULL;
    while(object.size()>0) {
      QString top=toShift(object);
      QString first=toShift(object);
      QString second=toShift(object);

      if (top!=(lastTop?lastTop->text(0):QString::null)) {
	lastFirst=NULL;
	lastTop=new QCheckListItem((source?SourceObjects:DestinationObjects),
				   top,QCheckListItem::CheckBox);
	if (!second.isEmpty()||first.isEmpty())
	  lastTop->setText(1,"USER");
      }
      if (first!=(lastFirst?lastFirst->text(0):QString::null)&&!first.isEmpty()) {
	lastFirst=new QCheckListItem(lastTop,first,QCheckListItem::CheckBox);
	if (second.isEmpty())
	  lastFirst->setText(1,top);
      }
      if (!second.isEmpty()&&lastFirst) {
	QListViewItem *item=new QCheckListItem(lastFirst,second,QCheckListItem::CheckBox);
	item->setText(1,first);
	item->setText(2,top);
      }
    }
  } TOCATCH
}

void toScript::changeMode(int mode)
{
  if (mode==0||mode==2)
    Destination->setEnabled(true);
  else if (mode==1||mode==3)
    Destination->setEnabled(false);
  if (mode==1||mode==2||mode==3)
    Tabs->setTabEnabled(ResizeTab,true);
  else if (mode==0)
    Tabs->setTabEnabled(ResizeTab,false);
  if (mode==1)
    IncludeContent->setEnabled(true);
  else if (mode==0||mode==2||mode==3)
    IncludeContent->setEnabled(false);
  if (mode==1||mode==2||mode==3) {
    IncludeHeader->setEnabled(true);
    IncludePrompt->setEnabled(true);
  } else if (mode==3) {
    IncludeHeader->setEnabled(false);
    IncludePrompt->setEnabled(false);
  }
  if (mode==0||mode==2||mode==3) {
    IncludeDDL->setEnabled(false);
    IncludeDDL->setChecked(mode!=3);
  } else if (mode==1)
    IncludeDDL->setEnabled(true);
  IncludeConstraints->setEnabled(IncludeDDL->isChecked());
  IncludeIndexes->setEnabled(IncludeDDL->isChecked());
  IncludeGrants->setEnabled(IncludeDDL->isChecked());
  IncludeStorage->setEnabled(IncludeDDL->isChecked());
  IncludeParallell->setEnabled(IncludeDDL->isChecked());
  IncludePartition->setEnabled(IncludeDDL->isChecked());
  IncludeCode->setEnabled(IncludeDDL->isChecked());
  IncludeComment->setEnabled(IncludeDDL->isChecked());
}

void toScript::objectClicked(QListViewItem *parent)
{
  QCheckListItem *pchk=dynamic_cast<QCheckListItem *>(parent);
  if (!pchk)
    return;
  bool on=pchk->isOn();
  QListViewItem *next=NULL;
  for (QListViewItem *item=parent->firstChild();item;item=next) {
    QCheckListItem *chk=dynamic_cast<QCheckListItem *>(item);
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
  QString src=(source?SourceSchema:DestinationSchema)->currentText();
  for(QListViewItem *parent=(source?SourceObjects:DestinationObjects)->firstChild();
      parent;
      parent=parent->nextSibling()) {
    QCheckListItem *chk=dynamic_cast<QCheckListItem *>(parent);
    if (chk) {
      bool ena=((src==chk->text(0))||(src=="All"));

      QListViewItem *next=NULL;
      for (QListViewItem *item=parent;item;item=next) {
	chk=dynamic_cast<QCheckListItem *>(item);
	if (chk) {
	  chk->setEnabled(ena);
	  (source?SourceObjects:DestinationObjects)->repaintItem(chk);
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
  QString init=Initial->sizeString();
  QString next=Next->sizeString();
  QString max=Limit->sizeString();
  QString maxNum;
  maxNum.sprintf("%010d",Limit->value());

  for (QListViewItem *item=Sizes->firstChild();item;item=item->nextSibling())
    if (max==item->text(0)) {
      toStatusMessage("Replacing existing size with new");
      delete item;
      break;
    }
  
  new QListViewItem(Sizes,max,init,next,maxNum);
  Sizes->setSorting(3);
}

void toScript::removeSize(void)
{
  QListViewItem *item=Sizes->selectedItem();
  if (item)
    delete item;
}

void toScript::setupExtract(toExtract &extr)
{
  extr.setCode       (IncludeCode->isEnabled()       &&IncludeCode->isChecked()       );
  extr.setComments   (IncludeComment->isEnabled()    &&IncludeComment->isChecked()    );
  extr.setConstraints(IncludeConstraints->isEnabled()&&IncludeConstraints->isChecked());
  extr.setContents   (IncludeContent->isEnabled()    &&IncludeContent->isChecked()    );
  extr.setGrants     (IncludeGrants->isEnabled()     &&IncludeGrants->isChecked()     );
  extr.setHeading    (IncludeHeader->isEnabled()     &&IncludeHeader->isChecked()     );
  extr.setIndexes    (IncludeIndexes->isEnabled()    &&IncludeIndexes->isChecked()    );
  extr.setParallel   (IncludeParallell->isEnabled()  &&IncludeParallell->isChecked()  );
  extr.setPartition  (IncludePartition->isEnabled()  &&IncludePartition->isChecked()  );
  extr.setPrompt     (IncludePrompt->isEnabled()     &&IncludePrompt->isChecked()     );
  extr.setStorage    (IncludeStorage->isEnabled()    &&IncludeStorage->isChecked()    );

  if (Schema->currentText()=="Same")
    extr.setSchema("1");
  else if (Schema->currentText()=="None")
    extr.setSchema(QString::null);
  else
    extr.setSchema(Schema->currentText());

  if (DontResize->isChecked())
    extr.setResize(QString::null);
  else if (AutoResize->isChecked())
    extr.setResize("1");
  else {
    QString siz;
    for (QListViewItem *item=Sizes->firstChild();item;item=item->nextSibling()) {
      siz+=item->text(0);
      siz+=":";
      siz+=item->text(1);
      siz+=":";
      siz+=item->text(2);
      if (item->nextSibling())
	siz+=":";
    }
    extr.setResize(siz);
  }
}
