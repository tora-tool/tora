//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003 Quest Software, Inc
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "toconf.h"
#include "tohelp.h"
#include "tohtml.h"
#include "tomain.h"
#include "toresultview.h"
#include "totool.h"

#ifdef TO_KDE
#include <kfiledialog.h>
#include <khtml_part.h>
#endif

#include <qaccel.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtextbrowser.h>
#include <qtextview.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvbox.h>

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
  bool ret=KHTMLPart::openURL(url);
  emit textChanged();
  return ret;
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

QString toHelpBrowser::source(void)
{
  if (Backward.begin()==Backward.end())
    return QString::null;
  return (*Backward.rbegin());
}

#endif

toHelp *toHelp::Window;

class toHelpAddFile : public toHelpAddFileUI {
public:
  toHelpAddFile(QWidget *parent,const char *name=0)
    : toHelpAddFileUI(parent,name,true)
  {
    OkButton->setEnabled(false);
    toHelp::connectDialog(this);
  }
  virtual void browse(void)
  {
    QString filename=toOpenFilename(Filename->text(),QString::fromLatin1("toc.htm*"),this);
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
    : toHelpSetupUI(parent,name),toSettingTab("additionalhelp.html"),Tool(tool)
  {
    int tot=Tool->config("Number","-1").toInt();
    if(tot!=-1) {
      for(int i=0;i<tot;i++) {
	QString num=QString::number(i);
	QString root=Tool->config(num.latin1(),"");
	num+=QString::fromLatin1("file");
	QString file=Tool->config(num.latin1(),"");
	new QListViewItem(FileList,root,file);
      }
    }
  }
  virtual void saveSetting(void)
  {
    int i=0;
    for(QListViewItem *item=FileList->firstChild();item;item=item->nextSibling()) {
      QString nam=QString::number(i);
      Tool->setConfig(nam.latin1(),item->text(0));
      nam+=QString::fromLatin1("file");
      Tool->setConfig(nam.latin1(),item->text(1));
      i++;
    }
    Tool->setConfig("Number",QString::number(i));
    delete toHelp::Window;
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
    QString filename=toOpenFilename(QString::null,QString::fromLatin1("*index.htm*"),this);
    try {
      toHtml file(toReadFile(filename));
      QString dsc;
      bool inDsc=false;
      QRegExp isToc(QString::fromLatin1("toc\\.html?$"));
      while(!file.eof()) {
	file.nextToken();
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
	  } else if (file.open()&&(!strcmp(file.tag(),"dd")||!strcmp(file.tag(),"book"))) {
	    dsc=QString::null;
	    inDsc=true;
	  }
	} else if (inDsc)
	  dsc+=QString::fromLatin1(file.text());
      }
    } catch (const QString &str) {
      TOMessageBox::warning(toMainWidget(),qApp->translate("toHelpPrefs","File error"),str);
    }
  }
};

QWidget *toHelpTool::configurationTab(QWidget *parent)
{
#ifdef TOAD
  return NULL;
#else
  return new toHelpPrefs(this,parent);
#endif
}

void toHelpTool::displayHelp(void)
{
  QWidget *cur=qApp->focusWidget();
  while(cur) {
    QDialog *dlg=dynamic_cast<QDialog *>(cur);
    if (dlg) {
      toHelp::displayHelp(dlg);
      return;
    }
    cur=cur->parentWidget();
  }
  // No dialog found
  toHelp::displayHelp();
}

static toHelpTool HelpTool;

