
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

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tosql.h"
#include "totool.h"
#include "toconnectionpool.h"

#include <string>
#include <time.h>

#include <qapplication.h>
#include <qdir.h>
#include <QProgressDialog>
#include <QTextStream>
#include <qwidget.h>

#include <QString>
#include <QDateTime>
#include <QPointer>

// A little magic to get lrefresh to work and get a check on qApp

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

// Connection provider implementation

std::map<QString, toConnectionProvider *> *toConnectionProvider::Providers;
std::map<QString, toConnectionProvider *> *toConnectionProvider::Types;

void toConnectionProvider::checkAlloc(void)
{
    if (!Providers)
        Providers = new std::map<QString, toConnectionProvider *>;
}

void toConnectionProvider::addProvider(const QString &provider)
{
    checkAlloc();
    Provider = provider;
    (*Providers)[Provider] = this;
}

toConnectionProvider::toConnectionProvider(const QString &provider, bool add
                                          )
{
    Provider = provider;
    if (add
       )
        addProvider(provider);
    if (!Types)
        Types = new std::map<QString, toConnectionProvider *>;
    (*Types)[provider] = this;
}

std::list<QString> toConnectionProvider::providedOptions(const QString &)
{
    std::list<QString> ret;
    return ret;
}

void toConnectionProvider::removeProvider(const QString &provider)
{
    std::map<QString, toConnectionProvider *>::iterator i = Providers->find(provider);
    if (i != Providers->end())
        Providers->erase(i);
}

toConnectionProvider::~toConnectionProvider()
{
    try
    {
        if (!Provider.isEmpty())
            removeProvider(Provider);
        std::map<QString, toConnectionProvider *>::iterator i = Types->find(Provider);
        if (i != Types->end())
            Types->erase(i);
    }
    catch (...) {}
}

std::list<QString> toConnectionProvider::providedHosts(const QString &)
{
    std::list<QString> ret;
    return ret;
}

std::list<QString> toConnectionProvider::providers()
{
    std::list<QString> ret;
    if (!Providers)
        return ret;
    for (std::map<QString, toConnectionProvider *>::iterator i = Providers->begin(); i != Providers->end(); i++)
        ret.insert(ret.end(), (*i).first);
    return ret;
}

void toConnectionProvider::initializeAll(void)
{
    if (Types)
        for (std::map<QString, toConnectionProvider *>::iterator i = Types->begin();
                i != Types->end(); i++)
            (*i).second->initialize();
}

toConnectionProvider &toConnectionProvider::fetchProvider(const QString &provider)
{
    checkAlloc();
    std::map<QString, toConnectionProvider *>::iterator i = Providers->find(provider);
    if (i == Providers->end())
        throw QT_TRANSLATE_NOOP("toConnectionProvider", "Tried to fetch unknown provider %1").arg(QString(provider));
    return *((*i).second);
}

std::list<QString> toConnectionProvider::options(const QString &provider)
{
    return fetchProvider(provider).providedOptions(provider);
}

QWidget *toConnectionProvider::configurationTab(const QString &provider, QWidget *parent)
{
    return fetchProvider(provider).providerConfigurationTab(provider, parent);
}

toConnection::connectionImpl *toConnectionProvider::connection(const QString &provider,
        toConnection *conn)
{
    return fetchProvider(provider).provideConnection(provider, conn);
}

std::list<QString> toConnectionProvider::hosts(const QString &provider)
{
    return fetchProvider(provider).providedHosts(provider);
}

std::list<QString> toConnectionProvider::databases(const QString &provider, const QString &host,
        const QString &user, const QString &pwd)
{
    return fetchProvider(provider).providedDatabases(provider, host, user, pwd);
}

// const QString &toConnectionProvider::config(const QString &tag, const QString &def) {
//     QString str = Provider;
//     str.append(":");
//     str.append(tag);
//     return toConfigurationSingle::Instance().globalConfig(str, def);
// }
//
// void toConnectionProvider::setConfig(const QString &tag, const QString &def) {
//     QString str = Provider;
//     str.append(":");
//     str.append(tag);
//     toConfigurationSingle::Instance().globalSetConfig(str, def);
// }

QWidget *toConnectionProvider::providerConfigurationTab(const QString &, QWidget *)
{
    return NULL;
}

// toQuery implementation

