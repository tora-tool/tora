
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

#include "tools/totemporary.h"
#include "core/utils.h"
#include "core/totool.h"
#include "core/tochangeconnection.h"
#include "core/toconnection.h"
#include "tools/tosgastatement.h"
#include "core/toglobalevent.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"

#include <QToolBar>
#include <QLayout>
#include <QSplitter>

#include "icons/refresh.xpm"
#include "icons/totemporary.xpm"
#include "toresulttableview.h"

static toSQL SQLListTemporaryObjects("toTemporary:ListTemporaryObjects",
                                      "SELECT s.sid || ',' || s.serial# \"Session\",\n"
                                      "       s.username \"User\",\n"
                                      "       u.TABLESPACE \"Tablespace\",\n"
                                      "       u.segtype \"Type\",\n"
                                      "       substr ( a.sql_text,1,150 ) \"SQL\",\n"
                                      "       round ( u.blocks * p.value / :siz<int>,2 )||:sizstr<char[50]> \"Size\",\n"
                                      "       s.sql_address || ':' || s.sql_hash_value \" address\",\n"
                                      "       s.sql_id \" SQL_ID\",\n"
                                      "       s.sql_child_number \" SQL_CHILD_NUMBER\"\n"
                                      "  FROM v$tempseg_usage u,\n"
                                      "       v$session s,\n"
                                      "       v$sqlarea a,\n"
                                      "       v$parameter p\n"
                                      " WHERE s.saddr = u.session_addr\n"
                                      "   AND a.address ( + ) = s.sql_address\n"
                                      "   AND a.hash_value ( + ) = s.sql_hash_value\n"
                                      "   AND p.name = 'db_block_size'",
                                      "Get temporary usage",
                                      "0900");

class toTemporaryTool : public toTool
{
        const char **pictureXPM(void) override
        {
            return const_cast<const char**>(totemporary_xpm);
        }
    public:
        toTemporaryTool()
            : toTool(130, "Temporary Objects")
        { }
        const char *menuItem() override
        {
            return "Temporary Objects";
        }
        toToolWidget* toolWindow(QWidget *parent, toConnection &connection) override
        {
            return new toTemporary(parent, connection);
        }
        bool canHandle(const toConnection &conn) override
        {
            if (conn.providerIs("Oracle") && conn.version() >= "0800")
                return true;
            return false;
        }
        void closeWindow(toConnection &connection) override {};
};

static toTemporaryTool TemporaryTool;

toTemporary::toTemporary(QWidget *main, toConnection &connection)
    : toToolWidget(TemporaryTool, "temporary.html", main, connection, "toTemporary")
    , ToolMenu(NULL)
{
    QToolBar *toolbar = Utils::toAllocBar(this, tr("Temporary Objects"));
    layout()->addWidget(toolbar);

    refreshAct =
        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                           tr("Refresh list"),
                           this,
                           SLOT(refresh()));

    toolbar->addWidget(new Utils::toSpacer());

    new toChangeConnection(toolbar);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(splitter);

    Objects  = new toResultTableView(true, false, splitter, "columnView");
    Objects->setSQL(SQLListTemporaryObjects);
    Objects->setReadAll(true);
    Objects->setSelectionBehavior(QAbstractItemView::SelectRows);

    QList<int> list;
    list.append(75);
    splitter->setSizes(list);

    QString unit(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::SizeUnit).toString());
    toQueryParams args = toQueryParams() << toQValue(Utils::toSizeDecode(unit)) << toQValue(unit);

    Objects->refreshWithParams(args);
    connect(Objects, SIGNAL(selectionChanged()), this, SLOT(changeItem()));

    Statement = new toSGAStatement(splitter);

    setFocusProxy(Objects);
}


void toTemporary::slotWindowActivated(toToolWidget *widget)
{
    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Temporary"), this);
            ToolMenu->addAction(refreshAct);

            toGlobalEventSingle::Instance().addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toTemporary::refresh(void)
{
    Objects->refresh();
}

void toTemporary::changeItem()
{
	QModelIndex selection1 = Objects->selectedIndex(8);
	QString sql_id = Objects->model()->data(selection1, Qt::EditRole).toString();
	QModelIndex selection2 = Objects->selectedIndex(9);
	QString child_id = Objects->model()->data(selection2, Qt::EditRole).toString();
	if (sql_id.isEmpty() || child_id.isEmpty())
		return;
	Statement->changeAddress(toQueryParams() << sql_id << child_id);
}
