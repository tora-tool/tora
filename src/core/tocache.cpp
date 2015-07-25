
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

#include "core/tocache.h"
#include "core/toconfiguration_new.h"
#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "core/toconnectionsubloan.h"
#include "core/toquery.h"
#include "core/tosql.h"
#include "core/toraversion.h"
#include "core/utils.h"
#include "core/toglobalevent.h"
#include "core/toglobalsettingenum.h"
#include "core/todatabasesetting.h"

#include <QtCore/QtDebug>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QTextStream>
#include <QtCore/QDataStream>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QProgressDialog>
//#include <boost/preprocessor/iteration/detail/local.hpp>

/* This method runs as a separate thread executed from:
 toCache::readObjects(toTask * t)
 */
void toCacheWorker::process() {
	QMutexLocker bLock(&parentConnection().getCache().backgroundThreadLock);

	parentConnection().getCache().setCacheState(toCache::READING_FROM_DISK);
	bool diskloaded = parentConnection().getCache().loadDiskCache();
	if (diskloaded) {
		// TODO: Check if really whole cache was loaded.
		// the file image could contain only some schemas or part of the schema
		parentConnection().getCache().setCacheState(toCache::DONE);
		return;
	}

	parentConnection().getCache().setCacheState(toCache::READING_FROM_DB);
	try {
		toConnectionSubLoan conn(parentConnection());
		toQuery objects(conn
                        , toSQL::sql("toConnection:ListObjectsInDatabase",parentConnection())
                        , toQueryParams());
		while (!objects.eof()) {
			if (parentConnection().Abort) {
				parentConnection().getCache().setCacheState(toCache::FAILED);
				return;
			}
			// TODO #warning  "check connection's parent Abort flag"
			QString owner = (QString)objects.readValue();
			QString name = (QString)objects.readValue();
			QString type = (QString)objects.readValue();
			QString comment = (QString)objects.readValue();
			toCache::CacheEntry *e = toCache::createCacheEntry(owner, name,
					type, comment);
			if (e)
				parentConnection().getCache().upsertEntry(e);
		}
	} catch (toConnection::exception const &exc) {
		parentConnection().getCache().setCacheState(toCache::FAILED);
		TLOG(2, toDecorator, __HERE__) << exc << std::endl;
		return;
	} catch (QString const &exc) {
		parentConnection().getCache().setCacheState(toCache::FAILED);
		TLOG(2, toDecorator, __HERE__) << exc << std::endl;
		return;
	}

	parentConnection().getCache().ownersRead = true;
	parentConnection().getCache().setCacheState(toCache::DONE);
}
;

// Forward declarations
// Allow cache entries to be serialized / de-serialized
QDataStream& operator<<(QDataStream& stream, const toCache::CacheEntry& e);
QDataStream& operator>>(QDataStream& stream, toCache::CacheEntry& e);

QString toCache::ObjectRef::toString() const
{
	if (first.isEmpty())
		return second;
	return QString(first) + '.' + second;
}

toCache::toCache(toConnection &parentConnection, QString const &description)
	: QObject(NULL)
	, ConnectionDescription(description)
	, refCount(1) // we assume that we were created from 1st toConnection
	, state(NOT_STARTED)
	, ownersRead(false)
	, usersRead(false)
	, parentConn(parentConnection)
	, m_threadWorker(new QThread(this))
	, m_cacheWorker(new toCacheWorker(parentConn))
	, m_trie(new QmlJS::PersistentTrie::Trie())
{
	m_threadWorker->setObjectName("toCacheWorker thread");
	m_cacheWorker->moveToThread(m_threadWorker);
	connect(this, SIGNAL(refreshCache()), m_cacheWorker, SLOT(process()));
}

toCache::~toCache()
{
	QMutexLocker bLock(&backgroundThreadLock); // wait till the background thread finishes
	QWriteLocker lock(&cacheLock);
	clearCache();
}

// Static methods

