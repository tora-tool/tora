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

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "tofilesize.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "toresultview.h"
#include "tosecurity.h"
#include "tosql.h"
#include "totool.h"

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qsplitter.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qvalidator.h>
#include <qworkspace.h>

#include "tosecurity.moc"
#include "tosecurityquotaui.moc"
#include "tosecurityroleui.moc"
#include "tosecurityuserui.moc"

#include "icons/addrole.xpm"
#include "icons/adduser.xpm"
#include "icons/commit.xpm"
#include "icons/copyuser.xpm"
#include "icons/refresh.xpm"
#include "icons/sql.xpm"
#include "icons/tosecurity.xpm"
#include "icons/trash.xpm"

static toSQL SQLUserInfo("toSecurity:UserInfo",
			 "SELECT Account_Status,\n"
			 "       Password,\n"
			 "       External_Name,\n"
			 "       Profile,\n"
			 "       Default_Tablespace,\n"
			 "       Temporary_Tablespace\n"
			 "  FROM sys.DBA_Users\n"
			 " WHERE UserName = :f1<char[100]>",
			 "Get information about a user, must have same columns and same binds.");

static toSQL SQLUserInfo7("toSecurity:UserInfo",
			  "SELECT 'OPEN',\n"
			  "       Password,\n"
			  "       NULL,\n"
			  "       Profile,\n"
			  "       Default_Tablespace,\n"
			  "       Temporary_Tablespace\n"
			  "  FROM sys.DBA_Users\n"
			  " WHERE UserName = :f1<char[100]>",
			  "",
			  "7.3");

static toSQL SQLRoleInfo("toSecurity:RoleInfo",
			 "SELECT Role,Password_required FROM sys.DBA_Roles WHERE Role = :f1<char[101]>",
			 "Get information about a role, must have same columns and same binds.");

static toSQL SQLProfiles("toSecurity:Profiles",
			 "SELECT DISTINCT Profile FROM sys.DBA_Profiles ORDER BY Profile",
			 "Get profiles available.");

static toSQL SQLTablespace("toSecurity:Tablespaces",
			   "SELECT DISTINCT Tablespace_Name FROM sys.DBA_Tablespaces"
			   " ORDER BY Tablespace_Name",
			   "Get tablespaces available.");

static toSQL SQLRoles("toSecurity:Roles",
		      "SELECT Role FROM sys.Dba_Roles ORDER BY Role",
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
		      "  FROM sys.DBA_TS_Quotas\n"
		      " WHERE Username = :f1<char[200]>\n"
		      " ORDER BY Tablespace_name",
		      "Get information about what quotas the user has, "
		      "must have same columns and same binds.");

static toSQL SQLSystemGrant("toSecurity:SystemGrant",
			    "SELECT privilege, NVL(admin_option,'NO') FROM sys.dba_sys_privs WHERE grantee = :f1<char[100]>",
			    "Get information about the system privileges a user has, should have same bindings and columns");

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
			    "			       NULL) FROM sys.DUAL",
			    "Takes a type as parameter and return ',' separated list of privileges");

static toSQL SQLObjectGrant("toSecurity:ObjectGrant",
			    "SELECT owner,\n"
			    "       table_name,\n"
			    "       privilege,\n"
			    "       grantable\n"
			    "  FROM sys.dba_tab_privs\n"
			    " WHERE grantee = :f1<char[100]>",
			    "Get the privilege on objects for a user or role, "
			    "must have same columns and binds");

static toSQL SQLRoleGrant("toSecurity:RoleGrant",
			  "SELECT granted_role,\n"
			  "       admin_option,\n"
			  "       default_role\n"
			  "  FROM sys.dba_role_privs\n"
			  " WHERE grantee = :f1<char[100]>",
			  "Get the roles granted to a user or role, "
			  "must have same columns and binds");

class toSecurityTool : public toTool {
protected:
  virtual const char **pictureXPM(void)
  { return tosecurity_xpm; }
public:
  toSecurityTool()
    : toTool(40,"Security Manager")
  { }
  virtual const char *menuItem()
  { return "Security Manager"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toSecurity(parent,connection);
  }
};

static toSecurityTool SecurityTool;

class toSecurityQuota : public toSecurityQuotaUI {
  QListViewItem *CurrentItem;
  void clearItem(QListViewItem *item);
  virtual void changeTablespace(void);
  virtual void changeSize(void);
public:
  toSecurityQuota(QWidget *parent);
  void changeUser(const QString &);
  QString sql(void);
  void clear(void);
  void update(void);
};

void toSecurityQuota::changeSize(void)
{
  if (CurrentItem) {
    if (Value->isChecked()) {
      QString siz;
      siz.sprintf("%.0f KB",double(Size->value()));
      CurrentItem->setText(1,siz);
    } else if (None->isChecked()) {
      CurrentItem->setText(1,qApp->translate("toSecurityQuota","None"));
    } else if (Unlimited->isChecked()) {
      CurrentItem->setText(1,qApp->translate("toSecurityQuota","Unlimited"));
    }
  } else
    SizeGroup->setEnabled(false);
}

