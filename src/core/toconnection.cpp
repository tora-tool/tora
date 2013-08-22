
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
#include "core/totool.h"
#include "core/toconfiguration.h"
#include "core/toconnectionprovider.h"

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
    , Schema(schema)
    , Color(color)
    , Options(options)
    , pConnectionImpl(NULL)
    , pCache(NULL)
    , pTrait(NULL)
    , NeedCommit(false)
    , Abort(false)
    , LoanCnt(0)
{
    pConnectionImpl = toConnectionProviderRegistrySing::Instance().get(provider).createConnectionImpl(*this);
    pTrait = toConnectionProviderRegistrySing::Instance().get(provider).createConnectionTrait();

    toConnectionSub* connSub = addConnection();
    Version = connSub->version();
    Connections.insert(connSub);

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
{
    //tool Connection = toConnectionProvider::connection(Provider, this);
    //ConnectionPool = new toConnectionPool(this);

    //PoolPtr sub(ConnectionPool);
    //Version = Connection->version(*sub);

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
            //sub->execute(sql, params);
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
    }
    else
        throw exception("Can not close non-existing toConnectionSub");
}

QList<QString> toConnection::running(void) const
{
    Utils::toBusy busy;
    QMutexLocker lock(&ConnectionLock);
    QList<QString> ret;
    ret << "Not implemented yet.";
    // this is insane. disabled code that tried to get sql from
    // running queries
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
    for (QList<QPointer<QWidget> >::iterator i = Widgets.begin();  i != Widgets.end();  i++)
    {
        if ((*i) && (*i) == widget)
        {
            Widgets.erase(i);
            break;
        }
    }
}

void toConnection::addWidget(QWidget *widget)
{
    Widgets.insert(Widgets.end(), QPointer<QWidget>(widget));
}

bool toConnection::closeWidgets(void)
{
    for (QList<QPointer<QWidget> >::iterator i = Widgets.begin(); i != Widgets.end();  i++)
    {

        if (!(*i))
            continue;

#pragma message WARN("TODO/FIXME: close tool windows")

        // make double sure destroy flag is set
        (*i)->setAttribute(Qt::WA_DeleteOnClose);
        if (!(*i)->close())
        {
            // close will fail if parent already closed.
            // closing parent will hide children though
            if ((*i)->isVisible())
                return false;
        }
        else
            delete *i;
    }

    Widgets.clear();
    return true;
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

void toConnection::setInit(const QString &key, const QString &sql)
{
    InitStrings.insert(key, sql);
}

void toConnection::delInit(const QString &key)
{
    InitStrings.remove(key);
}

QList<QString> const& toConnection::initStrings() const
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
        Connections.erase(Connections.begin());
        LoanCnt.fetchAndAddAcquire(1);
        return retval;
    }
    else
    {
        LoanCnt.fetchAndAddAcquire(1);
        return addConnection();
    }
}

void toConnection::putBackSub(toConnectionSub *conn)
{
    QMutexLocker clock(&ConnectionLock);
    Q_ASSERT_X( !Connections.contains(conn) , qPrintable(__QHERE__), "Invalid use of toConnectionSubLoan");
    LoanCnt.deref();
    if(conn->isBroken())
    	delete conn;
    else
    	Connections.insert(conn);
}

void toConnection::allExecute(toSQL const& sql)
{
    throw exception("Not implemented yet: void toConnection::allExecute(QString const& sql)");
}

void toConnection::allExecute(QString const& sql)
{
    throw exception("Not implemented yet: void toConnection::allExecute(QString const& sql)");
}
