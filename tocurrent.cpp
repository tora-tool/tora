//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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

static toSQL SQLVersion("toCurrent:Version",
			"select banner \"Version\" from v$version",
			"Display version of Oracle");

static toSQL SQLParameters("toCurrent:Parameters",
			   "select name,value,description from v$parameter",
			   "Display parameters of Oracle server");

toCurrent::toCurrent(QWidget *main,toConnection &connection)
  : toToolWidget(CurrentTool,"current.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,"SQL Output",connection.description());

  new QToolButton(QPixmap((const char **)refresh_xpm),
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
