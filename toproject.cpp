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

#include "tohighlightedtext.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "toproject.h"
#include "toworksheet.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include "toproject.moc"

#include "icons/addproject.xpm"
#include "icons/filesave.xpm"
#include "icons/new.xpm"
#include "icons/sql.xpm"
#include "icons/trash.xpm"
#include "icons/up.xpm"
#include "icons/down.xpm"

static toProjectTemplate ProjectTemplate;

#ifdef TO_KDE
#define PROJECT_EXTENSIONS	"*.sql *.pkg *.pkb|SQL files\n*.tpr|Project files\n*.txt|Text files\n*|All files"
#else
#define PROJECT_EXTENSIONS	"SQL (*.sql *.pkg *.pkb),Project files (*.tpr) ,Text (*.txt), All (*)"
#endif

bool toProjectTemplateItem::project(void)
{
  if (!parent())
    return true;
  if (Filename.length()>4&&Filename.mid(Filename.length()-4,4)==".tpr")
    return true;
  return false;
}

void toProjectTemplateItem::setup(const QString &name,bool open)
{
  Order=-1;
  setFilename(name);
  if (project()&&!Filename.isEmpty()&&open) {
    try {
      QString data=QString::fromUtf8(toReadFile(Filename));
      QStringList files=QStringList::split(QRegExp("\n"),data);
      toProjectTemplateItem *last=NULL;
      for(unsigned int i=0;i<files.count();i++)
	last=new toProjectTemplateItem(this,last,files[i]);
    } catch(const QString &exc) {
      toStatusMessage(exc);
    }
  }
}

void toProjectTemplateItem::setFilename(const QString &name)
{
  if (parent()) {
    int pos=name.findRev("/");
    if (pos<0)
      pos=name.findRev("\\");
    if (pos>=0)
      setText(0,name.mid(pos+1));
    else
      setText(0,name);
  }
  Filename=name;
}

toProjectTemplateItem::toProjectTemplateItem(toTemplateItem *item,
					     QListViewItem *after,
					     QString name,bool open)
  : toTemplateItem(item,QString::null,after)
{
  setup(name,open);
}

toProjectTemplateItem::toProjectTemplateItem(QListView *item,QString name,bool open)
  : toTemplateItem(ProjectTemplate,item,"SQL Project")
{
  setup(name,open);
}

int toProjectTemplateItem::order(bool asc)
{
  if(asc) {
    toProjectTemplateItem *item=previousSibling();
    if (item) {
      int attemptOrder=item->Order+1;
      if (attemptOrder!=Order)
	Order=item->order(asc)+1;
    } else
      Order=1;
  } else {
    toProjectTemplateItem *item=dynamic_cast<toProjectTemplateItem *>(nextSibling());
    if (item) {
      int attemptOrder=item->Order+1;
      if (attemptOrder!=Order)
	Order=item->order(asc)+1;
    } else
      Order=1;
  }
  return Order;
}

toProjectTemplateItem *toProjectTemplateItem::previousSibling()
{
  QListViewItem *item=itemAbove();
  if (!item)
    return NULL;
  if (item==parent())
    return NULL;
  while (item&&item->parent()!=parent())
    item=item->parent();
  return dynamic_cast<toProjectTemplateItem *>(item);
}

// Here be dragons! Basically all the order stuff will retain whatever order the
// items were created in.

QString toProjectTemplateItem::key(int col,bool asc) const
{
  if (!parent())
    return text(col);
  int no=((toProjectTemplateItem *)this)->order(asc);
  QString ret;
  ret.sprintf("%010d",no);
  return ret;
}

void toProjectTemplateItem::selected(void)
{
  if (project())
    return;
  try {
    toWorksheet::fileWorksheet(Filename);
  } catch(const QString &exc) {
    toStatusMessage(exc);
  }
}



QWidget *toProjectTemplateItem::selectedWidget(QWidget *parent)
{
  return ProjectTemplate.selectedWidget(parent);
}

QWidget *toProjectTemplate::selectedWidget(QWidget *parent)
{
  if (!Details)
    Details=new toProject(ProjectTemplate.root(),parent);
  return Details;
}

void toProjectTemplate::importData(std::map<QString,QString> &data,const QString &prefix)
{
  bool any=false;
  std::map<QString,QString>::iterator i=data.find(prefix+":");
  while(i!=data.end()) {
    if ((*i).first.startsWith(prefix)) {
      Import[(*i).first.mid(prefix.length()+1)]=(*i).second;
      any=true;
    } else if (any)
      break;
    i++;
  }
}

