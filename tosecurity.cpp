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

#include <stdio.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qradiobutton.h>
#include <qsplitter.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include "toconnection.h"
#include "tofilesize.h"
#include "tomain.h"
#include "toresultview.h"
#include "tosecurity.h"
#include "tosql.h"
#include "totool.h"

#include "tosecurity.moc"
#include "tosecurityquotaui.moc"
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
 WHERE UserName = :f1<char[100]>",
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

static toSQL SQLRoles("toSecurity:Roles",
		      "SELECT Role FROM Dba_Roles ORDER BY Role",
		      "Get roles available in DB, should return one entry");

static toSQL SQLListSystem("toSecurity:ListSystemPrivs",
			   "SELECT a.name
  FROM system_privilege_map a,
       v$enabledprivs b
 WHERE b.priv_number = a.privilege
 ORDER BY a.name",
			   "Get name of available system privileges");

static toSQL SQLQuota("toSecurity:Quota",
		      "SELECT Tablespace_name,
       Bytes,
       Max_bytes
  FROM DBA_TS_Quotas
 WHERE Username = :f1<char[200]>
 ORDER BY Tablespace_name",
		      "Get information about what quotas the user has, "
		      "must have same columns and same binds.");

static toSQL SQLSystemGrant("toSecurity:SystemGrant",
			    "SELECT privilege, NVL(admin_option,'NO') FROM dba_sys_privs WHERE grantee = :f1<char[100]>",
			    "Get information about the system privileges a user has, should have same bindings and columns");

static toSQL SQLObjectList("toSecurity:ObjectList",
			   "SELECT object_type,
       owner,
       object_name
  FROM all_objects
 WHERE object_type IN ('FUNCTION','LIBRARY','PACKAGE','PROCEDURE','SEQUENCE',
		       'TABLE','TYPE','VIEW','OPERATOR','DIRECTORY')
 ORDER BY owner,object_type,object_name",
			   "List objects available to set privileges on, must have same columns");

static toSQL SQLObjectPrivs("toSecurity:ObjectPrivs",
			    "SELECT DECODE(:type<char[100]>,'FUNCTION','EXECUTE',
			       'LIBRARY','EXECUTE',
			       'PACKAGE','EXECUTE',
			       'PROCEDURE','EXECUTE',
			       'SEQUENCE','ALTER,SELECT',
			       'TABLE','ALTER,DELETE,INDEX,INSERT,REFERENCES,SELECT,UPDATE',
			       'TYPE','EXECUTE',
			       'VIEW','DELETE,SELECT,INSERT,UPDATE',
			       'OPERATOR','EXECUTE',
			       'DIRECTORY','READ',
			       NULL) FROM DUAL",
			    "Takes a type as parameter and return ',' separated list of privileges");

static toSQL SQLObjectGrant("toSecurity:ObjectGrant",
			    "SELECT owner,
       table_name,
       privilege,
       grantable
  FROM dba_tab_privs
 WHERE grantee = :f1<char[100]>",
			    "Get the privilege on objects for a user or role, "
			    "must have same columns and binds");

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

class toSecurityQuota : public toSecurityQuotaUI {
  toConnection &Connection;
  QListViewItem *CurrentItem;
  void clearItem(QListViewItem *item);
  virtual void changeTablespace(void);
  virtual void changeSize(void);
public:
  toSecurityQuota(toConnection &conn,QWidget *parent);
  void changeUser(const QString &);
  QString sql(void);
};

void toSecurityQuota::changeSize(void)
{
  if (CurrentItem) {
    if (Value->isChecked()) {
      QString siz=QString::number(Size->value());
      siz+=" KB";
      CurrentItem->setText(1,siz);
    } else if (None->isChecked()) {
      CurrentItem->setText(1,"None");
    } else if (Unlimited->isChecked()) {
      CurrentItem->setText(1,"Unlimited");
    }
  } else
    SizeGroup->setEnabled(false);
}

toSecurityQuota::toSecurityQuota(toConnection &conn,QWidget *parent)
  : toSecurityQuotaUI(parent),Connection(conn)
{
  CurrentItem=NULL;
  try {
    otl_stream tablespaces(1,
			   SQLTablespace(Connection),
			   Connection.connection());
    QListViewItem *item=NULL;
    while(!tablespaces.eof()) {
      char buf[100];
      tablespaces>>buf;
      item=new QListViewItem(Tablespaces,item,QString::fromUtf8(buf),"None",QString::null,"None");
    }
  } TOCATCH
}

