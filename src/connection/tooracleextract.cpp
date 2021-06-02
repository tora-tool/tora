
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

#include "connection/tooracleextract.h"
#include "core/toconnectiontraits.h"
#include "core/toconfiguration.h"
#include "core/toquery.h"
#include "core/tosql.h"
#include "core/utils.h"
#include "connection/tooracleconfiguration.h"

#include <QtCore/QRegularExpression>
#include <QRegExp>

// Implementation misc

const QString toOracleExtract::PROMPT_SIMPLE("PROMPT %1\n\n");

void toOracleExtract::clearFlags()
{
    ext.setState("IsASnapIndex", QVariant(false));
    ext.setState("IsASnapTable", QVariant(false));
}

static toSQL SQLDisplaySource("toOracleExtract:ListSource",
                              "SELECT text\n"
                              "  FROM sys.all_source\n"
                              " WHERE type = :typ<char[100]>\n"
                              "   AND name = :nam<char[100]>\n"
                              "   AND owner = :own<char[100]>\n"
                              " ORDER BY line",
                              "Get source of an object from the database, "
                              "must have same columns and binds");

QString toOracleExtract::displaySource(
                                       const QString &owner,
                                       const QString &name,
                                       const QString &type,
                                       bool describe)
{
    if (!ext.getCode())
        return "";

    QString re("^\\s*");
    re += type;
    re += "\\s+";
    QRegExp StripType(re, Qt::CaseInsensitive);

    toConnectionSubLoan conn(connection());
    toQuery inf(conn, SQLDisplaySource, toQueryParams() << type << name << owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Couldn't find source for of %1.%2").arg(owner).arg(name);

    QString ret;
    if (PROMPT && !describe)
        ret = QString("PROMPT CREATE OR REPLACE %1 %2.%3\n\n")
              .arg(type)
              .arg(quote(owner))
              .arg(quote(name));
    bool first = true;
    while (!inf.eof())
    {
        QString line = (QString)inf.readValue();
        if (first)
        {
            int len;
            int pos = StripType.indexIn(line, 0);
            len = StripType.matchedLength();
            if (pos != 0)
                throw qApp->translate("toOracleExtract", "Displaying source of wrong type for %1. Got %2 expected 0.")
                .arg(type).arg(pos);
            QString tmp = QString("CREATE OR REPLACE %1 ").arg(type);
            if (!describe)
                tmp += owner;
            tmp += line.mid(len);
            line = tmp.simplified() + "\n"; // avoid growing amount of spaces

            first = false;
        }
        ret += line;
    }
    if (!describe)
        ret += "\n/\n\n";
    return ret;
}

static toSQL SQLKeyColumns("toOracleExtract:KeyColumns",
                           "SELECT  column_name\n"
                           "  FROM sys.all_%1_key_columns\n"
                           " WHERE name           = :nam<char[100]>\n"
                           "   AND owner          = :owner<char[100]>\n"
                           "   AND object_type LIKE :typ<char[100]>||'%'",
                           "Get key columns of partitions, "
                           "must have same binds, columns and %");

QString toOracleExtract::keyColumns(
                                    const QString &owner,
                                    const QString &name,
                                    const QString &type,
                                    const QString &table)
{
    toConnectionSubLoan conn(connection());
    toQuery inf(conn, toSQL::string(SQLKeyColumns, connection()).arg(table), toQueryParams() << name << owner << type);
    QString ret;
    bool first = true;
    while (!inf.eof())
    {
        if (first)
            first = false;
        else
            ret += "\n  , ";
        ret += quote((QString)inf.readValue());
    }
    return ret;
}

QString toOracleExtract::partitionKeyColumns(
        const QString &owner,
        const QString &name,
        const QString &type)
{
    return keyColumns(owner, name, type, "PART");
}

QString toOracleExtract::prepareDB(const QString &db)
{
    static QRegularExpression quote("'");
    QString ret = db;
    ret.replace(quote, "''");
    return ret;
}

QString toOracleExtract::segments()
{
    return ext.state("Segments").toString();
}

QString toOracleExtract::segments(const toSQL &sql)
{
    return toSQL::string(sql, connection()).arg(segments());
}

QString toOracleExtract::subPartitionKeyColumns(
        const QString &owner,
        const QString &name,
        const QString &type)
{
    return keyColumns(owner, name, type, "SUBPART");
}

// Implementation create utility functions

static toSQL SQLConstraintCols("toOracleExtract:ConstraintCols",
                               "SELECT column_name\n"
                               "  FROM sys.all_cons_columns\n"
                               " WHERE owner = :own<char[100]>\n"
                               "   AND constraint_name = :con<char[100]>\n"
                               " ORDER BY position",
                               "List columns in a constraint, must have same binds and columns");

QString toOracleExtract::constraintColumns(const QString &owner, const QString &name)
{
    toConnectionSubLoan conn(connection());
    toQuery query(conn, SQLConstraintCols, toQueryParams() << owner << name);

    QString ret = "(\n    ";
    bool first = true;
    while (!query.eof())
    {
        if (first)
            first = false;
        else
            ret += ",\n    ";
        ret += quote((QString)query.readValue());
    }
    ret += "\n)\n";
    return ret;
}

static toSQL SQLTableComments("toOracleExtract:TableComment",
                              "SELECT comments\n"
                              "  FROM sys.all_tab_comments\n"
                              " WHERE table_name = :nam<char[100]>\n"
                              "   AND comments IS NOT NULL\n"
                              "   AND owner = :own<char[100]>",
                              "Extract comments about a table, must have same columns and binds");

static toSQL SQLColumnComments("toOracleExtract:ColumnComment",
                               "SELECT column_name,\n"
                               "       comments\n"
                               "  FROM sys.all_col_comments\n"
                               " WHERE table_name = :nam<char[100]>\n"
                               "   AND comments IS NOT NULL\n"
                               "   AND owner = :own<char[100]>",
                               "Extract comments about a columns, must have same columns and binds");

QString toOracleExtract::createComments(
                                        const QString &owner,
                                        const QString &name)
{
    QString ret;
    toConnectionSubLoan conn(connection());
    if (ext.getComments())
    {
        QString sql;
        toQuery inf(conn, SQLTableComments, toQueryParams() << name << owner);
        while (!inf.eof())
        {
            sql = QString("COMMENT ON TABLE %1.%2 IS '%3'").
                  arg(quote(owner)).
                  arg(quote(name)).
                  arg(prepareDB((QString)inf.readValue()));
            if (PROMPT)
            {
                QStringList lines = sql.split(QRegularExpression("\n|\r\n|\r"));
                foreach(QString line, lines)
                {
                    ret += QString("PROMPT %1\n").arg(line);
                }
                ret += "\n";
            }
            ret += sql;
            ret += ";\n\n";
        }
        toQuery col(conn, SQLColumnComments, toQueryParams() << name << owner);
        while (!col.eof())
        {
            QString column = (QString)col.readValue();
            sql = QString("COMMENT ON COLUMN %1.%2.%3 IS '%4'").
                  arg(quote(owner)).
                  arg(quote(name)).
                  arg(quote(column)).
                  arg(prepareDB((QString)col.readValue()));
            if (PROMPT)
            {
                QStringList lines = sql.split(QRegularExpression("\n|\r\n|\r"));
                foreach(QString line, lines)
                {
                    ret += QString("PROMPT %1\n").arg(line);
                }
                ret += "\n";
            }
            ret += sql;
            ret += ";\n\n";
        }
    }
    return ret;
}

static toSQL SQLContextInfoDBA("toOracleExtract:ContextInfoDBA",
                               "SELECT\n"
                               "        ixv_class\n"
                               "      , ixv_object\n"
                               "      , ixv_attribute\n"
                               "      , ixv_value\n"
                               " FROM\n"
                               "        ctxsys.ctx_index_values\n"
                               " WHERE\n"
                               "        ixv_index_owner = :own<char[100]>\n"
                               "    AND\n"
                               "        ixv_index_name = :nam<char[100]>\n"
                               "     AND\n"
                               "        ixv_class = :cls<char[100]>\n"
                               " ORDER BY\n"
                               "        ixv_object",
                               "Get information on context index preferences.",
                               "0801");

static toSQL SQLContextInfoNoAttrDBA("toOracleExtract:ContextInfoNoAttrDBA",
                                     "SELECT\n"
                                     "        ixo_object\n"
                                     " FROM\n"
                                     "        ctxsys.ctx_index_objects\n"
                                     " WHERE\n"
                                     "        ixo_index_owner = :own<char[100]>\n"
                                     "    AND\n"
                                     "        ixo_index_name = :nam<char[100]>\n"
                                     "    AND\n"
                                     "        ixo_class = :cls<char[100]>",
                                     "Get the context preferences w/o attributes.",
                                     "0801");

static toSQL SQLContextColumnDBA("toOracleExtract:ContextColumnDBA",
                                 "SELECT\n"
                                 "        idx_language_column\n"
                                 "      , idx_format_column\n"
                                 "      , idx_charset_column\n"
                                 " FROM\n"
                                 "        ctxsys.ctx_indexes\n"
                                 " WHERE\n"
                                 "        idx_owner = :own<char[100]>\n"
                                 "    AND\n"
                                 "        idx_name = :nam<char[100]>",
                                 "Get the context column designations.",
                                 "0801");

static toSQL SQLContextInfo("toOracleExtract:ContextInfo",
                            "SELECT\n"
                            "        ixv_class\n"
                            "      , ixv_object\n"
                            "      , ixv_attribute\n"
                            "      , ixv_value\n"
                            " FROM\n"
                            "        ctxsys.ctx_user_index_values\n"
                            " WHERE\n"
                            "        ixv_index_name = :nam<char[100]>\n"
                            "     AND\n"
                            "        ixv_class = :cls<char[100]>\n"
                            " ORDER BY\n"
                            "        ixv_object",
                            "Get information on context index preferences.",
                            "0801");

static toSQL SQLContextInfoNoAttr("toOracleExtract:ContextInfoNoAttr",
                                  "SELECT\n"
                                  "        ixo_object\n"
                                  " FROM\n"
                                  "        ctxsys.ctx_user_index_objects\n"
                                  " WHERE\n"
                                  "        ixo_index_name = :nam<char[100]>\n"
                                  "    AND\n"
                                  "        ixo_class = :cls<char[100]>",
                                  "Get the context preferences w/o attributes.",
                                  "0801");

static toSQL SQLContextColumn("toOracleExtract:ContextColumn",
                              "SELECT\n"
                              "        idx_language_column\n"
                              "      , idx_format_column\n"
                              "      , idx_charset_column\n"
                              " FROM\n"
                              "        ctxsys.ctx_user_indexes\n"
                              " WHERE\n"
                              "        idx_name = :nam<char[100]>",
                              "Get the context column designations.",
                              "0801");

QString toOracleExtract::createContextPrefs(
        const QString &owner,
        const QString &name,
        const QString &sql)
{
    QString prefs = "";
    if (PROMPT)
        prefs += PROMPT_SIMPLE.arg("CREATE CONTEXT");
    prefs += qApp->translate("toOracleExtract",
                             "-- Context indexes can be quite complicated depending upon the\n"
                             "-- used parameters. The following is an attempt to recreate this\n"
                             "-- context index. But, a close scrutiny of the following code is\n"
                             "-- strongly recommended.\n\n");
    QString tmp;
    QString pre_name = "";
    QString parameters = "";
    QStringList ql;
    toQList resultset;
    bool first = true;
    bool isDBA = true;
    try
    {
        resultset = toQuery::readQuery(connection(),
                                       "SELECT * FROM ctxsys.ctx_indexes "
                                       "WHERE idx_owner = 'DUMMY'",
                                       toQueryParams());
    }
    catch (...)
    {
        isDBA = false;
    }
    ql << "DATASTORE" << "FILTER" << "LEXER" << "WORDLIST" << "STORAGE";
    // Lets start with discovering the preferences
    QStringList::Iterator it;
    for (it = ql.begin(); it != ql.end(); ++it)
    {
        if (isDBA)
            resultset = toQuery::readQuery(connection(), SQLContextInfoDBA, toQueryParams() << owner << name << *it);
        else
            resultset = toQuery::readQuery(connection(), SQLContextInfo, toQueryParams() << name << *it);
        if (!resultset.empty())
        {
            first = true;
            while (!resultset.empty())
            {
                QString pre_class = (QString)Utils::toShift(resultset);
                QString pre_obj = (QString)Utils::toShift(resultset);
                QString pre_attr = (QString)Utils::toShift(resultset);
                QString pre_val = (QString)Utils::toShift(resultset);
                if (first)
                {
                    first = false;
                    pre_name = QString("%1_%2").arg(name).arg(pre_obj);
                    pre_name.truncate(30);
                    tmp = QString("BEGIN\n  CTX_DDL.CREATE_PREFERENCE('%1','%2');\n")
                          .arg(pre_name).arg(pre_obj);
                    prefs += tmp;
                    parameters += QString("             %1 %2\n")
                                  .arg(pre_class).arg(pre_name);
                }
                tmp = QString("  CTX_DDL.SET_ATTRIBUTE('%1', '%2', '%3');\n")
                      .arg(pre_name).arg(pre_attr).arg(pre_val);
                prefs += tmp;
            }
            prefs += "END;\n\n";
        }
        else
        {
            // some preferences don't have any attributes and
            // so won't be caught above
            if (isDBA)
                resultset = toQuery::readQuery(connection(), SQLContextInfoNoAttrDBA, toQueryParams() << owner << name << *it);
            else
                resultset = toQuery::readQuery(connection(), SQLContextInfoNoAttr, toQueryParams() << name << *it);
            if (!resultset.empty())
            {
                QString pre_obj = (QString)Utils::toShift(resultset);
                pre_name = QString("%1_%2").arg(name).arg(pre_obj);
                pre_name.truncate(30);
                tmp = QString("BEGIN\n  CTX_DDL.CREATE_PREFERENCE('%1', '%2');\nEND;\n\n").arg(pre_name).arg(pre_obj);
                prefs += tmp;
                parameters += QString("             %1 %2\n")
                              .arg(*it).arg(pre_name);
            }
        }
    }

    // Now get the stoplist
    if (isDBA)
        resultset = toQuery::readQuery(connection(), SQLContextInfoDBA, toQueryParams() << owner << name << QString::fromLatin1("STOPLIST"));
    else
        resultset = toQuery::readQuery(connection(), SQLContextInfo, toQueryParams() << name << QString::fromLatin1("STOPLIST"));
    pre_name = "";
    while (!resultset.empty())
    {
        QString pre_class = (QString)Utils::toShift(resultset);
        QString pre_obj = (QString)Utils::toShift(resultset);
        QString pre_attr = (QString)Utils::toShift(resultset);
        QString pre_val = (QString)Utils::toShift(resultset);
        if (pre_name == "")
        {
            pre_name = QString("%1_STOPLIST").arg(name);
            pre_name.truncate(30);
            tmp = QString("BEGIN\n  CTX_DDL.CREATE_STOPLIST('%1');\n")
                  .arg(pre_name);
            prefs += tmp;
            parameters += QString("             STOPLIST %1\n").arg(pre_name);
        }
        pre_attr.remove(4, 1);
        tmp = QString("  CTX_DDL.ADD_%1('%2', '%3');\n")
              .arg(pre_attr).arg(pre_name).arg(pre_val);
        prefs += tmp;
    }
    if (pre_name != "")
        prefs += "END;\n\n";
    else
    {
        // This is most probably redundant but shouldn't hurt.
        // When everybody is out to get you paranoia just seems like a good idea.

        if (isDBA)
            resultset = toQuery::readQuery(connection(), SQLContextInfoNoAttrDBA, toQueryParams() << owner << name << QString::fromLatin1("STOPLIST"));
        else
            resultset = toQuery::readQuery(connection(), SQLContextInfoNoAttr, toQueryParams() << name << QString::fromLatin1("STOPLIST"));

        if (!resultset.empty())
        {
            QString pre_obj = (QString)Utils::toShift(resultset);
            pre_name = QString("%1_%2").arg(name).arg(pre_obj);
            pre_name.truncate(30);
            tmp = QString("BEGIN\n  CTX_DDL.CREATE_STOPLIST('%1');\nEND;\n\n")
                  .arg(pre_name);
            prefs += tmp;
            parameters += QString("             STOPLIST %1\n").arg(pre_name);
        }
    }

    // get the section_groups
    if (isDBA)
        resultset = toQuery::readQuery(connection(), SQLContextInfoDBA, toQueryParams() << owner << name << QString::fromLatin1("SECTION_GROUP"));
    else
        resultset = toQuery::readQuery(connection(), SQLContextInfo, toQueryParams() << name << QString::fromLatin1("SECTION_GROUP"));
    pre_name = "";
    while (!resultset.empty())
    {
        QString pre_class = (QString)Utils::toShift(resultset);
        QString pre_obj = (QString)Utils::toShift(resultset);
        QString pre_attr = (QString)Utils::toShift(resultset);
        QString pre_val = (QString)Utils::toShift(resultset);
        QString pre_val1 = pre_val.left(pre_val.indexOf(':', 0, Qt::CaseInsensitive));
        pre_val = pre_val.right(pre_val.length() - pre_val1.length() - 1);
        QString pre_val2 = pre_val.left(pre_val.indexOf(':', 0, Qt::CaseInsensitive));
        QString pre_val4 = pre_val.right(1);
        if (pre_val4 == "Y")
            pre_val4 = "TRUE";
        else
            pre_val4 = "FALSE";
        if (pre_name == "")
        {
            pre_name = QString("%1_SECTION_GROUP").arg(name);
            pre_name.truncate(30);
            tmp = QString("BEGIN\n  CTX_DDL.CREATE_SECTION_GROUP('%1', '%2');\n")
                  .arg(pre_name).arg(pre_obj);
            prefs += tmp;
            parameters += QString("             SECTION GROUP %1\n").arg(pre_name);
        }
        if (pre_attr == "ZONE")
            tmp = QString("  CTX_DDL.ADD_ZONE_SECTION('%1', '%2', '%3');\n").arg(pre_name).arg(pre_val1).arg(pre_val2);
        else if (pre_attr == "FIELD")
            tmp = QString("  CTX_DDL.ADD_FIELD_SECTION('%1', '%2', '%3', %4);\n").arg(pre_name).arg(pre_val1).arg(pre_val2).arg(pre_val4);
        else if (pre_attr == "SPECIAL")
            tmp = QString("  CTX_DDL.ADD_SPECIAL_SECTION('%1', '%2');\n").arg(pre_name).arg(pre_val1);
        prefs += tmp;
    }
    if (pre_name != "")
        prefs += "END;\n\n";
    else
    {
        if (isDBA)
            resultset = toQuery::readQuery(connection(), SQLContextInfoNoAttrDBA, toQueryParams() << owner << name << QString::fromLatin1("SECTION_GROUP"));
        else
            resultset = toQuery::readQuery(connection(), SQLContextInfoNoAttr, toQueryParams() << name << QString::fromLatin1("SECTION_GROUP"));
        if (!resultset.empty())
        {
            QString pre_obj = (QString)Utils::toShift(resultset);
            pre_name = QString("%1_%2").arg(name).arg(pre_obj);
            pre_name.truncate(30);
            tmp = QString("BEGIN\n  CTX_DDL.CREATE_SECTION_GROUP('%1', '%2');\nEND;\n\n").
                  arg(pre_name).arg(pre_obj);
            prefs += tmp;
            parameters += QString("             SECTION GROUP %1\n").arg(pre_name);
        }
    }

    // Lets look up the language, format, and charset columns
    // only if Oracle 8.1.6 and above
    if (connection().version() >= "080106")
    {
        if (isDBA)
            resultset = toQuery::readQuery(connection(), SQLContextColumnDBA, toQueryParams() << owner << name);
        else
            resultset = toQuery::readQuery(connection(), SQLContextColumn, toQueryParams() << name);
        if (!resultset.empty())
        {
            toQValue vlang = (QString)Utils::toShift(resultset);
            toQValue vfrmt = (QString)Utils::toShift(resultset);
            toQValue vcset = (QString)Utils::toShift(resultset);
            if (!vlang.isNull())
            {
                tmp = QString("             LANGUAGE COLUMN %1\n").arg((QString)vlang);
                parameters += tmp;
            }
            if (!vfrmt.isNull())
            {
                tmp = QString("             FORMAT COLUMN %1\n").arg((QString)vfrmt);
                parameters += tmp;
            }
            if (!vcset.isNull())
            {
                tmp = QString("             CHARSET COLUMN %1\n").arg((QString)vcset);
                parameters += tmp;
            }
        }
    }
    prefs += sql;
    parameters += "            ";
    tmp = QString("INDEXTYPE IS CTXSYS.CONTEXT\nPARAMETERS ('\n%1');\n\n").
          arg(parameters);
    prefs += tmp;
    return prefs;
}

static toSQL SQLIOTInfo("toOracleExtract:IOTInfo",
                        "SELECT\n"
                        "        DECODE(\n"
                        "                b.monitoring\n"
                        "               ,'NO','NOMONITORING'\n"
                        "               ,     'MONITORING'\n"
                        "              )\n"
                        "      , 'N/A'                         AS table_name\n"
                        "      , LTRIM(a.degree)\n"
                        "      , LTRIM(a.instances)\n"
                        "      , 'INDEX'                       AS organization\n"
                        "      , 'N/A'                         AS cache\n"
                        "      , 'N/A'                         AS pct_used\n"
                        "      , a.pct_free\n"
                        "      , DECODE(\n"
                        "                a.ini_trans\n"
                        "               ,0,1\n"
                        "               ,null,1\n"
                        "               ,a.ini_trans\n"
                        "              )                       AS ini_trans\n"
                        "      , DECODE(\n"
                        "                a.max_trans\n"
                        "               ,0,255\n"
                        "               ,null,255\n"
                        "               ,a.max_trans\n"
                        "              )                       AS max_trans\n"
                        "      , a.initial_extent\n"
                        "      , a.next_extent\n"
                        "      , a.min_extents\n"
                        "      , DECODE(\n"
                        "                a.max_extents\n"
                        "               ,2147483645,'unlimited'\n"
                        "               ,a.max_extents\n"
                        "              )                       AS max_extents\n"
                        "      , a.pct_increase\n"
                        "      , NVL(a.freelists,1)\n"
                        "      , NVL(a.freelist_groups,1)\n"
                        "      , a.buffer_pool                 AS buffer_pool\n"
                        "      , DECODE(\n"
                        "                b.logging\n"
                        "               ,'NO','NOLOGGING'\n"
                        "               ,     'LOGGING'\n"
                        "              )                       AS logging\n"
                        "      , a.tablespace_name             AS tablespace_name\n"
                        "      , DECODE(\n"
                        "                b.blocks\n"
                        "                ,null,GREATEST(a.initial_extent,a.next_extent) \n"
                        "                      / (b.blocks * 1024)\n"
                        "                ,'0' ,GREATEST(a.initial_extent,a.next_extent)\n"
                        "                      / (b.blocks * 1024)\n"
                        "                ,b.blocks\n"
                        "              )                       AS blocks\n"
                        " FROM\n"
                        "        sys.all_indexes a,\n"
                        "        sys.all_all_tables b,\n"
                        "        sys.all_constraints c\n"
                        " WHERE  a.table_name  = :nam<char[100]>\n"
                        "   AND  b.owner = a.owner\n"
                        "   AND  b.table_name = a.table_name\n"
                        "   AND  a.owner = :own<char[100]>\n"
                        "   AND  c.constraint_name = a.index_name\n"
                        "   AND  c.owner = a.owner\n"
                        "   AND  c.constraint_type = 'P'",
                        "Get storage information about a IOT storage, "
                        "same binds and columns");

static toSQL SQLOverflowInfo("toOracleExtract:OverflowInfo",
                             "SELECT\n"
                             "        '  '\n"
                             "      , 'N/A'\n"
                             "      , DECODE(\n"
                             "                LTRIM(t.cache)\n"
                             "               ,'Y','CACHE'\n"
                             "               ,    'NOCACHE'\n"
                             "              )\n"
                             "      , t.pct_used\n"
                             "      , t.pct_free\n"
                             "      , DECODE(\n"
                             "                t.ini_trans\n"
                             "               ,0,1\n"
                             "               ,null,1\n"
                             "               ,t.ini_trans\n"
                             "              )                       AS ini_trans\n"
                             "      , DECODE(\n"
                             "                t.max_trans\n"
                             "               ,0,255\n"
                             "               ,null,255\n"
                             "               ,t.max_trans\n"
                             "              )                       AS max_trans\n"
                             "      , t.initial_extent\n"
                             "      , t.next_extent\n"
                             "      , t.min_extents\n"
                             "      , DECODE(\n"
                             "                t.max_extents\n"
                             "               ,2147483645,'unlimited'\n"
                             "               ,           t.max_extents\n"
                             "              )                       AS max_extents\n"
                             "      , NVL(t.pct_increase,0)\n"
                             "      , NVL(t.freelists,1)\n"
                             "      , NVL(t.freelist_groups,1)\n"
                             "      , t.buffer_pool                 AS buffer_pool\n"
                             "      , DECODE(\n"
                             "                t.logging\n"
                             "               ,'NO','NOLOGGING'\n"
                             "               ,     'LOGGING'\n"
                             "              )                       AS logging\n"
                             "      , t.tablespace_name             AS tablespace_name\n"
                             "      , s.blocks - NVL(t.empty_blocks,0)\n"
                             " FROM\n"
                             "        sys.all_tables    t\n"
                             "      , %1  s\n"
                             " WHERE\n"
                             "            t.iot_name     = :nam<char[100]>\n"
                             "        AND t.table_name   = s.segment_name\n"
                             "        AND s.segment_type = 'TABLE'\n"
                             "        AND s.owner        = :own<char[100]>\n"
                             "        AND t.owner        = s.owner",
                             "Get information about overflow segment for table, must have same binds and columns",
                             "0800");

QString toOracleExtract::createIOT(const QString &owner
                                   , const QString &name)
{
    toQList storage = toQuery::readQuery(connection(), SQLIOTInfo, toQueryParams() << name << owner);

    QString ret = createTableText(storage, owner, name);
    if (ext.getStorage())
    {
        toQList overflow = toQuery::readQuery(connection(), segments(SQLOverflowInfo), toQueryParams() << name << owner);
        if (overflow.size() == 18)
        {
            ret += "OVERFLOW\n";
            ret += segmentAttributes(overflow);
        }
    }
    ret += ";\n\n";
    ret += createComments(owner, name);
    return ret;
}

static toSQL SQLPrimaryKey("toOracleExtract:PrimaryKey",
                           "SELECT a.constraint_name,b.tablespace_name\n"
                           "  FROM sys.all_constraints a,sys.all_indexes b\n"
                           " WHERE a.table_name = :nam<char[100]>\n"
                           "   AND a.constraint_type = 'P'\n"
                           "   AND a.owner = :own<char[100]>\n"
                           "   AND a.owner = b.owner\n"
                           "   AND a.constraint_name = b.index_name",
                           "Get constraint name for primary key of table, same binds and columns");

QString toOracleExtract::createTableText(
        toQList &result,
        const QString &owner,
        const QString &name)
{
    using namespace ToConfiguration;
    QString monitoring = (QString)Utils::toShift(result);
    QString table = (QString)Utils::toShift(result);
    QString degree = (QString)Utils::toShift(result);
    QString instances = (QString)Utils::toShift(result);
    QString organization = (QString)Utils::toShift(result);

    QString ret;
    if (PROMPT)
        ret = QString("PROMPT CREATE TABLE %1%2\n\n").arg(quote(owner)).arg(quote(name));
    ret += QString("CREATE TABLE %1%2\n(\n    ").arg(quote(owner)).arg(quote(name));
    ret += tableColumns(owner, name);
    if (organization == "INDEX" && ext.getStorage())
    {
        toQList res = toQuery::readQuery(connection(), SQLPrimaryKey, toQueryParams() << name << owner);
        if (res.size() != 2)
            throw qApp->translate("toOracleExtract", "Couldn't find primary key of %1.%2").arg(owner).arg(name);
        QString primary = (QString)*(res.begin());
        QString tablespace = (QString)*(res.rbegin());
        ret += QString("  , CONSTRAINT %1 PRIMARY KEY\n").arg(quote(primary));
        ret += indexColumns("      ", owner, primary);
        ret += QString("      USING TABLESPACE %2\n").arg(quote(tablespace));
    }
    ret += ")\n";
    if (!toConfigurationNewSingle::Instance().option(Oracle::SkipOrgMonBool).toBool())
    {
        if (connection().version() >= "0800" && ext.getStorage() && ! organization.isEmpty() )
        {
            ret += "ORGANIZATION        ";
            ret += organization;
            ret += "\n";
        }
        if (connection().version() >= "0801" && ext.getStorage() && ! monitoring.isEmpty() )
        {
            ret += monitoring;
            ret += "\n";
        }
    }
    if (ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty() )
        ret += QString("PARALLEL\n"
                       "(\n"
                       "  DEGREE            %1\n"
                       "  INSTANCES         %2\n"
                       ")\n").
               arg(degree).
               arg(instances);

    Utils::toUnShift(result, toQValue(organization));
    Utils::toUnShift(result, toQValue(""));

    ret += segmentAttributes(result);

    return ret;
}

static toSQL SQLMViewInfo("toOracleExtract:MaterializedViewInfo",
                          "       SELECT\n"
                          "              m.container_name\n"
                          "            , DECODE(\n"
                          "                      m.build_mode\n"
                          "                     ,'YES','USING PREBUILT TABLE'\n"
                          "                     ,DECODE(\n"
                          "                              m.last_refresh_date\n"
                          "                             ,null,'BUILD DEFERRED'\n"
                          "                             ,'BUILD IMMEDIATE'\n"
                          "                            )\n"
                          "                    )                                  AS build_mode\n"
                          "            , DECODE(\n"
                          "                      m.refresh_method\n"
                          "                     ,'NEVER','NEVER REFRESH'\n"
                          "                     ,'REFRESH ' || m.refresh_method\n"
                          "                    )                                  AS refresh_method\n"
                          "            , DECODE(\n"
                          "                      m.refresh_mode\n"
                          "                     ,'NEVER',null\n"
                          "                     ,'ON ' || m.refresh_mode\n"
                          "                    )                                  AS refresh_mode\n"
                          "            , TO_CHAR(s.start_with, 'DD-MON-YYYY HH24:MI:SS')\n"
                          "                                                       AS start_with\n"
                          "            , s.next\n"
                          "            , DECODE(\n"
                          "                      s.refresh_method\n"
                          "                     ,'PRIMARY KEY','WITH  PRIMARY KEY'\n"
                          "                     ,'ROWID'      ,'WITH  ROWID'\n"
                          "                     ,null\n"
                          "                    )                                  AS using_pk\n"
                          "            , s.master_rollback_seg\n"
                          "            , DECODE(\n"
                          "                      m.updatable\n"
                          "                     ,'N',null\n"
                          "                     ,DECODE(\n"
                          "                              m.rewrite_enabled\n"
                          "                             ,'Y','FOR UPDATE ENABLE QUERY REWRITE'\n"
                          "                             ,'N','FOR UPDATE DISABLE QUERY REWRITE'\n"
                          "                            )\n"
                          "                    )                                  AS updatable\n"
                          "            , s.query\n"
                          "       FROM\n"
                          "              sys.all_mviews     m\n"
                          "            , sys.all_snapshots  s\n"
                          "       WHERE\n"
                          "                  m.mview_name  = :nam<char[100]>\n"
                          "              AND s.name        = m.mview_name\n"
                          "              AND m.owner       = :own<char[100]>\n"
                          "              AND s.owner       = m.owner",
                          "Get information about materialized view, must have same columns and binds");

static toSQL SQLIndexName("toOracleExtract:TableIndexes",
                          "SELECT index_name\n"
                          "  FROM sys.all_indexes\n"
                          " WHERE table_name = :nam<char[100]>\n"
                          "   AND owner = own<char[100]>",
                          "Get indexes available to a table, must have same binds and columns");

QString toOracleExtract::createMView(
                                     const QString &owner,
                                     const QString &name,
                                     const QString &type)
{
    toConnectionSubLoan conn(connection());
    toQList result = toQuery::readQuery(connection(), SQLMViewInfo, toQueryParams() << name << owner);
    if (result.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find materialised table %1.%2").
        arg(quote(owner)).arg(quote(name));
    QString table = (QString)Utils::toShift(result);
    QString buildMode = (QString)Utils::toShift(result);
    QString refreshMethod = (QString)Utils::toShift(result);
    QString refreshMode = (QString)Utils::toShift(result);
    QString startWith = (QString)Utils::toShift(result);
    QString next = (QString)Utils::toShift(result);
    QString usingPK = (QString)Utils::toShift(result);
    QString masterRBSeg = (QString)Utils::toShift(result);
    QString updatable = (QString)Utils::toShift(result);
    QString query = (QString)Utils::toShift(result);

    toQuery inf(conn, SQLIndexName, toQueryParams() << table << owner);
    QString index(inf.readValue());

    QString ret;
    QString sql = QString("CREATE %1 %2.%3\n").arg(type).arg(quote(owner)).arg(quote(name));
    if (PROMPT)
        ret = PROMPT_SIMPLE.arg(sql);
    ret += sql;
    ret += createMViewTable(owner, table);
    ret += buildMode;
    ret += "\nUSING INDEX\n";
    ret += createMViewIndex(owner, index);

    if (refreshMethod != "NEVER REFRESH")
    {
        if (!startWith.isEmpty())
            ret += QString("START WITH TO_DATE('%1','DD-MM-YYYY HH24:MI:SS')\n").
                   arg(startWith);
        if (!next.isEmpty())
            ret += QString("NEXT  %1\n").arg(next);
        if (!usingPK.isEmpty())
        {
            ret += usingPK;
            ret += "\n";
        }
        if (!masterRBSeg.isEmpty() && ext.getStorage())
            ret += QString("USING MASTER ROLLBACK SEGMENT %1\n").
                   arg(quote(masterRBSeg));
    }

    if (!updatable.isEmpty())
    {
        ret += updatable;
        ret += "\n";
    }
    ret += "AS\n";
    ret += query;
    ret += ";\n\n\n";
    return ret;
}

QString toOracleExtract::createMViewIndex(
        const QString &owner,
        const QString &name)
{
    if (!ext.getStorage() || !ext.getParallel())
        return "";
    ext.setState("IsASnapIndex", true);

    static QRegExp start("^INITRANS");
    static QRegExp ignore("LOGGING");

    bool started = false;
    bool done = false;

    QString initial = createIndex(owner, name);
    QStringList linesIn = initial.split("\n");
    QString ret;

    for (QStringList::Iterator i = linesIn.begin(); i != linesIn.end() && !done; i++)
    {
        if (start.indexIn(*i) >= 0)
            started = true;
        if (started)
        {
            QString line = *i;
            if (line.length() > 0 && line.at(line.length() - 1) == ';')
            {
                line.truncate(line.length() - 1);
                done = true;
            }
            if (ignore.indexIn(line) < 0 && line.length() > 0)
            {
                ret += line;
                ret += "\n";
            }
        }
    }

    ext.setState("IsASnapIndex", false);
    return ret;
}

static toSQL SQLSnapshotInfo("toOracleExtract:SnapshotInfo",
                             "SELECT log_table,\n"
                             "       rowids,\n"
                             "       primary_key,\n"
                             "       filter_columns\n"
                             "  FROM sys.all_snapshot_logs\n"
                             "   AND master = :nam<char[100]>\n"
                             "   AND log_owner = :own<char[100]>",
                             "Get information about snapshot or materialized view log, "
                             "must have same binds and columns");

static toSQL SQLSnapshotColumns("toOracleExtract:SnapshotColumns",
                                "SELECT\n"
                                "        column_name\n"
                                " FROM\n"
                                "        sys.dba_snapshot_log_filter_cols\n"
                                " WHERE\n"
                                "            name  = :nam<char[100]>\n"
                                "        AND owner = :own<char[100]>\n"
                                " MINUS\n"
                                " SELECT\n"
                                "        column_name\n"
                                " FROM\n"
                                "        sys.all_cons_columns  c\n"
                                "      , sys.all_constraints   d\n"
                                " WHERE\n"
                                "            d.table_name      = :nam<char[100]>\n"
                                "        AND d.constraint_type = 'P'\n"
                                "        AND c.table_name      = d.table_name\n"
                                "        AND c.constraint_name = d.constraint_name\n"
                                "        AND d.owner           = :own<char[100]>\n"
                                "        AND c.owner           = d.owner",
                                "Get columns of snapshot log, must have same columns and binds");

QString toOracleExtract::createMViewLog( 
                                        const QString &owner,
                                        const QString &name,
                                        const QString &type)
{
    toQList result = toQuery::readQuery(connection(), SQLSnapshotInfo, toQueryParams() << name << owner);
    if (result.size() != 4)
        throw qApp->translate("toOracleExtract", "Couldn't find log %1.%2").arg(owner).arg(name);

    QString table = (QString)Utils::toShift(result);
    QString rowIds = (QString)Utils::toShift(result);
    QString primaryKey = (QString)Utils::toShift(result);
    QString filterColumns = (QString)Utils::toShift(result);

    QString ret;
    QString sql = QString("CREATE %1 LOG ON %2.%3\n").
                  arg(type).
                  arg(quote(owner)).
                  arg(quote(name));
    if (PROMPT)
        QString ret = PROMPT_SIMPLE.arg(sql);
    ret += sql;
    ret += createMViewTable(owner, table);

    if (rowIds == "YES" && primaryKey == "YES")
        ret += "WITH PRIMARY KEY, ROWID ";
    else if (rowIds == "YES")
        ret += "WITH ROWID ";
    else if (primaryKey == "YES")
        ret += "WITH PRIMARY KEY";

    toConnectionSubLoan conn(connection());
    toQuery inf(conn, SQLSnapshotColumns, toQueryParams() << name << owner);
    bool first = true;
    if (!inf.eof())
    {
        ret += "\n(\n    ";
        while (!inf.eof())
        {
            if (first)
                first = false;
            else
                ret += "\n  , ";
            ret += quote((QString)inf.readValue());
        }
        ret += ")";
    }
    ret += ";\n\n\n";
    return ret;
}

QString toOracleExtract::createMViewTable(
        const QString &owner,
        const QString &name)
{
    if (!ext.getStorage() || !ext.getParallel())
        return "";
    ext.setState("IsASnapTable", true);

    static QRegExp parallel("^PARALLEL");

    bool started = false;
    bool done = false;

    QString initial = createTable(owner, name);
    QStringList linesIn = initial.split("\n");
    QString ret;

    for (QStringList::Iterator i = linesIn.begin(); i != linesIn.end() && !done; i++)
    {
        if (parallel.indexIn(*i) >= 0)
            started = true;
        if (started)
        {
            QString line = *i;
            if (line.length() > 0 && line.at(line.length() - 1) == ';')
            {
                line.truncate(line.length() - 1);
                done = true;
            }
            if (line.length())
            {
                ret += line;
                ret += "\n";
            }
        }
    }

    ext.setState("IsASnapTable", false);
    return ret;
}

static toSQL SQLPartitionedIOTInfo("toOracleExtract:PartitionedIOTInfo",
                                   "SELECT\n"
                                   "        DECODE(\n"
                                   "                t.monitoring\n"
                                   "               ,'NO','NOMONITORING'\n"
                                   "               ,     'MONITORING'\n"
                                   "              )                       AS monitoring\n"
                                   "      , t.table_name\n"
                                   "      , LTRIM(t.degree)               AS degree\n"
                                   "      , LTRIM(t.instances)            AS instances\n"
                                   "      , 'INDEX'                       AS organization\n"
                                   "      , DECODE(\n"
                                   "                LTRIM(t.cache)\n"
                                   "               ,'Y','CACHE'\n"
                                   "               ,    'NOCACHE'\n"
                                   "              )                       AS cache\n"
                                   "      , 'N/A'                         AS pct_used\n"
                                   "      , p.def_pct_free                AS pct_free\n"
                                   "      , p.def_ini_trans               AS ini_trans\n"
                                   "      , p.def_max_trans               AS max_trans\n"
                                   "      ,DECODE(\n"
                                   "               p.def_initial_extent\n"
                                   "              ,'DEFAULT',s.initial_extent\n"
                                   "              ,p.def_initial_extent * :bs<char[100]> * 1024\n"
                                   "             )                        AS initial_extent\n"
                                   "      ,DECODE(\n"
                                   "               p.def_next_extent\n"
                                   "              ,'DEFAULT',s.next_extent\n"
                                   "              ,p.def_next_extent * :bs<char[100]> * 1024\n"
                                   "             )                        AS next_extent\n"
                                   "      , DECODE(\n"
                                   "                p.def_min_extents\n"
                                   "               ,'DEFAULT',s.min_extents\n"
                                   "               ,p.def_min_extents\n"
                                   "              )                       AS min_extents\n"
                                   "      , DECODE(\n"
                                   "                p.def_max_extents\n"
                                   "               ,'DEFAULT',DECODE(\n"
                                   "                                  s.max_extents\n"
                                   "                                 ,2147483645,'unlimited'\n"
                                   "                                 ,s.max_extents\n"
                                   "                                )\n"
                                   "               ,2147483645,'unlimited'\n"
                                   "               ,p.def_max_extents\n"
                                   "              )                       AS max_extents\n"
                                   "      , DECODE(\n"
                                   "                p.def_pct_increase\n"
                                   "               ,'DEFAULT',s.pct_increase\n"
                                   "               ,p.def_pct_increase\n"
                                   "              )                       AS pct_increase\n"
                                   "      , DECODE(\n"
                                   "                p.def_freelists\n"
                                   "               ,0,1\n"
                                   "               ,NVL(p.def_freelists,1)\n"
                                   "              )                       AS freelists\n"
                                   "      , DECODE(\n"
                                   "                p.def_freelist_groups\n"
                                   "               ,0,1\n"
                                   "               ,NVL(p.def_freelist_groups,1)\n"
                                   "              )                       AS freelist_groups\n"
                                   "      , p.def_buffer_pool             AS buffer_pool\n"
                                   "      , DECODE(\n"
                                   "                p.def_logging \n"
                                   "               ,'NO','NOLOGGING'\n"
                                   "               ,     'LOGGING'\n"
                                   "              )                       AS logging\n"
                                   "      , p.def_tablespace_name         AS tablespace_name\n"
                                   "      , t.blocks - NVL(t.empty_blocks,0)\n"
                                   " FROM\n"
                                   "        sys.all_all_tables    t\n"
                                   "      , sys.all_part_indexes  p\n"
                                   "      , sys.dba_tablespaces   s\n"
                                   " WHERE\n"
                                   "            t.table_name      = :name<char[100]>\n"
                                   "        AND p.table_name      = t.table_name\n"
                                   "        AND s.tablespace_name = p.def_tablespace_name\n"
                                   "        AND t.owner           = :own<char[100]>\n"
                                   "        AND p.owner           = t.owner",
                                   "Get information about a partitioned indexed organized table, "
                                   "must have same columns and binds",
                                   "0801");

static toSQL SQLPartitionedIOTInfo8("toOracleExtract:PartitionedIOTInfo",
                                    "SELECT\n"
                                    "        'N/A'                         AS monitoring\n"
                                    "      , t.table_name\n"
                                    "      , LTRIM(t.degree)               AS degree\n"
                                    "      , LTRIM(t.instances)            AS instances\n"
                                    "      , 'INDEX'                       AS organization\n"
                                    "      , DECODE(\n"
                                    "                LTRIM(t.cache)\n"
                                    "               ,'Y','CACHE'\n"
                                    "               ,    'NOCACHE'\n"
                                    "              )                       AS cache\n"
                                    "      , 'N/A'                         AS pct_used\n"
                                    "      , p.def_pct_free                AS pct_free\n"
                                    "      , p.def_ini_trans               AS ini_trans\n"
                                    "      , p.def_max_trans               AS max_trans\n"
                                    "      ,DECODE(\n"
                                    "               p.def_initial_extent\n"
                                    "              ,'DEFAULT',s.initial_extent\n"
                                    "              ,p.def_initial_extent * :bs<char[100]> * 1024\n"
                                    "             )                        AS initial_extent\n"
                                    "      ,DECODE(\n"
                                    "               p.def_next_extent\n"
                                    "              ,'DEFAULT',s.next_extent\n"
                                    "              ,p.def_next_extent * :bs<char[100]> * 1024\n"
                                    "             )                        AS next_extent\n"
                                    "      , DECODE(\n"
                                    "                p.def_min_extents\n"
                                    "               ,'DEFAULT',s.min_extents\n"
                                    "               ,p.def_min_extents\n"
                                    "              )                       AS min_extents\n"
                                    "      , DECODE(\n"
                                    "                p.def_max_extents\n"
                                    "               ,'DEFAULT',DECODE(\n"
                                    "                                  s.max_extents\n"
                                    "                                 ,2147483645,'unlimited'\n"
                                    "                                 ,s.max_extents\n"
                                    "                                )\n"
                                    "               ,2147483645,'unlimited'\n"
                                    "               ,p.def_max_extents\n"
                                    "              )                       AS max_extents\n"
                                    "      , DECODE(\n"
                                    "                p.def_pct_increase\n"
                                    "               ,'DEFAULT',s.pct_increase\n"
                                    "               ,p.def_pct_increase\n"
                                    "              )                       AS pct_increase\n"
                                    "      , DECODE(\n"
                                    "                p.def_freelists\n"
                                    "               ,0,1\n"
                                    "               ,NVL(p.def_freelists,1)\n"
                                    "              )                       AS freelists\n"
                                    "      , DECODE(\n"
                                    "                p.def_freelist_groups\n"
                                    "               ,0,1\n"
                                    "               ,NVL(p.def_freelist_groups,1)\n"
                                    "              )                       AS freelist_groups\n"
                                    "      , 'N/A'                         AS buffer_pool\n"
                                    "      , DECODE(\n"
                                    "                p.def_logging \n"
                                    "               ,'NO','NOLOGGING'\n"
                                    "               ,     'LOGGING'\n"
                                    "              )                       AS logging\n"
                                    "      , p.def_tablespace_name         AS tablespace_name\n"
                                    "      , t.blocks - NVL(t.empty_blocks,0)\n"
                                    " FROM\n"
                                    "        sys.all_all_tables    t\n"
                                    "      , sys.all_part_indexes  p\n"
                                    "      , sys.dba_tablespaces   s\n"
                                    " WHERE\n"
                                    "            t.table_name      = :name<char[100]>\n"
                                    "        AND p.table_name      = t.table_name\n"
                                    "        AND s.tablespace_name = p.def_tablespace_name\n"
                                    "        AND t.owner           = :own<char[100]>\n"
                                    "        AND p.owner           = t.owner",
                                    "",
                                    "0800");

static toSQL SQLPartitionIndexNames("toOracleExtract:PartitionIndexNames",
                                    "SELECT index_name\n"
                                    "  FROM sys.all_part_indexes\n"
                                    " WHERE table_name = :nam<char[100]>\n"
                                    "   AND owner      = :own<char[100]>\n"
                                    " ORDER BY index_name",
                                    "Index names of partition table, "
                                    "must have same binds and columns");

QString toOracleExtract::createPartitionedIOT(
        const QString &owner,
        const QString &name)
{
    toQList result = toQuery::readQuery(connection(), SQLPartitionedIOTInfo,
                                        toQueryParams() << QString::number(ext.getBlockSize()) << name << owner);
    QString ret = createTableText(result, owner, name);
    if (ext.getStorage())
    {
        toQList overflow = toQuery::readQuery(connection(), segments(SQLOverflowInfo), toQueryParams() << name << owner);
        if (overflow.size() == 18)
        {
            ret += "OVERFLOW\n";
            ret += segmentAttributes(overflow);
        }
    }

    if (ext.getPartition())
    {
        toConnectionSubLoan conn(connection());
        toQuery inf(conn, SQLPartitionIndexNames, toQueryParams() << name << owner);
        if (!inf.eof())
            throw qApp->translate("toOracleExtract", "Couldn't find index partitions for %1.%2").arg(owner).arg(name);
        QString index(inf.readValue());
        ret +=
            "PARTITION BY RANGE\n"
            "(\n    ";
        ret += partitionKeyColumns(owner, name, "TABLE");
        ret += "\n)\n";
        ret += rangePartitions(owner, index, "NONE", "IOT");
    }
    ret += createComments(owner, name);
    return ret;
}

static toSQL SQLIndexPartition8("toOracleExtract:IndexPartition",
                                "SELECT\n"
                                "        i.partitioning_type\n"
                                "      , 'N/A'                         AS subpartitioning_type\n"
                                "      , i.locality\n"
                                "      , 0                             AS compressed\n"
                                "      , 'INDEX'                       AS organization\n"
                                "      , 'N/A'                         AS cache\n"
                                "      , 'N/A'                         AS pct_used\n"
                                "      , i.def_pct_free\n"
                                "      , DECODE(\n"
                                "                i.def_ini_trans\n"
                                "               ,0,1\n"
                                "               ,null,1\n"
                                "               ,i.def_ini_trans\n"
                                "              )                       AS ini_trans\n"
                                "      , DECODE(\n"
                                "                i.def_max_trans\n"
                                "               ,0,255\n"
                                "               ,null,255\n"
                                "               ,i.def_max_trans\n"
                                "              )                       AS max_trans\n"
                                "      ,DECODE(\n"
                                "               i.def_initial_extent\n"
                                "              ,'DEFAULT',s.initial_extent\n"
                                "              ,i.def_initial_extent * :bs<char[100]> * 1024\n"
                                "             )                        AS initial_extent\n"
                                "      ,DECODE(\n"
                                "               i.def_next_extent\n"
                                "              ,'DEFAULT',s.next_extent\n"
                                "              ,i.def_next_extent * :bs<char[100]> * 1024\n"
                                "             )                        AS next_extent\n"
                                "      , DECODE(\n"
                                "                i.def_min_extents\n"
                                "               ,'DEFAULT',s.min_extents\n"
                                "               ,i.def_min_extents\n"
                                "              )                       AS min_extents\n"
                                "      , DECODE(\n"
                                "                i.def_max_extents\n"
                                "               ,'DEFAULT',DECODE(\n"
                                "                                  s.max_extents\n"
                                "                                 ,2147483645,'unlimited'\n"
                                "                                 ,s.max_extents\n"
                                "                                )\n"
                                "               ,2147483645,'unlimited'\n"
                                "               ,i.def_max_extents\n"
                                "              )                       AS max_extents\n"
                                "      , DECODE(\n"
                                "                i.def_pct_increase\n"
                                "               ,'DEFAULT',s.pct_increase\n"
                                "               ,i.def_pct_increase\n"
                                "              )                       AS pct_increase\n"
                                "      , DECODE(\n"
                                "                i.def_freelists\n"
                                "               ,0,1\n"
                                "               ,null,1\n"
                                "               ,i.def_freelists\n"
                                "              )                       AS freelists\n"
                                "      , DECODE(\n"
                                "                i.def_freelist_groups\n"
                                "               ,0,1\n"
                                "               ,null,1\n"
                                "               ,i.def_freelist_groups\n"
                                "              )                       AS freelist_groups\n"
                                "      , 'N/A'                         AS buffer_pool\n"
                                "      , DECODE(\n"
                                "                i.def_logging\n"
                                "               ,'NO','NOLOGGING'\n"
                                "               ,     'LOGGING'\n"
                                "              )                       AS logging\n"
                                "      , NVL(i.def_tablespace_name,s.tablespace_name)\n"
                                "      , GREATEST(\n"
                                "                  DECODE(\n"
                                "                          i.def_initial_extent\n"
                                "                         ,'DEFAULT',s.initial_extent / :bs<char[100]> / 1024\n"
                                "                         ,i.def_initial_extent\n"
                                "                        )\n"
                                "                 ,DECODE(\n"
                                "                          i.def_next_extent\n"
                                "                         ,'DEFAULT',s.next_extent / :bs<char[100]> / 1024\n"
                                "                         ,i.def_next_extent\n"
                                "                        )\n"
                                "                )                     AS blocks\n"
                                " FROM\n"
                                "        sys.all_part_indexes  i\n"
                                "      , sys.dba_tablespaces   s\n"
                                "      , sys.all_part_tables   t\n"
                                " WHERE\n"
                                "            i.index_name      = :nam<char[100]>\n"
                                "        AND t.table_name      = i.table_name\n"
                                "        AND s.tablespace_name = t.def_tablespace_name\n"
                                "        AND i.owner           = :own<char[100]>\n"
                                "        AND n.owner           = i.owner\n"
                                "        AND t.owner           = i.owner",
                                "Get information about index partitions, "
                                "must have same columns and same columns",
                                "0800");

static toSQL SQLIndexPartition("toOracleExtract:IndexPartition",
                               "SELECT\n"
                               "        i.partitioning_type\n"
                               "      , i.subpartitioning_type\n"
                               "      , i.locality\n"
                               "      , DECODE(\n"
                               "                n.compression\n"
                               "               ,'ENABLED',n.prefix_length\n"
                               "               ,0\n"
                               "              )                             AS compressed\n"
                               "      , 'INDEX'                       AS organization\n"
                               "      , 'N/A'                         AS cache\n"
                               "      , 'N/A'                         AS pct_used\n"
                               "      , i.def_pct_free\n"
                               "      , DECODE(\n"
                               "                i.def_ini_trans\n"
                               "               ,0,1\n"
                               "               ,null,1\n"
                               "               ,i.def_ini_trans\n"
                               "              )                       AS ini_trans\n"
                               "      , DECODE(\n"
                               "                i.def_max_trans\n"
                               "               ,0,255\n"
                               "               ,null,255\n"
                               "               ,i.def_max_trans\n"
                               "              )                       AS max_trans\n"
                               "      ,DECODE(\n"
                               "               i.def_initial_extent\n"
                               "              ,'DEFAULT',s.initial_extent\n"
                               "              ,i.def_initial_extent * :bs<char[100]> * 1024\n"
                               "             )                        AS initial_extent\n"
                               "      ,DECODE(\n"
                               "               i.def_next_extent\n"
                               "              ,'DEFAULT',s.next_extent\n"
                               "              ,i.def_next_extent * :bs<char[100]> * 1024\n"
                               "             )                        AS next_extent\n"
                               "      , DECODE(\n"
                               "                i.def_min_extents\n"
                               "               ,'DEFAULT',s.min_extents\n"
                               "               ,i.def_min_extents\n"
                               "              )                       AS min_extents\n"
                               "      , DECODE(\n"
                               "                i.def_max_extents\n"
                               "               ,'DEFAULT',DECODE(\n"
                               "                                  s.max_extents\n"
                               "                                 ,2147483645,'unlimited'\n"
                               "                                 ,s.max_extents\n"
                               "                                )\n"
                               "               ,2147483645,'unlimited'\n"
                               "               ,i.def_max_extents\n"
                               "              )                       AS max_extents\n"
                               "      , DECODE(\n"
                               "                i.def_pct_increase\n"
                               "               ,'DEFAULT',s.pct_increase\n"
                               "               ,i.def_pct_increase\n"
                               "              )                       AS pct_increase\n"
                               "      , DECODE(\n"
                               "                i.def_freelists\n"
                               "               ,0,1\n"
                               "               ,null,1\n"
                               "               ,i.def_freelists\n"
                               "              )                       AS freelists\n"
                               "      , DECODE(\n"
                               "                i.def_freelist_groups\n"
                               "               ,0,1\n"
                               "               ,null,1\n"
                               "               ,i.def_freelist_groups\n"
                               "              )                       AS freelist_groups\n"
                               "      , i.def_buffer_pool             AS buffer_pool\n"
                               "      , DECODE(\n"
                               "                i.def_logging\n"
                               "               ,'NO','NOLOGGING'\n"
                               "               ,     'LOGGING'\n"
                               "              )                       AS logging\n"
                               "      , NVL(i.def_tablespace_name,s.tablespace_name)\n"
                               "      , GREATEST(\n"
                               "                  DECODE(\n"
                               "                          i.def_initial_extent\n"
                               "                         ,'DEFAULT',s.initial_extent / :bs<char[100]> / 1024\n"
                               "                         ,i.def_initial_extent\n"
                               "                        )\n"
                               "                 ,DECODE(\n"
                               "                          i.def_next_extent\n"
                               "                         ,'DEFAULT',s.next_extent / :bs<char[100]> / 1024\n"
                               "                         ,i.def_next_extent\n"
                               "                        )\n"
                               "                )                     AS blocks\n"
                               " FROM\n"
                               "        sys.all_part_indexes  i\n"
                               "      , sys.all_indexes       n\n"
                               "      , sys.dba_tablespaces   s\n"
                               "      , sys.all_part_tables   t\n"
                               " WHERE\n"
                               "            i.index_name      = :nam<char[100]>\n"
                               "        AND n.index_name      = i.index_name\n"
                               "        AND t.table_name      = i.table_name\n"
                               "        AND s.tablespace_name = t.def_tablespace_name\n"
                               "        AND i.owner           = :own<char[100]>\n"
                               "        AND n.owner           = i.owner\n"
                               "        AND t.owner           = i.owner",
                               "",
                               "0801");

QString toOracleExtract::createPartitionedIndex(
        const QString &owner,
        const QString &name,
        const QString &sql)
{
    if (!ext.getPartition())
        return "";

    QString ret = sql;
    toQList result = toQuery::readQuery(connection(), SQLIndexPartition, toQueryParams() << QString::number(ext.getBlockSize()) << name << owner);
    QString partitionType = (QString)Utils::toShift(result);
    QString subPartitionType = (QString)Utils::toShift(result);
    QString locality = (QString)Utils::toShift(result);
    QString compressed = (QString)Utils::toShift(result);
    Utils::toUnShift(result, toQValue(""));

    ret += segmentAttributes(result);

    if (!compressed.isEmpty())
    {
        ret += "COMPRESS            ";
        ret += compressed;
        ret += "\n";
    }

    if (locality == "GLOBAL")
    {
        ret +=
            "GLOBAL PARTITION BY RANGE\n"
            "(\n"
            "    ";
        ret += partitionKeyColumns(owner, name, "INDEX");
        ret += "\n)\n";
        ret += rangePartitions(owner, name, subPartitionType, "GLOBAL");
    }
    else
    {
        ret += "LOCAL\n";
        if (partitionType == "RANGE")
            ret += rangePartitions(owner, name, subPartitionType, "LOCAL");
    }
    return ret;
}

static toSQL SQLPartitionTableInfo("toOracleExtract:PartitionTableInfo",
                                   "SELECT\n"
                                   "        DECODE(\n"
                                   "                t.monitoring\n"
                                   "               ,'NO','NOMONITORING'\n"
                                   "               ,     'MONITORING'\n"
                                   "              )                       AS monitoring\n"
                                   "      , t.table_name\n"
                                   "      , LTRIM(t.degree)               AS degree\n"
                                   "      , LTRIM(t.instances)            AS instances\n"
                                   "      , DECODE(\n"
                                   "                t.iot_type\n"
                                   "               ,'IOT','INDEX'\n"
                                   "               ,      'HEAP'\n"
                                   "              )                       AS organization\n"
                                   "      , DECODE(\n"
                                   "                LTRIM(t.cache)\n"
                                   "               ,'Y','CACHE'\n"
                                   "               ,    'NOCACHE'\n"
                                   "              )                       AS cache\n"
                                   "      , p.def_pct_used\n"
                                   "      , p.def_pct_free                AS pct_free\n"
                                   "      , p.def_ini_trans               AS ini_trans\n"
                                   "      , p.def_max_trans               AS max_trans\n"
                                   "      ,DECODE(\n"
                                   "               p.def_initial_extent\n"
                                   "              ,'DEFAULT',s.initial_extent\n"
                                   "              ,p.def_initial_extent * :bs<char[100]> * 1024\n"
                                   "             )                        AS initial_extent\n"
                                   "      ,DECODE(\n"
                                   "               p.def_next_extent\n"
                                   "              ,'DEFAULT',s.next_extent\n"
                                   "              ,p.def_next_extent * :bs<char[100]> * 1024\n"
                                   "             )                        AS next_extent\n"
                                   "      , DECODE(\n"
                                   "                p.def_min_extents\n"
                                   "               ,'DEFAULT',s.min_extents\n"
                                   "               ,p.def_min_extents\n"
                                   "              )                       AS min_extents\n"
                                   "      , DECODE(\n"
                                   "                p.def_max_extents\n"
                                   "               ,'DEFAULT',DECODE(\n"
                                   "                                  s.max_extents\n"
                                   "                                 ,2147483645,'unlimited'\n"
                                   "                                 ,s.max_extents\n"
                                   "                                )\n"
                                   "               ,2147483645,'unlimited'\n"
                                   "               ,p.def_max_extents\n"
                                   "              )                       AS max_extents\n"
                                   "      , DECODE(\n"
                                   "                p.def_pct_increase\n"
                                   "               ,'DEFAULT',s.pct_increase\n"
                                   "               ,p.def_pct_increase\n"
                                   "              )                       AS pct_increase\n"
                                   "      , DECODE(\n"
                                   "                p.def_freelists\n"
                                   "               ,0,1\n"
                                   "               ,NVL(p.def_freelists,1)\n"
                                   "              )                       AS freelists\n"
                                   "      , DECODE(\n"
                                   "                p.def_freelist_groups\n"
                                   "               ,0,1\n"
                                   "               ,NVL(p.def_freelist_groups,1)\n"
                                   "              )                       AS freelist_groups\n"
                                   "      , p.def_buffer_pool             AS buffer_pool\n"
                                   "      , DECODE(\n"
                                   "                p.def_logging \n"
                                   "               ,'NO','NOLOGGING'\n"
                                   "               ,     'LOGGING'\n"
                                   "              )                       AS logging\n"
                                   "      , p.def_tablespace_name         AS tablespace_name\n"
                                   "      , t.blocks - NVL(t.empty_blocks,0)\n"
                                   " FROM\n"
                                   "        sys.all_all_tables   t\n"
                                   "      , sys.all_part_tables  p\n"
                                   "      , sys.dba_tablespaces  s\n"
                                   " WHERE\n"
                                   "            t.table_name      = :nam<char[100]>\n"
                                   "        AND p.table_name      = t.table_name\n"
                                   "        AND s.tablespace_name = p.def_tablespace_name\n"
                                   "        AND t.owner           = :own<char[100]>\n"
                                   "        AND p.owner           = t.owner",
                                   "Get storage information about a partitioned table, "
                                   "same binds and columns",
                                   "0801");

static toSQL SQLPartitionTableInfo8("toOracleExtract:PartitionTableInfo",
                                    "SELECT\n"
                                    "        'N/A'                         AS monitoring\n"
                                    "      , t.table_name\n"
                                    "      , LTRIM(t.degree)               AS degree\n"
                                    "      , LTRIM(t.instances)            AS instances\n"
                                    "      , DECODE(\n"
                                    "                t.iot_type\n"
                                    "               ,'IOT','INDEX'\n"
                                    "               ,      'HEAP'\n"
                                    "              )                       AS organization\n"
                                    "      , DECODE(\n"
                                    "                LTRIM(t.cache)\n"
                                    "               ,'Y','CACHE'\n"
                                    "               ,    'NOCACHE'\n"
                                    "              )                       AS cache\n"
                                    "      , p.def_pct_used\n"
                                    "      , p.def_pct_free                AS pct_free\n"
                                    "      , p.def_ini_trans               AS ini_trans\n"
                                    "      , p.def_max_trans               AS max_trans\n"
                                    "      ,DECODE(\n"
                                    "               p.def_initial_extent\n"
                                    "              ,'DEFAULT',s.initial_extent\n"
                                    "              ,p.def_initial_extent * :bs<char[100]> * 1024\n"
                                    "             )                        AS initial_extent\n"
                                    "      ,DECODE(\n"
                                    "               p.def_next_extent\n"
                                    "              ,'DEFAULT',s.next_extent\n"
                                    "              ,p.def_next_extent * :bs<char[100]> * 1024\n"
                                    "             )                        AS next_extent\n"
                                    "      , DECODE(\n"
                                    "                p.def_min_extents\n"
                                    "               ,'DEFAULT',s.min_extents\n"
                                    "               ,p.def_min_extents\n"
                                    "              )                       AS min_extents\n"
                                    "      , DECODE(\n"
                                    "                p.def_max_extents\n"
                                    "               ,'DEFAULT',DECODE(\n"
                                    "                                  s.max_extents\n"
                                    "                                 ,2147483645,'unlimited'\n"
                                    "                                 ,s.max_extents\n"
                                    "                                )\n"
                                    "               ,2147483645,'unlimited'\n"
                                    "               ,p.def_max_extents\n"
                                    "              )                       AS max_extents\n"
                                    "      , DECODE(\n"
                                    "                p.def_pct_increase\n"
                                    "               ,'DEFAULT',s.pct_increase\n"
                                    "               ,p.def_pct_increase\n"
                                    "              )                       AS pct_increase\n"
                                    "      , DECODE(\n"
                                    "                p.def_freelists\n"
                                    "               ,0,1\n"
                                    "               ,NVL(p.def_freelists,1)\n"
                                    "              )                       AS freelists\n"
                                    "      , DECODE(\n"
                                    "                p.def_freelist_groups\n"
                                    "               ,0,1\n"
                                    "               ,NVL(p.def_freelist_groups,1)\n"
                                    "              )                       AS freelist_groups\n"
                                    "      , 'N/A'                         AS buffer_pool\n"
                                    "      , DECODE(\n"
                                    "                p.def_logging \n"
                                    "               ,'NO','NOLOGGING'\n"
                                    "               ,     'LOGGING'\n"
                                    "              )                       AS logging\n"
                                    "      , p.def_tablespace_name         AS tablespace_name\n"
                                    "      , t.blocks - NVL(t.empty_blocks,0)\n"
                                    " FROM\n"
                                    "        sys.all_all_tables   t\n"
                                    "      , sys.all_part_tables  p\n"
                                    "      , sys.dba_tablespaces  s\n"
                                    " WHERE\n"
                                    "            t.table_name      = :nam<char[100]>\n"
                                    "        AND p.table_name      = t.table_name\n"
                                    "        AND s.tablespace_name = p.def_tablespace_name\n"
                                    "        AND t.owner           = :own<char[100]>\n"
                                    "        AND p.owner           = t.owner",
                                    "",
                                    "0800");

static toSQL SQLPartitionType("toOracleExtract:PartitionType",
                              "SELECT\n"
                              "        partitioning_type\n"
                              "      , partition_count\n"
                              "      , subpartitioning_type\n"
                              "      , def_subpartition_count\n"
                              " FROM\n"
                              "        sys.all_part_tables\n"
                              " WHERE\n"
                              "            table_name = :nam<char[100]>\n"
                              "        AND owner = :own<char[100]>",
                              "Get partition type, must have same binds and columns",
                              "0801");

static toSQL SQLPartitionType8("toOracleExtract:PartitionType",
                               "SELECT\n"
                               "        partitioning_type\n"
                               "      , partition_count\n"
                               "      , 'N/A'                        AS subpartitioning_type\n"
                               "      , 'N/A'                        AS def_subpartition_count\n"
                               " FROM\n"
                               "        sys.all_part_tables\n"
                               " WHERE\n"
                               "            table_name = :nam<char[100]>\n"
                               "        AND owner = :own<char[100]>",
                               "",
                               "0800");

static toSQL SQLPartitionSegment("toOracleExtract:PartitionSegment",
                                 "SELECT\n"
                                 "        partition_name\n"
                                 "      , high_value\n"
                                 "      , 'N/A'\n"
                                 "      , pct_used\n"
                                 "      , pct_free\n"
                                 "      , ini_trans\n"
                                 "      , max_trans\n"
                                 "      , initial_extent\n"
                                 "      , next_extent\n"
                                 "      , min_extent\n"
                                 "      , DECODE(\n"
                                 "                max_extent\n"
                                 "               ,2147483645,'unlimited'\n"
                                 "               ,           max_extent\n"
                                 "              )                       AS max_extents\n"
                                 "      , pct_increase\n"
                                 "      , NVL(freelists,1)\n"
                                 "      , NVL(freelist_groups,1)\n"
                                 "      , buffer_pool\n"
                                 "      , DECODE(\n"
                                 "                logging \n"
                                 "               ,'NO','NOLOGGING'\n"
                                 "               ,     'LOGGING'\n"
                                 "              )                       AS logging\n"
                                 "      , tablespace_name\n"
                                 "      , blocks - NVL(empty_blocks,0)\n"
                                 " FROM\n"
                                 "        sys.all_tab_partitions\n"
                                 " WHERE  table_name = :nam<char[100]>\n"
                                 "   AND  table_owner = :own<char[100]>\n"
                                 " ORDER BY partition_name",
                                 "Information about segment storage for partitioned tables, "
                                 "must have same binds and columns",
                                 "0801");

static toSQL SQLPartitionSegment8("toOracleExtract:PartitionSegment",
                                  "SELECT\n"
                                  "        partition_name\n"
                                  "      , high_value\n"
                                  "      , 'N/A'\n"
                                  "      , pct_used\n"
                                  "      , pct_free\n"
                                  "      , ini_trans\n"
                                  "      , max_trans\n"
                                  "      , initial_extent\n"
                                  "      , next_extent\n"
                                  "      , min_extent\n"
                                  "      , DECODE(\n"
                                  "                max_extent\n"
                                  "               ,2147483645,'unlimited'\n"
                                  "               ,           max_extent\n"
                                  "              )                       AS max_extents\n"
                                  "      , pct_increase\n"
                                  "      , NVL(freelists,1)\n"
                                  "      , NVL(freelist_groups,1)\n"
                                  "      , 'N/A'                         AS buffer_pool\n"
                                  "      , DECODE(\n"
                                  "                logging \n"
                                  "               ,'NO','NOLOGGING'\n"
                                  "               ,     'LOGGING'\n"
                                  "              )                       AS logging\n"
                                  "      , tablespace_name\n"
                                  "      , blocks - NVL(empty_blocks,0)\n"
                                  " FROM\n"
                                  "        sys.all_tab_partitions\n"
                                  " WHERE  table_name = :nam<char[100]>\n"
                                  "   AND  table_owner = :own<char[100]>\n"
                                  " ORDER BY partition_name",
                                  "",
                                  "0800");

static toSQL SQLSubPartitionName("toOracleExtract:SubPartitionName",
                                 "SELECT subpartition_name,\n"
                                 "       tablespace_name\n"
                                 "  FROM sys.all_tab_subpartitions\n"
                                 " WHERE table_name = :nam<char[100]>\n"
                                 "   AND partition_name = :prt<char[100]>\n"
                                 "   AND table_owner = :own<char[100]>\n"
                                 " ORDER BY subpartition_name",
                                 "Get information about sub partitions, "
                                 "must have same columns and binds");

static toSQL SQLPartitionName("toOracleExtract:PartitionName",
                              "SELECT partition_name,\n"
                              "       tablespace_name\n"
                              "  FROM sys.all_tab_partitions\n"
                              " WHERE table_name = :nam<char[100]>\n"
                              "   AND table_owner = :own<char[100]>\n"
                              " ORDER BY partition_name",
                              "Get information about hash partition names, "
                              "must have same columns and binds");

QString toOracleExtract::createPartitionedTable(
        const QString &owner,
        const QString &name)
{
    toQList storage = toQuery::readQuery(connection(), SQLPartitionTableInfo, toQueryParams() << QString::number(ext.getBlockSize()) << name << owner);

    QString organization;
    {
        toQList::iterator i = storage.begin();
        i++;
        i++;
        i++;
        i++;
        organization = (QString)*i;
    }

    QString ret = createTableText(storage, owner, name);

    if (ext.getPartition())
    {
        toQList type = toQuery::readQuery(connection(), SQLPartitionType, toQueryParams() << name << owner);
        QString partitionType ((QString)Utils::toShift(type));
        QString partitionCount ((QString)Utils::toShift(type));
        QString subPartitionType ((QString)Utils::toShift(type));
        QString subPartitionCount((QString)Utils::toShift(type));

        ret += QString("PARTITION BY %1\n(\n    ").arg(partitionType);
        ret += partitionKeyColumns(owner, name, "TABLE");
        ret += "\n)\n";

        if (partitionType == "RANGE")
        {
            if (subPartitionType == "HASH")
            {
                ret += "SUBPARTITIONED BY HASH\n(\n    ";
                ret += subPartitionKeyColumns(owner, name, "TABLE");
                ret += "\n)\nSUBPARTITIONS ";
                ret += subPartitionCount;
                ret += "\n";
            }
            ret += "(\n";

            toQList segment = toQuery::readQuery(connection(), SQLPartitionSegment, toQueryParams() << name << owner);

            QString comma = "    ";
            while (!segment.empty())
            {
                toQList storage;
                QString partition = (QString)Utils::toShift(segment);
                QString highValue = (QString)Utils::toShift(segment);
                Utils::toPush(storage, toQValue("      "));
                Utils::toPush(storage, toQValue(organization));
                for (int i = 0; i < 16; i++)
                    Utils::toPush(storage, Utils::toShift(segment));

                ret += comma;
                ret += QString("PARTITION %1 VALUES LESS THAN\n"
                               "      (\n"
                               "        %2\n"
                               "      )\n").arg(quote(partition)).arg(highValue);
                ret += segmentAttributes(storage);
                comma = "  , ";

                if (subPartitionType == "HASH")
                {
                    toQList subs = toQuery::readQuery(connection(), SQLSubPartitionName, toQueryParams() << name << partition << owner);
                    bool first = true;
                    ret += "        (\n            ";
                    while (!subs.empty())
                    {
                        if (first)
                            first = false;
                        else
                            ret += "\n          , ";
                        ret += QString("SUBPARTITION %2 TABLESPACE %1").
                               arg(quote((QString)Utils::toShift(subs))).
                               arg(quote((QString)Utils::toShift(subs)));
                    }
                    ret += "\n        )\n";
                }
            }
        }
        else
        {
            toQList hash = toQuery::readQuery(connection(), SQLPartitionName, toQueryParams() << name << owner);
            bool first = true;
            ret += "(\n    ";
            while (!hash.empty())
            {
                if (first)
                    first = false;
                else
                    ret += "\n  , ";
                ret += QString("PARTITION %2 TABLESPACE %1").
                       arg(quote((QString)Utils::toShift(hash))).
                       arg(quote((QString)Utils::toShift(hash)));
            }
        }
        ret += ")\n";
    }

    ret += ";\n\n";
    ret += createComments(owner, name);
    return ret;
}

static toSQL SQLRolePrivs("toOracleExtract:RolePrivs",
                          "SELECT\n"
                          "        granted_role\n"
                          "      , DECODE(\n"
                          "                admin_option\n"
                          "               ,'YES','WITH ADMIN OPTION'\n"
                          "               ,null\n"
                          "              )                         AS admin_option\n"
                          "  FROM  sys.dba_role_privs\n"
                          " WHERE  grantee = :nam<char[100]>\n"
                          " ORDER  BY granted_role",
                          "Get roles granted, must have same columns and binds");

static toSQL SQLSystemPrivs("toOracleExtract:SystemPrivs",
                            "SELECT\n"
                            "        privilege\n"
                            "      , DECODE(\n"
                            "                admin_option\n"
                            "               ,'YES','WITH ADMIN OPTION'\n"
                            "               ,null\n"
                            "              )                         AS admin_option\n"
                            "  FROM  sys.dba_sys_privs\n"
                            " WHERE  grantee = :nam<char[100]>\n"
                            " ORDER  BY privilege",
                            "Get system priveleges granted, must have same columns and binds");

static toSQL SQLObjectPrivs("toOracleExtract:ObjectPrivs",
                            "SELECT  privilege\n"
                            "      , table_schema\n"
                            "      , table_name\n"
                            "      , DECODE(\n"
                            "                grantable\n"
                            "               ,'YES','WITH GRANT OPTION'\n"
                            "               ,null\n"
                            "              )                         AS grantable\n"
                            "  FROM  sys.all_tab_privs\n"
                            " WHERE  grantee = :nam<char[100]>\n"
                            " ORDER  BY table_name,privilege",
                            "Get object priveleges granted, must have same columns and binds");

QString toOracleExtract::grantedPrivs(
                                      const QString &dest,
                                      const QString &name,
                                      int typ)
{
    if (!ext.getGrants())
        return "";

    QString ret;
    if ((typ & 1) == 1)
    {
        toQList result = toQuery::readQuery(connection(), SQLSystemPrivs, toQueryParams() << name);
        while (!result.empty())
        {
            QString priv = QString((QString)Utils::toShift(result)).toLower();
            QString sql = QString("GRANT %1 TO %2 %3").
                          arg(priv).
                          arg(dest).
                          arg((QString)Utils::toShift(result));
            if (PROMPT)
                ret += PROMPT_SIMPLE.arg(sql);
            ret += sql;
            ret += ";\n\n\n";
        }
    }

    if ((typ & 2) == 2)
    {
        toQList result = toQuery::readQuery(connection(), SQLRolePrivs, toQueryParams() << name);
        while (!result.empty())
        {
            QString priv = QString((QString)Utils::toShift(result)).toLower();
            QString sql = QString("GRANT %1 TO %2 %3").
                          arg(priv).
                          arg(dest).
                          arg((QString)Utils::toShift(result));
            if (PROMPT)
                ret += PROMPT_SIMPLE.arg(sql);
            ret += sql;
            ret += ";\n\n\n";
        }
    }

    if ((typ & 4) == 4)
    {
        toQList result = toQuery::readQuery(connection(), SQLObjectPrivs, toQueryParams() << name);
        while (!result.empty())
        {
            QString priv = QString(Utils::toShift(result)).toLower();
            QString schema = ext.intSchema((QString)Utils::toShift(result), false);
            QString object = quote((QString)Utils::toShift(result));
            QString sql = QString("GRANT %1 ON %2%3 TO %4 %5").
                          arg(priv.toLower()).
                          arg(schema).
                          arg(object).
                          arg(dest).
                          arg((QString)Utils::toShift(result));
            if (PROMPT)
                ret += PROMPT_SIMPLE.arg(sql);
            ret += sql;
            ret += ";\n\n\n";
        }
    }
    return ret;
}

static toSQL SQLIndexColumns("toOracleExtract:IndexColumns",
                             "SELECT column_name,\n"
                             "       descend\n"
                             "  FROM sys.all_ind_columns\n"
                             " WHERE index_name = :nam<char[100]>\n"
                             "   AND index_owner = :own<char[100]>\n"
                             " ORDER BY column_position",
                             "Get column names from index, same binds and columns",
                             "0801");
static toSQL SQLIndexColumns7("toOracleExtract:IndexColumns",
                              "SELECT column_name,\n"
                              "       'ASC'\n"
                              "  FROM sys.all_ind_columns\n"
                              " WHERE index_name = :nam<char[100]>\n"
                              "   AND index_owner = :own<char[100]>\n"
                              " ORDER BY column_position",
                              "",
                              "0700");

// fix for #2692105: Can't extract DDL for some (probably function-based) indexes
//static toSQL SQLIndexFunction("toOracleExtract:IndexFunction",
//                              "SELECT c.default$\n"
//                              "  FROM sys.col$ c,\n"
//                              "       sys.all_indexes i,\n"
//                              "       sys.all_objects o\n"
//                              " WHERE i.index_name = :ind<char[100]>\n"
//                              "   AND o.object_name = i.table_name\n"
//                              "   AND c.obj# = o.object_id\n"
//                              "   AND c.name = :tab<char[100]>\n"
//                              "   AND i.owner = :own<char[100]>\n"
//                              "   AND o.owner = i.table_owner",
//                              "Get function of index column, same column and binds");
static toSQL SQLIndexFunction("toOracleExtract:IndexFunction",
                              "SELECT c.column_expression\n"
                              "  FROM sys.all_ind_expressions c,\n"
                              "      sys.all_ind_columns i\n"
                              "  WHERE \n"
                              "          c.table_name = i.table_name\n"
                              "      AND c.index_owner = i.index_owner\n"
                              "      AND c.column_position = i.column_position\n"
                              "      AND i.index_name = :ind<char[100]>\n"
                              "      AND i.column_name = :own<char[100]>\n"
                              "      AND i.index_owner = :col<char[100]>\n",
                              "Get function of index column, same column and binds");

QString toOracleExtract::indexColumns(
                                      const QString &indent,
                                      const QString &owner,
                                      const QString &name)
{
    toConnectionSubLoan conn(ext.connection());
    static QString quote_regex("\"");
    static QRegExp func("^sys_nc[0-9]+", Qt::CaseInsensitive);
    toQuery inf(conn, SQLIndexColumns, toQueryParams() << name << owner);
    QString ret = indent;
    ret += "(\n";
    bool first = true;
    while (!inf.eof())
    {
        QString col = (QString)inf.readValue();
        QString asc = (QString)inf.readValue();
        QString row;
        if (func.indexIn(col) >= 0)
        {
            toQuery def(conn, SQLIndexFunction, toQueryParams() << name << col << owner);
            if (!def.eof())
            {
                QString function((QString)def.readValue());
                function.replace(quote_regex, "");
                if (asc == "DESC")
                    row = QString("%1 DESC").arg(function, 30);
                else
                    row = function;
            }
            else if (asc == "DESC")
                row = QString("%1 DESC").arg(quote(col));
            else
                row = quote(col);
        }
        else if (asc == "DESC")
            row = QString("%1 DESC").arg(quote(col));
        else
            row = quote(col);
        ret += indent;
        if (first)
        {
            first = false;
            ret += "    ";
        }
        else
            ret += "  , ";
        ret += row;
        ret += "\n";
    }
    ret += indent;
    ret += ")\n";
    return ret;
}

static toSQL SQLRangePartitions("toOracleExtract:RangePartitions",
                                "SELECT  partition_name\n"
                                "      , high_value\n"
                                "      , 'N/A'                         AS cache\n"
                                "      , 'N/A'                         AS pct_used\n"
                                "      , pct_free\n"
                                "      , ini_trans\n"
                                "      , max_trans\n"
                                "      , initial_extent\n"
                                "      , next_extent\n"
                                "      , min_extent\n"
                                "      , DECODE(\n"
                                "                max_extent\n"
                                "               ,2147483645,'unlimited'\n"
                                "               ,           max_extent\n"
                                "              )                       AS max_extents\n"
                                "      , pct_increase\n"
                                "      , NVL(freelists,1)\n"
                                "      , NVL(freelist_groups,1)\n"
                                "      , buffer_pool\n"
                                "      , DECODE(\n"
                                "                logging \n"
                                "               ,'NO','NOLOGGING'\n"
                                "               ,     'LOGGING'\n"
                                "              )                       AS logging\n"
                                "      , tablespace_name\n"
                                "      , leaf_blocks                   AS blocks\n"
                                " FROM   sys.all_ind_partitions\n"
                                " WHERE      index_name  =  :nam<char[100]>\n"
                                "        AND index_owner =  :own<char[100]>\n"
                                " ORDER BY partition_name",
                                "Get information about partition ranges, must have same binds "
                                "and columns",
                                "0801");

static toSQL SQLRangePartitions8("toOracleExtract:RangePartitions",
                                 "SELECT  partition_name\n"
                                 "      , high_value\n"
                                 "      , 'N/A'                         AS cache\n"
                                 "      , 'N/A'                         AS pct_used\n"
                                 "      , pct_free\n"
                                 "      , ini_trans\n"
                                 "      , max_trans\n"
                                 "      , initial_extent\n"
                                 "      , next_extent\n"
                                 "      , min_extent\n"
                                 "      , DECODE(\n"
                                 "                max_extent\n"
                                 "               ,2147483645,'unlimited'\n"
                                 "               ,           max_extent\n"
                                 "              )                       AS max_extents\n"
                                 "      , pct_increase\n"
                                 "      , NVL(freelists,1)\n"
                                 "      , NVL(freelist_groups,1)\n"
                                 "      , 'N/A'                         AS buffer_pool\n"
                                 "      , DECODE(\n"
                                 "                logging \n"
                                 "               ,'NO','NOLOGGING'\n"
                                 "               ,     'LOGGING'\n"
                                 "              )                       AS logging\n"
                                 "      , tablespace_name\n"
                                 "      , leaf_blocks                   AS blocks\n"
                                 " FROM   sys.all_ind_partitions\n"
                                 " WHERE      index_name  =  :nam<char[100]>\n"
                                 "        AND index_owner =  :own<char[100]>\n"
                                 " ORDER BY partition_name",
                                 "",
                                 "0801");

static toSQL SQLIndexSubPartitionName("toOracleExtract:IndexSubPartitionName",
                                      "SELECT subpartition_name,\n"
                                      "       tablespace_name\n"
                                      "  FROM sys.all_ind_subpartitions\n"
                                      " WHERE index_name     = :ind<char[100]>\n"
                                      "   AND partition_name = :prt<char[100]>\n"
                                      "   AND index_owner    = :own<char[100]>\n"
                                      " ORDER BY subpartition_name",
                                      "Get information about a subpartition, "
                                      "must have same binds and columns");

QString toOracleExtract::rangePartitions(
        const QString &owner,
        const QString &name,
        const QString &subPartitionType,
        const QString &caller)
{
    if (!ext.getPartition())
        return "";

    toQList result = toQuery::readQuery(connection(), SQLRangePartitions, toQueryParams() << name << owner);
    if (result.empty() || result.size() % 18)
        throw qApp->translate("toOracleExtract", "Couldn't find partition range %1.%2").arg(owner).arg(name);

    QString ret;

    QString comma = "    ";
    while (!result.empty())
    {
        QString partition = (QString)Utils::toShift(result);
        QString highValue = (QString)Utils::toShift(result);

        toQList storage;
        Utils::toPush(storage, toQValue("      "));
        Utils::toPush(storage, toQValue("INDEX"));
        for (int i = 0; i < 16; i++)
            Utils::toPush(storage, Utils::toShift(result));

        ret += comma;
        ret += "PARTITION ";
        ret += quote(partition);
        if (caller != "LOCAL")
            ret += QString(" VALUES LESS THAN (%1)\n").arg(highValue);
        else
            ret += "\n";
        ret += segmentAttributes(storage);

        if (subPartitionType == "HASH")
        {
            toConnectionSubLoan conn(ext.connection());
            toQuery inf(conn, SQLIndexSubPartitionName, toQueryParams() << name << partition << owner);
            ret +=
                "        (\n"
                "            ";
            bool first = true;
            while (!inf.eof())
            {
                QString sub(inf.readValue());
                QString tablespace(inf.readValue());
                if (first)
                    first = false;
                else
                    ret += "\n          , ";
                ret += quote(sub);
                ret += " TABLESPACE ";
                ret += quote(tablespace);
            }
            ret += "\n        )\n";
        }
        comma = "  , ";
    }
    ret += ");\n\n";

    return ret;
}

QString toOracleExtract::segmentAttributes(toQList &result)
{
    using namespace ToConfiguration;

    QString ret;
    if (ext.getStorage())
    {
#if 0
        if (result.size() != 18)
            throw qApp->translate("toOracleExtract", "Internal error, result should be 18 in segment attributes (Was %1)").
            arg(result.size());
#endif
        if (result.size() != 18)
            return ret;

        toQList::iterator i = result.begin();

        QString indent = (QString)*i;
        i++;
        QString organization = (QString)*i;
        i++;
        QString cache = (QString)*i;
        i++;
        QString pctUsed = (QString)*i;
        i++;
        QString pctFree = (QString)*i;
        i++;
        QString iniTrans = (QString)*i;
        i++;
        QString maxTrans = (QString)*i;
        i++;
        QString initial = (QString)*i;
        i++;
        QString next = (QString)*i;
        i++;
        QString minExtents = (QString)*i;
        i++;
        QString maxExtents = (QString)*i;
        i++;
        QString pctIncrease = (QString)*i;
        i++;
        QString freelists = (QString)*i;
        i++;
        QString freelistGroups = (QString)*i;
        i++;
        QString bufferPool = (QString)*i;
        i++;
        QString logging = (QString)*i;
        i++;
        QString tablespace = (QString)*i;
        i++;
        QString blocks = (QString)*i;
        i++;

        if (!toConfigurationNewSingle::Instance().option(Oracle::SkipStorageExceptTablespaceBool).toBool())
        {
            if (organization == "HEAP")
            {
                if (cache != "N/A")
                {
                    ret += indent;
                    ret += cache;
                    ret += "\n";
                }
                if (!ext.state("IsASnapIndex").toBool() && !pctUsed.isEmpty())
                    ret += QString("%1PCTUSED             %2\n").arg(indent).arg(pctUsed);
            }
            if (!ext.state("IsASnapIndex").toBool())
                ret += QString("%1PCTFREE             %2\n").arg(indent).arg(pctFree);
            if (!ext.state("IsASnapTable").toBool())
                ret += QString("%1INITRANS            %2\n").arg(indent).arg(iniTrans);

            ret += QString("%1MAXTRANS            %2\n").arg(indent).arg(maxTrans);
            ret += indent;
            ret += "STORAGE\n";
            ret += indent;
            ret += "(\n";
            ret += QString("%1  INITIAL           %2\n").arg(indent).arg(initial);
            if (!next.isEmpty())
                ret += QString("%1  NEXT              %2\n").arg(indent).arg(next);
            ret += QString("%1  MINEXTENTS        %2\n").arg(indent).arg(minExtents);
            ret += QString("%1  MAXEXTENTS        %2\n").arg(indent).arg(maxExtents);
            if (!pctIncrease.isEmpty())
                ret += QString("%1  PCTINCREASE       %2\n").arg(indent).arg(pctIncrease);
            ret += QString("%1  FREELISTS         %2\n").arg(indent).arg(freelists);
            ret += QString("%1  FREELIST GROUPS   %2\n").arg(indent).arg(freelistGroups);
            if (connection().version() >= "0801")
            {
                ret += QString("%1  BUFFER_POOL       %2\n").arg(indent).arg(quote(bufferPool));
            }
            ret += indent;
            ret += ")\n";
            if (connection().version() >= "0800")
                ret += QString("%1%2\n").arg(indent).arg(logging);
        }
        ret += QString("%1TABLESPACE          %2\n").arg(indent).arg(quote(tablespace));
    }
    return ret;
}

static toSQL SQLTableColumns9("toOracleExtract:TableColumns",
                              "SELECT  column_name,\n"
                              "        RPAD(\n"
                              "             DECODE(\n"
                              "                     data_type\n"
                              "                    ,'NUMBER',DECODE(\n"
                              "                                      data_precision\n"
                              "                                     ,null,DECODE(\n"
                              "                                                   data_scale\n"
                              "                                                  ,0,'INTEGER'\n"
                              "                                                  ,  'NUMBER'\n"
                              "                                                 )\n"
                              "                                     ,'NUMBER'\n"
                              "                                    )\n"
                              "                    ,'RAW'     ,'RAW'\n"
                              "                    ,'CHAR'    ,'CHAR'\n"
                              "                    ,'NCHAR'   ,'NCHAR'\n"
                              "                    ,'UROWID'  ,'UROWID'\n"
                              "                    ,'VARCHAR2','VARCHAR2'\n"
                              "                    ,data_type\n"
                              "                   )\n"
                              "               || DECODE(\n"
                              "                          data_type\n"
                              "                         ,'DATE',null\n"
                              "                         ,'LONG',null\n"
                              "                         ,'NUMBER',DECODE(\n"
                              "                                           data_precision\n"
                              "                                          ,null,null\n"
                              "                                          ,'('\n"
                              "                                         )\n"
                              "                         ,'RAW'      ,'('\n"
                              "                         ,'CHAR'     ,'('\n"
                              "                         ,'NCHAR'    ,'('\n"
                              "                         ,'UROWID'   ,'('\n"
                              "                         ,'VARCHAR2' ,'('\n"
                              "                         ,'NVARCHAR2','('\n"
                              "                         ,null\n"
                              "                        )\n"
                              "               || DECODE(\n"
                              "                          data_type\n"
                              "                         ,'RAW'      ,data_length\n"
                              "                         ,'CHAR'     ,data_length\n"
                              "                         ,'NCHAR'    ,char_length\n"
                              "                         ,'UROWID'   ,data_length\n"
                              "                         ,'VARCHAR2' ,data_length\n"
                              "                         ,'NVARCHAR2',char_length\n"
                              "                         ,'NUMBER'   ,data_precision\n"
                              "                         , null\n"
                              "                        )\n"
                              "               || DECODE(\n"
                              "                          data_type\n"
                              "                         ,'NUMBER',DECODE(\n"
                              "                           TO_CHAR(data_precision)\n"
                              "                          ,null,null\n"
                              "                          ,DECODE(\n"
                              "                                   TO_CHAR(data_scale)\n"
                              "                                  ,null,null\n"
                              "                                  ,0   ,null\n"
                              "                                  ,',' || data_scale\n"
                              "                                 )\n"
                              "                              )\n"
                              "                        )\n"
                              "               || DECODE(\n"
                              "                          data_type\n"
                              "                         ,'DATE',null\n"
                              "                         ,'LONG',null\n"
                              "                         ,'NUMBER',DECODE(\n"
                              "                                           data_precision\n"
                              "                                          ,null,null\n"
                              "                                          ,')'\n"
                              "                                         )\n"
                              "                         ,'RAW'      ,')'\n"
                              "                         ,'CHAR'     ,')'\n"
                              "                         ,'NCHAR'    ,')'\n"
                              "                         ,'UROWID'   ,')'\n"
                              "                         ,'VARCHAR2' ,')'\n"
                              "                         ,'NVARCHAR2',')'\n"
                              "                         ,null\n"
                              "                        )\n"
                              "             ,32\n"
                              "            )\n"
                              "     , data_default\n"
                              "     , DECODE(\n"
                              "                nullable\n"
                              "               ,'N','NOT NULL'\n"
                              "               ,     null\n"
                              "              )\n"
                              "  FROM sys.all_tab_columns\n"
                              " WHERE table_name = :nam<char[100]>\n"
                              "   AND owner = :own<char[100]>\n"
                              " ORDER BY column_id",
                              "Extract column definitions from table",
                              "0900");

static toSQL SQLTableColumns("toOracleExtract:TableColumns",
                             "SELECT  column_name,\n"
                             "        RPAD(\n"
                             "             DECODE(\n"
                             "                     data_type\n"
                             "                    ,'NUMBER',DECODE(\n"
                             "                                      data_precision\n"
                             "                                     ,null,DECODE(\n"
                             "                                                   data_scale\n"
                             "                                                  ,0,'INTEGER'\n"
                             "                                                  ,  'NUMBER'\n"
                             "                                                 )\n"
                             "                                     ,'NUMBER'\n"
                             "                                    )\n"
                             "                    ,'RAW'     ,'RAW'\n"
                             "                    ,'CHAR'    ,'CHAR'\n"
                             "                    ,'NCHAR'   ,'NCHAR'\n"
                             "                    ,'UROWID'  ,'UROWID'\n"
                             "                    ,'VARCHAR2','VARCHAR2'\n"
                             "                    ,data_type\n"
                             "                   )\n"
                             "               || DECODE(\n"
                             "                          data_type\n"
                             "                         ,'DATE',null\n"
                             "                         ,'LONG',null\n"
                             "                         ,'NUMBER',DECODE(\n"
                             "                                           data_precision\n"
                             "                                          ,null,null\n"
                             "                                          ,'('\n"
                             "                                         )\n"
                             "                         ,'RAW'      ,'('\n"
                             "                         ,'CHAR'     ,'('\n"
                             "                         ,'NCHAR'    ,'('\n"
                             "                         ,'UROWID'   ,'('\n"
                             "                         ,'VARCHAR2' ,'('\n"
                             "                         ,'NVARCHAR2','('\n"
                             "                         ,null\n"
                             "                        )\n"
                             "               || DECODE(\n"
                             "                          data_type\n"
                             "                         ,'RAW'      ,data_length\n"
                             "                         ,'CHAR'     ,data_length\n"
                             "                         ,'NCHAR'    ,data_length\n"
                             "                         ,'UROWID'   ,data_length\n"
                             "                         ,'VARCHAR2' ,data_length\n"
                             "                         ,'NVARCHAR2',data_length\n"
                             "                         ,'NUMBER'   ,data_precision\n"
                             "                         , null\n"
                             "                        )\n"
                             "               || DECODE(\n"
                             "                          data_type\n"
                             "                         ,'NUMBER',DECODE(\n"
                             "                           TO_CHAR(data_precision)\n"
                             "                          ,null,null\n"
                             "                          ,DECODE(\n"
                             "                                   TO_CHAR(data_scale)\n"
                             "                                  ,null,null\n"
                             "                                  ,0   ,null\n"
                             "                                  ,',' || data_scale\n"
                             "                                 )\n"
                             "                              )\n"
                             "                        )\n"
                             "               || DECODE(\n"
                             "                          data_type\n"
                             "                         ,'DATE',null\n"
                             "                         ,'LONG',null\n"
                             "                         ,'NUMBER',DECODE(\n"
                             "                                           data_precision\n"
                             "                                          ,null,null\n"
                             "                                          ,')'\n"
                             "                                         )\n"
                             "                         ,'RAW'      ,')'\n"
                             "                         ,'CHAR'     ,')'\n"
                             "                         ,'NCHAR'    ,')'\n"
                             "                         ,'UROWID'   ,')'\n"
                             "                         ,'VARCHAR2' ,')'\n"
                             "                         ,'NVARCHAR2',')'\n"
                             "                         ,null\n"
                             "                        )\n"
                             "             ,32\n"
                             "            )\n"
                             "     , data_default\n"
                             "     , DECODE(\n"
                             "                nullable\n"
                             "               ,'N','NOT NULL'\n"
                             "               ,     null\n"
                             "              )\n"
                             "  FROM sys.all_tab_columns\n"
                             " WHERE table_name = :nam<char[100]>\n"
                             "   AND owner = :own<char[100]>\n"
                             " ORDER BY column_id",
                             "",
                             "0800");

static toSQL SQLTableColumns7("toOracleExtract:TableColumns",
                              "       SELECT\n"
                              "        column_name,\n"
                              "        RPAD(\n"
                              "             DECODE(\n"
                              "                     data_type\n"
                              "                    ,'NUMBER',DECODE(\n"
                              "                                      data_precision\n"
                              "                                     ,null,DECODE(\n"
                              "                                                   data_scale\n"
                              "                                                  ,0,'INTEGER'\n"
                              "                                                  ,  'NUMBER'\n"
                              "                                                 )\n"
                              "                                     ,'NUMBER'\n"
                              "                                    )\n"
                              "                    ,'RAW'     ,'RAW'\n"
                              "                    ,'CHAR'    ,'CHAR'\n"
                              "                    ,'UROWID'  ,'UROWID'\n"
                              "                    ,'VARCHAR2','VARCHAR2'\n"
                              "                    ,data_type\n"
                              "                   )\n"
                              "               || DECODE(\n"
                              "                          data_type\n"
                              "                         ,'DATE',null\n"
                              "                         ,'LONG',null\n"
                              "                         ,'NUMBER',DECODE(\n"
                              "                                           data_precision\n"
                              "                                          ,null,null\n"
                              "                                          ,'('\n"
                              "                                         )\n"
                              "                         ,'RAW'      ,'('\n"
                              "                         ,'CHAR'     ,'('\n"
                              "                         ,'UROWID'   ,'('\n"
                              "                         ,'VARCHAR2' ,'('\n"
                              "                         ,null\n"
                              "                        )\n"
                              "               || DECODE(\n"
                              "                          data_type\n"
                              "                         ,'RAW'      ,data_length\n"
                              "                         ,'CHAR'     ,data_length\n"
                              "                         ,'UROWID'   ,data_length\n"
                              "                         ,'VARCHAR2' ,data_length\n"
                              "                         ,'NUMBER'   ,data_precision\n"
                              "                         , null\n"
                              "                        )\n"
                              "               || DECODE(\n"
                              "                          data_type\n"
                              "                         ,'NUMBER',DECODE(\n"
                              "                                          data_precision\n"
                              "                                          ,null,null\n"
                              "                                          ,DECODE(\n"
                              "                                                  data_scale\n"
                              "                                                 ,null,null\n"
                              "                                                 ,0   ,null\n"
                              "                                                 ,',' || data_scale\n"
                              "                                                 )\n"
                              "                                         )\n"
                              "                        )\n"
                              "               || DECODE(\n"
                              "                          data_type\n"
                              "                         ,'DATE',null\n"
                              "                         ,'LONG',null\n"
                              "                         ,'NUMBER',DECODE(\n"
                              "                                           data_precision\n"
                              "                                          ,null,null\n"
                              "                                          ,')'\n"
                              "                                         )\n"
                              "                         ,'RAW'      ,')'\n"
                              "                         ,'CHAR'     ,')'\n"
                              "                         ,'UROWID'   ,')'\n"
                              "                         ,'VARCHAR2' ,')'\n"
                              "                         ,'NVARCHAR2',')'\n"
                              "                         ,null\n"
                              "                        )\n"
                              "             ,33\n"
                              "            )\n"
                              "     , data_default\n"
                              "     , DECODE(\n"
                              "                nullable\n"
                              "               ,'N','NOT NULL'\n"
                              "               ,     null\n"
                              "              )\n"
                              "  FROM sys.all_tab_columns\n"
                              " WHERE table_name = :nam<char[100]>\n"
                              "   AND owner = :own<char[100]>\n"
                              " ORDER BY column_id",
                              "",
                              "0700");

QString toOracleExtract::tableColumns(
                                      const QString &owner,
                                      const QString &name)
{
    toQList cols = toQuery::readQuery(connection(), SQLTableColumns, toQueryParams() << name << owner);
    bool first = true;
    QString ret;
    while (!cols.empty())
    {
        if (first)
            first = false;
        else
            ret += "\n  , ";
        ret += QString(quote((QString)Utils::toShift(cols))).leftJustified(32);
        ret += " ";
        ret += (QString)Utils::toShift(cols);
        QString def = (QString)Utils::toShift(cols);
        QString notNull = (QString)Utils::toShift(cols);
        if (!def.isEmpty())
        {
            ret += " DEFAULT ";
            ret += def.trimmed();
        }
        if (!notNull.isEmpty())
            ret += " " + notNull;
    }
    ret += "\n";
    return ret;
}

// Implementation describe utility functions

void toOracleExtract::describeAttributes(
        std::list<QString> &dsp,
        std::list<QString> &ctx,
        toQList &result)
{
    if (!ext.getStorage())
        return ;

#if 0
    if (result.size() != 18)
        throw qApp->translate("toOracleExtract", "Internal error, result should be 18 in segment attributes");
#endif
    if (result.size() != 18)
        return;

    toQList::iterator i = result.begin();

    QString indent = (QString)*i;
    i++;
    QString organization = (QString)*i;
    i++;
    QString cache = (QString)*i;
    i++;
    QString pctUsed = (QString)*i;
    i++;
    QString pctFree = (QString)*i;
    i++;
    QString iniTrans = (QString)*i;
    i++;
    QString maxTrans = (QString)*i;
    i++;
    QString initial = (QString)*i;
    i++;
    QString next = (QString)*i;
    i++;
    QString minExtents = (QString)*i;
    i++;
    QString maxExtents = (QString)*i;
    i++;
    QString pctIncrease = (QString)*i;
    i++;
    QString freelists = (QString)*i;
    i++;
    QString freelistGroups = (QString)*i;
    i++;
    QString bufferPool = (QString)*i;
    i++;
    QString logging = (QString)*i;
    i++;
    QString tablespace = (QString)*i;
    i++;
    QString blocks = (QString)*i;
    i++;

    QString ret;
    if (organization == "HEAP")
    {
        if (cache != "N/A")
            addDescription(dsp, ctx, "PARAMETERS", cache);
        if (!ext.state("IsASnapIndex").toBool() && !pctUsed.isEmpty())
            addDescription(dsp, ctx, "PARAMETERS", QString("PCTUSED %1").arg(pctUsed));
    }
    if (!ext.state("IsASnapIndex").toBool())
        addDescription(dsp, ctx, "PARAMETERS", QString("PCTFREE %1").arg(pctFree));
    if (!ext.state("IsASnapTable").toBool())
        addDescription(dsp, ctx, "PARAMETERS", QString("INITRANS %1").arg(iniTrans));

    addDescription(dsp, ctx, "PARAMETERS", QString("MAXTRANS %1").arg(maxTrans));
    addDescription(dsp, ctx, "STORAGE", QString("INITIAL %1").arg(initial));
    if (!next.isEmpty())
        addDescription(dsp, ctx, "STORAGE", QString("NEXT %1").arg(next));
    addDescription(dsp, ctx, "STORAGE", QString("MINEXTENTS %1").arg(minExtents));
    addDescription(dsp, ctx, "STORAGE", QString("MAXEXTENTS %1").arg(maxExtents));
    addDescription(dsp, ctx, "STORAGE", QString("PCTINCREASE %1").arg(pctIncrease));
    addDescription(dsp, ctx, "STORAGE", QString("FREELISTS %1").arg(freelists));
    addDescription(dsp, ctx, "STORAGE", QString("FREELIST GROUPS %1").arg(freelistGroups));
    if (connection().version() >= "0801")
    {
        addDescription(dsp, ctx, "STORAGE", QString("BUFFER_POOL %1").arg(quote(bufferPool)));
    }
    if (connection().version() >= "0800")
        addDescription(dsp, ctx, "PARAMETERS", logging);
    addDescription(dsp, ctx, "PARAMETERS", QString("TABLESPACE %1").arg(quote(tablespace)));
}

void toOracleExtract::describeComments(
                                       std::list<QString> &lst,
                                       std::list<QString> &ctx,
                                       const QString &owner,
                                       const QString &name)
{
    if (ext.getComments())
    {
        toConnectionSubLoan conn(connection());
        toQuery inf(conn, SQLTableComments, toQueryParams() << name << owner);
        while (!inf.eof())
        {
            addDescription(lst, ctx, "COMMENT", (QString)inf.readValue());
        }
        toQuery col(conn, SQLColumnComments, toQueryParams() << name << owner);
        while (!col.eof())
        {
            QString column = (QString)col.readValue();
            addDescription(lst, ctx, "COLUMN", quote(column), "COMMENT", (QString)col.readValue());
        }
    }
}

void toOracleExtract::describePrivs(std::list<QString> &lst, std::list<QString> &ctx, const QString &name)
{
    if (!ext.getGrants())
        return ;

    toQList result = toQuery::readQuery(connection(), SQLRolePrivs, toQueryParams() << name);
    while (!result.empty())
    {
        QString role = quote((QString)Utils::toShift(result));
        addDescription(lst, ctx, "GRANT", "ROLE " + role, (QString)Utils::toShift(result));
    }

    result = toQuery::readQuery(connection(), SQLSystemPrivs, toQueryParams() << name);
    while (!result.empty())
    {
        QString priv = QString(Utils::toShift(result)).toLower();
        addDescription(lst, ctx, "GRANT", priv, (QString)Utils::toShift(result));
    }

    result = toQuery::readQuery(connection(), SQLObjectPrivs, toQueryParams() << name);
    while (!result.empty())
    {
        QString priv = (QString)Utils::toShift(result);
        QString schema = ext.intSchema((QString)Utils::toShift(result), false);
        QString res = schema;
        res += quote((QString)Utils::toShift(result));
        addDescription(lst, ctx, "GRANT", priv.toLower(), "ON", res, (QString)Utils::toShift(result));
    }
}

void toOracleExtract::describeIOT(
                                  std::list<QString> &lst,
                                  std::list<QString> &ctx,
                                  const QString &owner,
                                  const QString &name)
{
    toQList storage = toQuery::readQuery(connection(), SQLIOTInfo, toQueryParams() << name << owner);

    describeTableText(lst, ctx, storage, owner, name);
    describeComments(lst, ctx, owner, name);
    if (ext.getStorage())
    {
        toQList overflow = toQuery::readQuery(connection(), segments(SQLOverflowInfo), toQueryParams() << name << owner);
        if (overflow.size() == 18)
        {
            ctx.insert(ctx.end(), "OVERFLOW");
            describeAttributes(lst, ctx, overflow);
        }
    }
}

void toOracleExtract::describeIndexColumns(
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &owner,
        const QString &name)
{
    static QRegularExpression quote_regex("\"");
    static QRegExp func("^sys_nc[0-9]g");
    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, SQLIndexColumns, toQueryParams() << name << owner);
    int num = 1;
    while (!inf.eof())
    {
        QString col = (QString)inf.readValue();
        QString asc = (QString)inf.readValue();
        QString row;
        if (func.indexIn(col) >= 0)
        {
            toConnectionSubLoan conn2(ext.connection());
            toQuery def(conn2, SQLIndexFunction, toQueryParams() << col << name << owner);
            QString function((QString)def.readValue());
            inf.readValue(); // we read function index from def, but inf has to be shifted too
            function.replace(quote_regex, "");
            if (asc == "DESC")
                row = QString("%1 DESC").arg(function, 30);
            else
                row = function;
        }
        else if (asc == "DESC")
            row = QString("%1 DESC").arg(quote(col));
        else
            row = quote(col);
        addDescription(lst, ctx, "COLUMN", row);
        addDescription(lst, ctx, "COLUMN", row, "ORDER", QString::number(num));
        num++;
    }
}

#if 0
void toOracleExtract::describeMView(const QString &owner, const QString &name, const QString &type, std::list<QString> &lst)
{
    toQList result = toQuery::readQuery(connection(), SQLMViewInfo, toQueryParams() << name << owner);
    if (result.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find materialised table %1.%2").
        arg(quote(owner)).arg(quote(name));
    QString table = (QString)Utils::toShift(result);
    QString buildMode = (QString)Utils::toShift(result);
    QString refreshMethod = (QString)Utils::toShift(result);
    QString refreshMode = (QString)Utils::toShift(result);
    QString startWith = (QString)Utils::toShift(result);
    QString next = (QString)Utils::toShift(result);
    QString usingPK = (QString)Utils::toShift(result);
    QString masterRBSeg = (QString)Utils::toShift(result);
    QString updatable = (QString)Utils::toShift(result);
    QString query = (QString)Utils::toShift(result);

    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, SQLIndexName, toQueryParams() << table << owner);
    QString index(inf.readValue());

    std::list<QString> ctx;
    ctx.insert(ctx.end(), quote(owner));
    ctx.insert(ctx.end(), type);
    ctx.insert(ctx.end(), quote(name));

    describeMViewTable(lst, ctx, owner, table);
    addDescription(lst, ctx, buildMode);
    ctx.insert(ctx.end(), "INDEX");
    ctx.insert(ctx.end(), quote(index));
    describeMViewIndex(lst, ctx, owner, index);

    if (refreshMethod != "NEVER REFRESH")
    {
        addDescription(lst, ctx, QString("START WITH %1").arg(startWith));
        if (!next.isEmpty())
            addDescription(lst, ctx, QString("NEXT %1").arg(next));
        addDescription(lst, ctx, usingPK);
        if (!masterRBSeg.isEmpty() && ext.getStorage())
            addDescription(lst, ctx, QString("USING MASTER ROLLBACK SEGMENT %1").
                           arg(quote(masterRBSeg)));
    }

    if (!updatable.isEmpty())
        addDescription(lst, ctx, updatable);
    addDescription(lst, ctx, QString("AS %1").arg(query));
}
#endif

static QString ReContext(std::list<QString> &ctx, int strip, const QString &str)
{
    QStringList lst = str.split("\01");
    QString ret;
    QString sep = "";
    for (std::list<QString>::iterator i = ctx.begin(); i != ctx.end(); i++)
    {
        ret += sep;
        ret += *i;
        if (sep.isEmpty())
            sep = "\01";
    }
    for (QStringList::Iterator j = lst.begin(); j != lst.end(); j++)
    {
        if (strip > 0)
        {
            ret += sep;
            ret += *j;
        }
        else
            strip--;
        if (sep.isEmpty())
            sep = "\01";
    }
    return ret;
}

#if 0
void toOracleExtract::describeMViewIndex(
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &schema,
        const QString &owner,
        const QString &name)
{
    if (!ext.getStorage() || !ext.getParallel())
        return ;
    ext.setState("IsASnapIndex", true);

    static QRegExp start("^[^\001]+[\001][^\001]+[\001][^\001]+[\001]INITTRANS");
    static QRegExp ignore("^[^\001]+[\001][^\001]+[\001][^\001]+[\001]LOGGING");

    bool started = false;
    bool done = false;

    std::list<QString> tbllst;
    describeIndex(owner, name, tbllst);
    QString ret;

    for (std::list<QString>::iterator i = tbllst.begin(); i != tbllst.end() && !done; i++)
    {
        if (start.indexIn(*i) >= 0)
            started = true;
        if (started)
            lst.insert(lst.end(), ReContext(ctx, 3, *i));
    }

    ext.setState("IsASnapIndex", false);
}

void toOracleExtract::describeMViewLog(
                                       std::list<QString> &lst,
                                       const QString &schema,
                                       const QString &owner,
                                       const QString &name,
                                       const QString &type)
{
    toQList result = toQuery::readQuery(connection(), SQLSnapshotInfo, toQueryParams() << name << owner);
    if (result.size() != 4)
        throw qApp->translate("toOracleExtract", "Couldn't find log %1.%2").arg(owner).arg(name);

    QString table = (QString)Utils::toShift(result);
    QString rowIds = (QString)Utils::toShift(result);
    QString primaryKey = (QString)Utils::toShift(result);
    QString filterColumns = (QString)Utils::toShift(result);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), type);
    ctx.insert(ctx.end(), quote(name));

    describeMViewTable(lst, ctx, schema, owner, table);

    if (rowIds == "YES" && primaryKey == "YES")
        ctx.insert(ctx.end(), "WITH PRIMARY KEY, ROWID");
    else if (rowIds == "YES")
        ctx.insert(ctx.end(), "WITH ROWID");
    else if (primaryKey == "YES")
        ctx.insert(ctx.end(), "WITH PRIMARY KEY");
    addDescription(lst, ctx);

    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, SQLSnapshotColumns, toQueryParams() << name << owner);
    bool first = true;
    if (!inf.eof())
    {
        QString col = "(";
        while (!inf.eof())
        {
            if (first)
                first = false;
            else
                col += ",";
            col += quote((QString)inf.readValue());
        }
        col += ")";
        addDescription(lst, ctx, col);
    }
}

