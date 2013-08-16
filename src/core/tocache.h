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

#ifndef TOCACHE_NEW_H
#define TOCACHE_NEW_H

#include "core/tora_export.h"

#include <QtCore/QMetaType>
#include <QtCore/QDate>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QSet>
#include <QtCore/QPointer>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtCore/QReadWriteLock>
#include <QtCore/QMutex>
//#include <map>

class QThread;

/** Object cache for a connection. This class is accessed only through toConnection
a could be a nested class of toConnection.
*/
class toConnection;
class toCacheEntryTable;
class toCacheEntryView;
class toCacheEntrySynonym;
class toCacheEntryFunction;
class toCacheEntryPackage;
class toCacheEntryPackageBody;
class toCacheEntryIndex;
class toCacheEntrySequence;
class toCacheEntryTrigger;

class toGlobalSetting;

class toResultModel;

class QDataStream;
class QFileInfo;
class QDir;

typedef QMap<QString, QVariant> toQAdditionalDescriptions;
Q_DECLARE_METATYPE(toQAdditionalDescriptions);

class toCacheWorker: public QObject
{
	Q_OBJECT;

public:
	toCacheWorker(toConnection &conn)
		: m_parentConnection(conn)
	{
		setObjectName(QString::fromLatin1("toCache::cacheObjects"));
	}

	/** Get the parent connection object of this connection. */
	toConnection& parentConnection(void)
	{
		return m_parentConnection;
	}

public slots:
	virtual void process(void);

private:
    	toConnection &m_parentConnection;
};


class TORA_EXPORT toCache : public QObject
{
	Q_OBJECT;

    friend class toConnection;
    friend class toGlobalSetting;
    friend class toCacheWorker;
public:
    /*** Nested types ***/

    /** Object reference, used as a lookup key
    */
    class TORA_EXPORT ObjectRef : public QPair<QString, QString> // 1st schema name, 2nd object name
    {
    	typedef QPair<QString, QString> super;
    public:
    	ObjectRef() : super() {};
    	ObjectRef(const super::first_type &t1, const super::second_type &t2) : super(t1, t2) {};

    	ObjectRef &operator=(const QPair<super::second_type, super::second_type> &other)
        { first = other.first; second = other.second; return *this; }

    	/** convert object reference into a string */
    	QString toString() const;

    	/** convert object reference into a string, use connection traits quoting mechanism*/
    	////QString toString(toConnection &con) const;
    };

    /** Nested class type
    */
#ifdef Q_COMPILER_CLASS_ENUM
    enum CacheEntryType : quint8
#else
    enum CacheEntryType
#endif
    {
        TABLE = 1,
        VIEW,
        SYNONYM,
        PROCEDURE,
        FUNCTION,
        PACKAGE,
        PACKAGE_BODY,
        INDEX,
        SEQUENCE,
        TRIGGER,
        DATABASE, // used by MySQL
        ANY, // used for querying purposes only
        TORA_SCHEMA_LIST, // courious object type - used internaly by TORA, if present browser knows that that schema was read from DB
        //TORA_USER_LIST, // courious object type - used internaly by TORA purpose unknown so far.
        USER,
        OTHER
    };

    /** Contains information about a database object. sub-classed by particular object types
    */
    struct CacheEntry
    {
        ObjectRef name;
        /** Object type
        */
        CacheEntryType type;
        /** Comment about this object
        */
        QString comment;
        /** synonyms (used for faster disk caching...)
        */
        QSet<CacheEntry const*> synonyms;

        /** A date when information about this particular object was last updated
        */
        QDate timestamp;

        /** String representation of cache entry details
         */
        QString details;

        /** true if additional attributes were fetched from DB
        */
        bool described;

        toQAdditionalDescriptions description;

        /** Create an object name with filled in values.
        */
        CacheEntry(const QString &owner, const QString &name, const QString &type, const QString &comment = QString::null);

        /** TODO delete this - this is courious constructor used to hold TORAs internal cache entries */
        CacheEntry(const QString &owner, const QString &objName, toCache::CacheEntryType objType, const QString &objComment);