void toSecurityQuota::clearItem(QListViewItem *item)
{
  item->setText(1,"None");
  item->setText(2,QString::null);
  item->setText(3,"None");
}

void toSecurityQuota::changeUser(const QString &user)
{
  Tablespaces->show();
  SizeGroup->show();
  Disabled->hide(); // Do we really have to bother about this?

  Tablespaces->clearSelection();
  QListViewItem *item=Tablespaces->firstChild();
  if (!user.isEmpty()) {
    try {
      otl_stream quota(1,
		       SQLQuota(Connection),
		       Connection.connection());
      quota<<user.utf8();
      while(!quota.eof()) {
	char buffer[200];
	double maxQuota;
	double usedQuota;
	quota>>buffer>>usedQuota>>maxQuota;
	QString tbl(QString::fromUtf8(buffer));
	while(item&&item->text(0)!=tbl) {
	  clearItem(item);
	  item=item->nextSibling();
	}
	if (item) {
	  QString usedStr=QString::number(usedQuota/1024);
	  usedStr+=" KB";
	  QString maxStr;
	  if (maxQuota<0)
	    maxStr="Unlimited";
	  else if (maxQuota==0)
	    maxStr="None";
	  else {
	    maxStr=QString::number(maxQuota/1024);
	    maxStr+=" KB";
	  }
	  item->setText(1,maxStr);
	  item->setText(2,usedStr);
	  item->setText(3,maxStr);
	  item=item->nextSibling();
	}
      }
    } TOCATCH
  }
  while(item) {
    clearItem(item);
    item=item->nextSibling();
  }
  SizeGroup->setEnabled(false);
  CurrentItem=NULL;
}

void toSecurityQuota::changeTablespace(void)
{
  CurrentItem=Tablespaces->selectedItem();
  if (CurrentItem) {
    QString siz=CurrentItem->text(1);
    if (siz=="None")
      None->setChecked(true);
    else if (siz=="Unlimited")
      Unlimited->setChecked(true);
    else {
      Value->setChecked(true);
      Size->setValue(siz.toInt());
    }
  }
  SizeGroup->setEnabled(true);
}

QString toSecurityQuota::sql(void)
{
  QString ret;
  for (QListViewItem *item=Tablespaces->firstChild();item;item=item->nextSibling()) {
    if (item->text(1)!=item->text(3)) {
      QString siz=item->text(1);
      if (siz.right(2)=="KB")
	siz.truncate(siz.length()-1);
      else if (siz=="None")
	siz="0 K";
      else if (siz=="Unlimited")
	siz="UNLIMITED";
      ret+=" QUOTA ";
      ret+=siz;
      ret+=" ON ";
      ret+=item->text(0);
    }
  }
  return ret;
}

class toSecurityUser : public toSecurityUserUI {
  toConnection &Connection;

  toSecurityQuota *Quota;
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
  toSecurityUser(toSecurityQuota *quota,toConnection &conn,QWidget *parent);
  void clear(void);
  void changeUser(const QString &);
  QString name(void)
  { return Name->text(); }
  QString sql(void);
};

QString toSecurityUser::sql(void)
{
  QString extra;
  if (Authentication->currentPage()==PasswordTab) {
    if (Password->text()!=Password2->text()) {
      switch(QMessageBox::warning(this,
				  "Passwords don't match",
				  "The two versions of the password doesn't match",
				  "Don't save","Cancel")) {
      case 0:
	return QString::null;
      case 1:
	throw QString("Passwords don't match");
      }
    }
    if (Password->text()!=OrgPassword) {
      extra=" IDENTIFIED BY \"";
      extra+=Password->text();
      extra+="\"";
    }
    if (OrgExpired!=ExpirePassword->isChecked()) {
      if (ExpirePassword->isChecked())
	extra+=" PASSWORD EXPIRE";
    }
  } else if (Authentication->currentPage()==GlobalTab) {
    if (OrgGlobal!=GlobalName->text()) {
      extra=" IDENTIFIED GLOBALLY AS \"";
      extra+=GlobalName->text();
      extra+="\"";
    }
  } else if ((AuthType!=external)&&(Authentication->currentPage()==ExternalTab))
    extra=" IDENTIFIED EXTERNALLY";

  if (OrgProfile!=Profile->currentText()) {
    extra+=" PROFILE \"";
    extra+=Profile->currentText();
    extra+="\"";
  }
  if (OrgDefault!=DefaultSpace->currentText()) {
    extra+=" DEFAULT TABLESPACE \"";
    extra+=DefaultSpace->currentText();
    extra+="\"";
  }
  if (OrgTemp!=TempSpace->currentText()) {
    extra+=" TEMPORARY TABLESPACE \"";
    extra+=TempSpace->currentText();
    extra+="\"";
  }
  if (OrgLocked!=Locked->isChecked()) {
    extra+=" ACCOUNT ";
    if (Locked->isChecked())
      extra+="LOCK";
    else
      extra+="UNLOCK";
  }
  extra+=Quota->sql();

  QString sql;
  if (Name->isEnabled()) {
    if (Name->text().isEmpty())
      return QString::null;
    sql="CREATE ";
  } else {
    if (extra.isEmpty())
      return QString::null;
    sql="ALTER ";
  }
  sql+="USER \"";
  sql+=Name->text();
  sql+="\"";
  sql+=extra;
  return sql;
}