void toProjectTemplate::exportData(std::map<QString,QString> &data,const QString &prefix)
{
  if (!Root)
    return;
  std::map<QListViewItem *,int> itemMap;
  QListViewItem *next;
  int id=0;
  data[prefix+":"]=Root->filename();
  for (QListViewItem *item=Root->firstChild();item;item=next) {
    id++;
    QString nam=prefix;
    nam+=":Items:";
    nam+=QString::number(id);
    nam+=":";
    itemMap[item]=id;
    if (item->parent())
      data[nam+"Parent"]=QString::number(itemMap[item->parent()]);
    else
      data[nam+"Parent"]="0";
    if (item->isOpen())
      data[nam+"Open"]="Yes";
    toProjectTemplateItem *projitem=dynamic_cast<toProjectTemplateItem *>(item);
    QString val;
    if (projitem)
      data[nam+"0"]=projitem->filename();

    if (item->firstChild())
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
      if (next==Root->nextSibling())
	break;
    }
  }
}

void toProjectTemplate::insertItems(QListView *parent,QToolBar *toolbar)
{
  Root=new toProjectTemplateItem(parent,Import[""],false);
  Root->setOpen(true);

  int id=1;
  std::map<QString,QString>::iterator i;
  std::map<int,toProjectTemplateItem *> itemMap;

  toProjectTemplateItem *last=NULL;

  while((i=Import.find("Items:"+QString::number(id)+":Parent"))!=Import.end()) {
    QString nam="Items:"+QString::number(id)+":";
    int parent=(*i).second.toInt();
    if (parent)
      last=new toProjectTemplateItem(itemMap[parent],last,Import[nam+"0"],false);
    else
      last=new toProjectTemplateItem(Root,last,Import[nam+"0"],false);
    if (!Import[nam+"Open"].isEmpty())
      last->setOpen(true);
    itemMap[id]=last;
    id++;
  }

  connect(parent,SIGNAL(currentChanged(QListViewItem *)),this,SLOT(changeItem(QListViewItem *)));
  AddFile=new QToolButton(QPixmap((const char **)addproject_xpm),
			  "Add file to project",
			  "Add file to project",
			  this,SLOT(addFile()),
			  toolbar);
  DelFile=new QToolButton(QPixmap((const char **)trash_xpm),
			  "Remove file from project",
			  "Remove file from project",
			  this,SLOT(delFile()),
			  toolbar);
  AddFile->setEnabled(false);
  DelFile->setEnabled(false);
}

void toProjectTemplate::changeItem(QListViewItem *item)
{
  bool ena=dynamic_cast<toProjectTemplateItem *>(item);
  AddFile->setEnabled(ena);
  DelFile->setEnabled(ena&&item->parent());
}

void toProjectTemplate::removeItems(QListViewItem *item)
{
  delete item;
}

void toProjectTemplate::addFile(void)
{
  QString file=toOpenFilename(QString::null,
			      PROJECT_EXTENSIONS,
			      toMainWidget());
  if (!file.isNull()) {
    QListView *view=Root->listView();
    toProjectTemplateItem *item=dynamic_cast<toProjectTemplateItem *>(view->currentItem());
    if (item) {
      item->setOpen(true);
      QListViewItem *last=item->firstChild();
      while(last&&last->nextSibling())
	last=last->nextSibling();
      if (item->project())
	new toProjectTemplateItem(item,last,file);
      else {
	item=dynamic_cast<toProjectTemplateItem *>(item->parent());
	if (item)
	  new toProjectTemplateItem(item,last,file);
      }
    }
    if (Details)
      Details->update();
  }
}

void toProjectTemplate::delFile(void)
{
  QListView *view=Root->listView();
  toProjectTemplateItem *item=dynamic_cast<toProjectTemplateItem *>(view->currentItem());
  delete item;
  if (Details)
    Details->update();
}

void toProject::update(toProjectTemplateItem *sourceparent,toResultViewItem *parent)
{
  if (parent==NULL) {
    parent=new toResultViewItem(Project,NULL,"SQL Project");
    parent->setOpen(true);
    ItemMap[parent]=sourceparent;
  }
  for (QListViewItem *item=sourceparent->firstChild();item;item=item->nextSibling()) {
    toProjectTemplateItem *projitem=dynamic_cast<toProjectTemplateItem *>(item);
    if (projitem) {
      QFile file(projitem->filename());
      unsigned int size=file.size();
      toResultViewItem *nitem;
      nitem=new toResultViewItem(parent,NULL,projitem->filename());
      if (projitem->project())
	update(projitem,nitem);
      else if (size>0)
	nitem->setText(1,QString::number(size));
      nitem->setOpen(true);
      if (projitem->isSelected())
	nitem->setSelected(true);
      ItemMap[nitem]=projitem;
    }
  }
}

