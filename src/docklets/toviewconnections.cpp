
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"
#include "toviewconnections.h"
#include "tomain.h"
#include "toconnectionmodel.h"

#include <QHeaderView>

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


void toViewConnections::handleActivated(const QModelIndex &index) {
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

    if(provider == toNewConnection::ORACLE_INSTANT)
    {
        // create the rest of the connect string. this will work
        // without an ORACLE_HOME.

        database = "//" + host + "/" + database;
        host = "";
    }

    if(opt.provider.startsWith("Oracle"))
        provider = "Oracle";

    // checks for existing connection
    std::list<QString> con = toMainWidget()->connections();
    for (std::list<QString>::iterator i = con.begin();i != con.end();i++)
    {
        try
        {
            toConnection &conn = toMainWidget()->connection(*i);

//             qDebug() << "user" << conn.user() << opt.username;
//             qDebug() << "prov" << conn.provider() << provider;
//             qDebug() << "host" << conn.host() << host;
//             qDebug() << "db" << conn.database() << database;
//             qDebug() << "schema" << conn.schema() << opt.schema;

            if(conn.user() == opt.username &&
               conn.provider() == provider &&
               conn.host() == host &&
               conn.database() == database &&
               (opt.schema.isEmpty() || (conn.schema() == opt.schema)))
            {
                toMainWidget()->createDefault();
                emit activated();
                return;
            }
        }
        catch (...) {}
    }

    try {
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
            toMainWidget()->addConnection(retCon, true);
        emit activated();
    }
    TOCATCH;
}