toSecurityUser::toSecurityUser(toSecurityQuota *quota,toConnection &conn,QWidget *parent)
  : toSecurityUserUI(parent),Connection(conn),Quota(quota)
{
  try {
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
  } TOCATCH
}

void toSecurityUser::clear(void)
{
  Name->setText("");
  Profile->setCurrentItem(0);
  Authentication->showPage(PasswordTab);
  Password->setText("");
  Password2->setText("");
  ExpirePassword->setChecked(false);
  ExpirePassword->setEnabled(true);
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
  try {
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
	ExpirePassword->setEnabled(false);
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
  } TOCATCH
}

class toSecurityRole : public toSecurityRoleUI {
  toConnection &Connection;
  toSecurityQuota *Quota;
  enum {
    password,
    global,
    external,
    none
  } AuthType;
public:
  toSecurityRole(toSecurityQuota *quota,toConnection &conn,QWidget *parent)
    : toSecurityRoleUI(parent),Connection(conn),Quota(quota)
  { }
  void changeRole(const QString &);
  QString sql(void);
  QString name(void)
  { return Name->text(); }
};

QString toSecurityRole::sql(void)
{
  QString extra;
  if (Authentication->currentPage()==PasswordTab) {
    if (Password->text()!=Password2->text()) {
      switch(QMessageBox::warning(this,
				  "Passwords don't match",
				  "The two versions of the password doesn't match",
				  "Don't save","Cancel")) {
      case 0:
	return QString::null;
      case 1:
	throw QString("Passwords don't match");
      }
    }
    if (Password->text().length()<=30) {
      extra=" IDENTIFIED BY \"";
      extra+=Password->text();
      extra+="\"";
    }
  } else if ((AuthType!=global)&&(Authentication->currentPage()==GlobalTab))
    extra=" IDENTIFIED GLOBALLY";
  else if ((AuthType!=external)&&(Authentication->currentPage()==ExternalTab))
    extra=" IDENTIFIED EXTERNALLY";
  else if ((AuthType!=none)&&(Authentication->currentPage()==NoneTab))
    extra=" NOT IDENTIFIED";
  extra+=Quota->sql();
  QString sql;
  if (Name->isEnabled()) {
    if (Name->text().isEmpty())
      return QString::null;
    sql="CREATE ";
  } else {
    if (extra.isEmpty())
      return QString::null;
    sql="ALTER ";
  }
  sql+="ROLE \"";
  sql+=Name->text();
  sql+="\"";
  sql+=extra;
  return sql;
}

void toSecurityRole::changeRole(const QString &role)
{
  try {
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
  } TOCATCH
}