toSecurityQuota::toSecurityQuota(QWidget *parent)
  : toSecurityQuotaUI(parent)
{
  CurrentItem=NULL;
  update();
}

void toSecurityQuota::update(void)
{
  Tablespaces->clear();
  try {
    toQuery tablespaces(toCurrentConnection(this),SQLTablespace);
    QListViewItem *item=NULL;
    while(!tablespaces.eof()) {
      item=new toResultViewItem(Tablespaces,item,tablespaces.readValue());
      item->setText(1,qApp->translate("toSecurityQuota","None"));
      item->setText(3,qApp->translate("toSecurityQuota","None"));
    }
  } TOCATCH
}

void toSecurityQuota::clearItem(QListViewItem *item)
{
  item->setText(1,qApp->translate("toSecurityQuota","None"));
  item->setText(2,QString::null);
  item->setText(3,qApp->translate("toSecurityQuota","None"));
}

void toSecurityQuota::clear(void)
{
  for (QListViewItem *item=Tablespaces->firstChild();item;item=item->nextSibling())
    item->setText(3,qApp->translate("toSecurityQuota","None"));
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
      toQuery quota(toCurrentConnection(this),SQLQuota,user);
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
	    maxStr=qApp->translate("toSecurityQuota","Unlimited");
	  else if (maxQuota==0)
	    maxStr=qApp->translate("toSecurityQuota","None");
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
    if (siz==qApp->translate("toSecurityQuota","None"))
      None->setChecked(true);
    else if (siz==qApp->translate("toSecurityQuota","Unlimited"))
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
      if (siz.right(2)==QString::fromLatin1("KB"))
	siz.truncate(siz.length()-1);
      else if (siz==qApp->translate("toSecurityQuota","None"))
	siz=QString::fromLatin1("0 K");
      else if (siz==qApp->translate("toSecurityQuota","Unlimited"))
	siz=QString::fromLatin1("UNLIMITED");
      ret+=QString::fromLatin1(" QUOTA ");
      ret+=siz;
      ret+=QString::fromLatin1(" ON ");
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
				   qApp->translate("toSecurityUser","Passwords don't match"),
				   qApp->translate("toSecurityUser","The two versions of the password doesn't match"),
				   qApp->translate("toSecurityUser","Don't save"),	
				   qApp->translate("toSecurityUser","Cancel"))) {
      case 0:
	return QString::null;
      case 1:
	throw qApp->translate("toSecurityUser","Passwords don't match");
      }
    }
    if (Password->text()!=OrgPassword) {
      extra=QString::fromLatin1(" IDENTIFIED BY \"");
      extra+=Password->text();
      extra+=QString::fromLatin1("\"");
    }
    if (OrgExpired!=ExpirePassword->isChecked()) {
      if (ExpirePassword->isChecked())
	extra+=QString::fromLatin1(" PASSWORD EXPIRE");
    }
  } else if (Authentication->currentPage()==GlobalTab) {
    if (OrgGlobal!=GlobalName->text()) {
      extra=QString::fromLatin1(" IDENTIFIED GLOBALLY AS '");
      extra+=GlobalName->text();
      extra+=QString::fromLatin1("'");
    }
  } else if ((AuthType!=external)&&(Authentication->currentPage()==ExternalTab))
    extra=QString::fromLatin1(" IDENTIFIED EXTERNALLY");

  if (OrgProfile!=Profile->currentText()) {
    extra+=QString::fromLatin1(" PROFILE \"");
    extra+=Profile->currentText();
    extra+=QString::fromLatin1("\"");
  }
  if (OrgDefault!=DefaultSpace->currentText()) {
    extra+=QString::fromLatin1(" DEFAULT TABLESPACE \"");
    extra+=DefaultSpace->currentText();
    extra+=QString::fromLatin1("\"");
  }
  if (OrgTemp!=TempSpace->currentText()) {
    extra+=QString::fromLatin1(" TEMPORARY TABLESPACE \"");
    extra+=TempSpace->currentText();
    extra+=QString::fromLatin1("\"");
  }
  if (OrgLocked!=Locked->isChecked()) {
    extra+=QString::fromLatin1(" ACCOUNT ");
    if (Locked->isChecked())
      extra+=QString::fromLatin1("LOCK");
    else
      extra+=QString::fromLatin1("UNLOCK");
  }
  extra+=Quota->sql();

  QString sql;
  if (Name->isEnabled()) {
    if (Name->text().isEmpty())
      return QString::null;
    sql=QString::fromLatin1("CREATE ");
  } else {
    if (extra.isEmpty())
      return QString::null;
    sql=QString::fromLatin1("ALTER ");
  }
  sql+=QString::fromLatin1("USER \"");
  sql+=Name->text();
  sql+=QString::fromLatin1("\"");
  sql+=extra;
  return sql;
}