        CacheEntry(CacheEntry const& other)
            : name(other.name)
            , type(other.type)
            , comment(other.comment)
            , described(false)
        { };

        /** Create an empty object name.
        */
        CacheEntry()
        : type(OTHER)
        , described(false)
        { };

        /** This class will be sub-classed by particular object Types
        */
        virtual ~CacheEntry() {};

        /** Fetch additional information from the DB */
        virtual void describe() {};

        bool operator < (const CacheEntry &) const;
        bool operator == (const CacheEntry &) const;

    }; // struct CacheEntry

    /** This structure is used to describe the resultset of a query.
    */
    struct ColumnDescription
    {
        /** Column name
        */
        QString Name;
        /** Datatype of string.
        */
        QString Datatype;
        /** If column can contain null values.
        */
        bool Null;
        /** Preferred alignment of this kind of value.
        */
        bool AlignRight;
        /** Comment on column (Only filled out in column cache.
        */
        QString Comment;

        QString ToolTip;
    }; // struct ColumnDescription

    /** Objects state - updated by background threads
    */
#ifdef Q_CC_MSVC
    enum CacheState : quint8
#else
    enum CacheState
#endif
    {
        NOT_STARTED        = 1,
        READING_STARTED    = 2,
        READING_FROM_DISK  = 4,
        READING_FROM_DB    = 8,
        DONE               = 16,
        FAILED             = 32
    };

    enum UserListType
    {
        USERS,
        OWNERS
    };

    /** Constructuctors, destructors
    */
    toCache(toConnection &parentConnection, QString const &description);

    ~toCache();

    /** simple cacheEntry factory */
    static CacheEntry* createCacheEntry(const QString &owner, const QString &name, const QString &type, const QString &comment);
    static CacheEntry* createCacheEntry(const QString &owner, const QString &name, CacheEntryType type, const QString &comment);
    static CacheEntry* cloneCacheEntry(CacheEntry const& other);

    //
    // Cache querying - getters
    //

    /** Use synonymMap to resolve object name */
    ObjectRef translateName(ObjectRef const&) const;

    /** Perform direct lookup on object cache - no synonym translation is made
     *  no db lookup is performed
     */
    CacheEntry const* findEntry(ObjectRef const&) const;

    /** Query the cache for entries of particular type
     */
    QList<CacheEntry const *> getEntriesInSchema(QString const& schema, CacheEntryType type = ANY) const;
    QList<CacheEntry const *> getEntriesInSchema(QString const& schema, QString const& type) const;

    /** Cbeck presence of entry in the DB
     */
    bool entryExists(ObjectRef const&e, CacheEntryType entryType = ANY) const;

    /** get list of the all the objects held in the cache */
    QList<CacheEntry const*> entries(bool wait = false) const;

    /** Check for presence of the special type entry - userList
     * listType can be either "users" (all db users) or owners(database objects owners)
     */
    bool userListExists(UserListType listType = USERS) const;

    /** List of database users / database object owners */
    QStringList userList(UserListType listType = USERS) const;

    //
    // Cache update - setters
    //

    /** Query additional information about the entry from the DB
     *  calls toConnectionSub::describe
     */
    CacheEntry const* describeEntry(CacheEntry const*);

    /** add/update new entry into cache */
    void upsertEntry(CacheEntry* e);

    /** add/update a list of objects in cache.
    * This should add any new object to the list as well as remove no longer
    * existing ones - within defined schema
    */
    void upsertSchemaEntries(QString const& schema, QString const& objType, QList<CacheEntry*> const& r);

    /** Clear current list of users and generate a new one */
    void upsertUserList(QList<CacheEntry*> const&r, UserListType listType = USERS);

    //
    // Cache manipulation
    //

    /** Reread the object and column cache.
     * Tries to read the cache from the disk first
     * Starts a new thread which will read all objects and synonyms from the database.
    */
    void readCache();

	/** Reread the object and column cache from database
	* Starts a new thread which will read all objects and synonyms from the database.
	* also: deletes disk cache file
	*/
	void rereadCache();

