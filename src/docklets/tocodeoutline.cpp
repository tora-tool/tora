
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

#include "core/utils.h"
#include "docklets/tocodeoutline.h"
#include "core/tologger.h"

#include "parsing/tsqlparse.h"

//#include "tomain.h"
//#include "toconnectionmodel.h"

#include <QtCore/QTimerEvent>
#include <QListWidget>
//#include <QListView>
//#include <QHeaderView>

REGISTER_VIEW("Outline", toCodeOutline);

toCodeOutline::toCodeOutline(QWidget *parent,
                             toWFlags flags)
    : toDocklet(tr("Outline"), parent, flags)
    , m_currentEditor(new editHandlerHolder())
    , m_timerID(-1)
{
    setObjectName("Code Outline");

    TabWidget = new QTabWidget(this);

    procedures = new QListWidget();
    TabWidget->addTab(procedures, "Procedures");

    functions = new QListWidget();
    TabWidget->addTab(functions, "Functions");

    cursors = new QListWidget();
    TabWidget->addTab(cursors, "Cursors");

    types = new QListWidget();
    TabWidget->addTab(types, "Types");

    exceptions = new QListWidget();
    TabWidget->addTab(exceptions, "Exceptions");

    //setFocusProxy(TableView); // TODO ?? What is this??
    setFocusProxy(TabWidget);

    //Model = toNewConnection::proxyModel();
    //toNewConnection::connectionModel()->readConfig();
    //TableView->setModel(Model);

    //TableView->horizontalHeader()->setStretchLastSection(true);
    //   TableView->horizontalHeader()->setHighlightSections(false);
    //   TableView->verticalHeader()->setVisible(false);
    //   TableView->hideColumn(0);
    //   TableView->hideColumn(1);
    //   TableView->hideColumn(5);

    //   TableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    //   TableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    //   TableView->setAlternatingRowColors(true);

    //   connect(TableView,
    //           SIGNAL(activated(const QModelIndex &)),
    //           this,
    //           SLOT(handleActivated(const QModelIndex &)));

    setWidget(TabWidget);

    m_timerID = startTimer(5000);
    TLOG(0, toDecorator, __HERE__) << "void toQueryModel::timerEvent(QTimerEvent *e) fired" << std::endl;
}

void toCodeOutline::timerEvent(QTimerEvent *e)
{
    QString newText;

    //TLOG(0,toDecorator,__HERE__) << "void toQueryModel::timerEvent(QTimerEvent *e) fired" << std::endl;

    if ( m_timerID != e->timerId())
    {
        toDocklet::timerEvent(e);
        return;
    }

    if ( m_currentEditor->m_current == NULL)
        return;

    newText = m_currentEditor->m_current->editText();
    if ( newText == m_lastText)
        return;

    m_lastText = newText;
    TLOG(0, toDecorator, __HERE__) << "New text: " << std::endl << m_lastText  << std::endl;

    try
    {
        std::auto_ptr <SQLParser::Statement> stat = StatementFactTwoParmSing::Instance().create("OraclePLSQL", m_lastText, "");
        TLOG(0, toDecorator, __HERE__) << "Parsing ok:" << std::endl
                                       << stat->root()->toStringRecursive().toStdString() << std::endl;

        procedures->clear();
        functions->clear();
        cursors->clear();
        types->clear();
        exceptions->clear();

        QMap<QString, const SQLParser::Token*>::const_iterator i = stat->declarations().begin();
        for (; i != stat->declarations().end(); ++i)
        {
            TLOG(0, toDecorator, __HERE__) << i.key() << ' ' << i.value()->getPosition().toString() << std::endl;
            QListWidgetItem *wi = new QListWidgetItem(i.key());
            wi->setToolTip(i.value()->getPosition().toString());

            SQLParser::Token const &node = *i.value();
            if (node.getTokenUsageType() == SQLParser::Token::Declaration)
            {
                switch (node.getTokenType())
                {
                    case SQLParser::Token::L_DATATYPE:
                        types->addItem(wi);
                        break;
                    case SQLParser::Token::L_FUNCTIONNAME:
                        functions->addItem(wi);
                        break;
                    case SQLParser::Token::L_PROCEDURENAME:
                        procedures->addItem(wi);
                        break;
                    case SQLParser::Token::L_CURSORNAME:
                        cursors->addItem(wi);
                        break;
                    case SQLParser::Token::L_EXCEPTIONNAME:
                        exceptions->addItem(wi);
                        break;
                }
            }
        }
    }
    catch ( SQLParser::ParseException const &e)
    {

    }
}

QIcon toCodeOutline::icon() const
{
    return QIcon(":/icons/connect.xpm");
}


QString toCodeOutline::name() const
{
    return tr("Outline");
}


void toCodeOutline::handleActivated(const QModelIndex &index)
{
    if (!index.isValid())
        return;

//    QModelIndex baseIndex = toNewConnection::proxyModel()->index(index.row(), 0);
//    int ind = toNewConnection::proxyModel()->data(baseIndex, Qt::DisplayRole).toInt();
//    toConnectionOptions opt = toNewConnection::connectionModel()->availableConnection(ind);
//
//    QString database = opt.database;
//    QString host     = opt.host;
//    QString provider = opt.provider;
//
//    if(opt.port)
//        host += ":" + QString::number(opt.port);
//
//    if(provider == toNewConnection::ORACLE_INSTANT)
//    {
//        // create the rest of the connect string. this will work
//        // without an ORACLE_HOME.
//
//        database = "//" + host + "/" + database;
//        host = "";
//    }
//
//    if(opt.provider.startsWith("Oracle"))
//        provider = "Oracle";
//
//    // checks for existing connection
//    std::list<QString> con = toMainWidget()->connections();
//    for (std::list<QString>::iterator i = con.begin();i != con.end();i++)
//    {
//        try
//        {
//            toConnection &conn = toMainWidget()->connection(*i);
//
////             qDebug() << "user" << conn.user() << opt.username;
////             qDebug() << "prov" << conn.provider() << provider;
////             qDebug() << "host" << conn.host() << host;
////             qDebug() << "db" << conn.database() << database;
////             qDebug() << "schema" << conn.schema() << opt.schema;
//
//            if(conn.user() == opt.username &&
//               conn.provider() == provider &&
//               conn.host() == host &&
//               conn.database() == database &&
//               (opt.schema.isEmpty() || (conn.schema() == opt.schema)))
//            {
//                toMainWidget()->createDefault();
//                emit activated();
//                return;
//            }
//        }
//        catch (...) {}
//    }
//
//    try {
//        toConnection *retCon = new toConnection(
//            provider,
//            opt.username,
//            opt.password,
//            host,
//            database,
//            opt.schema,
//            opt.color,
//            opt.options);
//
//        if(retCon)
//            toMainWidget()->addConnection(retCon, true);
//        emit activated();
//    }
//    TOCATCH;
}
