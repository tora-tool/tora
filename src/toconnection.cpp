
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
#include "tocache.h"

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
    ConnectionPool = new toConnectionPool(this);
    Cache = new toCache(description(false).trimmed());

    PoolPtr sub(ConnectionPool);
    Version = Connection->version(*sub);

    if (cache)
    {
        if (toConfigurationSingle::Instance().objectCache() == 1)
            Cache->readObjects(new cacheObjects(this));
    }
    else
    {
        Cache->ReadingValues.up();
        Cache->ReadingValues.up();
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
    Cache->ReadingValues.up();
    Cache->ReadingValues.up();
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
    this->Cache->writeDiskCache();

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

void toConnection::cacheObjects::run()
{
    bool diskloaded = false;

    // hold while running
    toLocker clock(Connection->CacheLock);

    try
    {
        diskloaded = Connection->Cache->loadDiskCache();
        if (!diskloaded && !Connection->Abort)
        {
            std::list<objectName> n = Connection->Connection->objectNames();
            if (!Connection->Abort)
                Connection->Cache->setObjectList(n);
        }

        Connection->Cache->ReadingValues.up();

        if (!diskloaded && !Connection->Abort)
        {
            std::map<QString, objectName> m =
                Connection->Connection->synonymMap(Connection->Cache->objects(true));
            if (!Connection->Abort)
            {
                Connection->Cache->setSynonymList(m);
                Connection->Cache->writeDiskCache();
            }
        }
    }
    catch (...)
    {
        if (Connection->Cache->ReadingValues.getValue() == 0)
            Connection->Cache->ReadingValues.up();
    }

    if (Connection)
        Connection->Cache->ReadingValues.up();
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

toSyntaxAnalyzer &toConnection::connectionImpl::analyzer()
{
    return toSyntaxAnalyzer::defaultAnalyzer();
}

toSyntaxAnalyzer &toConnection::analyzer()
{
    return Connection->analyzer();
}

std::list<toConnection::objectName> toConnection::connectionImpl::objectNames(void)
{
    std::list<objectName> ret;
    return ret;
}

std::map<QString, toConnection::objectName> toConnection::connectionImpl::synonymMap(std::list<objectName> &)
{
    std::map<QString, objectName> ret;
    return ret;
}

toQDescList toConnection::connectionImpl::columnDesc(const objectName &)
{
    toQDescList ret;
    return ret;
}

toQDescList &toConnection::columns(const objectName &object, bool nocache)
{
    toQDescList * cols;
    cols = &(Cache->columns(object));
    if (cols->size() == 0 || nocache)
    {
        Cache->addColumns(object, Connection->columnDesc(object));
        cols = &(Cache->columns(object));
    }

    return *cols;
}

void toConnection::rereadCache(void)
{
    Cache->rereadCache(new cacheObjects(this));
}

std::list<toConnection::objectName> &toConnection::objects(bool block)
{
    return Cache->objects(block);
}

bool toConnection::cacheAvailable(bool synonyms, bool block, bool need)
{
    return Cache->cacheAvailable(synonyms, block, need, new cacheObjects(this));
}

const toConnection::objectName &toConnection::realName(const QString &object, QString &synonym, bool block)
{
    return Cache->realName(object, synonym, block, user(), database());
}

const toConnection::objectName &toConnection::realName(const QString &object, bool block)
{
    QString dummy;
    return Cache->realName(object, dummy, block, user(), database());
}

std::list<toConnection::objectName> toConnection::tables(const objectName &object, bool nocache)
{
    return Cache->tables(object, nocache);
}

void toConnection::connectionImpl::parse(toConnectionSub *,
        const QString &)
{
    throw qApp->translate(
        "toConnection",
        "Parse only not implemented for this type of connection");
}