toSecurityUser::toSecurityUser(toSecurityQuota *quota,toConnection &conn,QWidget *parent)
  : toSecurityUserUI(parent),Connection(conn),Quota(quota)
{
  Name->setValidator(new toSecurityUpper(Name));
  setFocusProxy(Name);
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
  Name->setText(QString::null);
  Password->setText(QString::null);
  Password2->setText(QString::null);
  GlobalName->setText(QString::null);
  if (all) {
    Profile->setCurrentItem(0);
    Authentication->showPage(PasswordTab);
    ExpirePassword->setChecked(false);
    ExpirePassword->setEnabled(true);
    TempSpace->setCurrentItem(0);
    DefaultSpace->setCurrentItem(0);
    Locked->setChecked(false);
  }

  OrgProfile=OrgDefault=OrgTemp=OrgGlobal=QString::null;
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
      if (str.startsWith(QString::fromLatin1("EXPIRED"))) {
	ExpirePassword->setChecked(true);
	ExpirePassword->setEnabled(false);
	OrgExpired=true;
      } else if (str.startsWith(QString::fromLatin1("LOCKED"))) {
	Locked->setChecked(true);
	OrgLocked=true;
      }

      OrgPassword=query.readValue();
      QString pass=query.readValue();
      if (OrgPassword==QString::fromLatin1("GLOBAL")) {
	OrgPassword=QString::null;
	Authentication->showPage(GlobalTab);
	OrgGlobal=pass;
	GlobalName->setText(OrgGlobal);
	AuthType=global;
      } else if (OrgPassword==QString::fromLatin1("EXTERNAL")) {
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
  {
    Name->setValidator(new toSecurityUpper(Name));
    setFocusProxy(Name);
  }
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
				   qApp->translate("toSecurityRole","Passwords don't match"),
				   qApp->translate("toSecurityRole","The two versions of the password doesn't match"),
				   qApp->translate("toSecurityRole","Don't save"),
				   qApp->translate("toSecurityRole","Cancel"))) {
      case 0:
	return QString::null;
      case 1:
	throw qApp->translate("toSecurityRole","Passwords don't match");
      }
    }
    if (Password->text().length()>0) {
      extra=QString::fromLatin1(" IDENTIFIED BY \"");
      extra+=Password->text();
      extra+=QString::fromLatin1("\"");
    }
  } else if ((AuthType!=global)&&(Authentication->currentPage()==GlobalTab))
    extra=QString::fromLatin1(" IDENTIFIED GLOBALLY");
  else if ((AuthType!=external)&&(Authentication->currentPage()==ExternalTab))
    extra=QString::fromLatin1(" IDENTIFIED EXTERNALLY");
  else if ((AuthType!=none)&&(Authentication->currentPage()==NoneTab))
    extra=QString::fromLatin1(" NOT IDENTIFIED");
  extra+=Quota->sql();
  QString sql;
  if (Name->isEnabled()) {
    if (Name->text().isEmpty())
      return QString::null;
    sql=QString::fromLatin1("CREATE ");
  } else {
    if (extra.isEmpty())
      return QString::null;
    sql=QString::fromLatin1("ALTER ");
  }
  sql+=QString::fromLatin1("ROLE \"");
  sql+=Name->text();
  sql+=QString::fromLatin1("\"");
  sql+=extra;
  return sql;
}

void toSecurityRole::clear(void)
{
  Name->setText(QString::null);
  Name->setEnabled(true);
}

