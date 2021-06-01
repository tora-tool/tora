
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

#include "connection/toqpsqlquery.h"
#include "connection/toqpsqlprovider.h"
#include "connection/toqpsqlconnection.h"
#include "core/tosql.h"
#include "core/tocache.h"
#include "core/utils.h"
#include "parsing/tsqllexer.h"

#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
#include <libpq-fe.h>
#include <QtSql/QSqlDriver>
#endif

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>


// Only attempt to cancel a query using a secondary connection if we
// didn't build with the postgres api.
// Opening a second connection has drawbacks and can fail to
// successfully cancel queries if there is any problem getting a
// second connection, causing crashes.
static toSQL SQLCancelPg("toQSqlConnection:Cancel",
                         "SELECT pg_cancel_backend(:pid)",
                         "",
                         "0800",
                         "QPSQL");

QSqlQuery* psqlQuery::createQuery(const QString &sql)
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);

    QSqlQuery *ret = new QSqlQuery(*ptr);
    ret->setForwardOnly(true);
    bool prepared, executed;
    Q_UNUSED(prepared);
    Q_UNUSED(executed);
    if (!query()->params().empty())
    {
        QString s = stripBinds(query()->sql());
        prepared = ret->prepare(s);
        bindParam(ret, query()->params());
        executed = ret->exec();
    }
    else
    {
        executed = ret->exec(sql);
    }
    return ret;
}

psqlQuery::psqlQuery(toQueryAbstr *query, toQPSqlConnectionSub *conn)
    : queryImpl(query)
    , Query(NULL)
    , Connection(conn)
    , CurrentColumn(0)
    , EOQ(true)

{
}

psqlQuery::~psqlQuery()
{
    delete Query;
}

void psqlQuery::execute(void)
{
    Query = createQuery(query()->sql());
    checkQuery();
}

void psqlQuery::execute(QString const& sql)
{
    Query = createQuery(query()->sql());
    checkQuery();
}

