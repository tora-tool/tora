
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconnection.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tomysqlkeywords.h"
#include "tosql.h"
#include "totool.h"
#include "tothread.h"

#include <ctype.h>
#include <string.h>

#include <qapplication.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <qsqlfield.h>
#include <qsqlrecord.h>
#include <qsqlresult.h>

#include <QVBoxLayout>
#include <QGroupBox>
#include <QtSql>
#include <QGridLayout>
#include <QString>

#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
#ifdef CMAKE_BUILD
  // There is different location of pgsql includes sometimes
  // E.g. /usr/include/pgsql on suse etc. Cmake hadles it
  // easily for us.
  #include <libpq-fe.h>
#else
  // This is for autotools
  #include <postgresql/libpq-fe.h>
#endif
#endif

// #include <QDebug>

static toSQL SQLVersion("toQSqlConnection:Version",
                        "SHOW VARIABLES LIKE 'version'",
                        "Show version of database, "
                        "last value of first return record of result is used.",
                        "3.0",
                        "MySQL");

static toSQL SQLVersionSapDb("toQSqlConnection:Version",
                             "select ltrim(substring(kernel,10,10)) from versions",
                             "",
                             "",
                             "SapDB");

static toSQL SQLVersionPgSQL("toQSqlConnection:Version",
                             "SELECT SUBSTR(version(), STRPOS(version(), ' ') + 1, STRPOS(version(), 'on') - STRPOS(version(), ' ') - 2)",
                             "",
                             "7.1",
                             "PostgreSQL");

static toSQL SQLListDatabases("toQSqlConnection:ListDatabases",
                              "show databases",
                              "List the available databases for a mysql connection",
                              "3.0",
                              "MySQL");

static toSQL SQLListObjectsDatabase("toQSqlConnection:ListObjectsDatabase",
                                    "show table status from :f1<noquote>",
                                    "Get the available tables for a specific database (MySQL specific, won't work for anything else)",
                                    "3.0",
                                    "MySQL");

static toSQL SQLListObjects("toQSqlConnection:ListObjects",
                            "show tables",
                            "Get the available tables for a connection",
                            "3.0",
                            "MySQL");

static toSQL SQLListObjectsSapDb("toQSqlConnection:ListObjects",
                                 "select tablename \"Tablename\",\n"
                                 "       owner     \"Owner\",\n"
                                 "       tabletype \"Type\",\n"
                                 "from tables \n"
                                 "where tabletype not in (\'SYNONYM\',\'RESULT\') \n"
                                 "order by tablename",
                                 "",
                                 "",
                                 "SapDB");

static toSQL SQLListObjectsPgSQL("toQSqlConnection:ListObjects",
                                 "SELECT c.relname AS \"Tablename\",\n"
                                 "       n.nspname AS \"Owner\",\n"
                                 "       c.relkind AS \"Type\"\n"
                                 "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                                 " ORDER BY \"Tablename\"",
                                 "",
                                 "7.1",
                                 "PostgreSQL");

static toSQL SQLListSynonymsSapDb("toQSqlConnection:ListSynonyms",
                                  "SELECT synonymname \"Synonym\", owner \"Schema\", tablename \"Object\"\n"
                                  "from synonyms \n"
                                  "where tabletype not in (\'SYNONYM\',\'RESULT\') \n"
                                  "order by owner,tablename",
                                  "Get synonym list, should have same columns",
                                  "",
                                  "SapDb");

static toSQL SQLListSynonyms("toQSqlConnection:ListSynonyms",
                             "SELECT c.relname AS \"Synonym\", n.nspname AS \"Schema\", c.relname AS \"Object\"\n"
                             "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                             " ORDER BY n.nspname, c.relname",
                             "",
                             "7.1",
                             "PostgreSQL");

static toSQL SQLColumnComments("toQSqlConnection:ColumnComments",
                               "select a.attname,b.description\n"
                               "from\n"
                               "  pg_attribute a,\n"
                               "  pg_description b,\n"
                               "  pg_class c LEFT OUTER JOIN pg_namespace n ON c.relowner=n.oid\n"
                               "where\n"
                               "  a.oid=b.objoid\n"
                               "  and c.oid=a.attrelid\n"
                               "  and (u.nspname = :owner OR u.usesysid IS NULL)\n"
                               "  and c.relname=:table",
                               "Get the available comments on columns of a table, "
                               "must have same binds and columns",
                               "7.1",
                               "PostgreSQL");

static toSQL SQLColumnComments72("toQSqlConnection:ColumnComments",
                                 "select a.attname,b.description\n"
                                 "from\n"
                                 "  pg_attribute a,\n"
                                 "  pg_description b,\n"
                                 "  pg_class c LEFT OUTER JOIN pg_namespace n ON c.relowner=n.oid\n"
                                 "where\n"
                                 "  a.attnum=b.objsubid\n"
                                 "  and b.objoid=a.attrelid\n"
                                 "  and c.oid=a.attrelid\n"
                                 "  and (n.nspname = :owner OR u.usesysid IS NULL)\n"
                                 "  and c.relname=:table",
                                 "",
                                 "7.2",
                                 "PostgreSQL");

static toSQL SQLConnectionID("toQSqlConnection:ConnectionID",
                             "SELECT connection_id()",
                             "Get a connection ID for a session",
                             "3.23",
                             "MySQL");

static toSQL SQLConnectionIDPg("toQSqlConnection:ConnectionID",
                               "SELECT pg_backend_pid()",
                               "",
                               "",
                               "PostgreSQL");

static toSQL SQLCancel("toQSqlConnection:Cancel",
                       "KILL :f1",
                       "Cancel a connection given it's connection ID",
                       "3.23",
                       "MySQL");

static toSQL SQLCancelM5("toQSqlConnection:Cancel",
                       "KILL QUERY :f1",
                       "",
                       "5.0",
                       "MySQL");

static toSQL SQLCancelPg("toQSqlConnection:Cancel",
                         "SELECT pg_cancel_backend(:pid)",
                         "",
                         "8.0",
                         "PostgreSQL");

// seems to be the only way for < 8.0 to not get pg_cancel_backend
static toSQL SQLCancelPgOld("toQSqlConnection:Cancel",
                            "native",
                            "",
                            "",
                            "PostgreSQL");


void native_cancel(QSqlDriver *driver) {
    QVariant v = driver->handle();
    if(v.isValid() && v.typeName() == QString("PGconn*")) {
#ifdef LIBPQ_DECL_CANCEL
        PGconn *handle = *static_cast<PGconn **>(v.data());
        if(!handle)
            return;

        PGcancel *cancel = PQgetCancel(handle);
        if(!cancel)
            return;

        char *errbuf = new char[1024];
        PQcancel(cancel, errbuf, 1024);
        PQfreeCancel(cancel);
        delete[] errbuf;
#endif
    }
}


