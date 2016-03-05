
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

#include "connection/toqmysqlquery.h"
#include "connection/toqmysqlprovider.h"
#include "core/tosql.h"
#include "core/tocache.h"
#include "core/utils.h"
#include "parsing/tsqllexer.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>

static toSQL SQLCancel("toQSqlConnection:Cancel",
                       "KILL :f1",
                       "Cancel a connection given it's connection ID",
                       "0323",
                       "QMYSQL");

static toSQL SQLCancelM5("toQSqlConnection:Cancel",
                         "KILL QUERY :f1",
                         "",
                         "0500",
                         "QMYSQL");

QList<QString> mysqlQuery::extraData(const toQSqlProviderAggregate &aggr)
{
    QList<QString> ret;
    const QList<toCache::CacheEntry const*> &objects = query()->connection().getCache().entries(false);
    for (QList<toCache::CacheEntry const*>::const_iterator i = objects.begin(); i != objects.end(); i++)
    {
        if ((*i)->type == toCache::DATABASE && aggr.Type == toQSqlProviderAggregate::AllDatabases)
        {
            ret << (*i)->name.first;
        }
        else if ((*i)->type == toCache::TABLE)
        {
            if (aggr.Type == toQSqlProviderAggregate::AllTables ||
                    (aggr.Type == toQSqlProviderAggregate::CurrentDatabase && (*i)->name.first == query()->connection().user()) ||
                    (aggr.Type == toQSqlProviderAggregate::SpecifiedDatabase && (*i)->name.first == aggr.Data))
                ret << ((*i)->name.first + "." + (*i)->name.second);
        }
    }
    return ret;
}

QSqlQuery* mysqlQuery::createQuery(const QString &sql)
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);

    QSqlQuery *ret = new QSqlQuery(*ptr);
    ret->setForwardOnly(true);

    if (!query()->params().empty())
    {
        QString s = stripBinds(query()->sql());
        bool prepared = ret->prepare(s);
        bindParam(ret, query()->params());
        bool executed = ret->exec();
    }
    else
    {
        bool executed = ret->exec(sql);
    }
    return ret;
}

mysqlQuery::mysqlQuery(toQueryAbstr *query, toQMySqlConnectionSub *conn)
    : qsqlQuery(query, conn)
    , Query(NULL)
    , Connection(conn)
    , CurrentColumn(0)
    , EOQ(true)
{
}

mysqlQuery::~mysqlQuery()
{
    delete Query;
}

void mysqlQuery::execute(void)
{
    Query = createQuery(query()->sql());
    checkQuery();
}

void mysqlQuery::execute(QString const& sql)
{
    Query = createQuery(sql);
    checkQuery();
}
void mysqlQuery::cancel(void)
{
    if (!Connection->ConnectionID.isEmpty())
    {
        try
        {
            toConnection const &conn = query()->connection();
            const QString &sql = toSQL::sql(SQLCancel, conn);
            if (!sql.isEmpty() && sql != "native")
            {
                toConnectionSubLoan c(const_cast<toConnection&>(conn));
                toQuery(c, sql, toQueryParams() << Connection->ConnectionID);
            }
            else
            {
                // don't lock here or deadlock while waiting
                // for query to finish
                QSqlDatabase *c = const_cast<QSqlDatabase *>(&(Connection->Connection));
                //TODO PgSQL only? native_cancel(c->driver());
            }
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
        }
    }
}

toQValue mysqlQuery::readValue(void)
{
    if (!Query)
        throw toConnection::exception(QString::fromLatin1("Fetching from not executed query"));
    if (EOQ)
        throw toConnection::exception(QString::fromLatin1("Tried to read past end of query"));

    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
    QVariant retval;
    {
        retval = Query->value(CurrentColumn);
        if (Query->isNull(CurrentColumn))
            retval.clear();
    }

    CurrentColumn++;
    if (CurrentColumn == (unsigned int) Record.count())
    {
        CurrentColumn = 0;
        EOQ = !Query->next();
    }
    if (EOQ)
    {
        delete Query;
        Query = NULL;
    }

    return toQValue::fromVariant(retval);
}

bool mysqlQuery::eof(void)
{
    return EOQ;
}

unsigned long mysqlQuery::rowsProcessed(void)
{
    try
    {
        LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock, true);

        if (!Query)
            return 0L;
        return Query->numRowsAffected();
    }
    catch (...)
    {
        return 0L;
    }
}

unsigned mysqlQuery::columns(void)
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
    return Record.count();
}

toQColumnDescriptionList mysqlQuery::describe(void)
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
    toQColumnDescriptionList ret;
    if (Query && Query->isSelect())
    {
        ret = describe(Query->record());
    }
    return ret;
}

void mysqlQuery::checkQuery(void) // Must *not* call while locked
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
    if (!Query->isActive())
    {
        QString msg = QString::fromLatin1("Query not active ");
        msg += Query->lastQuery();
        throw toConnection::exception(toQMySqlConnectionSub::ErrorString(Query->lastError(), msg));
    }

    if (Query->isSelect())
    {
        Record = Query->record();
        EOQ = !Query->next();
        CurrentColumn = 0;
    }
    else
    {
        EOQ = true;
    }
}