void psqlQuery::cancel(void)
{
    if (!Connection->ConnectionID.isEmpty())
    {
        try
        {
            toConnection const &conn = query()->connection();
            const QString &sql = toSQL::sql(SQLCancelPg, conn);
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

toQValue psqlQuery::readValue(void)
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

bool psqlQuery::eof(void)
{
    return EOQ;
}

unsigned long psqlQuery::rowsProcessed(void)
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

unsigned psqlQuery::columns(void)
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
    return Record.count();
}

toQColumnDescriptionList psqlQuery::describe(void)
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
    toQColumnDescriptionList ret;
    if (Query && Query->isSelect())
    {
        ret = describe(Query->record());
    }
    return ret;
}

void psqlQuery::checkQuery(void) // Must *not* call while locked
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
    if (!Query->isActive())
    {
        QString msg = QString::fromLatin1("Query not active ");
        msg += Query->lastQuery();
        throw toConnection::exception(toQPSqlConnectionSub::ErrorString(Query->lastError(), msg));
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

toQColumnDescriptionList psqlQuery::describe(QSqlRecord record)
{
    ColumnDescriptions.clear();
    for (unsigned int i = 0; i < record.count(); i++)
    {
        toCache::ColumnDescription desc;
        desc.Name = record.fieldName(i);
        desc.AlignRight = false;
        int size = 1;

        QSqlField info = record.field(desc.Name);
        desc.Datatype = QString::fromLatin1("TBD(DATATYPE)");
        switch (info.typeID())
        {
            case BOOLOID:
                desc.Datatype = QString::fromLatin1("BOOLOID");
                break;
            case BYTEAOID:
                desc.Datatype = QString::fromLatin1("BYTEAOID");
                break;
            case CHAROID:
                desc.Datatype = QString::fromLatin1("CHAROID");
                break;
            case NAMEOID:
                desc.Datatype = QString::fromLatin1("NAMEOID");
                break;
            case INT8OID:
                desc.Datatype = QString::fromLatin1("INT8OID");
                break;
            case INT2OID:
                desc.Datatype = QString::fromLatin1("INT2OID");
                break;
            case INT2VECTOROID:
                desc.Datatype = QString::fromLatin1("INT2VECTOROID");
                break;
            case INT4OID:
                desc.Datatype = QString::fromLatin1("INT4OID");
                break;
            case REGPROCOID:
                desc.Datatype = QString::fromLatin1("REGPROCOID");
                break;
            case TEXTOID:
                desc.Datatype = QString::fromLatin1("TEXTOID");
                break;
            case OIDOID:
                desc.Datatype = QString::fromLatin1("OIDOID");
                break;
            case TIDOID:
                desc.Datatype = QString::fromLatin1("TIDOID");
                break;
            case XIDOID:
                desc.Datatype = QString::fromLatin1("XIDOID");
                break;
            case CIDOID:
                desc.Datatype = QString::fromLatin1("CIDOID");
                break;
            case OIDVECTOROID:
                desc.Datatype = QString::fromLatin1("OIDVECTOROID");
                break;
            case POINTOID:
                desc.Datatype = QString::fromLatin1("POINTOID");
                break;
            case LSEGOID:
                desc.Datatype = QString::fromLatin1("LSEGOID");
                break;
            case PATHOID:
                desc.Datatype = QString::fromLatin1("PATHOID");
                break;
            case BOXOID:
                desc.Datatype = QString::fromLatin1("BOXOID");
                break;
            case POLYGONOID:
                desc.Datatype = QString::fromLatin1("POLYGONOID");
                break;
            case LINEOID:
                desc.Datatype = QString::fromLatin1("LINEOID");
                break;
            case FLOAT4OID:
                desc.Datatype = QString::fromLatin1("FLOAT4OID");
                break;
            case FLOAT8OID:
                desc.Datatype = QString::fromLatin1("FLOAT8OID");
                break;
            case ABSTIMEOID:
                desc.Datatype = QString::fromLatin1("ABSTIMEOID");
                break;
            case RELTIMEOID:
                desc.Datatype = QString::fromLatin1("RELTIMEOID");
                break;
            case TINTERVALOID:
                desc.Datatype = QString::fromLatin1("TINTERVALOID");
                break;
            case UNKNOWNOID:
                desc.Datatype = QString::fromLatin1("UNKNOWNOID");
                break;
            case CIRCLEOID:
                desc.Datatype = QString::fromLatin1("CIRCLEOID");
                break;
            case CASHOID:
                desc.Datatype = QString::fromLatin1("CASHOID");
                break;
            case MACADDROID:
                desc.Datatype = QString::fromLatin1("MACADDROID");
                break;
            case INETOID:
                desc.Datatype = QString::fromLatin1("INETOID");
                break;
            case CIDROID:
                desc.Datatype = QString::fromLatin1("CIDROID");
                break;
            case BPCHAROID:
                desc.Datatype = QString::fromLatin1("BPCHAROID");
                break;
            case VARCHAROID:
                desc.Datatype = QString::fromLatin1("VARCHAROID");
                break;
            case DATEOID:
                desc.Datatype = QString::fromLatin1("DATEOID");
                break;
            case TIMEOID:
                desc.Datatype = QString::fromLatin1("TIMEOID");
                break;
            case TIMESTAMPOID:
                desc.Datatype = QString::fromLatin1("TIMESTAMPOID");
                break;
            case TIMESTAMPTZOID:
                desc.Datatype = QString::fromLatin1("TIMESTAMPTZOID");
                break;
            case INTERVALOID:
                desc.Datatype = QString::fromLatin1("INTERVALOID");
                break;
            case TIMETZOID:
                desc.Datatype = QString::fromLatin1("TIMETZOID");
                break;
            case BITOID:
                desc.Datatype = QString::fromLatin1("BITOID");
                break;
            case VARBITOID:
                desc.Datatype = QString::fromLatin1("VARBITOID");
                break;
            case NUMERICOID:
                desc.Datatype = QString::fromLatin1("NUMERICOID");
                break;
            case REFCURSOROID:
                desc.Datatype = QString::fromLatin1("REFCURSOROID");
                break;
            default:
                desc.Datatype = QString::fromLatin1("UNKNOWN(%1)").arg(info.typeID());
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

QString psqlQuery::stripBinds(const QString &in)
{
    BindParams.clear();
    QString retval;
    // TODO: no PostgreSQL Lexer ATM
    std::unique_ptr <SQLLexer::Lexer> lexer = LexerFactTwoParmSing::Instance().create("MySQLGuiLexer", "", "toCustomLexer");
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

void psqlQuery::bindParam(QSqlQuery *q, toQueryParams const &params)
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

