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
#include <qstring.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qsplitter.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>

#include "totool.h"
#include "toconf.h"
#include "tomarkedtext.h"
#include "tomain.h"
#include "tosql.h"
#include "toworksheet.h"
#include "tosqledit.h"

#include "tosqledit.moc"

#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/trash.xpm"
#include "icons/commit.xpm"

class toSQLEditTool : public toTool {
protected:
  QWidget * Window;

public:
  toSQLEditTool()
    : toTool(201,"SQL Dictionary Editor")
  { Window=NULL; }
  virtual const char *menuItem()
  { return "Edit SQL"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    if (Window)
      Window->setFocus();
    else
      Window=new toSQLEdit(parent,connection);
    return Window;
  }
  virtual void customSetup(void)
  {
    
  }
  void closeWindow(void)
  {
    Window=NULL;
  }
};

static toSQLEditTool SQLEditTool;

void toSQLEdit::updateStatements(void)
{
  Statements->clear();
  toSQL::sqlMap sql=toSQL::definitions();

  QListViewItem *head=NULL;
  QListViewItem *item=NULL;

  for(toSQL::sqlMap::iterator pos=sql.begin();pos!=sql.end();pos++) {
    QString str=(*pos).first;
    int i=str.find(":");
    if (i>=0) {
      if (!head||head->text(0)!=str.left(i)) {
	head=new QListViewItem(Statements,head,str.left(i));
	head->setSelectable(false);
      }
      item=new QListViewItem(head,item,str.right(str.length()-i-1));
    } else
      head=new QListViewItem(Statements,head,str);
  }
}

static QPixmap *toLoadPixmap;
static QPixmap *toSavePixmap;
static QPixmap *toCommitPixmap;
static QPixmap *toTrashPixmap;

toSQLEdit::toSQLEdit(QWidget *main,toConnection &connection)
  : QVBox(main,NULL,WDestructiveClose),Connection(connection)
{
  if (!toLoadPixmap)
    toLoadPixmap=new QPixmap((const char **)fileopen_xpm);
  if (!toSavePixmap)
    toSavePixmap=new QPixmap((const char **)filesave_xpm);
  if (!toCommitPixmap)
    toCommitPixmap=new QPixmap((const char **)commit_xpm);
  if (!toTrashPixmap)
    toTrashPixmap=new QPixmap((const char **)trash_xpm);

  QToolBar *toolbar=new QToolBar("Edit SQL",toMainWidget(),this);
  new QToolButton(*toLoadPixmap,
		  "Load SQL dictionary file",
		  "Load SQL dictionary file",
		  this,SLOT(loadSQL()),
		  toolbar);
  new QToolButton(*toSavePixmap,
		  "Save modified SQL to dictionary file",
		  "Save modified SQL to dictionary file",
		  this,SLOT(saveSQL()),
		  toolbar);
  toolbar->addSeparator();
  CommitButton=new QToolButton(*toCommitPixmap,
			       "Save this entry in the dictionary",
			       "Save this entry in the dictionary",
			       this,SLOT(commitChanges()),
			       toolbar);
  TrashButton=new QToolButton(*toTrashPixmap,
			      "Delete this version from the SQL dictionary",
			      "Delete this version from the SQL dictionary",
			      this,SLOT(deleteVersion()),
			      toolbar);
  CommitButton->setEnabled(false);
  TrashButton->setEnabled(false);
  toolbar->setStretchableWidget(new QLabel("",toolbar));

  QSplitter *splitter=new QSplitter(Horizontal,this);
  Statements=new QListView(splitter);
  Statements->setRootIsDecorated(true);
  Statements->addColumn("Text Name");
  QVBox *vbox=new QVBox(splitter);
  QHBox *hbox=new QHBox(vbox);
  Name=new QLineEdit(hbox);
  Version=new QComboBox(hbox);
  Version->setEditable(true);
  Version->setDuplicatesEnabled(false);
  splitter=new QSplitter(Vertical,vbox);
  Description=new toMarkedText(splitter);
  Editor=new toWorksheet(splitter,Connection,false);

  connect(Statements,SIGNAL(selectionChanged(void)),this,SLOT(selectionChanged(void)));
  connect(Version,SIGNAL(activated(const QString &)),this,SLOT(changeVersion(const QString &)));

  updateStatements();

  Connection.addWidget(this);
}

