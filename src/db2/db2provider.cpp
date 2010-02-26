#ifdef HAVE_DB2
#include "db2provider.h"
#include "db2connectionsub.h"
#include "todb2connection.h"


DB2Provider::DB2Provider(void)
        : toConnectionProvider("DB2", true)
{
}

toConnection::connectionImpl * DB2Provider::provideConnection(const QString &, toConnection *conn)
{
    return new DB2Connection(conn);
}

std::list<QString> DB2Provider::providedHosts(const QString &provider)
{
    std::list<QString> ret;
    ret.insert(ret.end(), "localhost");
    return ret;
}

std::list<QString> DB2Provider::providedOptions(const QString &provider)
{
    std::list<QString> ret;
    //ret.insert(ret.end(), "*SSL");
    return ret;
}

std::list<QString> DB2Provider::providedDatabases(const QString &, const QString &, const QString &, const QString &)
{
    std::list<QString> ret;
    // Need implementation
    return ret;
}

void DB2Provider::commit(toConnectionSub * sub)
{
    DB2ConnectionSub * db2 = DB2Connection::toDB2ConnectionSub(sub);
    if (!db2)
        throw "DB2 internal error: 04 Cannot convert common sub to DB2";
    else
        db2->commit();
}

void rollback(toConnectionSub * sub)
{
    DB2ConnectionSub * db2 = DB2Connection::toDB2ConnectionSub(sub);
    if (!db2)
        throw "DB2 internal error: 05 Cannot convert common sub to DB2";
    else
        db2->rollback();
}

toConnectionSub * DB2Provider::createConnection(void)
{
    qDebug() << "DB2Provider createConnection() ret 0";
    return 0;
}

QString DB2Provider::version(toConnectionSub * sub)
{
    DB2ConnectionSub * db2 = DB2Connection::toDB2ConnectionSub(sub);
    if (!db2)
        throw "DB2 internal error: 06 Cannot convert common sub to DB2";
    return db2 ? db2->version() : "n/a";
}

#endif
