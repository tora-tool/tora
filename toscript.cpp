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
#include "toscriptprogressui.moc"

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
			   "SELECT *
  FROM (SELECT 'TABLESPACE',tablespace_name,NULL
	  FROM dba_tablespaces
	UNION
	SELECT 'ROLE',role,NULL
	  FROM dba_roles
	UNION
	SELECT 'PUBLIC','SYNONYM',synonym_name
	  FROM all_synonyms WHERE owner = 'PUBLIC'
	UNION
	SELECT owner,'DATABASE LINK',db_link
	  FROM all_db_links
	UNION
	SELECT owner,object_type,object_name
	  FROM all_objects
	 WHERE object_type IN ('VIEW','TABLE','TYPE','SEQUENCE','PACKAGE',
			       'PACKAGE BODY','FUNCTION','PROCEDURE')
	UNION
	SELECT owner,'MATERIALIZED TABLE',mview_name AS object
	  FROM all_mviews
        UNION
        SELECT username,NULL,NULL
          FROM all_users)
  ORDER BY 1,2,3",
			   "Extract objects available to extract from the database, "
			   "should have same columns");

static toSQL SQLUserObjectList("toScript:UserExtractObject",
			       "SELECT owner,object_type,object_name
  FROM all_objects
 WHERE object_type IN ('VIEW','TABLE','TYPE','SEQUENCE','PACKAGE',
	               'PACKAGE BODY','FUNCTION','PROCEDURE')
 ORDER BY 1,2,3",
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
  CreateList=new toResultView(false,false,Connection,hsplitter);
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
  Objects->setSorting(0);

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
  DestinationConnection->setCurrentItem(def);
  changeSource(def);
  changeDestination(def);

  connect(SourceConnection,SIGNAL(activated(int)),this,SLOT(changeSource(int)));
  connect(DestinationConnection,SIGNAL(activated(int)),this,SLOT(changeDestination(int)));
  connect(SourceSchema,SIGNAL(activated(int)),this,SLOT(changeSourceSchema(int)));
  connect(Objects,SIGNAL(clicked(QListViewItem *)),this,SLOT(objectClicked(QListViewItem *)));

  Connection.addWidget(this);
}

toScript::~toScript()
{
  Connection.delWidget(this);
}

#include <stdio.h>

void toScript::execute(void)
{
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
  Tabs->setTabEnabled(ResultTab,mode==1||mode==2||mode==3);
  Tabs->setTabEnabled(DifferenceTab,mode==0||mode==2);

  list<QString> tableSpace;
  list<QString> profiles;
  list<QString> otherGlobal;

  list<QString> tables;

  list<QString> userViews;
  list<QString> userOther;

  QListViewItem *next=NULL;
  for (QListViewItem *item=Objects->firstChild();item;item=next) {
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

  list<QString> sourceObjects;
  for(list<QString>::iterator i=tableSpace.begin();i!=tableSpace.end();i++)
    toPush(sourceObjects,*i);
  for(list<QString>::iterator i=profiles.begin();i!=profiles.end();i++)
    toPush(sourceObjects,*i);
  for(list<QString>::iterator i=otherGlobal.begin();i!=otherGlobal.end();i++)
    toPush(sourceObjects,*i);
  for(list<QString>::iterator i=tables.begin();i!=tables.end();i++) {
    QString line="TABLE FAMILY";
    line+=":";
    line+=*i;
    toPush(sourceObjects,line);
  }
  for(list<QString>::iterator i=userViews.begin();i!=userViews.end();i++)
    toPush(sourceObjects,*i);
  for(list<QString>::iterator i=userOther.begin();i!=userOther.end();i++)
    toPush(sourceObjects,*i);
  for(list<QString>::iterator i=tables.begin();i!=tables.end();i++) {
    QString line="TABLE REFERENCES";
    line+=":";
    line+=*i;
    toPush(sourceObjects,line);
  }

  list<QString> sourceDescription;
  QString sourceScript;
  {
    displayProgress("Initialising extractor",0,sourceObjects.size());
    toExtract source(toMainWidget()->connection(SourceConnection->name()));
    setupExtract(source);
    int num=1;
    for(list<QString>::iterator i=sourceObjects.begin();i!=sourceObjects.end();i++) {
      displayProgress(*i,num,sourceObjects.size());
      QString type=*i;
      int pos=type.find(":");
      if (pos<0)
	throw QString("Internal error, missing : in operationlist");
      list<QString> object;
      toPush(object,type.right(type.length()-pos-1));
      type.truncate(pos);
      switch(mode) {
      case 1:
	sourceScript+=source.create(type,object);
	break;
      case 0:
      case 2:
	{
	  list<QString> dsc=source.describe(type,object);
	  dsc.sort();
	  sourceDescription.merge(dsc);
	}
	break;
      case 3:
	break;
      }
      num++;
    }
  }

  list<QString> destinationObjects;
  if (Destination->isEnabled()) {
  }
}

void toScript::changeSource(int)
{
  try {
    Objects->clear();
    SourceSchema->clear();
    SourceSchema->insertItem("All");
    toConnection &conn=toMainWidget()->connection(SourceConnection->currentText());
    list<QString> object;
    try {
      object=toReadQuery(conn,SQLObjectList(conn));
    } catch(...) {
      object=toReadQuery(conn,SQLUserObjectList(conn));
    }
    list<QString> schema=toReadQuery(conn,SQLSchemas(conn));
    while(schema.size()>0) {
      QString str=toShift(schema);
      SourceSchema->insertItem(str);
    }
    QListViewItem *lastTop=NULL;
    QListViewItem *lastFirst=NULL;
    while(object.size()>0) {
      QString top=toShift(object);
      QString first=toShift(object);
      QString second=toShift(object);

      if (top!=(lastTop?lastTop->text(0):QString::null)) {
	lastFirst=NULL;
	lastTop=new QCheckListItem(Objects,top,QCheckListItem::CheckBox);
      }
      if (first!=(lastFirst?lastFirst->text(0):QString::null)&&!first.isEmpty()) {
	lastFirst=new QCheckListItem(lastTop,first,QCheckListItem::CheckBox);
	if (second.isEmpty())
	  lastFirst->setText(1,top);
      }
      if (!second.isEmpty()) {
	QListViewItem *item=new QCheckListItem(lastFirst,second,QCheckListItem::CheckBox);
	item->setText(1,first);
	item->setText(2,top);
      }
    }
  } TOCATCH
}

void toScript::changeDestination(int)
{
  try {
    DestinationSchema->clear();
    DestinationSchema->insertItem("All");
    toConnection &conn=toMainWidget()->connection(DestinationConnection->currentText());
    list<QString> schema=toReadQuery(conn,SQLSchemas(conn));
    while(schema.size()>0) {
      QString str=toShift(schema);
      DestinationSchema->insertItem(str);
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

void toScript::changeSourceSchema(int)
{
  QString src=SourceSchema->currentText();
  for(QListViewItem *parent=Objects->firstChild();parent;parent=parent->nextSibling()) {
    QCheckListItem *chk=dynamic_cast<QCheckListItem *>(parent);
    if (chk) {
      bool ena=((src==chk->text(0))||(src=="All"));

      QListViewItem *next=NULL;
      for (QListViewItem *item=parent;item;item=next) {
	chk=dynamic_cast<QCheckListItem *>(item);
	if (chk) {
	  chk->setEnabled(ena);
	  Objects->repaintItem(chk);
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
