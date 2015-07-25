#ifdef HAVE_DB2

#ifndef DB2PROVIDER_H
#define DB2PROVIDER_H

#include "toconnection.h"


class DB2Provider : public toConnectionProvider
{
    public:

        DB2Provider(void);

        toConnection::connectionImpl * provideConnection(const QString &, toConnection *conn);
        std::list<QString> providedHosts(const QString &provider);
        std::list<QString> providedOptions(const QString &provider);
        std::list<QString> providedDatabases(const QString &, const QString &, const QString &, const QString &);
        void commit(toConnectionSub * sub);
        void rollback(toConnectionSub * sub);
        toConnectionSub *createConnection(void);
        QString version(toConnectionSub * sub);

};


#endif // DB2PROVIDER_H

#endif
