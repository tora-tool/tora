
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

#include "docklets/toviewconnections.h"
#include "core/toconnectionmodel.h"
#include "core/tonewconnection.h"
#include "core/toconnection.h"
#include "core/toconnectionregistry.h"
#include "core/tooracleconst.h"
#include "core/utils.h"
#include "core/toglobalevent.h"

#include <QtGui/QHeaderView>
#include <QtGui/QTableView>
#include <QtGui/QSortFilterProxyModel>

REGISTER_VIEW("Connection", toViewConnections);

toViewConnections::toViewConnections(QWidget *parent,
                                     Qt::WindowFlags flags)
    : toDocklet(tr("Connections"), parent, flags)
{
    setObjectName("Connections Docklet");

    TableView = new QTableView(this);
    setFocusProxy(TableView);

    Model = toNewConnection::proxyModel();
    toNewConnection::connectionModel()->readConfig();
    TableView->setModel(Model);

    TableView->horizontalHeader()->setStretchLastSection(true);
    TableView->horizontalHeader()->setHighlightSections(false);
    TableView->verticalHeader()->setVisible(false);
    TableView->hideColumn(0);
    TableView->hideColumn(1);
    TableView->hideColumn(5);

    TableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    TableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    TableView->setAlternatingRowColors(true);

    connect(TableView,
            SIGNAL(activated(const QModelIndex &)),
            this,
            SLOT(handleActivated(const QModelIndex &)));

    setWidget(TableView);
}


QIcon toViewConnections::icon() const
{
    return QIcon(":/icons/connect.xpm");
}


QString toViewConnections::name() const
{
    return tr("Connections");
}


void toViewConnections::handleActivated(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    QModelIndex baseIndex = toNewConnection::proxyModel()->index(index.row(), 0);
    int ind = toNewConnection::proxyModel()->data(baseIndex, Qt::DisplayRole).toInt();
    toConnectionOptions opt = toNewConnection::connectionModel()->availableConnection(ind);

    QString database = opt.database;
    QString host     = opt.host;
    QString provider = opt.provider;

    if(opt.port)
        host += ":" + QString::number(opt.port);

    if(provider == ORACLE_INSTANTCLIENT)
    {
        // create the rest of the connect string. this will work
        // without an ORACLE_HOME.

        database = "//" + host + "/" + database;
        host = "";
    }

    if(opt.provider.startsWith("Oracle"))
        provider = "Oracle";

    // checks for existing connection
    QList<toConnection*> const& connections = toConnectionRegistrySing::Instance().connections();
    foreach(toConnection * conn, connections)
    {
        try
        {
            if(conn->user() == opt.username &&
                    conn->provider() == provider &&
                    conn->host() == host &&
                    conn->database() == database &&
                    (opt.schema.isEmpty() || (conn->defaultSchema() == opt.schema)))
            {
		toGlobalEventSingle::Instance().createDefaultTool();
                emit activated();
                return;
            }
        }
        catch (...) {}
    }

    try
    {
        toConnection *retCon = new toConnection(
            provider,
            opt.username,
            opt.password,
            host,
            database,
            opt.schema,
            opt.color,
            opt.options);

        if(retCon)
            toGlobalEventSingle::Instance().addConnection(retCon, true);
        emit activated();
    }
    TOCATCH;
}
