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
#include "tohighlightedtext.h"
#include "tomain.h"
#include "toresultview.h"
#include "tosql.h"
#include "tosqledit.h"
#include "totool.h"
#include "toworksheet.h"

#ifdef TO_KDE
#include <kfiledialog.h>
#endif

#include <qcombobox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qsplitter.h>
#include <qstring.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include "tosqledit.moc"

#include "icons/add.xpm"
#include "icons/commit.xpm"
#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/tosqledit.xpm"
#include "icons/trash.xpm"

class toSQLEditTool : public toTool {
protected:
  QWidget * Window;

public:
  toSQLEditTool()
    : toTool(920,"SQL Dictionary Editor")
  { Window=NULL; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    if (Window)
      Window->setFocus();
    else
      Window=new toSQLEdit(parent,connection);
    Window->setIcon(QPixmap((const char **)tosqledit_xpm));
    return Window;
  }
  virtual void customSetup(int toolid)
  {
    toMainWidget()->editMenu()->insertSeparator();
    toMainWidget()->editMenu()->insertItem(QPixmap((const char **)tosqledit_xpm),
					   qApp->translate("toSQLEditTool","&Edit SQL..."),toolid);
    toMainWidget()->registerSQLEditor(toolid);
  }
  void closeWindow(void)
  {
    Window=NULL;
  }
  virtual bool canHandle(toConnection &conn)
  { return true; }
};

static toSQLEditTool SQLEditTool;

void toSQLEdit::updateStatements(const QString &sel)
{
  Statements->clear();
  toSQL::sqlMap sql=toSQL::definitions();

  QListViewItem *head=NULL;
  QListViewItem *item=NULL;

  for(toSQL::sqlMap::iterator pos=sql.begin();pos!=sql.end();pos++) {
    QString str=(*pos).first;
    int i=str.find(QString::fromLatin1(":"));
    if (i>=0) {
      if (!head||head->text(0)!=str.left(i)) {
	head=new QListViewItem(Statements,str.left(i));
	head->setSelectable(false);
      }
      item=new QListViewItem(head,str.right(str.length()-i-1));
      if (sel==str) {
	Statements->setSelected(item,true);
	Statements->setCurrentItem(item);
	Statements->setOpen(head,true);
      }
    } else {
      head=new QListViewItem(Statements,head,str);
      if (sel==str) {
	Statements->setSelected(head,true);
	Statements->setCurrentItem(item);
      }
    }
  }
}

toSQLEdit::toSQLEdit(QWidget *main,toConnection &connection)
  : toToolWidget(SQLEditTool,"sqledit.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,tr("SQL editor"));

  new QToolButton(QPixmap((const char **)fileopen_xpm),
		  tr("Load SQL dictionary file"),
		  tr("Load SQL dictionary file"),
		  this,SLOT(loadSQL()),
		  toolbar);
  new QToolButton(QPixmap((const char **)filesave_xpm),
		  tr("Save modified SQL to dictionary file"),
		  tr("Save modified SQL to dictionary file"),
		  this,SLOT(saveSQL()),
		  toolbar);
  toolbar->addSeparator();
  CommitButton=new QToolButton(QPixmap((const char **)commit_xpm),
			       tr("Save this entry in the dictionary"),
			       tr("Save this entry in the dictionary"),
			       this,SLOT(commitChanges()),
			       toolbar);
  TrashButton=new QToolButton(QPixmap((const char **)trash_xpm),
			      tr("Delete this version from the SQL dictionary"),
			      tr("Delete this version from the SQL dictionary"),
			      this,SLOT(deleteVersion()),
			      toolbar);
  new QToolButton(QPixmap((const char **)add_xpm),
		  tr("Start new SQL definition"),
		  tr("Start new SQL definition"),
		  this,SLOT(newSQL()),
		  toolbar);
  CommitButton->setEnabled(true);
  TrashButton->setEnabled(false);
  toolbar->setStretchableWidget(new QLabel(QString::null,toolbar));

  QSplitter *splitter=new QSplitter(Horizontal,this);
  Statements=new toListView(splitter);
  Statements->setRootIsDecorated(true);
  Statements->addColumn(tr("Text Name"));
  Statements->setSorting(0);
  Statements->setSelectionMode(QListView::Single);
  QVBox *vbox=new QVBox(splitter);

  QHBox *hbox=new QHBox(vbox);
  new QLabel(tr("Name")+" ",hbox);
  Name=new QLineEdit(hbox);
  new QLabel(" "+tr("Database")+" ",hbox);
  Version=new QComboBox(hbox);
  Version->setEditable(true);
  Version->setDuplicatesEnabled(false);
  LastVersion=connection.provider()+":Any";
  Version->insertItem(LastVersion);

  QFrame *line = new QFrame(vbox);
  line->setFrameStyle(QFrame::HLine|QFrame::Sunken);
  new QLabel(tr("Description"),vbox);
  splitter=new QSplitter(Vertical,vbox);
  Description=new toMarkedText(splitter);
  Editor=new toWorksheet(splitter,connection,false);

  connectList(true);
  connect(Version,SIGNAL(activated(const QString &)),this,SLOT(changeVersion(const QString &)));
  connect(toMainWidget(),SIGNAL(sqlEditor(const QString &)),this,SLOT(editSQL(const QString &)));

  updateStatements();

  setFocusProxy(Statements);
}

