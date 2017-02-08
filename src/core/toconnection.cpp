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

#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "core/toconnectiontraits.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/tocache.h"
#include "core/toqvalue.h"
#include "core/toquery.h"
#include "widgets/totoolwidget.h"
#include "core/toconfiguration.h"
#include "core/toconnectionprovider.h"
#include "widgets/toworkspace.h"
#include "core/todatabaseconfig.h"

#include <QMenu>

toConnection::toConnection(const QString &provider,
                           const QString &user, const QString &password,
                           const QString &host, const QString &database,
                           const QString &schema,
                           const QString &color,
                           const QSet<QString> &options)
    : Abort(false)
    , Provider(provider)
    , User(user)
    , Password(password)
    , Host(host)
    , Database(database)
    , Version("0000")
    , Color(color)
    , Options(options)
    , pConnectionImpl(NULL)
    , pTrait(NULL)
    , ConnectionOptions(provider, host, database, user, password, schema, color , 0, options)
    , pCache(NULL)
    , LoanCnt(0)
{
    pConnectionImpl = toConnectionProviderRegistrySing::Instance().get(provider).createConnectionImpl(*this);
    pTrait = toConnectionProviderRegistrySing::Instance().get(provider).createConnectionTrait();

    toConnectionSub* connSub = addConnection();
    Version = connSub->version();
    Connections.insert(connSub);

    setDefaultSchema(schema);

    if(!ConnectionOptions.options.contains("TEST"))
    {
    	pCache = new toCache(*this, description(false).trimmed());
    	////Version = connSub->version();

        QMutexLocker clock(&ConnectionLock);
        if (toConfigurationNewSingle::Instance().option(ToConfiguration::Database::ObjectCacheInt).toInt() == toCache::ON_CONNECT)
            pCache->readCache();
    }
}

toConnection::toConnection(const toConnectionOptions &opts)
    : Abort(false)
    , Provider(opts.provider)
    , User(opts.username)
    , Password(opts.password)
    , Host(opts.host)
    , Database(opts.database)
    , Version("0000")
    , Color(opts.color)
    , Options(opts.options)
    , pConnectionImpl(NULL)
    , pTrait(NULL)
    , ConnectionOptions(opts)
    , pCache(NULL)
    , LoanCnt(0)
{
    pConnectionImpl = toConnectionProviderRegistrySing::Instance().get(Provider).createConnectionImpl(*this);
    pTrait = toConnectionProviderRegistrySing::Instance().get(Provider).createConnectionTrait();

    toConnectionSub* connSub = addConnection();
    Version = connSub->version();
    Connections.insert(connSub);

    setDefaultSchema(opts.schema);

    if(!ConnectionOptions.options.contains("TEST"))
    {
        pCache = new toCache(*this, description(false).trimmed());
        ////Version = connSub->version();

        QMutexLocker clock(&ConnectionLock);
        if (toConfigurationNewSingle::Instance().option(ToConfiguration::Database::ObjectCacheInt) == toCache::ON_CONNECT)
            pCache->readCache();
    }
}

toConnection::toConnection(const toConnection &other)
    : Abort(other.Abort)
    , Provider(other.Provider)
    , User(other.User)
    , Password(other.Password)
    , Host(other.Host)
    , Database(other.Database)
    , Schema(other.Schema)
    , Version(other.Version)
    , Color(other.Color)
    , Options(other.Options)
    , pConnectionImpl(NULL)
    , pTrait(other.pTrait)
    , ConnectionOptions(other.ConnectionOptions)
    , pCache(NULL)
    , LoanCnt(0)
{
    //tool Connection = toConnectionProvider::connection(Provider, this);
    //ConnectionPool = new toConnectionPool(this);

    //PoolPtr sub(ConnectionPool);
    //Version = Connection->version(*sub);

    setDefaultSchema(other.Schema);

    {
        QWriteLocker lock(&pCache->cacheLock);
        pCache->refCount.fetchAndAddAcquire(1);
    }
}