toCache::CacheEntry* toCache::createCacheEntry(const QString &objOwner,	const QString &objName, CacheEntryType objType,	const QString &comment) {
	switch (objType) {
	case TABLE:
		return new toCacheEntryTable(objOwner, objName, comment);
	case VIEW:
		return new toCacheEntryView(objOwner, objName, comment);
	case SYNONYM:
		return new toCacheEntrySynonym(objOwner, objName, comment);
	case PROCEDURE:
		return new toCacheEntryProcedure(objOwner, objName, comment);
	case FUNCTION:
		return new toCacheEntryFunction(objOwner, objName, comment);
	case PACKAGE:
		return new toCacheEntryPackage(objOwner, objName, comment);
	case PACKAGE_BODY:
		return new toCacheEntryPackageBody(objOwner, objName, comment);
	case INDEX:
		return new toCacheEntryIndex(objOwner, objName, comment);
	case SEQUENCE:
		return new toCacheEntrySequence(objOwner, objName, comment);
	case TRIGGER:
		return new toCacheEntryTrigger(objOwner, objName, comment);
	case DATABASE:
		return new toCacheEntryDatabase(objOwner, objName, comment);
	case USER:
		return new toCacheEntryUser(objOwner);
	default:
		return NULL; // Do ignore some object types, like DBLINK or CLUSTER for example
		///throw QString("toCache: Unknown object type %1").arg(objType);
	}
}

toCache::CacheEntry* toCache::createCacheEntry(const QString &objOwner,	const QString &objName, const QString &objTypeStr, const QString &comment)
{
	CacheEntryType objType = cacheEntryType(objTypeStr);
	return createCacheEntry(objOwner, objName, objType, comment);
}
;

toCache::CacheEntry* toCache::cloneCacheEntry(CacheEntry const& other)
{
	return createCacheEntry(other.name.first, other.name.second, other.type, other.comment);
}

// Getters - const methods

toCache::ObjectRef toCache::translateName(ObjectRef const& n) const
{
	// TODO: it is not transitive
	QReadLocker lock(&cacheLock);
	if (synonymMap.contains(n))
		return ObjectRef(synonymMap.value(n)->name);
	else
		return ObjectRef(n);
	//return ObjectRef("", "");
}

toCache::CacheEntry const* toCache::findEntry(toCache::ObjectRef const& o) const
{
	QReadLocker lock(&cacheLock);
	if (o.first.isEmpty())
	{
		// Object owner was not specified - try to use objects "context" (default schema)
		ObjectRef objRef1(o);
		objRef1.first = o.context;
		toCache::CacheEntry const* retval = entryMap.value(objRef1, NULL);
		if (retval)
			return retval;

		toConnectionSubLoan conn(parentConn);
		ObjectRef objRef2 = conn->resolve(o);
		return entryMap.value(objRef2, NULL);
	}
	return entryMap.value(o, NULL);
}

QStringList toCache::completeEntry(QString const& tab) const
{
	using namespace QmlJS::PersistentTrie;
	return m_trie->complete(tab, "", LookupFlags(CaseInsensitive));
}

QList<toCache::CacheEntry const*> toCache::getEntriesInSchema(QString const& schema, CacheEntryType type) const
{
	QReadLocker lock(&cacheLock);
	QString schemaU = schema.toUpper();
	QList<toCache::CacheEntry const*> retval;

	QList<ObjectRef> objs = entryMap.keys(); // TODO there must be a better way of searching QMap (do not copy keys)
	Q_FOREACH(ObjectRef const & o, objs)
	{
		if ((entryMap.value(o)->type == type || type == toCache::ANY) && o.first == schemaU)
			retval.append(entryMap.value(o));
	}
	return retval;
}

QList<toCache::CacheEntry const*> toCache::getEntriesInSchema(QString const& schema, QString const& type) const
{
	// No lock here. the 2nd call to getEntriesInSchema will lock this.
	CacheEntryType t = cacheEntryType(type);
	if (t != toCache::OTHER)
		return getEntriesInSchema(schema, t);
	else
		return QList<toCache::CacheEntry const*>();
}

bool toCache::entryExists(ObjectRef const&e, toCache::CacheEntryType entryType) const
{
	QReadLocker lock(&cacheLock);
	if (entryMap.contains(e)&& (entryType == ANY || entryMap.value(e)->type == entryType))
		return true;
	return false;
}

QList<toCache::CacheEntry const*> toCache::entries(bool wait) const
{
	QReadLocker lock(&cacheLock);
	return entryMap.values();
}

bool toCache::userListExists(UserListType listType) const {
	if (listType == USERS)
		return usersRead;
	else
		return ownersRead;
}

