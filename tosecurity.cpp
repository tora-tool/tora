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

#include <qlabel.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qmessagebox.h>

#include "toconnection.h"
#include "tosecurity.h"
#include "tosql.h"
#include "totool.h"
#include "tomain.h"
#include "toresultview.h"

#include "tosecurity.moc"
#include "tosecurityroleui.moc"
#include "tosecurityuserui.moc"

#include "icons/refresh.xpm"
#include "icons/commit.xpm"
#include "icons/tosecurity.xpm"

static toSQL SQLUserInfo("toSecurity:UserInfo",
			 "SELECT Account_Status,
       Password,
       External_Name,
       Profile,
       Default_Tablespace,
       Temporary_Tablespace
  FROM DBA_Users
 WHERE UserName = :f1<char[31]>",
			 "Get information about a user, must have same columns and same binds.");

static toSQL SQLRoleInfo("toSecurity:RoleInfo",
			 "SELECT Role,Password_required FROM DBA_Roles WHERE Role = :f1<char[31]>",
			 "Get information about a role, must have same columns and same binds.");

static toSQL SQLProfiles("toSecurity:Profiles",
			 "SELECT DISTINCT Profile FROM DBA_Profiles ORDER BY Profile",
			 "Get profiles available.");

static toSQL SQLTablespace("toSecurity:Tablespaces",
			   "SELECT DISTINCT Tablespace_Name FROM DBA_Tablespaces"
			   " ORDER BY Tablespace_Name",
			   "Get tablespaces available.");

class toSecurityTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return tosecurity_xpm; }
public:
  toSecurityTool()
    : toTool(13,"Security Manager")
  { }
  virtual const char *menuItem()
  { return "Security Manager"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    QWidget *widget=new toSecurity(parent,connection);
    widget->setIcon(*toolbarImage());
    return widget;
  }
};

static toSecurityTool OutputTool;

static QPixmap *toRefreshPixmap;
static QPixmap *toCommitPixmap;

class toSecurityUser : public toSecurityUserUI {
  toConnection &Connection;
  enum {
    password,
    global,
    external
  } AuthType;
  QString OrgProfile;
  QString OrgDefault;
  QString OrgTemp;
  QString OrgGlobal;
  QString OrgPassword;

  bool OrgLocked;
  bool OrgExpired;
public:
  toSecurityUser(toConnection &conn,QWidget *parent);
  void clear(void);
  void changeUser(const QString &);
};

toSecurityUser::toSecurityUser(toConnection &conn,QWidget *parent)
  : toSecurityUserUI(parent),Connection(conn)
{
  otl_stream profiles(1,
		      SQLProfiles(Connection),
		      Connection.connection());
  while(!profiles.eof()) {
    char buf[100];
    profiles>>buf;
    Profile->insertItem(QString::fromUtf8(buf));
  }

  otl_stream tablespaces(1,
			 SQLTablespace(Connection),
			 Connection.connection());
  while(!tablespaces.eof()) {
    char buf[100];
    tablespaces>>buf;
    DefaultSpace->insertItem(QString::fromUtf8(buf));
    TempSpace->insertItem(QString::fromUtf8(buf));
  }
}

void toSecurityUser::clear(void)
{
  Name->setText("");
  Profile->setCurrentItem(0);
  Authentication->showPage(PasswordTab);
  Password->setText("");
  Password2->setText("");
  ExpirePassword->setChecked(false);
  TempSpace->setCurrentItem(0);
  DefaultSpace->setCurrentItem(0);
  Locked->setChecked(false);
  GlobalName->setText("");

  OrgProfile=OrgDefault=OrgTemp=OrgGlobal="";
  AuthType=password;
  Name->setEnabled(true);
  OrgLocked=OrgExpired=false;
}