class toSecurityPage : public QVBox {
  toSecurityRole *Role;
  toSecurityUser *User;
public:
  toSecurityPage(toSecurityQuota *quota,toConnection &conn,QWidget *parent)
    : QVBox(parent)
  {
    Role=new toSecurityRole(quota,conn,this);
    Role->hide();
    User=new toSecurityUser(quota,conn,this);
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
  QString name(void)
  {
    if (User->isHidden())
      return Role->name();
    else
      return User->name();
  }

  QString sql(void)
  {
    if (User->isHidden())
      return Role->sql();
    else
      return User->sql();
  }
};

toSecurityObject::toSecurityObject(toConnection &conn,QWidget *parent)
  : QListView(parent),Connection(conn)
{
  addColumn("Object");
  setRootIsDecorated(true);
  try {
    otl_stream object(1,
		      SQLObjectList(Connection),
		      Connection.connection());
    otl_stream typelst(1,
		       SQLObjectPrivs(Connection),
		       Connection.connection());
    QString oType;
    QString oOwner;
    QString oName;
    QListViewItem *typeItem=NULL;
    QListViewItem *ownerItem=NULL;
    QListViewItem *nameItem=NULL;
    QStringList Options;
    while(!object.eof()) {
      char buffer[100];
      object>>buffer;
      QString type(QString::fromUtf8(buffer));
      object>>buffer;
      QString owner(QString::fromUtf8(buffer));
      object>>buffer;
      QString name(QString::fromUtf8(buffer));
      if (owner!=oOwner) {
	oType=oName=QString::null;
	typeItem=nameItem=NULL;
	oOwner=owner;
	ownerItem=new QListViewItem(this,ownerItem,owner);
      }
      if (type!=oType) {
	oName=QString::null;
	nameItem=NULL;
	oType=type;
	typeItem=new QListViewItem(ownerItem,typeItem,type);
	typelst<<type.utf8();
	typelst>>buffer;
	Options=QStringList::split(",",QString::fromUtf8(buffer));
      }
      nameItem=new QListViewItem(typeItem,nameItem,name);
      for (QStringList::Iterator i=Options.begin();i!=Options.end();i++) {
	QListViewItem *item=new QCheckListItem(nameItem,*i,QCheckListItem::CheckBox);
	item->setText(2,name);
	item->setText(3,owner);
	new QCheckListItem(item,"Admin",QCheckListItem::CheckBox);
      }
    }
  } TOCATCH
  connect(this,SIGNAL(clicked(QListViewItem *)),this,SLOT(changed(QListViewItem *)));
}

void toSecurityObject::eraseUser()
{
  QListViewItem *next=NULL;
  for (QListViewItem *item=firstChild();item;item=next) {
    QCheckListItem *chk=dynamic_cast<QCheckListItem *>(item);
    if (chk) {
      chk->setOn(false);
      chk->setText(1,QString::null);
    }
    item->setOpen(false);
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
    }
  }
}

void toSecurityObject::changeUser(const QString &user)
{
  bool open=true;
  eraseUser();
  try {
    otl_stream grant(1,
		     SQLObjectGrant(Connection),
		     Connection.connection());
    grant<<user.utf8();
    while(!grant.eof()) {
      char buffer[200];
      grant>>buffer;
      QString owner(QString::fromUtf8(buffer));
      grant>>buffer;
      QString object(QString::fromUtf8(buffer));
      grant>>buffer;
      QString priv(QString::fromUtf8(buffer));
      grant>>buffer;
      QString admin(QString::fromUtf8(buffer));

      for (QListViewItem *ownerItem=firstChild();ownerItem;ownerItem=ownerItem->nextSibling()) {
	if (ownerItem->text(0)==owner) {
	  QListViewItem *next=NULL;
	  for (QListViewItem *item=ownerItem->firstChild();item&&item!=ownerItem;item=next) {
	    QCheckListItem *chk=dynamic_cast<QCheckListItem *>(item);
	    if (chk) {
	      if (chk->text(2)==object&&
		  chk->text(0)==priv) {
		chk->setText(1,"ON");
		chk->setOn(true);
		if (admin=="YES") {
		  QCheckListItem *chld=dynamic_cast<QCheckListItem *>(item->firstChild());
		  if (chld) {
		    chld->setText(1,"ON");
		    chld->setOn(true);
		    if (open)
		      chk->setOpen(true);
		  }
		}
		if (open)
		  for (QListViewItem *par=chk->parent();par;par=par->parent())
		    par->setOpen(true);
		break;
	      }
	    }
	    if (!chk&&item->firstChild())
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
	    }
	  }
	  break;
	}
      }
    }
  } TOCATCH
}