toConnectionSub* toConnection::addConnection()
{
    Utils::toBusy busy;
    toConnectionSub *sub = pConnectionImpl->createConnection();
    return sub;
}


void toConnection::closeConnection(toConnectionSub *sub)
{
    if (Connections.contains(sub))
    {
        sub->close();
        Connections.remove(sub);
//    } else if (LentConnections.contains(sub)) {
//    	sub->cancel();
    }
    else
        throw exception("Can not close non-existing toConnectionSub");
}

QList<QString> toConnection::running(void) const
{
    Utils::toBusy busy;
    QMutexLocker lock(&ConnectionLock);
    QList<QString> ret;
    Q_FOREACH(toConnectionSub *conn, LentConnections)
    {
        static QString sql("Session: %1\n%2\n");
        toQueryAbstr *query = conn->query();
        ret << sql.arg(conn->sessionId().first()).arg(query ? query->sql() : QString("None"));
    }
    return ret;
}

//std::list<QString> toConnection::primaryKeys(){
//  return Connection->primaryKeys();
//}

void toConnection::cancelAll(void)
{
    throw exception("void toConnection::cancelAll Not implemented yet");
    //conn ConnectionPool->cancelAll();
}

toConnection::~toConnection()
{
    Utils::toBusy busy;
    Abort = true;

#if QT_VERSION < 0x050000
    Q_ASSERT_X( (int)LoanCnt == 0 , qPrintable(__QHERE__), "toConnection deleted while BG query is running");
#else
    Q_ASSERT_X( LoanCnt.loadAcquire() == 0 , qPrintable(__QHERE__), "toConnection deleted while BG query is running");
#endif

    if(pCache)
    {
    	unsigned cacheNewRefCnt;
    	{
    		QWriteLocker clock(&getCache().cacheLock);
    		cacheNewRefCnt = getCache().refCount.deref();
    	}
    	if (cacheNewRefCnt == 0)
    	{
    		getCache().writeDiskCache();
    		delete this->pCache;
    	}
    }
    {
        closeWidgets();

        //ConnectionPool->cancelAll(true);
        //delete ConnectionPool;
        //ConnectionPool = 0;

        QMutexLocker lock(&ConnectionLock);
    }
    delete pConnectionImpl;
}

void toConnection::commit(toConnectionSub *sub)
{
    Utils::toBusy busy;
    sub->commit();
}

void toConnection::commit(void)
{
    QMutexLocker clock(&ConnectionLock);
#if QT_VERSION < 0x050000
    if ((int)LoanCnt)
#else
    if (LoanCnt.loadAcquire())
#endif
        throw qApp->translate("toConnection::commit", "Couldn't commit, while connection is active");

    Q_FOREACH(toConnectionSub *conn, Connections)
    {
        conn->commit();
    }
}

void toConnection::rollback(toConnectionSub *sub)
{
    Utils::toBusy busy;
    sub->rollback();
}

void toConnection::delWidget(QWidget *widget)
{
    Widgets.remove(widget);

#ifdef QT_DEBUG
    // Cross check tools connections against connections widgets
    // Iterate over all the tool windows, increment the counter for every tool using this toConnection instance
    QSet<QWidget*>::size_type toolCnt = 0;
    QList<toToolWidget*> tools = toWorkSpaceSingle::Instance().toolWindowList();
    Q_FOREACH(toToolWidget *tool, tools)
    {
        if (this->ConnectionOptions == tool->connection().connectionOptions())
            toolCnt++;
    }
    Q_ASSERT_X(toolCnt == Widgets.size(), qPrintable(__QHERE__), "Widgets.size() != toolCnt");
#endif
}

