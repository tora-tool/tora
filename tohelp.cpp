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
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qtextbrowser.h>
#include <qtextview.h>
#include <qcombobox.h>
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

#ifdef TO_KDE
#include "tohelpbrowser.h"
#include "tohelpbrowser.moc"

toHelpBrowser::toHelpBrowser(QWidget *parent,const char *name)
  : KHTMLPart(parent,name)
{
  connect(browserExtension(),
	  SIGNAL(openURLRequest(const KURL &,const KParts::URLArgs &)),
	  this,
	  SLOT(openURLRequest(const KURL &,const KParts::URLArgs &)));
}

void toHelpBrowser::openURLRequest(const KURL &url,const KParts::URLArgs &)
{
  emit textChanged();
  openURL(url);
}

bool toHelpBrowser::openURL(const KURL &url)
{
  if (Forward.size()>0) {
    emit forwardAvailable(false);
    Forward.clear();
  }
  if (Backward.size()==1)
    emit backwardAvailable(true);
  toPush(Backward,url.url());
  return KHTMLPart::openURL(url);
}

void toHelpBrowser::backward(void)
{
  toPush(Forward,toPop(Backward));
  QString url=(*Backward.rbegin());
  if (Forward.size()==1)
    emit forwardAvailable(true);
  if (Backward.size()==1)
    emit backwardAvailable(false);
  KHTMLPart::openURL(url);
  emit textChanged();
}

void toHelpBrowser::forward(void)
{
  QString url=toPop(Forward);
  if (Forward.size()==0)
    emit forwardAvailable(false);
  if (Backward.size()==1)
    emit backwardAvailable(true);
  toPush(Backward,url);
  KHTMLPart::openURL(url);
  emit textChanged();
}

#endif

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
  virtual void oracleManuals(void)
  {
    QString filename=toOpenFilename(QString::null,"index.htm*",this);
    try {
      toHtml file(toReadFile(filename));
      QString dsc;
      bool inDsc=false;
      QRegExp isToc("toc\\.html?$");
      while(!file.eof()) {
	file.nextTag();
	if (file.isTag()) {
	  if (file.open()&&!strcmp(file.tag(),"a")) {
	    QString href=toHelp::path(filename);
	    href+=QString::fromLatin1(file.value("href"));
	    if (!href.isEmpty()&&
		!dsc.isEmpty()&&
		href.find(isToc)>=0&&
		file.value("title")) {
	      new QListViewItem(FileList,dsc.simplifyWhiteSpace(),href);
	      inDsc=false;
	      dsc=QString::null;
	    }
	  } else if (file.open()&&!strcmp(file.tag(),"dd")) {
	    dsc=QString::null;
	    inDsc=true;
	  }
	} else if (inDsc)
	  dsc+=QString::fromLatin1(file.text());
      }
    } catch (const QString &str) {
      TOMessageBox::warning(toMainWidget(),"File error",str);
    }
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
  QToolBar *toolbar=toAllocBar(this,"Help Navigation",QString::null);
  l->addWidget(toolbar);

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
  Manuals=new QComboBox(box);
  Manuals->insertItem("All manuals");
  Result=new toListView(box);
  Result->setSorting(0);
  Result->addColumn("Result");
  Result->addColumn("Manual");
  connect(Sections,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeContent(QListViewItem *)));
  connect(Result,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeContent(QListViewItem *)));

#ifdef TO_KDE
  Help=new toHelpBrowser(splitter);
#else
  Help=new QTextBrowser(splitter);
  Help->mimeSourceFactory()->addFilePath(path());
