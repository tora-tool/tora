#ifndef TOCONNECTIONSUB
#define TOCONNECTIONSUB

#include "core/tocache.h"
#include "core/toqvalue.h"
#include "core/tora_export.h"

#include <QtCore/QDateTime>

class queryImpl;
class toQuery;

/** This class is an abstract definition of an actual connection to a database.
 * Each @ref toConnection object can have one or more actual connections to the
 * database depending on long running queries. Normally you will never need to
 * bother with this class if you aren't creating a new database provider
 * (@ref toConnectionProvider).
 */
class TORA_EXPORT toConnectionSub
{
    public:

        /** Create connection to database. */
        toConnectionSub() : Query(NULL), Broken(false), Initialized(false) {}

        /** Close connection. */
        virtual ~toConnectionSub() {}

        // GETTERS

        /** Query current running on connection or NULL. */
        inline toQuery *query()
        {
            return Query;
        }

        /** Get time when last query on this connection has finished */
        inline QDateTime lastUsed(void)
        {
            return LastUsed;
        }

        // SETTERS

        /** Set query currently running on connection. NULL means none. */
        inline void setQuery(toQuery *query)
        {
            Query = query;
        }

        // ACTIONS

        /** Cancel anything running on this sub. */
        virtual void cancel(void) { };

        /** Close connection. */
        virtual void close(void) = 0;

        virtual void commit(void) = 0;
        virtual void rollback(void) = 0;

        virtual QString version() = 0;

        virtual toQueryParams sessionId() = 0;

        virtual bool hasTransaction();

        virtual queryImpl* createQuery(toQuery *query) = 0;

        /** get additional details about db object */
        virtual toQAdditionalDescriptions* decribe(toCache::ObjectRef const&) = 0;

        /** resolve object name (synonym) */
        virtual toCache::ObjectRef resolve(toCache::ObjectRef const& objectName)
        {
            return objectName;
        };

        void execute(QString const& SQL);

        /** Set time when last query on this connection has finished to "now" */
        inline void setLastUsed(void)
        {
            LastUsed = QDateTime::currentDateTime();
        }

        inline bool isBroken()
        {
            return Broken;
        }

        inline bool isInitialized()
        {
            return Initialized;
        }

        inline void setInitialized(bool initialized)
        {
            Initialized = initialized;
        }

        inline QString const& schema() const
        {
            return Schema;
        }

        inline void setSchema(QString const& schema)
        {
            Schema = schema;
        }

    protected:
        toQuery *Query;
        bool Broken, Initialized;
        QString Schema;
        QDateTime LastUsed; // last time this db connection was actually used
};

#endif