void toConnection::addWidget(QWidget *widget)
{
    Widgets.insert(widget);

#ifdef QT_DEBUG
    // Cross check tools connections against connections widgets
    // Iterate over all the tool windows, increment the counter for every tool using this instance
    QSet<QWidget*>::size_type toolCnt = 0;
    QList<toToolWidget*> tools = toWorkSpaceSingle::Instance().toolWindowList();
    Q_FOREACH(toToolWidget *tool, tools)
    {
        if (this->ConnectionOptions == tool->connection().connectionOptions())
            toolCnt++;
    }
    // +1:  This widget was not yet regitered with toWorkSpaceSingle(has no parent). (See: toTool::createWindow())
    if (widget->parent() == NULL)
        toolCnt++;
    Q_ASSERT_X(toolCnt == Widgets.size(), qPrintable(__QHERE__), "Widgets.size() != toolCnt");
#endif
}

bool toConnection::closeWidgets(void)
{
    //for (QList<QPointer<QWidget> >::iterator i = Widgets.begin(); i != Widgets.end();  i++)
    Q_FOREACH(QWidget* widget, Widgets)
    {
        if (toToolWidget *toolWidget = dynamic_cast<toToolWidget*>(widget))
        {
            bool retval = toWorkSpaceSingle::Instance().closeToolWidget(toolWidget);
            if (retval)
            {
                bool removed = Widgets.remove(widget); // TODO: check if QT containers support modification while being iterated
                continue;
            }
            else
                return false;
        }
        Q_ASSERT_X(false, qPrintable(__QHERE__), qPrintable(QString("Unknown toConnectionWidget: %1").arg(widget->metaObject()->className())));
    }
    Q_ASSERT_X(Widgets.isEmpty(), qPrintable(__QHERE__), "toConnection::Widgets is not empty");
    return true;
}

void toConnection::connectionsMenu(QMenu *menu)
{
    menu->clear();
    //menu->disconnect(this);
    Q_FOREACH(QAction *a, ConnectionActions)
    {
        bool removed = ConnectionActions.remove(a);
        delete a;
    }

    QMutexLocker clock(&ConnectionLock);
    QMenu *active = menu->addMenu(tr("Active connections"));
    active->setDisabled(true);
    Q_FOREACH(toConnectionSub* conn, LentConnections)
    {
        QMenu *sess = menu->addMenu(conn->sessionId().first()+" "+conn->lastSql());
        QAction *cancel = new QAction("Cancel", this);
        cancel->setData(VPtr<toConnectionSub>::asQVariant(conn));
        sess->addAction(cancel);
        ConnectionActions.insert(cancel);
        connect(sess, SIGNAL(triggered(QAction *)), this, SLOT(commandCallback(QAction *)));
    }
    menu->addSeparator();
    QMenu *inactive = menu->addMenu(tr("Inactive connections"));
    inactive->setDisabled(true);
    Q_FOREACH(toConnectionSub* conn, Connections)
    {
        QMenu *sess = menu->addMenu(conn->sessionId().first()+" "+conn->lastSql());
        QAction *close = new QAction("Close", this);
        close->setData(VPtr<toConnectionSub>::asQVariant(conn));
        sess->addAction(close);
        ConnectionActions.insert(close);
        connect(sess, SIGNAL(triggered(QAction *)), this, SLOT(commandCallback(QAction *)));
    }
}

void toConnection::commandCallback(QAction *act)
{
	try {
		QString actionStr = act->text();
		toConnectionSub *conn = VPtr<toConnectionSub>::asPtr(act->data());
		if ( LentConnections.contains(conn))
		{
			conn->cancel();
		}
		else if ( Connections.contains(conn))
		{
			closeConnection(conn);
		}
		else
		{
			Q_ASSERT_X(false, qPrintable(__QHERE__), "Invalid QAction pointing onto unknown toConnectionSub");
		}
	} catch (toConnection::exception const& e) {
		Utils::toStatusMessage(e);
	}
}