#endif
  // Help->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
  setCaption("TOra Help Browser");

  QToolButton *button;
  button=new QToolButton(LeftArrow,toolbar);
  connect(Help,SIGNAL(backwardAvailable(bool)),
	  button,SLOT(setEnabled(bool)));
  button->setEnabled(false);
  connect(button,SIGNAL(clicked(void)),
	  Help,SLOT(backward(void)));
  QToolTip::add(button,"Backward one help page");

  button=new QToolButton(RightArrow,toolbar);
  connect(Help,SIGNAL(forwardAvailable(bool)),
	  button,SLOT(setEnabled(bool)));
  button->setEnabled(false);
  connect(button,SIGNAL(clicked(void)),
	  Help,SLOT(forward(void)));
  QToolTip::add(button,"Forward one help page");

  connect(Help,SIGNAL(textChanged(void)),
	  this,SLOT(removeSelection(void)));

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

  splitter->setResizeMode(tabs,QSplitter::KeepSize);
  setGeometry(x(),y(),max(width(),640),max(height(),480));

  QListViewItem *lastParent=NULL;
  for(map<QString,QString>::iterator i=Dsc.begin();i!=Dsc.end();i++) {
    try {
      QString path=toHelp::path((*i).second);
      QString filename=(*i).second;
      QListViewItem *parent;
      if ((*i).first=="TOra manual") {
	parent=new QListViewItem(Sections,NULL,(*i).first,QString::null,filename);
	if (!lastParent)
	  parent=lastParent;
      } else
	parent=lastParent=new QListViewItem(Sections,lastParent,(*i).first,
					    QString::null,filename);
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
		  last->setText(2,filename);
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
  for (QListViewItem *item=Sections->firstChild();item;item=item->nextSibling())
    Manuals->insertItem(item->text(0));

  Progress=new QProgressBar(box);
  Progress->setTotalSteps(Dsc.size());
  Progress->hide();

  Searching=false;
}

toHelp::~toHelp()
{
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
  QString file=path();
  file+=context;
  Window->Help->openURL(file);
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
  Window->Help->openURL(item->text(2));
#else
  if (item->text(2).find("htm")>=0)
    Help->setTextFormat(RichText);
  else
    Help->setTextFormat(AutoText);
  if (!item->text(2).isEmpty())
    Help->setSource(item->text(2));
#endif
}

void toHelp::search(void)
{
  if (Searching)
    return;
  Result->clear();
  QStringList words=QStringList::split(QRegExp("\\s+"),SearchLine->text().lower());
  if (words.count()==0)
    return;
  QRegExp strip("\\d+-\\d+\\s*,\\s+");
  QRegExp stripend(",$");
  int steps=1;
  Progress->setProgress(0);
  Progress->show();
  Searching=true;
  qApp->processEvents();
  for (QListViewItem *parent=Sections->firstChild();parent;parent=parent->nextSibling()) {
    if (Manuals->currentItem()==0||parent->text(0)==Manuals->currentText()) {
      QString path=toHelp::path(parent->text(2));
      QString filename=path;
      filename+="index.htm";
      try {
	toHtml file(toReadFile(filename));
	list<QString> Context;
	bool inDsc=false;
	bool aRestart=true;
	QString dsc;
	QString href;
	while(!file.eof()) {
	  file.nextTag();
	  if (file.isTag()) {
	    if (file.open()) {
	      if (!strcmp(file.tag(),"a")) {
		href=QString::fromLatin1(file.value("href"));
		if (href[0]=='#')
		  href=QString::null;
		else if (href.find("..")>=0)
		  href=QString::null;
	      } else if (!strcmp(file.tag(),"dd")) {
		inDsc=true;
		aRestart=false;
		href=dsc=QString::null;
	      } else if (!strcmp(file.tag(),"dl")) {
		toPush(Context,dsc.simplifyWhiteSpace());
		href=dsc=QString::null;
		inDsc=true;
	      }
	    } else if (!strcmp(file.tag(),"a")) {
	      if (!dsc.isEmpty()&&
		  !href.isEmpty()) {
		QString tmp;
		for (list<QString>::iterator i=Context.begin();i!=Context.end();i++)
		  if (i!=Context.begin()&&!(*i).isEmpty()) {
		    tmp+=*i;
		    tmp+=", ";
		  }
		tmp+=dsc.simplifyWhiteSpace();
		QString url=path;
		url+=href;
		aRestart=true;

		bool incl=true;
		for (size_t i=0;i<words.count();i++)
		  if (!tmp.contains(words[i],false)) {
		    incl=false;
		    break;
		  }

		if (incl) {
		  tmp.replace(strip," ");
		  tmp.replace(stripend," ");
		  QListViewItem *item=new toResultViewItem(Result,NULL,tmp.simplifyWhiteSpace());
		  item->setText(1,parent->text(0));
		  item->setText(2,url);
		}
		href=QString::null;
	      }
	    } else if (!strcmp(file.tag(),"dl")) {
	      toPop(Context);
	    }
	  } else if (inDsc) {
	    dsc+=QString::fromLatin1(file.text());
	  }
	}
      } TOCATCH
    }
    Progress->setProgress(steps);
    steps++;
    qApp->processEvents();
  }
  Progress->hide();
  Searching=false;
}

void toHelp::removeSelection(void)
{
  Sections->clearSelection();
  Result->clearSelection();
}