/** List of database users / database object owners */
QStringList toCache::userList(UserListType listType) const
{
	QReadLocker lock(&cacheLock);
	QStringList retval;
	QMap<QString, CacheEntry const*> const& map = (listType == USERS ? usersMap : ownersMap);
	Q_FOREACH(CacheEntry const * e, map) {
		retval.append(e->name.first);
	}
	return retval;
}

// Setters - non-const methods
toCache::CacheEntry const* toCache::describeEntry(toCache::CacheEntry const*e)
{
	// 1st check if the entry exists in the cache
	{
		if (! entryExists(e->name))
			throw QString("Entry not found in toCache: %1").arg(e->name.toString());
	}

	toCache::CacheEntry *entry = const_cast<toCache::CacheEntry*>(e);

	try {
		QWriteLocker lock(&cacheLock);
		toConnectionSubLoan conn(parentConn);


		toQAdditionalDescriptions* l = conn->decribe(e->name);
		entry->description.clear();
		entry->description = *l;

		delete l;
	} catch (...) {
		return NULL;
	}	
	return entry;
}

void toCache::upsertEntry(toCache::CacheEntry* e)
{
	QWriteLocker lock(&cacheLock);
	switch (e->type) {
	case SYNONYM:
		synonymMap.insert(e->name, e);
		// no break
	case TABLE:
	case VIEW:
		m_trie->insert(e->name.second);
		// no break
	case PROCEDURE:
	case FUNCTION:
	case PACKAGE:
	case PACKAGE_BODY:
	case INDEX:
	case SEQUENCE:
	case TRIGGER:
	case DATABASE:
	case TORA_SCHEMA_LIST: {
		QString const& schema = e->name.first;

		CacheEntry const* oldValue = entryMap.value(e->name, NULL);
		if (oldValue)
			delete oldValue;
		entryMap.insert(e->name, e);

		if (!usersMap.contains(schema)) {
			usersMap.insert(schema, new toCacheEntryUser(schema));
			emit userListRefreshed();
		}

		if (!ownersMap.contains(e->name.first)) {
			ownersMap.insert(schema, usersMap.value(schema));
		}
	}
		break;
	case USER: {
		CacheEntry const* oldValue = usersMap.value(e->name.first, NULL);
		if (oldValue == NULL) {
			usersMap.insert(e->name.first, e);
		} else {
			delete e;
		}
	}
		break;
	default:
		// HERE we ignore directories, dblinks, ... OTHER
		break;
	}
}

void toCache::upsertSchemaEntries(QString const& schema, QString const& objType, QList<toCache::CacheEntry*> const& rows)
{
	QWriteLocker lock(&cacheLock);
	CacheEntryType type = cacheEntryType(objType);
	if (type == OTHER)
		throw QString("toCache: Unknown object type OTHER");
	if (type == ANY)
		throw QString("toCache: Unsupported object type ANY");

	// Clear whole schema
	QList<ObjectRef> objs = entryMap.keys(); // TODO there must be a better way of deleting from QMap
	Q_FOREACH(ObjectRef const & o, objs)
	{
		if (o.first == schema && entryMap.value(o)->type == type)
			entryMap.remove(o);
	}

	// Add new entries in the schema
	Q_FOREACH(CacheEntry * e, rows)
	{
		entryMap.insert(e->name, e);
	}

	// Check if user/owner exists
	{
		if (!usersMap.contains(schema)) {
			usersMap.insert(schema, new toCacheEntryUser(schema));
		}

		if (!rows.empty() && !ownersMap.contains(schema)) {
			ownersMap.insert(schema, usersMap.value(schema));
		}
	}

	if (type == SYNONYM) {
		QList<ObjectRef> objs = synonymMap.keys(); // TODO there must be a better way of deleting from QMap
		Q_FOREACH(ObjectRef const & o, objs)
		{
			if (o.first == schema)
				synonymMap.remove(o);
		}

		Q_FOREACH(CacheEntry * e, rows)
		{
			synonymMap.insert(e->name, e);
		}
	}
}