void toSQLEdit::connectList(bool conn)
{
  if (conn)
    connect(Statements,SIGNAL(selectionChanged(void)),this,SLOT(selectionChanged(void)));
  else
    disconnect(Statements,SIGNAL(selectionChanged(void)),this,SLOT(selectionChanged(void)));
}

toSQLEdit::~toSQLEdit()
{
  SQLEditTool.closeWindow();
  toSQL::saveSQL(toTool::globalConfig(CONF_SQL_FILE,DEFAULT_SQL_FILE));
}

void toSQLEdit::loadSQL(void)
{
  try {
    QString filename=toOpenFilename(QString::null,QString::null,this);
    if (!filename.isEmpty()) {
      toSQL::loadSQL(filename);
      Filename=filename;
    }
  } TOCATCH
}

void toSQLEdit::saveSQL(void)
{
  QString filename=toSaveFilename(QString::null,QString::null,this);
  if (!filename.isEmpty()) {
    Filename=filename;
    toSQL::saveSQL(filename);
  }
}

void toSQLEdit::deleteVersion(void)
{
  QCString provider;
  QCString version;
  if (!splitVersion(Version->currentText(),provider,version))
    return;

  try {
    toSQL::deleteSQL(Name->text().latin1(),version,provider);
    Version->removeItem(Version->currentItem());

    if (Version->count()==0) {
      QListViewItem *item=toFindItem(Statements,Name->text());
      if (item) {
	connectList(false);
	delete item;
	connectList(true);
      }
      newSQL();
    } else
      selectionChanged(QString::fromLatin1(connection().provider()+":"+connection().version()));
  } TOCATCH
}

bool toSQLEdit::close(bool del)
{
  if (checkStore(false))
    return QVBox::close(del);
  return false;
}

bool toSQLEdit::splitVersion(const QString &split,QCString &provider,QCString &version)
{
  int found=split.find(QString::fromLatin1(":"));
  if (found<0) {
    TOMessageBox::warning(this,tr("Wrong format of version"),
			  tr("Should be database provider:version."),
			  tr("&Ok"));
    return false;
  }
  provider=split.mid(0,found).latin1();
  if (provider.length()==0) {
    TOMessageBox::warning(this,tr("Wrong format of version"),
			  tr("Should be database provider:version. Can't start with :."),
			  tr("&Ok"));
    return false;
  }
  version=split.mid(found+1).latin1();
  if (version.length()==0) {
    TOMessageBox::warning(this,tr("Wrong format of version"),
			  tr("Should be database provider:version. Can't end with the first :."),
			  tr("&Ok"));
    return false;
  }
  return true;
}