toQuery::toQuery(toConnection &conn,
                 const toSQL &sql,
                 const QString &arg1,
                 const QString &arg2,
                 const QString &arg3,
                 const QString &arg4,
                 const QString &arg5,
                 const QString &arg6,
                 const QString &arg7,
                 const QString &arg8,
                 const QString &arg9)
        : Connection(QPointer<toConnection>(&conn)),
        ConnectionSub(conn.pooledConnection()),
        SQL(sql(*Connection).toAscii())
{
    Mode = Normal;
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    if (numArgs > 0)
        Params.insert(Params.end(), arg1);
    if (numArgs > 1)
        Params.insert(Params.end(), arg2);
    if (numArgs > 2)
        Params.insert(Params.end(), arg3);
    if (numArgs > 3)
        Params.insert(Params.end(), arg4);
    if (numArgs > 4)
        Params.insert(Params.end(), arg5);
    if (numArgs > 5)
        Params.insert(Params.end(), arg6);
    if (numArgs > 6)
        Params.insert(Params.end(), arg7);
    if (numArgs > 7)
        Params.insert(Params.end(), arg8);
    if (numArgs > 8)
        Params.insert(Params.end(), arg9);

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn,
                 const QString &sql,
                 const QString &arg1,
                 const QString &arg2,
                 const QString &arg3,
                 const QString &arg4,
                 const QString &arg5,
                 const QString &arg6,
                 const QString &arg7,
                 const QString &arg8,
                 const QString &arg9)
        : Connection(QPointer<toConnection>(&conn)),
        ConnectionSub(conn.pooledConnection()),
        SQL(sql)
{
    Mode = Normal;
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    if (numArgs > 0)
        Params.insert(Params.end(), arg1);
    if (numArgs > 1)
        Params.insert(Params.end(), arg2);
    if (numArgs > 2)
        Params.insert(Params.end(), arg3);
    if (numArgs > 3)
        Params.insert(Params.end(), arg4);
    if (numArgs > 4)
        Params.insert(Params.end(), arg5);
    if (numArgs > 5)
        Params.insert(Params.end(), arg6);
    if (numArgs > 6)
        Params.insert(Params.end(), arg7);
    if (numArgs > 7)
        Params.insert(Params.end(), arg8);
    if (numArgs > 8)
        Params.insert(Params.end(), arg9);

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, const toSQL &sql, const toQList &params)
        : Connection(QPointer<toConnection>(&conn)),
        ConnectionSub(conn.pooledConnection()),
        Params(params),
        SQL(sql(conn).toAscii())
{
    Mode = Normal;
    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

// for testing sub
toQuery::toQuery(toConnection &conn,
                 toConnectionSub *sub,
                 const QString &sql,
                 const toQList &params)
        : Connection(QPointer<toConnection>(&conn)),
        ConnectionSub(sub),
        Params(params),
        SQL(sql)
{
    Mode = Test;
    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, const QString &sql, const toQList &params)
        : Connection(QPointer<toConnection>(&conn)),
        ConnectionSub(conn.pooledConnection()),
        Params(params),
        SQL(sql)
{
    Mode = Normal;
    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn,
                 queryMode mode,
                 const toSQL &sql,
                 const toQList &params)
        : Connection(QPointer<toConnection>(&conn)),
        Params(params),
        SQL(sql(conn).toAscii())
{
    Mode = mode;

    ConnectionSub = conn.pooledConnection();

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn,
                 queryMode mode,
                 const QString &sql,
                 const toQList &params)
        : Connection(QPointer<toConnection>(&conn)),
        Params(params),
        SQL(sql)
{
    Mode = mode;

    ConnectionSub = conn.pooledConnection();

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
        Query->execute();
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        Connection->ConnectionPool->release(ConnectionSub);
        throw;
    }
    ConnectionSub->setQuery(this);
}

toQuery::toQuery(toConnection &conn, queryMode mode)
        : Connection(QPointer<toConnection>(&conn))
{
    Mode = mode;

    ConnectionSub = conn.pooledConnection();

    toBusy busy;
    try
    {
        Query = NULL;
        Query = conn.Connection->createQuery(this, ConnectionSub);
    }
    catch (...)
    {
        if (Query)
            delete Query;
        Query = NULL;
        throw;
    }
    ConnectionSub->setQuery(this);
}

void toQuery::execute(const toSQL &sql, const toQList &params)
{
    toBusy busy;
    SQL = sql(*Connection);
    Params = params;
    Query->execute();
}

void toQuery::execute(const QString &sql, const toQList &params)
{
    toBusy busy;
    SQL = sql;
    Params = params;
    Query->execute();
}