/** Clear current list of users and generate a new one */
void toCache::upsertUserList(QList<CacheEntry*> const& r, UserListType listType)
{
	QWriteLocker lock(&cacheLock);
	QString username;
	if (listType == USERS)
	{
		Q_FOREACH(CacheEntry const * e, r)
		{
			username = e->name.first;
			if (usersMap.contains(username)) // Do not replace existing entry
				delete e;
			else
				usersMap.insert(username, e);
		}
	} else { // listType == OWNERS
		Q_FOREACH(CacheEntry const * e, r)
		{
			username = e->name.first;
			if (ownersMap.contains(username))
			{
				delete e;
			} else {
				CacheEntry const* userPointer = usersMap.value(username, NULL);
				if (userPointer) {
					ownersMap.insert(username, userPointer);
					delete e;
				} else
					ownersMap.insert(username, e);
			}
		}
	}

	if (listType == USERS)
		usersRead = true;
	else
		ownersRead = true;
}

void toCache::setCacheState(CacheState c) {
	QWriteLocker lock(&cacheLock);
	state = c;
}
;

toCache::CacheState toCache::cacheState() const {
	QReadLocker lock(&cacheLock);
	return state;
}

void toCache::rereadCache() {
	/** delete cache file to force reload
	 */
	QFileInfo filename(cacheFile());
	if (filename.isFile())
		QFile::remove(filename.absoluteFilePath());

	readCache();
}

void toCache::readCache() {
	if (toConfigurationNewSingle::Instance().option(ToConfiguration::Database::ObjectCacheInt).toInt() == NEVER)
	{
		QWriteLocker lock(&cacheLock);
		clearCache();
		return;
	}

	if (cacheRefreshRunning()) {
		Utils::toStatusMessage(
				qApp->translate("toConnection",
						"Not done caching objects, can not clear unread cache"));
		return;
	}

	try {
		setCacheState(toCache::READING_STARTED);
		QMutexLocker bLock(&backgroundThreadLock);
		if(!m_threadWorker->isRunning())
			m_threadWorker->start();
		///(new toThread(t))->start();
		emit refreshCache();
	} catch (...) {
		state = FAILED;
		return;
	}

	/// TODO migrate this too, probably somewhere in toConnection
	/// TODO turn this into emit
	/// TODO !RACE! here we call here we call one thread from the other
	// notify main window about the change of cache's state
	toGlobalEventSingle::Instance().checkCaching();
}

void toCache::wait4BGThread() {
	CacheState s = cacheState();
	while (s == toCache::READING_STARTED) // This state is set for a short period of time till the bg thread really starts
	{
		QThread::yieldCurrentThread();
		s = CacheState();
	}
	{
		QMutexLocker bLock(&backgroundThreadLock);
	}
}

//toResultModel* toCache::describeDBObjectReturnModel(ObjectRef const&)
//{
//	return NULL;
//}
//
//toCache::CacheEntry* toCache::describeDBObjectReturnEntry(ObjectRef const&)
//{
//	return NULL;
//}

QDir toCache::cacheDir() {
	QString home(QDir::homePath());
	QString dirname(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::CacheDirectory).toString());

	if (dirname.isEmpty()) {
#ifdef Q_OS_WIN32
		if (getenv("TEMP"))
			dirname = QString(getenv("TEMP"));
		else
#endif
			dirname = QString(home);
		dirname += "/.tora_cache";
	}
	return dirname;
} // cacheDir

QFileInfo toCache::cacheFile() {
	QString ret(ConnectionDescription.trimmed());
	// using instantclient connectionstrins can result in file name like this:
	//     isepl_global_stage@//oraclexe11:1521/xe
	// which is invalid. Just remove "/" or replace it with something safer.
	// colon ":" is invalid char for filename on Windows
	ret = ret.replace("/", "_");
	ret = ret.replace(":", "~");
	ret += ".bin";

	return QFileInfo(cacheDir(), ret);
} // cacheFile

void toCache::writeDiskCache() {
	QMutexLocker bLock(&backgroundThreadLock);

	if (cacheState() != toCache::DONE) {
		return;
	}

	if (!toConfigurationNewSingle::Instance().option(ToConfiguration::Database::ObjectCacheInt).toInt())
		return;

	QFileInfo fileInfo(cacheFile());
	QDir dir(cacheDir());

	if (!dir.exists())
		dir.mkdir(dir.absolutePath());

	QString version = QString::fromLatin1(TORAVERSION);

	QFile file(fileInfo.absoluteFilePath());
//TODO #warn "throw something here"
	if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate))
		return;

	{
		QReadLocker lock(&cacheLock);

		QDataStream out(&file);
		out << (quint8) 0;
		out << version; // Tora version
		out << (quint8) state; // cache state
		out << ownersRead;
		out << usersRead;
		out << ConnectionDescription;
		file.flush();

		out << (quint32) usersMap.size();
		out << (quint32) entryMap.size();

		QList<CacheEntry const*> vUsersMap = usersMap.values();
		Q_FOREACH(CacheEntry const*e, vUsersMap) {
			out << (*e);
		}

		QList<CacheEntry const*> vEntryMap = entryMap.values();
		Q_FOREACH(CacheEntry const*e, vEntryMap) {
			out << (*e);
		}

		file.seek(0L);
		out << (quint8) 1;
		file.flush();
		file.close();
	}
}