struct toQSqlProviderAggregate
{
    enum aggregateType
    {
        None,
        AllDatabases,
        AllTables,
        CurrentDatabase,
        SpecifiedDatabase
    } Type;
    QString Data;
    toQSqlProviderAggregate()
            : Type(None)
    { }
    toQSqlProviderAggregate(aggregateType type, const QString &data = QString::null)
            : Type(type), Data(data)
    { }
}
;

static QString QueryParam(const QString &in, toQList &params, std::list<QString> *extradata)
{
    QString ret;
    bool inString = false;
    toQList::iterator cpar = params.begin();
    QString query = QString(in);

    std::map<QString, QString> binds;

    for (int i = 0;i < query.length();i++)
    {
        QChar rc = query.at(i);
        char  c  = rc.toLatin1();

        char nc = 0;
        if (i + 1 < query.length())
            nc = query.at(i + 1).toLatin1();

        switch (c)
        {
        case '\\':
            ret += rc;
            ret += query.at(++i);
            break;
        case '\'':
            inString = !inString;
            ret += rc;
            break;
        case ':':
            // mostly for postgres-style casts, ignore ::
            if (nc == ':')
            {
                ret += rc;
                ret += nc;
                i++;
                break;
            }

            if (!inString)
            {
                QString nam;
                for (i++;i < query.length();i++)
                {
                    rc = query.at(i);
                    if (!rc.isLetterOrNumber())
                        break;
                    nam += rc;
                }
                c = rc.toLatin1();
                QString in;
                if (c == '<')
                {
                    for (i++;i < query.length();i++)
                    {
                        rc = query.at(i);
                        c = rc.toLatin1();
                        if (c == '>')
                        {
                            i++;
                            break;
                        }
                        in += rc;
                    }
                }
                i--;

                toQSqlProviderAggregate aggr;
                ;
                if (in == "alldatabases")
                    aggr = toQSqlProviderAggregate(toQSqlProviderAggregate::AllDatabases);
                else if (in == "alltables")
                    aggr = toQSqlProviderAggregate(toQSqlProviderAggregate::AllTables);
                else if (in == "currenttables")
                    aggr = toQSqlProviderAggregate(toQSqlProviderAggregate::CurrentDatabase);
                else if (in == "database")
                {
                    aggr = toQSqlProviderAggregate(toQSqlProviderAggregate::SpecifiedDatabase);
                }

                QString str;
                QString tmp;
                if (aggr.Type == toQSqlProviderAggregate::None ||
                        aggr.Type == toQSqlProviderAggregate::SpecifiedDatabase)
                {
                    if (nam.isEmpty())
                        break;

                    if (binds.find(nam) != binds.end())
                    {
                        ret += binds[nam];
                        break;
                    }
                    if (cpar == params.end())
                        throw toConnection::exception(QString::fromLatin1("Not all bind variables supplied"), i);
                    if ((*cpar).isNull())
                    {
                        str = QString::fromLatin1("NULL");
                    }
                    else if ((*cpar).isInt() || (*cpar).isDouble())
                    {
                        str = QString(*cpar);
                    }
                    tmp = (*cpar);
                    cpar++;
                }
                if (str.isNull())
                {
                    if (aggr.Type != toQSqlProviderAggregate::None)
                    {
                        if (extradata)
                        {
                            if ( extradata->empty() )
                                return QString::null;
                            tmp = toShift(*extradata);
                        }
                        else
                        {
                            aggr.Data = tmp;
                            throw aggr;
                        }
                    }
                    else
                    {
                        if (in != QString::fromLatin1("noquote"))
                            str += QString::fromLatin1("'");
                    }
                    for (int j = 0;j < tmp.length();j++)
                    {
                        QChar d = tmp.at(j);
                        switch (d.toLatin1())
                        {
                        case 0:
                            str += QString::fromLatin1("\\0");
                            break;
                        case '\n':
                            str += QString::fromLatin1("\\n");
                            break;
                        case '\t':
                            str += QString::fromLatin1("\\t");
                            break;
                        case '\r':
                            str += QString::fromLatin1("\\r");
                            break;
                        case '\'':
                            str += QString::fromLatin1("\\\'");
                            break;
                        case '\"':
                            str += QString::fromLatin1("\\\"");
                            break;
                        case '\\':
                            str += QString::fromLatin1("\\\\");
                            break;
                        default:
                            str += d;
                        }
                    }
                    if (in != QString::fromLatin1("noquote") && aggr.Type == toQSqlProviderAggregate::None)
                        str += QString::fromLatin1("'");
                }
                binds[nam] = str;
                ret += str;
                break;
            }
        default:
            ret += rc;
        }
    }
    return ret;
}

static QString ErrorString(const QSqlError &err, const QString &sql = QString::null)
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

// PostgreSQL datatypes (From pg_type.h)

#define BOOLOID                 16
#define BYTEAOID                17
#define CHAROID                 18
#define NAMEOID                 19
#define INT8OID                 20
#define INT2OID                 21
#define INT2VECTOROID    22
#define INT4OID                 23
#define REGPROCOID              24
#define TEXTOID                 25
#define OIDOID                  26
#define TIDOID           27
#define XIDOID    28
#define CIDOID    29
#define OIDVECTOROID     30
#define POINTOID                600
#define LSEGOID                 601
#define PATHOID                 602
#define BOXOID                  603
#define POLYGONOID              604
#define LINEOID                 628
#define FLOAT4OID   700
#define FLOAT8OID   701
#define ABSTIMEOID              702
#define RELTIMEOID              703
#define TINTERVALOID     704
#define UNKNOWNOID              705
#define CIRCLEOID               718
#define CASHOID   790
#define MACADDROID   829
#define INETOID   869
#define CIDROID   650
#define BPCHAROID               1042
#define VARCHAROID              1043
#define DATEOID                 1082
#define TIMEOID                 1083
#define TIMESTAMPOID     1114
#define TIMESTAMPTZOID   1184
#define INTERVALOID             1186
#define TIMETZOID               1266
#define BITOID     1560
#define VARBITOID        1562
#define NUMERICOID              1700
#define REFCURSOROID     1790

// MySQL datatypes (From mysql_com.h)

