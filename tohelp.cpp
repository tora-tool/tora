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

#include <qprogressbar.h>
#include <qdir.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtextbrowser.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qregexp.h>

#include "tomain.h"
#include "tohelp.h"
#include "toconf.h"
#include "toresultview.h"

#include "tohelp.moc"

toHelp *toHelp::Window;
map<QString,QString> *toHelp::Files;

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
  Sections->addColumn("File");
  Sections->setSorting(0);
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

  Help=new QTextBrowser(splitter);
  QString path=toHelpPath();
  Help->mimeSourceFactory()->addFilePath(path);
  setCaption("TOra Help Browser");
  splitter->setResizeMode(tabs,QSplitter::KeepSize);
  setGeometry(x(),y(),640,480);
  if (!Files) {
    Files=new map<QString,QString>;
    QDir d(path,"*.htm*",QDir::Name,QDir::Files);
    QRegExp begin("<TITLE>",false);
    QRegExp end("</TITLE>",false);
    for (unsigned int i=0;i<d.count();i++) {
      try {
	QString path=d.filePath(d[i]);
	QString file=toReadFile(path);
	int pos=file.find(begin);
	if (pos>=0) {
	  int epos=file.find(end,pos);
	  if (epos>=0)
	    (*Files)[file.mid(pos+7,epos-pos-7)]=path;
	}
      } TOCATCH
    }
  }

  QListViewItem *last=NULL;
  int lastLevel=0;
  QStringList lstCtx;
  for(map<QString,QString>::iterator i=Files->begin();i!=Files->end();i++) {
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
	last=new QListViewItem(Sections,ctx[lastLevel]);
      lastLevel++;
    }
    if (last)
      last=new QListViewItem(last,ctx[lastLevel]);
    else
      last=new QListViewItem(Sections,ctx[lastLevel]);
    last->setText(1,(*i).second);
    if ((*i).second==Help->source())
      last->setSelected(true);
    lstCtx=ctx;
    lastLevel++;
  }

  Progress=new QProgressBar(box);
  Progress->setTotalSteps(Files->size());
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

void toHelp::displayHelp(const QString &context)
{
  if (!Window)
    new toHelp(toMainWidget(),"Help window");
  Window->Help->setSource(context);
  Window->show();
}

void toHelp::changeContent(QListViewItem *item)
{
  if (item->text(1).find("htm")>=0)
    Help->setTextFormat(RichText);
  else
    Help->setTextFormat(AutoText);
  Help->setSource(item->text(1));
}

void toHelp::search(void)
{
  Quit=true;
  while (Searching)
    qApp->processEvents();
  Searching=true;
  Quit=false;
  QRegExp white("\\s+");
  QStringList words=QStringList::split(white,SearchLine->text());
  QRegExp re("<[^>]*>");
  Result->clear();
  int count=1;
  Progress->show();
  for (map<QString,QString>::iterator i=Files->begin();i!=Files->end()&&!Quit;i++) {
    try {
      QString file=toReadFile((*i).second);
      file.replace(re," ");
      file.simplifyWhiteSpace();
      file+=" ";
      file.prepend(" ");

      for(unsigned int j=0;j<words.count();j++) {
	QString word=" ";
	word+=words[j];
	word+=" ";
	if(file.find(word)<0)
	  break;
	if (j+1==words.count())
	  (new toResultViewItem(Result,NULL,(*i).first))->setText(1,(*i).second);
      }
    } TOCATCH
    Progress->setProgress(count++);
    qApp->processEvents();
  }
  Progress->hide();
  Searching=false;
  if (!Result->firstChild())
    new toResultViewItem(Result,NULL,"No hits");
}