toSQLEdit::~toSQLEdit()
{
  SQLEditTool.closeWindow();
  Connection.delWidget(this);
  toSQL::saveSQL(toTool::globalConfig(CONF_SQL_FILE,DEFAULT_SQL_FILE));
}

void toSQLEdit::loadSQL(void)
{
  QString filename=QFileDialog::getOpenFileName(QString::null,"*.sql\n*.txt",this);
  if (!filename.isEmpty()) {
    toSQL::loadSQL(filename);
    Filename=filename;
  }
}

void toSQLEdit::saveSQL(void)
{
  QString filename=QFileDialog::getSaveFileName(QString::null,"*.sql\n*.txt",this);
  if (!filename.isEmpty()) {
    Filename=filename;
    toSQL::saveSQL(filename);
  }
}

void toSQLEdit::deleteVersion(void)
{
  toSQL::deleteSQL(Name->text(),Version->currentText());

  selectionChanged(Connection.version());
  if (Version->count()==0) {
    TrashButton->setEnabled(false);
    CommitButton->setEnabled(false);
    updateStatements();
    Name->clear();
    Description->clear();
  }
}

bool toSQLEdit::close(bool del)
{
  if (checkStore(false))
    return QVBox::close(del);
  return false;
}

void toSQLEdit::commitChanges(void)
{
  toSQL::updateSQL(Name->text(),
		   Editor->editor()->text(),
		   Description->text(),
		   Version->currentText());
  TrashButton->setEnabled(true);
  CommitButton->setEnabled(true);

  Name->setEdited(false);
  Description->setEdited(false);
  Editor->editor()->setEdited(false);
  LastVersion=Version->currentText();
}

bool toSQLEdit::checkStore(bool justVer)
{
  if ((Name->edited()||
       Editor->editor()->edited()||
       (!justVer&&Version->currentText()!=LastVersion)||
       Description->edited())&&
      Version->currentText().length()>0) {
    switch (QMessageBox::information(this,"Modified SQL dictionary",
				     "Save changes into the SQL dictionary",
				     "&Yes","&No","&Cancel",0,2)) {
    case 0:
      toSQL::updateSQL(Name->text(),
		       Editor->editor()->text(),
		       Description->text(),
		       justVer?LastVersion:Version->currentText());
      TrashButton->setEnabled(true);
      CommitButton->setEnabled(true);
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
  if (!Editor->editor()->edited()||checkStore(true))
    selectionChanged(ver);
}

void toSQLEdit::selectionChanged(void)
{
  if (checkStore(false))
    selectionChanged(Connection.version());
}

void toSQLEdit::selectionChanged(const QString &maxver)
{
  QListViewItem *item=Statements->selectedItem();
  if (item) {
    QString name=item->text(0);
    while(item->parent()) {
      item=item->parent();
      name.prepend(":");
      name.prepend(item->text(0));
    }
    toSQL::sqlMap sql=toSQL::definitions();
    toSQL::definition &def=sql[name];
    list<toSQL::version> &ver=def.Versions;

    Name->setText(name);
    Name->setEdited(false);
    Description->setText(def.Description);
    Description->setEdited(false);

    Version->clear();
    LastVersion="";
    list<toSQL::version>::iterator j=ver.end();
    int ind;
    for (list<toSQL::version>::iterator i=ver.begin();i!=ver.end();i++) {
      Version->insertItem((*i).Version);
      if ((*i).Version<=maxver||j==ver.end()) {
	j=i;
	LastVersion=(*i).Version;
	ind=Version->count()-1;
      }
    }
    if (j!=ver.end()) {
      Editor->editor()->setText((*j).SQL);
      TrashButton->setEnabled(true);
      CommitButton->setEnabled(true);
      Version->setCurrentItem(ind);
    } else {
      Editor->editor()->clear();
      TrashButton->setEnabled(false);
      CommitButton->setEnabled(false);
    }
    Editor->editor()->setEdited(false);
  }
}
