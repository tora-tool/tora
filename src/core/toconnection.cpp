
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "core/toconnectiontraits.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconf.h"
#include "core/tocache.h"
#include "core/toqvalue.h"
#include "core/toquery.h"
#include "core/totool.h"
#include "core/toconfiguration.h"
#include "core/toconnectionprovider.h"
#include "core/toworkspace.h"

#include <QtGui/QMenu>

toConnection::toConnection(const QString &provider,
                           const QString &user, const QString &password,
                           const QString &host, const QString &database,
                           const QString &schema,
                           const QString &color,
                           const QSet<QString> &options)
    : Provider(provider)
    , User(user)
    , Password(password)
    , Host(host)
    , Database(database)
    , Color(color)
    , Options(options)
    , pConnectionImpl(NULL)
    , pCache(NULL)
    , pTrait(NULL)
    , NeedCommit(false)
    , Abort(false)
    , LoanCnt(0)
	, ConnectionOptions(provider, host, database, user, password, schema, color , 0, options)
{
    pConnectionImpl = toConnectionProviderRegistrySing::Instance().get(provider).createConnectionImpl(*this);
    pTrait = toConnectionProviderRegistrySing::Instance().get(provider).createConnectionTrait();

    toConnectionSub* connSub = addConnection();
    Version = connSub->version();
    Connections.insert(connSub);

    setSchema(schema);

    pCache = new toCache(*this, description(false).trimmed());

    ////Version = connSub->version();
    {
        QMutexLocker clock(&ConnectionLock);
        if (toConfigurationSingle::Instance().objectCache() == toConfiguration::ON_CONNECT)
        	pCache->readCache();

    }
}

toConnection::toConnection(const toConnectionOptions &opts)
    : Provider(opts.provider)
    , User(opts.username)
    , Password(opts.password)
    , Host(opts.host)
    , Database(opts.database)
    , Color(opts.color)
    , Options(opts.options)
    , pConnectionImpl(NULL)
    , pCache(NULL)
    , pTrait(NULL)
    , NeedCommit(false)
    , Abort(false)
    , LoanCnt(0)
	, ConnectionOptions(opts)
{
    pConnectionImpl = toConnectionProviderRegistrySing::Instance().get(Provider).createConnectionImpl(*this);
    pTrait = toConnectionProviderRegistrySing::Instance().get(Provider).createConnectionTrait();

    toConnectionSub* connSub = addConnection();
    Version = connSub->version();
    Connections.insert(connSub);

    setSchema(opts.schema);

    pCache = new toCache(*this, description(false).trimmed());

    ////Version = connSub->version();
    {
        QMutexLocker clock(&ConnectionLock);
        if (toConfigurationSingle::Instance().objectCache() == toConfiguration::ON_CONNECT)
        	pCache->readCache();

    }
}

toConnection::toConnection(const toConnection &other)
    : Provider(other.Provider)
    , User(other.User)
    , Password(other.Password)
    , Host(other.Host)
    , Database(other.Database)
	, Version(other.Version)
    , Schema(other.Schema)
    , Color(other.Color)
    , Options(other.Options)
    , pConnectionImpl(NULL)
    , pCache(NULL)
    , pTrait(other.pTrait)
    , NeedCommit(other.NeedCommit)
    , Abort(other.Abort)
    , LoanCnt(0)
	, ConnectionOptions(other.ConnectionOptions)
{
    //tool Connection = toConnectionProvider::connection(Provider, this);
    //ConnectionPool = new toConnectionPool(this);

    //PoolPtr sub(ConnectionPool);
    //Version = Connection->version(*sub);

    setSchema(other.Schema);

    {
    	QWriteLocker lock(&pCache->cacheLock);
        pCache->refCount.fetchAndAddAcquire(1);
    }
}

toConnectionSub* toConnection::addConnection()
{
    Utils::toBusy busy;
    toConnectionSub *sub = pConnectionImpl->createConnection();
    //QMutexLocker lock(ConnectionLock); already locked by caller

    toQueryParams params;
    foreach (QString const & sql, InitStrings)
    {
        try
        {
    		//toQuery q(sub, sql, toQueryParams());
    		//q.eof();
        }
        TOCATCH
    }

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
    } else
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
    	toQuery *query = conn->query();
    	ret << sql.arg(conn->sessionId()).arg(query ? query->sql() : QString("None"));
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

    Q_ASSERT_X( (int)LoanCnt == 0 , qPrintable(__QHERE__), "toConnection deleted while BG query is running");

    unsigned cacheNewRefCnt;
    {
    	QWriteLocker clock(&getCache().cacheLock);
        cacheNewRefCnt = getCache().refCount.deref();
    }
    if(cacheNewRefCnt == 0)
    {
		getCache().writeDiskCache();
        delete this->pCache;
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
    if((int)LoanCnt)
    	throw qApp->translate("toConnection::commit", "Couldn't commit, while connection is active");

    Q_FOREACH(toConnectionSub *conn, Connections)
    {
    	conn->commit();
    }
    setNeedCommit(false);
}

void toConnection::rollback(toConnectionSub *sub)
{
    Utils::toBusy busy;
    sub->rollback();
}