bool toCache::loadDiskCache() {
	if (!toConfigurationNewSingle::Instance().option(ToConfiguration::Database::ObjectCacheInt).toInt())
		return false;

	QFileInfo fileInfo(cacheFile());
	QDir dir(cacheDir());

	if (!dir.exists())
		dir.mkdir(dir.absolutePath());

	QDateTime today;

	if (!fileInfo.isReadable())
		return false;

	if (fileInfo.lastModified().addDays(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::CacheTimeout).toInt()) < today)
		return false;

	clearCache();

//#warn TODO "throw something here
	quint32 usersMapSize, entryMapSize;
	QFile file(fileInfo.absoluteFilePath());
	file.open(QIODevice::ReadOnly);

	quint8 s1, s2;
	bool oRead, uRead;
	QString version;

	QDataStream in(&file);
	{
		QWriteLocker lock(&cacheLock);

		in >> s1;
		in >> version; // Tora version
		in >> s2; // cache state
		in >> oRead;
		in >> uRead;
		in >> ConnectionDescription;

		// Assume the cache file is corrupted if
		// - 1st byte != 0x01 -- see writeDistCache() for details
		// - application version differs
		// - cache state != toCache::DONE
		if (s1 != 1 || version != QString::fromLatin1(TORAVERSION)
				|| s2 != toCache::DONE) {
			file.close();
			file.remove();
			return false;
		}

		in >> usersMapSize;
		in >> entryMapSize;
	}

	for (quint32 i = 0; i < usersMapSize; i++) {
		CacheEntry e, *cloned;
		in >> e;
		cloned = cloneCacheEntry(e);
		if (cloned != NULL)
			upsertEntry(cloned);
	}

	for (quint32 i = 0; i < entryMapSize; i++) {
		CacheEntry e, *cloned;
		in >> e;
		cloned = cloneCacheEntry(e);
		if (cloned != NULL)
			upsertEntry(cloned);
	}

	state = (CacheState) s2;
	usersRead = uRead;
	ownersRead = oRead;

	file.close();
	return true;
}

/**
 * private functions
 */
void toCache::clearCache() {
	QList<CacheEntry const*> v = entryMap.values();
	Q_FOREACH(CacheEntry const * e, v) {
		delete e;
	}

	entryMap.clear();
	synonymMap.clear();
	columnCache.clear();

	QList<CacheEntry const*> u = usersMap.values();
	Q_FOREACH(CacheEntry const * e, u) {
		delete e;
	}
	ownersMap.clear();
	usersMap.clear();

	m_trie = QSharedPointer<QmlJS::PersistentTrie::Trie>(new QmlJS::PersistentTrie::Trie());
}
;

/*static*/toCache::CacheEntryType toCache::cacheEntryType(
		QString const& objType) {
	if (objType == "TABLE")
		return TABLE;
	else if (objType == "VIEW")
		return VIEW;
	else if (objType == "SYNONYM")
		return SYNONYM;
	else if (objType == "PROCEDURE")
		return PROCEDURE;
	else if (objType == "FUNCTION")
		return FUNCTION;
	else if (objType == "PACKAGE")
		return PACKAGE;
	else if (objType == "PACKAGE BODY")
		return PACKAGE_BODY;
	else if (objType == "INDEX")
		return INDEX;
	else if (objType == "SEQUENCE")
		return SEQUENCE;
	else if (objType == "TRIGGER")
		return TRIGGER;
	else if (objType == "DATABASE")
		return DATABASE;
	else
		return OTHER;
}