enum enum_field_types { FIELD_TYPE_DECIMAL, FIELD_TYPE_TINY,
                        FIELD_TYPE_SHORT, FIELD_TYPE_LONG,
                        FIELD_TYPE_FLOAT, FIELD_TYPE_DOUBLE,
                        FIELD_TYPE_NULL, FIELD_TYPE_TIMESTAMP,
                        FIELD_TYPE_LONGLONG, FIELD_TYPE_INT24,
                        FIELD_TYPE_DATE, FIELD_TYPE_TIME,
                        FIELD_TYPE_DATETIME, FIELD_TYPE_YEAR,
                        FIELD_TYPE_NEWDATE,
                        FIELD_TYPE_ENUM = 247,
                        FIELD_TYPE_SET = 248,
                        FIELD_TYPE_TINY_BLOB = 249,
                        FIELD_TYPE_MEDIUM_BLOB = 250,
                        FIELD_TYPE_LONG_BLOB = 251,
                        FIELD_TYPE_BLOB = 252,
                        FIELD_TYPE_VAR_STRING = 253,
                        FIELD_TYPE_STRING = 254
                      };

// SAPDb Datatypes (from /opt/sapdb/interfaces/odbc/incl/*.h)
#define SAP_SQL_UNKNOWN_TYPE    0
#define SAP_SQL_DECIMAL         3
#define SAP_SQL_NUMERIC         2
#define SAP_SQL_REAL            7
#define SAP_SQL_FLOAT           6
#define SAP_SQL_DOUBLE          8

#define SAP_SQL_INTEGER         4
#define SAP_SQL_SMALLINT        5
#define SAP_SQL_BIT            (-7)
#define SAP_SQL_TINYINT        (-6)
#define SAP_SQL_BIGINT         (-5)

#define SAP_SQL_CHAR            1
#define SAP_SQL_VARCHAR        12
#define SAP_SQL_LONGVARCHAR    (-1)

#define SAP_SQL_BINARY         (-2)
#define SAP_SQL_VARBINARY      (-3)
#define SAP_SQL_LONGVARBINARY  (-4)

#define SAP_SQL_DATE       9
#define SAP_SQL_DATETIME       9
#define SAP_SQL_TYPE_DATE      91

#define SAP_SQL_TIME      10
#define SAP_SQL_TYPE_TIME       92

#define SAP_SQL_TIMESTAMP 11
#define SAP_SQL_TYPE_TIMESTAMP  93