void toSecurityObject::sql(const QString &user,list<QString> &sqlLst)
{
  QListViewItem *next=NULL;
  for (QListViewItem *item=firstChild();item;item=next) {
    QCheckListItem *check=dynamic_cast<QCheckListItem *>(item);
    QCheckListItem *chld=dynamic_cast<QCheckListItem *>(item->firstChild());
    if (check) {
      QString sql;
      QString what=item->text(0);
      what+=" ON \"";
      what+=item->text(3);
      what+="\".\"";
      what+=item->text(2);
      what+="\" ";
      if (chld&&chld->isOn()&&chld->text(1).isEmpty()) {
	sql="GRANT ";
	sql+=what;
	sql+="TO \"";
	sql+=user;
	sql+="\" WITH GRANT OPTION";
	sqlLst.insert(sqlLst.end(),sql);
      } else if (check->isOn()&&!item->text(1).isEmpty()) {
	if (chld&&!chld->isOn()&&!chld->text(1).isEmpty()) {
	  sql="REVOKE ";
	  sql+=what;
	  sql+="FROM \"";
	  sql+=user;
	  sql+="\"";
	  sqlLst.insert(sqlLst.end(),sql);	

	  sql="GRANT ";
	  sql+=what;
	  sql+="TO \"";
	  sql+=user;
	  sql+="\"";
	  sqlLst.insert(sqlLst.end(),sql);
	}
      } else if (check->isOn()&&item->text(1).isEmpty()) {
	sql="GRANT ";
	sql+=what;
	sql+="TO \"";
	sql+=user;
	sql+="\"";
	sqlLst.insert(sqlLst.end(),sql);
      } else if (!check->isOn()&&!item->text(1).isEmpty()) {
	sql="REVOKE ";
	sql+=what;
	sql+="FROM \"";
	sql+=user;
	sql+="\"";
	sqlLst.insert(sqlLst.end(),sql);
      }
    }
    if (!check&&item->firstChild())
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
    }
  }
}

void toSecurityObject::changed(QListViewItem *org)
{
  QCheckListItem *item=dynamic_cast<QCheckListItem *>(org);
  if (item) {
    if (item->isOn()) {
      item=dynamic_cast<QCheckListItem *>(item->parent());
      if (item)
	item->setOn(true);
    } else {
      item=dynamic_cast<QCheckListItem *>(item->firstChild());
      if (item)
	item->setOn(false);
    }
  }
}

toSecuritySystem::toSecuritySystem(toConnection &conn,QWidget *parent)
  : QListView(parent),Connection(conn)
{
  addColumn("Privilege name");
  setRootIsDecorated(true);
  try {
    otl_stream priv(1,
		    SQLListSystem(Connection),
		    Connection.connection());
    char buffer[100];
    while(!priv.eof()) {
      priv>>buffer;
      QCheckListItem *item=new QCheckListItem(this,QString::fromUtf8(buffer),QCheckListItem::CheckBox);
      new QCheckListItem(item,"Admin",QCheckListItem::CheckBox);
    }
    setSorting(0);
  } TOCATCH
  connect(this,SIGNAL(clicked(QListViewItem *)),this,SLOT(changed(QListViewItem *)));
}

void toSecuritySystem::sql(const QString &user,list<QString> &sqlLst)
{
  for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
    QString sql;
    QCheckListItem *check=dynamic_cast<QCheckListItem *>(item);
    QCheckListItem *chld=dynamic_cast<QCheckListItem *>(item->firstChild());
    if (chld&&chld->isOn()&&chld->text(1).isEmpty()) {
      sql="GRANT ";
      sql+=item->text(0);
      sql+=" TO \"";
      sql+=user;
      sql+="\" WITH ADMIN OPTION";
      sqlLst.insert(sqlLst.end(),sql);
    } else if (check->isOn()&&!item->text(1).isEmpty()) {
      if (chld&&!chld->isOn()&&!chld->text(1).isEmpty()) {
	sql="REVOKE ";
	sql+=item->text(0);
	sql+=" FROM \"";
	sql+=user;
	sql+="\"";
	sqlLst.insert(sqlLst.end(),sql);	

	sql="GRANT ";
	sql+=item->text(0);
	sql+=" TO \"";
	sql+=user;
	sql+="\"";
	sqlLst.insert(sqlLst.end(),sql);
      }
    } else if (check->isOn()&&item->text(1).isEmpty()) {
      sql="GRANT ";
      sql+=item->text(0);
      sql+=" TO \"";
      sql+=user;
      sql+="\"";
      sqlLst.insert(sqlLst.end(),sql);
    } else if (!check->isOn()&&!item->text(1).isEmpty()) {
      sql="REVOKE ";
      sql+=item->text(0);
      sql+=" FROM \"";
      sql+=user;
      sql+="\"";
      sqlLst.insert(sqlLst.end(),sql);
    }
  }
}

