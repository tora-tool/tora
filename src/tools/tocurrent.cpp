
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "tools/tocurrent.h"
#include "core/utils.h"
#include "core/tochangeconnection.h"
#include "core/toconnection.h"
#include "core/tomainwindow.h"
#include "core/toeventquery.h"
#include "core/toeventquery.h"
#include "core/tosql.h"
#include "core/totool.h"

#include <QLabel>
#include <QMenuBar>
#include <QTabWidget>
#include <QToolBar>
#include <QtGui/QPixmap>
#include <QLayout>

#include "icons/refresh.xpm"
#include "icons/tocurrent.xpm"

#include "toresultparam.h"
#include "toresultstats.h"
#include "toresulttableview.h"
#include "toresultview.h"

class toCurrentInfoTool : public toTool
{
    protected:
        std::map<toConnection *, QWidget *> Windows;

        const char **pictureXPM(void) override
        {
            return const_cast<const char**>(tocurrent_xpm);
        }
    public:
        toCurrentInfoTool()
            : toTool(240, "Current Session")
        { }

        const char *menuItem() override
        {
            return "Current Session";
        }

        toToolWidget* toolWindow(QWidget *parent, toConnection &connection) override
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
                toToolWidget* window = new toCurrent(parent, connection);
                Windows[&connection] = window;
                return window;
            }
        }

        void closeWindow(toConnection &connection) override
        {
            std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
            if (i != Windows.end())
                Windows.erase(i);
        }

        bool canHandle(const toConnection &conn) override
        {
            return conn.providerIs("Oracle");
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
    : toToolWidget(CurrentTool, "current.html", main, connection, "toCurrent")
{
    QToolBar *toolbar = Utils::toAllocBar(this, tr("Current Session"));
    layout()->addWidget(toolbar);

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                       tr("Update"),
                       this,
                       SLOT(slotRefresh(void)));

    toolbar->addWidget(new Utils::toSpacer());

    new toChangeConnection(toolbar);

    Tabs = new QTabWidget(this);
    layout()->addWidget(Tabs);

    Grants = new toListView(Tabs);
    Grants->setSortingEnabled(false);
    Grants->addColumn(tr("Privilege"));
    Grants->addColumn(tr("Type"));
    Grants->addColumn(tr("Grantable"));
    Grants->setRootIsDecorated(true);
    Tabs->addTab(Grants, tr("Privileges"));

    Version = new toResultTableView(true, false, Tabs);
    Version->setSQL(SQLVersion);
    Tabs->addTab(Version, tr("Version"));

    Parameters = new toResultParam(Tabs);
    Tabs->addTab(Parameters, tr("Parameters"));

    Statistics = new toResultStats(false, Tabs);
    Tabs->addTab(Statistics, tr("Statistics"));

    ResourceLimit = new toResultTableView(true, false, Tabs, "resource");
    ResourceLimit->setSQL(SQLResourceLimit);
    Tabs->addTab(ResourceLimit, tr("Resource Limits"));

    Query = NULL;
    slotRefresh();

    setFocusProxy(Tabs);
}

toCurrent::~toCurrent()
{
}

void toCurrent::closeEvent(QCloseEvent *event)
{
    try
    {
        CurrentTool.closeWindow(connection());
    }
    TOCATCH;

    event->accept();
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

void toCurrent::addList(bool isrole, toTreeWidgetItem *parent, const QString &type, const toSQL &sql, const QString &role)
{
    Updates.append(update(isrole, parent, type, sql(connection()), role));
}

void toCurrent::processUpdate()
{
    if (Updates.empty())
        return;

    CurrentUpdate = Updates.takeFirst();

    toQueryParams param;
    if (!CurrentUpdate.Role.isEmpty())
        param << toQValue(CurrentUpdate.Role);
    Query = new toEventQuery(this
                             , connection()
                             , CurrentUpdate.SQL
                             , param
                             , toEventQuery::READ_ALL);
    connect(Query, &toEventQuery::dataAvailable, this, &toCurrent::receiveData);
    connect(Query, &toEventQuery::done, this, &toCurrent::queryDone);
    Query->start();
} // processUpdate

void toCurrent::receiveData(toEventQuery*)
{
    try
    {
        while (Query->hasMore())
        {
            toTreeWidgetItem *item;
            if (CurrentUpdate.Parent)
                item = new toResultViewItem(CurrentUpdate.Parent, NULL);
            else
                item = new toResultViewItem(Grants, NULL);
            item->setText(0, (QString)Query->readValue());
            item->setText(1, CurrentUpdate.Type);
            item->setText(2, (QString)Query->readValue());
            if (CurrentUpdate.IsRole)
            {
                // Add details of this role to the query que
                addList(false, item, tr("System"), SQLRoleSysPrivs, item->text(0));
                addList(false, item, tr("Object"), SQLRoleTabPrivs, item->text(0));
                addList(true, item, tr("Role"), SQLRoleRolePrivs, item->text(0));
            }
        }
    }
    catch (const QString &exc)
    {
        Utils::toStatusMessage(exc);
        delete Query;
        Query = NULL;
    }
}

void toCurrent::queryDone()
{
    delete Query;
    Query = NULL;
    processUpdate(); // done with this query, so process the next one!
    Grants->resizeColumnsToContents();
} // queryDone

void toCurrent::slotRefresh()
{
    try
    {
        Parameters->refresh();
        Version->refresh();
        Statistics->slotRefreshStats();
        Grants->clear();
        ResourceLimit->refresh();

        Updates.clear();
        delete Query;
        Query = NULL;

        addList(false, NULL, tr("System"), SQLUserSysPrivs);
        addList(false, NULL, tr("Object"), SQLUserTabPrivs);
        addList(true, NULL, tr("Role"), SQLUserRolePrivs);

        processUpdate();
    }
    TOCATCH
}