void toOracleExtract::describeMViewTable(
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &schema,
        const QString &owner,
        const QString &name)
{
    if (!ext.getStorage() || !ext.getParallel())
        return ;
    ext.setState("IsASnapTable", true);
    //                        Schema        Table         Name
    static QRegExp parallel("^[^\001]+[\001][^\001]+[\001][^\001]+[\001]PARALLEL");

    bool started = false;
    bool done = false;

    std::list<QString> tbllst;
    describeTable(owner, name, tbllst);
    QString ret;

    for (std::list<QString>::iterator i = tbllst.begin(); i != tbllst.end() && !done; i++)
    {
        if (parallel.indexIn(*i) >= 0)
            started = true;
        if (started)
            lst.insert(lst.end(), ReContext(ctx, 3, *i));
    }

    ext.setState("IsASnapTable", false);
}
#endif

void toOracleExtract::describePartitions(
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &owner,
        const QString &name,
        const QString &subPartitionType,
        const QString &caller)
{
    if (!ext.getPartition())
        return ;

    toQList result = toQuery::readQuery(connection(), SQLRangePartitions, toQueryParams() << name << owner);
    if (result.empty() || result.size() % 18)
        throw qApp->translate("toOracleExtract", "Couldn't find partition range %1.%2").arg(owner).arg(name);

    while (!result.empty())
    {
        QString partition = (QString)Utils::toShift(result);
        QString highValue = (QString)Utils::toShift(result);

        toQList storage;
        Utils::toPush(storage, toQValue("      "));
        Utils::toPush(storage, toQValue("INDEX"));
        for (int i = 0; i < 16; i++)
            Utils::toPush(storage, Utils::toShift(result));

        std::list<QString> cctx = ctx;
        cctx.insert(cctx.end(), quote(partition));

        addDescription(lst, cctx);
        if (caller != "LOCAL")
            addDescription(lst, cctx, "RANGE", QString("VALUES LESS THAN (%1)").arg(highValue));
        describeAttributes(lst, cctx, storage);

        if (subPartitionType == "HASH")
        {
            toConnectionSubLoan conn(ext.connection());
            toQuery inf(conn, SQLIndexSubPartitionName, toQueryParams() << name << partition << owner);
            while (!inf.eof())
            {
                QString sub((QString)inf.readValue());
                QString tablespace((QString)inf.readValue());
                addDescription(lst, cctx, "HASH", QString("%1 TABLESPACE %2").arg(quote(sub)).arg(quote(tablespace)));
            }
        }
    }
}

