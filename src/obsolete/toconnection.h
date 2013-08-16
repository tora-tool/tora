
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

#ifndef TOCONNECTION_H
#define TOCONNECTION_H

#include "config.h"
#include "toqvalue.h"
#include "tothread.h"
///#include "tocache.h"
#include "tocachenew.h"
#include "toqueryimpl.h"

#include <list>
#include <map>
#include <set>

#include <QtGui/QString>
#include <QtGui/QPointer>
#include <QtGui/QMetaType>
#include <QtGui/QDateTime>

class QWidget;
class toConnection;
class toConnectionProvider;
class toSQL;
class toQuery;
class toSyntaxAnalyzer;
class toConnectionPool;

/** This class is an abstract definition of an actual connection to a database.
 * Each @ref toConnection object can have one or more actual connections to the
 * database depending on long running queries. Normally you will never need to
 * bother with this class if you aren't creating a new database provider
 * (@ref toConnectionProvider).
 */
class toConnectionSub
{
    toQuery *Query;
    QDateTime LastUsed; // last time this db connection was actually used
public:
    /** Create connection to database.
     */
    toConnectionSub()
    {
        Query = NULL;
    }
    /** Close connection.
     */
    virtual ~toConnectionSub()
    { }
    /** Query current running on connection or NULL.
     */
    toQuery *query()
    {
        return Query;
    }
    /** Set query currently running on connection. NULL means none.
     */
    void setQuery(toQuery *query)
    {
        Query = query;
    }
    /** Cancel anything running on this sub.
     */
    virtual void cancel(void)
    { }
    /** Get time when last query on this connection has finished
     */
    QDateTime lastUsed(void)
    {
        return LastUsed;
    }
    /** Set time when last query on this connection has finished to "now"
     */
    void setLastUsed(void)
    {
        LastUsed = QDateTime::currentDateTime();
    }
};

/** Represent a database connection in TOra. Observe that this can mean several actual
 * connections to the database as queries that are expected to run a long time are sometimes
 * executed in their own connection to make sure the interface doesn't lock up for a long time.
 */
class toConnection : public QObject
{
    Q_OBJECT;

    QString Provider;
    QString User;
    QString Password;
    QString Host;
    QString Database;
    QString Schema;
    QString Version;
    QString Color;
    std::list<QPointer<QWidget> > Widgets;
    std::list<QString> InitStrings;
    std::set<QString> Options;
    bool NeedCommit;

    mutable toLock ConnectionLock;

    //toConnectionPool *ConnectionPool;

public:

    //typedef toCache::objectName objectName;

    /** Class that could be used to throw exceptions in connection errors. Must use if you
     * want to indicate error offset.
     */
    class exception : public QString
    {
        int Offset;
    public:
        /** Create an empty exception
         */
        exception() : QString()
        {
            Offset = -1;
        }

        /** Create an exception with a string description.
         */
        exception(const QString &str, int offset = -1)
            : QString(str)
        {
            Offset = offset;
        }
        /** Get the offset of the error of the current statement.
         */
        int offset(void) const
        {
            return Offset;
        }
        /** Set the offset of the error of the error.
         */
        void setOffset(int offset)
        {
            Offset = offset;
        }
    };

    /** This class is an abstract baseclass to actually implement the communication with the
     * database.
     * (See also @ref toQuery::queryImpl and @ref toConnectionProvider)
     */
    class connectionImpl
    {
        toConnection *Connection;
    public:
        /** Get the parent connection object of this connection.
         */
        toConnection &connection(void)
        {
            return *Connection;
        }
        /** Create a new connection implementation for a connection.
         * @param conn Connection to implement.
         */
        connectionImpl(toConnection *conn)
        {
            Connection = conn;
        };
        /** Destructor.
         */
        virtual ~connectionImpl()
        { }

        /** Commit the supplied actual database connection.
         */
        virtual void commit(toConnectionSub *) = 0;
        /** Rollback the supplied actual database connection.
         */
        virtual void rollback(toConnectionSub *) = 0;

        /** If not true can never run more than one query per connection sub and TOra will
         * work around this limitation by opening more connections if needed.
         */
        virtual bool handleMultipleQueries()
        {
            return true;
        }

        /** Create a new connection to the database.
         */
        virtual toConnectionSub *createConnection(void) = 0;
        /** Close a connection to the database.
         */
        virtual void closeConnection(toConnectionSub *) = 0;

        /** Get the version of the database connected to.
         */
        virtual QString version(toConnectionSub *) = 0;