void toSecurityRole::changeRole(const QString &role)
{
  try {
    toQuery query(Connection,SQLRoleInfo,role);
    Password->setText(QString::null);
    Password2->setText(QString::null);
    if (!query.eof()) {
      Name->setText(role);
      Name->setEnabled(false);

      QString str(query.readValue());
      if (str==QString::fromLatin1("YES")) {
	AuthType=password;
	Authentication->showPage(PasswordTab);
      } else if (str==QString::fromLatin1("GLOBAL")) {
	AuthType=global;
	Authentication->showPage(GlobalTab);
      } else if (str==QString::fromLatin1("EXTERNAL")) {
	AuthType=external;
	Authentication->showPage(ExternalTab);
      } else {
	AuthType=none;
	Authentication->showPage(NoneTab);
      }
    } else {
      Name->setText(QString::null);
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
    setFocusProxy(User);
  }
  void changePage(const QString &nam,bool user)
  {
    if (user) {
      Role->hide();
      User->show();
      User->changeUser(nam);
      setFocusProxy(User);
    } else {
      User->hide();
      Role->show();
      Role->changeRole(nam);
      setFocusProxy(Role);
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

toSecurityObject::toSecurityObject(QWidget *parent)
  : toListView(parent)
{
  addColumn(tr("Object"));
  setRootIsDecorated(true);
  update();
  setSorting(0);
  connect(this,SIGNAL(clicked(QListViewItem *)),this,SLOT(changed(QListViewItem *)));
}


void toSecurityObject::update(void)
{
  clear();
  try {
    QString oType;
    QString oOwner;
    QString oName;
    std::list<toConnection::objectName> &objectList=toCurrentConnection(this).objects(true);
    std::map<QString,QStringList> TypeOptions;
    toQuery typelst(toCurrentConnection(this));
    QListViewItem *typeItem=NULL;
    QListViewItem *ownerItem=NULL;
    QListViewItem *nameItem=NULL;
    QStringList Options;
    for(std::list<toConnection::objectName>::iterator i=objectList.begin();i!=objectList.end();i++) {
      QString type=(*i).Type;
      QString owner=(*i).Owner;
      QString name=(*i).Name;
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
	if (TypeOptions.find(type)==TypeOptions.end()) {
	  toQList args;
	  toPush(args,toQValue(type));
	  typelst.execute(SQLObjectPrivs,args);
	  Options=QStringList::split(QString::fromLatin1(","),typelst.readValue());
	  TypeOptions[type]=Options;
	} else
	  Options=TypeOptions[type];

	if (Options.count()>0) {
	  for (typeItem=ownerItem->firstChild();typeItem;typeItem=typeItem->nextSibling()) {
	    if (typeItem->text(0)==type)
	      break;
	  }
	  if (!typeItem)
	    typeItem=new toResultViewItem(ownerItem,typeItem,type);
	}
      }
      if (Options.count()>0) {
	nameItem=new toResultViewItem(typeItem,nameItem,name);
	for (QStringList::Iterator i=Options.begin();i!=Options.end();i++) {
	  QListViewItem *item=new toResultViewCheck(nameItem,*i,QCheckListItem::CheckBox);
	  item->setText(2,name);
	  item->setText(3,owner);
	  new toResultViewCheck(item,tr("Admin"),QCheckListItem::CheckBox);
	}
      }
    }
  } TOCATCH
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
    std::map<QString,std::map<QString,std::map<QString,QString> > > privs;
    toQuery grant(toCurrentConnection(this),SQLObjectGrant,user);
    QString yes="YES";
    QString admstr="ADMIN";
    QString normalstr="normal";
    while(!grant.eof()) {
      QString owner(grant.readValue());
      QString object(grant.readValue());
      QString priv(grant.readValue());
      QString admin(grant.readValue());

      ((privs[owner])[object])[priv]=(admin==yes?admstr:normalstr);
    }

    for (QListViewItem *ownerItem=firstChild();ownerItem;ownerItem=ownerItem->nextSibling()) {
      for(QListViewItem *tmpitem=ownerItem->firstChild();tmpitem;tmpitem=tmpitem->nextSibling()) {
	for(QListViewItem *object=tmpitem->firstChild();object;object=object->nextSibling()) {
	  for(QListViewItem *type=object->firstChild();type;type=type->nextSibling()) {
	    QString t=((privs[ownerItem->text(0)])[object->text(0)])[type->text(0)];
	    if (!t.isNull()) {
	      toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(type);
	      if (chk) {
		chk->setText(1,tr("ON"));
		chk->setOn(true);
		if (t==admstr) {
		  toResultViewCheck *chld=dynamic_cast<toResultViewCheck *>(type->firstChild());
		  if (chld) {
		    chld->setText(1,tr("ON"));
		    chld->setOn(true);
		    if (open)
		      chk->setOpen(true);
		  }
		}
	      }
	      if (open)
		for(QListViewItem *par=chk->parent();par;par=par->parent())
		  par->setOpen(true);
	    }
	  }
	}
      }
    }
  } TOCATCH
}

void toSecurityObject::sql(const QString &user,std::list<QString> &sqlLst)
{
  QListViewItem *next=NULL;
  for (QListViewItem *item=firstChild();item;item=next) {
    toResultViewCheck *check=dynamic_cast<toResultViewCheck *>(item);
    toResultViewCheck *chld=dynamic_cast<toResultViewCheck *>(item->firstChild());
    if (check) {
      QString sql;
      QString what=item->text(0);
      what+=QString::fromLatin1(" ON \"");
      what+=item->text(3);
      what+=QString::fromLatin1("\".\"");
      what+=item->text(2);
      what+=QString::fromLatin1("\" ");
      if (chld&&chld->isOn()&&chld->text(1).isEmpty()) {
	sql=QString::fromLatin1("GRANT ");
	sql+=what;
	sql+=QString::fromLatin1("TO \"");
	sql+=user;
	sql+=QString::fromLatin1("\" WITH GRANT OPTION");
	sqlLst.insert(sqlLst.end(),sql);
      } else if (check->isOn()&&!item->text(1).isEmpty()) {
	if (chld&&!chld->isOn()&&!chld->text(1).isEmpty()) {
	  sql=QString::fromLatin1("REVOKE ");
	  sql+=what;
	  sql+=QString::fromLatin1("FROM \"");
	  sql+=user;
	  sql+=QString::fromLatin1("\"");
	  sqlLst.insert(sqlLst.end(),sql);	

	  sql=QString::fromLatin1("GRANT ");
	  sql+=what;
	  sql+=QString::fromLatin1("TO \"");
	  sql+=user;
	  sql+=QString::fromLatin1("\"");
	  sqlLst.insert(sqlLst.end(),sql);
	}
      } else if (check->isOn()&&item->text(1).isEmpty()) {
	sql=QString::fromLatin1("GRANT ");
	sql+=what;
	sql+=QString::fromLatin1("TO \"");
	sql+=user;
	sql+=QString::fromLatin1("\"");
	sqlLst.insert(sqlLst.end(),sql);
      } else if (!check->isOn()&&!item->text(1).isEmpty()) {
	sql=QString::fromLatin1("REVOKE ");
	sql+=what;
	sql+=QString::fromLatin1("FROM \"");
	sql+=user;
	sql+=QString::fromLatin1("\"");
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

toSecuritySystem::toSecuritySystem(QWidget *parent)
  : toListView(parent)
{
  addColumn(tr("Privilege name"));
  setRootIsDecorated(true);
  update();
  setSorting(0);
  connect(this,SIGNAL(clicked(QListViewItem *)),this,SLOT(changed(QListViewItem *)));
}

void toSecuritySystem::update(void)
{
  clear();
  try {
    toQuery priv(toCurrentConnection(this),SQLListSystem);
    while(!priv.eof()) {
      toResultViewCheck *item=new toResultViewCheck(this,priv.readValue(),
						    QCheckListItem::CheckBox);
      new toResultViewCheck(item,tr("Admin"),QCheckListItem::CheckBox);
    }
  } TOCATCH
}

void toSecuritySystem::sql(const QString &user,std::list<QString> &sqlLst)
{
  for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
    QString sql;
    toResultViewCheck *check=dynamic_cast<toResultViewCheck *>(item);
    toResultViewCheck *chld=dynamic_cast<toResultViewCheck *>(item->firstChild());
    if (chld&&chld->isOn()&&chld->text(1).isEmpty()) {
      sql=QString::fromLatin1("GRANT ");
      sql+=item->text(0);
      sql+=QString::fromLatin1(" TO \"");
      sql+=user;
      sql+=QString::fromLatin1("\" WITH ADMIN OPTION");
      sqlLst.insert(sqlLst.end(),sql);
    } else if (check->isOn()&&!item->text(1).isEmpty()) {
      if (chld&&!chld->isOn()&&!chld->text(1).isEmpty()) {
	sql=QString::fromLatin1("REVOKE ");
	sql+=item->text(0);
	sql+=QString::fromLatin1(" FROM \"");
	sql+=user;
	sql+=QString::fromLatin1("\"");
	sqlLst.insert(sqlLst.end(),sql);	

	sql=QString::fromLatin1("GRANT ");
	sql+=item->text(0);
	sql+=QString::fromLatin1(" TO \"");
	sql+=user;
	sql+=QString::fromLatin1("\"");
	sqlLst.insert(sqlLst.end(),sql);
      }
    } else if (check->isOn()&&item->text(1).isEmpty()) {
      sql=QString::fromLatin1("GRANT ");
      sql+=item->text(0);
      sql+=QString::fromLatin1(" TO \"");
      sql+=user;
      sql+=QString::fromLatin1("\"");
      sqlLst.insert(sqlLst.end(),sql);
    } else if (!check->isOn()&&!item->text(1).isEmpty()) {
      sql=QString::fromLatin1("REVOKE ");
      sql+=item->text(0);
      sql+=QString::fromLatin1(" FROM \"");
      sql+=user;
      sql+=QString::fromLatin1("\"");
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
    toQuery query(toCurrentConnection(this),SQLSystemGrant,user);
    while(!query.eof()) {
      QString str=query.readValue();
      QString admin=query.readValue();
      for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
	if (item->text(0)==str) {
	  toResultViewCheck *chk=dynamic_cast<toResultViewCheck *>(item);
	  if (chk)
	    chk->setOn(true);
	  item->setText(1,tr("ON"));
	  if (admin!=tr("NO")&&item->firstChild()) {
	    chk=dynamic_cast<toResultViewCheck *>(item->firstChild());
	    if (chk)
	      chk->setOn(true);
	    if (chk->parent())
	      chk->parent()->setOpen(true);
	    item->firstChild()->setText(1,tr("ON"));
	  }
	  break;
	}
      }
    }
  } TOCATCH
}

toSecurityRoleGrant::toSecurityRoleGrant(QWidget *parent)
  : toListView(parent)
{
  addColumn(tr("Role name"));
  setRootIsDecorated(true);
  update();
  setSorting(0);
  connect(this,SIGNAL(clicked(QListViewItem *)),this,SLOT(changed(QListViewItem *)));
}

void toSecurityRoleGrant::update(void)
{
  clear();
  try {
    toQuery priv(toCurrentConnection(this),SQLRoles);
    while(!priv.eof()) {
      toResultViewCheck *item=new toResultViewCheck(this,priv.readValue(),QCheckListItem::CheckBox);
      new toResultViewCheck(item,tr("Admin"),QCheckListItem::CheckBox);
      new toResultViewCheck(item,tr("Default"),QCheckListItem::CheckBox);
    }
  } TOCATCH
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

void toSecurityRoleGrant::sql(const QString &user,std::list<QString> &sqlLst)
{
  bool any=false;
  bool chg=false;
  QString except;
  QString sql;
  for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
    toResultViewCheck *check=dynamic_cast<toResultViewCheck *>(item);
    QCheckListItem *chld=findChild(item,tr("Admin"));
    QCheckListItem *def=findChild(item,tr("Default"));
    if (def&&check) {
      if (!def->isOn()&&check->isOn()) {
	if (!except.isEmpty())
	  except+=QString::fromLatin1(",\"");
	else
	  except+=QString::fromLatin1(" EXCEPT \"");
	except+=item->text(0);
	except+=QString::fromLatin1("\"");
      } else if (check->isOn()&&def->isOn())
	any=true;
      if (def->isOn()==def->text(1).isEmpty())
	chg=true;
    }
    if (chld&&chld->isOn()&&chld->text(1).isEmpty()) {
      if (check->isOn()&&!item->text(1).isEmpty()) {
	sql=QString::fromLatin1("REVOKE \"");
	sql+=item->text(0);
	sql+=QString::fromLatin1("\" FROM \"");
	sql+=user;
	sql+=QString::fromLatin1("\"");
	sqlLst.insert(sqlLst.end(),sql);
      }
      sql=QString::fromLatin1("GRANT \"");
      sql+=item->text(0);
      sql+=QString::fromLatin1("\" TO \"");
      sql+=user;
      sql+=QString::fromLatin1("\" WITH ADMIN OPTION");
      sqlLst.insert(sqlLst.end(),sql);
      chg=true;
    } else if (check->isOn()&&!item->text(1).isEmpty()) {
      if (chld&&!chld->isOn()&&!chld->text(1).isEmpty()) {
	sql=QString::fromLatin1("REVOKE \"");
	sql+=item->text(0);
	sql+=QString::fromLatin1("\" FROM \"");
	sql+=user;
	sql+=QString::fromLatin1("\"");
	sqlLst.insert(sqlLst.end(),sql);	

	sql=QString::fromLatin1("GRANT \"");
	sql+=item->text(0);
	sql+=QString::fromLatin1("\" TO \"");
	sql+=user;
	sql+=QString::fromLatin1("\"");
	sqlLst.insert(sqlLst.end(),sql);
	chg=true;
      }
    } else if (check->isOn()&&item->text(1).isEmpty()) {
      sql=QString::fromLatin1("GRANT \"");
      sql+=item->text(0);
      sql+=QString::fromLatin1("\" TO \"");
      sql+=user;
      sql+=QString::fromLatin1("\"");
      sqlLst.insert(sqlLst.end(),sql);
      chg=true;
    } else if (!check->isOn()&&!item->text(1).isEmpty()) {
      sql=QString::fromLatin1("REVOKE \"");
      sql+=item->text(0);
      sql+=QString::fromLatin1("\" FROM \"");
      sql+=user;
      sql+=QString::fromLatin1("\"");
      sqlLst.insert(sqlLst.end(),sql);
      chg=true;
    }
  }
  if (chg) {
    sql=QString::fromLatin1("ALTER USER \"");
    sql+=user;
    sql+=QString::fromLatin1("\" DEFAULT ROLE ");
    if (any) {
      sql+=QString::fromLatin1("ALL");
      sql+=except;
    } else
      sql+=QString::fromLatin1("NONE");
    sqlLst.insert(sqlLst.end(),sql);
  }
}

void toSecurityRoleGrant::changed(QListViewItem *org)
{
  toResultViewCheck *item=dynamic_cast<toResultViewCheck *>(org);
  if (item) {
    if (item->isOn()) {
      QCheckListItem *chld=findChild(item,tr("Default"));
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
	  if (chk->text(0)==tr("Default"))
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
    toQuery query(toCurrentConnection(this),SQLRoleGrant,username);
    while(!query.eof()) {
      QString str=query.readValue();
      QString admin=query.readValue();
      QString def=query.readValue();
      for (QListViewItem *item=firstChild();item;item=item->nextSibling()) {
	if (item->text(0)==str) {
	  QCheckListItem *chk=dynamic_cast<toResultViewCheck *>(item);
	  if (chk)
	    chk->setOn(true);
	  item->setText(1,tr("ON"));
	  chk=findChild(item,tr("Admin"));
	  if (admin==tr("YES")&&chk) {
	    chk->setOn(true);
	    chk->setText(1,tr("ON"));
	    if (chk->parent())
	      chk->parent()->setOpen(true);
	  }
	  chk=findChild(item,tr("Default"));
	  if (def==tr("YES")&&chk) {
	    chk->setOn(true);
	    chk->setText(1,tr("ON"));
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
  : toToolWidget(SecurityTool,"security.html",main,connection)
{
  toBusy busy;

  QToolBar *toolbar=toAllocBar(this,tr("Security manager"));
  toolbar->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed));

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  tr("Update user and role list"),
		  tr("Update user and role list"),
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)commit_xpm),
		  tr("Save changes"),
		  tr("Save changes"),
		  this,SLOT(saveChanges(void)),
		  toolbar);
  DropButton=new QToolButton(QPixmap((const char **)trash_xpm),
			     tr("Remove user/role"),
			     tr("Remove user/role"),
			     this,SLOT(drop(void)),
			     toolbar);
  DropButton->setEnabled(false);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)adduser_xpm),
		  tr("Add new user"),
		  tr("Add new user"),
		  this,SLOT(addUser(void)),
		  toolbar);
  new QToolButton(QPixmap((const char **)addrole_xpm),
		  tr("Add new role"),
		  tr("Add new role"),
		  this,SLOT(addRole(void)),
		  toolbar);
  CopyButton=new QToolButton(QPixmap((const char **)copyuser_xpm),
			     tr("Copy current user or role"),
			     tr("Copy current user or role"),
			     this,SLOT(copy(void)),
			     toolbar);
  CopyButton->setEnabled(false);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)sql_xpm),
		  tr("Display SQL needed to make current changes"),
		  tr("Display SQL needed to make current changes"),
		  this,SLOT(displaySQL(void)),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel(toolbar,TO_KDE_TOOLBAR_WIDGET));
  new toChangeConnection(toolbar,TO_KDE_TOOLBAR_WIDGET);

  QSplitter *splitter=new QSplitter(Horizontal,this);
  UserList=new toListView(splitter);
  UserList->addColumn(tr("Users/Roles"));
  UserList->setSQLName(QString::fromLatin1("toSecurity:Users/Roles"));
  UserList->setRootIsDecorated(true);
  UserList->setSelectionMode(QListView::Single);
  Tabs=new QTabWidget(splitter);
  Quota=new toSecurityQuota(Tabs);
  General=new toSecurityPage(Quota,connection,Tabs);
  Tabs->addTab(General,tr("&General"));
  RoleGrant=new toSecurityRoleGrant(Tabs);
  Tabs->addTab(RoleGrant,tr("&Roles"));
  SystemGrant=new toSecuritySystem(Tabs);
  Tabs->addTab(SystemGrant,tr("&System Privileges"));
  ObjectGrant=new toSecurityObject(Tabs);
  Tabs->addTab(ObjectGrant,tr("&Object Privileges"));
  Tabs->addTab(Quota,tr("&Quota"));
  UserList->setSelectionMode(QListView::Single);
  connect(UserList,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeUser(QListViewItem *)));
  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));
  refresh();
  connect(this,SIGNAL(connectionChange()),
	  this,SLOT(refresh()));
  setFocusProxy(Tabs);
}