void toOracleExtract::describePartitionedIOT(
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &owner,
        const QString &name)
{
    toQList result = toQuery::readQuery(connection(), SQLPartitionedIOTInfo, toQueryParams() << QString::number(ext.getBlockSize()) << name << owner);
    describeTableText(lst, ctx, result, owner, name);
    if (ext.getPartition())
    {
        toConnectionSubLoan conn(ext.connection());
        toQuery inf(conn, SQLPartitionIndexNames, toQueryParams() << name << owner);
        if (!inf.eof())
            throw qApp->translate("toOracleExtract", "Couldn't find index partitions for %1.%2").arg(owner).arg(name);
        QString index(inf.readValue());
        std::list<QString> cctx = ctx;
        Utils::toPush(cctx, QString("PARTITION COLUMNS"));
        Utils::toPush(cctx, partitionKeyColumns(owner, name, "TABLE"));
        addDescription(lst, cctx);
        describePartitions(lst, cctx, owner, index, "NONE", "IOT");
    }
    describeComments(lst, ctx, owner, name);
    if (ext.getStorage())
    {
        toQList overflow = toQuery::readQuery(connection(), segments(SQLOverflowInfo), toQueryParams() << name << owner);
        if (overflow.size() == 18)
        {
            ctx.insert(ctx.end(), "OVERFLOW");
            describeAttributes(lst, ctx, overflow);
        }
    }
}

