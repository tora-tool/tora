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
#include "tohelp.h"
#include "tomarkedtext.h"
#include "tonoblockquery.h"
#include "toresultview.h"
#include "totemplate.h"
#include "totool.h"

#ifdef TO_KDE
#include <kfiledialog.h>
#endif

#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qtextview.h>
#include <qtimer.h>
#include <qtoolbar.h>

#include "totemplate.moc"
#include "totemplateaddfileui.moc"
#include "totemplateeditui.moc"
#include "totemplatesetupui.moc"

#include "icons/totemplate.xpm"

static std::map<QString,QString> DefaultText(void)
{
  std::map<QString,QString> def;
  QString file=toPluginPath();
  file+="/sqlfunctions.tpl";
  def["PL/SQL Functions"]=file;
  file=toPluginPath();
  file+="/hints.tpl";
  def["Optimizer Hints"]=file;
  return def;
}

class toTemplateEdit : public toTemplateEditUI, public toHelpContext {
  std::map<QString,QString> &TemplateMap;
  std::map<QString,QString>::iterator LastTemplate;
  void connectList(bool conn)
  {
    if (conn)
      connect(Templates,SIGNAL(selectionChanged()),this,SLOT(changeSelection()));
    else
      disconnect(Templates,SIGNAL(selectionChanged()),this,SLOT(changeSelection()));
  }
  QListViewItem *findLast(void)
  {
    QString name=(*LastTemplate).first;
    return toFindItem(Templates,name);
  }
  void allocateItem(void)
  {
    QStringList lst=QStringList::split(":",Name->text());
    unsigned int li=0;
    QListViewItem *parent=NULL;
    for(QListViewItem *item=Templates->firstChild();item&&li<lst.count();) {
      if (item->text(0)==lst[li]) {
	li++;
	parent=item;
	item=item->firstChild();
      } else
	item=item->nextSibling();
    }
    while(li<lst.count()) {
      if (parent)
	parent=new QListViewItem(parent,lst[li]);
      else
	parent=new QListViewItem(Templates,lst[li]);
      li++;
    }
  }
  bool clearUnused(QListViewItem *first,const QString &pre)
  {
    bool ret=false;
    while(first) {
      QListViewItem *delitem=first;
      QString str=pre;
      if (!str.isEmpty())
	str+=":";
      str+=first->text(0);
      if (first->firstChild()&&clearUnused(first->firstChild(),str))
	delitem=NULL;
      if (delitem&&TemplateMap.find(str)!=TemplateMap.end())
	delitem=NULL;
      first=first->nextSibling();
      if (!delitem)
	ret=true;
      else
	delete delitem;
    }
    return ret;
  }
public:
  virtual void updateFromMap(void)
  {
    while(Templates->firstChild())
      delete Templates->firstChild();
    QListViewItem *last=NULL;
    int lastLevel=0;
    QStringList lstCtx;
    for(std::map<QString,QString>::iterator i=TemplateMap.begin();i!=TemplateMap.end();i++) {
      QStringList ctx=QStringList::split(":",(*i).first);
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
	throw QString("Internal error, lastLevel < 0");
      while(lastLevel<int(ctx.count())-1) {
	if (last)
	  last=new QListViewItem(last,ctx[lastLevel]);
	else
	  last=new QListViewItem(Templates,ctx[lastLevel]);
	last->setOpen(true);
	lastLevel++;
      }
      if (last)
	last=new QListViewItem(last,ctx[lastLevel]);
      else
	last=new QListViewItem(Templates,ctx[lastLevel]);
      last->setOpen(true);
      if (i==LastTemplate)
	last->setSelected(true);
      lstCtx=ctx;
      lastLevel++;
    }
  }
  toTemplateEdit(std::map<QString,QString> &pairs,QWidget *parent,const char *name=0)
    : toTemplateEditUI(parent,name,true,WStyle_Maximize),
      toHelpContext("template.html#editor"),
      TemplateMap(pairs)
  {
    toHelp::connectDialog(this);
    LastTemplate=TemplateMap.end();
    updateFromMap();
    Description->setWordWrap(toMarkedText::WidgetWidth);
  }
  virtual void remove(void)
  {
    if (LastTemplate!=TemplateMap.end()) {
      QListViewItem *item=findLast();
      TemplateMap.erase(LastTemplate);
      LastTemplate=TemplateMap.end();
      Name->setText("");
      Description->setText("");
      if (item) {
	connectList(false);
	clearUnused(Templates->firstChild(),"");
	connectList(true);
      }
    }
  }
  virtual void preview(void)
  {
    Preview->setText(Description->text());
  }
  QString name(QListViewItem *item)
  {
    QString str=item->text(0);
    for(item=item->parent();item;item=item->parent()) {
      str.prepend(":");
      str.prepend(item->text(0));
    }
    return str;
  }
  virtual void newTemplate(void)
  {
    changeSelection();
    LastTemplate=TemplateMap.end();
    Description->setText("");
    QListViewItem *item=Templates->selectedItem();
    if (item) {
      connectList(false);
      Templates->setSelected(item,false);
      connectList(true);
      item=item->parent();
    }
    QString str;
    if (item) {
      str=name(item);
      str+=":";
    }
    Name->setText(str);
  }
  virtual void changeSelection(void)
  {
    bool update=false;
    if (LastTemplate!=TemplateMap.end()) {
      if (Name->text()!=(*LastTemplate).first||
	  Description->text()!=(*LastTemplate).second) {
	TemplateMap.erase(LastTemplate);
	TemplateMap[Name->text()]=Description->text();
	allocateItem();
	update=true;
      }
    } else if (!Name->text().isEmpty()) {
      TemplateMap[Name->text()]=Description->text();
      allocateItem();
      update=true;
    }
    LastTemplate=TemplateMap.end();

    QListViewItem *item=Templates->selectedItem();
    if (item) {
      QString str=name(item);
      LastTemplate=TemplateMap.find(str);
      if (LastTemplate!=TemplateMap.end()) {
	Name->setText((*LastTemplate).first);
	Description->setText((*LastTemplate).second);
	Preview->setText((*LastTemplate).second);
      } else {
	Name->setText("");
	Description->clear();
	Preview->setText("");
      }
    } else
      LastTemplate=TemplateMap.end();
    clearUnused(Templates->firstChild(),"");
  }
};

