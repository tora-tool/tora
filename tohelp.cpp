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
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
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

TO_NAMESPACE;

#ifdef TO_KDE
#include <kfiledialog.h>
#include <khtml_part.h>
#endif

#include <qdir.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtextbrowser.h>
#include <qtextview.h>
#include <qtimer.h>
#include <qvbox.h>

#include "tomain.h"
#include "tohtml.h"
#include "tohelp.h"
#include "toconf.h"
#include "toresultview.h"
#include "totool.h"

#include "tohelp.moc"
#include "tohelpaddfileui.moc"
#include "tohelpsetupui.moc"

toHelp *toHelp::Window;

class toHelpAddFile : public toHelpAddFileUI {
public:
  toHelpAddFile(QWidget *parent,const char *name=0)
    : toHelpAddFileUI(parent,name,true)
  { OkButton->setEnabled(false); }
  virtual void browse(void)
  {
    QString filename=toOpenFilename(Filename->text(),"toc.htm*",this);
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

class toHelpPrefs : public toHelpSetupUI, public toSettingTab
{
  toTool *Tool;
public:
  toHelpPrefs(toTool *tool,QWidget *parent,const char *name=0)
    : toHelpSetupUI(parent,name),Tool(tool)
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
    toHelpAddFile file(this);
    if (file.exec())
      new QListViewItem(FileList,file.Root->text(),file.Filename->text());
  }
  virtual void delFile(void)
  {
    delete FileList->selectedItem();
  }
};

class toHelpTool : public toTool {
public:
  toHelpTool()
    : toTool(501,"Additional Help")
  { }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  { return NULL; }
  virtual QWidget *configurationTab(QWidget *parent)
  { return new toHelpPrefs(this,parent); }
};

static toHelpTool HelpTool;

toHelp::toHelp(QWidget *parent,const char *name)
  : QDialog(parent,name,false,WDestructiveClose)
{
  Window=this;
  QBoxLayout *l=new QVBoxLayout(this);
  QSplitter *splitter=new QSplitter(Horizontal,this);
  l->addWidget(splitter);

  QTabWidget *tabs=new QTabWidget(splitter);
  Sections=new toListView(tabs);
  Sections->addColumn("Contents");
  Sections->setSorting(-1);
  Sections->setRootIsDecorated(true);

  tabs->addTab(Sections,"Contents");
  QVBox *box=new QVBox(tabs);
  tabs->addTab(box,"Search");
  SearchLine=new QLineEdit(box);
  connect(SearchLine,SIGNAL(returnPressed()),this,SLOT(search()));
  Result=new toListView(box);
  Result->addColumn("Result");
  connect(Sections,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeContent(QListViewItem *)));
  connect(Result,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeContent(QListViewItem *)));

#ifdef TO_KDE
  Help=new KHTMLPart(splitter);
#else  
  Help=new QTextBrowser(splitter);
#endif
  setCaption("TOra Help Browser");
  splitter->setResizeMode(tabs,QSplitter::KeepSize);
  setGeometry(x(),y(),640,480);

  map<QString,QString> Dsc;
  Dsc["TOra manual"]=toHelpPath();
  int tot=HelpTool.config("Number",QString::number(-1)).toInt();
  if(tot!=-1) {
    for(int i=0;i<tot;i++) {
      QString num=QString::number(i);
      QString dsc=HelpTool.config(num,"");
      num+="file";
      QString file=HelpTool.config(num,"");
      Dsc[dsc]=file;
    }
  }
#ifndef TO_KDE
  Help->mimeSourceFactory()->addFilePath(path());
#endif
  for(map<QString,QString>::iterator i=Dsc.begin();i!=Dsc.end();i++) {
    try {
      QString path=toHelp::path((*i).second);
      QString filename=path;
      QListViewItem *parent=new QListViewItem(Sections,NULL,(*i).first,filename);
      toHtml file(toReadFile(filename));
      bool inA=false;
      QString dsc;
      QString href;
      QListViewItem *last=NULL;
      while(!file.eof()) {
	file.nextTag();

	if (!file.isTag()) {
	  if (inA) {
	    dsc+=QString::fromLatin1(file.text());
	    dsc=dsc.simplifyWhiteSpace();
	  }
	} else {
	  const char *c=file.tag();
	  if (!strcmp(c,"a")) {
	    if (file.open()) {
	      href=QString::fromLatin1(file.value("href"));
	      if (!href.isEmpty())
		inA=true;
	    } else {
	      if (inA&&
		  !dsc.isEmpty()&&
		  !href.isEmpty()) {
		if (href.find("//")<0&&
		    href.find("..")<0) {
		  last=new QListViewItem(parent,last,dsc);
		  filename=path;
		  filename+="/";
		  filename+=href;
		  last->setText(1,filename);
		}
		dsc="";
	      }
	      inA=false;
	    }
	  } else if (!strcmp(c,"dl")) {
	    if (file.open()) {
	      if (!last)
		last=new QListViewItem(parent,NULL,"--------");
	      parent=last;
	      last=NULL;
	    } else {
	      last=parent;
	      parent=parent->parent();
	      if (!parent)
		throw QString("Missing parent, unbalanced dl in help file content");
	    }
	  }
	}
      }
    } TOCATCH
  }

  Progress=new QProgressBar(box);
#if 0
  Progress->setTotalSteps(Files->size());
#endif
  Progress->hide();

  Searching=false;
}

toHelp::~toHelp()
{
  Quit=true;
  while (Searching)
    qApp->processEvents();
  Window=NULL;
}

QString toHelp::path(const QString &path=QString::null)
{
  QString cur;
  if (path.isNull())
    cur=toHelpPath();
  else
    cur=path;
  cur.replace(QRegExp("[^/]+$"),"");
  return cur;
}

void toHelp::displayHelp(const QString &context)
{
  if (!Window)
    new toHelp(toMainWidget(),"Help window");
#ifdef TO_KDE
  QString file="file://";
  file+=path();
  file+=context;
  Window->Help->openURL(KURL(file));
#else
  if (context.find("htm")>=0)
    Window->Help->setTextFormat(RichText);
  else
    Window->Help->setTextFormat(AutoText);
  Window->Help->setSource(context);
#endif
  Window->show();
}

void toHelp::changeContent(QListViewItem *item)
{
#ifdef TO_KDE
  QString file="file://";
  file+=item->text(1);
  Window->Help->openURL(KURL(file));
#else
  if (item->text(1).find("htm")>=0)
    Help->setTextFormat(RichText);
  else
    Help->setTextFormat(AutoText);
  if (!item->text(1).isEmpty())
    Help->setSource(item->text(1));
#endif
}

void toHelp::search(void)
{
}