void toOracleExtract::describePartitionedIndex(
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &owner,
        const QString &name)
{
    if (!ext.getPartition())
        return ;

    toQList result = toQuery::readQuery(ext.connection() , SQLIndexPartition, toQueryParams() << QString::number(ext.getBlockSize()) << owner << name);
    QString partitionType = (QString)Utils::toShift(result);
    QString subPartitionType = (QString)Utils::toShift(result);
    QString locality = (QString)Utils::toShift(result);
    QString compressed = (QString)Utils::toShift(result);
    Utils::toUnShift(result, toQValue(""));

    describeAttributes(lst, ctx, result);

    if (!compressed.isEmpty())
        addDescription(lst, ctx, "COMPRESS", compressed);

    if (locality == "GLOBAL")
    {
        std::list<QString> cctx = ctx;
        Utils::toPush(cctx, QString("GLOBAL PARTITION COLUMNS"));
        Utils::toPush(cctx, partitionKeyColumns(owner, name, "INDEX"));

        addDescription(lst, cctx);
        describePartitions(lst, cctx, owner, name, subPartitionType, "GLOBAL");
    }
    else
    {
        std::list<QString> cctx = ctx;
        Utils::toPush(cctx, QString("LOCAL PARTITION"));
        addDescription(lst, cctx);
        if (partitionType == "RANGE")
            describePartitions(lst, cctx, owner, name, subPartitionType, "LOCAL");
    }
}

void toOracleExtract::describePartitionedTable(
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &owner,
        const QString &name)
{
    toQList storage = toQuery::readQuery(connection(), SQLPartitionTableInfo,
                                         toQueryParams() << QString::number(ext.getBlockSize()) << name << owner);

    QString organization;
    {
        toQList::iterator i = storage.begin();
        i++;
        i++;
        i++;
        i++;
        organization = (QString)*i;
    }

    describeTableText(lst, ctx, storage, owner, name);

    if (ext.getPartition())
    {
        toQList type = toQuery::readQuery(connection(), SQLPartitionType, toQueryParams() << name << owner);
        QString partitionType ((QString)Utils::toShift(type));
        QString partitionCount ((QString)Utils::toShift(type));
        QString subPartitionType ((QString)Utils::toShift(type));
        QString subPartitionCount((QString)Utils::toShift(type));

        QString prtstr = QString("PARTITION BY %1 (%2)").
                         arg(partitionType).
                         arg(partitionKeyColumns(owner, name, "TABLE"));
        addDescription(lst, ctx, prtstr);
        QString subprtstr;

        if (partitionType == "RANGE")
        {
            if (subPartitionType == "HASH")
            {
                subprtstr = QString("SUBPARTITIONED BY HASH (%1) SUBPARTITIONS %2").
                            arg(subPartitionKeyColumns(owner, name, "TABLE")).
                            arg(subPartitionCount);
                addDescription(lst, ctx, subprtstr);
            }

            toQList segment = toQuery::readQuery(connection(), SQLPartitionSegment, toQueryParams() << name << owner);

            while (!segment.empty())
            {
                toQList storage;
                QString partition = (QString)Utils::toShift(segment);
                QString highValue = (QString)Utils::toShift(segment);
                Utils::toPush(storage, toQValue("      "));
                Utils::toPush(storage, toQValue(organization));
                for (int i = 0; i < 16; i++)
                    Utils::toPush(storage, Utils::toShift(segment));

                std::list<QString> cctx = ctx;
                cctx.insert(cctx.end(), prtstr);
                cctx.insert(cctx.end(), quote(partition));
                addDescription(lst, cctx, "RANGE", QString("VALUES LESS THAN %2").
                               arg(highValue));
                describeAttributes(lst, cctx, storage);

                if (subPartitionType == "HASH")
                {
                    toQList subs = toQuery::readQuery(connection(), SQLSubPartitionName,
                                                      toQueryParams() << name << partition << owner);
                    while (!subs.empty())
                    {
                        QString subpart = quote((QString)Utils::toShift(subs));
                        QString tabspac = (QString)Utils::toShift(subs);
                        addDescription(lst, cctx, subprtstr, subpart);
                        addDescription(lst, cctx, subprtstr, subpart,
                                       QString("TABLESPACE %1").arg(quote(tabspac)));
                    }
                }
            }
        }
        else
        {
            toQList hash = toQuery::readQuery(connection(), SQLPartitionName,
                                              toQueryParams() << name << owner);
            while (!hash.empty())
            {
                QString partition = quote((QString)Utils::toShift(hash));
                QString tablespac = (QString)Utils::toShift(hash);
                addDescription(lst, ctx, prtstr, partition);
                addDescription(lst, ctx, prtstr, partition,
                               QString("TABLESPACE %1").arg(quote(tablespac)));
            }
        }
    }

    describeComments(lst, ctx, owner, name);
}

