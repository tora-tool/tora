//***************************************************************************
/*
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

TO_NAMESPACE;

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
#include <qvalidator.h>

#include "tochangeconnection.h"
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
#include "icons/trash.xpm"
#include "icons/addrole.xpm"
#include "icons/adduser.xpm"
#include "icons/copyuser.xpm"

static toSQL SQLUserInfo("toSecurity:UserInfo",
			 "SELECT Account_Status,\n"
			 "       Password,\n"
			 "       External_Name,\n"
			 "       Profile,\n"
			 "       Default_Tablespace,\n"
			 "       Temporary_Tablespace\n"
			 "  FROM DBA_Users\n"
			 " WHERE UserName = :f1<char[100]>",
			 "Get information about a user, must have same columns and same binds.");

static toSQL SQLRoleInfo("toSecurity:RoleInfo",
			 "SELECT Role,Password_required FROM DBA_Roles WHERE Role = :f1<char[101]>",
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
			   "SELECT a.name\n"
			   "  FROM system_privilege_map a,\n"
			   "       v$enabledprivs b\n"
			   " WHERE b.priv_number = a.privilege\n"
			   " ORDER BY a.name",
			   "Get name of available system privileges");

static toSQL SQLQuota("toSecurity:Quota",
		      "SELECT Tablespace_name,\n"
		      "       Bytes,\n"
		      "       Max_bytes\n"
		      "  FROM DBA_TS_Quotas\n"
		      " WHERE Username = :f1<char[200]>\n"
		      " ORDER BY Tablespace_name",
		      "Get information about what quotas the user has, "
		      "must have same columns and same binds.");

static toSQL SQLSystemGrant("toSecurity:SystemGrant",
			    "SELECT privilege, NVL(admin_option,'NO') FROM dba_sys_privs WHERE grantee = :f1<char[100]>",
			    "Get information about the system privileges a user has, should have same bindings and columns");

static toSQL SQLObjectList("toSecurity:ObjectList",
			   "SELECT object_type,\n"
			   "       owner,\n"
			   "       object_name\n"
			   "  FROM all_objects\n"
			   " WHERE object_type IN ('FUNCTION','LIBRARY','PACKAGE','PROCEDURE','SEQUENCE',\n"
			   "		       'TABLE','TYPE','VIEW','OPERATOR','DIRECTORY')\n"
			   " ORDER BY owner,object_type,object_name",
			   "List objects available to set privileges on, must have same columns");

static toSQL SQLObjectPrivs("toSecurity:ObjectPrivs",
			    "SELECT DECODE(:type<char[100]>,'FUNCTION','EXECUTE',\n"
			    "			       'LIBRARY','EXECUTE',\n"
			    "			       'PACKAGE','EXECUTE',\n"
			    "			       'PROCEDURE','EXECUTE',\n"
			    "			       'SEQUENCE','ALTER,SELECT',\n"
			    "			       'TABLE','ALTER,DELETE,INDEX,INSERT,REFERENCES,SELECT,UPDATE',\n"
			    "			       'TYPE','EXECUTE',\n"
			    "			       'VIEW','DELETE,SELECT,INSERT,UPDATE',\n"
			    "			       'OPERATOR','EXECUTE',\n"
			    "			       'DIRECTORY','READ',\n"
			    "			       NULL) FROM DUAL",
			    "Takes a type as parameter and return ',' separated list of privileges");

static toSQL SQLObjectGrant("toSecurity:ObjectGrant",
			    "SELECT owner,\n"
			    "       table_name,\n"
			    "       privilege,\n"
			    "       grantable\n"
			    "  FROM dba_tab_privs\n"
			    " WHERE grantee = :f1<char[100]>",
			    "Get the privilege on objects for a user or role, "
			    "must have same columns and binds");

static toSQL SQLRoleGrant("toSecurity:RoleGrant",
			  "SELECT granted_role,\n"
			  "       admin_option,\n"
			  "       default_role\n"
			  "  FROM dba_role_privs\n"
			  " WHERE grantee = :f1<char[100]>",
			  "Get the roles granted to a user or role, "
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
static QPixmap *toTrashPixmap;
static QPixmap *toAddRolePixmap;
static QPixmap *toAddUserPixmap;
static QPixmap *toCopyUserPixmap;

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
  void clear(void);
};

void toSecurityQuota::changeSize(void)
{
  if (CurrentItem) {
    if (Value->isChecked()) {
      QString siz;
      siz.sprintf("%.0f KB",double(Size->value()));
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
    toQuery tablespaces(Connection,SQLTablespace);
    QListViewItem *item=NULL;
    while(!tablespaces.eof()) {
      item=new toResultViewItem(Tablespaces,item,tablespaces.readValue());
      item->setText(1,"None");
      item->setText(3,"None");
    }
  } TOCATCH
}

void toSecurityQuota::clearItem(QListViewItem *item)
{
  item->setText(1,"None");
  item->setText(2,QString::null);
  item->setText(3,"None");
}

void toSecurityQuota::clear(void)
{
  for (QListViewItem *item=Tablespaces->firstChild();item;item=item->nextSibling())
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
      toQuery quota(Connection,SQLQuota,user);
      while(!quota.eof()) {
	double maxQuota;
	double usedQuota;
	QString tbl(quota.readValue());
	while(item&&item->text(0)!=tbl) {
	  clearItem(item);
	  item=item->nextSibling();
	}
	usedQuota=quota.readValue().toDouble();
	maxQuota=quota.readValue().toDouble();
	if (item) {
	  QString usedStr;
	  QString maxStr;
	  usedStr.sprintf("%.0f KB",usedQuota/1024);
	  if (maxQuota<0)
	    maxStr="Unlimited";
	  else if (maxQuota==0)
	    maxStr="None";
	  else {
	    maxStr.sprintf("%.0f KB",maxQuota/1024);
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

class toSecurityUpper : public QValidator {
public:
  toSecurityUpper(QWidget *parent)
    : QValidator(parent)
  { }
  virtual State validate(QString &str,int &) const
  { str=str.upper(); return Acceptable; }
};

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
  void clear(bool all=true);
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
      switch(TOMessageBox::warning(this,
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
      extra=" IDENTIFIED GLOBALLY AS '";
      extra+=GlobalName->text();
      extra+="'";
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
  Name->setValidator(new toSecurityUpper(Name));
  try {
    toQuery profiles(Connection,SQLProfiles);
    while(!profiles.eof())
      Profile->insertItem(profiles.readValue());

    toQuery tablespaces(Connection,
			SQLTablespace);
    while(!tablespaces.eof()) {
      QString buf=tablespaces.readValue();
      DefaultSpace->insertItem(buf);
      TempSpace->insertItem(buf);
    }
  } TOCATCH
}

void toSecurityUser::clear(bool all)
{
  Name->setText("");
  Password->setText("");
  Password2->setText("");
  GlobalName->setText("");
  if (all) {
    Profile->setCurrentItem(0);
    Authentication->showPage(PasswordTab);
    ExpirePassword->setChecked(false);
    ExpirePassword->setEnabled(true);
    TempSpace->setCurrentItem(0);
    DefaultSpace->setCurrentItem(0);
    Locked->setChecked(false);
  }

  OrgProfile=OrgDefault=OrgTemp=OrgGlobal="";
  AuthType=password;
  Name->setEnabled(true);
  OrgLocked=OrgExpired=false;
}

void toSecurityUser::changeUser(const QString &user)
{
  clear();
  try {
    toQuery query(Connection,SQLUserInfo,user);
    if (!query.eof()) {
      Name->setEnabled(false);
      Name->setText(user);

      QString str(query.readValue());
      if (str.startsWith("EXPIRED")) {
	ExpirePassword->setChecked(true);
	ExpirePassword->setEnabled(false);
	OrgExpired=true;
      } else if (str.startsWith("LOCKED")) {
	Locked->setChecked(true);
	OrgLocked=true;
      }

      OrgPassword=query.readValue();
      QString pass=query.readValue();
      if (OrgPassword=="GLOBAL") {
	OrgPassword=QString::null;
	Authentication->showPage(GlobalTab);
	OrgGlobal=pass;
	GlobalName->setText(OrgGlobal);
	AuthType=global;
      } else if (OrgPassword=="EXTERNAL") {
	OrgPassword=QString::null;
	Authentication->showPage(ExternalTab);
	AuthType=external;
      } else {
	Password->setText(OrgPassword);
	Password2->setText(OrgPassword);
	AuthType=password;
      }

      {
        str=query.readValue();
        for (int i=0;i<Profile->count();i++) {
  	  if (Profile->text(i)==str) {
	    Profile->setCurrentItem(i);
	    OrgProfile=str;
	    break;
	  }
	}
      }

      {
	str=query.readValue();
	for (int i=0;i<DefaultSpace->count();i++) {
	  if (DefaultSpace->text(i)==str) {
	    DefaultSpace->setCurrentItem(i);
	    OrgDefault=str;
	    break;
	  }
	}
      }

      {
	str=query.readValue();
	for (int i=0;i<TempSpace->count();i++) {
	  if (TempSpace->text(i)==str) {
	    TempSpace->setCurrentItem(i);
	    OrgTemp=str;
	    break;
	  }
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
  { Name->setValidator(new toSecurityUpper(Name)); }
  void clear(void);
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
      switch(TOMessageBox::warning(this,
				   "Passwords don't match",
				   "The two versions of the password doesn't match",
				   "Don't save","Cancel")) {
      case 0:
	return QString::null;
      case 1:
	throw QString("Passwords don't match");
      }
    }
    if (Password->text().length()>0) {
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

void toSecurityRole::clear(void)
{
  Name->setText("");
  Name->setEnabled(true);
}

void toSecurityRole::changeRole(const QString &role)
{
  try {
    toQuery query(Connection,SQLRoleInfo,role);
    Password->setText("");
    Password2->setText("");
    if (!query.eof()) {
      Name->setText(role);
      Name->setEnabled(false);

      QString str(query.readValue());
      if (str=="YES") {
	AuthType=password;
	Authentication->showPage(PasswordTab);
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
      Name->setText("");
      Name->setEnabled(true);
      AuthType=none;
      Authentication->showPage(NoneTab);
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
  void clear(void)
  {
    if (User->isHidden())
      Role->clear();
    else
      User->clear(false);
  }
  bool user(void)
  {
    if (User->isHidden())
      return false;
    return true;
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
    QString oType;
    QString oOwner;
    QString oName;
    toQuery object(Connection,SQLObjectList);
    toQuery typelst(Connection);
    QListViewItem *typeItem=NULL;
    QListViewItem *ownerItem=NULL;
    QListViewItem *nameItem=NULL;
    QStringList Options;
    while(!object.eof()) {
      QString type(object.readValue());
      QString owner(object.readValue());
      QString name(object.readValue());
      if (owner!=oOwner) {
	oType=oName=QString::null;
	typeItem=nameItem=NULL;
	oOwner=owner;
	ownerItem=new toResultViewItem(this,ownerItem,owner);
      }
      if (type!=oType) {
	oName=QString::null;
	nameItem=NULL;
	oType=type;
	typeItem=new toResultViewItem(ownerItem,typeItem,type);
	toQList args;
	toPush(args,toQValue(type));
	typelst.execute(SQLObjectPrivs,args);
	Options=QStringList::split(",",typelst.readValue());
      }
      nameItem=new toResultViewItem(typeItem,nameItem,name);
      for (QStringList::Iterator i=Options.begin();i!=Options.end();i++) {
	QListViewItem *item=new toResultViewCheck(nameItem,*i,QCheckListItem::CheckBox);
	item->setText(2,name);
	item->setText(3,owner);
	new toResultViewCheck(item,"Admin",QCheckListItem::CheckBox);
      }
    }
  } TOCATCH
  connect(this,SIGNAL(clicked(QListViewItem *)),this,SLOT(changed(QListViewItem *)));
}

void toSecurityObject::eraseUser(bool all)
{
  QListViewItem *next=NULL;
  for (QListViewItem *item=firstChild();item;item=next) {
    toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(item);
    if (chk) {
      if (all)
	chk->setOn(false);
      chk->setText(1,QString::null);
    }
    if (all)
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
    toQuery grant(Connection,SQLObjectGrant,user);
    while(!grant.eof()) {
      QString owner(grant.readValue());
      QString object(grant.readValue());
      QString priv(grant.readValue());
      QString admin(grant.readValue());

      for (QListViewItem *ownerItem=firstChild();ownerItem;ownerItem=ownerItem->nextSibling()) {
	if (ownerItem->text(0)==owner) {
	  QListViewItem *next=NULL;
	  for (QListViewItem *item=ownerItem->firstChild();item&&item!=ownerItem;item=next) {
	    toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(item);
	    if (chk) {
	      if (chk->text(2)==object&&
		  chk->text(0)==priv) {
		chk->setText(1,"ON");
		chk->setOn(true);
		if (admin=="YES") {
		  toResultViewCheck *chld=dynamic_cast<toResultViewCheck *>(item->firstChild());
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
    toResultViewCheck *check=dynamic_cast<toResultViewCheck *>(item);
    toResultViewCheck *chld=dynamic_cast<toResultViewCheck *>(item->firstChild());
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
  toResultViewCheck *item=dynamic_cast<toResultViewCheck *>(org);
  if (item) {
    if (item->isOn()) {
      item=dynamic_cast<toResultViewCheck *>(item->parent());
      if (item)
	item->setOn(true);
    } else {
      item=dynamic_cast<toResultViewCheck *>(item->firstChild());
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
    toQuery priv(Connection,SQLListSystem);
    while(!priv.eof()) {
      toResultViewCheck *item=new toResultViewCheck(this,priv.readValue(),
						    QCheckListItem::CheckBox);
      new toResultViewCheck(item,"Admin",QCheckListItem::CheckBox);
    }
    setSorting(0);
  } TOCATCH
  connect(this,SIGNAL(clicked(QListViewItem *)),this,SLOT(changed(QListViewItem *)));
}

void toSecuritySystem::sql(const QString &user,list<QString> &sqlLst)
{
  for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
    QString sql;
    toResultViewCheck *check=dynamic_cast<toResultViewCheck *>(item);
    toResultViewCheck *chld=dynamic_cast<toResultViewCheck *>(item->firstChild());
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
  toResultViewCheck *item=dynamic_cast<toResultViewCheck *>(org);
  if (item) {
    if (item->isOn()) {
      item=dynamic_cast<toResultViewCheck *>(item->parent());
      if (item)
	item->setOn(true);
    } else {
      item=dynamic_cast<toResultViewCheck *>(item->firstChild());
      if (item)
	item->setOn(false);
    }
  }
}

void toSecuritySystem::eraseUser(bool all)
{
  for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
    toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(item);
    if (chk&&all)
      chk->setOn(false);
    item->setText(1,QString::null);
    for (QListViewItem *chld=item->firstChild();chld;chld=chld->nextSibling()) {
      chld->setText(1,QString::null);
      toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(chld);
      if (chk&&all)
	chk->setOn(false);
    }
  }
}

void toSecuritySystem::changeUser(const QString &user)
{
  eraseUser();
  try {
    toQuery query(Connection,SQLSystemGrant,user);
    while(!query.eof()) {
      QString str=query.readValue();
      QString admin=query.readValue();
      for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
	if (item->text(0)==str) {
	  toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(item);
	  if (chk)
	    chk->setOn(true);
	  item->setText(1,"ON");
	  if (admin!="NO"&&item->firstChild()) {
	    chk=dynamic_cast<toResultViewCheck *>(item->firstChild());
	    if (chk)
	      chk->setOn(true);
	    if (chk->parent())
	      chk->parent()->setOpen(true);
	    item->firstChild()->setText(1,"ON");
	  }
	  break;
	}
      }
    }
  } TOCATCH
}

toSecurityRoleGrant::toSecurityRoleGrant(toConnection &conn,QWidget *parent)
  : QListView(parent),Connection(conn)
{
  addColumn("Role name");
  setRootIsDecorated(true);
  try {
    toQuery priv(Connection,SQLRoles);
    while(!priv.eof()) {
      toResultViewCheck *item=new toResultViewCheck(this,priv.readValue(),QCheckListItem::CheckBox);
      new toResultViewCheck(item,"Admin",QCheckListItem::CheckBox);
      new toResultViewCheck(item,"Default",QCheckListItem::CheckBox);
    }
    setSorting(0);
  } TOCATCH
  connect(this,SIGNAL(clicked(QListViewItem *)),this,SLOT(changed(QListViewItem *)));
}

QCheckListItem *toSecurityRoleGrant::findChild(QListViewItem *parent,const QString &name)
{
  for (QListViewItem *item=parent->firstChild();item;item=item->nextSibling()) {
    if (item->text(0)==name) {
      toResultViewCheck *ret=dynamic_cast<toResultViewCheck *>(item);
      if (ret->isEnabled())
	return ret;
      else
	return NULL;
    }
  }
  return NULL;
}

void toSecurityRoleGrant::sql(const QString &user,list<QString> &sqlLst)
{
  bool any=false;
  bool chg=false;
  QString except;
  QString sql;
  for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
    toResultViewCheck *check=dynamic_cast<toResultViewCheck *>(item);
    QCheckListItem *chld=findChild(item,"Admin");
    QCheckListItem *def=findChild(item,"Default");
    if (def&&check) {
      if (!def->isOn()&&check->isOn()) {
	if (!except.isEmpty())
	  except+=",\"";
	else
	  except+=" EXCEPT \"";
	except+=item->text(0);
	except+="\"";
      } else if (check->isOn()&&def->isOn())
	any=true;
      if (def->isOn()==def->text(1).isEmpty())
	chg=true;
    }
    if (chld&&chld->isOn()&&chld->text(1).isEmpty()) {
      if (check->isOn()&&!item->text(1).isEmpty()) {
	sql="REVOKE \"";
	sql+=item->text(0);
	sql+="\" FROM \"";
	sql+=user;
	sql+="\"";
	sqlLst.insert(sqlLst.end(),sql);
      }
      sql="GRANT \"";
      sql+=item->text(0);
      sql+="\" TO \"";
      sql+=user;
      sql+="\" WITH ADMIN OPTION";
      sqlLst.insert(sqlLst.end(),sql);
      chg=true;
    } else if (check->isOn()&&!item->text(1).isEmpty()) {
      if (chld&&!chld->isOn()&&!chld->text(1).isEmpty()) {
	sql="REVOKE \"";
	sql+=item->text(0);
	sql+="\" FROM \"";
	sql+=user;
	sql+="\"";
	sqlLst.insert(sqlLst.end(),sql);	

	sql="GRANT \"";
	sql+=item->text(0);
	sql+="\" TO \"";
	sql+=user;
	sql+="\"";
	sqlLst.insert(sqlLst.end(),sql);
	chg=true;
      }
    } else if (check->isOn()&&item->text(1).isEmpty()) {
      sql="GRANT \"";
      sql+=item->text(0);
      sql+="\" TO \"";
      sql+=user;
      sql+="\"";
      sqlLst.insert(sqlLst.end(),sql);
      chg=true;
    } else if (!check->isOn()&&!item->text(1).isEmpty()) {
      sql="REVOKE \"";
      sql+=item->text(0);
      sql+="\" FROM \"";
      sql+=user;
      sql+="\"";
      sqlLst.insert(sqlLst.end(),sql);
      chg=true;
    }
  }
  if (chg) {
    sql="ALTER USER \"";
    sql+=user;
    sql+="\" DEFAULT ROLE ";
    if (any) {
      sql+="ALL";
      sql+=except;
    } else
      sql+="NONE";
    sqlLst.insert(sqlLst.end(),sql);
  }
}

void toSecurityRoleGrant::changed(QListViewItem *org)
{
  toResultViewCheck *item=dynamic_cast<toResultViewCheck *>(org);
  if (item) {
    if (item->isOn()) {
      QCheckListItem *chld=findChild(item,"Default");
      if (chld)
	chld->setOn(true);
      item=dynamic_cast<toResultViewCheck *>(item->parent());
      if (item)
	item->setOn(true);
    } else {
      for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
	toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(item->firstChild());
	if (chk)
	  chk->setOn(false);
      }
    }
  }
}

void toSecurityRoleGrant::eraseUser(bool user,bool all)
{
  for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
    toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(item);
    if (chk&&all)
      chk->setOn(false);
    item->setText(1,QString::null);
    for (QListViewItem *chld=item->firstChild();chld;chld=chld->nextSibling()) {
      chld->setText(1,QString::null);
      toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(chld);
      if (chk) {
	if (all) {
	  chk->setOn(false);
	  if (chk->text(0)=="Default")
	    chk->setEnabled(user);
	}
      }
    }
  }
}

void toSecurityRoleGrant::changeUser(bool user,const QString &username)
{
  eraseUser(user);
  try {
    toQuery query(Connection,SQLRoleGrant,username);
    while(!query.eof()) {
      QString str=query.readValue();
      QString admin=query.readValue();
      QString def=query.readValue();
      for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
	if (item->text(0)==str) {
	  QCheckListItem *chk=dynamic_cast<toResultViewCheck *>(item);
	  if (chk)
	    chk->setOn(true);
	  item->setText(1,"ON");
	  chk=findChild(item,"Admin");
	  if (admin=="YES"&&chk) {
	    chk->setOn(true);
	    chk->setText(1,"ON");
	    if (chk->parent())
	      chk->parent()->setOpen(true);
	  }
	  chk=findChild(item,"Default");
	  if (def=="YES"&&chk) {
	    chk->setOn(true);
	    chk->setText(1,"ON");
	    if (chk->parent())
	      chk->parent()->setOpen(true);
	  }
	  break;
	}
      }
    }
  } TOCATCH
}

toSecurity::toSecurity(QWidget *main,toConnection &connection)
  : toToolWidget("security.html",main,connection)
{
  toBusy busy;
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);
  if (!toCommitPixmap)
    toCommitPixmap=new QPixmap((const char **)commit_xpm);
  if (!toTrashPixmap)
    toTrashPixmap=new QPixmap((const char **)trash_xpm);
  if (!toAddRolePixmap)
    toAddRolePixmap=new QPixmap((const char **)addrole_xpm);
  if (!toAddUserPixmap)
    toAddUserPixmap=new QPixmap((const char **)adduser_xpm);
  if (!toCopyUserPixmap)
    toCopyUserPixmap=new QPixmap((const char **)copyuser_xpm);

  QToolBar *toolbar=toAllocBar(this,"Security manager",connection.description());

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
  DropButton=new QToolButton(*toTrashPixmap,
			     "Remove user/role",
			     "Remove user/role",
			     this,SLOT(drop(void)),
			     toolbar);
  DropButton->setEnabled(false);
  toolbar->addSeparator();
  new QToolButton(*toAddUserPixmap,
		  "Add new user",
		  "Add new user",
		  this,SLOT(addUser(void)),
		  toolbar);
  new QToolButton(*toAddRolePixmap,
		  "Add new role",
		  "Add new role",
		  this,SLOT(addRole(void)),
		  toolbar);
  CopyButton=new QToolButton(*toCopyUserPixmap,
			     "Copy current user or role",
			     "Copy current user or role",
			     this,SLOT(copy(void)),
			     toolbar);
  CopyButton->setEnabled(false);
  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  QSplitter *splitter=new QSplitter(Horizontal,this);
  UserList=new toResultView(false,false,splitter);
  UserList->addColumn("Users/Roles");
  UserList->setSQLName("toSecurity:Users/Roles");
  UserList->setRootIsDecorated(true);
  Tabs=new QTabWidget(splitter);
  Quota=new toSecurityQuota(connection,Tabs);
  General=new toSecurityPage(Quota,connection,Tabs);
  Tabs->addTab(General,"General");
  RoleGrant=new toSecurityRoleGrant(connection,Tabs);
  Tabs->addTab(RoleGrant,"Roles");
  SystemGrant=new toSecuritySystem(connection,Tabs);
  Tabs->addTab(SystemGrant,"System Privileges");
  ObjectGrant=new toSecurityObject(connection,Tabs);
  Tabs->addTab(ObjectGrant,"Object Privileges");
  Tabs->addTab(Quota,"Quota");
  connect(UserList,SIGNAL(currentChanged(QListViewItem *)),
	  this,SLOT(changeUser(QListViewItem *)));
  refresh();
}

list<QString> toSecurity::sql(void)
{
  list<QString> ret;
  QString tmp=General->sql();
  if (!tmp.isEmpty())
    ret.insert(ret.end(),tmp);
  QString name=General->name();
  if (!name.isEmpty()) {
    SystemGrant->sql(name,ret);
    ObjectGrant->sql(name,ret);
    RoleGrant->sql(name,ret);
  }
#if 0
  for (list<QString>::iterator i=ret.begin();i!=ret.end();i++)
    printf("SQL:%s\n",(const char *)(*i));
#endif

  return ret;
}

void toSecurity::changeUser(bool ask)
{
  if (ask) {
    try {
      list<QString> sqlList=sql();
      if (sqlList.size()!=0) {
	switch(TOMessageBox::warning(this,
				     "Save changes?",
				     "Save the changes made to this user?",
				     "Save","Discard","Cancel")) {
	case 0:
	  saveChanges();
	  return;
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
  }

  try {
    QString sel;
    QListViewItem *item=UserList->currentItem();
    if (item) {
      UserID=item->text(1);
      DropButton->setEnabled(item->parent());
      CopyButton->setEnabled(item->parent());

      if (UserID[4]!=':')
	throw QString("Invalid security ID");
      bool user=false;
      if (UserID.startsWith("USER"))
	user=true;
      QString username=UserID.right(UserID.length()-5);
      General->changePage(username,user);
      Quota->changeUser(username);
      Tabs->setTabEnabled(Quota,user);
      RoleGrant->changeUser(user,username);
      SystemGrant->changeUser(username);
      ObjectGrant->changeUser(username);
    }
  } TOCATCH
}

void toSecurity::refresh(void)
{
  disconnect(UserList,SIGNAL(currentChanged(QListViewItem *)),
	     this,SLOT(changeUser(QListViewItem *)));
  UserList->clear();
  try {
    QListViewItem *parent=new toResultViewItem(UserList,NULL,"Users");
    parent->setText(1,"USER:");
    parent->setOpen(true);
    parent->setSelectable(false);
    toQuery user(connection(),toSQL::string(toSQL::TOSQL_USERLIST,connection()));
    QListViewItem *item=NULL;
    while(!user.eof()) {
      QString tmp=user.readValue();
      QString id="USER:";
      id+=tmp;
      item=new toResultViewItem(parent,item,tmp);
      item->setText(1,id);
      if (id==UserID)
	UserList->setSelected(item,true);
    }
    parent=new toResultViewItem(UserList,parent,"Roles");
    parent->setText(1,"ROLE:");
    parent->setOpen(true);
    parent->setSelectable(false);
    toQuery roles(connection(),SQLRoles);
    item=NULL;
    while(!roles.eof()) {
      QString tmp=roles.readValue();
      QString id="ROLE:";
      id+=tmp;
      item=new toResultViewItem(parent,item,tmp);
      item->setText(1,id);
      if (id==UserID)
	UserList->setSelected(item,true);
    }
  } TOCATCH
  connect(UserList,SIGNAL(currentChanged(QListViewItem *)),
	  this,SLOT(changeUser(QListViewItem *)));
}

void toSecurity::saveChanges()
{
  list<QString> sqlList=sql();
  for (list<QString>::iterator i=sqlList.begin();i!=sqlList.end();i++) {
    try {
      connection().execute(*i);
    } TOCATCH
  }
  if (General->user())
    UserID="USER:";
  else
    UserID="ROLE:";
  UserID+=General->name();
  refresh();
  changeUser(false);
}

void toSecurity::drop()
{
  if (UserID.length()>5) {
    QString str="DROP ";
    if (General->user())
      str+="USER";
    else
      str+="ROLE";
    str+=" \"";
    str+=UserID.right(UserID.length()-5);
    str+="\"";
    try {
      connection().execute(str);
      refresh();
      changeUser(false);
    } catch(...) {
      switch(TOMessageBox::warning(this,
				   "Are you sure?",
				   "The user still owns objects, add the cascade option?",
				   "Yes","No")) {
      case 0:
	str+=" CASCADE";
	try {
	  connection().execute(str);
	  refresh();
	  changeUser(false);
	} TOCATCH
	return;
      case 1:
	break;
      }
    }
  }
}

void toSecurity::addUser(void)
{
  for (QListViewItem *item=UserList->firstChild();item;item=item->nextSibling())
    if (item->text(1)=="USER:") {
      UserList->clearSelection();
      UserList->setCurrentItem(item);
      break;
    }
}

void toSecurity::addRole(void)
{
  for (QListViewItem *item=UserList->firstChild();item;item=item->nextSibling())
    if (item->text(1)=="ROLE:") {
      UserList->clearSelection();
      UserList->setCurrentItem(item);
      break;
    }
}

void toSecurity::copy(void)
{
  General->clear();
  SystemGrant->eraseUser(false);
  RoleGrant->eraseUser(General->user(),false);
  ObjectGrant->eraseUser(false);
  Quota->clear();
  if (General->user())
    UserID="USER:";
  else
    UserID="ROLE:";
  for (QListViewItem *item=UserList->firstChild();item;item=item->nextSibling())
    if (item->text(1)==UserID) {
      disconnect(UserList,SIGNAL(currentChanged(QListViewItem *)),
		 this,SLOT(changeUser(QListViewItem *)));
      UserList->clearSelection();
      UserList->setCurrentItem(item);
      connect(UserList,SIGNAL(currentChanged(QListViewItem *)),
	      this,SLOT(changeUser(QListViewItem *)));
      break;
    }
}