    /** translate object type name QString("TABLE") => CacheEntryType::TABLE */
    static CacheEntryType cacheEntryType(QString const& objTypeName);

    /** translate object type CacheEntryType::TABLE => QString("TABLE") */
    static QString cacheEntryTypeToString(CacheEntryType objType);

    /** Note: this functions is not 100% correct and should be used for testing purposes only */
    void wait4BGThread();

	/** returns true if the background thread is running - non-blocking
	 * used by toMain to update toBackgroundLabel
	 */
    bool cacheRefreshRunning() const;

private:

    /** setter for cache state */
    void setCacheState(CacheState);

    /** getter for cache state */
    CacheState cacheState() const;

    CacheState state;

    QMap<ObjectRef, CacheEntry const*> entryMap;
    QMap<ObjectRef, CacheEntry const*> synonymMap;
    QMap<QString, CacheEntry const*> columnCache;
    QMap<QString, CacheEntry const*> ownersMap, usersMap;
    bool ownersRead, usersRead;
    toConnection &parentConn;

    /** Return the file used to store cache contents for this connection.
    * @return A string representing a full path and filename of cache file
    */
    QFileInfo cacheFile();

    /** Return the directory storing files (caches) of all connections.
    * @return A string representing a full path to cache store directory
    */
    static QDir cacheDir();

    /** Load cache information for current connection from a file on disk
    * @return True if cache was loaded
    */
    bool loadDiskCache(void);

    /** write disk cache
    */
    void writeDiskCache(void);

    /** remove all the entries from all the maps, Note: caller should lock instance state first */
    void clearCache();

    /** This lock is used by all getters and setters
    an Instance of toCache is shared between multiple connections.
    */
    mutable QReadWriteLock cacheLock;

    /** Instance of this lock is held while the backgound thread runs
     */
    QMutex backgroundThreadLock;

    /** Description of connection for which this instance of cache is used.
    *  This name is used as a filename to store cache content between TOra runs.
    */
    QString ConnectionDescription;

    /** Multiple connections can point onto same toCache instance, the last connection should delete me.
    */
    QAtomicInt refCount;

    QThread *m_threadWorker;
    toCacheWorker *m_cacheWorker;

signals:
       void userListRefreshed(void);
       void refreshCache();
}; // toCache


/** A short representation of list<toCache::ColumnDescription>
*/
typedef QList<toCache::ColumnDescription> toQColumnDescriptionList;
Q_DECLARE_METATYPE(toQColumnDescriptionList);

class toCacheEntryTable : public toCache::CacheEntry
{
public:
    toCacheEntryTable(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

class toCacheEntryView : public toCache::CacheEntry
{
public:
    toCacheEntryView(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

class toCacheEntrySynonym : public toCache::CacheEntry
{
public:
    toCacheEntrySynonym(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

class toCacheEntryProcedure: public toCache::CacheEntry
{
public:
    toCacheEntryProcedure(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

class toCacheEntryFunction: public toCache::CacheEntry
{
public:
    toCacheEntryFunction(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

class toCacheEntryPackage: public toCache::CacheEntry
{
public:
    toCacheEntryPackage(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

class toCacheEntryPackageBody: public toCache::CacheEntry
{
public:
    toCacheEntryPackageBody(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

class toCacheEntryIndex: public toCache::CacheEntry
{
public:
    toCacheEntryIndex(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

class toCacheEntrySequence: public toCache::CacheEntry
{
public:
    toCacheEntrySequence(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

class toCacheEntryTrigger: public toCache::CacheEntry
{
public:
    toCacheEntryTrigger(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

class toCacheEntryDatabase: public toCache::CacheEntry
{
public:
    toCacheEntryDatabase(const QString &owner, const QString &name, const QString &comment = QString::null);

private:
};

// Special "fake" cache entry - not held in entryMap
class toCacheEntryUser: public toCache::CacheEntry
{
public:
    toCacheEntryUser(const QString& user);
private:
};


#endif