void toOracleExtract::describeSource(
                                     std::list<QString> &lst,
                                     const QString &owner,
                                     const QString &name,
                                     const QString &type)
{
    if (!ext.getCode())
        return ;

    std::list<QString> ctx;
    ctx.insert(ctx.end(), quote(owner));
    ctx.insert(ctx.end(), type);
    ctx.insert(ctx.end(), quote(name));
    addDescription(lst, ctx);
    addDescription(lst, ctx, displaySource(owner, name, type, true));
}

void toOracleExtract::describeTableColumns(
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &owner,
        const QString &name)
{
    toQList cols = toQuery::readQuery(connection(), SQLTableColumns, toQueryParams() << name << owner);
    int num = 1;
    while (!cols.empty())
    {
        QString col = quote((QString)Utils::toShift(cols));
        QString line = (QString)Utils::toShift(cols);
        QString extra = (QString)Utils::toShift(cols);
        extra = extra.trimmed();
        // In Oracle (at least up to 11R2 if default value was set on column you cannot totally
        // remove it from data dictionary. Default value is reported as being null rather than
        // being empty/not set. Therefore this comparison to "null" is added.
        if (!extra.isEmpty() && (extra.toLower() != "null"))
        {
            line += " DEFAULT ";
            line += extra.trimmed(); // extra could have trailing spaces
        }
        extra = (QString)Utils::toShift(cols);  // TS 2009-11-15 changed += to = as default value is already
        //               added as "line" so not required as "extra"
        addDescription(lst, ctx, "COLUMN", col);
        addDescription(lst, ctx, "COLUMN", col, line);
        if (!extra.isEmpty())
            addDescription(lst, ctx, "COLUMN", col, "EXTRA", extra);
        addDescription(lst, ctx, "COLUMN", col, "ORDER", QString::number(num));
        num++;
    }
}

void toOracleExtract::describeTableText(
                                        std::list<QString> &lst,
                                        std::list<QString> &ctx,
                                        toQList &result,
                                        const QString &owner,
                                        const QString &name)
{
    QString monitoring = (QString)Utils::toShift(result);
    QString table = (QString)Utils::toShift(result);
    QString degree = (QString)Utils::toShift(result);
    QString instances = (QString)Utils::toShift(result);
    QString organization = (QString)Utils::toShift(result);

    QString ret;
    addDescription(lst, ctx);
    describeTableColumns(lst, ctx, owner, name);
    if (connection().version() >= "0800" && ext.getStorage() && !organization.isEmpty() )
        addDescription(lst, ctx, "PARAMETERS", QString("ORGANIZATION %1").arg(organization));
    if (connection().version() >= "0801" && ext.getStorage() && ! monitoring.isEmpty()  )
        addDescription(lst, ctx, "PARAMETERS", monitoring);
    if (ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty() )
    {
        addDescription(lst, ctx, "PARALLEL", QString("DEGREE %1").arg(degree));
        addDescription(lst, ctx, "PARALLEL", QString("INSTANCES %1").arg(instances));
    }

    Utils::toUnShift(result, toQValue(organization));
    Utils::toUnShift(result, toQValue(""));

    describeAttributes(lst, ctx, result);
}

// Implementation create functions

static toSQL SQLListConstraint("toOracleExtract:ListConstraint",
                               "SELECT table_name,\n"
                               "       constraint_type,\n"
                               "       search_condition,\n"
                               "       r_owner,\n"
                               "       r_constraint_name,\n"
                               "       delete_rule,\n"
                               "       DECODE(status,'ENABLED','ENABLE NOVALIDATE','DISABLE'),\n"
                               "       deferrable,\n"
                               "       deferred\n"
                               "  FROM sys.all_constraints\n"
                               " WHERE owner = :own<char[100]>\n"
                               "   AND constraint_name = :nam<char[100]>",
                               "Get information about a constraint, same binds and columns",
                               "0800");

static toSQL SQLListConstraint7("toOracleExtract:ListConstraint",
                                "SELECT table_name,\n"
                                "       constraint_type,\n"
                                "       search_condition,\n"
                                "       r_owner,\n"
                                "       r_constraint_name,\n"
                                "       delete_rule,\n"
                                "       DECODE(status,'ENABLED','ENABLE','DISABLE'),\n"
                                "       NULL,\n"
                                "       NULL\n"
                                "  FROM sys.all_constraints\n"
                                " WHERE owner = :own<char[100]>\n"
                                "   AND constraint_name = :nam<char[100]>",
                                "",
                                "0700");

static toSQL SQLConstraintTable("toOracleExtract:ConstraintTable",
                                "SELECT table_name\n"
                                "  FROM sys.all_constraints\n"
                                " WHERE owner = :own<char[100]>\n"
                                "   AND constraint_name = :nam<char[100]>",
                                "Get tablename from constraint name, same binds and columns");

QString toOracleExtract::createConstraint(
        const QString &owner,
        const QString &name)
{
    QString ret;

    if (ext.getConstraints())
    {
        toConnectionSubLoan conn(ext.connection());
        toQuery inf(conn, SQLListConstraint, toQueryParams() << owner << name);
        if (inf.eof())
            throw qApp->translate("toOracleExtract", "Constraint %1.%2 doesn't exist").arg(owner).arg(name);
        QString table((QString)inf.readValue());
        QString tchr((QString)inf.readValue());
        QString search((QString)inf.readValue());
        QString rOwner((QString)inf.readValue());
        QString rName((QString)inf.readValue());
        QString delRule((QString)inf.readValue());
        QString status((QString)inf.readValue());
        QString defferable((QString)inf.readValue());
        QString deffered((QString)inf.readValue());

        QString type =
            (tchr == "P") ? "PRIMARY KEY" :
            (tchr == "U") ? "UNIQUE" :
            (tchr == "R") ? "FOREIGN KEY" :
            "CHECK";

        QString sql = QString("ALTER TABLE %1.%2 ADD CONSTRAINT %3 %4\n")
                      .arg(quote(owner))
                      .arg(quote(table))
                      .arg(quote(name))
                      .arg(type);
        if (PROMPT)
            ret += PROMPT_SIMPLE.arg(sql);
        ret += sql;
        if (tchr == "C")
        {
            ret += "    (";
            ret += search;
            ret += ")\n";
        }
        else
        {
            ret += constraintColumns(owner, name);

            if (tchr == "R")
            {
                toConnectionSubLoan conn(ext.connection());
                toQuery query(conn, SQLConstraintTable, toQueryParams() << rOwner << rName);
                ret += " REFERENCES ";
                ret += ext.intSchema(rOwner, false);
                ret += quote((QString)query.readValue());
                ret += constraintColumns(rOwner, rName);

                if (delRule == "CASCADE")
                    ret += "ON DELETE CASCADE\n";
            }
        }
        if (connection().version() < "08")
        {
            if (status != "ENABLED")
                ret += status;
        }
        else
        {
            ret += defferable;
            ret += "\nINITIALLY ";
            ret += deffered;
            ret += "\n";
            ret += status;
        }
        ret += ";\n\n\n";
    }
    return ret;
}

static toSQL SQLDBLink("toOracleExtract:ExtractDBLink",
                       "SELECT l.userid,\n"
                       "       l.password,\n"
                       "       l.host\n"
                       "  FROM sys.link$ l,\n"
                       "       sys.user$ u\n"
                       " WHERE u.name    = :own<char[100]>\n"
                       "   AND l.owner# = u.user#\n"
                       "   AND l.name LIKE :nam<char[100]>||'%'",
                       "Get information about a DB Link, must have same binds and columns");

QString toOracleExtract::createDBLink(
                                      const QString &owner,
                                      const QString &name)
{
    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, SQLDBLink, toQueryParams() << owner << name);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "DBLink %1.%2 doesn't exist").arg(owner).arg(name);
    QString user((QString)inf.readValue());
    QString password((QString)inf.readValue());
    QString host((QString)inf.readValue());
    QString publ = (owner == "PUBLIC") ? " PUBLIC" : "";
    QString sql = QString("CREATE%1 DATABASE LINK %2").arg(publ).arg(quote(name));
    QString ret;
    if (PROMPT)
        ret += PROMPT_SIMPLE.arg(sql);
    ret += sql;
    ret += QString("\nCONNECT TO %1 IDENTIFIED BY %2 USING '%3';\n\n\n").
           arg(user.toLower()).
           arg(password.toLower()).
           arg(prepareDB(host));
    return ret;
}

static toSQL SQLDirectory("toOracleExtract:ExtractDirectory",
                          "SELECT directory_name,\n"
                          "       directory_path\n"
                          "  FROM all_directories\n"
                          " WHERE directory_name = :nam<char[100]>",
                          "Get information about a DB Link, must have same binds and columns");

QString toOracleExtract::createDirectory(
        const QString &owner,
        const QString &name)
{
    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, SQLDirectory, toQueryParams() << name);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Directory %1 doesn't exist").arg(name);
    QString dname((QString)inf.readValue());
    QString path((QString)inf.readValue());
    QString sql = QString("CREATE DIRECTORY %1").arg(quote(dname));
    QString ret;
    if (PROMPT)
        ret += PROMPT_SIMPLE.arg(sql);
    ret += QString("CREATE DIRECTORY %1 AS '%2';").arg(quote(dname)).arg(path);
    return ret;
}

static toSQL SQLPartitionSegmentType("toOracleExtract:PartitionSegment type",
                                     "SELECT SUBSTR(segment_type,7),\n"
                                     "       TO_CHAR(blocks)\n"
                                     "  FROM %1\n"
                                     " WHERE segment_name = :nam<char[100]>\n"
                                     "   AND partition_name = :prt<char[100]>\n"
                                     "   AND owner = :own<char[100]>",
                                     "Get segment type for object partition, same binds and columns");
#if 0
static toSQL SQLExchangeIndex("toOracleExtract:ExchangeIndex",
                              "SELECT\n"
                              "        LTRIM(i.degree)\n"
                              "      , LTRIM(i.instances)\n"
                              "      , i.table_name\n"
                              "      , DECODE(\n"
                              "                i.uniqueness\n"
                              "               ,'UNIQUE',' UNIQUE'\n"
                              "               ,null\n"
                              "              )                       AS uniqueness\n"
                              "      , DECODE(\n"
                              "                i.index_type\n"
                              "               ,'BITMAP',' BITMAP'\n"
                              "               ,null\n"
                              "              )                       AS index_type\n"
                              "      , 'INDEX'                       AS organization\n"
                              "      , 'N/A'                         AS cache\n"
                              "      , 'N/A'                         AS pct_used\n"
                              "      , p.pct_free\n"
                              "      , DECODE(\n"
                              "                p.ini_trans\n"
                              "               ,0,1\n"
                              "               ,null,1\n"
                              "               ,p.ini_trans\n"
                              "              )                       AS ini_trans\n"
                              "      , DECODE(\n"
                              "                p.max_trans\n"
                              "               ,0,255\n"
                              "               ,null,255\n"
                              "               ,p.max_trans\n"
                              "              )                       AS max_trans\n"
                              "      , p.initial_extent\n"
                              "      , p.next_extent\n"
                              "      , p.min_extent\n"
                              "      , DECODE(\n"
                              "                p.max_extent\n"
                              "               ,2147483645,'unlimited'\n"
                              "               ,           p.max_extent\n"
                              "              )                       AS max_extent\n"
                              "      , p.pct_increase\n"
                              "      , NVL(p.freelists,1)\n"
                              "      , NVL(p.freelist_groups,1)\n"
                              "      , p.buffer_pool                 AS buffer_pool\n"
                              "      , DECODE(\n"
                              "                p.logging\n"
                              "               ,'NO','NOLOGGING'\n"
                              "               ,     'LOGGING'\n"
                              "              )                       AS logging\n"
                              "      , p.tablespace_name             AS tablespace_name\n"
                              "      , %2                            AS blocks\n"
                              " FROM\n"
                              "        sys.all_indexes  i\n"
                              "      , sys.all_ind_%1s  p\n"
                              " WHERE\n"
                              "            p.index_name   = :nam<char[100]>\n"
                              "        AND p.%1_name      = :typ<char[100]>\n"
                              "        AND i.index_name   = p.index_name\n"
                              "        AND p.index_owner  = :own<char[100]>\n"
                              "        AND i.owner        = p.index_owner",
                              "Get information about exchange index, "
                              "must have same %, binds and columns");

QString toOracleExtract::createExchangeIndex(
        const QString &schema,
        const QString &owner,
        const QString &name)
{
    QStringList str = name.split(":");
    if (str.count() != 2)
        throw ("When calling createExchangeIndex name should contain :");
    QString segment = str.first();
    QString partition = str.last();

    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, segments(SQLPartitionSegmentType), toQueryParams() << segment << partition << owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Exchange index %1.%2 doesn't exist").arg(owner).arg(name);
    QString type = (QString)inf.readValue();
    QString blocks = (QString)inf.readValue();

    QString sql = toSQL::string(SQLExchangeIndex, connection()).arg(type).arg(blocks);
    toQList result = toQuery::readQuery(connection(), sql, toQueryParams() << segment << partition << owner);
    QString degree = (QString)Utils::toShift(result);
    QString instances = (QString)Utils::toShift(result);
    QString table = (QString)Utils::toShift(result);
    QString unique = (QString)Utils::toShift(result);
    QString bitmap = (QString)Utils::toShift(result);

    Utils::toUnShift(result, toQValue(""));

    sql = QString("CREATE%1%2 INDEX %3%4 ON %3%5\n").
          arg(unique).arg(bitmap).arg(schema).arg(segment).arg(quote(table));

    QString ret;
    if (PROMPT)
        ret += PROMPT_SIMPLE.arg(sql);
    ret += sql;
    ret += indexColumns("", owner, segment);
    if (ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty() )
    {
        ret += "PARALLEL\n(\n  DEGREE            ";
        ret += degree;
        ret += "\n  INSTANCES         ";
        ret += instances;
        ret += ")\n";
    }
    ret += segmentAttributes(result);
    ret += ";\n\n\n";
    return ret;
}

static toSQL SQLExchangeTable("toOracleExtract:ExchangeTable",
                              "SELECT\n"
                              "        DECODE(\n"
                              "                t.monitoring\n"
                              "               ,'NO','NOMONITORING'\n"
                              "               ,     'MONITORING'\n"
                              "              )                              AS monitoring\n"
                              "      , t.table_name\n"
                              "      , LTRIM(t.degree)                      AS degree\n"
                              "      , LTRIM(t.instances)                   AS instances\n"
                              "      , 'HEAP'                               AS organization\n"
                              "      , DECODE(\n"
                              "                t.cache\n"
                              "               ,'y','CACHE'\n"
                              "               ,    'NOCACHE'\n"
                              "              )                              AS cache\n"
                              "      , p.pct_used\n"
                              "      , p.pct_free\n"
                              "      , p.ini_trans\n"
                              "      , p.max_trans\n"
                              "      , p.initial_extent\n"
                              "      , p.next_extent\n"
                              "      , p.min_extent\n"
                              "      , DECODE(\n"
                              "                p.max_extent\n"
                              "               ,2147483645,'unlimited'\n"
                              "               ,p.max_extent\n"
                              "              )                              AS max_extents\n"
                              "      , p.pct_increase\n"
                              "      , p.freelists\n"
                              "      , p.freelist_groups\n"
                              "      , p.buffer_pool                        AS buffer_pool\n"
                              "      , DECODE(\n"
                              "                p.logging\n"
                              "               ,'NO','NOLOGGING'\n"
                              "               ,     'LOGGING'\n"
                              "              )                              AS logging\n"
                              "      , p.tablespace_name                    AS tablespace_name\n"
                              "      , %2 - NVL(p.empty_blocks,0)           AS blocks\n"
                              " FROM\n"
                              "        sys.all_tables        t\n"
                              "      , sys.all_tab_%1s       p\n"
                              " WHERE\n"
                              "            p.table_name   = :nam<char[100]>\n"
                              "        AND p.%1_name      = :sgm<char[100]>\n"
                              "        AND t.table_name   = p.table_name\n"
                              "        AND p.table_owner  = :own<char[100]>\n"
                              "        AND t.owner        = p.table_owner",
                              "Get information about exchange table, "
                              "must have same %, binds and columns");

QString toOracleExtract::createExchangeTable(
        const QString &schema,
        const QString &owner,
        const QString &name)
{
    QStringList str = name.split(":");
    if (str.count() != 2)
        throw ("When calling createExchangeTable name should contain :");
    QString segment = str.first();
    QString partition = str.last();

    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, segments(SQLPartitionSegmentType), toQueryParams() << segment << partition << owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Exchange table %1.%2 doesn't exist").arg(owner).arg(name);
    QString type = (QString)inf.readValue();
    QString blocks = (QString)inf.readValue();

    QString sql = toSQL::string(SQLExchangeTable, connection()).arg(type).arg(blocks);
    toQList result = toQuery::readQuery(connection(), sql, toQueryParams() << segment << partition << owner);
    QString ret = createTableText(result, schema, owner, segment);
    ret += ";\n\n";
    return ret;
}
#endif

QString toOracleExtract::createFunction(
                                        const QString &owner,
                                        const QString &name)
{
    return displaySource(owner, name, "FUNCTION", false);
}

static toSQL SQLIndexInfo7("toOracleExtract:IndexInfo",
                           "SELECT  'N/A'                           AS partitioned\n"
                           "      , table_name\n"
                           "      , table_owner\n"
                           "      , NULL\n"
                           "      , DECODE(\n"
                           "                uniqueness\n"
                           "               ,'UNIQUE',' UNIQUE'\n"
                           "               ,null\n"
                           "              )\n"
                           "      , null                            AS bitmap\n"
                           "      , null                            AS domain\n"
                           "      , null\n"
                           "      , null\n"
                           "      , null\n"
                           "  FROM sys.all_indexes\n"
                           " WHERE index_name = :nam<char[100]>\n"
                           "   AND owner = :own<char[100]>",
                           "Initial information about an index, same binds and columns",
                           "0700");
static toSQL SQLIndexInfo("toOracleExtract:IndexInfo",
                          "SELECT partitioned\n"
                          "      , table_name\n"
                          "      , table_owner\n"
                          "      , DECODE(index_type,'NORMAL/REV','REVERSE',NULL)\n"
                          "      , DECODE(\n"
                          "                uniqueness\n"
                          "               ,'UNIQUE',' UNIQUE'\n"
                          "               ,null\n"
                          "              )\n"
                          "      , DECODE(\n"
                          "                index_type\n"
                          "               ,'BITMAP',' BITMAP'\n"
                          "               ,null\n"
                          "              )\n"
                          "      , DECODE(\n"
                          "                index_type\n"
                          "               ,'DOMAIN','DOMAIN'\n"
                          "               ,null\n"
                          "              )\n"
                          "      , ityp_owner\n"
                          "      , ityp_name\n"
                          "      , parameters\n"
                          "  FROM sys.all_indexes\n"
                          " WHERE index_name = :nam<char[100]>\n"
                          "   AND owner = :own<char[100]>",
                          "",
                          "0800");

static toSQL SQLIndexSegment("toOracleExtract:IndexSegment",
                             "SELECT  LTRIM(i.degree)\n"
                             "      , LTRIM(i.instances)\n"
                             "      , DECODE(\n"
                             "                i.compression\n"
                             "               ,'ENABLED',i.prefix_length\n"
                             "               ,0\n"
                             "              )                             AS compressed\n"
                             "      , 'INDEX'                       AS organization\n"
                             "      , 'N/A'                         AS cache\n"
                             "      , 'N/A'                         AS pct_used\n"
                             "      , i.pct_free\n"
                             "      , DECODE(\n"
                             "                i.ini_trans\n"
                             "               ,0,1\n"
                             "               ,null,1\n"
                             "               ,i.ini_trans\n"
                             "              )                       AS ini_trans\n"
                             "      , DECODE(\n"
                             "                i.max_trans\n"
                             "               ,0,255\n"
                             "               ,null,255\n"
                             "               ,i.max_trans\n"
                             "              )                       AS max_trans\n"
                             "      , i.initial_extent\n"
                             "      , i.next_extent\n"
                             "      , i.min_extents\n"
                             "      , DECODE(\n"
                             "                i.max_extents\n"
                             "               ,2147483645,'unlimited'\n"
                             "               ,           i.max_extents\n"
                             "              )                       AS max_extents\n"
                             "      , i.pct_increase\n"
                             "      , NVL(i.freelists,1)\n"
                             "      , NVL(i.freelist_groups,1)\n"
                             "      , i.buffer_pool                 AS buffer_pool\n"
                             "      , DECODE(\n"
                             "                i.logging\n"
                             "               ,'NO','NOLOGGING'\n"
                             "               ,     'LOGGING'\n"
                             "              )                       AS logging\n"
                             "      , i.tablespace_name             AS tablespace_name\n"
                             "      , s.blocks\n"
                             "  FROM  sys.all_indexes   i\n"
                             "      , %1  s\n"
                             " WHERE  i.index_name   = :nam<char[100]>\n"
                             "   AND  s.segment_name = i.index_name\n"
                             "   AND  s.segment_type  = 'INDEX'\n"
                             "   AND  i.owner        = :own<char[100]>\n"
                             "   AND  s.owner        = i.owner",
                             "Get information about how index is stored",
                             "0801");

static toSQL SQLIndexSegment8("toOracleExtract:IndexSegment",
                              "SELECT  LTRIM(i.degree)\n"
                              "      , LTRIM(i.instances)\n"
                              "      , 0                             AS compressed\n"
                              "      , 'INDEX'                       AS organization\n"
                              "      , 'N/A'                         AS cache\n"
                              "      , 'N/A'                         AS pct_used\n"
                              "      , i.pct_free\n"
                              "      , DECODE(\n"
                              "                i.ini_trans\n"
                              "               ,0,1\n"
                              "               ,null,1\n"
                              "               ,i.ini_trans\n"
                              "              )                       AS ini_trans\n"
                              "      , DECODE(\n"
                              "                i.max_trans\n"
                              "               ,0,255\n"
                              "               ,null,255\n"
                              "               ,i.max_trans\n"
                              "              )                       AS max_trans\n"
                              "      , i.initial_extent\n"
                              "      , i.next_extent\n"
                              "      , i.min_extents\n"
                              "      , DECODE(\n"
                              "                i.max_extents\n"
                              "               ,2147483645,'unlimited'\n"
                              "               ,           i.max_extents\n"
                              "              )                       AS max_extents\n"
                              "      , i.pct_increase\n"
                              "      , NVL(i.freelists,1)\n"
                              "      , NVL(i.freelist_groups,1)\n"
                              "      , i.buffer_pool                 AS buffer_pool\n"
                              "      , DECODE(\n"
                              "                i.logging\n"
                              "               ,'NO','NOLOGGING'\n"
                              "               ,     'LOGGING'\n"
                              "              )                       AS logging\n"
                              "      , i.tablespace_name             AS tablespace_name\n"
                              "      , s.blocks\n"
                              "  FROM  sys.all_indexes   i\n"
                              "      , %1  s\n"
                              " WHERE  i.index_name   = :nam<char[100]>\n"
                              "   AND  s.segment_type = 'INDEX'\n"
                              "   AND  s.segment_name = i.index_name\n"
                              "   AND  i.owner        = :own<char[100]>\n"
                              "   AND  s.owner        = i.owner",
                              "",
                              "0800");

static toSQL SQLIndexSegment7("toOracleExtract:IndexSegment",
                              "SELECT  'N/A'                         AS degree\n"
                              "      , 'N/A'                         AS instances\n"
                              "      , 0                             AS compressed\n"
                              "      , 'INDEX'                       AS organization\n"
                              "      , 'N/A'                         AS cache\n"
                              "      , 'N/A'                         AS pct_used\n"
                              "      , i.pct_free\n"
                              "      , DECODE(\n"
                              "                i.ini_trans\n"
                              "               ,0,1\n"
                              "               ,null,1\n"
                              "               ,i.ini_trans\n"
                              "              )                       AS ini_trans\n"
                              "      , DECODE(\n"
                              "                i.max_trans\n"
                              "               ,0,255\n"
                              "               ,null,255\n"
                              "               ,i.max_trans\n"
                              "              )                       AS max_trans\n"
                              "      , i.initial_extent\n"
                              "      , i.next_extent\n"
                              "      , i.min_extents\n"
                              "      , DECODE(\n"
                              "                i.max_extents\n"
                              "               ,2147483645,'unlimited'\n"
                              "               ,           i.max_extents\n"
                              "              )                       AS max_extents\n"
                              "      , i.pct_increase\n"
                              "      , NVL(i.freelists,1)\n"
                              "      , NVL(i.freelist_groups,1)\n"
                              "      , 'N/A'                         AS buffer_pool\n"
                              "      , 'N/A'                         AS logging\n"
                              "      , i.tablespace_name             AS tablespace_name\n"
                              "      , s.blocks\n"
                              "  FROM  sys.all_indexes   i\n"
                              "      , %1  s\n"
                              " WHERE  i.index_name   = :nam<char[100]>\n"
                              "   AND  s.segment_name = i.index_name\n"
                              "   AND  s.segment_type = 'INDEX'\n"
                              "   AND  i.owner        = :own<char[100]>\n"
                              "   AND  s.owner        = i.owner",
                              "",
                              "0700");