void toSecurityUser::changeUser(const QString &user)
{
  clear();
  otl_stream query(1,
		   SQLUserInfo(Connection),
		   Connection.connection());
  query<<user.utf8();
  if (!query.eof()) {
    Name->setEnabled(false);
    Name->setText(user);

    char buffer[100];
    query>>buffer;
    QString str(QString::fromUtf8(buffer));
    if (str.startsWith("EXPIRES")) {
      ExpirePassword->setChecked(true);
      OrgExpired=true;
    } else if (str.startsWith("LOCKED")) {
      Locked->setChecked(true);
      OrgLocked=true;
    }

    query>>buffer;
    if (strlen(buffer)) {
      OrgPassword=QString::fromUtf8(buffer);
      Password->setText(OrgPassword);
      Password2->setText(OrgPassword);
      AuthType=password;
      query>>buffer;
    } else {
      query>>buffer;
      if (strlen(buffer)) {
	Authentication->showPage(GlobalTab);
	OrgGlobal=QString::fromUtf8(buffer);
	GlobalName->setText(OrgGlobal);
	AuthType=global;
      } else {
	Authentication->showPage(ExternalTab);
	AuthType=external;
      }
    }

    query>>buffer;
    str=QString::fromUtf8(buffer);
    for (int i=0;i<Profile->count();i++) {
      if (Profile->text(i)==str) {
	Profile->setCurrentItem(i);
	OrgProfile=str;
	break;
      }
    }

    query>>buffer;
    str=QString::fromUtf8(buffer);
    for (int i=0;i<DefaultSpace->count();i++) {
      if (DefaultSpace->text(i)==str) {
	DefaultSpace->setCurrentItem(i);
	OrgDefault=str;
	break;
      }
    }

    query>>buffer;
    str=QString::fromUtf8(buffer);
    for (int i=0;i<TempSpace->count();i++) {
      if (TempSpace->text(i)==str) {
	TempSpace->setCurrentItem(i);
	OrgTemp=str;
	break;
      }
    }
  }
}

class toSecurityRole : public toSecurityRoleUI {
  toConnection &Connection;
  enum {
    password,
    global,
    external,
    none
  } AuthType;
public:
  toSecurityRole(toConnection &conn,QWidget *parent)
    : toSecurityRoleUI(parent),Connection(conn)
  { }
  void changeRole(const QString &);
  void saveChanges(void);
};

void toSecurityRole::saveChanges(void)
{
  QString extra;
  if ((AuthType==password)&&(Authentication!=PasswordTab)) {
    if (Password->text()!=Password2->text()) {
      switch(QMessageBox::warning(this,
				  "Passwords don't match",
				  "The two versions of the password doesn't match",
				  "Don't save","Cancel")) {
      case 0:
	return;
      case 1:
	throw QString("Passwords don't match");
      }
    }
    if (Password->text().length()<=30) {
      extra=" IDENTIFIED BY \"";
      extra+=Password->text();
      extra+="\"";
    }
  } else if ((AuthType==global)&&(Authentication!=GlobalTab))
    extra=" IDENTIFIED GLOBALLY";
  else if ((AuthType==external)&&(Authentication!=ExternalTab))
    extra=" IDENTIFIED EXTERNALLY";
  else if ((AuthType==none)&&(Authentication!=NoneTab))
    extra=" NOT IDENTIFIED";
  QString sql;
  if (Name->isEnabled())
    sql="CREATE ";
  else
    sql="ALTER ";
  sql+="ROLE \"";
  sql+=Name->text();
  sql+="\"";
  sql+=extra;
  try {
    otl_cursor::direct_exec(Connection.connection(),sql.utf8());
  } TOCATCH
}