toHelp::toHelp(QWidget *parent,const char *name,bool modal)
  : QDialog(parent,name,modal,
	    WStyle_Customize|WStyle_NormalBorder|
	    WStyle_Title|WStyle_SysMenu|
	    WStyle_Minimize|WStyle_Maximize)
{
  if (!modal)
    Window=this;
  QBoxLayout *l=new QVBoxLayout(this);
  QToolBar *toolbar=toAllocBar(this,tr("Help Navigation"));
  l->addWidget(toolbar);

  QSplitter *splitter=new QSplitter(Horizontal,this);
  l->addWidget(splitter);

  QTabWidget *tabs=new QTabWidget(splitter);
  Sections=new toListView(tabs);
  Sections->addColumn(tr("Contents"));
  Sections->setSorting(-1);
  Sections->setRootIsDecorated(true);

  tabs->addTab(Sections,tr("Contents"));
  QVBox *box=new QVBox(tabs);
  tabs->addTab(box,tr("Search"));
  SearchLine=new QLineEdit(box);
  connect(SearchLine,SIGNAL(returnPressed()),this,SLOT(search()));
  Manuals=new QComboBox(box);
  Manuals->insertItem(tr("All manuals"));
  Result=new toListView(box);
  Result->setSorting(0);
  Result->addColumn(tr("Result"));
  Result->addColumn(tr("Manual"));
  Result->setSelectionMode(QListView::Single);
  Sections->setSelectionMode(QListView::Single);
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
  setCaption(tr("Help Browser"));

  connect(Help,SIGNAL(textChanged(void)),
	  this,SLOT(removeSelection(void)));

  QToolButton *button;
  button=new QToolButton(LeftArrow,toolbar);
  connect(Help,SIGNAL(backwardAvailable(bool)),
	  button,SLOT(setEnabled(bool)));
  button->setEnabled(false);
  connect(button,SIGNAL(clicked(void)),
	  Help,SLOT(backward(void)));
  QToolTip::add(button,tr("Backward one help page"));

  button=new QToolButton(RightArrow,toolbar);
  connect(Help,SIGNAL(forwardAvailable(bool)),
	  button,SLOT(setEnabled(bool)));
  button->setEnabled(false);
  connect(button,SIGNAL(clicked(void)),
	  Help,SLOT(forward(void)));
  QToolTip::add(button,tr("Forward one help page"));

  toolbar->setStretchableWidget(new QLabel(toolbar,TO_KDE_TOOLBAR_WIDGET));

  std::map<QString,QString> Dsc;
  Dsc[tr(TOAPPNAME " manual")]=toHelpPath();
  int tot=HelpTool.config("Number","-1").toInt();
  if(tot!=-1) {
    for(int i=0;i<tot;i++) {
      QString num=QString::number(i);
      QString dsc=HelpTool.config(num.latin1(),"");
      num+=QString::fromLatin1("file");
      QString file=HelpTool.config(num.latin1(),"");
      Dsc[dsc]=file;
    }
  }

  splitter->setResizeMode(tabs,QSplitter::KeepSize);
  setGeometry(x(),y(),max(width(),640),max(height(),480));

  QListViewItem *lastParent=NULL;
  for(std::map<QString,QString>::iterator i=Dsc.begin();i!=Dsc.end();i++) {
    try {
      QString path=toHelp::path((*i).second);
      QString filename=(*i).second;
      QListViewItem *parent;
      if ((*i).first==tr("TOra manual")) {
	parent=new QListViewItem(Sections,NULL,(*i).first,QString::null,filename);
	if (!lastParent)
	  lastParent=parent;
      } else
	parent=lastParent=new QListViewItem(Sections,lastParent,(*i).first,
					    QString::null,filename);
      toHtml file(toReadFile(filename));
      bool inA=false;
      QString dsc;
      QCString href;
      QListViewItem *last=NULL;
      while(!file.eof()) {
	file.nextToken();

	if (!file.isTag()) {
	  if (inA) {
	    dsc+=QString::fromLatin1(file.text());
	    dsc=dsc.simplifyWhiteSpace();
	  }
	} else {
	  const char *c=file.tag();
	  if (!strcmp(c,"a")) {
	    if (file.open()) {
	      href=file.value("href");
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
		  filename+=QString::fromLatin1(href);
		  last->setText(2,filename);
		}
		dsc="";
	      }
	      inA=false;
	    }
	  } else if (!strcmp(c,"dl")) {
	    if (file.open()) {
	      if (!last)
		last=new QListViewItem(parent,NULL,QString::fromLatin1("--------"));
	      parent=last;
	      last=NULL;
	    } else {
	      last=parent;
	      parent=parent->parent();
	      if (!parent)
		throw tr("Missing parent, unbalanced dl in help file content");
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
  if (Window==this)
    Window=NULL;
}

QString toHelp::path(const QString &path)
{
  QString cur;
  if (path.isNull())
    cur=toHelpPath();
  else
    cur=path;
  cur.replace(QRegExp(QString::fromLatin1("[^/]+$")),QString::null);
  return cur;
}

void toHelp::displayHelp(const QString &context,QWidget *parent)
{
  toHelp *window;
  if (!Window||parent)
    window=new toHelp(NULL,tr("Help window"),parent);
  else
    window=Window;
  QString file=path();
#ifndef TOAD
  file+=context;
#else
  file+="/toc.htm";
#endif
#ifdef TO_KDE
  window->Help->openURL(file);
#else
  if (context.find("htm")>=0)
    window->Help->setTextFormat(RichText);
  else
    window->Help->setTextFormat(AutoText);
  window->Help->setSource(file);
#  if 0 // Necessary?
  window->removeSelection();
#  endif
#endif
  if (parent) {
    window->exec();
    delete window;
  } else
    window->show();
}

void toHelp::displayHelp(QWidget *parent)
{
  QWidget *cur=qApp->focusWidget();
  while(cur) {
    toHelpContext *ctx=dynamic_cast<toHelpContext *>(cur);
    if (ctx&&!ctx->context().isEmpty()) {
      toHelp::displayHelp(ctx->context(),parent);
      return;
    }
    cur=cur->parentWidget();
  }
  toHelp::displayHelp(QString::fromLatin1("toc.htm"),parent);
}

void toHelp::connectDialog(QDialog *dialog)
{
  QAccel *a=new QAccel(dialog);
  a->connectItem(a->insertItem(toKeySequence(tr("F1", "Dialog|Help"))),
		 &HelpTool,
		 SLOT(displayHelp()));
}

void toHelp::changeContent(QListViewItem *item)
{
#ifdef TO_KDE
  Help->openURL(item->text(2));
#else
  disconnect(Help,SIGNAL(textChanged(void)),
	     this,SLOT(removeSelection(void)));

  if (item->text(2).find("htm")>=0)
    Help->setTextFormat(RichText);
  else
    Help->setTextFormat(AutoText);
  if (!item->text(2).isEmpty())
    Help->setSource(item->text(2));

  connect(Help,SIGNAL(textChanged(void)),
	  this,SLOT(removeSelection(void)));
#endif
}

void toHelp::search(void)
{
  if (Searching)
    return;
  Result->clear();
  QStringList words=QStringList::split(QRegExp(QString::fromLatin1("\\s+")),SearchLine->text().lower());
  if (words.count()==0)
    return;
  QRegExp strip(QString::fromLatin1("\\d+-\\d+\\s*,\\s+"));
  QRegExp stripend(QString::fromLatin1(",$"));
  int steps=1;
  Progress->setProgress(0);
  Progress->show();
  Searching=true;
  qApp->processEvents();
  for (QListViewItem *parent=Sections->firstChild();parent;parent=parent->nextSibling()) {
    if (Manuals->currentItem()==0||parent->text(0)==Manuals->currentText()) {
      QString path=toHelp::path(parent->text(2));
      QString filename=path;
      filename+=QString::fromLatin1("index.htm");
      try {
	toHtml file(toReadFile(filename));
	std::list<QString> Context;
	bool inDsc=false;
	bool aRestart=true;
	QCString dsc;
	QCString href;
	while(!file.eof()) {
	  file.nextToken();
	  if (file.isTag()) {
	    if (file.open()) {
	      if (!strcmp(file.tag(),"a")) {
		href=file.value("href");
		if (href[0]=='#')
		  href="";
		else if (href.find("..")>=0)
		  href="";
	      } else if (!strcmp(file.tag(),"dd")) {
		inDsc=true;
		aRestart=false;
		href=dsc="";
	      } else if (!strcmp(file.tag(),"dl")) {
		toPush(Context,QString::fromLatin1(dsc.simplifyWhiteSpace()));
		href=dsc="";
		inDsc=true;
	      }
	    } else if (!strcmp(file.tag(),"a")) {
	      if (!dsc.isEmpty()&&
		  !href.isEmpty()) {
		QString tmp;
		for (std::list<QString>::iterator i=Context.begin();i!=Context.end();i++)
		  if (i!=Context.begin()&&!(*i).isEmpty()) {
		    tmp+=*i;
		    tmp+=QString::fromLatin1(", ");
		  }
		tmp+=QString::fromLatin1(dsc.simplifyWhiteSpace());
		QString url=path;
		url+=QString::fromLatin1(href);
		aRestart=true;

		bool incl=true;
		{
		  for (size_t i=0;i<words.count();i++)
		    if (!tmp.contains(words[i],false)) {
		      incl=false;
		      break;
		    }
		}

		if (incl) {
		  tmp.replace(strip,QString::fromLatin1(" "));
		  tmp.replace(stripend,QString::fromLatin1(" "));
		  QListViewItem *item=new toResultViewItem(Result,NULL,tmp.simplifyWhiteSpace());
		  item->setText(1,parent->text(0));
		  item->setText(2,url);
		}
		href="";
	      }
	    } else if (!strcmp(file.tag(),"dl")) {
	      toPop(Context);
	    }
	  } else if (inDsc) {
	    dsc+=file.text();
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

void toHelp::setSelection(toListView *lst,const QString &source)
{
  disconnect(lst,SIGNAL(selectionChanged(QListViewItem *)),
	     this,SLOT(changeContent(QListViewItem *)));

  bool any=false;

  QString t=source;
  t.replace(QRegExp(QString::fromLatin1("^file:")),QString::fromLatin1(""));

  QListViewItem *next=NULL;
  for (QListViewItem *item=lst->firstChild();item;item=next) {

    if ((item->text(2)==t)!=bool(item->isSelected())) {
      if (item->text(2)==t) {
	any=true;
	lst->setSelected(item,true);
	lst->ensureItemVisible(item);
	for(QListViewItem *parent=item->parent();parent;parent=parent->parent())
	  lst->setOpen(parent,true);
	break;
      } else
	lst->setSelected(item,false);
    }

    if (item->firstChild())
      next=item->firstChild();
    else if (item->nextSibling()) {
      next=item->nextSibling();
    } else {
      next=item;
      do {
	next=next->parent();
      } while(next&&!next->nextSibling());
      if (next)
	next=next->nextSibling();
    }
  }

  connect(lst,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeContent(QListViewItem *)));

  if (!any) {
    QString t=source;
    t.replace(QRegExp(QString::fromLatin1("#[^#]*$")),QString::null);
    if (t!=source)
      setSelection(lst,t);
  }
}

void toHelp::removeSelection(void)
{
  setSelection(Sections,Help->source());
  setSelection(Result,Help->source());
}