class toTemplateAddFile : public toTemplateAddFileUI {
public:
  toTemplateAddFile(QWidget *parent,const char *name=0)
    : toTemplateAddFileUI(parent,name,true)
  {
    OkButton->setEnabled(false);
    toHelp::connectDialog(this);
  }
  virtual void browse(void)
  {
    QFileInfo file(Filename->text());
    QString filename=toOpenFilename(file.dirPath(),"*.tpl",this);
    if (!filename.isEmpty())
      Filename->setText(filename);
  }
  virtual void valid(void)
  {
    if (Filename->text().isEmpty()||Root->text().isEmpty())
      OkButton->setEnabled(false);
    else
      OkButton->setEnabled(true);
  }
};

class toTemplatePrefs : public toTemplateSetupUI, public toSettingTab
{
  toTool *Tool;
public:
  toTemplatePrefs(toTool *tool,QWidget *parent,const char *name=0)
    : toTemplateSetupUI(parent,name),toSettingTab("template.html#setup"),Tool(tool)
  {
    std::map<QString,QString> def=DefaultText();

    int tot=Tool->config("Number",QString::number(-1)).toInt();
    {
      for(int i=0;i<tot;i++) {
	QString num=QString::number(i);
        QString root=Tool->config(num,"");
        num+="file";
        QString file=Tool->config(num,"");
	new QListViewItem(FileList,root,file);
	if (def.find(root)!=def.end())
	  def.erase(def.find(root));
      }
    }
    for (std::map<QString,QString>::iterator i=def.begin();i!=def.end();i++)
      new QListViewItem(FileList,(*i).first,(*i).second);
  }
  virtual void saveSetting(void)
  {
    int i=0;
    for(QListViewItem *item=FileList->firstChild();item;item=item->nextSibling()) {
      QString nam=QString::number(i);
      Tool->setConfig(nam,item->text(0));
      nam+="file";
      Tool->setConfig(nam,item->text(1));
      i++;
    }
    Tool->setConfig("Number",QString::number(i));
  }
  virtual void addFile(void)
  {
    toTemplateAddFile file(this);
    if (file.exec())
      new QListViewItem(FileList,file.Root->text(),file.Filename->text());
  }
  virtual void editFile(void)
  {
    QListViewItem *item=FileList->selectedItem();
    if (item) {
      try {
	QString file=item->text(1);
	std::map<QString,QString> pairs;
	if (!toTool::loadMap(file,pairs)) {
	  if (TOMessageBox::warning(this,
				    "Couldn't open file.",
				    "Couldn't open file. Start on new file?",
				    "&Ok",
				    "Cancel")==1)
	    return;
	}
	toTemplateEdit edit(pairs,this);
	if (edit.exec()) {
	  edit.changeSelection();
	  if (!toTool::saveMap(file,pairs))
	    throw QString("Couldn't write file");
	}
      } catch (const QString &str) {
	TOMessageBox::warning(this,
			      "Couldn't open file",
			      str,
			      "&Ok");
      }
    }
  }
  virtual void delFile(void)
  {
    delete FileList->selectedItem();
  }
};