toQColumnDescriptionList mysqlQuery::describe(QSqlRecord record)
{
    ColumnDescriptions.clear();
    for (unsigned int i = 0; i < record.count(); i++)
    {
        toCache::ColumnDescription desc;
        desc.AlignRight = false;
        desc.Name = record.fieldName(i);
        int size = 1;
        QSqlField info = record.field(desc.Name);
        switch (info.typeID())
        {
            case FIELD_TYPE_DECIMAL:
                desc.Datatype = QString::fromLatin1("DECIMAL");
                break;
            case FIELD_TYPE_TINY:
                desc.Datatype = QString::fromLatin1("TINY");
                break;
            case FIELD_TYPE_SHORT:
                desc.Datatype = QString::fromLatin1("SHORT");
                break;
            case FIELD_TYPE_LONG:
                desc.Datatype = QString::fromLatin1("LONG");
                break;
            case FIELD_TYPE_FLOAT:
                desc.Datatype = QString::fromLatin1("FLOAT");
                break;
            case FIELD_TYPE_DOUBLE:
                desc.Datatype = QString::fromLatin1("DOUBLE");
                break;
            case FIELD_TYPE_NULL:
                desc.Datatype = QString::fromLatin1("NULL");
                break;
            case FIELD_TYPE_TIMESTAMP:
                desc.Datatype = QString::fromLatin1("TIMESTAMP");
                break;
            case FIELD_TYPE_LONGLONG:
                desc.Datatype = QString::fromLatin1("LONGLONG");
                break;
            case FIELD_TYPE_INT24:
                desc.Datatype = QString::fromLatin1("INT23");
                break;
            case FIELD_TYPE_DATE:
                desc.Datatype = QString::fromLatin1("DATE");
                break;
            case FIELD_TYPE_TIME:
                desc.Datatype = QString::fromLatin1("TIME");
                break;
            case FIELD_TYPE_DATETIME:
                desc.Datatype = QString::fromLatin1("DATETIME");
                break;
            case FIELD_TYPE_YEAR:
                desc.Datatype = QString::fromLatin1("YEAR");
                break;
            case FIELD_TYPE_NEWDATE:
                desc.Datatype = QString::fromLatin1("NEWDATE");
                break;
            case FIELD_TYPE_ENUM:
                desc.Datatype = QString::fromLatin1("ENUM");
                break;
            case FIELD_TYPE_SET:
                desc.Datatype = QString::fromLatin1("SET");
                break;
            case FIELD_TYPE_TINY_BLOB:
                desc.Datatype = QString::fromLatin1("TINY_BLOB");
                break;
            case FIELD_TYPE_MEDIUM_BLOB:
                desc.Datatype = QString::fromLatin1("MEDIUM_BLOB");
                break;
            case FIELD_TYPE_LONG_BLOB:
                desc.Datatype = QString::fromLatin1("LONG_BLOB");
                break;
            case FIELD_TYPE_BLOB:
                desc.Datatype = QString::fromLatin1("BLOB");
                break;
            case FIELD_TYPE_VAR_STRING:
                desc.Datatype = QString::fromLatin1("VAR_STRING");
                break;
            case FIELD_TYPE_STRING:
                desc.Datatype = QString::fromLatin1("STRING");
                break;
            default:
                desc.Datatype = QString::fromLatin1("UNKNOWN");
                break;
        }

        if (info.length() > size)
        {
            desc.Datatype += QString::fromLatin1(" (");
            if (info.length() % size == 0)
                desc.Datatype += QString::number(info.length() / size);
            else
                desc.Datatype += QString::number(info.length());
            if (info.precision() > 0)
            {
                desc.Datatype += QString::fromLatin1(",");
                desc.Datatype += QString::number(info.precision());
            }
            desc.Datatype += QString::fromLatin1(")");
        }
        desc.Null = !info.requiredStatus();
        ColumnDescriptions.append(desc);
    }
    return ColumnDescriptions;
}

QString mysqlQuery::stripBinds(const QString &in)
{
    BindParams.clear();
    QString retval;
    std::auto_ptr <SQLLexer::Lexer> lexer = LexerFactTwoParmSing::Instance().create("mySQLGuiLexer", "", "toCustomLexer");
    lexer->setStatement(in);

    SQLLexer::Lexer::token_const_iterator start = lexer->begin();
    while (start->getTokenType() != SQLLexer::Token::X_EOF)
    {
        switch (start->getTokenType())
        {
            case SQLLexer::Token::L_BIND_VAR:
                retval += start->getText();
                BindParams << start->getText();
                break;
            case SQLLexer::Token::L_BIND_VAR_WITH_PARAMS:
                {
                    QString l1 = start->getText();
                    QString l2 = l1.left( l1.indexOf('<'));
                    QString l3 = l2.leftJustified( l1.length(), ' ');
                    BindParams << l2;
                    retval += l3;
                }
                break;
            default:
                retval += start->getText();
                //no break here
        }
        start++;
    }
    return retval;
}

void mysqlQuery::bindParam(QSqlQuery *q, toQueryParams const &params)
{
    Q_ASSERT_X(BindParams.size() <= params.size()
               , qPrintable(__QHERE__)
               , qPrintable(QString("Bind variables mismatch: %1 vs. %2").arg(BindParams.size()).arg(params.size())));
    for (int i = 0; i < BindParams.size(); i++)
    {
        q->bindValue(BindParams.at(i), params.at(i).displayData());
        TLOG(6, toDecorator, __HERE__) << "	Binding " << BindParams.at(i) << " <= " << params.at(i).displayData() << std::endl;
    }
}
