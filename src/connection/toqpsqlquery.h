
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

#ifndef __QPSQL_QUERY__
#define __QPSQL_QUERY__

#include "core/toquery.h"
#include "core/toqueryimpl.h"

#include <QtSql/QSqlRecord>
#include <QtCore/QList>

class QSqlQuery;
class toQPSqlConnectionSub;

class psqlQuery : public queryImpl
{
        // PostgreSQL datatypes (From pg_type.h)
        enum DataTypeEnum
        {
            BOOLOID 	= 16,
            BYTEAOID 	= 17,
            CHAROID 	= 18,
            NAMEOID 	= 19,
            INT8OID 	= 20,
            INT2OID 	= 21,
            INT2VECTOROID = 22,
            INT4OID 	= 23,
            REGPROCOID 	= 24,
            TEXTOID 	= 25,
            OIDOID 	= 26,
            TIDOID 	= 27,
            XIDOID 	= 28,
            CIDOID 	= 29,
            OIDVECTOROID 	= 30,
            POINTOID 	= 600,
            LSEGOID 	= 601,
            PATHOID 	= 602,
            BOXOID 	= 603,
            POLYGONOID 	= 604,
            LINEOID 	= 628,
            FLOAT4OID 	= 700,
            FLOAT8OID 	= 701,
            ABSTIMEOID 	= 702,
            RELTIMEOID 	= 703,
            TINTERVALOID = 704,
            UNKNOWNOID = 705,
            CIRCLEOID = 718,
            CASHOID = 790,
            MACADDROID = 829,
            INETOID = 869,
            CIDROID = 650,
            BPCHAROID 	= 1042,
            VARCHAROID 	= 1043,
            DATEOID 	= 1082,
            TIMEOID 	= 1083,
            TIMESTAMPOID 	= 1114,
            TIMESTAMPTZOID = 1184,
            INTERVALOID 	= 1186,
            TIMETZOID 	= 1266,
            BITOID 	= 1560,
            VARBITOID 	= 1562,
            NUMERICOID 	= 1700,
            REFCURSOROID 	= 1790
        };
    public:
        psqlQuery(toQueryAbstr *query, toQPSqlConnectionSub *conn);
        virtual ~psqlQuery();
        virtual void execute(void);
        virtual void execute(QString const&);
        virtual void cancel(void);
        virtual toQValue readValue(void);
        virtual bool eof(void);
        virtual unsigned long rowsProcessed(void);
        virtual unsigned columns(void);
        virtual toQColumnDescriptionList describe(void);
    private:
        toQColumnDescriptionList describe(QSqlRecord record);
        QString stripBinds(const QString &in);
        void bindParam(QSqlQuery *q, toQueryParams const &params);
        static QString QueryParam(const QString &in, toQueryParams const &params, QList<QString> &extradata);
        QSqlQuery *Query;
        QSqlRecord Record;
        QStringList BindParams;
        toQPSqlConnectionSub *Connection;
        toQColumnDescriptionList ColumnDescriptions;
        unsigned CurrentColumn;
        bool EOQ;
        void checkQuery(void);
        QSqlQuery *createQuery(const QString &sql);
};

#endif