toProject::toProject(toProjectTemplateItem *top,QWidget *parent)
  : QVBox(parent)
{
  Root=top;

  QToolBar *toolbar=toAllocBar(this,"SQL project",QString::null);
  new QToolButton(QPixmap((const char **)addproject_xpm),
		  "Add file to project",
		  "Add file to project",
		  this,SLOT(addFile()),
		  toolbar);
  new QToolButton(QPixmap((const char **)filesave_xpm),
		  "Save project",
		  "Save project",
		  this,SLOT(saveProject()),
		  toolbar);
  DelFile=new QToolButton(QPixmap((const char **)trash_xpm),
			  "Remove file from project",
			  "Remove file from project",
			  this,SLOT(delFile()),
			  toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)new_xpm),
		  "Add new subproject",
		  "Add new subproject",
		  this,SLOT(newProject()),
		  toolbar);
  new QToolButton(QPixmap((const char **)sql_xpm),
		  "Generate SQL for this project",
		  "Generate SQL for this project",
		  this,SLOT(generateSQL()),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)up_xpm),
		  "Move up in project",
		  "Move up in project",
		  this,SLOT(moveUp()),
		  toolbar);
  new QToolButton(QPixmap((const char **)down_xpm),
		  "Move down in project",
		  "Move down in project",
		  this,SLOT(moveDown()),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel(toolbar));
  Project=new toListView(this);
  Project->addColumn("File");
  Project->addColumn("Size");
  Project->setSelectionMode(QListView::Single);
  Project->setRootIsDecorated(true);
  Project->setSorting(0);
  connect(Project,SIGNAL(selectionChanged()),
	  this,SLOT(selectionChanged()));
  update();
}

void toProject::update(void)
{
  Project->clear();
  update(Root,NULL);
}

void toProject::selectionChanged(void)
{
  QListViewItem *item=Project->selectedItem();
  if (item) {
    DelFile->setEnabled(item->parent());
    toProjectTemplateItem *oi=ItemMap[item];
    if (oi) {
      oi->listView()->setSelected(oi,true);
    }
  }
}

void toProject::addFile(void)
{
  selectionChanged();
  ProjectTemplate.addFile();
}

void toProject::delFile(void)
{
  selectionChanged();
  ProjectTemplate.delFile();
}

void toProject::moveDown(void)
{
  QListViewItem *item=Project->selectedItem();
  if (item) {
    toProjectTemplateItem *oi=ItemMap[item];
    if (oi) {
      QListViewItem *item=oi->nextSibling();
      QListViewItem *parent=oi->parent();
      if (item&&parent) {
	oi->moveItem(item);
	update();
      }
    }
  }
}

void toProject::moveUp(void)
{
  QListViewItem *item=Project->selectedItem();
  if (item) {
    toProjectTemplateItem *oi=ItemMap[item];
    if (oi) {
      QListViewItem *item=oi->previousSibling();
      QListViewItem *parent=oi->parent();
      if (item&&parent) {
	item->moveItem(oi);
	update();
      }
    }
  }
}

void toProject::newProject(void)
{
  QListViewItem *item=Project->selectedItem();
  if (item) {
    toProjectTemplateItem *oi=ItemMap[item];
    if (oi) {
      QListViewItem *last=oi;
      if (!oi->project())
	oi=dynamic_cast<toProjectTemplateItem *>(oi->parent());
      else {
	last=last->firstChild();
	while(last&&last->nextSibling())
	  last=last->nextSibling();
      }
      if (oi) {
	new toProjectTemplateItem(oi,last,"untitled.tpr");
	Project->update();
      }
    }
  }
}

void toProject::saveProject(void)
{
  QListViewItem *item=Project->selectedItem();
  if (item) {
    toProjectTemplateItem *oi=ItemMap[item];
    if (oi) {
      if (!oi->project())
	oi=dynamic_cast<toProjectTemplateItem *>(oi->parent());
      if (oi) {
	QFileInfo file(oi->filename());
	QString fn=oi->filename();
	fn=toSaveFilename(file.dirPath(),"*.tpr",this);
	if (!fn.isEmpty()) {
	  QString data;
	  for(QListViewItem *item=oi->firstChild();item;item=item->nextSibling()) {
	    toProjectTemplateItem *projitem=dynamic_cast<toProjectTemplateItem *>(item);
	    data+=projitem->filename()+"\n";
	  }
	  if (toWriteFile(fn,data.utf8()))
	    oi->setFilename(fn);
	}
      }
    }
  }
}

QString toProject::generateSQL(toProjectTemplateItem *parent)
{
  QString data;
  for(toProjectTemplateItem *item=dynamic_cast<toProjectTemplateItem *>(parent->firstChild());
      item;
      item=dynamic_cast<toProjectTemplateItem *>(item->nextSibling())) {
    if (item->project()) {
      data+="\n\n-- Start of project "+item->filename()+"\n\n";
      data+=generateSQL(item);
    } else {
      data+="\n\n-- Start of file "+item->filename()+"\n\n";
      data+=QString::fromLocal8Bit(toReadFile(item->filename()));
    }
  }
  return data;
}

void toProject::generateSQL(void)
{
  QListViewItem *item=Project->selectedItem();
  if (item) {
    toProjectTemplateItem *oi=ItemMap[item];
    if (oi) {
      if (!oi->project())
	oi=dynamic_cast<toProjectTemplateItem *>(oi->parent());
      if (oi) {
	QString data=generateSQL(oi);
	new toMemoEditor(toMainWidget(),data);
      }
    }
  }
}

toProject::~toProject()
{
  if (ProjectTemplate.Details==this)
    ProjectTemplate.Details=NULL;
}