        /** Return a string representation to address an object.
         * @param name The name to be quoted.
         * @param quoteLowercase Enclose in quotes when identifier has lowercase letters
         * @return String addressing table.
         */
        virtual QString quote(const QString &name, const bool quoteLowercase = true)
        {
            return name;
        }
        /** Perform the opposite of @ref quote.
         * @param name The name to be un-quoted.
         * @return String addressing table.
         */
        virtual QString unQuote(const QString &name)
        {
            return name;
        }

        /**
         * Get syntax analyzer for connection
         * @return A reference to the syntax analyzer to use for the connection.
         */
        virtual toSyntaxAnalyzer &analyzer();

        /** Extract available objects to query for connection. Any access to the
         * database should always be run using a long running query. If something
         * goes wrong should throw exception.
         * @return List of available objects.
         */
        ///virtual QList<toConnection::objectName> objectNames(const QString &owner = "",
        ///                                                        const QString &type = "",
        ///                                                        const QString &name = "");

        virtual QList<toCacheNew::CacheEntry*> objectNamesNew(QString const& schema = "") = 0;

        /** Get synonyms available for connection. Any access to the
         * database should always be run using a long running query. If something
         * goes wrong should throw exception.
         * @param objects Available objects for the connection. Objects
         *                are sorted in owner and name order. Don't modify
         *                this list.
         * @return Map of synonyms to objectnames.
         */
        ///virtual std::map<QString, toConnection::objectName> synonymMap(QList<toConnection::objectName> &objects);
        /** Extract available columns to query for a table.
         * @param table Table to get column for.
         * @return List of columns for table or view.
         */
        ///virtual toQDescList columnDesc(const toConnection::objectName &table);
        ///virtual toQDescList columnDesc(const toCacheNew::ObjectRef &table);
        virtual void describeObject(toCacheNew::CacheEntry*, toConnectionSub *conn) = 0;

        /** Create a new query implementation for this connection.
         * @return A query implementation, allocated with new.
         */
        virtual queryImpl *createQuery(toQuery *query, toConnectionSub *conn) = 0;
        /** Execute a query on an actual connection without caring about the result.
         * @param conn Connection to execute on.
         * @param sql SQL to execute.
         * @param params Parameters to pass to query.
         */
        virtual void execute(toConnectionSub *conn, const QString &sql, toQList &params) = 0;
        /** Parse a query on an actual connection and report any syntax problems encountered.
         * Defaults to not implemented.
         * @param conn Connection to execute on.
         * @param sql SQL to parse
         */
        virtual void parse(toConnectionSub *conn, const QString &sql);
        /** Get a list for primary keys
         *
         */
        virtual std::list<QString> primaryKeys()
        {
            return std::list<QString>();
        }
    };

private:

    toConnectionSub* addConnection(void);
    void closeConnection(toConnectionSub *sub);

    connectionImpl *Connection;

    //class cacheObjects : public toTask
    //{
    //  QPointer<toConnection> Connection;
    //public:
    //    cacheObjects(toConnection *conn)
    //        : Connection(conn)
    //    { }
    //    virtual void run(void);
    //};
    //friend class cacheObjects;

    class cacheObjectsNew : public toTask
    {
        QPointer<toConnection> Connection;
    public:
        cacheObjectsNew(toConnection *conn)
            : Connection(conn)
        { }
        virtual void run(void);
    };
    friend class cacheObjectsNew;

    bool Abort;

    toConnectionSub* pooledConnection(void);

public:
    ///toCache * Cache;
    toCacheNew *CacheNew;

    /** Create a new connection.
     * @param provider Which database provider to use for this connection.
     * (See @ref to toDatabaseConnection)
     * @param user User to connect to the database with.
     * @param password Password to connect with.
     * @param host Host to connect to the database with.
     * @param database Database to connect to.
     * @param schema Default schema to switch to.
     * @param color Highlighting color for GUI widgets
     * @param options Options used to connect to the database with.
     */
    toConnection(const QString &provider, const QString &user, const QString &password,
                 const QString &host, const QString &database, const QString &schema,
                 const QString &color,
                 const std::set<QString> &options);
    /** Create a copy of a connection. Will not cache objects, so objects will never be available
     *  in a subconnection.
     * @param conn Connection to copy.
     */
    explicit toConnection(const toConnection &conn);
    /** Destroy connection.
     */
    virtual ~toConnection();