QString toOracleExtract::createIndex(
                                     const QString &owner,
                                     const QString &name)
{
    if (!ext.getIndexes())
        return "";

    toConnectionSubLoan conn(ext.connection());
    toQList res = toQuery::readQuery(ext.connection(), SQLIndexInfo, toQueryParams() << name << owner);
    if (res.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find index %1.%2").arg(owner).arg(name);

    QString partitioned = (QString)Utils::toShift(res);
    QString table = (QString)Utils::toShift(res);
    QString tableOwner = (QString)Utils::toShift(res);
    QString reverse = (QString)Utils::toShift(res);
    QString unique = (QString)Utils::toShift(res);
    QString bitmap = (QString)Utils::toShift(res);
    QString domain = (QString)Utils::toShift(res);
    QString domOwner = (QString)Utils::toShift(res);
    QString domName = (QString)Utils::toShift(res);
    QString domParam = (QString)Utils::toShift(res);

    toQList storage = toQuery::readQuery(ext.connection(), segments(SQLIndexSegment), toQueryParams() << name << owner);
    QString degree = (QString)Utils::toShift(storage);
    QString instances = (QString)Utils::toShift(storage);
    QString compressed = (QString)Utils::toShift(storage);

    QString schema2 = ext.intSchema(tableOwner, false);

    QString ret;
    QString sql = QString("CREATE%1%2 INDEX %3.%4 ON %5.%6\n")
                  .arg(unique)
                  .arg(bitmap)
                  .arg(quote(owner))
                  .arg(quote(name))
                  .arg(schema2)
                  .arg(quote(table));
    if (PROMPT)
        ret += PROMPT_SIMPLE.arg(sql);
    ret += sql;
    ret += indexColumns("", owner, name);
    if (domain == "DOMAIN")
    {
        if (connection().version() >= "0801" && domOwner == "CTXSYS" && domName == "CONTEXT")
        {
            ret = createContextPrefs(owner, name, ret);
            return ret;
        }
        ret += QString("INDEXTYPE IS %1.%2\nPARAMETERS ('%3');\n\n").
               arg(quote(domOwner)).arg(quote(domName)).arg(prepareDB(domParam));
        return ret;
    }
    if (connection().version() >= "0800" && ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty() )
    {
        ret += QString("PARALLEL\n"
                       "(\n"
                       "  DEGREE            %1\n"
                       "  INSTANCES         %2\n"
                       ")\n").
               arg(degree).
               arg(instances);
    }
    if (partitioned == "YES")
        return createPartitionedIndex(owner, name, ret);

    Utils::toUnShift(storage, toQValue(""));
    ret += segmentAttributes(storage);
    if (!compressed.isEmpty() && compressed != "0" && ext.getStorage())
    {
        ret += "COMPRESS            ";
        ret += compressed;
        ret += "\n";
    }
    if (!reverse.isEmpty())
    {
        ret += reverse;
        ret += "\n";
    }
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::createMaterializedView(
        const QString &owner,
        const QString &name)
{
    return createMView(owner, name, "MATERIALIZED VIEW");
}

QString toOracleExtract::createMaterializedViewLog(
        const QString &owner,
        const QString &name)
{
    return createMViewLog(owner, name, "MATERIALIZED VIEW");
}

QString toOracleExtract::createPackage(
                                       const QString &owner,
                                       const QString &name)
{
    return displaySource(owner, name, "PACKAGE", false);
}

QString toOracleExtract::createPackageBody(
        const QString &owner,
        const QString &name)
{
    return displaySource(owner, name, "PACKAGE BODY", false);
}

QString toOracleExtract::createProcedure(
        const QString &owner,
        const QString &name)
{
    return displaySource(owner, name, "PROCEDURE", false);
}

static toSQL SQLProfileInfo("toOracleExtract:ProfileInfo",
                            "SELECT\n"
                            "        RPAD(resource_name,27)\n"
                            "      , DECODE(\n"
                            "                RESOURCE_NAME\n"
                            "               ,'PASSWORD_VERIFY_FUNCTION'\n"
                            "               ,DECODE(limit\n"
                            "                      ,'UNLIMITED','null'\n"
                            "                      ,limit\n"
                            "                      )\n"
                            "               ,limit\n"
                            "              )\n"
                            " FROM\n"
                            "        sys.dba_profiles\n"
                            " WHERE\n"
                            "        profile = :nam<char[100]>\n"
                            " ORDER\n"
                            "    BY\n"
                            "        DECODE(\n"
                            "               SUBSTR(resource_name,1,8)\n"
                            "              ,'FAILED_L',2\n"
                            "              ,'PASSWORD',2\n"
                            "              ,1\n"
                            "             )\n"
                            "      , resource_name",
                            "Get information about a profile, must have same binds and columns");

QString toOracleExtract::createProfile(const QString &name)
{
    toQList info = toQuery::readQuery(connection(),
                                      SQLProfileInfo,
                                      toQueryParams() << name);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find profile %1").arg(name);

    QString ret;
    if (PROMPT)
        ret = QString("PROMPT CREATE PROFILE %1\n\n").arg(quote(name));
    ret += QString("CREATE PROFILE %1\n").arg(quote(name));

    while (!info.empty())
    {
        ret += "   ";
        ret += (QString)Utils::toShift(info);
        ret += " ";
        ret += (QString)Utils::toShift(info);
        ret += "\n";
    }
    ret += ";\n\n";
    return ret;
}

static toSQL SQLRoleInfo("toOracleExtract:RoleInfo",
                         "SELECT\n"
                         "        DECODE(\n"
                         "                r.password_required\n"
                         "               ,'YES', DECODE(\n"
                         "                               u.password\n"
                         "                              ,'EXTERNAL','IDENTIFIED EXTERNALLY'\n"
                         "                              ,'IDENTIFIED BY VALUES ''' \n"
                         "                                || u.password || ''''\n"
                         "                             )\n"
                         "               ,'NOT IDENTIFIED'\n"
                         "              )                         AS password\n"
                         " FROM\n"
                         "        sys.dba_roles   r\n"
                         "      , sys.user$  u\n"
                         " WHERE\n"
                         "            r.role = :rol<char[100]>\n"
                         "        AND u.name = r.role",
                         "Get information about a role, must have same binds and columns");

QString toOracleExtract::createRole(const QString &name)
{
    toQList info = toQuery::readQuery(connection(),
                                      SQLRoleInfo,
                                      toQueryParams() << name);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find role %1").arg(name);

    QString ret;
    if (PROMPT)
        ret = QString("PROMPT CREATE ROLE %1\n\n").arg(quote(name));
    ret += QString("CREATE ROLE %1 %2;\n\n").arg(quote(name)).arg((QString)Utils::toShift(info));
    ret += grantedPrivs(quote(name), name, 1);
    return ret;
}

static toSQL SQLSequenceInfo("toOracleExtract:SequenceInfo",
                             "SELECT  'START WITH       '\n"
                             "         || LTRIM(TO_CHAR(last_number,'fm999999999'))\n"
                             "                                         AS start_with\n"
                             "      , 'INCREMENT BY     '\n"
                             "         || LTRIM(TO_CHAR(increment_by,'fm999999999')) AS imcrement_by\n"
                             "      , DECODE(\n"
                             "                min_value\n"
                             "               ,0,'NOMINVALUE'\n"
                             "               ,'MINVALUE         ' || TO_CHAR(min_value)\n"
                             "              )                          AS min_value\n"
                             "      , DECODE(\n"
                             "                TO_CHAR(max_value,'fm999999999999999999999999999')\n"
                             "               ,'999999999999999999999999999','NOMAXVALUE'\n"
                             "               ,'MAXVALUE         ' || TO_CHAR(max_value)\n"
                             "              )                          AS max_value\n"
                             "      , DECODE(\n"
                             "                cache_size\n"
                             "               ,0,'NOCACHE'\n"
                             "               ,'CACHE            ' || TO_CHAR(cache_size)\n"
                             "              )                          AS cache_size\n"
                             "      , DECODE(\n"
                             "                cycle_flag\n"
                             "               ,'Y','CYCLE'\n"
                             "               ,'N', 'NOCYCLE'\n"
                             "              )                          AS cycle_flag\n"
                             "      , DECODE(\n"
                             "                order_flag\n"
                             "               ,'Y','ORDER'\n"
                             "               ,'N', 'NOORDER'\n"
                             "              )                          AS order_flag\n"
                             " FROM\n"
                             "        sys.all_sequences\n"
                             " WHERE\n"
                             "            sequence_name  = :nam<char[100]>\n"
                             "        AND sequence_owner = :own<char[100]>",
                             "Get information about sequences, must have same binds");

QString toOracleExtract::createSequence(const QString &owner, const QString &name)
{
    toQList info = toQuery::readQuery(connection(),
                                      SQLSequenceInfo,
                                      toQueryParams() << name << owner);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find sequence %1").arg(name);

    QString ret;
    if (PROMPT)
        ret = QString("PROMPT CREATE SEQUENCE %1.%2\n\n").arg(quote(owner)).arg(quote(name));
    ret += QString("CREATE SEQUENCE %1.%2\n").arg(quote(owner)).arg(quote(name));

    while (!info.empty())
    {
        ret += "   ";
        ret += (QString)Utils::toShift(info);
        ret += "\n";
    }
    ret += ";\n\n";
    return ret;
}

#if 0
QString toOracleExtract::createSnapshot(const QString &owner, const QString &name)
{
    return createMViewLog(owner, name, "SNAPSHOT");
}

QString toOracleExtract::createSnapshotLog(const QString &owner, const QString &name)
{
    return createMViewLog(owner, name, "SNAPSHOT LOG");
}
#endif

static toSQL SQLSynonymInfo("toOracleExtract:SynonymInfo",
                            "SELECT  table_owner\n"
                            "      , table_name\n"
                            "      , NVL(db_link,'NULL')\n"
                            " FROM\n"
                            "        sys.all_synonyms\n"
                            " WHERE\n"
                            "            synonym_name = :nam<char[100]>\n"
                            "        AND owner = :own<char[100]>",
                            "Get information about a synonym, must have same binds and columns");

QString toOracleExtract::createSynonym(const QString &owner, const QString &name)
{
    toQList info = toQuery::readQuery(connection(),
                                      SQLSynonymInfo,
                                      toQueryParams() << name << owner);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find synonym %1.%2").arg(owner).arg(name);

    QString tableOwner = (QString)Utils::toShift(info);
    QString tableName = (QString)Utils::toShift(info);
    QString dbLink = (QString)Utils::toShift(info);
    if (dbLink == "NULL")
        dbLink = "";
    else
        dbLink = "@" + quote(dbLink);
    QString useSchema = (owner == "PUBLIC") ? QString("") : owner;
    QString isPublic  = (owner == "PUBLIC") ? " PUBLIC" : "";
    QString tableSchema = ext.intSchema(tableOwner, false);
    QString andReplace = (ext.getReplace()) ? " OR REPLACE" : "";

    QString sql = QString("CREATE%1%2 SYNONYM %3.%4 FOR %5.%6.%7")
		.arg(andReplace)
		.arg(isPublic)
		.arg(useSchema)
		.arg(quote(name))
		.arg(tableSchema)
		.arg(quote(tableName))
		.arg(dbLink);
    QString ret;
    if (PROMPT)
        ret += PROMPT_SIMPLE.arg(sql);
    ret += sql;
    ret += ";\n\n";
    return ret;
}

static toSQL SQLTableType("toOracleExtract:TableType",
                          "SELECT partitioned,\n"
                          "      iot_type\n"
                          " FROM sys.all_tables\n"
                          "WHERE table_name = :nam<char[100]>\n"
                          "  AND owner = :own<char[100]>",
                          "Get table type, must have same columns and binds",
                          "0800");

static toSQL SQLTableType7("toOracleExtract:TableType",
                           "SELECT 'NO',\n"
                           "      'NOT IOT'\n"
                           " FROM sys.all_tables\n"
                           "WHERE table_name = :nam<char[100]>\n"
                           "  AND owner = :own<char[100]>",
                           "",
                           "0700");

static toSQL SQLTableInfo("toOracleExtract:TableInfo",
                          "SELECT\n"
                          "        DECODE(\n"
                          "                t.monitoring\n"
                          "               ,'NO','NOMONITORING'\n"
                          "               ,     'MONITORING'\n"
                          "              )                       AS monitoring\n"
                          "      , 'N/A'                         AS table_name\n"
                          "      , LTRIM(t.degree)\n"
                          "      , LTRIM(t.instances)\n"
                          "      , DECODE(\n"
                          "                t.iot_type\n"
                          "               ,'IOT','INDEX'\n"
                          "               ,      'HEAP'\n"
                          "              )                       AS organization\n"
                          "      , DECODE(\n"
                          "                LTRIM(t.cache)\n"
                          "               ,'Y','CACHE'\n"
                          "               ,    'NOCACHE'\n"
                          "              )\n"
                          "      , t.pct_used\n"
                          "      , t.pct_free\n"
                          "      , DECODE(\n"
                          "                t.ini_trans\n"
                          "               ,0,1\n"
                          "               ,null,1\n"
                          "               ,t.ini_trans\n"
                          "              )                       AS ini_trans\n"
                          "      , DECODE(\n"
                          "                t.max_trans\n"
                          "               ,0,255\n"
                          "               ,null,255\n"
                          "               ,t.max_trans\n"
                          "              )                       AS max_trans\n"
                          "      , t.initial_extent\n"
                          "      , t.next_extent\n"
                          "      , t.min_extents\n"
                          "      , DECODE(\n"
                          "                t.max_extents\n"
                          "               ,2147483645,'UNLIMITED'\n"
                          "               ,           t.max_extents\n"
                          "              )                       AS max_extents\n"
                          "      , NVL(t.pct_increase,0)\n"
                          "      , NVL(t.freelists,1)\n"
                          "      , NVL(t.freelist_groups,1)\n"
                          "      , t.buffer_pool                 AS buffer_pool\n"
                          "      , DECODE(\n"
                          "                t.logging\n"
                          "               ,'NO','NOLOGGING'\n"
                          "               ,     'LOGGING'\n"
                          "              )                       AS logging\n"
                          "      , t.tablespace_name             AS tablespace_name\n"
                          "      , s.blocks - NVL(t.empty_blocks,0)\n"
                          " FROM\n"
                          "        sys.all_tables    t\n"
                          "      , %1  s\n"
                          " WHERE\n"
                          "            t.table_name   = :nam<char[100]>\n"
                          "        AND t.table_name   = s.segment_name\n"
                          "        AND s.segment_type = 'TABLE'\n"
                          "        AND s.owner        = :own<char[100]>\n"
                          "        AND t.owner        = s.owner",
                          "Get information about a vanilla table, must have same binds and columns",
                          "0801");

static toSQL SQLTableInfo8("toOracleExtract:TableInfo",
                           "SELECT\n"
                           "        'N/A'                         AS monitoring\n"
                           "      , 'N/A'                         AS table_name\n"
                           "      , LTRIM(t.degree)\n"
                           "      , LTRIM(t.instances)\n"
                           "      , DECODE(\n"
                           "                t.iot_type\n"
                           "               ,'IOT','INDEX'\n"
                           "               ,      'HEAP'\n"
                           "              )                       AS organization\n"
                           "      , DECODE(\n"
                           "                LTRIM(t.cache)\n"
                           "               ,'Y','CACHE'\n"
                           "               ,    'NOCACHE'\n"
                           "              )\n"
                           "      , t.pct_used\n"
                           "      , t.pct_free\n"
                           "      , DECODE(\n"
                           "                t.ini_trans\n"
                           "               ,0,1\n"
                           "               ,null,1\n"
                           "               ,t.ini_trans\n"
                           "              )                       AS ini_trans\n"
                           "      , DECODE(\n"
                           "                t.max_trans\n"
                           "               ,0,255\n"
                           "               ,null,255\n"
                           "               ,t.max_trans\n"
                           "              )                       AS max_trans\n"
                           "      , t.initial_extent\n"
                           "      , t.next_extent\n"
                           "      , t.min_extents\n"
                           "      , DECODE(\n"
                           "                t.max_extents\n"
                           "               ,2147483645,'unlimited'\n"
                           "               ,           t.max_extents\n"
                           "              )                       AS max_extents\n"
                           "      , NVL(t.pct_increase,0)\n"
                           "      , NVL(t.freelists,1)\n"
                           "      , NVL(t.freelist_groups,1)\n"
                           "      , 'N/A'                         AS buffer_pool\n"
                           "      , DECODE(\n"
                           "                t.logging\n"
                           "               ,'NO','NOLOGGING'\n"
                           "               ,     'LOGGING'\n"
                           "              )                       AS logging\n"
                           "      , t.tablespace_name             AS tablespace_name\n"
                           "      , s.blocks - NVL(t.empty_blocks,0)\n"
                           " FROM\n"
                           "        sys.all_tables    t\n"
                           "      , %1  s\n"
                           " WHERE\n"
                           "            t.table_name   = :nam<char[100]>\n"
                           "        AND t.table_name   = s.segment_name\n"
                           "        AND s.segment_type = 'TABLE'\n"
                           "        AND s.owner        = :own<char[100]>\n"
                           "        AND t.owner        = s.owner",
                           "",
                           "0800");

static toSQL SQLTableInfo7("toOracleExtract:TableInfo",
                           "SELECT\n"
                           "        'N/A'                         AS monitoring\n"
                           "      , 'N/A'                         AS table_name\n"
                           "      , LTRIM(t.degree)\n"
                           "      , LTRIM(t.instances)\n"
                           "      , 'N/A'                         AS organization\n"
                           "      , DECODE(\n"
                           "                LTRIM(t.cache)\n"
                           "               ,'Y','CACHE'\n"
                           "               ,    'NOCACHE'\n"
                           "              )\n"
                           "      , t.pct_used\n"
                           "      , t.pct_free\n"
                           "      , DECODE(\n"
                           "                t.ini_trans\n"
                           "               ,0,1\n"
                           "               ,null,1\n"
                           "               ,t.ini_trans\n"
                           "              )                       AS ini_trans\n"
                           "      , DECODE(\n"
                           "                t.max_trans\n"
                           "               ,0,255\n"
                           "               ,null,255\n"
                           "               ,t.max_trans\n"
                           "              )                       AS max_trans\n"
                           "      , t.initial_extent\n"
                           "      , t.next_extent\n"
                           "      , t.min_extents\n"
                           "      , DECODE(\n"
                           "                t.max_extents\n"
                           "               ,2147483645,'unlimited'\n"
                           "               ,           t.max_extents\n"
                           "              )                       AS max_extents\n"
                           "      , NVL(t.pct_increase,0)\n"
                           "      , NVL(t.freelists,1)\n"
                           "      , NVL(t.freelist_groups,1)\n"
                           "      , 'N/A'                         AS buffer_pool\n"
                           "      , 'N/A'                         AS logging\n"
                           "      , t.tablespace_name             AS tablespace_name\n"
                           "      , s.blocks - NVL(t.empty_blocks,0)\n"
                           " FROM\n"
                           "        sys.all_tables    t\n"
                           "      , %1  s\n"
                           " WHERE\n"
                           "            t.table_name   = :nam<char[100]>\n"
                           "        AND t.table_name   = s.segment_name\n"
                           "        AND s.segment_type = 'TABLE'\n"
                           "        AND s.owner        = :own<char[100]>\n"
                           "        AND t.owner        = s.owner",
                           "",
                           "0700");

QString toOracleExtract::createTable(const QString &owner, const QString &name)
{
    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, SQLTableType, toQueryParams() << name << owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Couldn't find table %1.%2").arg(owner).arg(name);

    QString partitioned((QString)inf.readValue());
    QString iot_type((QString)inf.readValue());

    if (iot_type == "IOT")
    {
        if (partitioned == "YES")
            return createPartitionedIOT(owner, name);
        else
            return createIOT(owner, name);
    }
    else if (partitioned == "YES")
        return createPartitionedTable(owner, name);

    toQList result = toQuery::readQuery(connection(), segments(SQLTableInfo), toQueryParams() << name << owner);
    QString ret = createTableText(result, owner, name);
    ret += ";\n\n";
    ret += createComments(owner, name);
    return ret;
}

static toSQL SQLTableConstraints("toOracleExtract:TableConstraints",
                                 "SELECT\n"
                                 "        constraint_type,\n"
                                 "        constraint_name,\n"
                                 "        NULL\n"
                                 " FROM\n"
                                 "        sys.all_constraints cn\n"
                                 " WHERE      table_name       = :nam<char[100]>\n"
                                 "        AND owner            = :own<char[100]>\n"
                                 "        AND constraint_type IN('P','U','C')\n"
                                 "        AND generated        != 'GENERATED NAME'\n"
                                 " ORDER\n"
                                 "    BY\n"
                                 "       DECODE(\n"
                                 "               constraint_type\n"
                                 "              ,'P',1\n"
                                 "              ,'U',2\n"
                                 "              ,'C',4\n"
                                 "             )\n"
                                 "     , constraint_name",
                                 "Get constraints tied to a table except referential, same binds and columns",
                                 "0800");
static toSQL SQLTableConstraints7("toOracleExtract:TableConstraints",
                                  "SELECT\n"
                                  "        constraint_type,\n"
                                  "        constraint_name,\n"
                                  "        search_condition\n"
                                  " FROM\n"
                                  "        sys.all_constraints cn\n"
                                  " WHERE      table_name       = :nam<char[100]>\n"
                                  "        AND owner            = :own<char[100]>\n"
                                  "        AND constraint_type IN('P','U','C')\n"
                                  " ORDER\n"
                                  "    BY\n"
                                  "       DECODE(\n"
                                  "               constraint_type\n"
                                  "              ,'P',1\n"
                                  "              ,'U',2\n"
                                  "              ,'C',4\n"
                                  "             )\n"
                                  "     , constraint_name",
                                  "",
                                  "0703");

static toSQL SQLTableReferences("toOracleExtract:TableReferences",
                                "SELECT\n"
                                "        constraint_name\n"
                                " FROM\n"
                                "        sys.all_constraints cn\n"
                                " WHERE      table_name       = :nam<char[100]>\n"
                                "        AND owner            = :own<char[100]>\n"
                                "        AND constraint_type IN('R')\n"
                                "        AND generated        != 'GENERATED NAME'\n"
                                " ORDER\n"
                                "    BY\n"
                                "       DECODE(\n"
                                "               constraint_type\n"
                                "              ,'R',1\n"
                                "             )\n"
                                "     , constraint_name",
                                "Get foreign constraints from a table, same binds and columns",
                                "0800");

static toSQL SQLTableReferences7("toOracleExtract:TableReferences",
                                 "SELECT\n"
                                 "        constraint_name\n"
                                 " FROM\n"
                                 "        sys.all_constraints cn\n"
                                 " WHERE      table_name       = :nam<char[100]>\n"
                                 "        AND owner            = :own<char[100]>\n"
                                 "        AND constraint_type IN('R')\n"
                                 " ORDER\n"
                                 "    BY\n"
                                 "       DECODE(\n"
                                 "               constraint_type\n"
                                 "              ,'R',1\n"
                                 "             )\n"
                                 "     , constraint_name",
                                 "",
                                 "0703");

static toSQL SQLTableTriggers("toOracleExtract:TableTriggers",
                              "SELECT  trigger_name\n"
                              "  FROM  sys.all_triggers\n"
                              " WHERE      table_name = :nam<char[100]>\n"
                              "        AND owner      = :own<char[100]>\n"
                              " ORDER  BY  trigger_name",
                              "Get triggers for a table, must have same columns and binds");

static toSQL SQLIndexNames("toOracleExtract:IndexNames",
                           "SELECT owner,index_name\n"
                           "  FROM sys.all_indexes a\n"
                           " WHERE table_name = :nam<char[100]>\n"
                           "   AND table_owner = :own<char[100]>",
                           "Get all indexes on table, same binds and columns");

QString toOracleExtract::createTableFamily(const QString &owner, const QString &name)
{
    QString ret = createTable(owner, name);

    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, SQLTableType, toQueryParams() << name << owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Couldn't find table %1.%2").arg(owner).arg(name);

    inf.readValue();
    QString iotType((QString)inf.readValue());

    toQList constraints = toQuery::readQuery(connection(), SQLTableConstraints, toQueryParams() << name << owner);
    toQList indexes = toQuery::readQuery(connection(), SQLIndexNames, toQueryParams() << name << owner);

    while (!indexes.empty())
    {
        QString indOwner(Utils::toShift(indexes));
        QString indName(Utils::toShift(indexes));
        bool add
            = true;
        if (iotType == "IOT")
        {
            for (toQList::iterator i = constraints.begin(); i != constraints.end(); i++)
            {
                QString consType = (QString)*i;
                i++;
                QString consName = (QString)*i;
                i++;
                if (consType == "P" && consName == indName && owner == indOwner)
                {
                    add = false;
                    break;
                }
            }
        }
        if (add)
            ret += createIndex(indOwner, indName);
    }

    while (!constraints.empty())
    {
        QString type = (QString)Utils::toShift(constraints);
        QString name = (QString)Utils::toShift(constraints);
        QString search = (QString)Utils::toShift(constraints);
        if ((type != "P" || iotType != "IOT") && !search.contains(" IS NOT NULL"))
            ret += createConstraint(owner, name);
    }

    toQList triggers = toQuery::readQuery(connection(), SQLTableTriggers, toQueryParams() << name << owner);
    while (!triggers.empty())
        ret += createTrigger(owner, (QString)Utils::toShift(triggers));
    return ret;
}

void toOracleExtract::createTableContents(const QString &owner, const QString &name, QTextStream &stream)
{
    if (ext.getContents())
    {
        if (PROMPT)
            stream << QString("PROMPT CONTENTS OF %1.%2\n\n").arg(quote(owner)).arg(quote(name));

        toConnectionSubLoan conn(ext.connection());
        toQuery query(conn, QString("SELECT * FROM %1.%2").arg(quote(owner)).arg(quote(name)), toQueryParams());

        toQColumnDescriptionList desc = query.describe();
        int cols = query.columns();
        bool first = true;
        QString dateformat(toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::ConfDateFormat).toString());

        QString beg = QString("INSERT INTO %1.%2 (").arg(quote(owner)).arg(quote(name));
        bool *dates = new bool[desc.size()];
        try
        {
            int num = 0;
            for (toQColumnDescriptionList::iterator i = desc.begin(); i != desc.end(); i++)
            {
                if (first)
                    first = false;
                else
                    beg += ",";
                beg += (*i).Name;
                dates[num] = (*i).Datatype.contains("DATE");
                num++;
            }
            beg += ") VALUES (";

            QRegularExpression find("'");

            int row = 0;
            int maxRow = ext.getCommitDistance();

            while (!query.eof())
            {
                QString line = beg;
                first = true;
                for (int i = 0; i < cols; i++)
                {
                    if (first)
                        first = false;
                    else
                        line += ",";
                    QString val = (QString)query.readValue();
                    if (dates[i])
                    {
                        if (val.isNull())
                            line += "NULL";
                        else
                            line += QString("TO_DATE('%1','%2')").arg(val).arg(dateformat);
                    }
                    else
                    {
                        if (val.isNull())
                            line += "NULL";
                        else
                        {
                            val.replace(find, "''");
                            line += "'";
                            line += val;
                            line += "'";
                        }
                    };
                }
                line += ");\n";
                stream << line;

                row++;
                if (maxRow > 0 && row % maxRow == 0)
                    stream << "COMMIT;\n";
            }
        }
        catch (...)
        {
            delete[] dates;
            throw;
        }
        delete[] dates;
        stream << "COMMIT;\n\n";
    }
}

QString toOracleExtract::createTableReferences(const QString &owner, const QString &name)
{
    QString ret;
    toQList constraints = toQuery::readQuery(ext.connection(), SQLTableReferences, toQueryParams() << name << owner);
    while (!constraints.empty())
        ret += createConstraint(owner, (QString)Utils::toShift(constraints));
    return ret;
}

static toSQL SQLTablespaceInfo("toOracleExtract:TablespaceInfo",
                               "SELECT  initial_extent\n"
                               "      , next_extent\n"
                               "      , min_extents\n"
                               "      , DECODE(\n"
                               "                max_extents\n"
                               "               ,2147483645,'unlimited'\n"
                               "               ,null,DECODE(\n"
                               "                              :bs<char[100]>\n"
                               "                            , 1,  57\n"
                               "                            , 2, 121\n"
                               "                            , 4, 249\n"
                               "                            , 8, 505\n"
                               "                            ,16,1017\n"
                               "                            ,32,2041\n"
                               "                            ,'\?\?\?'\n"
                               "                           )\n"
                               "               ,max_extents\n"
                               "              )                       AS max_extents\n"
                               "      , pct_increase\n"
                               "      , min_extlen\n"
                               "      , contents\n"
                               "      , logging\n"
                               "      , extent_management\n"
                               "      , allocation_type\n"
                               " FROM\n"
                               "        sys.dba_tablespaces\n"
                               " WHERE\n"
                               "        tablespace_name = :nam<char[100]>",
                               "Get tablespace information, must have same columns and binds",
                               "0801");

static toSQL SQLTablespaceInfo8("toOracleExtract:TablespaceInfo",
                                "SELECT  initial_extent\n"
                                "      , next_extent\n"
                                "      , min_extents\n"
                                "      , DECODE(\n"
                                "                max_extents\n"
                                "               ,2147483645,'unlimited'\n"
                                "               ,null,DECODE(\n"
                                "                             :bs<char[100]>\n"
                                "                            , 1,  57\n"
                                "                            , 2, 121\n"
                                "                            , 4, 249\n"
                                "                            , 8, 505\n"
                                "                            ,16,1017\n"
                                "                            ,32,2041\n"
                                "                            ,'\?\?\?'\n"
                                "                           )\n"
                                "               ,max_extents\n"
                                "              )                       AS max_extents\n"
                                "      , pct_increase\n"
                                "      , min_extlen\n"
                                "      , contents\n"
                                "      , DECODE(\n"
                                "                logging \n"
                                "               ,'NO','NOLOGGING'\n"
                                "               ,     'LOGGING'\n"
                                "              )                       AS logging\n"
                                "      , 'N/A'                         AS extent_management\n"
                                "      , 'N/A'                         AS allocation_type\n"
                                " FROM\n"
                                "        sys.dba_tablespaces\n"
                                " WHERE\n"
                                "        tablespace_name = :nam<char[100]>",
                                "",
                                "0800");

static toSQL SQLTablespaceInfo7("toOracleExtract:TablespaceInfo",
                                "SELECT  initial_extent\n"
                                "      , next_extent\n"
                                "      , min_extents\n"
                                "      , DECODE(\n"
                                "                max_extents\n"
                                "               ,2147483645,'unlimited'\n"
                                "               ,null,DECODE(\n"
                                "                             $block_size\n"
                                "                            , 1,  57\n"
                                "                            , 2, 121\n"
                                "                            , 4, 249\n"
                                "                            , 8, 505\n"
                                "                            ,16,1017\n"
                                "                            ,32,2041\n"
                                "                            ,'\?\?\?'\n"
                                "                           )\n"
                                "               ,max_extents\n"
                                "              )                       AS max_extents\n"
                                "      , pct_increase\n"
                                "      , min_extlen\n"
                                "      , contents\n"
                                "      , DECODE(\n"
                                "                logging \n"
                                "               ,'NO','NOLOGGING'\n"
                                "               ,     'LOGGING'\n"
                                "              )                       AS logging\n"
                                "      , 'N/A'                         AS extent_management\n"
                                "      , 'N/A'                         AS allocation_type\n"
                                " FROM\n"
                                "        sys.dba_tablespaces\n"
                                " WHERE\n"
                                "        tablespace_name = :nam<char[100]>",
                                "",
                                "0700");

static toSQL SQLDatafileInfo("toOracleExtract:DatafileInfo",
                             "SELECT\n"
                             "        file_name\n"
                             "      , bytes\n"
                             "      , autoextensible\n"
                             "      , DECODE(\n"
                             "                SIGN(2147483645 - maxbytes)\n"
                             "               ,-1,'unlimited'\n"
                             "               ,maxbytes\n"
                             "              )                               AS maxbytes\n"
                             "      , increment_by * :bs<char[100]> * 1024     AS increment_by\n"
                             " FROM\n"
                             "        (select bytes,autoextensible,blocks,increment_by,maxbytes,file_name,tablespace_name\n"
                             "         from sys.dba_temp_files\n"
                             "          union\n"
                             "         select bytes,autoextensible,blocks,increment_by,maxbytes,file_name,tablespace_name\n"
                             "         from sys.dba_data_files)\n"
                             " WHERE\n"
                             "        tablespace_name = :nam<char[100]>\n"
                             " ORDER  BY file_name",
                             "Get information about datafiles in a tablespace, "
                             "same binds and columns",
                             "0800");

static toSQL SQLDatafileInfo7("toOracleExtract:DatafileInfo",
                              "SELECT\n"
                              "        file_name\n"
                              "      , bytes\n"
                              "      , 'N/A'                                 AS autoextensible\n"
                              "      , 'N/A'                                 AS maxbytes\n"
                              "      , DECODE(:bs<char[100]>,\n"
                              "               NULL,'N/A','N/A')              AS increment_by\n"
                              " FROM\n"
                              "        sys.dba_data_files\n"
                              " WHERE\n"
                              "        tablespace_name = :nam<char[100]>\n"
                              " ORDER  BY file_name",
                              "",
                              "0700");

QString toOracleExtract::createTablespace(const QString &name)
{
    toQList info = toQuery::readQuery(connection(),
                                      SQLTablespaceInfo,
                                      toQueryParams() << QString::number(ext.getBlockSize()) << name);

    if (info.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find tablespace %1").arg(name);

    QString initial = (QString)Utils::toShift(info);
    QString next = (QString)Utils::toShift(info);
    QString minExtents = (QString)Utils::toShift(info);
    QString maxExtents = (QString)Utils::toShift(info);
    QString pctIncrease = (QString)Utils::toShift(info);
    QString minExtlen = (QString)Utils::toShift(info);
    QString contents = (QString)Utils::toShift(info);
    QString logging = (QString)Utils::toShift(info);
    QString extentManagement = (QString)Utils::toShift(info);
    QString allocationType = (QString)Utils::toShift(info);

    QString ret;
    QString sql;

    if (extentManagement == "LOCAL" && contents == "TEMPORARY")
        sql = QString("CREATE TEMPORARY TABLESPACE %1\n").arg(quote(name));
    else
        sql = QString("CREATE TABLESPACE %1\n").arg(quote(name));

    if (PROMPT)
        ret += PROMPT_SIMPLE.arg(sql);
    ret += sql;

    toQList files = toQuery::readQuery(ext.connection(),
                                       SQLDatafileInfo,
                                       toQueryParams() << QString::number(ext.getBlockSize()) << name);
    if (extentManagement == "LOCAL" && contents == "TEMPORARY")
        ret += "TEMPFILE\n";
    else
        ret += "DATAFILE\n";
    QString comma = "  ";
    while (!files.empty())
    {
        QString fileName = (QString)Utils::toShift(files);
        QString bytes = (QString)Utils::toShift(files);
        QString autoExtensible = (QString)Utils::toShift(files);
        QString maxBytes = (QString)Utils::toShift(files);
        QString incrementBy = (QString)Utils::toShift(files);

        ret += QString("%1 '%2' SIZE %3 REUSE\n").
               arg(comma).
               arg(prepareDB(fileName)).
               arg(bytes);
        if (connection().version() >= "0800" && ext.getStorage())
        {
            ret += "       AUTOEXTEND ";
            if (autoExtensible == "YES")
                ret += QString("ON NEXT %1 MAXSIZE %2\n").
                       arg(incrementBy).
                       arg(maxBytes);
            else
                ret += "OFF\n";
        }
        comma = " ,";
    }

    if (ext.getStorage())
    {
        if (extentManagement == "LOCAL")
        {
            ret += "EXTENT MANAGEMENT LOCAL ";
            if (allocationType == "SYSTEM")
                ret += "AUTOALLOCATE\n";
            else
                ret += QString("UNIFORM SIZE %1\n").arg(next);
        }
        else
        {
            ret += QString("DEFAULT STORAGE\n"
                           "(\n"
                           "  INITIAL        %1\n").arg(initial);
            if (!next.isEmpty())
                ret += QString("  NEXT           %1\n").arg(next);
            ret += QString("  MINEXTENTS     %1\n"
                           "  MAXEXTENTS     %2\n"
                           "  PCTINCREASE    %3\n"
                           ")\n").
                   arg(minExtents).
                   arg(maxExtents).
                   arg(pctIncrease);
            if (minExtlen.toInt() > 0)
                ret += QString("MINIMUM EXTENT   %1\n").arg(minExtlen);
            if (connection().version() >= "0801")
                ret += "EXTENT MANAGEMENT DICTIONARY\n";
        }
        if (connection().version() >= "0800" && (contents != "TEMPORARY" || extentManagement != "LOCAL"))
        {
            ret += logging;
            ret += "\n";
        }
    }
    ret += ";\n\n\n";
    return ret;
}

static toSQL SQLTriggerInfo("toOracleExtract:TriggerInfo",
                            "SELECT  trigger_type\n"
                            "      , RTRIM(triggering_event)\n"
                            "      , table_owner\n"
                            "      , table_name\n"
                            "      , base_object_type\n"
                            "      , referencing_names\n"
                            "      , upper(description)\n"
                            "      , when_clause\n"
                            "      , trigger_body\n"
                            "      , DECODE(status,'ENABLED','ENABLE','DISABLE')\n"
                            " FROM\n"
                            "        sys.all_triggers\n"
                            " WHERE\n"
                            "            trigger_name = :nam<char[100]>\n"
                            "        AND owner        = :own<char[100]>",
                            "Get information about triggers, must have same binds and columns",
                            "0801");

static toSQL SQLTriggerInfo8("toOracleExtract:TriggerInfo",
                             "SELECT\n"
                             "        trigger_type\n"
                             "      , RTRIM(triggering_event)\n"
                             "      , table_owner\n"
                             "      , table_name\n"
                             "      , 'TABLE'                           AS base_object_type\n"
                             "      , referencing_names\n"
                             "      , upper(description)\n"
                             "      , when_clause\n"
                             "      , trigger_body\n"
                             "      , DECODE(status,'ENABLED','ENABLE','DISABLE')\n"
                             " FROM\n"
                             "        sys.all_triggers\n"
                             " WHERE\n"
                             "            trigger_name = :nam<char[100]>\n"
                             "        AND owner        = :own<char[100]>",
                             "",
                             "0800");

QString toOracleExtract::createTrigger(const QString &owner, const QString &name)
{
    if (!ext.getCode())
        return "";
    toQList result = toQuery::readQuery(ext.connection(), SQLTriggerInfo, toQueryParams() << name << owner);
    if (result.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find trigger %1.%2").arg(owner).arg(name);
    QString triggerType = (QString)Utils::toShift(result);
    QString event = (QString)Utils::toShift(result);
    QString tableOwner = (QString)Utils::toShift(result);
    QString table = (QString)Utils::toShift(result);
    QString baseType = (QString)Utils::toShift(result);
    QString refNames = (QString)Utils::toShift(result);
    QString description = (QString)Utils::toShift(result);
    QString when = (QString)Utils::toShift(result);
    QString body = (QString)Utils::toShift(result);
    QString status = (QString)Utils::toShift(result);

    QString trgType;
    if (triggerType.indexOf("BEFORE") >= 0)
        trgType = "BEFORE";
    else if (triggerType.indexOf("AFTER") >= 0)
        trgType = "AFTER";
    else if (triggerType.indexOf("INSTEAD OF") >= 0)
        trgType = "INSTEAD OF";


    QString trgPart = trgType + " " + event;
    QRegularExpression src("\\s" + trgPart + "\\s", QRegularExpression::CaseInsensitiveOption);
    description.replace(QRegularExpression("\nON"), QString("\n ON"));
    int pos = description.indexOf(src);
    //QString columns=description;
    QString columns;
    if (pos >= 0)
    {
        pos += trgPart.length() + 2;
        int endPos = description.indexOf(" ON ", pos);
        if (endPos >= 0)
        {
            columns = description.right(description.length() - pos);
            columns.truncate(endPos - pos);
        }
    }
    QString schema2 = ext.intSchema(tableOwner, false);
    QString object;
    if (baseType == "TABLE" || baseType == "VIEW")
    {
        object = schema2;
        object += table;
    }
    else if (baseType == "SCHEMA")
    {
        object = owner;
        object += "SCHEMA";
    }
    else
    {
        object = baseType;
    }
    QString sql = QString("CREATE OR REPLACE TRIGGER %1.%2\n").arg(quote(owner)).arg(quote(name));
    QString ret;
    if (PROMPT)
        ret += PROMPT_SIMPLE.arg(sql);
    ret += sql;
    ret += QString("%1 %2 %3 ON %4\n").arg(trgType).arg(event).arg(columns).arg(object);
    if (baseType.indexOf("TABLE") >= 0 || baseType.indexOf("VIEW") >= 0)
    {
        ret += refNames;
        ret += "\n";
    }
    if (triggerType.indexOf("EACH ROW") >= 0)
        ret += "FOR EACH ROW\n";
    if (!when.isEmpty())
    {
        ret += "WHEN (";
        ret += when;
        ret += ")\n";
    }
    ret += body;
    ret += "\n/\n\n\n";
    if (status != "ENABLE")
    {
        ret += QString("ALTER TRIGGER %1.%2 DISABLE;\n\n").arg(quote(owner)).arg(quote(name));
    }
    return ret;
}

QString toOracleExtract::createType(const QString &owner, const QString &name)
{
    return displaySource(owner, name, "TYPE", false);
}

static toSQL SQLUserInfo("toOracleExtract:UserInfo",
                         "SELECT\n"
                         "        DECODE(\n"
                         "                password\n"
                         "               ,'EXTERNAL','EXTERNALLY'\n"
                         "               ,'BY VALUES ''' || password || ''''\n"
                         "              )                         AS password\n"
                         "      , profile\n"
                         "      , default_tablespace\n"
                         "      , temporary_tablespace\n"
                         " FROM\n"
                         "        sys.dba_users\n"
                         " WHERE\n"
                         "        username = :nam<char[100]>",
                         "Information about authentication for a user, "
                         "same binds and columns");

static toSQL SQLUserQuotas("toOracleExtract:UserQuotas",
                           "SELECT\n"
                           "        DECODE(\n"
                           "                max_bytes\n"
                           "               ,-1,'unlimited'\n"
                           "               ,TO_CHAR(max_bytes,'99999999')\n"
                           "              )                         AS max_bytes\n"
                           "      , tablespace_name\n"
                           " FROM\n"
                           "        sys.dba_ts_quotas\n"
                           " WHERE\n"
                           "        username = :nam<char[100]>\n"
                           " ORDER  BY tablespace_name",
                           "Get information about tablespaces for a user, "
                           "same binds and columns");

QString toOracleExtract::createUser(const QString &name)
{
    toQList info = toQuery::readQuery(connection(), SQLUserInfo, toQueryParams() << name);

    if (info.size() != 4)
        throw qApp->translate("toOracleExtract", "Couldn't find user %1").arg(name);

    QString password = (QString)Utils::toShift(info);
    QString profile = (QString)Utils::toShift(info);
    QString defaultTablespace = (QString)Utils::toShift(info);
    QString temporaryTablespace = (QString)Utils::toShift(info);

    QString ret;
    QString nam;
    if (ext.getSchema() != "1" && !ext.getSchema().isEmpty())
        nam = ext.getSchema().toLower();
    else
        nam = quote(name);
    if (PROMPT)
        ret = QString("PROMPT CREATE USER %1\n\n").arg(nam);
    ret += QString("CREATE USER %1 IDENTIFIED %2\n"
                   "   PROFILE              %3\n"
                   "   DEFAULT TABLESPACE   %4\n"
                   "   TEMPORARY TABLESPACE %5\n").
           arg(nam).
           arg(password).
           arg(quote(profile)).
           arg(quote(defaultTablespace)).
           arg(quote(temporaryTablespace));

    if (ext.getStorage())
    {
        toQList quota = toQuery::readQuery(ext.connection(), SQLUserQuotas, toQueryParams() << name);
        while (!quota.empty())
        {
            QString siz = (QString)Utils::toShift(quota);
            QString tab = (QString)Utils::toShift(quota);
            ret += QString("   QUOTA %1 ON %2\n").
                   arg(siz).
                   arg(quote(tab));
        }
    }
    ret += ";\n\n\n";
    ret += grantedPrivs(nam, name, 3);
    return ret;
}

static toSQL SQLViewSource("toOracleExtract:ViewSource",
                           "SELECT  text\n"
                           " FROM\n"
                           "        sys.all_views\n"
                           " WHERE\n"
                           "            view_name = :nam<char[100]>\n"
                           "        AND owner = :own<char[100]>",
                           "Get the source of the view, must have same binds and columns");

static toSQL SQLViewColumns("toOracleExtract:ViewColumns",
                            "SELECT column_name\n"
                            "  FROM sys.all_tab_columns\n"
                            " WHERE table_name = :nam<char[100]>\n"
                            "   AND owner      = :own<char[100]>\n"
                            " ORDER BY column_id",
                            "Get column names of the view, must have same binds and columns");

QString toOracleExtract::createView(const QString &owner, const QString &name)
{
    if (!ext.getCode())
        return "";
    toQList source = toQuery::readQuery(ext.connection(),
                                        SQLViewSource,
                                        toQueryParams() << name << owner);
    if (source.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find view %1.%2").arg(quote(owner)).arg(quote(name));

    QString text = (QString)Utils::toShift(source);
    QString ret;
    QString sql = QString("CREATE OR REPLACE VIEW %1%2\n").arg(quote(owner)).arg(quote(name));
    if (PROMPT)
        ret += PROMPT_SIMPLE.arg(sql);
    ret += sql;
    toQList cols = toQuery::readQuery(connection(),
                                      SQLViewColumns,
                                      toQueryParams() << name << owner);
    ret += "(";
    QString sep = "\n    ";
    while (!cols.empty())
    {
        QString str = (QString)Utils::toShift(cols);
        ret += sep;
        ret += quote(str);
        sep = "\n  , ";
    }
    ret += "\n)\n";
    ret += "AS\n";
    ret += text;
    ret += ";\n\n\n";
    ret += createComments(owner, name);
    return ret;
}

// Implementation describe functions

void toOracleExtract::describeConstraint(const QString &owner, const QString &name,	std::list<QString> &lst)
{
    if (ext.getConstraints())
    {
        toConnectionSubLoan conn(ext.connection());
        toQuery inf(conn, SQLListConstraint, toQueryParams() << owner << name);
        if (inf.eof())
            throw qApp->translate("toOracleExtract", "Constraint %1.%2 doesn't exist").arg(owner).arg(name);
        QString table((QString)inf.readValue());
        QString tchr((QString)inf.readValue());
        QString search((QString)inf.readValue());
        QString rOwner((QString)inf.readValue());
        QString rName((QString)inf.readValue());
        QString delRule((QString)inf.readValue());
        QString status((QString)inf.readValue());
        QString defferable((QString)inf.readValue());
        QString deffered((QString)inf.readValue());

        QString type =
            (tchr == "P") ? "PRIMARY KEY" :
            (tchr == "U") ? "UNIQUE" :
            (tchr == "R") ? "FOREIGN KEY" :
            "CHECK";

        std::list<QString> ctx;
        ctx.insert(ctx.end(), quote(owner));
        ctx.insert(ctx.end(), "TABLE");
        ctx.insert(ctx.end(), quote(table));
        ctx.insert(ctx.end(), "CONSTRAINT");
        ctx.insert(ctx.end(), quote(name));

        QString ret = type;
        if (tchr == "C")
        {
            ret += " (";
            ret += search;
            ret += ")";
        }
        else
        {
            ret += " ";
            ret += constraintColumns(owner, name).simplified();

            if (tchr == "R")
            {
                toConnectionSubLoan conn(ext.connection());
                toQuery query(conn, SQLConstraintTable, toQueryParams() << rOwner << rName);
                ret += " REFERENCES ";
                ret += ext.intSchema(rOwner, false);
                ret += quote((QString)query.readValue());
                ret += constraintColumns(rOwner, rName);

                if (delRule == "CASCADE")
                    ret += "ON DELETE CASCADE";
            }
        }
        addDescription(lst, ctx, "DEFINITION", ret);

        if (status.startsWith("ENABLE"))
            status = "ENABLED";

        if (connection().version() < "08")
            ret = status;
        else
        {
            addDescription(lst, ctx, "STATUS", defferable);
            ret = "INITIALLY ";
            ret += deffered;
            addDescription(lst, ctx, "STATUS", ret);
            ret = status;
        }
        addDescription(lst, ctx, "STATUS", ret);
    }
}

void toOracleExtract::describeDBLink(const QString &owner, const QString &name, std::list<QString> &lst)
{
    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, SQLDBLink, toQueryParams() << owner << name);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "DBLink %1.%2 doesn't exist").arg(owner).arg(name);
    QString user((QString)inf.readValue());
    QString password((QString)inf.readValue());
    QString host((QString)inf.readValue());
    QString publ = (owner == "PUBLIC") ? "PUBLIC" : "";
    QString sql = QString("CREATE%1 DATABASE LINK %2").arg(publ).arg(quote(name));
    QString ret;

    std::list<QString> ctx;
    ctx.insert(ctx.end(), "DATABASE LINK");
    ctx.insert(ctx.end(), quote(name));

    addDescription(lst, ctx, publ, QString("%1 IDENTIFIED BY %2 USING '%3'").
                   arg(user.toLower()).
                   arg(password.toLower()).
                   arg(prepareDB(host)));
}

#if 0
void toOracleExtract::describeExchangeIndex(
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name)
{
    QStringList str = name.split(":");
    if (str.count() != 2)
        throw ("When calling createExchangeIndex name should contain :");
    QString segment = str.first();
    QString partition = str.last();

    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, segments(SQLPartitionSegmentType), toQueryParams() << segment <<  partition << owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Exchange index %1.%2 doesn't exist").arg(owner).arg(name);
    QString type = (QString)inf.readValue();
    QString blocks = (QString)inf.readValue();

    QString sql = toSQL::string(SQLExchangeIndex, connection()).arg(type).arg(blocks);
    toQList result = toQuery::readQuery(connection(), sql, toQueryParams() << segment << partition << owner);
    QString degree = (QString)Utils::toShift(result);
    QString instances = (QString)Utils::toShift(result);
    QString table = (QString)Utils::toShift(result);
    QString unique = (QString)Utils::toShift(result);
    QString bitmap = (QString)Utils::toShift(result);

    Utils::toUnShift(result, toQValue(""));

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), "TABLE");
    ctx.insert(ctx.end(), quote(table));
    ctx.insert(ctx.end(), "EXCHANGE INDEX");
    ctx.insert(ctx.end(), segment);
    ctx.insert(ctx.end(), partition);

    describeIndexColumns(lst, ctx, owner, segment);
    addDescription(lst, ctx, QString("%1%2").arg(unique).arg(bitmap));
    addDescription(lst, ctx, QString("%1%2").arg(segment).arg(table));
    if (ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty())
    {
        addDescription(lst, ctx, "PARALLEL", "DEGREE", degree);
        addDescription(lst, ctx, "PARALLEL", "INSTANCES", instances);
    }
    describeAttributes(lst, ctx, result);
}