void toQuery::execute(const toSQL &sql)
{
    std::list<toQValue> params;
    params.clear();
    execute(sql, params);
}

void toQuery::execute(const toSQL &sql, const QString &param)
{
    std::list<toQValue> params;
    params.clear();
    params.insert(params.end(), param);
    execute(sql, params);
}

void toQuery::execute(const toSQL &sql, const QString &param1, const QString &param2, const QString &param3)
{
    std::list<toQValue> params;
    params.clear();
    params.insert(params.end(), param1);
    params.insert(params.end(), param2);
    params.insert(params.end(), param3);
    execute(sql, params);
}

toQuery::~toQuery()
{
    toBusy busy;
    if (Query)
        delete Query;
    try
    {
        if (ConnectionSub->query() == this)
            ConnectionSub->setQuery(NULL);
        if (Mode != Test && Connection && Connection->ConnectionPool)
            Connection->ConnectionPool->release(ConnectionSub);
    }
    catch (...) {}
}

bool toQuery::eof(void)
{
    return Query->eof();
}

toQList toQuery::readQuery(toConnection &conn, const toSQL &sql, toQList &params)
{
    toBusy busy;
    toQuery query(conn, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQuery(toConnection &conn, const QString &sql, toQList &params)
{
    toBusy busy;
    toQuery query(conn, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQuery(const QString &sql, toQList &params)
{
    toBusy busy;
    SQL = sql;
    Params = params;
    Query->execute();
    toQList ret;
    while (!eof())
        ret.insert(ret.end(), readValue());
    return ret;
}

toQList toQuery::readQuery(toConnection &conn, const toSQL &sql,
                           const QString &arg1, const QString &arg2,
                           const QString &arg3, const QString &arg4,
                           const QString &arg5, const QString &arg6,
                           const QString &arg7, const QString &arg8,
                           const QString &arg9)
{
    toBusy busy;
    toQuery query(conn, sql, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQuery(toConnection &conn, const QString &sql,
                           const QString &arg1, const QString &arg2,
                           const QString &arg3, const QString &arg4,
                           const QString &arg5, const QString &arg6,
                           const QString &arg7, const QString &arg8,
                           const QString &arg9)
{
    toBusy busy;
    toQuery query(conn, sql, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQueryNull(toConnection &conn, const toSQL &sql, toQList &params)
{
    toBusy busy;
    toQuery query(conn, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValueNull());
    return ret;
}

toQList toQuery::readQueryNull(toConnection &conn, const QString &sql, toQList &params)
{
    toQuery query(conn, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValueNull());
    return ret;
}

toQList toQuery::readQueryNull(toConnection &conn, const toSQL &sql,
                               const QString &arg1, const QString &arg2,
                               const QString &arg3, const QString &arg4,
                               const QString &arg5, const QString &arg6,
                               const QString &arg7, const QString &arg8,
                               const QString &arg9)
{
    toBusy busy;
    toQuery query(conn, sql, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValueNull());
    return ret;
}

toQList toQuery::readQueryNull(toConnection &conn, const QString &sql,
                               const QString &arg1, const QString &arg2,
                               const QString &arg3, const QString &arg4,
                               const QString &arg5, const QString &arg6,
                               const QString &arg7, const QString &arg8,
                               const QString &arg9)
{
    toBusy busy;
    toQuery query(conn, sql, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValueNull());
    return ret;
}

toQValue toQuery::readValue(void)
{
    if (!Connection)
        return toQValue(0);

    if (Connection->Abort)
        throw qApp->translate("toQuery", "Query aborted");
    return toNull(Query->readValue());
}

toQValue toQuery::readValueNull(void)
{
    if (!Connection)
        return toQValue(0);

    if (Connection->Abort)
        throw qApp->translate("toQuery", "Query aborted");
    return Query->readValue();
}

void toQuery::cancel(void)
{
    Query->cancel();
}

// toConnection implementation

toConnectionSub* toConnection::addConnection()
{
    toBusy busy;
    toConnectionSub *sub = Connection->createConnection();
    toLocker lock(Lock)
    ;
    toQList params;
    for (std::list<QString>::iterator i = InitStrings.begin(); i != InitStrings.end(); i++)
    {
        try
        {
            Connection->execute(sub, *i, params);
        }
        TOCATCH
    }

    return sub;
}


void toConnection::closeConnection(toConnectionSub *sub)
{
    if (Connection)
        Connection->closeConnection(sub);
}


toConnection::toConnection(const QString &provider,
                           const QString &user, const QString &password,
                           const QString &host, const QString &database,
                           const QString &schema,
                           const QString &color,
                           const std::set<QString> &options, bool cache)
        : Provider(provider),
        User(user),
        Password(password),
        Host(host),
        Database(database),
        Schema(schema),
        Color(color),
        Options(options),
        Connection(0)
{
    Connection = toConnectionProvider::connection(Provider, this);
    NeedCommit = Abort = false;
    ReadingCache = false;
    ConnectionPool = new toConnectionPool(this);

    PoolPtr sub(ConnectionPool);
    Version = Connection->version(*sub);

    if (cache)
    {
        if (toConfigurationSingle::Instance().objectCache() == 1)
            readObjects();
    }
    else
    {
        ReadingValues.up();
        ReadingValues.up();
    }
}

toConnection::toConnection(const toConnection &conn)
        : Provider(conn.Provider),
        User(conn.User),
        Password(conn.Password),
        Host(conn.Host),
        Database(conn.Database),
        Schema(conn.Schema),
        Color(conn.Color),
        Options(conn.Options),
        Connection(0)
{
    Connection = toConnectionProvider::connection(Provider, this);
    ReadingValues.up();
    ReadingValues.up();
    ReadingCache = false;
    NeedCommit = Abort = false;
    ConnectionPool = new toConnectionPool(this);

    PoolPtr sub(ConnectionPool);
    Version = Connection->version(*sub);
}

std::list<QString> toConnection::running(void)
{
    toBusy busy;
    toLocker lock(Lock)
    ;
    std::list<QString> ret;
    // this is insane. disabled code that tried to get sql from
    // running queries
    return ret;
}

void toConnection::cancelAll(void)
{
    ConnectionPool->cancelAll();
}

toConnection::~toConnection()
{
    toBusy busy;
    Abort = true;

    {
        closeWidgets();

        ConnectionPool->cancelAll(true);

        // wait for cacheObjects to finish
        toLocker clock(CacheLock);

        delete ConnectionPool;
        ConnectionPool = 0;

        toLocker lock(Lock);
    }
    if (ReadingCache)
    {
        ReadingValues.down();
        ReadingValues.down();
    }
    delete Connection;
}

toConnectionSub* toConnection::pooledConnection(void)
{
    return ConnectionPool->borrow();
}

void toConnection::commit(toConnectionSub *sub)
{
    toBusy busy;
    Connection->commit(sub);
}

void toConnection::commit(void)
{
    ConnectionPool->commit();
    setNeedCommit(false);
}

void toConnection::rollback(toConnectionSub *sub)
{
    toBusy busy;
    Connection->rollback(sub);
}

void toConnection::rollback(void)
{
    ConnectionPool->rollback();
    setNeedCommit(false);
}

void toConnection::delWidget(QWidget *widget)
{
    for (std::list<QPointer<QWidget> >::iterator i = Widgets.begin();
            i != Widgets.end();
            i++)
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
    for (std::list<QPointer<QWidget> >::iterator i = Widgets.begin();
            i != Widgets.end();
            i++)
    {

        if (!(*i))
            continue;

        // make double sure destroy flag is set
        (*i)->setAttribute(Qt::WA_DeleteOnClose);

        // black magic to close widget's MDI subwindow too
        if ((*i)->parent()
                && (*i)->parent()->metaObject()->className() == QMdiSubWindow::staticMetaObject.className())
        {
            qobject_cast<QMdiSubWindow*>((*i)->parent())->close();
        }

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

    if (!toIsMySQL(*this))
        ret += Database;

    if (!Host.isEmpty() && Host != "SQL*Net")
    {
        if (!toIsMySQL(*this))
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

void toConnection::addInit(const QString &sql)
{
    delInit(sql);
    InitStrings.insert(InitStrings.end(), sql);
}

void toConnection::delInit(const QString &sql)
{
    std::list<QString>::iterator i = InitStrings.begin();
    while (i != InitStrings.end())
    {
        if ((*i) == sql)
            i = InitStrings.erase(i);
        else
            i++;
    }
}

const std::list<QString> toConnection::initStrings() const
{
    return InitStrings;
}

void toConnection::parse(const QString &sql)
{
    toBusy busy;

    PoolPtr sub(ConnectionPool);
    Version = Connection->version(*sub);

    Connection->parse(*sub, sql);
}

void toConnection::parse(const toSQL &sql)
{
    toBusy busy;

    PoolPtr sub(ConnectionPool);
    Version = Connection->version(*sub);

    Connection->parse(*sub, toSQL::sql(sql, *this));
}

void toConnection::execute(const toSQL &sql, toQList &params)
{
    toBusy busy;

    PoolPtr sub(ConnectionPool);
    Version = Connection->version(*sub);

    Connection->execute(*sub, toSQL::sql(sql, *this), params);
}

void toConnection::execute(const QString &sql, toQList &params)
{
    toBusy busy;

    PoolPtr sub(ConnectionPool);
    Version = Connection->version(*sub);

    Connection->execute(*sub, sql, params);
}

void toConnection::execute(const toSQL &sql,
                           const QString &arg1, const QString &arg2,
                           const QString &arg3, const QString &arg4,
                           const QString &arg5, const QString &arg6,
                           const QString &arg7, const QString &arg8,
                           const QString &arg9)
{
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    toQList args;
    if (numArgs > 0)
        args.insert(args.end(), arg1);
    if (numArgs > 1)
        args.insert(args.end(), arg2);
    if (numArgs > 2)
        args.insert(args.end(), arg3);
    if (numArgs > 3)
        args.insert(args.end(), arg4);
    if (numArgs > 4)
        args.insert(args.end(), arg5);
    if (numArgs > 5)
        args.insert(args.end(), arg6);
    if (numArgs > 6)
        args.insert(args.end(), arg7);
    if (numArgs > 7)
        args.insert(args.end(), arg8);
    if (numArgs > 8)
        args.insert(args.end(), arg9);

    toBusy busy;

    PoolPtr sub(ConnectionPool);
    Version = Connection->version(*sub);

    Connection->execute(*sub, toSQL::sql(sql, *this), args);
}

void toConnection::execute(const QString &sql,
                           const QString &arg1, const QString &arg2,
                           const QString &arg3, const QString &arg4,
                           const QString &arg5, const QString &arg6,
                           const QString &arg7, const QString &arg8,
                           const QString &arg9)
{
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    toQList args;
    if (numArgs > 0)
        args.insert(args.end(), arg1);
    if (numArgs > 1)
        args.insert(args.end(), arg2);
    if (numArgs > 2)
        args.insert(args.end(), arg3);
    if (numArgs > 3)
        args.insert(args.end(), arg4);
    if (numArgs > 4)
        args.insert(args.end(), arg5);
    if (numArgs > 5)
        args.insert(args.end(), arg6);
    if (numArgs > 6)
        args.insert(args.end(), arg7);
    if (numArgs > 7)
        args.insert(args.end(), arg8);
    if (numArgs > 8)
        args.insert(args.end(), arg9);

    toBusy busy;

    PoolPtr sub(ConnectionPool);
    Version = Connection->version(*sub);

    Connection->execute(*sub, sql, args);
}

void toConnection::allExecute(const toSQL &sql, toQList &params)
{
    ConnectionPool->executeAll(toSQL::sql(sql, *this), params);
}

void toConnection::allExecute(const QString &sql, toQList &params)
{
    ConnectionPool->executeAll(sql, params);
}

void toConnection::allExecute(const toSQL &sql,
                              const QString &arg1, const QString &arg2,
                              const QString &arg3, const QString &arg4,
                              const QString &arg5, const QString &arg6,
                              const QString &arg7, const QString &arg8,
                              const QString &arg9)
{
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    toQList args;
    if (numArgs > 0)
        args.insert(args.end(), arg1);
    if (numArgs > 1)
        args.insert(args.end(), arg2);
    if (numArgs > 2)
        args.insert(args.end(), arg3);
    if (numArgs > 3)
        args.insert(args.end(), arg4);
    if (numArgs > 4)
        args.insert(args.end(), arg5);
    if (numArgs > 5)
        args.insert(args.end(), arg6);
    if (numArgs > 6)
        args.insert(args.end(), arg7);
    if (numArgs > 7)
        args.insert(args.end(), arg8);
    if (numArgs > 8)
        args.insert(args.end(), arg9);

    ConnectionPool->executeAll(toSQL::sql(sql, *this), args);
}

void toConnection::allExecute(const QString &sql,
                              const QString &arg1, const QString &arg2,
                              const QString &arg3, const QString &arg4,
                              const QString &arg5, const QString &arg6,
                              const QString &arg7, const QString &arg8,
                              const QString &arg9)
{
    int numArgs;
    if (!arg9.isNull())
        numArgs = 9;
    else if (!arg8.isNull())
        numArgs = 8;
    else if (!arg7.isNull())
        numArgs = 7;
    else if (!arg6.isNull())
        numArgs = 6;
    else if (!arg5.isNull())
        numArgs = 5;
    else if (!arg4.isNull())
        numArgs = 4;
    else if (!arg3.isNull())
        numArgs = 3;
    else if (!arg2.isNull())
        numArgs = 2;
    else if (!arg1.isNull())
        numArgs = 1;
    else
        numArgs = 0;

    toQList args;
    if (numArgs > 0)
        args.insert(args.end(), arg1);
    if (numArgs > 1)
        args.insert(args.end(), arg2);
    if (numArgs > 2)
        args.insert(args.end(), arg3);
    if (numArgs > 3)
        args.insert(args.end(), arg4);
    if (numArgs > 4)
        args.insert(args.end(), arg5);
    if (numArgs > 5)
        args.insert(args.end(), arg6);
    if (numArgs > 6)
        args.insert(args.end(), arg7);
    if (numArgs > 7)
        args.insert(args.end(), arg8);
    if (numArgs > 8)
        args.insert(args.end(), arg9);

    ConnectionPool->executeAll(sql, args);
}

const QString &toConnection::provider(void) const
{
    return Provider;
}

QString toConnection::cacheDir()
{
    QString home(QDir::homePath());
    QString dirname(toConfigurationSingle::Instance().cacheDir());

    if (dirname.isEmpty())
    {
#ifdef Q_OS_WIN32
        if (getenv("TEMP"))
            dirname = QString(getenv("TEMP"));
        else
#endif
            dirname = QString(home);
        dirname += "/.tora_cache";
    }
    return dirname;
}

QString toConnection::cacheFile()
{
    QString dbname(description(false).trimmed());

    return (cacheDir() + "/" + dbname).trimmed();
}

bool toConnection::loadDiskCache()
{
    if (!toConfigurationSingle::Instance().cacheDisk())
        return false;

    toConnection::objectName *cur = 0;
    int objCounter = 0;
    int synCounter = 0;

    QString filename = cacheFile();

    QFile file(filename);

    if (!QFile::exists(filename))
        return false;

    QFileInfo fi(file);
    QDateTime today;
    if (fi.lastModified().addDays(toConfigurationSingle::Instance().cacheTimeout()) < today)
        return false;

    /** read in all data
     */

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QString data = file.readAll();

    /** build cache lists
     */

    QStringList records = data.split("\x1D", QString::KeepEmptyParts);
    for (QStringList::Iterator i = records.begin(); i != records.end(); i++)
    {
        objCounter++;
        QStringList record = (*i).split("\x1E", QString::KeepEmptyParts);
        QStringList::Iterator rec = record.begin();
        cur = new objectName;
        (*cur).Owner = (*rec);
        rec++;
        (*cur).Name = (*rec);
        rec++;
        (*cur).Type = (*rec);
        rec++;
        (*cur).Comment = (*rec);
        rec++;
        QStringList slist = (*rec).split("\x1F", QString::SkipEmptyParts);
        for (QStringList::Iterator s = slist.begin(); s != slist.end(); s++)
        {
            SynonymMap[(*s)] = (*cur);
            (*cur).Synonyms.insert((*cur).Synonyms.end(), (*s));
            synCounter++;
        }
        ObjectNames.insert(ObjectNames.end(), (*cur));
        delete cur;
        cur = 0;
    }
    return true;
}

void toConnection::writeDiskCache()
{
    QString text;
    long objCounter = 0;
    long synCounter = 0;

    if (!toConfigurationSingle::Instance().cacheDisk())
        return ;


    QString filename(cacheFile());

    /** check pathnames and create
     */

    QString dirname(cacheDir());
    QDir dir;
    dir.setPath(dirname);

    if (!dir.exists(dirname))
        dir.mkdir(dirname);


    /** build record to write out
     */

    QStringList record;
    QStringList records;
    QStringList recordSynonym;
    for (std::list<objectName>::iterator i = ObjectNames.begin(); i != ObjectNames.end(); i++)
    {
        record.clear();
        record.append((*i).Owner);
        record.append((*i).Name);
        record.append((*i).Type);
        record.append((*i).Comment);
        for (std::list<QString>::iterator s = (*i).Synonyms.begin(); s != (*i).Synonyms.end(); s++)
        {
            recordSynonym.append((*s));
            synCounter++;
        }
        record.append(recordSynonym.join("\x1F"));
        recordSynonym.clear();
        objCounter++;
        records.append(record.join("\x1E"));
    }
    /** open file
     */
    QFile file(filename);
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);
    QTextStream t(&file);
    t << records.join("\x1D");
    file.flush();
    file.close();
}

void toConnection::cacheObjects::run()
{
    bool diskloaded = false;

    // hold while running
    toLocker clock(Connection->CacheLock);

    try
    {
        diskloaded = Connection->loadDiskCache();
        if (!diskloaded && !Connection->Abort)
        {
            std::list<objectName> n = Connection->Connection->objectNames();
            if (!Connection->Abort)
                Connection->ObjectNames = n;
        }

        Connection->ObjectNames.sort();
        Connection->ReadingValues.up();

        if (!diskloaded && !Connection->Abort)
        {
            std::map<QString, objectName> m =
                Connection->Connection->synonymMap(Connection->ObjectNames);
            if (!Connection->Abort)
            {
                Connection->SynonymMap = m;
                Connection->writeDiskCache();
            }
        }
    }
    catch (...)
    {
        if (Connection->ReadingValues.getValue() == 0)
            Connection->ReadingValues.up();
    }

    if (Connection)
        Connection->ReadingValues.up();
}


void toConnection::readObjects(void)
{
    if (toConfigurationSingle::Instance().objectCache() == 3)
    {
        ReadingCache = false;
        return ;
    }

    if (!ReadingCache)
    {
        ReadingCache = true;
        try
        {
            (new toThread(new cacheObjects(this)))->start();
        }
        catch (...)
        {
            ReadingCache = false;
        }
    }
}

void toConnection::rereadCache(void)
{

    if (toConfigurationSingle::Instance().objectCache() == 3)
    {
        ColumnCache.clear();
        return ;
    }

    if (ReadingValues.getValue() < 2 && ReadingCache)
    {
        toStatusMessage(qApp->translate("toConnection",
                                        "Not done caching objects, can not clear unread cache"));
        return ;
    }


    ReadingCache = false;
    while (ReadingValues.getValue() > 0)
        ReadingValues.down();

    ObjectNames.clear();
    ColumnCache.clear();
    SynonymMap.clear();

    /** delete cache file to force reload
     */

    QString filename(cacheFile());

    if (QFile::exists(filename))
        QFile::remove(filename);

    readObjects();
}

QString toConnection::quote(const QString &name, const bool quoteLowercase)
{
    if (!name.isNull())
        return Connection->quote(name, quoteLowercase);
    return QString::null;
}

QString toConnection::unQuote(const QString &name)
{
    if (!name.isNull())
        return Connection->unQuote(name);
    return QString::null;
}

bool toConnection::cacheAvailable(bool synonyms, bool block, bool need)
{
    if (toConfigurationSingle::Instance().objectCache() == 3)
        return true;

    if (!ReadingCache)
    {
        if (!need)
            return true;
        if (toConfigurationSingle::Instance().objectCache() == 2 && !block)
            return true;
        readObjects();
        toMainWidget()->checkCaching();
    }
    if (ReadingValues.getValue() == 0 || (ReadingValues.getValue() == 1 && synonyms == true))
    {
        if (block)
        {
            toBusy busy;
            if (toThread::mainThread())
            {
                QProgressDialog waiting(qApp->translate("toConnection",
                                                        "Waiting for object caching to be completed.\n"
                                                        "Canceling this dialog will probably leave some list of\n"
                                                        "database objects empty."),
                                        qApp->translate("toConnection", "Cancel"),
                                        0,
                                        10,
                                        toMainWidget());
                waiting.setWindowTitle(qApp->translate("toConnection", "Waiting for object cache"));
                int num = 1;

                int waitVal = (synonyms ? 2 : 1);
                do
                {
                    qApp->processEvents();
                    toThread::msleep(100);
                    waiting.setValue((++num) % 10);
                    if (waiting.wasCanceled())
                        return false;
                }
                while (ReadingValues.getValue() < waitVal);
            }

            ReadingValues.down();
            if (synonyms)
            {
                ReadingValues.down();
                ReadingValues.up();
            }
            ReadingValues.up();
        }
        else
            return false;
    }
    return true;
}

std::list<toConnection::objectName> &toConnection::objects(bool block)
{
    if (!cacheAvailable(false, block))
    {
        toStatusMessage(qApp->translate("toConnection", "Not done caching objects"), false, false);
        static std::list<objectName> ret;
        return ret;
    }

    return ObjectNames;
}

void toConnection::addIfNotExists(toConnection::objectName &obj)
{
    if (!cacheAvailable(true, false))
    {
        toStatusMessage(qApp->translate("toConnection", "Not done caching objects"), false, false);
        return ;
    }
    std::list<toConnection::objectName>::iterator i = ObjectNames.begin();
    while (i != ObjectNames.end() && (*i) < obj)
        i++;
    if (i != ObjectNames.end() && *i == obj) // Already exists, don't add
        return ;
    ObjectNames.insert(i, obj);
}

std::map<QString, toConnection::objectName> &toConnection::synonyms(bool block)
{
    if (!cacheAvailable(true, block))
    {
        toStatusMessage(qApp->translate("toConnection", "Not done caching objects"), false, false);
        static std::map<QString, objectName> ret;
        return ret;
    }

    return SynonymMap;
}

const toConnection::objectName &toConnection::realName(const QString &object,
        QString &synonym,
        bool block)
{
    if (!cacheAvailable(true, block))
        throw qApp->translate("toConnection", "Not done caching objects");

    QString name;
    QString owner;

    QChar q('"');
    QChar c('.');

    bool quote = false;
    for (int pos = 0; pos < object.length(); pos++)
    {
        if (object.at(pos) == q)
        {
            quote = !quote;
        }
        else
        {
            if (!quote && object.at(pos) == c)
            {
                owner = name;
                name = QString::null;
            }
            else
                name += object.at(pos);
        }
    }

    QString uo = owner.toUpper();
    QString un = name.toUpper();

    synonym = QString::null;
    for (std::list<objectName>::iterator i = ObjectNames.begin(); i != ObjectNames.end(); i++)
    {
        if (owner.isEmpty())
        {
            if (((*i).Name == un || (*i).Name == name) &&
                    ((*i).Owner == user().toUpper() || (*i).Owner == database()))
                return *i;
        }
        else if (((*i).Name == un || (*i).Name == name) &&
                 ((*i).Owner == uo || (*i).Owner == owner))
            return *i;
    }
    if (owner.isEmpty())
    {
        std::map<QString, objectName>::iterator i = SynonymMap.find(name);
        if (i == SynonymMap.end() && un != name)
        {
            i = SynonymMap.find(un);
            synonym = un;
        }
        else
            synonym = name;
        if (i != SynonymMap.end())
        {
            return (*i).second;
        }
    }
    throw qApp->translate(
        "toConnection",
        "Object %1 not available for %2").arg(object).arg(user());
}

const toConnection::objectName &toConnection::realName(const QString &object, bool block)
{
    QString dummy;
    return realName(object, dummy, block);
}

toQDescList &toConnection::columns(const objectName &object, bool nocache)
{
    std::map<objectName, toQDescList>::iterator i = ColumnCache.find(object);
    if (i == ColumnCache.end() || nocache)
    {
        ColumnCache[object] = Connection->columnDesc(object);
    }

    return ColumnCache[object];
}

bool toConnection::objectName::operator < (const objectName &nam) const
{
    if (Owner < nam.Owner || (Owner.isNull() && !nam.Owner.isNull()))
        return true;
    if (Owner > nam.Owner || (!Owner.isNull() && nam.Owner.isNull()))
        return false;
    if (Name < nam.Name || (Name.isNull() && !nam.Name.isNull()))
        return true;
    if (Name > nam.Name || (!Name.isNull() && nam.Name.isNull()))
        return false;
    if (Type < nam.Type)
        return true;
    return false;
}

bool toConnection::objectName::operator == (const objectName &nam) const
{
    return Owner == nam.Owner && Name == nam.Name && Type == nam.Type;
}

toSyntaxAnalyzer &toConnection::connectionImpl::analyzer()
{
    return toSyntaxAnalyzer::defaultAnalyzer();
}

toSyntaxAnalyzer &toConnection::analyzer()
{
    return Connection->analyzer();
}

std::list<toConnection::objectName>
toConnection::connectionImpl::objectNames(void)
{
    std::list<toConnection::objectName> ret;
    return ret;
}

std::map<QString, toConnection::objectName>
toConnection::connectionImpl::synonymMap(std::list<toConnection::objectName> &)
{
    std::map<QString, toConnection::objectName> ret;
    return ret;
}

toQDescList toConnection::connectionImpl::columnDesc(const objectName &)
{
    toQDescList ret;
    return ret;
}

void toConnection::connectionImpl::parse(toConnectionSub *,
        const QString &)
{
    throw qApp->translate(
        "toConnection",
        "Parse only not implemented for this type of connection");
}