    //* Get the options for the connection.
    const std::set
    <QString> &options() const
    {
        return Options;
    }
    /** Try to close all the widgets associated with this connection.
     * @return True if all widgets agreed to close.
     */
    bool closeWidgets(void);
    /** Get username of connection.
     */
    const QString &user() const
    {
        return User;
    }
    /** Get password of connection.
     */
    const QString &password() const
    {
        return Password;
    }
    /** Change password of connection.
     */
    void setPassword(const QString &pwd)
    {
        Password = pwd;
    }
    /** Get host of connection.
     */
    const QString &host() const
    {
        return Host;
    }
    /** Get database of connection.
     */
    const QString &database() const
    {
        return Database;
    }
    /** Get schema of connection.
     */
    const QString &schema() const
    {
        return Schema;
    }
    /**
     * Set connection's current schema
     *
     */
    inline void setSchema(const QString &schema)
    {
        Schema = schema;
    }
    /** Get version of connection.
     */
    const QString &version() const
    {
        return Version;
    }
    /** Get provider of connection.
     */
    const QString &provider() const;

    QString color()
    {
        return Color;
    }

    void setColor(const QString & c)
    {
        Color = c;
    }

    /** Change the current database. Observe that this only changes the record of what is the current database. You will still need
     * to change the database oppinion on what database is the current one.
     */
    void setDatabase(const QString &database)
    {
        Database = database;
    }

    /** Get a description of this connection.
     * @version Include version in returned string.
     */
    virtual QString description(bool version = true) const;

    /** Set if this connection needs to be committed.
     */
    void setNeedCommit(bool needCommit = true)
    {
        NeedCommit = needCommit;
    }
    /**
     * Get information about if the connection has uncommitted data.
     *
     * @return Whether uncommitted data is available.
     */
    bool needCommit(void) const
    {
        return NeedCommit;
    }

    /**
     * Commit connection implementation
     */
    void commit(toConnectionSub *sub);

    /**
     * Rollback connection implementation
     */
    void rollback(toConnectionSub *rollback);

    /**
     * Commit all connections.
     */
    virtual void commit(void);
    /**
     * Rollback all connections.
     */
    virtual void rollback(void);

    /** Parse a query on an actual connection and report any syntax problems encountered.
     * Defaults to not implemented.
     * @param conn Connection to execute on.
     * @param sql SQL to parse
     */
    void parse(const QString &sql);
    /** Parse a query on an actual connection and report any syntax problems encountered.
     * Defaults to not implemented.
     * @param conn Connection to execute on.
     * @param sql SQL to parse
     */
    void parse(const toSQL &sql);

    /** Execute a statement without caring about the result.
     * @param sql SQL to execute
     * @param params Parameters to pass to query.
     */
    void execute(const toSQL &sql,
                 toQList &params);
    /** Execute a statement without caring about the result.
     * @param sql SQL to execute
     * @param params Parameters to pass to query.
     */
    void execute(const QString &sql,
                 toQList &params);
    /** Execute a statement without caring about the result.
     * @param sql SQL to execute
     * @param arg1 Parameters to pass to query.
     */
    void execute(const toSQL &sql,
                 const QString &arg1 = QString::null, const QString &arg2 = QString::null,
                 const QString &arg3 = QString::null, const QString &arg4 = QString::null,
                 const QString &arg5 = QString::null, const QString &arg6 = QString::null,
                 const QString &arg7 = QString::null, const QString &arg8 = QString::null,
                 const QString &arg9 = QString::null);
    /** Execute a statement without caring about the result.
     * @param sql SQL to execute
     * @param arg1 Parameters to pass to query.
     */
    void execute(const QString &sql,
                 const QString &arg1 = QString::null, const QString &arg2 = QString::null,
                 const QString &arg3 = QString::null, const QString &arg4 = QString::null,
                 const QString &arg5 = QString::null, const QString &arg6 = QString::null,
                 const QString &arg7 = QString::null, const QString &arg8 = QString::null,
                 const QString &arg9 = QString::null);

    /** Execute a statement without caring about the result on all open database connections.
     * @param sql SQL to execute
     * @param params Parameters to pass to query.
     */
    void allExecute(const toSQL &sql,
                    toQList &params);
    /** Execute a statement without caring about the result on all open database connections.
     * @param sql SQL to execute
     * @param params Parameters to pass to query.
     */
    void allExecute(const QString &sql,
                    toQList &params);
    /** Execute a statement without caring about the result on all open database connections.
     * @param sql SQL to execute
     * @param arg1 Parameters to pass to query.
     */
    void allExecute(const toSQL &sql,
                    const QString &arg1 = QString::null, const QString &arg2 = QString::null,
                    const QString &arg3 = QString::null, const QString &arg4 = QString::null,
                    const QString &arg5 = QString::null, const QString &arg6 = QString::null,
                    const QString &arg7 = QString::null, const QString &arg8 = QString::null,
                    const QString &arg9 = QString::null);
    /** Execute a statement without caring about the result on all open database connections.
     * @param sql SQL to execute
     * @param arg1 Parameters to pass to query.
     */
    void allExecute(const QString &sql,
                    const QString &arg1 = QString::null, const QString &arg2 = QString::null,
                    const QString &arg3 = QString::null, const QString &arg4 = QString::null,
                    const QString &arg5 = QString::null, const QString &arg6 = QString::null,
                    const QString &arg7 = QString::null, const QString &arg8 = QString::null,
                    const QString &arg9 = QString::null);