void toOracleExtract::describeExchangeTable(
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name)
{
    QStringList str = name.split(":");
    if (str.count() != 2)
        throw ("When calling createExchangeTable name should contain :");
    QString segment = str.first();
    QString partition = str.last();

    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, segments(SQLPartitionSegmentType), toQueryParams() << segment << partition << owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Exchange table %1.%2 doesn't exist").arg(owner).arg(name);
    QString type = (QString)inf.readValue();
    QString blocks = (QString)inf.readValue();

    QString sql = toSQL::string(SQLExchangeTable, connection()).arg(type).arg(blocks);
    toQList result = toQuery::readQuery(connection(), sql, toQueryParams() << segment << partition << owner);
    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), "EXCHANGE TABLE");
    ctx.insert(ctx.end(), segment);
    ctx.insert(ctx.end(), partition);
    describeTableText(lst, ctx, result, schema, owner, segment);
}
#endif

void toOracleExtract::describeFunction(const QString &owner, const QString &name, std::list<QString> &lst)
{
	describeSource(lst, owner, name, "FUNCTION");
}

void toOracleExtract::describeIndex(const QString &owner, const QString &name, std::list<QString> &lst)
{
    if (!ext.getIndexes())
        return ;

    toQList res = toQuery::readQuery(connection(), SQLIndexInfo, toQueryParams() << name << owner);
    if (res.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find index %1.%2").arg(owner).arg(name);

    QString partitioned = (QString)Utils::toShift(res);
    QString table = (QString)Utils::toShift(res);
    QString tableOwner = (QString)Utils::toShift(res);
    QString reverse = (QString)Utils::toShift(res);
    QString unique = (QString)Utils::toShift(res);
    QString bitmap = (QString)Utils::toShift(res);
    QString domain = (QString)Utils::toShift(res);
    QString domOwner = (QString)Utils::toShift(res);
    QString domName = (QString)Utils::toShift(res);
    QString domParam = (QString)Utils::toShift(res);

    toQList storage = toQuery::readQuery(connection(), segments(SQLIndexSegment), toQueryParams() << name << owner);
    QString degree = (QString)Utils::toShift(storage);
    QString instances = (QString)Utils::toShift(storage);
    QString compressed = (QString)Utils::toShift(storage);

    QString schema2 = ext.intSchema(tableOwner, false);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), quote(owner));
    ctx.insert(ctx.end(), QString("INDEX"));
    ctx.insert(ctx.end(), quote(name));
    addDescription(lst, ctx, "ON", schema2 + quote(table));
    addDescription(lst, ctx, QString("%1%2 INDEX").arg(unique).arg(bitmap).mid(1));

    addDescription(lst, ctx);
    if (!reverse.isEmpty())
        addDescription(lst, ctx, reverse);
    describeIndexColumns(lst, ctx, owner, name);
    if (domain == "DOMAIN")
    {
        addDescription(lst, ctx, "DOMAIN", QString("INDEXTYPE IS DOMAINOWNER %1.%2").
                       arg(quote(domOwner)).
                       arg(quote(domName)));
        addDescription(lst, ctx, "DOMAIN", QString("PARAMETERS %1").arg(domParam));
        return ;
    }
    if (connection().version() >= "0800" && ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty())
    {
        addDescription(lst, ctx, "PARALLEL", QString("DEGREE %1").arg(degree));
        addDescription(lst, ctx, "PARALLEL", QString("INSTANCES %1").arg(instances));
    }
    if (partitioned == "YES")
    {
        describePartitionedIndex(lst, ctx, owner, name);
        return ;
    }

    Utils::toUnShift(storage, toQValue(""));
    describeAttributes(lst, ctx, storage);
    if (!compressed.isEmpty() && compressed != "0" && ext.getStorage())
        addDescription(lst, ctx, "COMPRESS", compressed);
}

#if 0
void toOracleExtract::describeMaterializedView(const QString &owner, const QString &name, std::list<QString> &lst)
{
    describeMView(owner, name, "MATERIALIZED VIEW", lst);
}

void toOracleExtract::describeMaterializedViewLog(std::list<QString> &lst, const QString &owner, const QString &name)
{
    describeMViewLog(lst, owner, name, "MATERIALIZED VIEW LOG");
}
#endif

void toOracleExtract::describePackage(const QString &owner, const QString &name, std::list<QString> &lst)
{
    describeSource(lst, owner, name, "PACKAGE");
}

void toOracleExtract::describePackageBody(const QString &owner, const QString &name, std::list<QString> &lst)
{
    describeSource(lst, owner, name, "PACKAGE BODY");
}

void toOracleExtract::describeProcedure(const QString &owner, const QString &name, std::list<QString> &lst)
{
    describeSource(lst, owner, name, "PROCEDURE");
}

void toOracleExtract::describeProfile(const QString &name, std::list<QString> &lst)
{
    toQList info = toQuery::readQuery(connection(), SQLProfileInfo, toQueryParams() << name);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find profile %1").arg(name);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), "NONE");
    ctx.insert(ctx.end(), "PROFILE");
    ctx.insert(ctx.end(), quote(name));
    addDescription(lst, ctx);

    while (!info.empty())
    {
        QString nam = (QString)Utils::toShift(info);
        QString val = (QString)Utils::toShift(info);
        addDescription(lst, ctx, nam, val);
    }
}

void toOracleExtract::describeRole(const QString &name, std::list<QString> &lst)
{
    toQList info = toQuery::readQuery(connection(), SQLRoleInfo, toQueryParams() << name);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find role %1").arg(name);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), "NONE");
    ctx.insert(ctx.end(), "ROLE");
    ctx.insert(ctx.end(), quote(name));
    addDescription(lst, ctx);
    addDescription(lst, ctx, "INFO", (QString)Utils::toShift(info));
    describePrivs(lst, ctx, name);
}

void toOracleExtract::describeSequence(const QString &owner, const QString &name, std::list<QString> &lst)
{
    toQList info = toQuery::readQuery(connection(),
                                      SQLSequenceInfo,
                                      toQueryParams() << name << owner);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find sequence %1").arg(name);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), quote(owner));
    ctx.insert(ctx.end(), "SEQUENCE");
    ctx.insert(ctx.end(), quote(name));

    addDescription(lst, ctx);

    while (!info.empty())
        addDescription(lst, ctx, (QString)Utils::toShift(info));
}

#if 0
void toOracleExtract::describeSnapshot(
                                       std::list<QString> &lst,
                                       const QString &owner,
                                       const QString &name)
{
    describeMView(lst, schema, owner, name, "SNAPSHOT");
}

void toOracleExtract::describeSnapshotLog(
        std::list<QString> &lst,
        const QString &owner,
        const QString &name)
{
    describeMViewLog(lst, schema, owner, name, "SNAPSHOT LOG");
}
#endif

void toOracleExtract::describeSynonym(const QString &owner, const QString &name, std::list<QString> &lst)
{
    toQList info = toQuery::readQuery(connection(), SQLSynonymInfo, toQueryParams() << name << owner);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find synonym %1.%2").arg(owner).arg(name);

    QString tableOwner = (QString)Utils::toShift(info);
    QString tableName = (QString)Utils::toShift(info);
    QString dbLink = (QString)Utils::toShift(info);
    if (dbLink == "NULL")
        dbLink = "";
    else
        dbLink = "@" + quote(dbLink);
    QString tableSchema = ext.intSchema(tableOwner, true);

    std::list<QString> ctx;

    ctx.insert(ctx.end(), owner);
    addDescription(lst, ctx, "SYNONYM", QString("%1.%2.%3").arg(tableSchema).arg(quote(tableName)).arg(dbLink));
}

void toOracleExtract::describeTable(const QString &owner, const QString &name, std::list<QString> &lst)
{
    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, SQLTableType, toQueryParams() << name << owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Couldn't find table %1.%2").arg(owner).arg(name);

    QString partitioned((QString)inf.readValue());
    QString iot_type((QString)inf.readValue());

    std::list<QString> ctx;
    ctx.insert(ctx.end(), quote(owner));
    ctx.insert(ctx.end(), "TABLE");
    ctx.insert(ctx.end(), quote(name));

    if (iot_type == "IOT" && ext.getIndexes())
    {
        if (partitioned == "YES")
            describePartitionedIOT(lst, ctx, owner, name);
        else
            describeIOT(lst, ctx, owner, name);
        return ;
    }
    else if (partitioned == "YES")
    {
        describePartitionedTable(lst, ctx, owner, name);
        return ;
    }

    toQList result = toQuery::readQuery(connection(), segments(SQLTableInfo), toQueryParams() << name << owner);
    describeTableText(lst, ctx, result, owner, name);
    describeComments(lst, ctx, owner, name);
}