/*static*/QString toCache::cacheEntryTypeToString(
		toCache::CacheEntryType objType) {
	switch (objType) {
	case TABLE:
		return "TABLE";
	case VIEW:
		return "VIEW";
	case SYNONYM:
		return "SYNONYM";
	case PROCEDURE:
		return "PROCEDURE";
	case FUNCTION:
		return "FUNCTION";
	case PACKAGE:
		return "PACKAGE";
	case PACKAGE_BODY:
		return "PACKAGE BODY";
	case INDEX:
		return "INDEX";
	case SEQUENCE:
		return "SEQUENCE";
	case TRIGGER:
		return "TRIGGER";
	case DATABASE:
		return "DATABASE";
	default:
		//return NULL;
		throw QString("toCacheNew: Unknown object type %1").arg(
				(quint32) objType);
	}
}

bool toCache::cacheRefreshRunning() const {
	return cacheState() & ( READING_STARTED | READING_FROM_DISK | READING_FROM_DB);
}

toCache::CacheEntry::CacheEntry(const QString &owner, const QString &objName,
		const QString &objType, const QString &objComment) :
		name(ObjectRef(owner, objName, owner)), type(cacheEntryType(objType)), comment(
				objComment), timestamp(QDate::currentDate()), described(false) {
	if (type == OTHER)
		throw QString("toCache: Unknown object type %1").arg(objType);
}
;

/** TODO delete this - this is curious constructor used to hold TORAs internal cache entries (like SCHEMA_LIST) */
toCache::CacheEntry::CacheEntry(const QString &owner, const QString &objName,
		toCache::CacheEntryType objType, const QString &objComment) :
		name(ObjectRef(owner, objName, owner)), type(objType), comment(objComment), timestamp(
				QDate::currentDate()), described(false) {
	if (type == OTHER)
		throw QString("toCache: Unknown object type %1").arg((quint8) objType);
}
;

bool toCache::CacheEntry::operator <(const toCache::CacheEntry &other) const {
	if (type < other.type)
		return true;
	if (type == other.type && name.first < other.name.first)
		return true;
	if (type == other.type && name.first == other.name.first
			&& name.second < other.name.second)
		return true;
	return false;
}
;

bool toCache::CacheEntry::operator ==(const toCache::CacheEntry &other) const {
	return type == other.type && name == other.name;
}
;

QDataStream& operator<<(QDataStream& stream, const toCache::CacheEntry& e) {
	stream << e.name << (quint8) e.type << e.comment << e.timestamp
			<< e.details;
	return stream;
}

QDataStream& operator>>(QDataStream& stream, toCache::CacheEntry& e) {
	quint8 type;

	stream >> e.name >> type >> e.comment >> e.timestamp >> e.details;

	e.type = (toCache::CacheEntryType) type;
	e.described = false;
	return stream;
}

toCacheEntryTable::toCacheEntryTable(const QString &owner, const QString &name,
		const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::TABLE, comment) {
}
;

toCacheEntryView::toCacheEntryView(const QString &owner, const QString &name,
		const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::VIEW, comment) {
}
;

toCacheEntrySynonym::toCacheEntrySynonym(const QString &owner,
		const QString &name, const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::SYNONYM, comment) {
}
;

toCacheEntryProcedure::toCacheEntryProcedure(const QString &owner,
		const QString &name, const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::PROCEDURE, comment) {
}
;

toCacheEntryFunction::toCacheEntryFunction(const QString &owner,
		const QString &name, const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::FUNCTION, comment) {
}
;

toCacheEntryPackage::toCacheEntryPackage(const QString &owner,
		const QString &name, const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::PACKAGE, comment) {
}
;

toCacheEntryPackageBody::toCacheEntryPackageBody(const QString &owner,
		const QString &name, const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::PACKAGE_BODY, comment) {
}
;

toCacheEntryIndex::toCacheEntryIndex(const QString &owner, const QString &name,
		const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::INDEX, comment) {
}
;

toCacheEntrySequence::toCacheEntrySequence(const QString &owner,
		const QString &name, const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::SEQUENCE, comment) {
}
;

toCacheEntryTrigger::toCacheEntryTrigger(const QString &owner,
		const QString &name, const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::TRIGGER, comment) {
}
;

toCacheEntryDatabase::toCacheEntryDatabase(const QString &owner,
		const QString &name, const QString &comment) :
		toCache::CacheEntry(owner, name, toCache::DATABASE, comment) {
}
;

toCacheEntryUser::toCacheEntryUser(const QString &schema) :
		toCache::CacheEntry(schema, schema, toCache::USER, "") {
}
;