    /**
     * Add a object that uses this connection. This is needed to ensure that all widgets
     * that make use of a connection are destroyed when the connection is closed. Usually
     * tool windows need to call this function.
     *
     * @param widget The widget to add to the connection.
     */
    void addWidget(QWidget *widget);

    /**
     * Remove a widget from this connection. Should be called by the destructor of the
     * widget that has called addWidget.
     *
     * @see addWidget
     * @param widget Widget to remove from the widget list.
     */
    void delWidget(QWidget *widget);

    /**
     * Add a statement to be run uppon making new connections.
     * @param sql Statement to run.
     */
    void addInit(const QString &sql);
    /**
     * Remove a statement that was added using @ref addInit.
     */
    void delInit(const QString &sql);
    /**
     * Get a list of the current init strings.
     */
    const std::list<QString>& initStrings() const;

    /** Return a string representation to address an object.
     * @param name The name to be quoted.
     * @param quoteLowercase Enclose in quotes when identifier has lowercase letters
     * @return String addressing table.
     */
    QString quote(const QString &name, const bool quoteLowercase = true);
    /** Perform the opposite of @ref quote.
     * @param name The name to be un-quoted.
     * @return String addressing table.
     */
    QString unQuote(const QString &name);

    /**
     * Get syntax analyzer for connection
     * @return A reference to the syntax analyzer to use for the connection.
     */
    virtual toSyntaxAnalyzer &analyzer();

    /**
     * Get the objects available for the current user from cache. Do not modify the returned list.
     * @param block Indicate wether or not to block until cached objects are available.
     * @return A list of object available for the current user. The list is sorted in
     *         owner and name order.
     */
    ///const QList<toConnection::objectName>& objects(bool block) const;
    QList<toCacheNew::CacheEntry const*> objects(bool block) const;

    /**
     * Reread specified objects from database to the cache. If null is given as value of any
     * parameter - then any value for that parameter is ok in database matches.
     * @param owner only read object belonging to given schema
     * @param type only read objects of given type
     * @param name only read objects with given name.
     * @return true if at least one new object was added to cache
     */
    //bool rereadObjects(const QString &owner = "",
    //                   const QString &type = "",
    //                   const QString &name = "");
    bool rereadObjectNew(const QString &owner = "", const QString &name = "");


    /** Check if cache is available or not.
     * @param type (can be either OBJECTS or SYNONYMS)
     * @param block Block until cache is done.
     * @return True if cache is available.
     */
    ///bool cacheAvailable(toCache::cacheEntryType type, bool block = false);

    /** Non-blocking version of cacheAvailable, used by toMain to update toBackgroundLabel
     */
    bool cacheRefreshRunning() const;

    /**
     * Get a list of the available columns for a table. This function caches the responses
     * and should be fairly fast after the first call. Do not modify the returned list.
     * @param table The table to describe.
     * @param nocache Don't use cached values even if they are available.
     * @return A list of the columns for a table.
     */
    ///const toQDescList& columns(const toConnection::objectName &table, bool nocache = false);
    /// toCacheNew BIG FAT TODO

    /**
     * Get the real object name of a synonym.
     * @param object Object name
     * @param synonym Filled with the synonym used to access the object returned or empty.
     * @param block Block if not done caching object.
     */
    ///const toConnection::objectName& realName(const QString &object, QString &synonym, bool block) const;
    /// toCacheNew BIG FAT TODO

    /**
     * Get the real object name of an object.
     * @param object Object name
     * @param block Block if not done caching object.
     */
    ///const toConnection::objectName &realName(const QString &object, bool block) const;
    /// toCacheNew BIG FAT TODO

    /**
     * Get a list of object names for a owner, typically this is a
     * list of tables for a database or schema.
     *
     */
    ///const std::list<toConnection::objectName> tables(const toConnection::objectName &object, bool nocache = false) const;
    /// toCacheNew BIG FAT TODO

    /**
     * Reread the object and column cache.
     */
    void rereadCache(void);

    /** Try to stop all running queries.
     */
    void cancelAll(void);

    /** Get a list of currently running SQL.
     */
    std::list<QString> running(void);

