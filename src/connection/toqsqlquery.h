
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

#ifndef __QSQL_QUERY__
#define __QSQL_QUERY__

#include "core/toquery.h"
#include "core/toqueryimpl.h"

#include <QtSql/QSqlRecord>
#include <QtCore/QList>

class QSqlQuery;
class toQSqlConnectionSub;

class qsqlQuery : public queryImpl
{
    public:
        qsqlQuery(toQueryAbstr *query, toQSqlConnectionSub *conn);

        virtual ~qsqlQuery();

        virtual void execute(void);

        virtual void execute(QString const&);

        virtual void cancel(void);

        virtual toQValue readValue(void);

        virtual bool eof(void);

        virtual unsigned long rowsProcessed(void);

        virtual unsigned columns(void);

        virtual toQColumnDescriptionList describe(void);
    private:
        static toQColumnDescriptionList Describe(const QString &type, QSqlRecord record, int *order, unsigned int orderSize);
        static QString QueryParam(const QString &in, toQueryParams const &params, QList<QString> &extradata);

        QSqlQuery *Query;
        QSqlRecord Record;
        toQSqlConnectionSub *Connection;
        QString CurrentExtra;
        QList<QString> ExtraData;
        bool EOQ;
        unsigned int Column;
        unsigned int ColumnOrderSize;
        int *ColumnOrder;

        void checkQuery(void);

        QSqlQuery *createQuery(const QString &query);
};

#endif
