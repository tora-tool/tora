/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "tochangeconnection.h"
#include "toconnection.h"
#include "toconf.h"
#include "tocurrent.h"
#include "tomain.h"
#include "tonoblockquery.h"
#include "toresultlong.h"
#include "toresultparam.h"
#include "toresultstats.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <qlabel.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include "tocurrent.moc"

#include "icons/refresh.xpm"
#include "icons/tocurrent.xpm"

class toCurrentInfoTool : public toTool
{
protected:
    std::map<toConnection *, QWidget *> Windows;

    virtual const char **pictureXPM(void)
    {
        return tocurrent_xpm;
    }
public:
    toCurrentInfoTool()
            : toTool(240, "Current Session")
    { }
    virtual const char *menuItem()
    {
        return "Current Session";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
        if (i != Windows.end())
        {
            (*i).second->raise();
            (*i).second->setFocus();
            return NULL;
        }
        else
        {
            QWidget *window = new toCurrent(parent, connection);
            Windows[&connection] = window;
            return window;
        }
    }
    void closeWindow(toConnection &connection)
    {
        std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
        if (i != Windows.end())
            Windows.erase(i);
    }
};

static toCurrentInfoTool CurrentTool;

static toSQL SQLVersion("toCurrent:Version",
                        "select banner \"Version\" from v$version",
                        "Display version of Oracle");

static toSQL SQLResourceLimit("toCurrent:ResourceLimit",
                              "SELECT * FROM v$resource_limit ORDER BY resource_name",
                              "List resource limits");

toCurrent::toCurrent(QWidget *main, toConnection &connection)
        : toToolWidget(CurrentTool, "current.html", main, connection)
{
    QToolBar *toolbar = toAllocBar(this, tr("Current Session"));

    new QToolButton(QPixmap((const char **)refresh_xpm),
                    tr("Update"),
                    tr("Update"),
                    this, SLOT(refresh(void)),
                    toolbar);
    toolbar->setStretchableWidget(new QLabel(toolbar, TO_KDE_TOOLBAR_WIDGET));
    new toChangeConnection(toolbar, TO_KDE_TOOLBAR_WIDGET);

    Tabs = new QTabWidget(this);

    Grants = new toListView(Tabs);
    Grants->setSorting(0);
    Grants->addColumn(tr("Privilege"));
    Grants->addColumn(tr("Type"));
    Grants->addColumn(tr("Grantable"));
    Grants->setRootIsDecorated(true);
    Tabs->addTab(Grants, tr("Privileges"));

    Version = new toResultLong(true, false, toQuery::Background, Tabs);
    Version->setSQL(SQLVersion);
    Tabs->addTab(Version, tr("Version"));

    Parameters = new toResultParam(Tabs);
    Tabs->addTab(Parameters, tr("Parameters"));

    Statistics = new toResultStats(false, Tabs);
    Tabs->addTab(Statistics, tr("Statistics"));

    ResourceLimit = new toResultLong(true, false, toQuery::Background, Tabs, "resource");
    ResourceLimit->setSQL(SQLResourceLimit);
    Tabs->addTab(ResourceLimit, tr("Resource Limits"));

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(windowActivated(QWidget *)),
            this, SLOT(windowActivated(QWidget *)));

    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));

    Query = NULL;
    refresh();

    setFocusProxy(Tabs);
}

void toCurrent::windowActivated(QWidget *widget)
{
    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QPopupMenu(this);
            ToolMenu->insertItem(QPixmap((const char **)refresh_xpm), tr("&Refresh"),
                                 this, SLOT(refresh(void)),
                                 toKeySequence(tr("F5", "Current session|Refresh")));
            toMainWidget()->menuBar()->insertItem(tr("&Current Session"), ToolMenu, -1, toToolMenuIndex());
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

toCurrent::~toCurrent()
{
    try
    {
        CurrentTool.closeWindow(connection());
    }
    TOCATCH
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

void toCurrent::addList(bool isrole, QListViewItem *parent, const QString &type, const toSQL &sql, const QString &role)
{
    Updates.insert(Updates.end(), update(isrole, parent, type, sql(connection()), role));
}

void toCurrent::poll()
{
    try
    {
        if (Query)
        {
            while (Query && Query->poll())
            {
                if (Query->eof())
                {
                    delete Query;
                    Query = NULL;
                }
                else
                {
                    QListViewItem *item;
                    if (CurrentUpdate.Parent)
                        item = new toResultViewItem(CurrentUpdate.Parent, NULL);
                    else
                        item = new toResultViewItem(Grants, NULL);
                    item->setText(0, Query->readValue());
                    item->setText(1, CurrentUpdate.Type);
                    item->setText(2, Query->readValue());
                    if (CurrentUpdate.IsRole)
                    {
                        addList(false, item, tr("System"), SQLRoleSysPrivs, item->text(0));
                        addList(false, item, tr("Object"), SQLRoleTabPrivs, item->text(0));
                        addList(true, item, tr("Role"), SQLRoleRolePrivs, item->text(0));
                    }
                }
            }
        }

        if (!Query)
        {
            if ( Updates.empty() )
            {
                Poll.stop();
                return ;
            }

            CurrentUpdate = toShift(Updates);

            toQList param;
            if (!CurrentUpdate.Role.isEmpty())
                toPush(param, toQValue(CurrentUpdate.Role));
            Query = new toNoBlockQuery(connection(),
                                       toQuery::Background,
                                       CurrentUpdate.SQL,
                                       param);
        }
    }
    catch (const QString &exc)
    {
        toStatusMessage(exc);
        delete Query;
        Query = NULL;
    }
}

void toCurrent::refresh()
{
    try
    {
        Parameters->refresh();
        Version->refresh();
        Statistics->refreshStats();
        Grants->clear();
        ResourceLimit->refresh();

        Updates.clear();
        delete Query;
        Query = NULL;

        addList(false, NULL, tr("System"), SQLUserSysPrivs);
        addList(false, NULL, tr("Object"), SQLUserTabPrivs);
        addList(true, NULL, tr("Role"), SQLUserRolePrivs);

        poll();
        Poll.start(100);
    }
    TOCATCH
}