void toSecuritySystem::changed(QListViewItem *org)
{
  QCheckListItem *item=dynamic_cast<QCheckListItem *>(org);
  if (item) {
    if (item->isOn()) {
      item=dynamic_cast<QCheckListItem *>(item->parent());
      if (item)
	item->setOn(true);
    } else {
      item=dynamic_cast<QCheckListItem *>(item->firstChild());
      if (item)
	item->setOn(false);
    }
  }
}

void toSecuritySystem::setOn(QListViewItem *org,bool val)
{
  QCheckListItem *item=dynamic_cast<QCheckListItem *>(org);
  if (item) {
    item->setOn(val);
    if (!val) {
      item=dynamic_cast<QCheckListItem *>(item->firstChild());
      if (item)
	item->setOn(false);
    }
  }
}

void toSecuritySystem::eraseUser()
{
  for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
    setOn(item,false);
    item->setText(1,QString::null);
    if (item->firstChild())
      item->firstChild()->setText(1,QString::null);
  }
}

void toSecuritySystem::changeUser(const QString &user)
{
  eraseUser();
  try {
    otl_stream query(1,
		     SQLSystemGrant(Connection),
		     Connection.connection());
    query<<user.utf8();
    while(!query.eof()) {
      char buffer[1024];
      query>>buffer;
      QString str=QString::fromUtf8(buffer);
      query>>buffer;
      QString admin=QString::fromUtf8(buffer);
      for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
	if (item->text(0)==str) {
	  setOn(item,true);
	  item->setText(1,"ON");
	  if (admin!="NO"&&item->firstChild()) {
	    setOn(item->firstChild(),true);
	    item->firstChild()->setText(1,"ON");
	  }
	  break;
	}
      }
    }
  } TOCATCH
}

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
		  this,SLOT(saveChanges(void)),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel("",toolbar));

  Connection.addWidget(this);

  QSplitter *splitter=new QSplitter(Horizontal,this);
  UserList=new toResultView(false,false,Connection,splitter);
  UserList->addColumn("Users/Roles");
  UserList->setSQLName("toSecurity:Users/Roles");
  UserList->setRootIsDecorated(true);
  Tabs=new QTabWidget(splitter);
  Quota=new toSecurityQuota(Connection,Tabs);
  General=new toSecurityPage(Quota,Connection,Tabs);
  Tabs->addTab(General,"General");
  SystemGrant=new toSecuritySystem(Connection,Tabs);
  Tabs->addTab(SystemGrant,"System Privileges");
  ObjectGrant=new toSecurityObject(Connection,Tabs);
  Tabs->addTab(ObjectGrant,"Object Privileges");
  Tabs->addTab(Quota,"Quota");
  connect(UserList,SIGNAL(selectionChanged()),this,SLOT(changeUser()));
  refresh();
}

toSecurity::~toSecurity()
{
  Connection.delWidget(this);
}

list<QString> toSecurity::sql(void)
{
  list<QString> ret;
  QString tmp=General->sql();
  if (!tmp.isEmpty())
    ret.insert(ret.end(),tmp);
  QString name=General->name();
  SystemGrant->sql(name,ret);
  ObjectGrant->sql(name,ret);
  for (list<QString>::iterator i=ret.begin();i!=ret.end();i++)
    printf("SQL:%s\n",(const char *)(*i));

  return ret;
}

void toSecurity::changeUser(void)
{
  try {
    list<QString> sqlList=sql();
    if (sqlList.size()!=0) {
      switch(QMessageBox::warning(this,
				  "Save changes?",
				  "Save the changes made to this user?",
				  "Save","Discard","Cancel")) {
      case 0:
	// saveShit...
	break;
      case 1:
	break;
      case 2:
	return;
      }
    }
  } catch (const QString &str) {
    toStatusMessage(str);
    return;
  }

  try {
    QString sel;
    QListViewItem *item=UserList->selectedItem();
    if (item) {
      UserID=item->text(1);
      if (UserID[4]!=':')
	throw QString("Invalid security ID");
      bool user=false;
      if (UserID.startsWith("USER"))
	user=true;
      QString username=UserID.right(UserID.length()-5);
      General->changePage(username,user);
      Quota->changeUser(username);
      SystemGrant->changeUser(username);
      ObjectGrant->changeUser(username);
    }
  } TOCATCH
}

void toSecurity::refresh(void)
{
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
      if (id==UserID)
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
      if (id==UserID)
	item->setSelected(true);
    }
  } TOCATCH
}

void toSecurity::saveChanges()
{
  sql();
}
