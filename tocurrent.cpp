//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qworkspace.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qlabel.h>

#include "tochangeconnection.h"
#include "tomain.h"
#include "toresultview.h"
#include "totool.h"
#include "tosql.h"
#include "tocurrent.h"
#include "toresultstats.h"
#include "toconnection.h"

#include "tocurrent.moc"

#include "icons/refresh.xpm"
#include "icons/tocurrent.xpm"

class toCurrentInfoTool : public toTool {
protected:
  std::map<toConnection *,QWidget *> Windows;

  virtual char **pictureXPM(void)
  { return tocurrent_xpm; }
public:
  toCurrentInfoTool()
    : toTool(103,"Current Session")
  { }
  virtual const char *menuItem()
  { return "Current Session"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    std::map<toConnection *,QWidget *>::iterator i=Windows.find(&connection);
    if (i!=Windows.end()) {
      (*i).second->setFocus();
      return NULL;
    } else {
      QWidget *window=new toCurrent(parent,connection);
      Windows[&connection]=window;
      window->setIcon(*toolbarImage());
      return window;
    }
  }
  void closeWindow(toConnection &connection)
  {
    std::map<toConnection *,QWidget *>::iterator i=Windows.find(&connection);
    if (i!=Windows.end())
      Windows.erase(i);
  }
};

static toCurrentInfoTool CurrentTool;

static QPixmap *toRefreshPixmap;

static toSQL SQLVersion("toCurrent:Version",
			"select banner \"Version\" from v$version",
			"Display version of Oracle");

static toSQL SQLParameters("toCurrent:Parameters",
			   "select name,value,description from v$parameter",
			   "Display parameters of Oracle server");

toCurrent::toCurrent(QWidget *main,toConnection &connection)
  : toToolWidget(CurrentTool,"current.html",main,connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);

  QToolBar *toolbar=toAllocBar(this,"SQL Output",connection.description());

  new QToolButton(*toRefreshPixmap,
		  "Update",
		  "Update",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  Tabs=new QTabWidget(this);
  Grants=new toListView(Tabs);
  Grants->setSorting(0);
  Grants->addColumn("Privilege");
  Grants->addColumn("Type");
  Grants->addColumn("Grantable");
  Grants->setRootIsDecorated(true);
  Tabs->addTab(Grants,"Privileges");
  Version=new toResultView(true,false,Tabs);
  Version->setSQL(SQLVersion);
  Tabs->addTab(Version,"Version");
  Parameters=new toResultView(true,false,Tabs);
  Parameters->setSQL(SQLParameters);
  Tabs->addTab(Parameters,"Parameters");
  Statistics=new toResultStats(false,Tabs);
  Tabs->addTab(Statistics,"Statistics");
  refresh();
}

toCurrent::~toCurrent()
{
  CurrentTool.closeWindow(connection());
}

static toSQL SQLRoleTabPrivs("toCurrent:RoleTabPrivs",
			     "select privilege||' on '||owner||'.'||table_name,grantable from role_tab_privs where role = :role<char[100]>",
			     "Get information about privileges granted on objects to a role, must have same binds and columns");

static toSQL SQLRoleSysPrivs("toCurrent:RoleSysPrivs",
			     "select privilege,admin_option from role_sys_privs where role = :role<char[100]>",
			     "Get information about system privileges granted to role, must have same binds and columns");

static toSQL SQLRoleRolePrivs("toCurrent:RoleRolePrivs",
			      "select granted_role,admin_option from role_role_privs where role = :role<char[100]>",
			      "Get information about roles granted to a role, must have same binds and columns");

static toSQL SQLUserTabPrivs("toCurrent:UserTabPrivs",
			     "select privilege || ' on ' || owner||'.'||table_name,grantable from user_tab_privs",
			     "Get information about privileges granted on objects to a user, must have same columns");

static toSQL SQLUserSysPrivs("toCurrent:UserSysPrivs",
			     "select privilege,admin_option from user_sys_privs",
			     "Get information about system privileges granted to user, must have same columns");

static toSQL SQLUserRolePrivs("toCurrent:UserRolePrivs",
			      "select granted_role,admin_option from user_role_privs",
			      "Get information about roles granted to a user, must have same columns");

void toCurrent::addList(QListViewItem *parent,const QString &type,toSQL &sql,const QString &role)
{
  toQList result=toQuery::readQuery(connection(),sql,role);
  while(result.size()>0) {
    QListViewItem *item;
    if (parent)
      item=new toResultViewItem(parent,NULL);
    else
      item=new toResultViewItem(Grants,NULL);
    item->setText(0,toShift(result));
    item->setText(1,type);
    item->setText(2,toShift(result));
  }
}

void toCurrent::addRole(QListViewItem *parent)
{
  addList(parent,"System",SQLRoleSysPrivs,parent->text(0));
  addList(parent,"Object",SQLRoleTabPrivs,parent->text(0));
  toQList result=toQuery::readQuery(connection(),SQLRoleRolePrivs,parent->text(0));
  while(result.size()>0) {
    QListViewItem *item;
    item=new toResultViewItem(parent,NULL);
    item->setText(0,toShift(result));
    item->setText(1,"Role");
    item->setText(2,toShift(result));
    addRole(item);
  }
}

void toCurrent::refresh()
{
  Parameters->refresh();
  Parameters->setSorting(0);
  Version->refresh();
  Statistics->refreshStats();
  Grants->clear();

  addList(NULL,"System",SQLUserSysPrivs);
  addList(NULL,"Object",SQLUserTabPrivs);

  toQList result=toQuery::readQuery(connection(),SQLUserRolePrivs);
  while(result.size()>0) {
    QListViewItem *item;
    item=new toResultViewItem(Grants,NULL);
    item->setText(0,toShift(result));
    item->setText(1,"Role");
    item->setText(2,toShift(result));
    addRole(item);
  }
}
