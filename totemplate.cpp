//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifdef TO_KDE
#include <kfiledialog.h>
#endif

#include <qtimer.h>
#include <qsplitter.h>
#include <qtextview.h>
#include <qfileinfo.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlineedit.h>

#include "toresultview.h"
#include "totool.h"
#include "totemplate.h"
#include "tomain.h"
#include "tomarkedtext.h"
#include "toconf.h"
#include "tohelp.h"
#include "toconnection.h"

#include "totemplate.moc"
#include "totemplatesetupui.moc"
#include "totemplateaddfileui.moc"
#include "totemplateeditui.moc"

#include "icons/totemplate.xpm"

class toTemplateEdit : public toTemplateEditUI, public toHelpContext {
  std::map<QString,QString> &TemplateMap;
  std::map<QString,QString>::iterator LastTemplate;
  toTimer Timer;
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
    connect(&Timer,SIGNAL(timeout()),this,SLOT(updateFromMap()));
    updateFromMap();
    Description->setWordWrap(toMarkedText::WidgetWidth);
  }
  virtual void remove(void)
  {
    if (LastTemplate!=TemplateMap.end()) {
      TemplateMap.erase(LastTemplate);
      LastTemplate=TemplateMap.end();
      Name->setText("");
      Description->setText("");
      updateFromMap();
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
    if (item)
      item=item->parent();
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
	update=true;
      }
    } else if (!Name->text().isEmpty()) {
      TemplateMap[Name->text()]=Description->text();
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
      }
    } else
      LastTemplate=TemplateMap.end();
    if (update)
      Timer.start(500,true);
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
    int tot=Tool->config("Number",QString::number(-1)).toInt();
    if(tot!=-1) {
      for(int i=0;i<tot;i++) {
	QString num=QString::number(i);
	QString root=Tool->config(num,"");
	num+="file";
	QString file=Tool->config(num,"");
	new QListViewItem(FileList,root,file);
      }
    } else {
      QString file=toPluginPath();
      file+="/sqlfunctions.tpl";
      new QListViewItem(FileList,"PL/SQL Functions",file);
    }
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
    : toTool(401,"SQL Template")
  { Dock=NULL; }
  virtual const char *menuItem()
  { return "SQL Template"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    if (Dock) {
      if (Dock->isHidden())
	delete Dock;
      else {
	delete Dock;
	Dock=NULL;
	return NULL;
      }
    }
    Dock=toAllocDock("Template",connection.description(),*toolbarImage());
    QWidget *window=new toTemplate(Dock);
    toAttachDock(Dock,window,QMainWindow::Left);
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
    try {
      if (dynamic_cast<toTemplate *>(lst))
        return dynamic_cast<toTemplate *>(lst);
    } catch (...) {
    }
    lst=lst->parent();
  }
  throw("Not a toTemplate parent");
}

toTemplate *toTemplate::templateWidget(QListViewItem *item)
{
  return templateWidget(item->listView());
}

toTemplate::toTemplate(QWidget *parent)
  : QVBox(parent),toHelpContext("template.html")
{
  Splitter=new QSplitter(Vertical,this);
  List=new toListView(Splitter);
  List->addColumn("Template");
  List->setRootIsDecorated(true);
  List->setSorting(0);
  List->setShowSortIndicator(false);
  List->setTreeStepSize(10);
  Frame=new QVBox(Splitter);

  connect(List,SIGNAL(expanded(QListViewItem *)),this,SLOT(expand(QListViewItem *)));
  connect(List,SIGNAL(collapsed(QListViewItem *)),this,SLOT(collapse(QListViewItem *)));

  if (toTemplateProvider::Providers)
    for (std::list<toTemplateProvider *>::iterator i=toTemplateProvider::Providers->begin();
	 i!=toTemplateProvider::Providers->end();
	 i++)
      (*i)->insertItems(List);

  WidgetExtra=NULL;
  setWidget(NULL);
}

toTemplate::~toTemplate()
{
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

void toTemplate::collapse(QListViewItem *item)
{
  try {
    toTemplateItem *ti=dynamic_cast<toTemplateItem *>(item);
    if (ti)
      ti->collapse();
  } catch (...) {
  }
}

void toTemplateItem::setSelected(bool sel)
{
  toTemplate *temp=toTemplate::templateWidget(this);
  if (sel&&temp)
    temp->setWidget(selectedWidget(toTemplate::parentWidget(this)));
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
  { }
  void addFile(QListView *parent,const QString &root,const QString &file);
  virtual void insertItems(QListView *parent);
};

void toTextTemplate::insertItems(QListView *parent)
{
  int tot=TemplateTool.config("Number",QString::number(-1)).toInt();
  if(tot!=-1) {
    for(int i=0;i<tot;i++) {
      QString num=QString::number(i);
      QString root=TemplateTool.config(num,"");
      num+="file";
      QString file=TemplateTool.config(num,"");
      addFile(parent,root,file);
    }
  } else {
    QString file=toPluginPath();
    file+="/sqlfunctions.tpl";
    addFile(parent,"PL/SQL Functions",file);
  }
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

void toTemplateSQL::expand(void)
{
  while(firstChild())
    delete firstChild();
  try {
    toQuery query(connection(),SQL,parameters());
    while(!query.eof()) {
      createChild(query.readValue());
      for (int j=1;j<query.columns();j++)
	query.readValue();
    }
  } TOCATCH
}

static toTextTemplate TextTemplate;