void toSQLEdit::commitChanges(bool changeSelected)
{
  QCString provider;
  QCString version;
  if (!splitVersion(Version->currentText(),provider,version))
    return;
  QString name=Name->text();
  QListViewItem *item=toFindItem(Statements,name);
  if (!item) {
    int i=name.find(QString::fromLatin1(":"));
    if (i>=0) {
      item=toFindItem(Statements,name.mid(0,i));
      if (!item)
	item=new QListViewItem(Statements,name.mid(0,i));
      item=new QListViewItem(item,name.mid(i+1));
    } else
      item=new QListViewItem(Statements,name);
  }
  connectList(false);
  if (changeSelected) {
    Statements->setSelected(item,true);
    Statements->setCurrentItem(item);
    if (item->parent()&&!item->parent()->isOpen())
      item->parent()->setOpen(true);
  }
  connectList(true);
  if (Description->text().isEmpty()) {
    TOMessageBox::warning(this,tr("Missing description"),
			  tr("No description filled in. This is necessary to save SQL."
			  " Adding undescribed as description."),
			  tr("&Ok"));
    Description->setText(tr("Undescribed"));
  }
  toSQL::updateSQL(name.latin1(),
		   Editor->editor()->text(),
		   Description->text(),
		   version,
		   provider);
  TrashButton->setEnabled(true);
  CommitButton->setEnabled(true);

  bool update=Name->edited();

  Name->setEdited(false);
  Description->setEdited(false);
  Editor->editor()->setEdited(false);
  LastVersion=Version->currentText();
  if (update)
    updateStatements(Name->text());
  Statements->setFocus();
}

bool toSQLEdit::checkStore(bool justVer)
{
  if ((Name->edited()||
       Editor->editor()->edited()||
       (!justVer&&Version->currentText()!=LastVersion)||
       Description->edited())&&
      Version->currentText().length()>0) {
    switch (TOMessageBox::information(this,tr("Modified SQL dictionary"),
				      tr("Save changes into the SQL dictionary"),
				      tr("&Yes"),tr("&No"),tr("Cancel"),0,2)) {
    case 0:
      commitChanges(false);
      break;
    case 1:
      Name->setEdited(false);
      Description->setEdited(false);
      Editor->editor()->setEdited(false);
      LastVersion=Version->currentText();
      return true;
    case 2:
      return false;
    }
  }
  return true;
}

void toSQLEdit::changeVersion(const QString &ver)
{
  if (!Editor->editor()->edited()||checkStore(true)) {
    selectionChanged(ver);
    if (Version->currentText()!=ver) {
      Version->insertItem(ver);
      Version->lineEdit()->setText(ver);
    }
  }
}

void toSQLEdit::selectionChanged(void)
{
  try {
    if (checkStore(false))
      selectionChanged(QString::fromLatin1(connection().provider()+":"+connection().version()));
  } TOCATCH
}

void toSQLEdit::changeSQL(const QString &name,const QString &maxver)
{
  toSQL::sqlMap sql=toSQL::definitions();
  Name->setText(name);
  Name->setEdited(false);
  
  QListViewItem *item=toFindItem(Statements,name);
  if (item) {
    connectList(false);
    Statements->setSelected(item,true);
    Statements->setCurrentItem(item);
    if (item->parent()&&!item->parent()->isOpen())
      item->parent()->setOpen(true);
    connectList(true);
  }

  Version->clear();
  LastVersion="";
  if (sql.find(name.latin1())!=sql.end()) {
    toSQL::definition &def=sql[name.latin1()];
    std::list<toSQL::version> &ver=def.Versions;
  
    Description->setText(def.Description);
    Description->setEdited(false);

    std::list<toSQL::version>::iterator j=ver.end();
    int ind;
    for (std::list<toSQL::version>::iterator i=ver.begin();i!=ver.end();i++) {
      QString str=(*i).Provider;
      str+=QString::fromLatin1(":");
      str+=(*i).Version;
      Version->insertItem(str);
      if (str<=maxver||j==ver.end()) {
	j=i;
	LastVersion=str;
	ind=Version->count()-1;
      }
    }
    if (j!=ver.end()) {
      Editor->editor()->setText((*j).SQL);
      TrashButton->setEnabled(true);
      CommitButton->setEnabled(true);
      Version->setCurrentItem(ind);
    }
  } else {
    Description->clear();
    Editor->editor()->clear();
    TrashButton->setEnabled(false);
    CommitButton->setEnabled(true);
  }
  if (LastVersion.isEmpty()) {
    LastVersion=QString::fromLatin1("Any:Any");
    Version->insertItem(LastVersion);
  }
  Editor->editor()->setEdited(false);
}