class toTemplateTool : public toTool {
  TODock *Dock;
protected:
  virtual char **pictureXPM(void)
  { return totemplate_xpm; }
public:
  toTemplateTool()
    : toTool(410,"SQL Template")
  { Dock=NULL; }
  virtual const char *menuItem()
  { return "SQL Template"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    if (Dock) {
      if (Dock->isHidden()) {
	delete Dock;
	Dock=NULL;
      } else {
	delete Dock;
	Dock=NULL;
	return NULL;
      }
    }
    Dock=toAllocDock("Template",QString::null,*toolbarImage());
    toTemplate *window=new toTemplate(Dock);
    toAttachDock(Dock,window,QMainWindow::Left);
    window->attachResult();
    return Dock;
  }
  void closeWindow(toConnection &connection)
  { Dock=NULL; }
  virtual QWidget *configurationTab(QWidget *parent)
  { return new toTemplatePrefs(this,parent); }
  virtual bool canHandle(toConnection &conn)
  { return true; }
};

static toTemplateTool TemplateTool;

QWidget *toTemplate::parentWidget(QListViewItem *item)
{
  return templateWidget(item)->frame();
}

toTemplate *toTemplate::templateWidget(QListView *obj)
{
  QObject *lst=obj;
  while(lst) {
    toTemplate *tpl=dynamic_cast<toTemplate *>(lst);
    if (tpl)
      return tpl;
    lst=lst->parent();
  }
  throw("Not a toTemplate parent");
}

toTemplate *toTemplate::templateWidget(QListViewItem *item)
{
  return templateWidget(item->listView());
}

class toTemplateResult : public QVBox {
  toTemplate *Template;
public:
  toTemplateResult(TODock *parent,toTemplate *temp)
    : QVBox(parent),Template(temp)
  { }
  virtual ~toTemplateResult()
  { Template->closeFrame(); }
};

toTemplate::toTemplate(QWidget *parent)
  : QVBox(parent),toHelpContext("template.html")
{
  Toolbar=toAllocBar(this,"Template Toolbar",QString::null);

  List=new toListView(this);
  List->addColumn("Template");
  List->setRootIsDecorated(true);
  List->setSorting(0);
  List->setShowSortIndicator(false);
  List->setTreeStepSize(10);
  List->setSelectionMode(QListView::Single);
  TODock *dock;
  dock=Result=toAllocDock("Template result",
			  QString::null,
			  *TemplateTool.toolbarImage());
  Frame=new toTemplateResult(dock,this);

  connect(List,SIGNAL(expanded(QListViewItem *)),this,SLOT(expand(QListViewItem *)));
  connect(List,SIGNAL(collapsed(QListViewItem *)),this,SLOT(collapse(QListViewItem *)));
  connect(List,SIGNAL(doubleClicked(QListViewItem *)),this,SLOT(doubleClick(QListViewItem *)));

  if (toTemplateProvider::Providers)
    for (std::list<toTemplateProvider *>::iterator i=toTemplateProvider::Providers->begin();
	 i!=toTemplateProvider::Providers->end();
	 i++)
      (*i)->insertItems(List,Toolbar);

  Toolbar->setStretchableWidget(new QLabel("",Toolbar));

  WidgetExtra=NULL;
  setWidget(NULL);

  setFocusProxy(List);
}

toTemplate::~toTemplate()
{
  delete Result;
}

void toTemplate::attachResult(void)
{
  toAttachDock((TODock *)Result,Frame,QMainWindow::Bottom);
}

void toTemplate::closeFrame(void)
{
  Result=NULL;
  Frame=NULL;
}

void toTemplate::expand(QListViewItem *item)
{
  try {
    toTemplateItem *ti=dynamic_cast<toTemplateItem *>(item);
    if (ti)
      ti->expand();
  } catch (...) {
  }
}

void toTemplate::doubleClick(QListViewItem *item)
{
  try {
    toTemplateItem *ti=dynamic_cast<toTemplateItem *>(item);
    if (ti)
      ti->doubleClick();
  } catch (...) {
  }
}

QWidget *toTemplate::frame(void)
{
  if (Result->isHidden())
    attachResult();
  return Frame;
}