#define TO_ID_COPY		(toMain::TO_TOOL_MENU_ID+ 0)
#define TO_ID_DROP		(toMain::TO_TOOL_MENU_ID+ 1)

void toSecurity::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),tr("&Refresh"),
			   this,SLOT(refresh(void)),
			   toKeySequence(tr("F5", "Security|Refresh")));
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)commit_xpm),tr("&Save changes"),
			   this,SLOT(saveChanges()),
			   toKeySequence(tr("Ctrl+Return", "Security|Save changes")));
      ToolMenu->insertItem(QPixmap((const char **)trash_xpm),tr("&Remove user/role"),
			   this,SLOT(drop()),0,TO_ID_DROP);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)adduser_xpm),tr("Add &user"),
			   this,SLOT(addUser()),
			   toKeySequence(tr("Ctrl+Shift+U", "Security|Add user")));
      ToolMenu->insertItem(QPixmap((const char **)addrole_xpm),tr("Add &role"),
			   this,SLOT(addRole()),
			   toKeySequence(tr("Ctrl+Shift+R", "Security|Add role")));
      ToolMenu->insertItem(QPixmap((const char **)copyuser_xpm),tr("&Copy current"),
			   this,SLOT(copy()),
			   toKeySequence(tr("Ctrl+Shift+O", "Security|Copy current")),TO_ID_COPY);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)sql_xpm),tr("Display SQL..."),
			   this,SLOT(displaySQL()),
			   toKeySequence(tr("F4", "Security|Display SQL")));
      toMainWidget()->menuBar()->insertItem(tr("&Security"),ToolMenu,-1,toToolMenuIndex());
      ToolMenu->setItemEnabled(TO_ID_DROP,DropButton->isEnabled());
      ToolMenu->setItemEnabled(TO_ID_COPY,CopyButton->isEnabled());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