void toSQLEdit::selectionChanged(const QString &maxver)
{
  try {
    QListViewItem *item=Statements->selectedItem();
    if (item) {
      QString name=item->text(0);
      while(item->parent()) {
	item=item->parent();
	name.prepend(QString::fromLatin1(":"));
	name.prepend(item->text(0));
      }
      changeSQL(name,maxver);
    }
  } TOCATCH
}

void toSQLEdit::editSQL(const QString &nam)
{
  try {
    if (checkStore(false))
      changeSQL(nam,QString::fromLatin1(connection().provider()+":"+connection().version()));
  } TOCATCH
}

void toSQLEdit::newSQL(void)
{
  if (checkStore(false)) {
    QString name=Name->text();
    int found=name.find(QString::fromLatin1(":"));
    if(found<0)
      name=QString::null;
    else
      name=name.mid(0,found+1);
    try {
      changeSQL(name,QString::fromLatin1(connection().provider()+":Any"));
    } TOCATCH
  }
}

static toSQLTemplate SQLTemplate;

toSQLTemplateItem::toSQLTemplateItem(QListView *parent)
  : toTemplateItem(SQLTemplate,parent,qApp->translate("toSQL","SQL Dictionary"))
{
  setExpandable(true);
}

static QString JustLast(const QCString &str)
{
  int pos=str.findRev(":");
  if (pos>=0)
    return QString::fromLatin1(str.mid(pos+1));
  return QString::fromLatin1(str);
}

toSQLTemplateItem::toSQLTemplateItem(toSQLTemplateItem *parent,
				     const QCString &name)
  : toTemplateItem(parent,JustLast(name))
{
  Name=name;
  setExpandable(true);
}

void toSQLTemplateItem::expand(void)
{
  std::list<QCString> def;
  if (Name.isEmpty())
    def=toSQL::range(Name);
  else
    def=toSQL::range(Name+":");
  QCString last;
  for(std::list<QCString>::iterator sql=def.begin();sql!=def.end();sql++) {
    QCString name=*sql;
    if (!Name.isEmpty())
      name=name.mid(Name.length()+1);
    if (name.find(":")!=-1)
      name=name.mid(0,name.find(":"));
    if (name!=last) {
      if (Name.isEmpty())
	new toSQLTemplateItem(this,name);
      else
	new toSQLTemplateItem(this,Name+":"+name);
      last=name;
    }
  }
}

QString toSQLTemplateItem::allText(int col) const
{
  try {
    toSQL::sqlMap defs=toSQL::definitions();
    if (defs.find(Name)==defs.end())
      return QString::null;
    return toSQL::string(Name,toMainWidget()->currentConnection());
  } catch(...) {
    return QString::null;
  }
}

void toSQLTemplateItem::collapse(void)
{
  while(firstChild())
    delete firstChild();
}

QWidget *toSQLTemplateItem::selectedWidget(QWidget *parent)
{
  toHighlightedText *widget=new toHighlightedText(parent);
  widget->setReadOnly(true);
  widget->setText(allText(0));
  return widget;
}