    /** Get all necessary primary keys for row manipulation.
     */
    std::list<QString> primaryKeys();

    friend class toQuery;
    friend class toConnectionPool;
}; // toConnection

Q_DECLARE_METATYPE(toConnection::exception);

/** This class is used to implement new database providers.
 */

class toConnectionProvider
{
public:
    static std::map<QString, toConnectionProvider *> *Providers;
    static std::map<QString, toConnectionProvider *> *Types;
    QString Provider;
    static void checkAlloc(void);

    /** Get the provider object for a given provider name.
     * @param provider Name of provider.
     * @return Reference to provider object.
     */
    static toConnectionProvider &fetchProvider(const QString &provider);
protected:
    /** Add a provider to the list that this provider can handle.
     */
    void addProvider(const QString &provider);
    /** Remove a provider from the list that this provider can handle.
     */
    void removeProvider(const QString &provider);
public:
    /** Create a new provider with the specified name.
     * @param provider Name of the provider.
     */
    toConnectionProvider(const QString &provider, bool add
                         = true);
    /** Destructor.
     */
    virtual ~toConnectionProvider();

    /** Create an implementation of a connection to this database.
     * @param provider Provider to use for connection.
     * @param conn The connection object to use the created connection.
     * @return A connection implementation created with new.
     */
    virtual toConnection::connectionImpl *provideConnection(const QString &provider,
            toConnection *conn) = 0;
    /** List the available hosts this database provider knows about.
     * @return A list of hosts.
     */
    virtual std::list<QString> providedHosts(const QString &provider);
    /** List the available databases this provider knows about for a given host.
     * @param host Host to return connections for.
     * @param user That might be needed.
     * @param password That might be needed.
     * @return A list of databases available for a given host.
     */
    virtual std::list<QString> providedDatabases(const QString &provider,
            const QString &host,
            const QString &user,
            const QString &pwd) = 0;
    /** Will be called after program has been started and before connections have been opened.
     *  Use for initialization.
     */
    virtual void initialize(void)
    { }

    /** Get a list of options available for the connection. An option with the name
     * "-" indicates a break should be made to separate the rest of the options from the previous
     * options. An option preceded by "*" means selected by default. The * shoul be stripped before
     * before passing it to the connection call.
     */
    virtual std::list<QString> providedOptions(const QString &provider);

    /**
     * Create and return configuration tab for this connectiontype. The returned widget should also
     * be a childclass of @ref toSettingTab.
     *
     * @return A pointer to the widget containing the setup tab for this tool or NULL of
     * no settings are available.
     */
    virtual QWidget *providerConfigurationTab(const QString &provider, QWidget *parent);

    /** Get a list of names for providers.
     */
    static QWidget *configurationTab(const QString &provider, QWidget *parent);
    /** Get a list of names for providers.
     */
    static std::list<QString> providers();
    /** Get a list of options for a given provider.
     */
    static std::list<QString> options(const QString &provider);
    /** Implement a connection for a given provider.
     * @param provider Provider to implement.
     * @param conn Connection to create implementation for.
     */
    static toConnection::connectionImpl *connection(const QString &provider, toConnection *conn);
    /** Get a list of hosts this provider knows about.
     */
    static std::list<QString> hosts(const QString &provider);
    /** Get a list of databases for a given provider and host.
     * @param provider Provider to fetch databases for.
     * @param host Host to fetch databases for.
     * @param user That might be needed.
     * @param password That might be needed.
     * @return List of known databases.
     */
    static std::list<QString> databases(const QString &provider, const QString &host,
                                        const QString &user, const QString &pwd);
    /**
     * Get connection specific settings.
     *
     * Setting names are hierachical separated by ':' instead of '/' usually used
     * in filenames. As an example all settings for the tool 'Example' would be
     * under the 'Example:{settingname}' name. Observe that the settings are stored
     * under the main provider name as passed to the toConnectionProvider constructor.
     *
     * @param tag The name of the configuration setting.
     * @param def Contents of this setting.
     */
//     const QString &config(const QString &tag, const QString &def);
    /**
     * Change connectionspecific setting. Depending on the implementation this can change the
     * contents on disk or not.
     *
     * Setting names are hierachical separated by ':' instead of '/' usually used
     * in filenames. As an example all settings for the tool 'Example' would be
     * under the 'Example:{settingname}' name. Observe that the settings are stored
     * under the main provider name as passed to the toConnectionProvider constructor.
     *
     * @param tag The name of the configuration setting.
     * @param def Default value of the setting, if it is not available.
     */
//     void setConfig(const QString &tag, const QString &value);

    /** Call all initializers
     */
    static void initializeAll(void);
};

#endif
