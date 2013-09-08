
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

#include "core/tora_export.h"
#include "core/toquery.h"
#include "core/tologger.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtGui/QWidget>
#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QMetaType>
#include <QtCore/QDateTime>
#include <QtCore/QAtomicInt>

class toConnectionSub;
class toConnectionTraits;
class toCache;           // defined in tocache.h
class toQuery;           // defined in toquery.h
class queryImpl;         // defined in toqueryimpl.h
class toConnectionSubLoan;
class toSQL;

class QMenu;

/** Represent a database connection in TOra. Observe that this can mean several actual
 * connections to the database as queries that are expected to run a long time are sometimes
 * executed in their own connection to make sure the interface doesn't lock up for a long time.
 *
 * NOTE: this class has no virtual methods and is not designed to be sub-classed.
 * Insted of this it uses helper class @ref toConnetionsImpl. Sub-class of toConnectionImpl is provided by @ref toConnectionProvider.
 *
 * This class serves as a wrapper around multiple instances of @ref toConnectionSub. An instance of @ref toConnectionSub
 * is returned by sub-class of @ref toConnetionsImpl.
 */
class toConnection : public QObject
{
    Q_OBJECT;

    friend class toQuery;
    friend class toCache;
	friend class toCacheWorker;

public:
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
                 const QSet<QString> &options);

    /** Create a copy of a connection. Will not cache objects, so objects will never be available
     *  in a subconnection.
     * @param conn Connection to copy.
     */
    explicit toConnection(const toConnection &conn);

    /** Destroy connection.
     */
    virtual ~toConnection();

    // GETTERS
    /** Get provider name of connection. */
    inline QString const& provider() const
    {
        return Provider;
    }
    inline bool providerIs(const char* prov) const
    {
        return Provider == prov;
    }

    /** Get the options for the connection. */
    inline QSet<QString> const& options() const
    {
        return Options;
    }

    /** Get username of connection. */
    inline QString const& user() const
    {
        return User;
    }

    /** Get password of connection.  */
    inline QString const& password() const
    {
        return Password;
    }

    /** Get host of connection. */
    inline QString const& host() const
    {
        return Host;
    }

    /** Get database of connection. */
    inline QString const& database() const
    {
        return Database;
    }

    /** Get schema of connection. */
    inline QString const& schema() const
    {
        return Schema;
    }

    /** Get color name for connection. */
    inline QString const& color() const
    {
        return Color;
    }

    /** Get version string for connection. */
    inline const QString &version() const
    {
        return Version;
    }

    /** Get a description of this connection.
     * @version Include version in returned string.
     */
    QString description(bool version = true) const;

    /** Get a list of the current init strings. */
    QList<QString> initStrings() const;

    /**
     * Get information about if the connection has uncommitted data.
     * @return Whether uncommitted data is available.
     */
    inline bool needCommit(void) const
    {
        return NeedCommit;
    }

    /** Get a list of currently running SQLs */
    virtual QList<QString> running(void) const;

    /** Return the connection most closely associated with a widget. Currently connections are
    * only stored in toToolWidgets.
    * @return Reference toConnection object closest to the current.
    */
    static toConnection& currentConnection(QObject *widget);

    inline toCache& getCache()
    {
        return *pCache;
    }

    inline toCache const& getCache() const
    {
        return *pCache;
    }

    inline toConnectionTraits const& getTraits() const
    {
        return *pTrait;
    }

    // SETTERS

    /** Change password of connection. */
    inline void setPassword(QString const& pwd)
    {
        Password = pwd;
    }

    /** Set connection's current schema. */
    void setSchema(QString const & schema);

    /** set connections' color */
    inline void setColor(QString const& color)
    {
        Color = color;
    }

    /** Set if this connection needs to be committed. */
    void setNeedCommit(bool needCommit = true)
    {
        NeedCommit = needCommit;
    }

    // ACTIONS
    /** Execute a statement without caring about the result.
     * @param sql SQL to execute
     * @param params Parameters to pass to query.
     */
    void allExecute(QString const& sql);
    void allExecute(toSQL const& sql);

    /** Commit connection implementation */
    void commit(toConnectionSub *sub);

    /** Commit all connections. */
    void commit(void);

    /** Rollback connection implementation */
    void rollback(toConnectionSub *rollback);

    /** Rollback all connections. */
    void rollback(void);

    /** Try to close all the widgets associated with this connection.
     * @return True if all widgets agreed to close.
     */
    bool closeWidgets(void);

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

    void connectionsMenu(QMenu*);

    /**
     * Add a statement to be run uppon making new connections.
     * @param sql Statement to run.
     */
    void setInit(const QString &key, const QString &sql);

    /** Remove a statement that was added using @ref addInit. */
    void delInit(const QString &key);

    /** Try to stop all running queries. */
    void cancelAll(void);

    // NESTED CLASSES - types

    /** Class that could be used to throw exceptions in connection errors. Must use if you
     * want to indicate error offset.
     */
    class exception : public QString
    {
        int Offset, Line, Column;
    public:
        /** Create an empty exception */
        inline exception() : QString(), Offset(-1), Line(-1), Column(-1) {}

        /** Create an exception with a string description. */
        inline exception(const QString &str, int offset = -1, int line = -1, int column = -1)
        	: QString(str)
        	, Offset(offset)
        	, Line(line)
        	, Column(column)
        {}

        /** Get the offset of the error of the current statement. */
        inline int offset(void) const
        {
            return Offset;
        }

        inline int line() const
        {
            return Line;
        }

        inline int column() const
        {
            return Column;
        }
    };

    /** This class is an abstract baseclass to actually implement the communication with the
     * database(@ref toConnectionProvider).
     * (See also @ref toQuery::queryImpl and @ref toConnectionProvider)
     */
    class connectionImpl
    {
        toConnection &_pConnection;
    public:
        /** Get the parent connection object of this connection. */
        toConnection& parentConnection(void)
        {
            return _pConnection;
        }

        /** Create a new connection implementation for a connection.
         * @param conn Connection to implement.
         */
        connectionImpl(toConnection &conn) : _pConnection(conn) {}

        /** Destructor. */
        virtual ~connectionImpl() {}

        /** Create a new connection to the database. */
        virtual toConnectionSub* createConnection(void) = 0;

        /** Close a connection to the database. */
        virtual void closeConnection(toConnectionSub *) = 0;
    };

protected:
    bool Abort;
    mutable QMutex ConnectionLock;

private:
    toConnectionSub* borrowSub();
    void putBackSub(toConnectionSub*);
    friend class toConnectionSubLoan;

    toConnectionSub* addConnection(void);
    void closeConnection(toConnectionSub *sub);

    QString Provider;
    QString User;
    QString Password;
    QString Host;
    QString Database;
    QString Schema;
    QString Version;
    QString Color;
    QList<QPointer<QWidget> > Widgets;
    QMap<QString, QString> InitStrings; // Key, SQL
    QSet<QString> Options;
    QSet<toConnectionSub*> Connections, LentConnections;
    connectionImpl *pConnectionImpl;
    toConnectionTraits *pTrait;
    toCache *pCache;
    QAtomicInt LoanCnt;
    friend class toResultModel;

    bool NeedCommit;
}; // toConnection

Q_DECLARE_METATYPE(toConnection::exception);

#endif