static std::list<toQuery::queryDescribe> Describe(const QString &type, QSqlRecord record, int *order, unsigned int orderSize)
{
    std::list<toQuery::queryDescribe> ret;
    unsigned int count = record.count();
    if (order)
    {
        count = orderSize;
    }
    for (unsigned int i = 0;i < count;i++)
    {
        toQuery::queryDescribe desc;
        desc.AlignRight = false;
        int col = i;
        if (order)
            col = order[i] - 1;
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
        if (type == "MySQL")
        {
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
        }
        else if (type == "PostgreSQL")
        {
            switch (info.typeID())
            {
            case BOOLOID:
                desc.Datatype = QString::fromLatin1("BOOL");
                break;
            case BYTEAOID:
                desc.Datatype = QString::fromLatin1("BYTEA");
                break;
            case CHAROID:
                desc.Datatype = QString::fromLatin1("CHAR");
                break;
            case NAMEOID:
                size = 32;
                desc.Datatype = QString::fromLatin1("NAME");
                break;
            case INT8OID:
                size = 8;
                desc.Datatype = QString::fromLatin1("INT8");
                break;
            case INT2OID:
                size = 2;
                desc.Datatype = QString::fromLatin1("INT2");
                break;
            case INT2VECTOROID:
                size = 2;
                desc.Datatype = QString::fromLatin1("INT2VECTOR");
                break;
            case INT4OID:
                size = 4;
                desc.Datatype = QString::fromLatin1("INT4");
                break;
            case REGPROCOID:
                size = 4;
                desc.Datatype = QString::fromLatin1("REGPROC");
                break;
            case TEXTOID:
                desc.Datatype = QString::fromLatin1("TEXT");
                break;
            case OIDOID:
                size = 4;
                desc.Datatype = QString::fromLatin1("OID");
                break;
            case TIDOID:
                size = 6;
                desc.Datatype = QString::fromLatin1("TID");
                break;
            case XIDOID:
                size = 4;
                desc.Datatype = QString::fromLatin1("XID");
                break;
            case CIDOID:
                size = 4;
                desc.Datatype = QString::fromLatin1("CID");
                break;
            case OIDVECTOROID:
                size = 4;
                desc.Datatype = QString::fromLatin1("OIDVECTOR");
                break;
            case POINTOID:
                size = 16;
                desc.Datatype = QString::fromLatin1("POINT");
                break;
            case LSEGOID:
                size = 32;
                desc.Datatype = QString::fromLatin1("LSEG");
                break;
            case PATHOID:
                desc.Datatype = QString::fromLatin1("PATH");
                break;
            case BOXOID:
                size = 32;
                desc.Datatype = QString::fromLatin1("BOX");
                break;
            case POLYGONOID:
                desc.Datatype = QString::fromLatin1("POLYGON");
                break;
            case LINEOID:
                size = 32;
                desc.Datatype = QString::fromLatin1("LINE");
                break;
            case FLOAT4OID:
                size = 4;
                desc.Datatype = QString::fromLatin1("FLOAT4");
                break;
            case FLOAT8OID:
                size = 8;
                desc.Datatype = QString::fromLatin1("FLOAT8");
                break;
            case ABSTIMEOID:
                size = 4;
                desc.Datatype = QString::fromLatin1("ABSTIME");
                break;
            case RELTIMEOID:
                size = 4;
                desc.Datatype = QString::fromLatin1("RELTIME");
                break;
            case TINTERVALOID:
                size = 12;
                desc.Datatype = QString::fromLatin1("TINTERVAL");
                break;
            case UNKNOWNOID:
                desc.Datatype = QString::fromLatin1("UNKNOWN");
                break;
            case CIRCLEOID:
                size = 24;
                desc.Datatype = QString::fromLatin1("CIRCLE");
                break;
            case CASHOID:
                size = 4;
                desc.Datatype = QString::fromLatin1("MONEY");
                break;
            case MACADDROID:
                size = 6;
                desc.Datatype = QString::fromLatin1("MACADDR");
                break;
            case INETOID:
                desc.Datatype = QString::fromLatin1("INET");
                break;
            case CIDROID:
                desc.Datatype = QString::fromLatin1("CIDR");
                break;
            case BPCHAROID:
                desc.Datatype = QString::fromLatin1("BPCHAR");
                break;
            case VARCHAROID:
                desc.Datatype = QString::fromLatin1("VARCHAR");
                break;
            case DATEOID:
                size = 4;
                desc.Datatype = QString::fromLatin1("DATE");
                break;
            case TIMEOID:
                size = 8;
                desc.Datatype = QString::fromLatin1("TIME");
                break;
            case TIMESTAMPOID:
                size = 8;
                desc.Datatype = QString::fromLatin1("TIMESTAMP");
                break;
            case TIMESTAMPTZOID:
                size = 8;
                desc.Datatype = QString::fromLatin1("TIMESTAMPTZ");
                break;
            case INTERVALOID:
                size = 12;
                desc.Datatype = QString::fromLatin1("INTERVAL");
                break;
            case TIMETZOID:
                size = 12;
                desc.Datatype = QString::fromLatin1("TIMETZ");
                break;
            case BITOID:
                desc.Datatype = QString::fromLatin1("BIT");
                break;
            case VARBITOID:
                desc.Datatype = QString::fromLatin1("VARBIT");
                break;
            case NUMERICOID:
                desc.Datatype = QString::fromLatin1("NUMERIC");
                break;
            case REFCURSOROID:
                desc.Datatype = QString::fromLatin1("REFCURSOR");
                break;
            default:
                desc.Datatype = QString::fromLatin1("UNKNOWN");
                break;
            }
        }
        else if (type == "SapDB")
        {
            switch (info.typeID())
            {
            case SAP_SQL_UNKNOWN_TYPE:
                desc.Datatype = QString::fromLatin1("UNKNOWN");
                break;
            case SAP_SQL_BIT:
                desc.Datatype = QString::fromLatin1("BIT");
                break;
            case SAP_SQL_INTEGER:
                desc.Datatype = QString::fromLatin1("INTEGER");
                break;
            case SAP_SQL_SMALLINT:
                desc.Datatype = QString::fromLatin1("SMALLINT");
                break;
            case SAP_SQL_BIGINT:
                desc.Datatype = QString::fromLatin1("BIGINT");
                break;
            case SAP_SQL_TINYINT:
                desc.Datatype = QString::fromLatin1("TINYINT");
                break;
            case SAP_SQL_NUMERIC:
                desc.Datatype = QString::fromLatin1("NUMERIC");
                break;
            case SAP_SQL_DECIMAL:
                desc.Datatype = QString::fromLatin1("DECIMAL");
                break;
            case SAP_SQL_FLOAT:
                desc.Datatype = QString::fromLatin1("FLOAT");
                break;
            case SAP_SQL_REAL:
                desc.Datatype = QString::fromLatin1("REAL");
                break;
            case SAP_SQL_DOUBLE:
                desc.Datatype = QString::fromLatin1("DOUBLE");
                break;
            case SAP_SQL_DATE:
                desc.Datatype = QString::fromLatin1("DATE");
                break;
            case SAP_SQL_TYPE_DATE:
                desc.Datatype = QString::fromLatin1("DATE");
                break;
            case SAP_SQL_TIME:
                desc.Datatype = QString::fromLatin1("TIME");
                break;
            case SAP_SQL_TYPE_TIME:
                desc.Datatype = QString::fromLatin1("TIME");
                break;
            case SAP_SQL_TIMESTAMP:
                desc.Datatype = QString::fromLatin1("TIMESTAMP");
                break;
            case SAP_SQL_TYPE_TIMESTAMP:
                desc.Datatype = QString::fromLatin1("TIMESTAMP");
                break;
            case SAP_SQL_CHAR:
                desc.Datatype = QString::fromLatin1("CHAR");
                break;
            case SAP_SQL_VARCHAR:
                desc.Datatype = QString::fromLatin1("VARCHAR");
                break;
            case SAP_SQL_LONGVARCHAR:
                desc.Datatype = QString::fromLatin1("LONGVARCHAR");
                break;
            case SAP_SQL_BINARY:
                desc.Datatype = QString::fromLatin1("BINARY");
                break;
            case SAP_SQL_VARBINARY:
                desc.Datatype = QString::fromLatin1("BINARY");
                break;
            case SAP_SQL_LONGVARBINARY:
                desc.Datatype = QString::fromLatin1("LONGVARBINARY");
                break;
            default:
                desc.Datatype = QString::fromLatin1("UNKNOWN");
                break;
            }
        }
        else
        {
            switch (info.type())
            {
            default:
                desc.Datatype = QString::fromLatin1("UNKNOWN");
                break;
            case QVariant::Invalid:
                desc.Datatype = QString::fromLatin1("INVALID");
                break;
            case QVariant::List:
                desc.Datatype = QString::fromLatin1("LIST");
                break;
            case QVariant::Map:
                desc.Datatype = QString::fromLatin1("MAP");
                break;
            case QVariant::String:
                desc.Datatype = QString::fromLatin1("VARCHAR");
                break;
            case QVariant::StringList:
                desc.Datatype = QString::fromLatin1("STRINGLIST");
                break;
            case QVariant::Font:
                desc.Datatype = QString::fromLatin1("FONT");
                break;
            case QVariant::Pixmap:
                desc.Datatype = QString::fromLatin1("PIXMAP");
                break;
            case QVariant::Brush:
                desc.Datatype = QString::fromLatin1("BRUSH");
                break;
            case QVariant::Rect:
                desc.Datatype = QString::fromLatin1("RECT");
                break;
            case QVariant::Size:
                desc.Datatype = QString::fromLatin1("SIZE");
                break;
            case QVariant::Color:
                desc.Datatype = QString::fromLatin1("COLOR");
                break;
            case QVariant::Palette:
                desc.Datatype = QString::fromLatin1("PALETTE");
                break;
            case QVariant::Icon:
                desc.Datatype = QString::fromLatin1("ICON");
                break;
            case QVariant::Point:
                desc.Datatype = QString::fromLatin1("POINT");
                break;
            case QVariant::Image:
                desc.Datatype = QString::fromLatin1("IMAGE");
                break;
            case QVariant::Int:
                desc.Datatype = QString::fromLatin1("INT");
                desc.AlignRight = true;
                break;
            case QVariant::UInt:
                desc.Datatype = QString::fromLatin1("UINT");
                desc.AlignRight = true;
                break;
            case QVariant::Bool:
                desc.Datatype = QString::fromLatin1("BOOL");
                break;
            case QVariant::Double:
                desc.Datatype = QString::fromLatin1("DOUBLE");
                desc.AlignRight = true;
                break;
            case QVariant::Region:
                desc.Datatype = QString::fromLatin1("REGION");
                break;
            case QVariant::Bitmap:
                desc.Datatype = QString::fromLatin1("BITMAP");
                break;
            case QVariant::Cursor:
                desc.Datatype = QString::fromLatin1("CURSOR");
                break;
            case QVariant::Date:
                desc.Datatype = QString::fromLatin1("DATE");
                break;
            case QVariant::Time:
                desc.Datatype = QString::fromLatin1("TIME");
                break;
            case QVariant::DateTime:
                desc.Datatype = QString::fromLatin1("DATETIME");
                break;
            case QVariant::ByteArray:
                // qt4 ByteArray == CString
                desc.Datatype = QString::fromLatin1("BLOB");
                break;
            case QVariant::BitArray:
                desc.Datatype = QString::fromLatin1("BITARRAY");
                break;
            case QVariant::SizePolicy:
                desc.Datatype = QString::fromLatin1("SIZEPOLICY");
                break;
            case QVariant::KeySequence:
                desc.Datatype = QString::fromLatin1("KEYSEQUENCE");
                break;
            }
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

class toQSqlProvider : public toConnectionProvider
{
    QStringList Drivers;
public:
    static bool OnlyForward;

    class qSqlSetting : public QWidget, public toSettingTab
    {
        QCheckBox *OnlyForward;
        QLabel    *CreateLabel;
        QComboBox *BeforeCreateAction;
    public:
        qSqlSetting(QWidget *parent)
                : QWidget(parent), toSettingTab("database.html#qsql")
        {

            QVBoxLayout *vbox = new QVBoxLayout;
            this->setLayout(vbox);
            vbox->setSpacing(0);
            vbox->setContentsMargins(0, 0, 0, 0);

            QGroupBox *box = new QGroupBox(this);
            box->setSizePolicy(QSizePolicy::Expanding,
                               QSizePolicy::Expanding);
            vbox->addWidget(box);

            vbox = new QVBoxLayout;
            vbox->setSpacing(6);
            vbox->setContentsMargins(11, 11, 11, 11);

            OnlyForward = new QCheckBox(
                qApp->translate(
                    "qSqlSetting",
                    "Posibility to break MySQL queries (Can require more connections)"),
                box);
            OnlyForward->setChecked(toConfigurationSingle::Instance().onlyForward());
            vbox->addWidget(OnlyForward);

            CreateLabel = new QLabel(
                qApp->translate("qSqlSetting", "When calling create routine statement in worksheet"),
                box);
            vbox->addWidget(CreateLabel);

            BeforeCreateAction = new QComboBox(box);
            BeforeCreateAction->clear();
            BeforeCreateAction->insertItems(0, QStringList()
              << qApp->translate("qSqlSetting", "Do nothing")
              << qApp->translate("qSqlSetting", "Drop before creating")
              << qApp->translate("qSqlSetting", "Drop before creating (if exists)")
              << qApp->translate("qSqlSetting", "Ask")
              << qApp->translate("qSqlSetting", "Ask (if exists)"));
            BeforeCreateAction->setCurrentIndex(toConfigurationSingle::Instance().createAction());
            vbox->addWidget(BeforeCreateAction);

            QSpacerItem *spacer = new QSpacerItem(
                20,
                20,
                QSizePolicy::Minimum,
                QSizePolicy::Expanding);
            vbox->addItem(spacer);

            box->setLayout(vbox);
        }
        virtual void saveSetting(void)
        {
            toConfigurationSingle::Instance().setOnlyForward(OnlyForward->isChecked());
            toQSqlProvider::OnlyForward = OnlyForward->isChecked();
            toConfigurationSingle::Instance().setCreateAction(BeforeCreateAction->currentIndex());
        }
    };

    static QString fromQSqlName(const QString &driv)
    {
        if (driv == QString::fromLatin1("QMYSQL"))
            return "MySQL";
        else if (driv == QString::fromLatin1("QPSQL"))
            return "PostgreSQL";
        else if (driv == QString::fromLatin1("QTDS"))
            return "Microsoft SQL/TDS";
        else if (driv == QString::fromLatin1("QSAPDB"))
            return "SapDB";
        else if (driv == QString::fromLatin1("QODBC"))
            return "ODBC";
        return "";
    }
    static QString toQSqlName(const QString &driv)
    {
        if (driv == "MySQL")
            return QString::fromLatin1("QMYSQL");
        else if (driv == "PostgreSQL")
            return QString::fromLatin1("QPSQL");
        else if (driv == "Microsoft SQL/TDS")
            return QString::fromLatin1("QTDS");
        else if (driv == "SapDB")
            return QString::fromLatin1("QSAPDB");
        else if (driv == "ODBC")
            return QString::fromLatin1("QODBC");
        return QString::null;
    }

    class mySQLAnalyzer : public toSyntaxAnalyzer
    {
    public:
        mySQLAnalyzer()
                : toSyntaxAnalyzer(MySQLKeywords)
        { }
        virtual QChar quoteCharacter()
        {
            return '`';
        }
        virtual bool declareBlock()
        {
            return false;
        }
    };

    class qSqlSub : public toConnectionSub
    {
    public:
        // use compilter to prevent accidental unprotected access. Use
        // LockingPtr.
        volatile QSqlDatabase Connection;
        QMutex Lock;
        QString Name;
        QString ConnectionID;

        qSqlSub(QSqlDatabase conn, const QString &name)
                : Connection(conn), Name(name) {
        }

        void lockUp()
        {
        }

        void lockDown ()
        {
        }

        int getLockValue()
        {
            return 1;
        }

        ~qSqlSub()
        {
            LockingPtr<QSqlDatabase> ptr(Connection, Lock);
            ptr->close();
        }

        virtual void cancel() {
            if (!ConnectionID.isEmpty())
            {
                // try a native cancel if possible. because
                // qsqldatabase can't cancel queries itself and this
                // is called during pool shutdown, we can't rely on
                // calling another connection to cancel this one.
                QSqlDatabase *c = const_cast<QSqlDatabase *>(&(Connection));
                native_cancel(c->driver());
            }
        }

        // doh. better release the lock before calling this
        void throwError(const QString &sql)
        {
            LockingPtr<QSqlDatabase> ptr(Connection, Lock);
            throw ErrorString(ptr->lastError(), sql);
        }
    };

    class qSqlQuery : public toQuery::queryImpl
    {
        QSqlQuery *Query;
        QSqlRecord Record;
        qSqlSub *Connection;
        QString CurrentExtra;
        std::list<QString> ExtraData;
        bool EOQ;
        unsigned int Column;
        unsigned int ColumnOrderSize;
        int *ColumnOrder;

        void checkQuery(void);

        std::list<QString> extraData(const toQSqlProviderAggregate &aggr)
        {
            std::list<QString> ret;
            std::list<toConnection::objectName> &objects = query()->connection().objects(false);
            for (std::list<toConnection::objectName>::iterator i = objects.begin();i != objects.end();i++)
            {
                if ((*i).Type == "DATABASE" && aggr.Type == toQSqlProviderAggregate::AllDatabases)
                {
                    toPush(ret, (*i).Owner);
                }
                else if ((*i).Type == "TABLE")
                {
                    if (aggr.Type == toQSqlProviderAggregate::AllTables ||
                            (aggr.Type == toQSqlProviderAggregate::CurrentDatabase && (*i).Owner == query()->connection().user()) ||
                            (aggr.Type == toQSqlProviderAggregate::SpecifiedDatabase && (*i).Owner == aggr.Data))
                        toPush(ret, (*i).Owner + "." + (*i).Name);
                }
            }
            return ret;
        }
        QSqlQuery *createQuery(const QString &query)
        {
            LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);

            QSqlQuery *ret;
            ret = new QSqlQuery(*ptr);
            if (toQSqlProvider::OnlyForward)
                ret->setForwardOnly(true);
            ret->exec(query);
            return ret;
        }
    public:
        qSqlQuery(toQuery *query, qSqlSub *conn)
                : toQuery::queryImpl(query), Connection(conn)
        {
            Column = 0;
            ColumnOrder = NULL;
            EOQ = true;
            Query = NULL;
        }
        virtual ~qSqlQuery()
        {
            delete Query;
            delete[] ColumnOrder;
        }
        virtual void execute(void);

        virtual void cancel(void)
        {
            if (!Connection->ConnectionID.isEmpty())
            {
                try
                {
                    toConnection &conn = query()->connection();
                    const QString &sql = toSQL::sql(SQLCancel, conn);
                    if(!sql.isEmpty() && sql != "native") {
                        toQList pars;
                        pars.insert(pars.end(), Connection->ConnectionID);
                        conn.execute(sql, pars);
                    }
                    else {
                        // don't lock here or deadlock while waiting
                        // for query to finish
                        QSqlDatabase *c = const_cast<QSqlDatabase *>(&(Connection->Connection));
                        native_cancel(c->driver());
                    }
                }
                catch (...)
                {}
            }
        }

        QString parseReorder(const QString &str)
        {
            if (str.toUpper().startsWith("TOAD"))
            {
                std::list<int> order;
                int num = -1;
                int i;
                for (i = 4;i < str.length();i++)
                {
                    char c = str.at(i).toAscii();
                    if (isspace(c))
                        ;
                    else if (isdigit(c))
                    {
                        if (num < 0)
                            num = 0;
                        num *= 10;
                        num += c - '0';
                    }
                    else if (c == '*')
                    {
                        if (num >= 0)
                            throw QString("Invalid column selection, number before *");
                        if (c == '*')
                        {
                            order.insert(order.end(), -1);
                            do
                            {
                                i++;
                            }
                            while (str.at(i).isSpace());
                            break;
                        }
                    }
                    else
                    {
                        if (num < 0)
                            throw QString("Invalid column selection, number missing");
                        order.insert(order.end(), num);
                        num = -1;
                        if (c != ',')
                            break;
                    }
                }
                ColumnOrderSize = order.size();
                if (ColumnOrderSize == 0)
                    throw QString("Missing column selection");
                delete[] ColumnOrder;
                ColumnOrder = new int[ColumnOrderSize];
                int pos = 0;
                for (std::list<int>::iterator j = order.begin();j != order.end();j++, pos++)
                    ColumnOrder[pos] = *j;

                return str.mid(i);
            }
            else
                return str;
        }

        virtual toQValue readValue(void)
        {
            if (!Query)
                throw QString::fromLatin1("Fetching from unexecuted query");
            if (EOQ)
                throw QString::fromLatin1("Tried to read past end of query");

            LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
            QVariant val;
            bool fixEmpty = false;
            if (ColumnOrder)
            {
                int col = ColumnOrder[Column];
                if (col >= 1)
                {
                    val = Query->value(col - 1);
                    if (Query->isNull(col - 1))
                        val.clear();
                    else if ((val.type() == QVariant::Date || val.type() == QVariant::DateTime) && val.isNull())
                        fixEmpty = true;
                }
                else if (col == 0)
                {
                    val = CurrentExtra;
                }
            }
            else
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
            if ((ColumnOrder && Column == ColumnOrderSize) || (!ColumnOrder && Column == (unsigned int) Record.count()))
            {
                Column = 0;
                EOQ = !Query->next();
            }
            if (EOQ && ExtraData.begin() != ExtraData.end())
            {
                delete Query;
                Query = NULL;
                CurrentExtra = *ExtraData.begin();

                ptr.unlock();
                Query = createQuery(QueryParam(parseReorder(query()->sql()), query()->params(), &ExtraData));
                checkQuery();
                ptr.lock();
            }

            return toQValue::fromVariant(val);
        }
        virtual bool eof(void)
        {
            return EOQ;
        }
        virtual int rowsProcessed(void)
        {
            try {
                LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock, true);

                if (!Query)
                    return 0;
                return Query->numRowsAffected();
            }
            catch(...) {
                return 0;
            }
        }
        virtual int columns(void)
        {
            LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
            int ret = Record.count();
            if (ColumnOrder)
                ret = ColumnOrderSize;

            return ret;
        }
        virtual std::list<toQuery::queryDescribe> describe(void)
        {
            LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);
            std::list<toQuery::queryDescribe> ret;
            if (Query && Query->isSelect())
            {
                QString provider = query()->connection().provider();
                QSqlRecord rec = Query->record();
                ret = Describe(provider, rec, ColumnOrder, ColumnOrderSize);
            }
            return ret;
        }
    };

class qSqlConnection : public toConnection::connectionImpl
    {
        bool Multiple;
        bool HasTransactions;
        qSqlSub *qSqlConv(toConnectionSub *sub)
        {
            qSqlSub *conn = dynamic_cast<qSqlSub *>(sub);
            if (!conn)
                throw QString::fromLatin1("Internal error, not QSql sub connection");
            return conn;
        }
        toSyntaxAnalyzer *MySQLAnalyzer;
    public:
        qSqlConnection(toConnection *conn)
                : toConnection::connectionImpl(conn)
        {
            if (conn->provider() == "SapDB" ||
                    (conn->provider() == "MySQL" && toQSqlProvider::OnlyForward))
                Multiple = false;
            else
                Multiple = true;
            if (conn->provider() == "MySQL")
                HasTransactions = false;
            else
                HasTransactions = true;
            MySQLAnalyzer = NULL;
        }

        virtual toSyntaxAnalyzer &analyzer()
        {
            if (connection().provider() == "MySQL")
            {
                if (!MySQLAnalyzer)
                    MySQLAnalyzer = new mySQLAnalyzer();
                return *MySQLAnalyzer;
            }
            else
                return toSyntaxAnalyzer::defaultAnalyzer();
        }

        virtual QString quote(const QString &name)
        {
            if (connection().provider() == "PostgreSQL")
            {
                bool ok = true;
                for (int i = 0;i < name.length();i++)
                {
                    if (name.at(i).toLower() != name.at(i) || !toIsIdent(name.at(i)))
                        ok = false;
                }
                if (!ok)
                    return QString::fromLatin1("\"") + name + QString::fromLatin1("\"");
            }
            return name;
        }

        virtual QString unQuote(const QString &name)
        {
            if (connection().provider() == "PostgreSQL")
            {
                if (name.at(0).toLatin1() == '\"' && name.at(name.length() - 1).toLatin1() == '\"')
                    return name.left(name.length() - 1).right(name.length() - 2);
            }
            else if (connection().provider() == "MySQL")
            {
                if (name.at(0).toLatin1() == '`' && name.at(name.length() - 1).toLatin1() == '`')
                    return name.left(name.length() - 1).right(name.length() - 2);
            }
            return name;
        }

        virtual std::list<toConnection::objectName> objectNames(void)
        {
            std::list<toConnection::objectName> ret;

            toConnection::objectName cur;
            try
            {
                toQuery databases(connection(), SQLListDatabases);
                while (!databases.eof())
                {
                    QString db = databases.readValueNull();
                    cur.Owner = db;
                    cur.Type = "DATABASE";
                    cur.Name = QString::null;
                    ret.insert(ret.end(), cur);
                    try
                    {
                        toQuery tables(connection(), SQLListObjectsDatabase, db);
                        while (!tables.eof())
                        {
                            cur.Name = tables.readValueNull();
                            cur.Owner = db;
                            cur.Type = "TABLE";
                            for (int i = 2;i < tables.columns();i++) // I just wan't the last column except for the first one
                                tables.readValueNull();
                            cur.Comment = tables.readValueNull();
                            ret.insert(ret.end(), cur);
                        }
                    }
                    catch (...)
                        {}
                }
            }
            catch (...)
            {
                toQuery tables(connection(), SQLListObjects);
                while (!tables.eof())
                {
                    cur.Name = tables.readValueNull();
                    if (tables.columns() > 1)
                        cur.Owner = tables.readValueNull();
                    else
                        cur.Owner = connection().database();
                    if (tables.columns() > 2)
                        cur.Type = tables.readValueNull();
                    else
                        cur.Type = QString::fromLatin1("TABLE");
                    ret.insert(ret.end(), cur);
                }
            }

            return ret;
        }

        virtual std::map<QString, toConnection::objectName> synonymMap(std::list<toConnection::objectName> &objects)
        {
            std::map<QString, toConnection::objectName> ret;

            try
            {
                toConnection::objectName cur;
                cur.Type = QString::fromLatin1("A");

                toQuery synonyms(connection(), SQLListSynonyms);
                std::list<toConnection::objectName>::iterator i = objects.begin();
                while (!synonyms.eof())
                {
                    QString synonym = synonyms.readValueNull();
                    cur.Owner = synonyms.readValueNull();
                    cur.Name = synonyms.readValueNull();
                    while (i != objects.end() && (*i) < cur)
                        i++;
                    if (i == objects.end())
                        break;
                    if (cur.Name == (*i).Name && cur.Owner == (*i).Owner)
                    {
                        ret[synonym] = (*i);
                        (*i).Synonyms.insert((*i).Synonyms.end(), synonym);
                    }
                }
            }
            catch (...)
                {}

            return ret;
        }

        virtual toQDescList columnDesc(const toConnection::objectName &table)
        {
            toBusy busy;

            std::map<QString, QString> comments;
            try
            {
                toQuery comment(connection(), SQLColumnComments, table.Owner, table.Name);
                while (!comment.eof())
                {
                    QString col = comment.readValue();
                    comments[col] = comment.readValueNull();
                }
            }
            catch (...)
                {}

            try
            {
                toQDescList desc;
                if (connection().provider() == "PostgreSQL")
                {
                    toQuery query(connection(), toQuery::Normal);
                    qSqlSub *sub = dynamic_cast<qSqlSub *>(query.connectionSub());
                    if (sub)
                    {
                        LockingPtr<QSqlDatabase> ptr(sub->Connection, sub->Lock);
                        desc = Describe(connection().provider(), ptr->record(quote(table.Name)), NULL, 0);
                    }
                }
                else
                {
                    QString SQL = QString::fromLatin1("SELECT * FROM ");
                    SQL += quote(table.Owner);
                    SQL += QString::fromLatin1(".");
                    SQL += quote(table.Name);
                    SQL += QString::fromLatin1(" WHERE NULL=NULL");
                    toQuery query(connection(), SQL);
                    desc = query.describe();
                }
                for (toQDescList::iterator j = desc.begin();j != desc.end();j++)
                    (*j).Comment = comments[(*j).Name];

                return desc;
            }
            catch (...)
                {}

            toQDescList ret;
            return ret;
        }

        virtual bool handleMultipleQueries()
        {
            return Multiple;
        }

        virtual void commit(toConnectionSub *sub)
        {
            qSqlSub *conn = qSqlConv(sub);
            LockingPtr<QSqlDatabase> ptr(conn->Connection, conn->Lock);
            if(!ptr->commit() && HasTransactions) {
                ptr.unlock();
                conn->throwError(QString::fromLatin1("COMMIT"));
            }
        }
        virtual void rollback(toConnectionSub *sub)
        {
            qSqlSub *conn = qSqlConv(sub);
            LockingPtr<QSqlDatabase> ptr(conn->Connection, conn->Lock);
            if(!ptr->rollback() && HasTransactions) {
                ptr.unlock();
                conn->throwError(QString::fromLatin1("ROLLBACK"));
            }
        }

        virtual toConnectionSub *createConnection(void);

        void closeConnection(toConnectionSub *conn)
        {
            delete conn;
        }

        virtual QString version(toConnectionSub *sub)
        {
            QString ret;
            qSqlSub *conn = qSqlConv(sub);
            try
            {
                LockingPtr<QSqlDatabase> ptr(conn->Connection, conn->Lock);

                QSqlQuery query = ptr->exec(toSQL::string(SQLVersion,
                                                          connection()));
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
                {}
            return ret;
        }

        virtual toQuery::queryImpl *createQuery(toQuery *query, toConnectionSub *sub)
        {
            return new qSqlQuery(query, qSqlConv(sub));
        }
        virtual void execute(toConnectionSub *sub, const QString &sql, toQList &params)
        {
            qSqlSub *conn = qSqlConv(sub);
            try
            {
                LockingPtr<QSqlDatabase> ptr(conn->Connection, conn->Lock);
                QSqlQuery Query(ptr->exec(QueryParam(sql, params, NULL)));
                if (!Query.isActive())
                {
                    QString msg = QString::fromLatin1("Query not active ");
                    msg += sql;
                    ptr.unlock();
                    throw ErrorString(Query.lastError(), msg);
                }
            }
            catch (const toQSqlProviderAggregate &)
            {
                // Ok, this one is complicated and will probably never be used.
                throw QString("Direct exec aggregate queries are not supported, use a toQuery object for this one");
            }
        }
    };

    toQSqlProvider(void)
            : toConnectionProvider("QSql", false)
    {
        OnlyForward = toConfigurationSingle::Instance().onlyForward();
    }

    virtual void initialize(void)
    {
        Drivers = QSqlDatabase::drivers();
        for (int i = 0;i < Drivers.count();i++)
        {
            QString t = fromQSqlName(Drivers[i]);
            if (!t.isEmpty())
                addProvider(t);
        }
    }


    ~toQSqlProvider()
    {
#ifdef Q_WS_MAC
#warning "toQSqlProvider is disabled for Mac OS X due crashes. Maybe it is not a correct solution..."
#else
        for (int i = 0;i < Drivers.count();i++)
        {
            QString t = fromQSqlName(Drivers[i]);
            if (!t.isEmpty())
                removeProvider(t);
        }
#endif
    }

    virtual QWidget *providerConfigurationTab(const QString &provider,
            QWidget *parent)
    {
        if (provider == "MySQL")
            return new qSqlSetting(parent);
        return NULL;
    }

    virtual toConnection::connectionImpl *provideConnection(const QString &, toConnection *conn)
    {
        return new qSqlConnection(conn);
    }
    virtual std::list<QString> providedHosts(const QString &provider)
    {
        std::list<QString> ret;
        ret.insert(ret.end(), "localhost");
        if (provider == "MySQL")
            ret.insert(ret.end(), ":3306");
        else if (provider == "PostgreSQL")
            ret.insert(ret.end(), ":5432");
        return ret;
    }

    virtual std::list<QString> providedOptions(const QString &provider)
    {
        std::list<QString> ret;
        if (provider == "MySQL")
        {
            ret.insert(ret.end(), "*SSL");
            ret.insert(ret.end(), "*Compress");
            ret.insert(ret.end(), "-");
            ret.insert(ret.end(), "Ignore Space");
            ret.insert(ret.end(), "No Schema");
        }
        return ret;
    }
    virtual std::list<QString> providedDatabases(const QString &, const QString &, const QString &, const QString &)
    {
        std::list<QString> ret;

        // Need implementation

        return ret;
    }

    static qSqlSub* createConnection(toConnection &conn);
};

static toQSqlProvider QSqlProvider;
toLock myLock;

void toQSqlProvider::qSqlQuery::execute(void)
{
    Query = NULL;
    try
    {
        Query = createQuery(QueryParam(parseReorder(query()->sql()), query()->params(), NULL));
    }
    catch (const toQSqlProviderAggregate &aggr)
    {
        ExtraData = extraData(aggr);
        if (ExtraData.begin() != ExtraData.end())
            CurrentExtra = *ExtraData.begin();

        QString t = QueryParam(parseReorder(query()->sql()), query()->params(), &ExtraData);
        if (t.isEmpty())
        {
            toStatusMessage("Nothing to send to aggregate query");
            Query = NULL;
            EOQ = true;
            return;
        }
        else
            Query = createQuery(t);
    }

    checkQuery();
}

void toQSqlProvider::qSqlQuery::checkQuery(void) // Must *not* call while locked
{
    LockingPtr<QSqlDatabase> ptr(Connection->Connection, Connection->Lock);

    do
    {
        if (!Query->isActive())
        {
            QString msg = QString::fromLatin1("Query not active ");
            msg += query()->sql();
            throw ErrorString(Query->lastError(), msg);
        }

        if (Query->isSelect())
        {
            Record = Query->record();
            if (ColumnOrder && ColumnOrder[ColumnOrderSize - 1] == -1)
            {
                unsigned int newsize = ColumnOrderSize + Record.count() - 1;
                int *newalloc = new int[newsize];
                unsigned int i;
                for (i = 0;i < ColumnOrderSize - 1;i++)
                    newalloc[i] = ColumnOrder[i];
                for (int colnum = 1;i < newsize;i++, colnum++)
                    newalloc[i] = colnum;
                delete[] ColumnOrder;
                ColumnOrder = newalloc;
                ColumnOrderSize = newsize;
            }
            EOQ = !Query->next();
            Column = 0;
        }
        else
        {
            EOQ = true;
        }
        if (EOQ && ExtraData.begin() != ExtraData.end())
        {
            delete Query;
            Query = NULL;

            ptr.unlock();
            Query = createQuery(QueryParam(parseReorder(query()->sql()),
                                           query()->params(),
                                           &ExtraData));
            ptr.lock();
        }
    }
    while (ExtraData.begin() != ExtraData.end() && EOQ);
}

toQSqlProvider::qSqlSub *toQSqlProvider::createConnection(toConnection &conn)
{
    static int ID = 0;
    ID++;

    QString dbName = QString::number(ID);
    QSqlDatabase db = QSqlDatabase::addDatabase(toQSqlName(conn.provider()), dbName);
    db.setDatabaseName(conn.database());
    QString host = conn.host();
    int pos = host.indexOf(QString(":"));
    if (pos < 0)
        db.setHostName(host);
    else
    {
        db.setHostName(host.mid(0, pos));
        db.setPort(host.mid(pos + 1).toInt());
    }

    QString opt;

    std::set
    <QString> options = conn.options();
    if (options.find("Compress") != options.end())
        opt += ";CLIENT_COMPRESS";
    if (options.find("Ignore Space") != options.end())
        opt += ";CLIENT_IGNORE_SPACE";
    if (options.find("No Schema") != options.end())
        opt += ";CLIENT_NO_SCHEMA";
    if (options.find("SSL") != options.end())
        opt += ";CLIENT_SSL";

    if (!opt.isEmpty())
        db.setConnectOptions(opt.mid(1)); // Strip first ; character

    db.open(conn.user(), conn.password());
    if (!db.isOpen())
    {
        QString t = ErrorString(db.lastError());
        QSqlDatabase::removeDatabase(dbName);
        throw t;
    }

    toQSqlProvider::qSqlSub *ret = new toQSqlProvider::qSqlSub(db, dbName);

    // Try to figure out the connection ID for canceling
    try
    {
        QString sql = SQLConnectionID(conn);

        QSqlQuery query = db.exec(sql);
        if (query.next())
            ret->ConnectionID = query.value(0).toString();
    }
    catch (...)
    {}
    return ret;
}

toConnectionSub *toQSqlProvider::qSqlConnection::createConnection(void)
{
    return toQSqlProvider::createConnection(connection());
}

bool toQSqlProvider::OnlyForward;