QString toConnection::description(bool version) const
{
    QString ret(User);
    ret += QString::fromLatin1("@");

    if (!providerIs("QMYSQL"))
        ret += Database;

    if (!Host.isEmpty() && Host != "SQL*Net")
    {
        if (!providerIs("QMYSQL"))
            ret += QString::fromLatin1(".");
        ret += Host;
    }

    if (version)
    {
        if (!Version.isEmpty())
        {
            ret += QString::fromLatin1(" [");
            ret += Version;
            ret += QString::fromLatin1("]");
        }
    }
    return ret;
}

/** Set connection's current schema. */
void toConnection::setDefaultSchema(QString const & schema)
{
    Schema = schema;
}

void toConnection::setInit(const QString &key, const QString &sql)
{
    InitStrings.insert(key, sql);
}

void toConnection::delInit(const QString &key)
{
    InitStrings.remove(key);
}

QList<QString> toConnection::initStrings() const
{
    return InitStrings.values();
}

toConnection& toConnection::currentConnection(QObject *cur)
{
    while (cur)
    {
        toConnectionWidget *conn = dynamic_cast<toConnectionWidget *>(cur);
        if (conn)
            return conn->connection();
        cur = cur->parent();
    }
    throw qApp->translate("toConnection::currentConnection", "Couldn't find parent connection. Internal error.");
}

toConnectionSub* toConnection::borrowSub()
{
    QMutexLocker clock(&ConnectionLock);
    if (!Connections.empty())
    {
        toConnectionSub* retval = *(Connections.begin());
        Connections.remove(retval);
        LentConnections.insert(retval);
        LoanCnt.fetchAndAddAcquire(1);
#if QT_VERSION < 0x050000
        Q_ASSERT_X((int)LoanCnt == LentConnections.size(), qPrintable(__QHERE__), "Invalid number of lent toConnectionSub(s)");
#else
        Q_ASSERT_X(LoanCnt.loadAcquire() == LentConnections.size(), qPrintable(__QHERE__), "Invalid number of lent toConnectionSub(s)");
#endif
        return retval;
    }
    else
    {
        toConnectionSub* retval = addConnection();
        LoanCnt.fetchAndAddAcquire(1);
        LentConnections.insert(retval);
#if QT_VERSION < 0x050000
        Q_ASSERT_X((int)LoanCnt == LentConnections.size(), qPrintable(__QHERE__), "Invalid number of lent toConnectionSub(s)");
#else
        Q_ASSERT_X(LoanCnt.loadAcquire() == LentConnections.size(), qPrintable(__QHERE__), "Invalid number of lent toConnectionSub(s)");
#endif
        return retval;
    }
}

void toConnection::putBackSub(toConnectionSub *conn)
{
    QMutexLocker clock(&ConnectionLock);
    Q_ASSERT_X( !Connections.contains(conn) , qPrintable(__QHERE__), "Invalid use of toConnectionSubLoan");
    LoanCnt.deref();

    try
    {
        if (conn->hasTransaction())
            conn->rollback();
    }
    TOCATCH

    if (conn->isBroken())
    {
        delete conn;
    }
    else if (Connections.size() >= toConfigurationNewSingle::Instance().option(ToConfiguration::Database::CachedConnectionsInt).toInt())
    {
        delete conn;
    }
    else
        Connections.insert(conn);
    bool removed = LentConnections.remove(conn);
    Q_ASSERT_X(removed, qPrintable(__QHERE__), "Lent connection not found");
#if QT_VERSION < 0x050000
    Q_ASSERT_X((int)LoanCnt == LentConnections.size(), qPrintable(__QHERE__), "Invalid number of lent toConnectionSub(s)");
#else
    Q_ASSERT_X(LoanCnt.loadAcquire() == LentConnections.size(), qPrintable(__QHERE__), "Invalid number of lent toConnectionSub(s)");
#endif
}

void toConnection::allExecute(QString const& sql)
{
#if 0
    try
    {
        Q_FOREACH(toConnectionSub *con, Connections)
        {
            //TODO
            toQuery q(*con, sql, toQueryParams());
            q.eof();
        }
    }
    TOCATCH
#endif
}