void toConnection::rollback(void)
{
    QMutexLocker clock(&ConnectionLock);
    if((int)LoanCnt)
    	throw qApp->translate("toConnection::commit", "Couldn't commit, while connection is active");

    Q_FOREACH(toConnectionSub *conn, Connections)
    {
    	conn->rollback();
    }
    setNeedCommit(false);
}

void toConnection::delWidget(QWidget *widget)
{
	Widgets.remove(widget);

#ifdef QT_DEBUG
    // Cross check tools connections against connections widgets
    // Iterate over all the tool windows, increment the counter for every tool using this toConnection instance
    unsigned toolCnt = 0;
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
    unsigned toolCnt = 0;
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
	Q_FOREACH(toConnectionSub* conn, LentConnections)
	{
		QMenu *sess = menu->addMenu(conn->sessionId());
		QAction *cancel = new QAction("Cancel", this);
		QAction *close = new QAction("Close", this);
		cancel->setData(VPtr<toConnectionSub>::asQVariant(conn));
		close->setData(VPtr<toConnectionSub>::asQVariant(conn));
		sess->addAction(cancel);
		sess->addAction(close);
		ConnectionActions.insert(cancel);
		ConnectionActions.insert(close);
		connect(sess, SIGNAL(triggered(QAction *)), this, SLOT(commandCallback(QAction *)));
	}
	menu->addSeparator();
	Q_FOREACH(toConnectionSub* conn, Connections)
	{
		QMenu *sess = menu->addMenu(conn->sessionId());
		QAction *close = new QAction("Close", this);
		close->setData(VPtr<toConnectionSub>::asQVariant(conn));
		sess->addAction(close);
		ConnectionActions.insert(close);
		connect(sess, SIGNAL(triggered(QAction *)), this, SLOT(commandCallback(QAction *)));
	}
}

void toConnection::commandCallback(QAction *act)
{
	QString actionStr = act->text();
	toConnectionSub *conn = VPtr<toConnectionSub>::asPtr(act->data());
	if ( LentConnections.contains(conn))
	{

	} else if ( Connections.contains(conn)) {

	} else {
		Q_ASSERT_X(false, qPrintable(__QHERE__), "Unvalid QAction poiting onto unknown toConnectionSub");
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
void toConnection::setSchema(QString const & schema)
{
	if(this->Schema == schema)
		return;

#define CHANGE_CURRENT_SCHEMA QString("ALTER SESSION SET CURRENT_SCHEMA = \"%1\"")
#define CHANGE_CURRENT_SCHEMA_PG QString("SET search_path TO %1,\"$user\",public")
#define CHANGE_CURRENT_SCHEMA_TD QString("DATABASE \"%1\"")
#define CHANGE_CURRENT_SCHEMA_MY QString("USE `%1`")
    Schema = schema;

	if(Schema.isEmpty())
	{
		delInit("SCHEMA");
		return;
	}

    if (providerIs("Oracle"))
        setInit("SCHEMA", CHANGE_CURRENT_SCHEMA.arg(schema));
    else if (providerIs("QMYSQL"))
        setInit("SCHEMA", CHANGE_CURRENT_SCHEMA_MY.arg(schema));
    else if (providerIs("PostgreSQL"))
    	setInit("SCHEMA", CHANGE_CURRENT_SCHEMA_PG.arg(schema));
    else if (providerIs("Teradata"))
    	setInit("SCHEMA", CHANGE_CURRENT_SCHEMA_TD.arg(schema));
    else
        throw QString("No support for changing schema for this database");

    allExecute(InitStrings.value("SCHEMA"));
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
		Q_ASSERT_X((int)LoanCnt == LentConnections.size(), qPrintable(__QHERE__), "Invalid number of lent toConnectionSub(s)");
        return retval;
    }
    else
    {
		toConnectionSub* retval = addConnection();
		LoanCnt.fetchAndAddAcquire(1);
		LentConnections.insert(retval);
		Q_ASSERT_X((int)LoanCnt == LentConnections.size(), qPrintable(__QHERE__), "Invalid number of lent toConnectionSub(s)");
        return retval;
    }
}

void toConnection::putBackSub(toConnectionSub *conn)
{
    QMutexLocker clock(&ConnectionLock);
    Q_ASSERT_X( !Connections.contains(conn) , qPrintable(__QHERE__), "Invalid use of toConnectionSubLoan");
    LoanCnt.deref();
    if(conn->isBroken())
    {
    	delete conn;
    } else if(Connections.size() >= toConfigurationSingle::Instance().cachedConnections()) {
    	delete conn;
    }
    else
    	Connections.insert(conn);
    bool removed = LentConnections.remove(conn);
	Q_ASSERT_X(removed, qPrintable(__QHERE__), "Lent connection not found");
	Q_ASSERT_X((int)LoanCnt == LentConnections.size(), qPrintable(__QHERE__), "Invalid number of lent toConnectionSub(s)");
}

void toConnection::allExecute(toSQL const& sql)
{
    throw exception("Not implemented yet: void toConnection::allExecute(QString const& sql)");
}

void toConnection::allExecute(QString const& sql)
{
	try
	{
		Q_FOREACH(toConnectionSub *con, Connections)
		{
			//TODO
			//toQuery q(*con, sql, toQueryParams());
			//q.eof();
		}
	}
	TOCATCH
}