void toSecurityRole::changeRole(const QString &role)
{
  otl_stream query(1,
		   SQLRoleInfo(Connection),
		   Connection.connection());
  query<<role.utf8();
  if (!query.eof()) {
    Name->setText(role);
    Name->setEnabled(false);

    char buffer[100];
    query>>buffer;
    QString str(QString::fromUtf8(buffer));
    Password->setText("");
    Password2->setText("");
    if (str=="YES") {
      AuthType=password;
      Authentication->showPage(PasswordTab);
      Password->setText("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
      Password2->setText("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    } else if (str=="GLOBAL") {
      AuthType=global;
      Authentication->showPage(GlobalTab);
    } else if (str=="EXTERNAL") {
      AuthType=external;
      Authentication->showPage(ExternalTab);
    } else {
      AuthType=none;
      Authentication->showPage(NoneTab);
    }
  } else {
    Name->setEnabled(true);
    AuthType=none;
  }
}

class toSecurityPage : public QVBox {
  toSecurityRole *Role;
  toSecurityUser *User;
public:
  toSecurityPage(toConnection &conn,QWidget *parent)
    : QVBox(parent)
  {
    Role=new toSecurityRole(conn,this);
    Role->hide();
    User=new toSecurityUser(conn,this);
  }
  void changePage(const QString &nam,bool user)
  {
    if (user) {
      Role->hide();
      User->show();
      User->changeUser(nam);
    } else {
      User->hide();
      Role->show();
      Role->changeRole(nam);
    }
  }
};

toSecurity::toSecurity(QWidget *main,toConnection &connection)
  : QVBox(main,NULL,WDestructiveClose),Connection(connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);
  if (!toCommitPixmap)
    toCommitPixmap=new QPixmap((const char **)commit_xpm);

  QToolBar *toolbar=new QToolBar("SQL Output",toMainWidget(),this);
  new QToolButton(*toRefreshPixmap,
		  "Poll for output now",
		  "Poll for output now",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(*toCommitPixmap,
		  "Save changes",
		  "Save changes",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel("",toolbar));

  Connection.addWidget(this);

  QSplitter *splitter=new QSplitter(Horizontal,this);
  UserList=new toResultView(false,false,Connection,splitter);
  UserList->addColumn("Users/Roles");
  UserList->setSQLName("toSecurity:Users/Roles");
  UserList->setRootIsDecorated(true);
  Tabs=new QTabWidget(splitter);
  General=new toSecurityPage(Connection,Tabs);
  Tabs->addTab(General,"General");
  ObjectGrant=new toResultView(false,false,Connection,Tabs);
  Tabs->addTab(ObjectGrant,"Objects");
  connect(UserList,SIGNAL(selectionChanged()),this,SLOT(changeUser()));
  refresh();
}

toSecurity::~toSecurity()
{
  Connection.delWidget(this);
}

static toSQL SQLRoles("toSecurity:Roles",
		      "SELECT Role FROM Dba_Roles ORDER BY Role",
		      "Get roles available in DB, should return one entry");

void toSecurity::changeUser(void)
{
  try {
    QString sel;
    QListViewItem *item=UserList->selectedItem();
    if (item) {
      sel=item->text(1);
      if (sel[4]!=':')
	throw QString("Invalid security ID");
      bool user=false;
      if (sel.startsWith("USER"))
	user=true;
      General->changePage(sel.right(sel.length()-5),user);
    }
  } TOCATCH
}

void toSecurity::refresh(void)
{
  QString sel;
  {
    QListViewItem *item=UserList->selectedItem();
    if (item)
      sel=item->text(1);
  }

  UserList->clear();
  try {
    QListViewItem *parent=new toResultViewItem(UserList,NULL,"Users");
    parent->setOpen(true);
    parent->setSelectable(false);
    otl_stream user(1,
		    toSQL::sql(TOSQL_USERLIST,Connection),
		    Connection.connection());
    QListViewItem *item=NULL;
    while(!user.eof()) {
      char buffer[1024];
      user>>buffer;
      QString id="USER:";
      id+=QString::fromUtf8(buffer);
      item=new toResultViewItem(parent,item,QString::fromUtf8(buffer));
      item->setText(1,id);
      if (id==sel)
	item->setSelected(true);
    }
    parent=new toResultViewItem(UserList,parent,"Roles");
    parent->setOpen(true);
    parent->setSelectable(false);
    otl_stream roles(1,
		    SQLRoles(Connection),
		    Connection.connection());
    item=NULL;
    while(!roles.eof()) {
      char buffer[1024];
      roles>>buffer;
      QString id="ROLE:";
      id+=QString::fromUtf8(buffer);
      item=new toResultViewItem(parent,item,QString::fromUtf8(buffer));
      item->setText(1,id);
      if (id==sel)
	item->setSelected(true);
    }
  } TOCATCH
}