void toOracleExtract::describeTableFamily(const QString &owner, const QString &name, std::list<QString> &lst)
{
    describeTable(owner, name, lst);

    toQList indexes = toQuery::readQuery(connection(), SQLIndexNames, toQueryParams() << name << owner);
    while (!indexes.empty())
    {
        QString indOwner(Utils::toShift(indexes));
        describeIndex(indOwner, (QString)Utils::toShift(indexes), lst);
    }

    toConnectionSubLoan conn(ext.connection());
    toQuery inf(conn, SQLTableType, toQueryParams() << name << owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Couldn't find table %1.%2").arg(owner).arg(name);

    inf.readValue();
    QString iotType((QString)inf.readValue());

    toQList constraints = toQuery::readQuery(connection(), SQLTableConstraints, toQueryParams() << name << owner);
    while (!constraints.empty())
    {
        if ( (QString)Utils::toShift(constraints) != "P" || iotType != "IOT")
            describeConstraint(owner, (QString)Utils::toShift(constraints), lst);
        else
            Utils::toShift(constraints);
        Utils::toShift(constraints);
    }

    toQList triggers = toQuery::readQuery(ext.connection(), SQLTableTriggers, toQueryParams() << name << owner);
    while (!triggers.empty())
        describeTrigger(owner, (QString)Utils::toShift(triggers), lst);
}

void toOracleExtract::describeTableReferences(const QString &owner, const QString &name, std::list<QString> &lst)
{
    toQList constraints = toQuery::readQuery(connection(), SQLTableReferences, toQueryParams() << name << owner);
    while (!constraints.empty())
        describeConstraint(owner, (QString)Utils::toShift(constraints), lst);
}

void toOracleExtract::describeTablespace(const QString &owner, const QString &name, std::list<QString> &lst)
{
    toQList info = toQuery::readQuery(ext.connection(),
                                      SQLTablespaceInfo,
                                      toQueryParams() << QString::number(ext.getBlockSize()) << name);

    if (info.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find tablespace %1").arg(name);

    QString initial = (QString)Utils::toShift(info);
    QString next = (QString)Utils::toShift(info);
    QString minExtents = (QString)Utils::toShift(info);
    QString maxExtents = (QString)Utils::toShift(info);
    QString pctIncrease = (QString)Utils::toShift(info);
    QString minExtlen = (QString)Utils::toShift(info);
    QString contents = (QString)Utils::toShift(info);
    QString logging = (QString)Utils::toShift(info);
    QString extentManagement = (QString)Utils::toShift(info);
    QString allocationType = (QString)Utils::toShift(info);

    std::list<QString> ctx;

    ctx.insert(ctx.end(), "NONE");
    if (extentManagement == "LOCAL" && contents == "TEMPORARY")
        ctx.insert(ctx.end(), "TEMPORARY TABLESPACE");
    else
        ctx.insert(ctx.end(), "TABLESPACE");
    ctx.insert(ctx.end(), quote(name));

    addDescription(lst, ctx);

    toQList files = toQuery::readQuery(connection(),
                                       SQLDatafileInfo,
                                       toQueryParams() << QString::number(ext.getBlockSize()) << name);
    while (!files.empty())
    {
        QString fileName = (QString)Utils::toShift(files);
        QString bytes = (QString)Utils::toShift(files);
        QString autoExtensible = (QString)Utils::toShift(files);
        QString maxBytes = (QString)Utils::toShift(files);
        QString incrementBy = (QString)Utils::toShift(files);

        std::list<QString> cctx = ctx;
        if (extentManagement == "LOCAL" && contents == "TEMPORARY")
            cctx.insert(cctx.end(), "TEMPFILE");
        else
            cctx.insert(cctx.end(), "DATAFILE");
        cctx.insert(cctx.end(), prepareDB(fileName));

        addDescription(lst, cctx);
        addDescription(lst, cctx, QString("SIZE %3").arg(bytes));

        if (connection().version() >= "0800" && ext.getStorage())
        {
            QString ret = "AUTOEXTEND ";
            if (autoExtensible == "YES")
                ret += QString("ON NEXT %1 MAXSIZE %2\n").
                       arg(incrementBy).
                       arg(maxBytes);
            else
                ret += "OFF\n";
            addDescription(lst, cctx, ret);
        }
    }

    if (ext.getStorage())
    {
        if (extentManagement == "LOCAL")
        {
            QString ret = "EXTENT MANAGEMENT LOCAL ";
            if (allocationType == "SYSTEM")
                ret += "AUTOALLOCATE";
            else
                ret += QString("UNIFORM SIZE %1").arg(next);
            addDescription(lst, ctx, ret);
        }
        else
        {
            addDescription(lst, ctx, "STORAGE", QString("INITIAL %1").arg(initial));
            if (!next.isEmpty())
                addDescription(lst, ctx, "STORAGE", QString("NEXT %1").arg(next));
            addDescription(lst, ctx, "STORAGE", QString("MINEXTENTS %1").arg(minExtents));
            addDescription(lst, ctx, "STORAGE", QString("MAXEXTENTS %1").arg(maxExtents));
            addDescription(lst, ctx, "STORAGE", QString("PCTINCREASE %1").arg(pctIncrease));
            addDescription(lst, ctx, QString("MINIMUM EXTENT %1").arg(minExtlen));
            addDescription(lst, ctx, "EXTENT MANAGEMENT DICTIONARY");
        }
        if (connection().version() >= "0800" && (contents != "TEMPORARY" || extentManagement != "LOCAL"))
            addDescription(lst, ctx, logging);
    }
}

void toOracleExtract::describeTrigger(const QString &owner, const QString &name, std::list<QString> &lst)
{
    if (!ext.getCode())
        return ;

    toQList result = toQuery::readQuery(connection(), SQLTriggerInfo, toQueryParams() << name << owner);
    if (result.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find trigger %1.%2").arg(owner).arg(name);
    QString triggerType = (QString)Utils::toShift(result);
    QString event = (QString)Utils::toShift(result);
    QString tableOwner = (QString)Utils::toShift(result);
    QString table = (QString)Utils::toShift(result);
    QString baseType = (QString)Utils::toShift(result);
    QString refNames = (QString)Utils::toShift(result);
    QString description = (QString)Utils::toShift(result);
    QString when = (QString)Utils::toShift(result);
    QString body = (QString)Utils::toShift(result);
    QString status = (QString)Utils::toShift(result);

    QString trgType;
    if (triggerType.indexOf("BEFORE") >= 0)
        trgType = "BEFORE";
    else if (triggerType.indexOf("AFTER") >= 0)
        trgType = "AFTER";
    else if (triggerType.indexOf("INSTEAD OF") >= 0)
        trgType = "INSTEAD OF";

    QString src = trgType;
    src += " ";
    src += event;
    description.replace(QRegularExpression("\nON"), QString("\n ON"));
    int pos = description.indexOf(src);
    QString columns;
    if (pos >= 0)
    {
        pos += src.length();
        int endPos = description.indexOf(" ON ", pos);
        if (endPos >= 0)
        {
            columns = description.right(description.length() - pos);
            columns.truncate(endPos - pos);
        }
    }
    QString schema2 = ext.intSchema(tableOwner, true);
    QString object;
    if (baseType == "TABLE" || baseType == "VIEW")
    {
        object = schema2;
        if (!schema2.isEmpty())
            object += ".";
        object += table;
    }
    else if (baseType == "SCHEMA")
    {
        object = owner;
        if (!owner.isEmpty())
            object += ".";
        object += "SCHEMA";
    }
    else
    {
        object = baseType;
    }

    std::list<QString> ctx;
    ctx.insert(ctx.end(), quote(owner));
    ctx.insert(ctx.end(), "TRIGGER");
    ctx.insert(ctx.end(), quote(name));
    addDescription(lst, ctx);
    QString tmp = triggerType;
    tmp += event;
    tmp += columns;
    addDescription(lst, ctx, tmp);
    tmp = "ON ";
    tmp += object;
    if (!refNames.isEmpty())
    {
        tmp += " ";
        tmp += refNames;
    }
    addDescription(lst, ctx, object);
    if (!when.isEmpty())
        addDescription(lst, ctx, when);
    addDescription(lst, ctx, "BODY", body);
    addDescription(lst, ctx, status);
}

void toOracleExtract::describeType(const QString &owner, const QString &name, std::list<QString> &lst)
{
    describeSource(lst, owner, name, "TYPE");
}

void toOracleExtract::describeUser(const QString &owner, const QString &name, std::list<QString> &lst)
{
    toQList info = toQuery::readQuery(connection(),
                                      SQLUserInfo,
                                      toQueryParams() << name);

    if (info.size() != 4)
        throw qApp->translate("toOracleExtract", "Couldn't find user %1").arg(name);

    QString password = (QString)Utils::toShift(info);
    QString profile = (QString)Utils::toShift(info);
    QString defaultTablespace = (QString)Utils::toShift(info);
    QString temporaryTablespace = (QString)Utils::toShift(info);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), "NONE");
    ctx.insert(ctx.end(), "USER");
    QString nam;
    if (ext.getSchema() != "1" && !ext.getSchema().isEmpty())
        nam = ext.getSchema().toLower();
    else
        nam = quote(name);
    ctx.insert(ctx.end(), nam);

    addDescription(lst, ctx);
    addDescription(lst, ctx, QString("PROFILE %1").arg(quote(profile)));
    addDescription(lst, ctx, QString("DEFAULT TABLESPACE %1").arg(quote(defaultTablespace)));
    addDescription(lst, ctx, QString("TEMPORARY TABLESPACE %1").arg(quote(temporaryTablespace)));

    if (ext.getStorage())
    {
        toQList quota = toQuery::readQuery(connection(), SQLUserQuotas, toQueryParams() << name);
        while (!quota.empty())
        {
            QString siz = (QString)Utils::toShift(quota);
            QString tab = (QString)Utils::toShift(quota);
            addDescription(lst, ctx, "QUOTA", QString("%1 ON %2").
                           arg(siz).
                           arg(quote(tab)));
        }
    }
    describePrivs(lst, ctx, name);
}

void toOracleExtract::describeView(const QString &owner, const QString &name, std::list<QString> &lst)
{
    if (!ext.getCode())
        return ;
    toQList source = toQuery::readQuery(connection(),
                                        SQLViewSource,
                                        toQueryParams() << name << owner);
    if (source.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find view %1.%2").arg(quote(owner)).arg(quote(name));

    std::list<QString> ctx;
    ctx.insert(ctx.end(), quote(owner));
    ctx.insert(ctx.end(), "VIEW");
    ctx.insert(ctx.end(), quote(name));

    addDescription(lst, ctx);
    QString text = (QString)Utils::toShift(source);
    addDescription(lst, ctx, "AS", text);
    describeComments(lst, ctx, owner, name);

    toConnectionSubLoan conn(ext.connection());
    toQuery query(conn, "SELECT * FROM " + quote(owner) + "." + quote(name) + " WHERE NULL = NULL", toQueryParams());

    toQColumnDescriptionList desc = query.describe();
    int num = 1;
    for (toQColumnDescriptionList::iterator i = desc.begin(); i != desc.end(); i++)
    {
        addDescription(lst, ctx, "COLUMN", (*i).Name);
        addDescription(lst, ctx, "COLUMN", (*i).Name, (*i).Datatype);
        addDescription(lst, ctx, "COLUMN", (*i).Name, "ORDER", QString::number(num));
        num++;
    }
}

static toSQL SQLDbmsMetadataGetDdl("toOracleExtract:DbmsMetadataGetDdl",
                                   "SELECT dbms_metadata.get_ddl(:typ<char[100]>,\n"
                                   "                             :nam<char[100]>,\n"
                                   "                             :sch<char[100]>) FROM dual",
                                   "Get object creation ddl using dbms_metadata package");

static toSQL SQLDbmsMetadataGetDir("toOracleExtract:DbmsMetadataGetDir",
                                   "SELECT dbms_metadata.get_ddl(:typ<char[100]>,\n"
                                   "                             :nam<char[100]>) FROM dual",
                                   "Get directory creation ddl using dbms_metadata package");


static toSQL SQLDbmsMetadataUser    ("toOracleExtract:DbmsMetadataUser",
        "SELECT DBMS_METADATA.get_ddl(:typ<char[100]>, :nam<char[31]>) FROM dual",
        "Get DDL for USER"
        );

static toSQL SQLDbmsMetadataRoles    ("toOracleExtract:DbmsMetadataRoles",
        "SELECT DBMS_METADATA.GET_GRANTED_DDL('ROLE_GRANT', :nam<char[31]>) from dual",
        "Get DDL for USER's ROLES");

static toSQL SQLDbmsMetadataObjGrant("toOracleExtract:DbmsMetadataObjGrant",
        "SELECT DBMS_METADATA.GET_GRANTED_DDL('OBJECT_GRANT', :nam<char[31]>) from dual",
        "Get DDL for USER's object GRANTS");

static toSQL SQLDbmsMetadataSysGrant("toOracleExtract:DbmsMetadataSysGrant"
        , "SELECT DBMS_METADATA.GET_GRANTED_DDL('SYSTEM_GRANT', :nam<char[31]>) from dual",
        "Get DDL for USER's system GRANTS");

static toSQL SQLDbmsMetadataQuota   ("toOracleExtract:DbmsMetadataQuota",
        "SELECT DBMS_METADATA.GET_GRANTED_DDL('TABLESPACE_QUOTA', :nam<char[31]>) from dual"
        ,"GET DDL FOR USERS's tablespace QUOTAs");

static toSQL SQLDbmsMetadataDefRole ("toOracleExtract:DbmsMetadataDefRole",
        "SELECT DBMS_METADATA.GET_GRANTED_DDL('DEFAULT_ROLE', :nam<char[31]>) from dual"
        ,"Get DDL for USER's default ROLE(s)");

static toSQL SQLDbmsMetadataSetTransform("toOracleExtract:DbmsMetadataGetSetTransform",
        "begin                                                                                             \n"
        " DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM, 'SQLTERMINATOR',true);        \n"
        " DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM, 'PRETTY', true);              \n"
        " DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM, 'CONSTRAINTS_AS_ALTER',true); \n"
        "end;                                                                                              \n",
        "Configure dbms_metadata package");

QString toOracleExtract::createMetadata( const QString &owner, const QString &name, toExtract::ObjectType typeEnum)
{
    typedef toExtract T;
    toConnectionSubLoan conn(ext.connection());
    QRegularExpression leading_space("^(\n|\r\n|\r| |\t)*");
    QString ret;


    /* TODO
     * DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,�PRETTY�,false);
     * DBMS_METADATA.SET_TRANSFORM_PARAM(DBMS_METADATA.SESSION_TRANSFORM,�SQLTERMINATOR�,true);
     * DBMS_METADATA.SET_TRANSFORM_PARAM (DBMS_METADATA.SESSION_TRANSFORM, �STORAGE�,false);
     * DBMS_METADATA.SET_TRANSFORM_PARAM (DBMS_METADATA.SESSION_TRANSFORM, �SEGMENT_ATTRIBUTES�,false);
     */
    toQuery query1(conn, SQLDbmsMetadataSetTransform, toQueryParams());
    query1.eof();

    QString typeString = ENUM_NAME(toExtract, ObjectType, typeEnum);
    typeString.replace('_', ' ');

    switch (typeEnum)
    {
        case T::DIRECTORY:
        {
            toQuery inf(conn, SQLDbmsMetadataGetDir, toQueryParams() << typeString << name);
            if (inf.eof())
                throw qApp->translate("toOracleExtract", "Couldn't get meta information for %1 %2").arg("DIRECTORY").arg(name);

            if (PROMPT)
                ret = QString("PROMPT CREATE OR REPLACE %1 %2\n\n")
                .arg("DIRECTORY")
                .arg(quote(name));
            toQValue sql = inf.readValue();
            ret += sql.userData().replace(leading_space, "");
            ret += "\n\n\n";
        };
        break;
        case T::USER:
        case T::ROLE:
        {
            try {
                toQuery user(conn, SQLDbmsMetadataUser, toQueryParams() << typeString << name);
                if (user.eof()) throw qApp->translate("toOracleExtract", "Couldn't get meta information for user %1").arg(name);
                if (PROMPT) ret += QString("PROMPT CREATE %1 %2\n\n").arg(typeString).arg(quote(name));
                ret += user.readValue().userData().replace(leading_space, "");
                ret += "\n\n";
            }
            catch (QString const& e) {
                ret += e;
            }

            try {
                toQuery roles(conn, SQLDbmsMetadataRoles, toQueryParams() << name);
                if (roles.eof()) throw qApp->translate("toOracleExtract", "Couldn't get meta information for user %1").arg(name);
                if (PROMPT) ret += QString("PROMPT ROLES\n\n");
                ret += roles.readValue().userData().replace(leading_space, "");
                ret += "\n\n";
            }
            catch (QString const& e) {
                Q_UNUSED(e); // ignore ORA-31608
            }

            try {
                toQuery obj(conn, SQLDbmsMetadataObjGrant, toQueryParams() << name);
                if (obj.eof()) throw qApp->translate("toOracleExtract", "Couldn't get meta information for user %1").arg(name);
                if (PROMPT) ret += QString("PROMPT OBJECT GRANTS\n\n");
                ret += obj.readValue().userData().replace(leading_space, "");
                ret += "\n\n";
            }
            catch (QString const& e) {
                Q_UNUSED(e); // ignore ORA-31608
            }

            try {
                toQuery inf(conn, SQLDbmsMetadataSysGrant, toQueryParams() << name);
                if (inf.eof()) throw qApp->translate("toOracleExtract", "Couldn't get meta information for user %1").arg(name);
                if (PROMPT) ret += QString("PROMPT SYSTEM GRANTS\n\n");
                ret += inf.readValue().userData().replace(leading_space, "");
                ret += "\n\n";
            }
            catch (QString const& e) {
                Q_UNUSED(e); // ignore ORA-31608
            }

            try {
                toQuery quota(conn, SQLDbmsMetadataQuota, toQueryParams() << name);
                if (quota.eof()) throw qApp->translate("toOracleExtract", "Couldn't get meta information for user %1").arg(name);
                if (PROMPT) ret += QString("PROMPT USER QUOTAS\n\n");
                ret += quota.readValue().userData().replace(leading_space, "");
                ret += "\n\n";
            }
            catch (QString const& e) {
                Q_UNUSED(e); // ignore ORA-31608
            }

            if (typeEnum == T::USER)
            {
                try {
                    toQuery defrole(conn, SQLDbmsMetadataDefRole, toQueryParams() << name);
                    if (defrole.eof()) throw qApp->translate("toOracleExtract", "Couldn't get meta information for user %1").arg(name);
                    if (PROMPT) ret += QString("PROMPT USER DEFAULT ROLE(s)\n\n");
                    ret += defrole.readValue().userData().replace(leading_space, "");
                    ret += "\n\n";
                }
                catch (QString const& e) {
                    ret += e;
                }
            }
            ret += "\n";
        } break;
        case T::TABLE_REFERENCES:
            break;
        case T::TABLE_FAMILY:
        {
            toQuery inf(conn, SQLDbmsMetadataGetDdl, toQueryParams() << QString("TABLE") << name << owner);
            if (inf.eof())
                throw qApp->translate("toOracleExtract", "Couldn't get meta information for %1 %2.%3").arg(typeString).arg(owner).arg(name);

            if (PROMPT)
                ret = QString("PROMPT CREATE TABLE %1.%2\n\n")
                .arg(quote(owner))
                .arg(quote(name));
            toQValue sql = inf.readValue();
            ret += sql.userData().replace(leading_space, "");

            // TODO TABLE REFERWENCES

            ret += "\n\n\n";
        } break;
        case T::TABLE_CONTENTS:
        {
            QTextStream stream(&ret, QIODevice::WriteOnly);
            createTableContents(owner, name, stream);
            ret += "\n\n";
        } break;
        default:
        {
            QString type2 = typeString;
            if (typeEnum == T::PACKAGE_BODY)
                type2 = "PACKAGE_BODY";
            else if (typeEnum == T::PACKAGE)
                type2 = "PACKAGE_SPEC";

            toQuery inf(conn, SQLDbmsMetadataGetDdl, toQueryParams() << type2 << name << owner);
            if (inf.eof())
                throw qApp->translate("toOracleExtract", "Couldn't get meta information for %1 %2.%3").arg(typeString).arg(owner).arg(name);

            if (PROMPT)
                ret = QString("PROMPT CREATE OR REPLACE %1 %2.%3\n\n").
                arg(typeString).
                arg(quote(owner)).
                arg(quote(name));
            toQValue sql = inf.readValue();
            ret += sql.userData().replace(leading_space, "");
            ret += "\n\n\n";
        }
    }
    return ret;
} // createMeatada

// Implementation public interface

toOracleExtract::toOracleExtract(toExtract &parent)
    : toExtract::extractor(parent)
{
#if 0
    typedef toExtract T;
    // Supports Oracle
    registerExtract(ORACLE_NAME, QString(), T::NO_TYPE);

    // Register creates
    registerExtract(ORACLE_NAME, "CREATE", T::CONSTRAINT);
    registerExtract(ORACLE_NAME, "CREATE", T::DATABASE_LINK);
    registerExtract(ORACLE_NAME, "CREATE", T::DIRECTORY);
#if 0
    registerExtract(ORACLE_NAME, "CREATE", "EXCHANGE INDEX");
    registerExtract(ORACLE_NAME, "CREATE", "EXCHANGE TABLE");
#endif
    registerExtract(ORACLE_NAME, "CREATE", T::FUNCTION);
    registerExtract(ORACLE_NAME, "CREATE", T::INDEX);
    registerExtract(ORACLE_NAME, "CREATE", T::MATERIALIZED_VIEW);
#if 0
    registerExtract(ORACLE_NAME, "CREATE", T::MATERIALIZED_VIEW_LOG);
#endif
    registerExtract(ORACLE_NAME, "CREATE", T::PACKAGE);
    registerExtract(ORACLE_NAME, "CREATE", T::PACKAGE_BODY);
    registerExtract(ORACLE_NAME, "CREATE", T::PROCEDURE);
    registerExtract(ORACLE_NAME, "CREATE", T::PROFILE);
    registerExtract(ORACLE_NAME, "CREATE", T::ROLE);
#if 0
    registerExtract(ORACLE_NAME, "CREATE", "ROLE GRANTS");
    registerExtract(ORACLE_NAME, "CREATE", "ROLLBACK SEGMENT");
#endif
    registerExtract(ORACLE_NAME, "CREATE", T::SEQUENCE);
#if 0
    registerExtract(ORACLE_NAME, "CREATE", "SNAPSHOT");
    registerExtract(ORACLE_NAME, "CREATE", "SNAPSHOT LOG");
#endif
    registerExtract(ORACLE_NAME, "CREATE", T::SYNONYM);
    registerExtract(ORACLE_NAME, "CREATE", T::TABLE);
    registerExtract(ORACLE_NAME, "CREATE", T::TABLE_FAMILY);
    registerExtract(ORACLE_NAME, "CREATE", T::TABLE_CONTENTS);
    registerExtract(ORACLE_NAME, "CREATE", T::TABLE_REFERENCES);
#if 0
    registerExtract(ORACLE_NAME, "CREATE", T::TABLESPACE);
#endif
    registerExtract(ORACLE_NAME, "CREATE", T::TRIGGER);
    registerExtract(ORACLE_NAME, "CREATE", T::TYPE);
    registerExtract(ORACLE_NAME, "CREATE", T::USER);
#if 0
    registerExtract(ORACLE_NAME, "CREATE", "USER GRANTS");
#endif
    registerExtract(ORACLE_NAME, "CREATE", T::VIEW);

    // Register describes
    registerExtract(ORACLE_NAME, "DESCRIBE", T::CONSTRAINT);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::DATABASE_LINK);
#if 0
    registerExtract(ORACLE_NAME, "DESCRIBE", "EXCHANGE INDEX");
    registerExtract(ORACLE_NAME, "DESCRIBE", "EXCHANGE TABLE");
#endif
    registerExtract(ORACLE_NAME, "DESCRIBE", T::FUNCTION);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::INDEX);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::MATERIALIZED_VIEW);
#if 0
    registerExtract(ORACLE_NAME, "DESCRIBE", "MATERIALIZED VIEW LOG");
#endif
    registerExtract(ORACLE_NAME, "DESCRIBE", T::PACKAGE);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::PACKAGE_BODY);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::PROCEDURE);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::PROFILE);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::ROLE);
#if 0
    registerExtract(ORACLE_NAME, "DESCRIBE", "ROLE GRANTS");
    registerExtract(ORACLE_NAME, "DESCRIBE", "ROLLBACK SEGMENT");
#endif
    registerExtract(ORACLE_NAME, "DESCRIBE", T::SEQUENCE);
#if 0
    registerExtract(ORACLE_NAME, "DESCRIBE", "SNAPSHOT");
    registerExtract(ORACLE_NAME, "DESCRIBE", "SNAPSHOT LOG");
#endif
    registerExtract(ORACLE_NAME, "DESCRIBE", T::SYNONYM);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::TABLE);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::TABLE_FAMILY);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::TABLE_CONTENTS);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::TABLE_REFERENCES);
#if 0
    registerExtract(ORACLE_NAME, "DESCRIBE", "TABLESPACE");
#endif
    registerExtract(ORACLE_NAME, "DESCRIBE", T::TRIGGER);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::TYPE);
    registerExtract(ORACLE_NAME, "DESCRIBE", T::USER);
#if 0
    registerExtract(ORACLE_NAME, "DESCRIBE", "USER GRANTS");
#endif
    registerExtract(ORACLE_NAME, "DESCRIBE", T::VIEW);

    // Register drops
    registerExtract(ORACLE_NAME, "DROP", T::CONSTRAINT);
    registerExtract(ORACLE_NAME, "DROP", T::DATABASE_LINK);
#if 0
    registerExtract(ORACLE_NAME, "DROP", "DIMENSION");
#endif
    registerExtract(ORACLE_NAME, "DROP", T::DIRECTORY);
    registerExtract(ORACLE_NAME, "DROP", T::FUNCTION);
    registerExtract(ORACLE_NAME, "DROP", T::INDEX);
#if 0
    registerExtract(ORACLE_NAME, "DROP", "MATERIALIZED VIEW");
    registerExtract(ORACLE_NAME, "DROP", "MATERIALIZED VIEW LOG");
#endif
    registerExtract(ORACLE_NAME, "DROP", T::PACKAGE);
    registerExtract(ORACLE_NAME, "DROP", T::PROCEDURE);
    registerExtract(ORACLE_NAME, "DROP", T::PROFILE);
    registerExtract(ORACLE_NAME, "DROP", T::ROLE);
#if 0
    registerExtract(ORACLE_NAME, "DROP", "ROLLBACK SEGMENT");
#endif
    registerExtract(ORACLE_NAME, "DROP", T::SEQUENCE);
#if 0
    registerExtract(ORACLE_NAME, "DROP", "SNAPSHOT");
    registerExtract(ORACLE_NAME, "DROP", "SNAPSHOT LOG");
#endif
    registerExtract(ORACLE_NAME, "DROP", T::SYNONYM);
    registerExtract(ORACLE_NAME, "DROP", T::TABLE);
#if 0
    registerExtract(ORACLE_NAME, "DROP", "TABLESPACE");
#endif
    registerExtract(ORACLE_NAME, "DROP", T::TRIGGER);
    registerExtract(ORACLE_NAME, "DROP", T::TYPE);
    registerExtract(ORACLE_NAME, "DROP", T::USER);
    registerExtract(ORACLE_NAME, "DROP", T::VIEW);

    // Register migrates
    registerExtract(ORACLE_NAME, "MIGRATE", T::TABLE);
#endif
    toExtract::datatype * d;
//  d = new toExtract::datatype("VARCHAR", 2000);
//  oracle_datatypes.insert(oracle_datatypes.end(), *d);
    d = new toExtract::datatype("VARCHAR2", 4000);
    oracle_datatypes.insert(oracle_datatypes.end(), *d);
    d = new toExtract::datatype("NUMBER", 32, 32);
    oracle_datatypes.insert(oracle_datatypes.end(), *d);
    d = new toExtract::datatype("DATE");
    oracle_datatypes.insert(oracle_datatypes.end(), *d);
    d = new toExtract::datatype("INTEGER");
    oracle_datatypes.insert(oracle_datatypes.end(), *d);
    d = new toExtract::datatype("LONG");
    oracle_datatypes.insert(oracle_datatypes.end(), *d);
    d = new toExtract::datatype("CLOB");
    oracle_datatypes.insert(oracle_datatypes.end(), *d);
}

std::list<toExtract::datatype> toOracleExtract::datatypes() const
{
    return oracle_datatypes;
}

toOracleExtract::~toOracleExtract()
{
}

static toSQL SQLSetSizing("toOracleExtract:SetSizing",
                          "SELECT value FROM v$parameter WHERE name = 'db_block_size'",
                          "Get information about block sizes from alloced space, same columns");

static toSQL SQLSetSizingFallback("toOracleExtract:SetSizingFallback",
                                  "SELECT bytes/blocks/1024   AS block_size\n"
                                  "  FROM user_free_space\n"
                                  " WHERE bytes  IS NOT NULL\n"
                                  "   AND blocks IS NOT NULL\n"
                                  "   AND ROWNUM < 2",
                                  "Get information about block sizes, same columns");

static toSQL SQLSetSizingFallback2("toOracleExtract:SetSizingFallback2",
                                   "SELECT bytes/blocks/1024   AS block_size\n"
                                   "  FROM user_segments\n"
                                   " WHERE bytes  IS NOT NULL\n"
                                   "   AND blocks IS NOT NULL\n"
                                   "   AND ROWNUM < 2",
                                   "Get information about block sizes, same columns");

void toOracleExtract::initialize()
{
    QString DbaSegments;
    toConnectionSubLoan conn(connection());
    try
    {
        toConnectionSubLoan conn(ext.connection());
        toQuery q(conn, "select * from sys.dba_segments where null=null", toQueryParams());
        q.eof();
        DbaSegments = "sys.dba_segments";
    }
    catch (QString const&)
    {
        DbaSegments = QString("(select '%1' owner,user_segments.* from sys.user_segments)").arg(connection().user().toUpper());
    }
    ext.setState("Segments", DbaSegments);

    try
    {
        toQList ret = toQuery::readQuery(ext.connection(), SQLSetSizing, toQueryParams());
        ext.setBlockSize(Utils::toShift(ret).toInt());
    }
    catch (...)
    {
        toQList ret = toQuery::readQuery(ext.connection(), SQLSetSizingFallback, toQueryParams());
        if (ret.empty())
            ret = toQuery::readQuery(ext.connection(), SQLSetSizingFallback2, toQueryParams());
        ext.setBlockSize(Utils::toShift(ret).toInt());
    }
}

void toOracleExtract::create(
                             QTextStream &stream,
                             toExtract::ObjectType type,
                             const QString &schema,
                             const QString &owner,
                             const QString &name)
{
    using namespace ToConfiguration;
    typedef toExtract T;

    clearFlags();

    if (toConfigurationNewSingle::Instance().option(Oracle::UseDbmsMetadataBool).toBool())
    {
        stream << createMetadata(owner, name, type);
        return;
    }
	switch (type)
	{
	case T::CONSTRAINT:        stream << createConstraint(owner, name);  break;
	case T::DATABASE_LINK:     stream << createDBLink(owner, name);      break;
	case T::DIRECTORY:         stream << createDirectory(owner, name);   break;
#if 0
	else if (type == "EXCHANGE INDEX") stream << createExchangeIndex(schema, owner, name);
	else if (type == "EXCHANGE TABLE") stream << createExchangeTable(schema, owner, name);
#endif
	case T::FUNCTION:          stream << createFunction(owner, name);    break;
	case T::INDEX:             stream << createIndex(owner, name);       break;
	case T::MATERIALIZED_VIEW: stream << createMaterializedView(owner, name); break;
#if 0
	else if (type == "MATERIALIZED VIEW LOG")
		stream << createMaterializedViewLog(schema, owner, name);
#endif
	case T::PACKAGE:           stream << createPackage(owner, name);     break;
	case T::PACKAGE_BODY:      stream << createPackageBody(owner, name); break;
	case T::PROCEDURE:         stream << createProcedure(owner, name);   break;
	case T::PROFILE:           stream << createProfile(name);     break;
	case T::ROLE:              stream << createRole(name);        break;
#if 0
	case T::ROLE GRANTS : stream << grantedPrivs(ext, quote(name), name, 6);     break;
	case T::ROLLBACK SEGMENT : stream << createRollbackSegment(schema, owner, name); break;
#endif
	case T::SEQUENCE:          stream << createSequence(owner, name); break;
#if 0
	else if (type == "SNAPSHOT")        stream << createSnapshot(schema, owner, name); break;
	else if (type == "SNAPSHOT LOG")        stream << createSnapshotLog(schema, owner, name); break;
#endif
	case T::SYNONYM:           stream << createSynonym(owner, name); break;
	case T::TABLE:             stream << createTable(owner, name); break;
	case T::TABLE_FAMILY:      stream << createTableFamily(owner, name); break;
	case T::TABLE_CONTENTS:    createTableContents(owner, name, stream); break;
	case T::TABLE_REFERENCES:  stream << createTableReferences(owner, name); break;
#if 0
	case T::TABLESPACE:        stream << createTablespace(name); break;
#endif
	case T::TRIGGER:           stream << createTrigger(owner, name); break;
	case T::TYPE:              stream << createType(owner, name); break;
	case T::USER:              stream << createUser(name); break;
#if 0
	case T::USER_GRANTS:
	{
		QString nam;
		if (ext.getSchema() != "1" && !ext.getSchema().isEmpty())
			nam = ext.getSchema().toLower();
		else
			nam = quote(name);
		stream << grantedPrivs(ext, nam, name, 4);
	} break;
#endif
	case T::VIEW:              stream << createView(owner, name); break;
#if 0
	else
	{
		throw qApp->translate("toOracleExtract", "Invalid type %1 to create").arg(type);
	}
#endif
	}
}

void toOracleExtract::describe(
                               std::list<QString> &lst,
                               toExtract::ObjectType type,
                               const QString &schema,
                               const QString &owner,
                               const QString &name)
{
    typedef toExtract T;
    clearFlags();

    switch(type)
    {
        case T::CONSTRAINT:       describeConstraint(owner, name, lst); break;
        case T::DATABASE_LINK:    describeDBLink(owner, name, lst); break;
#if 0
        case T::EXCHANGE INDEX:   describeExchangeIndex(owner, name, lst); break;
        case T::EXCHANGE TABLE:   describeExchangeTable(owner, name, lst); break;
#endif
        case T::FUNCTION:         describeFunction(owner, name, lst); break;
        case T::INDEX:            describeIndex(owner, name, lst); break;
#if 0
        case T::MATERIALIZED VIEW:        describeMaterializedView(owner, name, lst); break;
        case T::MATERIALIZED VIEW LOG:        describeMaterializedViewLog(owner, name, lst); break;
#endif
        case T::PACKAGE:          describePackage(owner, name, lst); break;
        case T::PACKAGE_BODY:     describePackageBody(owner, name, lst); break;
        case T::PROCEDURE:        describeProcedure(owner, name, lst); break;
        case T::PROFILE:          describeProfile(name, lst); break;
        case T::ROLE:             describeRole(name, lst); break;
#if 0
        case T::ROLE_GRANTS:
        {
            // A nop, everything is done in describe role
        }
        case T::ROLLBACK SEGMENT:        describeRollbackSegment(owner, name, lst); break;
#endif
        case T::SEQUENCE:         describeSequence(owner, name, lst); break;
#if 0
        case T::SNAPSHOT:         describeSnapshot(owner, name, lst); break;
        case T::SNAPSHOT LOG:     describeSnapshotLog(owner, name, lst); break;
#endif
        case T::SYNONYM:          describeSynonym(owner, name, lst); break;
        case T::TABLE:            describeTable(owner, name, lst); break;
        case T::TABLE_FAMILY:     describeTableFamily(owner, name, lst); break;
        case T::TABLE_REFERENCES: describeTableReferences(owner, name, lst); break;
        case T::TABLE_CONTENTS:
        {
            // A nop, nothing is described of contents
        } break;
#if 0
        case T::TABLESPACE:       describeTablespace(owner, name, lst); break;
#endif
        case T::TRIGGER:          describeTrigger(owner, name, lst); break;
        case T::TYPE:             describeType(owner, name, lst); break;
        case T::USER:             describeUser(owner, name, lst); break;
#if 0
        case T::USER_GRANTS:
        {
            // A nop, everything is done in describe user
        } break;
#endif
        case T::VIEW:             describeView(owner, name, lst); break;
#if 0
        else
        {
            throw qApp->translate("toOracleExtract", "Invalid type %1 to describe").arg(type);
        }
#endif
    }
}

Util::RegisterInFactory<toOracleExtract, ExtractorFactorySing> regToOracleExtract("Oracle");
