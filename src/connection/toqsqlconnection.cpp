
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

#include "connection/toqsqlconnection.h"
#include "connection/toqsqlquery.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>
#include "core/tosql.h"

toQSqlConnectionSub::toQSqlConnectionSub(toConnection const& parent, QSqlDatabase const& db, QString const& dbname)
    : ParentConnection(parent)
    , Connection(db)
    , Name(dbname)
    , HasTransactions(false)
{
    ConnectionID = sessionId();
}

toQSqlConnectionSub::~toQSqlConnectionSub()
{

}

void toQSqlConnectionSub::cancel()
{

}

void toQSqlConnectionSub::close()
{

}

void toQSqlConnectionSub::commit()
{
    LockingPtr<QSqlDatabase> ptr(Connection, Lock);
    if (!ptr->commit() && HasTransactions)
    {
        ptr.unlock();
        throwError(QString::fromLatin1("COMMIT"));
    }
}

void toQSqlConnectionSub::rollback()
{
    LockingPtr<QSqlDatabase> ptr(Connection, Lock);
    if (!ptr->rollback() && HasTransactions)
    {
        ptr.unlock();
        throwError(QString::fromLatin1("ROLLBACK"));
    }
}

QString toQSqlConnectionSub::version()
{
    QString ret;
    try
    {
        LockingPtr<QSqlDatabase> ptr(Connection, Lock);

        QSqlQuery query = ptr->exec(toSQL::sql("toQSqlConnection:Version", ParentConnection));
        if (query.next())
        {
            if (query.isValid())
            {
                QSqlRecord record = query.record();
                QVariant val = query.value(record.count() - 1);
                ret = val.toString().toLatin1();
            }
        }
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
    return ret;
}

QString toQSqlConnectionSub::sessionId()
{
    QString ret;
    try
    {
        LockingPtr<QSqlDatabase> ptr(Connection, Lock);

        QSqlQuery query = ptr->exec(toSQL::sql("toQSqlConnection:ConnectionID", ParentConnection));
        if (query.next() && query.isValid())
        {
            QSqlRecord record = query.record();
            QVariant val = query.value(record.count() - 1);
            ret = val.toString().toLatin1();
        }
    }
    catch (std::exception const&e)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception:" << e.what() << std::endl;
    }
    catch (QString const&s)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception:" << qPrintable(s) << std::endl;
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
    return ret;
}

queryImpl* toQSqlConnectionSub::createQuery(toQuery *query)
{
    return new qsqlQuery(query, this);
}

toQAdditionalDescriptions* toQSqlConnectionSub::decribe(toCache::ObjectRef const&)
{
    return NULL;
}

toConnectionSub *toQSqlConnectionImpl::createConnection(void)
{
    static QAtomicInt ID_COUNTER(0);
    int ID = ID_COUNTER.fetchAndAddAcquire(1);

    QString dbName = QString::number(ID);

    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", dbName);
    db.setDatabaseName(parentConnection().database());
    db.open(parentConnection().user(), parentConnection().password());
    if (!db.isOpen())
    {
        QString t = toQSqlConnectionSub::ErrorString(db.lastError());
        QSqlDatabase::removeDatabase(dbName);
        throw t;
    }
    toQSqlConnectionSub *ret = new toQSqlConnectionSub(parentConnection(), db, dbName);
    return ret;
}

void toQSqlConnectionImpl::closeConnection(toConnectionSub *)
{
    throw QString("Not implemented yet toQSqlConnectionImpl::closeConnection");
}

QString toQSqlConnectionSub::ErrorString(const QSqlError &err, const QString &sql)
{
    QString ret;
    if (err.databaseText().isEmpty())
    {
        if (err.driverText().isEmpty())
            ret = QString::fromLatin1("Unknown error");
        else
            ret = err.driverText();
    }
    else
        ret = err.databaseText();
    if (!sql.isEmpty())
        ret += QString::fromLatin1("\n\n") + sql;
    return ret;
}

void toQSqlConnectionSub::throwError(const QString &sql)
{
    LockingPtr<QSqlDatabase> ptr(Connection, Lock);
    throw ErrorString(ptr->lastError(), sql);
}
