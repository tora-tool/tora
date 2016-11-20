
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

#include "connection/toqsqlquery.h"
#include "connection/toqsqlprovider.h"
#include "connection/toqsqlconnection.h"
#include "core/tosql.h"
#include "core/tocache.h"
#include "core/utils.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>

QSqlQuery* qsqlQuery::createQuery(const QString &query)
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);

    QSqlQuery *ret;
    ret = new QSqlQuery(*ptr);
    ret->setForwardOnly(true);
    ret->exec(query);
    return ret;
}

qsqlQuery::qsqlQuery(toQueryAbstr *query, toQSqlConnectionSub *conn)
    : queryImpl(query)
    , Connection(conn)
    , Column(0)
{
    EOQ = true;
    Query = NULL;
}

qsqlQuery::~qsqlQuery()
{
    delete Query;
}

void qsqlQuery::execute(void)
{
    Query = createQuery(query()->sql());
    checkQuery();
}

void qsqlQuery::execute(QString const& sql)
{
    QList<QString> empty;
    Query = createQuery(sql);
    checkQuery();
}

void qsqlQuery::cancel(void)
{
    if (!Connection->ConnectionID.isEmpty())
    {
        try
        {
            toConnection const &conn = query()->connection();
            const QString &sql = toSQL::sql("toQSqlConnection:Cancel", conn);
            if (!sql.isEmpty())
            {
                toConnectionSubLoan c(const_cast<toConnection&>(conn));
                toQuery(c, sql, toQueryParams() << Connection->ConnectionID);
            }
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
        }
    }
}

toQValue qsqlQuery::readValue(void)
{
    if (!Query)
        throw QString::fromLatin1("Fetching from unexecuted query");
    if (EOQ)
        throw QString::fromLatin1("Tried to read past end of query");

    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
    QVariant val;
    bool fixEmpty = false;
    {
        val = Query->value(Column);
        if (Query->isNull(Column))
            val.clear();
        else if ((val.type() == QVariant::Date || val.type() == QVariant::DateTime) && val.isNull())
            fixEmpty = true;
    }
    if (fixEmpty)
    {
        switch (val.type())
        {
            case QVariant::Date:
                val = QVariant(QString("0000-00-00"));
                break;
            case QVariant::DateTime:
                val = QVariant(QString("0000-00-00T00:00:00"));
                break;
            default:
                break;
                // Do nothing
        }
    }

    // sapdb marks value as invalid on some views
    // for example tables,indexes etc, so ignore this check
    Column++;
    if (Column == (unsigned int) Record.count())
    {
        Column = 0;
        EOQ = !Query->next();
    }

    return toQValue::fromVariant(val);
}

bool qsqlQuery::eof(void)
{
    return EOQ;
}

unsigned long qsqlQuery::rowsProcessed(void)
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

unsigned qsqlQuery::columns(void)
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
    return Record.count();
}

toQColumnDescriptionList qsqlQuery::describe(void)
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
    toQColumnDescriptionList ret;
    if (Query && Query->isSelect())
    {
        QString provider = query()->connection().provider();
        QSqlRecord rec = Query->record();
        ret = Describe(provider, rec);
    }
    return ret;
}

void qsqlQuery::checkQuery(void) // Must *not* call while locked
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);

    if (!Query->isActive())
    {
        QString msg = QString::fromLatin1("Query not active ");
        msg += query()->sql();
        throw toQSqlConnectionSub::ErrorString(Query->lastError(), msg);
    }

    if (Query->isSelect())
    {
        Record = Query->record();
        EOQ = !Query->next();
        Column = 0;
    }
    else
    {
        EOQ = true;
    }
}

toQColumnDescriptionList qsqlQuery::Describe(const QString &type, QSqlRecord record)
{
    toQColumnDescriptionList ret;
    unsigned int count = record.count();
    for (unsigned int i = 0; i < count; i++)
    {
        toCache::ColumnDescription desc;
        desc.AlignRight = false;
        int col = i;
        if (col == -1)
        {
            desc.Name = "Database";
            desc.Datatype = "STRING";
            desc.Null = false;
            desc.AlignRight = false;
            ret.insert(ret.end(), desc);
            continue;
        }
        desc.Name = record.fieldName(col);
        desc.AlignRight = false;

        int size = 1;

        QSqlField info = record.field(desc.Name);

        switch (info.type())
        {
            case QVariant::String:
                desc.Datatype = QString::fromLatin1("VARCHAR");
                break;
//		case FIELD_TYPE_DECIMAL:
//			desc.Datatype = QString::fromLatin1("DECIMAL");
//			break;
//		case FIELD_TYPE_TINY:
//			desc.Datatype = QString::fromLatin1("TINY");
//			break;
//		case FIELD_TYPE_SHORT:
//			desc.Datatype = QString::fromLatin1("SHORT");
//			break;
//		case FIELD_TYPE_LONG:
//			desc.Datatype = QString::fromLatin1("LONG");
//			break;
//		case FIELD_TYPE_FLOAT:
//			desc.Datatype = QString::fromLatin1("FLOAT");
//			break;
//		case FIELD_TYPE_DOUBLE:
//			desc.Datatype = QString::fromLatin1("DOUBLE");
//			break;
//		case FIELD_TYPE_NULL:
//			desc.Datatype = QString::fromLatin1("NULL");
//			break;
//		case FIELD_TYPE_TIMESTAMP:
//			desc.Datatype = QString::fromLatin1("TIMESTAMP");
//			break;
//		case FIELD_TYPE_LONGLONG:
//			desc.Datatype = QString::fromLatin1("LONGLONG");
//			break;
//		case FIELD_TYPE_INT24:
//			desc.Datatype = QString::fromLatin1("INT23");
//			break;
//		case FIELD_TYPE_DATE:
//			desc.Datatype = QString::fromLatin1("DATE");
//			break;
//		case FIELD_TYPE_TIME:
//			desc.Datatype = QString::fromLatin1("TIME");
//			break;
//		case FIELD_TYPE_DATETIME:
//			desc.Datatype = QString::fromLatin1("DATETIME");
//			break;
//		case FIELD_TYPE_YEAR:
//			desc.Datatype = QString::fromLatin1("YEAR");
//			break;
//		case FIELD_TYPE_NEWDATE:
//			desc.Datatype = QString::fromLatin1("NEWDATE");
//			break;
//		case FIELD_TYPE_ENUM:
//			desc.Datatype = QString::fromLatin1("ENUM");
//			break;
//		case FIELD_TYPE_SET:
//			desc.Datatype = QString::fromLatin1("SET");
//			break;
//		case FIELD_TYPE_TINY_BLOB:
//			desc.Datatype = QString::fromLatin1("TINY_BLOB");
//			break;
//		case FIELD_TYPE_MEDIUM_BLOB:
//			desc.Datatype = QString::fromLatin1("MEDIUM_BLOB");
//			break;
//		case FIELD_TYPE_LONG_BLOB:
//			desc.Datatype = QString::fromLatin1("LONG_BLOB");
//			break;
//		case FIELD_TYPE_BLOB:
//			desc.Datatype = QString::fromLatin1("BLOB");
//			break;
//		case FIELD_TYPE_VAR_STRING:
//			desc.Datatype = QString::fromLatin1("VAR_STRING");
//			break;
//		case FIELD_TYPE_STRING:
//			desc.Datatype = QString::fromLatin1("STRING");
//			break;
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

        ret.insert(ret.end(), desc);
    }
    return ret;
}