void toSecurity::displaySQL(void)
{
  std::list<QString> lines=sql();
  QString res;
  for(std::list<QString>::iterator i=lines.begin();i!=lines.end();i++) {
    res+=*i;
    res+=QString::fromLatin1(";\n");
  }
  if (res.length()>0)
    new toMemoEditor(this,res,-1,-1,true);
  else
    toStatusMessage(tr("No changes made"));
}

std::list<QString> toSecurity::sql(void)
{
  std::list<QString> ret;
  try {
    QString tmp=General->sql();
    if (!tmp.isEmpty())
      ret.insert(ret.end(),tmp);
    QString name=General->name();
    if (!name.isEmpty()) {
      SystemGrant->sql(name,ret);
      ObjectGrant->sql(name,ret);
      RoleGrant->sql(name,ret);
    }
  } catch(const QString &str) {
    toStatusMessage(str);
    std::list<QString> empty;
    return empty;
  }

  return ret;
}

void toSecurity::changeUser(bool ask)
{
  if (ask) {
    try {
      std::list<QString> sqlList=sql();
      if (sqlList.size()!=0) {
	switch(TOMessageBox::warning(this,
				     tr("Save changes?"),
				     tr("Save the changes made to this user?"),
				     tr("Save"),tr("Discard"),tr("Cancel"))) {
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
    QListViewItem *item=UserList->selectedItem();
    if (item) {
      toBusy busy;
      UserID=item->text(1);
      DropButton->setEnabled(item->parent());
      CopyButton->setEnabled(item->parent());
      if (ToolMenu) {
	ToolMenu->setItemEnabled(TO_ID_DROP,DropButton->isEnabled());
	ToolMenu->setItemEnabled(TO_ID_COPY,CopyButton->isEnabled());
      }

      if (UserID[4].latin1()!=':')
	throw tr("Invalid security ID");
      bool user=false;
      if (UserID.startsWith(QString::fromLatin1("USER")))
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
  toBusy busy;
  disconnect(UserList,SIGNAL(selectionChanged(QListViewItem *)),
	     this,SLOT(changeUser(QListViewItem *)));
  SystemGrant->update();
  RoleGrant->update();
  ObjectGrant->update();
  Quota->update();
  UserList->clear();
  try {
    QListViewItem *parent=new toResultViewItem(UserList,NULL,QString::fromLatin1("Users"));
    parent->setText(1,QString::fromLatin1("USER:"));
    parent->setOpen(true);
    toQuery user(connection(),toSQL::string(toSQL::TOSQL_USERLIST,connection()));
    QListViewItem *item=NULL;
    while(!user.eof()) {
      QString tmp=user.readValue();
      QString id=QString::fromLatin1("USER:");
      id+=tmp;
      item=new toResultViewItem(parent,item,tmp);
      item->setText(1,id);
      if (id==UserID)
	UserList->setSelected(item,true);
    }
    parent=new toResultViewItem(UserList,parent,tr("Roles"));
    parent->setText(1,QString::fromLatin1("ROLE:"));
    parent->setOpen(true);
    toQuery roles(connection(),SQLRoles);
    item=NULL;
    while(!roles.eof()) {
      QString tmp=roles.readValue();
      QString id=QString::fromLatin1("ROLE:");
      id+=tmp;
      item=new toResultViewItem(parent,item,tmp);
      item->setText(1,id);
      if (id==UserID)
	UserList->setSelected(item,true);
    }
  } TOCATCH
  connect(UserList,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeUser(QListViewItem *)));
}

void toSecurity::saveChanges()
{
  std::list<QString> sqlList=sql();
  for (std::list<QString>::iterator i=sqlList.begin();i!=sqlList.end();i++) {
    try {
      connection().execute(*i);
    } TOCATCH
  }
  if (General->user())
    UserID=QString::fromLatin1("USER:");
  else
    UserID=QString::fromLatin1("ROLE:");
  UserID+=General->name();
  refresh();
  changeUser(false);
}

void toSecurity::drop()
{
  if (UserID.length()>5) {
    QString str=QString::fromLatin1("DROP ");
    if (General->user())
      str+=QString::fromLatin1("USER");
    else
      str+=QString::fromLatin1("ROLE");
    str+=QString::fromLatin1(" \"");
    str+=UserID.right(UserID.length()-5);
    str+=QString::fromLatin1("\"");
    try {
      connection().execute(str);
      refresh();
      changeUser(false);
    } catch(...) {
      switch(TOMessageBox::warning(this,
				   tr("Are you sure?"),
				   tr("The user still owns objects, add the cascade option?"),
				   tr("Yes"),tr("No"))) {
      case 0:
	str+=QString::fromLatin1(" CASCADE");
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
    if (item->text(1)==QString::fromLatin1("USER:")) {
      UserList->clearSelection();
      UserList->setCurrentItem(item);
      Tabs->showPage(General);
      General->setFocus();
      break;
    }
}

void toSecurity::addRole(void)
{
  for (QListViewItem *item=UserList->firstChild();item;item=item->nextSibling())
    if (item->text(1)==QString::fromLatin1("ROLE:")) {
      UserList->clearSelection();
      UserList->setCurrentItem(item);
      Tabs->showPage(General);
      General->setFocus();
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
    UserID=QString::fromLatin1("USER:");
  else
    UserID=QString::fromLatin1("ROLE:");
  for (QListViewItem *item=UserList->firstChild();item;item=item->nextSibling())
    if (item->text(1)==UserID) {
      disconnect(UserList,SIGNAL(selectionChanged(QListViewItem *)),
		 this,SLOT(changeUser(QListViewItem *)));
      UserList->clearSelection();
      UserList->setCurrentItem(item);
      connect(UserList,SIGNAL(selectionChanged(QListViewItem *)),
	      this,SLOT(changeUser(QListViewItem *)));
      break;
    }
}