void toTemplate::collapse(QListViewItem *item)
{
  toTemplateItem *ti=dynamic_cast<toTemplateItem *>(item);
  if (ti)
    ti->collapse();
}

void toTemplateItem::setSelected(bool sel)
{
  toTemplate *temp=toTemplate::templateWidget(this);
  if (sel&&temp) {
    QWidget *frame=toTemplate::parentWidget(this);
    if (frame)
      temp->setWidget(selectedWidget(frame));
  }
  toResultViewItem::setSelected(sel);
}

QWidget *toTemplateText::selectedWidget(QWidget *parent)
{
  return new QTextView(Note,QString::null,parent);
}

void toTemplate::setWidget(QWidget *widget)
{
  if (!widget)
    widget=new QTextView(frame());

  widget->show();
  if (WidgetExtra)
    delete WidgetExtra;

  WidgetExtra=widget;
}

class toTextTemplate : toTemplateProvider {
public:
  toTextTemplate()
    : toTemplateProvider("Text")
  { }
  void addFile(QListView *parent,const QString &root,const QString &file);
  virtual void insertItems(QListView *parent,QToolBar *toolbar);
};

void toTextTemplate::insertItems(QListView *parent,QToolBar *)
{
  int tot=TemplateTool.config("Number",QString::number(-1)).toInt();
  std::map<QString,QString> def=DefaultText();

  {
    for(int i=0;i<tot;i++) {
      QString num=QString::number(i);
      QString root=TemplateTool.config(num,"");
      num+="file";
      QString file=TemplateTool.config(num,"");
      addFile(parent,root,file);
      if (def.find(root)!=def.end())
	def.erase(def.find(root));
    }
  }
  for (std::map<QString,QString>::iterator i=def.begin();i!=def.end();i++)
    addFile(parent,(*i).first,(*i).second);
}

void toTextTemplate::addFile(QListView *parent,const QString &root,const QString &file)
{
  std::map<QString,QString> pairs;
  toTool::loadMap(file,pairs);
  toTemplateItem *last=new toTemplateItem(*this,parent,root);
  int lastLevel=0;
  QStringList lstCtx;
  for(std::map<QString,QString>::iterator i=pairs.begin();i!=pairs.end();i++) {
    QStringList ctx=QStringList::split(":",(*i).first);
    if (last) {
      while(last&&lastLevel>=int(ctx.count())) {
	last=dynamic_cast<toTemplateItem *>(last->parent());
	lastLevel--;
      }
      while(last&&lastLevel>=0&&!toCompareLists(lstCtx,ctx,(unsigned int)lastLevel)) {
	last=dynamic_cast<toTemplateItem *>(last->parent());
	lastLevel--;
      }
    }
    if (lastLevel<0)
      throw QString("Internal error, lastLevel < 0");
    while(lastLevel<int(ctx.count())-1) {
      last=new toTemplateItem(last,ctx[lastLevel]);
      lastLevel++;
    }
    last=new toTemplateText(last,ctx[lastLevel],(*i).second);
    lstCtx=ctx;
    lastLevel++;
  }
}

toTemplateSQL::toTemplateSQL(toConnection &conn,toTemplateItem *parent,
			     const QString &name,const QString &sql)
  : toTemplateItem(parent,name),Object(this),Connection(conn),SQL(sql)
{
  setExpandable(true);
}

toTemplateSQLObject::toTemplateSQLObject(toTemplateSQL *parent)
  : Parent(parent)
{
  Query=NULL;
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
}

void toTemplateSQL::expand(void)
{
  while(firstChild())
    delete firstChild();
  Object.expand();
}

void toTemplateSQLObject::expand(void)
{
  try {
    delete Query;
    Query=NULL;
    Query=new toNoBlockQuery(Parent->connection(),toQuery::Background,
			     Parent->SQL,Parent->parameters());
    Poll.start(100);
  } TOCATCH
}

void toTemplateSQLObject::poll(void)
{
  try {
    if (QApplication::activeModalWidget()) // Template is never in widget
      return;
    if (Query&&Query->poll()) {
      toQDescList desc=Query->describe();
      while(Query->poll()&&!Query->eof()) {
	Parent->createChild(Query->readValue());
	for (unsigned int j=1;j<desc.size();j++)
	  Query->readValue();
      }
      if (Query->eof()) {
	delete Query;
	Query=NULL;
	Poll.stop();
      }
    }
  } catch (const QString &str) {
    delete Query;
    Query=NULL;
    Poll.stop();
    toStatusMessage(str);
  }
}

toTemplateSQLObject::~toTemplateSQLObject()
{
  delete Query;
}

static toTextTemplate TextTemplate;
