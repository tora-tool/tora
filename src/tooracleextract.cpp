
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

#include "toconf.h"
#include "toconnection.h"
#include "toextract.h"
#include "tosql.h"
#include "totool.h"

#include <qapplication.h>
#include <qregexp.h>

// Some convenient defines

static const QString ORACLE_NAME("Oracle");
#define CONNECTION (ext.connection())
#define QUOTE(x) (ext.connection().quote(x))
#define PROMPT (ext.getPrompt())

#define addDescription toExtract::addDescription

class toOracleExtract : public toExtract::extractor
{

    // Misc utility functions

    void clearFlags(toExtract &ext) const;
    QString displaySource(toExtract &ext, const QString &schema, const QString &owner,
                          const QString &name, const QString &type, bool describe) const;
    QString keyColumns(toExtract &ext, const QString &owner, const QString &name,
                       const QString &type, const QString &table) const;
    QString partitionKeyColumns(toExtract &ext, const QString &owner,
                                const QString &name, const QString &type) const;
    static QString prepareDB(const QString &data);
    QString segments(toExtract &ext) const;
    QString segments(toExtract &ext, const toSQL &sql) const;
    QString subPartitionKeyColumns(toExtract &ext, const QString &owner,
                                   const QString &name, const QString &type) const;

    // Create utility functions

    QString constraintColumns(toExtract &ext, const QString &owner, const QString &name) const;
    QString createComments(toExtract &ext, const QString &schema,
                           const QString &owner, const QString &name) const;
    QString createContextPrefs(toExtract &ext, const QString &schema, const QString &owner,
                               const QString &name, const QString &sql) const;
    QString createIOT(toExtract &ext, const QString &schema, const QString &owner,
                      const QString &name) const;
    QString createMView(toExtract &ext, const QString &schema, const QString &owner,
                        const QString &name, const QString &type) const;
    QString createMViewIndex(toExtract &ext, const QString &schema, const QString &owner,
                             const QString &name) const;
    QString createMViewLog(toExtract &ext, const QString &schema, const QString &owner,
                           const QString &name, const QString &type) const;
    QString createMViewTable(toExtract &ext, const QString &schema, const QString &owner,
                             const QString &name) const;
    QString createPartitionedIOT(toExtract &ext, const QString &schema,
                                 const QString &owner, const QString &name) const;
    QString createPartitionedIndex(toExtract &ext, const QString &schema, const QString &owner,
                                   const QString &name, const QString &sql) const;
    QString createPartitionedTable(toExtract &ext, const QString &schema,
                                   const QString &owner, const QString &name) const;
    QString createTableText(toExtract &ext, toQList &result, const QString &schema,
                            const QString &owner, const QString &name) const;
    QString grantedPrivs(toExtract &ext, const QString &dest, const QString &name, int typ) const;
    QString indexColumns(toExtract &ext, const QString &indent, const QString &owner,
                         const QString &name) const;
    QString rangePartitions(toExtract &ext, const QString &owner, const QString &name,
                            const QString &subPartitionType, const QString &caller) const;
    QString segmentAttributes(toExtract &ext, toQList &result) const;
    QString tableColumns(toExtract &ext, const QString &owner, const QString &name) const;

    // Describe utility functions
    void describeAttributes(toExtract &ext, std::list<QString> &dsp, std::list<QString> &ctx,
                            toQList &result) const;
    void describeComments(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                          const QString &owner, const QString &name) const;
    void describePrivs(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                       const QString &name) const;
    void describeIOT(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                     const QString &schema, const QString &owner, const QString &name) const;
    void describeIndexColumns(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                              const QString &owner, const QString &name) const;
    void describeMView(toExtract &ext, std::list<QString> &lst, const QString &schema,
                       const QString &owner, const QString &name, const QString &type) const;
    void describeMViewIndex(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                            const QString &schema, const QString &owner, const QString &name) const;
    void describeMViewLog(toExtract &ext, std::list<QString> &lst, const QString &schema,
                          const QString &owner, const QString &name, const QString &type) const;
    void describeMViewTable(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                            const QString &schema, const QString &owner, const QString &name) const;
    void describePartitions(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                            const QString &owner, const QString &name,
                            const QString &subPartitionType, const QString &caller) const;
    void describePartitionedIOT(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                                const QString &schema, const QString &owner,
                                const QString &name) const;
    void describePartitionedIndex(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                                  const QString &schema, const QString &owner,
                                  const QString &name) const;
    void describePartitionedTable(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                                  const QString &schema, const QString &owner,
                                  const QString &name) const;
    void describeSource(toExtract &ext, std::list<QString> &lst, const QString &schema,
                        const QString &owner, const QString &name, const QString &type) const;
    void describeTableColumns(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                              const QString &owner, const QString &name) const;
    void describeTableText(toExtract &ext, std::list<QString> &lst, std::list<QString> &ctx,
                           toQList &result, const QString &schema, const QString &owner,
                           const QString &name) const;

    // Migrate utility functions

    QString migrateAttributes(std::list<QString> &desc,
                              std::list<QString>::iterator i,
                              int level,
                              const QString &indent) const;
    QString migrateIndexColumns(std::list<QString> &destin,
                                std::list<QString>::iterator i,
                                const QString &context) const;
    QString migratePartitions(std::list<QString> &desc,
                              std::list<QString>::iterator &i,
                              int level,
                              const QString &indent) const;
    QString migratePrivs(toExtract &ext,
                         std::list<QString> &source,
                         std::list<QString> &destin,
                         const QString &type) const;
    QString migrateSource(toExtract &ext,
                          std::list<QString> &source,
                          std::list<QString> &destin,
                          const QString &type) const;

    // Create functions

    QString createConstraint(toExtract &ext, const QString &schema, const QString &owner,
                             const QString &name) const;
    QString createDBLink(toExtract &ext, const QString &schema, const QString &owner,
                         const QString &name) const;
    QString createExchangeIndex(toExtract &ext, const QString &schema, const QString &owner,
                                const QString &name) const;
    QString createExchangeTable(toExtract &ext, const QString &schema, const QString &owner,
                                const QString &name) const;
    QString createFunction(toExtract &ext, const QString &schema, const QString &owner,
                           const QString &name) const;
    QString createIndex(toExtract &ext, const QString &schema, const QString &owner,
                        const QString &name) const;
    QString createMaterializedView(toExtract &ext, const QString &schema, const QString &owner,
                                   const QString &name) const;
    QString createMaterializedViewLog(toExtract &ext, const QString &schema, const QString &owner,
                                      const QString &name) const;
    QString createPackage(toExtract &ext, const QString &schema, const QString &owner,
                          const QString &name) const;
    QString createPackageBody(toExtract &ext, const QString &schema, const QString &owner,
                              const QString &name) const;
    QString createProcedure(toExtract &ext, const QString &schema, const QString &owner,
                            const QString &name) const;
    QString createProfile(toExtract &ext, const QString &schema, const QString &owner,
                          const QString &name) const;
    QString createRole(toExtract &ext, const QString &schema, const QString &owner,
                       const QString &name) const;
    QString createRollbackSegment(toExtract &ext, const QString &schema, const QString &owner,
                                  const QString &name) const;
    QString createSequence(toExtract &ext, const QString &schema, const QString &owner,
                           const QString &name) const;
    QString createSnapshot(toExtract &ext, const QString &schema, const QString &owner,
                           const QString &name) const;
    QString createSnapshotLog(toExtract &ext, const QString &schema, const QString &owner,
                              const QString &name) const;
    QString createSynonym(toExtract &ext, const QString &schema, const QString &owner,
                          const QString &name) const;
    QString createTable(toExtract &ext, const QString &schema, const QString &owner,
                        const QString &name) const;
    QString createTableFamily(toExtract &ext, const QString &schema, const QString &owner,
                              const QString &name) const;
    void createTableContents(toExtract &ext, QTextStream &stream,
                             const QString &schema, const QString &owner, const QString &name) const;
    QString createTableReferences(toExtract &ext, const QString &schema, const QString &owner,
                                  const QString &name) const;
    QString createTablespace(toExtract &ext, const QString &schema, const QString &owner,
                             const QString &name) const;
    QString createTrigger(toExtract &ext, const QString &schema, const QString &owner,
                          const QString &name) const;
    QString createType(toExtract &ext, const QString &schema, const QString &owner,
                       const QString &name) const;
    QString createUser(toExtract &ext, const QString &schema, const QString &owner,
                       const QString &name) const;
    QString createView(toExtract &ext, const QString &schema, const QString &owner,
                       const QString &name) const;

    // Describe functions
    void describeConstraint(toExtract &ext, std::list<QString> &lst, const QString &schema,
                            const QString &owner, const QString &name) const;
    void describeDBLink(toExtract &ext, std::list<QString> &lst, const QString &schema,
                        const QString &owner, const QString &name) const;
    void describeExchangeIndex(toExtract &ext, std::list<QString> &lst, const QString &schema,
                               const QString &owner, const QString &name) const;
    void describeExchangeTable(toExtract &ext, std::list<QString> &lst, const QString &schema,
                               const QString &owner, const QString &name) const;
    void describeFunction(toExtract &ext, std::list<QString> &lst, const QString &schema,
                          const QString &owner, const QString &name) const;
    void describeIndex(toExtract &ext, std::list<QString> &lst, const QString &schema,
                       const QString &owner, const QString &name) const;
    void describeMaterializedView(toExtract &ext, std::list<QString> &lst, const QString &schema,
                                  const QString &owner, const QString &name) const;
    void describeMaterializedViewLog(toExtract &ext, std::list<QString> &lst, const QString &schema,
                                     const QString &owner, const QString &name) const;
    void describePackage(toExtract &ext, std::list<QString> &lst, const QString &schema,
                         const QString &owner, const QString &name) const;
    void describePackageBody(toExtract &ext, std::list<QString> &lst, const QString &schema,
                             const QString &owner, const QString &name) const;
    void describeProcedure(toExtract &ext, std::list<QString> &lst, const QString &schema,
                           const QString &owner, const QString &name) const;
    void describeProfile(toExtract &ext, std::list<QString> &lst, const QString &schema,
                         const QString &owner, const QString &name) const;
    void describeRole(toExtract &ext, std::list<QString> &lst, const QString &schema,
                      const QString &owner, const QString &name) const;
    void describeRollbackSegment(toExtract &ext, std::list<QString> &lst, const QString &schema,
                                 const QString &owner, const QString &name) const;
    void describeSequence(toExtract &ext, std::list<QString> &lst, const QString &schema,
                          const QString &owner, const QString &name) const;
    void describeSnapshot(toExtract &ext, std::list<QString> &lst, const QString &schema,
                          const QString &owner, const QString &name) const;
    void describeSnapshotLog(toExtract &ext, std::list<QString> &lst, const QString &schema,
                             const QString &owner, const QString &name) const;
    void describeSynonym(toExtract &ext, std::list<QString> &lst, const QString &schema,
                         const QString &owner, const QString &name) const;
    void describeTable(toExtract &ext, std::list<QString> &lst, const QString &schema,
                       const QString &owner, const QString &name) const;
    void describeTableFamily(toExtract &ext, std::list<QString> &lst, const QString &schema,
                             const QString &owner, const QString &name) const;
    void describeTableReferences(toExtract &ext, std::list<QString> &lst, const QString &schema,
                                 const QString &owner, const QString &name) const;
    void describeTablespace(toExtract &ext, std::list<QString> &lst, const QString &schema,
                            const QString &owner, const QString &name) const;
    void describeTrigger(toExtract &ext, std::list<QString> &lst, const QString &schema,
                         const QString &owner, const QString &name) const;
    void describeType(toExtract &ext, std::list<QString> &lst, const QString &schema,
                      const QString &owner, const QString &name) const;
    void describeUser(toExtract &ext, std::list<QString> &lst, const QString &schema,
                      const QString &owner, const QString &name) const;
    void describeView(toExtract &ext, std::list<QString> &lst, const QString &schema,
                      const QString &owner, const QString &name) const;

    // Drop functions
    QString dropConstraint(toExtract &ext, const QString &schema, const QString &owner,
                           const QString &type, const QString &name) const;
    QString dropDatabaseLink(toExtract &ext, const QString &schema, const QString &owner,
                             const QString &type, const QString &name) const;
    QString dropMViewLog(toExtract &ext, const QString &schema, const QString &owner,
                         const QString &type, const QString &name) const;
    QString dropObject(toExtract &ext, const QString &schema, const QString &owner,
                       const QString &type, const QString &name) const;
    QString dropProfile(toExtract &ext, const QString &schema, const QString &owner,
                        const QString &type, const QString &name) const;
    QString dropSchemaObject(toExtract &ext, const QString &schema, const QString &owner,
                             const QString &type, const QString &name) const;
    QString dropSynonym(toExtract &ext, const QString &schema, const QString &owner,
                        const QString &type, const QString &name) const;
    QString dropTable(toExtract &ext, const QString &schema, const QString &owner,
                      const QString &type, const QString &name) const;
    QString dropTablespace(toExtract &ext, const QString &schema, const QString &owner,
                           const QString &type, const QString &name) const;
    QString dropUser(toExtract &ext, const QString &schema, const QString &owner,
                     const QString &type, const QString &name) const;

    // Migrate functions
    QString migrateConstraint(toExtract &ext, std::list<QString> &source,
                              std::list<QString> &destin) const;
    QString migrateDBLink(toExtract &ext, std::list<QString> &source,
                          std::list<QString> &destin) const;
    QString migrateFunction(toExtract &ext, std::list<QString> &source,
                            std::list<QString> &destin) const;
    QString migrateIndex(toExtract &ext, std::list<QString> &source,
                         std::list<QString> &destin) const;
    QString migratePackage(toExtract &ext, std::list<QString> &source,
                           std::list<QString> &destin) const;
    QString migratePackageBody(toExtract &ext, std::list<QString> &source,
                               std::list<QString> &destin) const;
    QString migrateProcedure(toExtract &ext, std::list<QString> &source,
                             std::list<QString> &destin) const;
    QString migrateRole(toExtract &ext, std::list<QString> &source,
                        std::list<QString> &destin) const;
    QString migrateSequence(toExtract &ext, std::list<QString> &source,
                            std::list<QString> &destin) const;
public:
    // Public interface

    toOracleExtract();
    virtual ~toOracleExtract();

    virtual void initialize(toExtract &ext) const;
    virtual void create(toExtract &ext, QTextStream &stream,
                        const QString &type, const QString &schema,
                        const QString &owner, const QString &name) const;
    virtual void describe(toExtract &ext, std::list<QString> &lst, const QString &type,
                          const QString &schema, const QString &owner, const QString &name) const;
    virtual void drop(toExtract &ext, QTextStream &stream, const QString &type, const QString &schema,
                      const QString &owner, const QString &name) const;
};

static toOracleExtract OracleExtractor;

// Implementation misc

void toOracleExtract::clearFlags(toExtract &ext) const
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

QString toOracleExtract::displaySource(toExtract &ext,
                                       const QString &schema,
                                       const QString &owner,
                                       const QString &name,
                                       const QString &type,
                                       bool describe) const
{
    if (!ext.getCode())
        return "";

    QString re("^\\s*");
    re += type;
    re += "\\s+";
    QRegExp StripType(re, Qt::CaseInsensitive);

    toQuery inf(CONNECTION, SQLDisplaySource, type, name, owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Couldn't find source for of %1.%2").arg(owner).arg(name);

    QString ret;
    if (PROMPT && !describe)
        ret = QString("PROMPT CREATE OR REPLACE %1 %2%3\n\n").
              arg(type).
              arg(schema).
              arg(QUOTE(name));
    bool first = true;
    while (!inf.eof())
    {
        QString line = inf.readValue();
        if (first)
        {
            int len;
            int pos = StripType.indexIn(line, 0);
            len = StripType.matchedLength();
            if (pos != 0)
                throw qApp->translate("toOracleExtract", "Displaying source of wrong type for %1. Got %2 expected 0.")
                .arg(type).arg(pos);
            QString tmp = QString("CREATE OR REPLACE %1 ")
                          .arg(type);
            if (!describe)
                tmp += schema;
            tmp += line.mid(len);
            line = tmp;

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

QString toOracleExtract::keyColumns(toExtract &ext,
                                    const QString &owner,
                                    const QString &name,
                                    const QString &type,
                                    const QString &table) const
{
    toQuery inf(CONNECTION, toSQL::string(SQLKeyColumns, CONNECTION).arg(table),
                name, owner, type);
    QString ret;
    bool first = true;
    while (!inf.eof())
    {
        if (first)
            first = false;
        else
            ret += "\n  , ";
        ret += QUOTE(inf.readValue());
    }
    return ret;
}

QString toOracleExtract::partitionKeyColumns(toExtract &ext,
        const QString &owner,
        const QString &name,
        const QString &type) const
{
    return keyColumns(ext, owner, name, type, "PART");
}

QString toOracleExtract::prepareDB(const QString &db)
{
    static QRegExp quote("'");
    QString ret = db;
    ret.replace(quote, "''");
    return ret;
}

QString toOracleExtract::segments(toExtract &ext) const
{
    return ext.state("Segments").toString();
}

QString toOracleExtract::segments(toExtract &ext, const toSQL &sql) const
{
    return toSQL::string(sql, CONNECTION).arg(segments(ext));
}

QString toOracleExtract::subPartitionKeyColumns(toExtract &ext,
        const QString &owner,
        const QString &name,
        const QString &type) const
{
    return keyColumns(ext, owner, name, type, "SUBPART");
}

// Implementation create utility functions

static toSQL SQLConstraintCols("toOracleExtract:ConstraintCols",
                               "SELECT column_name\n"
                               "  FROM sys.all_cons_columns\n"
                               " WHERE owner = :own<char[100]>\n"
                               "   AND constraint_name = :con<char[100]>\n"
                               " ORDER BY position",
                               "List columns in a constraint, must have same binds and columns");

QString toOracleExtract::constraintColumns(toExtract &ext,
        const QString &owner,
        const QString &name) const
{
    toQuery query(CONNECTION, SQLConstraintCols, owner, name);

    QString ret = "(\n    ";
    bool first = true;
    while (!query.eof())
    {
        if (first)
            first = false;
        else
            ret += ",\n    ";
        ret += QUOTE(query.readValue());
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

QString toOracleExtract::createComments(toExtract &ext,
                                        const QString &schema,
                                        const QString &owner,
                                        const QString &name) const
{
    QString ret;
    if (ext.getComments())
    {
        QString sql;
        toQuery inf(CONNECTION, SQLTableComments, name, owner);
        while (!inf.eof())
        {
            sql = QString("COMMENT ON TABLE %1%2 IS '%3'").
                  arg(schema).
                  arg(QUOTE(name)).
                  arg(prepareDB(inf.readValue()));
            if (PROMPT)
            {
                ret += "PROMPT ";
                ret += sql;
                ret += "\n\n";
            }
            ret += sql;
            ret += ";\n\n";
        }
        toQuery col(CONNECTION, SQLColumnComments, name, owner);
        while (!col.eof())
        {
            QString column = col.readValue();
            sql = QString("COMMENT ON COLUMN %1%2.%3 IS '%4'").
                  arg(schema).
                  arg(QUOTE(name)).
                  arg(QUOTE(column)).
                  arg(prepareDB(col.readValue()));
            if (PROMPT)
            {
                ret += "PROMPT ";
                ret += sql;
                ret += "\n\n";
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

QString toOracleExtract::createContextPrefs(toExtract &ext,
        const QString &,
        const QString &owner,
        const QString &name,
        const QString &sql) const
{
    QString prefs = "";
    if (PROMPT)
        prefs += "PROMPT CREATE CONTEXT PREFERENCES\n\n";
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
        resultset = toQuery::readQueryNull(CONNECTION,
                                           "SELECT * FROM ctxsys.ctx_indexes "
                                           "WHERE idx_owner = 'DUMMY'");
    }
    catch (...)
    {
        isDBA = false;
    }
    ql << "DATASTORE" << "FILTER" << "LEXER" << "WORDLIST" << "STORAGE";
    // Lets start with discovering the preferences
    QStringList::Iterator it;
    for (it = ql.begin();it != ql.end();++it)
    {
        if (isDBA)
            resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfoDBA,
                                               owner, name, *it);
        else
            resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfo,
                                               name, *it);
        if (!resultset.empty())
        {
            first = true;
            while (!resultset.empty())
            {
                QString pre_class = toShift(resultset);
                QString pre_obj = toShift(resultset);
                QString pre_attr = toShift(resultset);
                QString pre_val = toShift(resultset);
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
                resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfoNoAttrDBA,
                                                   owner, name, *it);
            else
                resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfoNoAttr,
                                                   name, *it);
            if (!resultset.empty())
            {
                QString pre_obj = toShift(resultset);
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
        resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfoDBA,
                                           owner, name, "STOPLIST");
    else
        resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfo,
                                           name, "STOPLIST");
    pre_name = "";
    while (!resultset.empty())
    {
        QString pre_class = toShift(resultset);
        QString pre_obj = toShift(resultset);
        QString pre_attr = toShift(resultset);
        QString pre_val = toShift(resultset);
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
            resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfoNoAttrDBA,
                                               owner, name, "STOPLIST");
        else
            resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfoNoAttr,
                                               name, "STOPLIST");

        if (!resultset.empty())
        {
            QString pre_obj = toShift(resultset);
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
        resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfoDBA,
                                           owner, name, "SECTION_GROUP");
    else
        resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfo,
                                           name, "SECTION_GROUP");
    pre_name = "";
    while (!resultset.empty())
    {
        QString pre_class = toShift(resultset);
        QString pre_obj = toShift(resultset);
        QString pre_attr = toShift(resultset);
        QString pre_val = toShift(resultset);
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
            tmp = QString("  CTX_DDL.ADD_ZONE_SECTION('%1', '%2', '%3');\n")
                  .arg(pre_name).arg(pre_val1).arg(pre_val2);
        else if (pre_attr == "FIELD")
            tmp = QString("  CTX_DDL.ADD_FIELD_SECTION('%1', '%2', '%3', %4);\n")
                  .arg(pre_name).arg(pre_val1).arg(pre_val2).arg(pre_val4);
        else if (pre_attr == "SPECIAL")
            tmp = QString("  CTX_DDL.ADD_SPECIAL_SECTION('%1', '%2');\n")
                  .arg(pre_name).arg(pre_val1);
        prefs += tmp;
    }
    if (pre_name != "")
        prefs += "END;\n\n";
    else
    {
        if (isDBA)
            resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfoNoAttrDBA,
                                               owner, name, "SECTION_GROUP");
        else
            resultset = toQuery::readQueryNull(CONNECTION, SQLContextInfoNoAttr,
                                               name, "SECTION_GROUP");
        if (!resultset.empty())
        {
            QString pre_obj = toShift(resultset);
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
    if (CONNECTION.version() >= "080106")
    {
        if (isDBA)
            resultset = toQuery::readQueryNull(CONNECTION, SQLContextColumnDBA,
                                               owner, name);
        else
            resultset = toQuery::readQueryNull(CONNECTION, SQLContextColumn,
                                               name);
        if (!resultset.empty())
        {
            toQValue vlang = toShift(resultset);
            toQValue vfrmt = toShift(resultset);
            toQValue vcset = toShift(resultset);
            if (!vlang.isNull())
            {
                tmp = QString("             LANGUAGE COLUMN %1\n").arg(vlang);
                parameters += tmp;
            }
            if (!vfrmt.isNull())
            {
                tmp = QString("             FORMAT COLUMN %1\n").arg(vfrmt);
                parameters += tmp;
            }
            if (!vcset.isNull())
            {
                tmp = QString("             CHARSET COLUMN %1\n").arg(vcset);
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

QString toOracleExtract::createIOT(toExtract &ext, const QString &schema, const QString &owner,
                                   const QString &name) const
{
    toQList storage = toQuery::readQueryNull(CONNECTION, SQLIOTInfo, name, owner);

    QString ret = createTableText(ext, storage, schema, owner, name);
    if (ext.getStorage())
    {
        toQList overflow = toQuery::readQueryNull(CONNECTION, segments(ext, SQLOverflowInfo), name, owner);
        if (overflow.size() == 18)
        {
            ret += "OVERFLOW\n";
            ret += segmentAttributes(ext, overflow);
        }
    }
    ret += ";\n\n";
    ret += createComments(ext, schema, owner, name);
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

QString toOracleExtract::createTableText(toExtract &ext,
        toQList &result,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    QString monitoring = toShift(result);
    QString table = toShift(result);
    QString degree = toShift(result);
    QString instances = toShift(result);
    QString organization = toShift(result);

    QString ret;
    if (PROMPT)
        ret = QString("PROMPT CREATE TABLE %1%2\n\n").arg(schema).arg(QUOTE(name));
    ret += QString("CREATE TABLE %1%2\n(\n    ").arg(schema).arg(QUOTE(name));
    ret += tableColumns(ext, owner, name);
    if (organization == "INDEX" && ext.getStorage())
    {
        toQList res = toQuery::readQueryNull(CONNECTION, SQLPrimaryKey, name, owner);
        if (res.size() != 2)
            throw qApp->translate("toOracleExtract", "Couldn't find primary key of %1.%2").arg(owner).arg(name);
        QString primary = *(res.begin());
        QString tablespace = *(res.rbegin());
        ret += QString("  , CONSTRAINT %1 PRIMARY KEY\n").arg(QUOTE(primary));
        ret += indexColumns(ext, "      ", owner, primary);
        ret += QString("      USING TABLESPACE %2\n").arg(QUOTE(tablespace));
    }
    ret += ")\n";
    if (CONNECTION.version() >= "0800" && ext.getStorage() && ! organization.isEmpty() )
    {
        ret += "ORGANIZATION        ";
        ret += organization;
        ret += "\n";
    }
    if (CONNECTION.version() >= "0801" && ext.getStorage() && ! monitoring.isEmpty() )
    {
        ret += monitoring;
        ret += "\n";
    }
    if (ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty() )
        ret += QString("PARALLEL\n"
                       "(\n"
                       "  DEGREE            %1\n"
                       "  INSTANCES         %2\n"
                       ")\n").
               arg(degree).
               arg(instances);

    toUnShift(result, toQValue(organization));
    toUnShift(result, toQValue(""));

    ret += segmentAttributes(ext, result);

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

QString toOracleExtract::createMView(toExtract &ext,
                                     const QString &schema,
                                     const QString &owner,
                                     const QString &name,
                                     const QString &type) const
{
    toQList result = toQuery::readQueryNull(CONNECTION, SQLMViewInfo, name, owner);
    if (result.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find materialised table %1.%2").
        arg(QUOTE(owner)).arg(QUOTE(name));
    QString table = toShift(result);
    QString buildMode = toShift(result);
    QString refreshMethod = toShift(result);
    QString refreshMode = toShift(result);
    QString startWith = toShift(result);
    QString next = toShift(result);
    QString usingPK = toShift(result);
    QString masterRBSeg = toShift(result);
    QString updatable = toShift(result);
    QString query = toShift(result);

    toQuery inf(CONNECTION, SQLIndexName, table, owner);
    QString index(inf.readValue());

    QString ret;
    QString sql = QString("CREATE %1 %2%3\n").arg(type).arg(schema).arg(QUOTE(name));
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n";
    }
    ret += sql;
    ret += createMViewTable(ext, schema, owner, table);
    ret += buildMode;
    ret += "\nUSING INDEX\n";
    ret += createMViewIndex(ext, schema, owner, index);

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
                   arg(QUOTE(masterRBSeg));
    }

    if (!updatable.isEmpty())
    {
        ret += updatable;
        ret += "\n";
    }
    ret += "AS\n";
    ret += query;
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::createMViewIndex(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    if (!ext.getStorage() || !ext.getParallel())
        return "";
    ext.setState("IsASnapIndex", true);

    static QRegExp start("^INITRANS");
    static QRegExp ignore("LOGGING");

    bool started = false;
    bool done = false;

    QString initial = createIndex(ext, schema, owner, name);
    QStringList linesIn = initial.split("\n");
    QString ret;

    for (QStringList::Iterator i = linesIn.begin();i != linesIn.end() && !done;i++)
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

QString toOracleExtract::createMViewLog(toExtract &ext,
                                        const QString &schema,
                                        const QString &owner,
                                        const QString &name,
                                        const QString &type) const
{
    toQList result = toQuery::readQueryNull(CONNECTION, SQLSnapshotInfo, name, owner);
    if (result.size() != 4)
        throw qApp->translate("toOracleExtract", "Couldn't find log %1.%2").arg(owner).arg(name);

    QString table = toShift(result);
    QString rowIds = toShift(result);
    QString primaryKey = toShift(result);
    QString filterColumns = toShift(result);

    QString ret;
    QString sql = QString("CREATE %1 LOG ON %2%3\n").
                  arg(type).
                  arg(schema).
                  arg(QUOTE(name));
    if (PROMPT)
    {
        QString ret = "PROMPT ";
        ret += sql;
        ret += "\n";
    }
    ret += sql;
    ret += createMViewTable(ext, schema, owner, table);

    if (rowIds == "YES" && primaryKey == "YES")
        ret += "WITH PRIMARY KEY, ROWID ";
    else if (rowIds == "YES")
        ret += "WITH ROWID ";
    else if (primaryKey == "YES")
        ret += "WITH PRIMARY KEY";

    toQuery inf(CONNECTION, SQLSnapshotColumns, name, owner);
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
            ret += QUOTE(inf.readValue());
        }
        ret += ")";
    }
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::createMViewTable(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    if (!ext.getStorage() || !ext.getParallel())
        return "";
    ext.setState("IsASnapTable", true);

    static QRegExp parallel("^PARALLEL");

    bool started = false;
    bool done = false;

    QString initial = createTable(ext, schema, owner, name);
    QStringList linesIn = initial.split("\n");
    QString ret;

    for (QStringList::Iterator i = linesIn.begin();i != linesIn.end() && !done;i++)
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

QString toOracleExtract::createPartitionedIOT(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    toQList result = toQuery::readQueryNull(CONNECTION, SQLPartitionedIOTInfo,
                                            QString::number(ext.getBlockSize()), name, owner);
    QString ret = createTableText(ext, result, schema, owner, name);
    if (ext.getStorage())
    {
        toQList overflow = toQuery::readQueryNull(CONNECTION, segments(ext, SQLOverflowInfo), name, owner);
        if (overflow.size() == 18)
        {
            ret += "OVERFLOW\n";
            ret += segmentAttributes(ext, overflow);
        }
    }

    if (ext.getPartition())
    {
        toQuery inf(CONNECTION, SQLPartitionIndexNames, name, owner);
        if (!inf.eof())
            throw qApp->translate("toOracleExtract", "Couldn't find index partitions for %1.%2").arg(owner).arg(name);
        QString index(inf.readValue());
        ret +=
            "PARTITION BY RANGE\n"
            "(\n    ";
        ret += partitionKeyColumns(ext, owner, name, "TABLE");
        ret += "\n)\n";
        ret += rangePartitions(ext, owner, index, "NONE", "IOT");
    }
    ret += createComments(ext, schema, owner, name);
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

QString toOracleExtract::createPartitionedIndex(toExtract &ext,
        const QString &,
        const QString &owner,
        const QString &name,
        const QString &sql) const
{
    if (!ext.getPartition())
        return "";

    QString ret = sql;
    toQList result = toQuery::readQueryNull(CONNECTION, SQLIndexPartition,
                                            QString::number(ext.getBlockSize()), name, owner);
    QString partitionType = toShift(result);
    QString subPartitionType = toShift(result);
    QString locality = toShift(result);
    QString compressed = toShift(result);
    toUnShift(result, toQValue(""));

    ret += segmentAttributes(ext, result);

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
        ret += partitionKeyColumns(ext, owner, name, "INDEX");
        ret += "\n)\n";
        ret += rangePartitions(ext, owner, name, subPartitionType, "GLOBAL");
    }
    else
    {
        ret += "LOCAL\n";
        if (partitionType == "RANGE")
            ret += rangePartitions(ext, owner, name, subPartitionType, "LOCAL");
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

QString toOracleExtract::createPartitionedTable(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    toQList storage = toQuery::readQueryNull(CONNECTION, SQLPartitionTableInfo,
                      QString::number(ext.getBlockSize()), name, owner);

    QString organization;
    {
        toQList::iterator i = storage.begin();
        i++;
        i++;
        i++;
        i++;
        organization = *i;
    }

    QString ret = createTableText(ext, storage, schema, owner, name);

    if (ext.getPartition())
    {
        toQList type = toQuery::readQueryNull(CONNECTION, SQLPartitionType, name, owner);
        QString partitionType (toShift(type));
        QString partitionCount (toShift(type));
        QString subPartitionType (toShift(type));
        QString subPartitionCount(toShift(type));

        ret += QString("PARTITION BY %1\n(\n    ").arg(partitionType);
        ret += partitionKeyColumns(ext, owner, name, "TABLE");
        ret += "\n)\n";

        if (partitionType == "RANGE")
        {
            if (subPartitionType == "HASH")
            {
                ret += "SUBPARTITIONED BY HASH\n(\n    ";
                ret += subPartitionKeyColumns(ext, owner, name, "TABLE");
                ret += "\n)\nSUBPARTITIONS ";
                ret += subPartitionCount;
                ret += "\n";
            }
            ret += "(\n";

            toQList segment = toQuery::readQueryNull(CONNECTION, SQLPartitionSegment, name, owner);

            QString comma = "    ";
            while (!segment.empty())
            {
                toQList storage;
                QString partition = toShift(segment);
                QString highValue = toShift(segment);
                toPush(storage, toQValue("      "));
                toPush(storage, toQValue(organization));
                for (int i = 0;i < 16;i++)
                    toPush(storage, toShift(segment));

                ret += comma;
                ret += QString("PARTITION %1 VALUES LESS THAN\n"
                               "      (\n"
                               "        %2\n"
                               "      )\n").arg(QUOTE(partition)).arg(highValue);
                ret += segmentAttributes(ext, storage);
                comma = "  , ";

                if (subPartitionType == "HASH")
                {
                    toQList subs = toQuery::readQueryNull(CONNECTION, SQLSubPartitionName,
                                                          name, partition, owner);
                    bool first = true;
                    ret += "        (\n            ";
                    while (!subs.empty())
                    {
                        if (first)
                            first = false;
                        else
                            ret += "\n          , ";
                        ret += QString("SUBPARTITION %2 TABLESPACE %1").
                               arg(QUOTE(toShift(subs))).
                               arg(QUOTE(toShift(subs)));
                    }
                    ret += "\n        )\n";
                }
            }
        }
        else
        {
            toQList hash = toQuery::readQueryNull(CONNECTION, SQLPartitionName, name, owner);
            bool first = true;
            ret += "(\n    ";
            while (!hash.empty())
            {
                if (first)
                    first = false;
                else
                    ret += "\n  , ";
                ret += QString("PARTITION %2 TABLESPACE %1").
                       arg(QUOTE(toShift(hash))).
                       arg(QUOTE(toShift(hash)));
            }
        }
        ret += ")\n";
    }

    ret += ";\n\n";
    ret += createComments(ext, schema, owner, name);
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

QString toOracleExtract::grantedPrivs(toExtract &ext,
                                      const QString &dest,
                                      const QString &name,
                                      int typ) const
{
    if (!ext.getGrants())
        return "";

    QString ret;
    if ((typ&1) == 1)
    {
        toQList result = toQuery::readQueryNull(CONNECTION, SQLSystemPrivs, name);
        while (!result.empty())
        {
            QString priv = QString(toShift(result)).toLower();
            QString sql = QString("GRANT %1 TO %2 %3").
                          arg(priv).
                          arg(dest).
                          arg(toShift(result));
            if (PROMPT)
            {
                ret += "PROMPT ";
                ret += sql;
                ret += "\n\n";
            }
            ret += sql;
            ret += ";\n\n";
        }
    }

    if ((typ&2) == 2)
    {
        toQList result = toQuery::readQueryNull(CONNECTION, SQLRolePrivs, name);
        while (!result.empty())
        {
            QString priv = QString(toShift(result)).toLower();
            QString sql = QString("GRANT %1 TO %2 %3").
                          arg(priv).
                          arg(dest).
                          arg(toShift(result));
            if (PROMPT)
            {
                ret += "PROMPT ";
                ret += sql;
                ret += "\n\n";
            }
            ret += sql;
            ret += ";\n\n";
        }
    }

    if ((typ&4) == 4)
    {
        toQList result = toQuery::readQueryNull(CONNECTION, SQLObjectPrivs, name);
        while (!result.empty())
        {
            QString priv = QString(toShift(result)).toLower();
            QString schema = ext.intSchema(toShift(result), false);
            QString object = QUOTE(toShift(result));
            QString sql = QString("GRANT %1 ON %2%3 TO %4 %5").
                          arg(priv.toLower()).
                          arg(schema).
                          arg(object).
                          arg(dest).
                          arg(toShift(result));
            if (PROMPT)
            {
                ret += "PROMPT ";
                ret += sql;
                ret += "\n\n";
            }
            ret += sql;
            ret += ";\n\n";
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
                              "07");
static toSQL SQLIndexFunction("toOracleExtract:IndexFunction",
                              "SELECT c.default$\n"
                              "  FROM sys.col$ c,\n"
                              "       sys.all_indexes i,\n"
                              "       sys.all_objects o\n"
                              " WHERE i.index_name = :ind<char[100]>\n"
                              "   AND o.object_name = i.table_name\n"
                              "   AND c.obj# = o.object_id\n"
                              "   AND c.name = :tab<char[100]>\n"
                              "   AND i.owner = :own<char[100]>\n"
                              "   AND o.owner = i.table_owner",
                              "Get function of index column, same column and binds");

QString toOracleExtract::indexColumns(toExtract &ext,
                                      const QString &indent,
                                      const QString &owner,
                                      const QString &name) const
{
    static QRegExp quote("\"");
    static QRegExp func("^sys_nc[0-9]+", Qt::CaseInsensitive);
    toQuery inf(CONNECTION, SQLIndexColumns, name, owner);
    QString ret = indent;
    ret += "(\n";
    bool first = true;
    while (!inf.eof())
    {
        QString col = inf.readValue();
        QString asc = inf.readValue();
        QString row;
        if (func.indexIn(col) >= 0)
        {
            toQuery def(CONNECTION, SQLIndexFunction, name, col, owner);
            if (!def.eof())
            {
                QString function(def.readValue());
                function.replace(quote, "");
                if (asc == "DESC")
                    row = QString("%1 DESC").arg(function, 30);
                else
                    row = function;
            }
            else if (asc == "DESC")
                row = QString("%1 DESC").arg(QUOTE(col));
            else
                row = QUOTE(col);
        }
        else if (asc == "DESC")
            row = QString("%1 DESC").arg(QUOTE(col));
        else
            row = QUOTE(col);
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

QString toOracleExtract::rangePartitions(toExtract &ext,
        const QString &owner,
        const QString &name,
        const QString &subPartitionType,
        const QString &caller) const
{
    if (!ext.getPartition())
        return "";

    toQList result = toQuery::readQueryNull(CONNECTION, SQLRangePartitions, name, owner);
    if (result.empty() || result.size() % 18)
        throw qApp->translate("toOracleExtract", "Couldn't find partition range %1.%2").arg(owner).arg(name);

    QString ret;

    QString comma = "    ";
    while (!result.empty())
    {
        QString partition = toShift(result);
        QString highValue = toShift(result);

        toQList storage;
        toPush(storage, toQValue("      "));
        toPush(storage, toQValue("INDEX"));
        for (int i = 0;i < 16;i++)
            toPush(storage, toShift(result));

        ret += comma;
        ret += "PARTITION ";
        ret += QUOTE(partition);
        if (caller != "LOCAL")
            ret += QString(" VALUES LESS THAN (%1)\n").arg(highValue);
        else
            ret += "\n";
        ret += segmentAttributes(ext, storage);

        if (subPartitionType == "HASH")
        {
            toQuery inf(CONNECTION, SQLIndexSubPartitionName, name, partition, owner);
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
                ret += QUOTE(sub);
                ret += " TABLESPACE ";
                ret += QUOTE(tablespace);
            }
            ret += "\n        )\n";
        }
        comma = "  , ";
    }
    ret += ");\n\n";

    return ret;
}

QString toOracleExtract::segmentAttributes(toExtract &ext, toQList &result) const
{
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

        QString indent = *i;
        i++;
        QString organization = *i;
        i++;
        QString cache = *i;
        i++;
        QString pctUsed = *i;
        i++;
        QString pctFree = *i;
        i++;
        QString iniTrans = *i;
        i++;
        QString maxTrans = *i;
        i++;
        QString initial = *i;
        i++;
        QString next = *i;
        i++;
        QString minExtents = *i;
        i++;
        QString maxExtents = *i;
        i++;
        QString pctIncrease = *i;
        i++;
        QString freelists = *i;
        i++;
        QString freelistGroups = *i;
        i++;
        QString bufferPool = *i;
        i++;
        QString logging = *i;
        i++;
        QString tablespace = *i;
        i++;
        QString blocks = *i;
        i++;

        if (ext.getResize())
            ext.initialNext(blocks, initial, next);

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
        if (CONNECTION.version() >= "0801")
        {
            ret += QString("%1  BUFFER_POOL       %2\n").arg(indent).arg(QUOTE(bufferPool));
        }
        ret += indent;
        ret += ")\n";
        if (CONNECTION.version() >= "0800")
            ret += QString("%1%2\n").arg(indent).arg(logging);
        ret += QString("%1TABLESPACE          %2\n").arg(indent).arg(QUOTE(tablespace));
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
                              "                                                  ,  'NUMBER   '\n"
                              "                                                 )\n"
                              "                                     ,'NUMBER   '\n"
                              "                                    )\n"
                              "                    ,'RAW'     ,'RAW      '\n"
                              "                    ,'CHAR'    ,'CHAR     '\n"
                              "                    ,'NCHAR'   ,'NCHAR    '\n"
                              "                    ,'UROWID'  ,'UROWID   '\n"
                              "                    ,'VARCHAR2','VARCHAR2 '\n"
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
                              "09");

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
                             "                                                  ,  'NUMBER   '\n"
                             "                                                 )\n"
                             "                                     ,'NUMBER   '\n"
                             "                                    )\n"
                             "                    ,'RAW'     ,'RAW      '\n"
                             "                    ,'CHAR'    ,'CHAR     '\n"
                             "                    ,'NCHAR'   ,'NCHAR    '\n"
                             "                    ,'UROWID'  ,'UROWID   '\n"
                             "                    ,'VARCHAR2','VARCHAR2 '\n"
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
                              "                                                  ,  'NUMBER   '\n"
                              "                                                 )\n"
                              "                                     ,'NUMBER   '\n"
                              "                                    )\n"
                              "                    ,'RAW'     ,'RAW      '\n"
                              "                    ,'CHAR'    ,'CHAR     '\n"
                              "                    ,'UROWID'  ,'UROWID   '\n"
                              "                    ,'VARCHAR2','VARCHAR2 '\n"
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

QString toOracleExtract::tableColumns(toExtract &ext,
                                      const QString &owner,
                                      const QString &name) const
{
    toQList cols = toQuery::readQueryNull(CONNECTION, SQLTableColumns, name, owner);
    bool first = true;
    QString ret;
    while (!cols.empty())
    {
        if (first)
            first = false;
        else
            ret += "\n  , ";
        ret += QString(QUOTE(toShift(cols))).leftJustified(32);
        ret += " ";
        ret += toShift(cols);
        QString def = toShift(cols);
        QString notNull = toShift(cols);
        if (!def.isEmpty())
        {
            ret += "DEFAULT ";
            ret += def.trimmed();
            ret += " ";
        }
        ret += notNull;
    }
    ret += "\n";
    return ret;
}

// Implementation describe utility functions

void toOracleExtract::describeAttributes(toExtract &ext,
        std::list<QString> &dsp,
        std::list<QString> &ctx,
        toQList &result) const
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

    QString indent = *i;
    i++;
    QString organization = *i;
    i++;
    QString cache = *i;
    i++;
    QString pctUsed = *i;
    i++;
    QString pctFree = *i;
    i++;
    QString iniTrans = *i;
    i++;
    QString maxTrans = *i;
    i++;
    QString initial = *i;
    i++;
    QString next = *i;
    i++;
    QString minExtents = *i;
    i++;
    QString maxExtents = *i;
    i++;
    QString pctIncrease = *i;
    i++;
    QString freelists = *i;
    i++;
    QString freelistGroups = *i;
    i++;
    QString bufferPool = *i;
    i++;
    QString logging = *i;
    i++;
    QString tablespace = *i;
    i++;
    QString blocks = *i;
    i++;

    if (ext.getResize())
        ext.initialNext(blocks, initial, next);

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
    if (CONNECTION.version() >= "0801")
    {
        addDescription(dsp, ctx, "STORAGE", QString("BUFFER_POOL %1").arg(QUOTE(bufferPool)));
    }
    if (CONNECTION.version() >= "0800")
        addDescription(dsp, ctx, "PARAMETERS", logging);
    addDescription(dsp, ctx, "PARAMETERS", QString("TABLESPACE %1").arg(QUOTE(tablespace)));
}

void toOracleExtract::describeComments(toExtract &ext,
                                       std::list<QString> &lst,
                                       std::list<QString> &ctx,
                                       const QString &owner,
                                       const QString &name) const
{
    if (ext.getComments())
    {
        toQuery inf(CONNECTION, SQLTableComments, name, owner);
        while (!inf.eof())
        {
            addDescription(lst, ctx, "COMMENT", inf.readValue());
        }
        toQuery col(CONNECTION, SQLColumnComments, name, owner);
        while (!col.eof())
        {
            QString column = col.readValue();
            addDescription(lst, ctx, "COLUMN", QUOTE(column), "COMMENT", col.readValue());
        }
    }
}

void toOracleExtract::describePrivs(toExtract &ext,
                                    std::list<QString> &lst,
                                    std::list<QString> &ctx,
                                    const QString &name) const
{
    if (!ext.getGrants())
        return ;

    toQList result = toQuery::readQueryNull(CONNECTION, SQLRolePrivs, name);
    while (!result.empty())
    {
        QString role = QUOTE(toShift(result));
        addDescription(lst, ctx, "GRANT", "ROLE " + role, toShift(result));
    }

    result = toQuery::readQueryNull(CONNECTION, SQLSystemPrivs, name);
    while (!result.empty())
    {
        QString priv = QString(toShift(result)).toLower();
        addDescription(lst, ctx, "GRANT", priv, toShift(result));
    }

    result = toQuery::readQueryNull(CONNECTION, SQLObjectPrivs, name);
    while (!result.empty())
    {
        QString priv = toShift(result);
        QString schema = ext.intSchema(toShift(result), false);
        QString res = schema;
        res += QUOTE(toShift(result));
        addDescription(lst, ctx, "GRANT", priv.toLower(), "ON", res, toShift(result));
    }
}

void toOracleExtract::describeIOT(toExtract &ext,
                                  std::list<QString> &lst,
                                  std::list<QString> &ctx,
                                  const QString &schema,
                                  const QString &owner,
                                  const QString &name) const
{
    toQList storage = toQuery::readQueryNull(CONNECTION, SQLIOTInfo, name, owner);

    describeTableText(ext, lst, ctx, storage, schema, owner, name);
    describeComments(ext, lst, ctx, owner, name);
    if (ext.getStorage())
    {
        toQList overflow = toQuery::readQueryNull(CONNECTION, segments(ext, SQLOverflowInfo), name, owner);
        if (overflow.size() == 18)
        {
            ctx.insert(ctx.end(), "OVERFLOW");
            describeAttributes(ext, lst, ctx, overflow);
        }
    }
}

void toOracleExtract::describeIndexColumns(toExtract &ext,
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &owner,
        const QString &name) const
{
    static QRegExp quote("\"");
    static QRegExp func("^sys_nc[0-9]g");
    toQuery inf(CONNECTION, SQLIndexColumns, name, owner);
    int num = 1;
    while (!inf.eof())
    {
        QString col = inf.readValue();
        QString asc = inf.readValue();
        QString row;
        if (func.indexIn(col) >= 0)
        {
            toQuery def(CONNECTION, SQLIndexFunction, col, name, owner);
            QString function(inf.readValue());
            function.replace(quote, "");
            if (asc == "DESC")
                row = QString("%1 DESC").arg(function, 30);
            else
                row = function;
        }
        else if (asc == "DESC")
            row = QString("%1 DESC").arg(QUOTE(col));
        else
            row = QUOTE(col);
        addDescription(lst, ctx, "COLUMN", row);
        addDescription(lst, ctx, "COLUMN", row, "ORDER", QString::number(num));
        num++;
    }
}

void toOracleExtract::describeMView(toExtract &ext, std::list<QString> &lst,
                                    const QString &schema, const QString &owner,
                                    const QString &name, const QString &type) const
{
    toQList result = toQuery::readQueryNull(CONNECTION, SQLMViewInfo, name, owner);
    if (result.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find materialised table %1.%2").
        arg(QUOTE(owner)).arg(QUOTE(name));
    QString table = toShift(result);
    QString buildMode = toShift(result);
    QString refreshMethod = toShift(result);
    QString refreshMode = toShift(result);
    QString startWith = toShift(result);
    QString next = toShift(result);
    QString usingPK = toShift(result);
    QString masterRBSeg = toShift(result);
    QString updatable = toShift(result);
    QString query = toShift(result);

    toQuery inf(CONNECTION, SQLIndexName, table, owner);
    QString index(inf.readValue());

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), type);
    ctx.insert(ctx.end(), QUOTE(name));

    describeMViewTable(ext, lst, ctx, schema, owner, table);
    addDescription(lst, ctx, buildMode);
    ctx.insert(ctx.end(), "INDEX");
    ctx.insert(ctx.end(), QUOTE(index));
    describeMViewIndex(ext, lst, ctx, schema, owner, index);

    if (refreshMethod != "NEVER REFRESH")
    {
        addDescription(lst, ctx, QString("START WITH %1").arg(startWith));
        if (!next.isEmpty())
            addDescription(lst, ctx, QString("NEXT %1").arg(next));
        addDescription(lst, ctx, usingPK);
        if (!masterRBSeg.isEmpty() && ext.getStorage())
            addDescription(lst, ctx, QString("USING MASTER ROLLBACK SEGMENT %1").
                           arg(QUOTE(masterRBSeg)));
    }

    if (!updatable.isEmpty())
        addDescription(lst, ctx, updatable);
    addDescription(lst, ctx, QString("AS %1").arg(query));
}

static QString ReContext(std::list<QString> &ctx, int strip, const QString &str)
{
    QStringList lst = str.split("\01");
    QString ret;
    QString sep = "";
    for (std::list<QString>::iterator i = ctx.begin();i != ctx.end();i++)
    {
        ret += sep;
        ret += *i;
        if (sep.isEmpty())
            sep = "\01";
    }
    for (QStringList::Iterator j = lst.begin();j != lst.end();j++)
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

void toOracleExtract::describeMViewIndex(toExtract &ext,
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    if (!ext.getStorage() || !ext.getParallel())
        return ;
    ext.setState("IsASnapIndex", true);

    static QRegExp start("^[^\001]+[\001][^\001]+[\001][^\001]+[\001]INITTRANS");
    static QRegExp ignore("^[^\001]+[\001][^\001]+[\001][^\001]+[\001]LOGGING");

    bool started = false;
    bool done = false;

    std::list<QString> tbllst;
    describeIndex(ext, tbllst, schema, owner, name);
    QString ret;

    for (std::list<QString>::iterator i = tbllst.begin();i != tbllst.end() && !done;i++)
    {
        if (start.indexIn(*i) >= 0)
            started = true;
        if (started)
            lst.insert(lst.end(), ReContext(ctx, 3, *i));
    }

    ext.setState("IsASnapIndex", false);
}

void toOracleExtract::describeMViewLog(toExtract &ext,
                                       std::list<QString> &lst,
                                       const QString &schema,
                                       const QString &owner,
                                       const QString &name,
                                       const QString &type) const
{
    toQList result = toQuery::readQueryNull(CONNECTION, SQLSnapshotInfo, name, owner);
    if (result.size() != 4)
        throw qApp->translate("toOracleExtract", "Couldn't find log %1.%2").arg(owner).arg(name);

    QString table = toShift(result);
    QString rowIds = toShift(result);
    QString primaryKey = toShift(result);
    QString filterColumns = toShift(result);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), type);
    ctx.insert(ctx.end(), QUOTE(name));

    describeMViewTable(ext, lst, ctx, schema, owner, table);

    if (rowIds == "YES" && primaryKey == "YES")
        ctx.insert(ctx.end(), "WITH PRIMARY KEY, ROWID");
    else if (rowIds == "YES")
        ctx.insert(ctx.end(), "WITH ROWID");
    else if (primaryKey == "YES")
        ctx.insert(ctx.end(), "WITH PRIMARY KEY");
    addDescription(lst, ctx);

    toQuery inf(CONNECTION, SQLSnapshotColumns, name, owner);
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
            col += QUOTE(inf.readValue());
        }
        col += ")";
        addDescription(lst, ctx, col);
    }
}

void toOracleExtract::describeMViewTable(toExtract &ext,
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    if (!ext.getStorage() || !ext.getParallel())
        return ;
    ext.setState("IsASnapTable", true);
    //                        Schema        Table         Name
    static QRegExp parallel("^[^\001]+[\001][^\001]+[\001][^\001]+[\001]PARALLEL");

    bool started = false;
    bool done = false;

    std::list<QString> tbllst;
    describeTable(ext, tbllst, schema, owner, name);
    QString ret;

    for (std::list<QString>::iterator i = tbllst.begin();i != tbllst.end() && !done;i++)
    {
        if (parallel.indexIn(*i) >= 0)
            started = true;
        if (started)
            lst.insert(lst.end(), ReContext(ctx, 3, *i));
    }

    ext.setState("IsASnapTable", false);
}

void toOracleExtract::describePartitions(toExtract &ext,
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &owner,
        const QString &name,
        const QString &subPartitionType,
        const QString &caller) const
{
    if (!ext.getPartition())
        return ;

    toQList result = toQuery::readQueryNull(CONNECTION, SQLRangePartitions, name, owner);
    if (result.empty() || result.size() % 18)
        throw qApp->translate("toOracleExtract", "Couldn't find partition range %1.%2").arg(owner).arg(name);

    while (!result.empty())
    {
        QString partition = toShift(result);
        QString highValue = toShift(result);

        toQList storage;
        toPush(storage, toQValue("      "));
        toPush(storage, toQValue("INDEX"));
        for (int i = 0;i < 16;i++)
            toPush(storage, toShift(result));

        std::list<QString> cctx = ctx;
        cctx.insert(cctx.end(), QUOTE(partition));

        addDescription(lst, cctx);
        if (caller != "LOCAL")
            addDescription(lst, cctx, "RANGE", QString("VALUES LESS THAN (%1)").arg(highValue));
        describeAttributes(ext, lst, cctx, storage);

        if (subPartitionType == "HASH")
        {
            toQuery inf(CONNECTION, SQLIndexSubPartitionName, name, partition, owner);
            while (!inf.eof())
            {
                QString sub(inf.readValue());
                QString tablespace(inf.readValue());
                addDescription(lst, cctx, "HASH", QString("%1 TABLESPACE %2").arg(QUOTE(sub)).arg(QUOTE(tablespace)));
            }
        }
    }
}

void toOracleExtract::describePartitionedIOT(toExtract &ext,
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    toQList result = toQuery::readQueryNull(CONNECTION, SQLPartitionedIOTInfo,
                                            QString::number(ext.getBlockSize()), name, owner);
    describeTableText(ext, lst, ctx, result, schema, owner, name);
    if (ext.getPartition())
    {
        toQuery inf(CONNECTION, SQLPartitionIndexNames, name, owner);
        if (!inf.eof())
            throw qApp->translate("toOracleExtract", "Couldn't find index partitions for %1.%2").arg(owner).arg(name);
        QString index(inf.readValue());
        std::list<QString> cctx = ctx;
        toPush(cctx, QString("PARTITION COLUMNS"));
        toPush(cctx, partitionKeyColumns(ext, owner, name, "TABLE"));
        addDescription(lst, cctx);
        describePartitions(ext, lst, cctx, owner, index, "NONE", "IOT");
    }
    describeComments(ext, lst, ctx, owner, name);
    if (ext.getStorage())
    {
        toQList overflow = toQuery::readQueryNull(CONNECTION, segments(ext, SQLOverflowInfo), name, owner);
        if (overflow.size() == 18)
        {
            ctx.insert(ctx.end(), "OVERFLOW");
            describeAttributes(ext, lst, ctx, overflow);
        }
    }
}

void toOracleExtract::describePartitionedIndex(toExtract &ext,
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &,
        const QString &owner,
        const QString &name) const
{
    if (!ext.getPartition())
        return ;

    toQList result = toQuery::readQueryNull(CONNECTION, SQLIndexPartition,
                                            QString::number(ext.getBlockSize()), owner, name);
    QString partitionType = toShift(result);
    QString subPartitionType = toShift(result);
    QString locality = toShift(result);
    QString compressed = toShift(result);
    toUnShift(result, toQValue(""));

    describeAttributes(ext, lst, ctx, result);

    if (!compressed.isEmpty())
        addDescription(lst, ctx, "COMPRESS", compressed);

    if (locality == "GLOBAL")
    {
        std::list<QString> cctx = ctx;
        toPush(cctx, QString("GLOBAL PARTITION COLUMNS"));
        toPush(cctx, partitionKeyColumns(ext, owner, name, "INDEX"));

        addDescription(lst, cctx);
        describePartitions(ext, lst, cctx, owner, name, subPartitionType, "GLOBAL");
    }
    else
    {
        std::list<QString> cctx = ctx;
        toPush(cctx, QString("LOCAL PARTITION"));
        addDescription(lst, cctx);
        if (partitionType == "RANGE")
            describePartitions(ext, lst, cctx, owner, name, subPartitionType, "LOCAL");
    }
}

void toOracleExtract::describePartitionedTable(toExtract &ext,
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    toQList storage = toQuery::readQueryNull(CONNECTION, SQLPartitionTableInfo,
                      QString::number(ext.getBlockSize()), name, owner);

    QString organization;
    {
        toQList::iterator i = storage.begin();
        i++;
        i++;
        i++;
        i++;
        organization = *i;
    }

    describeTableText(ext, lst, ctx, storage, schema, owner, name);

    if (ext.getPartition())
    {
        toQList type = toQuery::readQueryNull(CONNECTION, SQLPartitionType, name, owner);
        QString partitionType (toShift(type));
        QString partitionCount (toShift(type));
        QString subPartitionType (toShift(type));
        QString subPartitionCount(toShift(type));

        QString prtstr = QString("PARTITION BY %1 (%2)").
                         arg(partitionType).
                         arg(partitionKeyColumns(ext, owner, name, "TABLE"));
        addDescription(lst, ctx, prtstr);
        QString subprtstr;

        if (partitionType == "RANGE")
        {
            if (subPartitionType == "HASH")
            {
                subprtstr = QString("SUBPARTITIONED BY HASH (%1) SUBPARTITIONS %2").
                            arg(subPartitionKeyColumns(ext, owner, name, "TABLE")).
                            arg(subPartitionCount);
                addDescription(lst, ctx, subprtstr);
            }

            toQList segment = toQuery::readQueryNull(CONNECTION, SQLPartitionSegment, name, owner);

            while (!segment.empty())
            {
                toQList storage;
                QString partition = toShift(segment);
                QString highValue = toShift(segment);
                toPush(storage, toQValue("      "));
                toPush(storage, toQValue(organization));
                for (int i = 0;i < 16;i++)
                    toPush(storage, toShift(segment));

                std::list<QString> cctx = ctx;
                cctx.insert(cctx.end(), prtstr);
                cctx.insert(cctx.end(), QUOTE(partition));
                addDescription(lst, cctx, "RANGE", QString("VALUES LESS THAN %2").
                               arg(highValue));
                describeAttributes(ext, lst, cctx, storage);

                if (subPartitionType == "HASH")
                {
                    toQList subs = toQuery::readQueryNull(CONNECTION, SQLSubPartitionName,
                                                          name, partition, owner);
                    while (!subs.empty())
                    {
                        QString subpart = QUOTE(toShift(subs));
                        QString tabspac = toShift(subs);
                        addDescription(lst, cctx, subprtstr, subpart);
                        addDescription(lst, cctx, subprtstr, subpart,
                                       QString("TABLESPACE %1").arg(QUOTE(tabspac)));
                    }
                }
            }
        }
        else
        {
            toQList hash = toQuery::readQueryNull(CONNECTION, SQLPartitionName,
                                                  name, owner);
            while (!hash.empty())
            {
                QString partition = QUOTE(toShift(hash));
                QString tablespac = toShift(hash);
                addDescription(lst, ctx, prtstr, partition);
                addDescription(lst, ctx, prtstr, partition,
                               QString("TABLESPACE %1").arg(QUOTE(tablespac)));
            }
        }
    }

    describeComments(ext, lst, ctx, owner, name);
}

void toOracleExtract::describeSource(toExtract &ext,
                                     std::list<QString> &lst,
                                     const QString &schema,
                                     const QString &owner,
                                     const QString &name,
                                     const QString &type) const
{
    if (!ext.getCode())
        return ;

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), type);
    ctx.insert(ctx.end(), QUOTE(name));
    addDescription(lst, ctx);
    addDescription(lst, ctx, displaySource(ext, schema, owner, name, type, true));
}

void toOracleExtract::describeTableColumns(toExtract &ext,
        std::list<QString> &lst,
        std::list<QString> &ctx,
        const QString &owner,
        const QString &name) const
{
    toQList cols = toQuery::readQueryNull(CONNECTION, SQLTableColumns, name, owner);
    int num = 1;
    while (!cols.empty())
    {
        QString col = QUOTE(toShift(cols));
        QString line = toShift(cols);
        QString extra = toShift(cols);
        if (!extra.isEmpty())
        {
            line += " DEFAULT ";
            line += extra;
        }
        extra += toShift(cols);
        addDescription(lst, ctx, "COLUMN", col);
        addDescription(lst, ctx, "COLUMN", col, line);
        addDescription(lst, ctx, "COLUMN", col, "EXTRA", extra);
        addDescription(lst, ctx, "COLUMN", col, "ORDER", QString::number(num));
        num++;
    }
}

void toOracleExtract::describeTableText(toExtract &ext,
                                        std::list<QString> &lst,
                                        std::list<QString> &ctx,
                                        toQList &result,
                                        const QString &,
                                        const QString &owner,
                                        const QString &name) const
{
    QString monitoring = toShift(result);
    QString table = toShift(result);
    QString degree = toShift(result);
    QString instances = toShift(result);
    QString organization = toShift(result);

    QString ret;
    addDescription(lst, ctx);
    describeTableColumns(ext, lst, ctx, owner, name);
    if (CONNECTION.version() >= "0800" && ext.getStorage() && !organization.isEmpty() )
        addDescription(lst, ctx, "PARAMETERS", QString("ORGANIZATION %1").arg(organization));
    if (CONNECTION.version() >= "0801" && ext.getStorage() && ! monitoring.isEmpty()  )
        addDescription(lst, ctx, "PARAMETERS", monitoring);
    if (ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty() )
    {
        addDescription(lst, ctx, "PARALLEL", QString("DEGREE %1").arg(degree));
        addDescription(lst, ctx, "PARALLEL", QString("INSTANCES %1").arg(instances));
    }

    toUnShift(result, toQValue(organization));
    toUnShift(result, toQValue(""));

    describeAttributes(ext, lst, ctx, result);
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

QString toOracleExtract::createConstraint(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    QString ret;

    if (ext.getConstraints())
    {
        toQuery inf(CONNECTION, SQLListConstraint, owner, name);
        if (inf.eof())
            throw qApp->translate("toOracleExtract", "Constraint %1.%2 doesn't exist").arg(owner).arg(name);
        QString table(inf.readValue());
        QString tchr(inf.readValue());
        QString search(inf.readValue());
        QString rOwner(inf.readValue());
        QString rName(inf.readValue());
        QString delRule(inf.readValue());
        QString status(inf.readValue());
        QString defferable(inf.readValue());
        QString deffered(inf.readValue());

        QString type =
            (tchr == "P") ? "PRIMARY KEY" :
            (tchr == "U") ? "UNIQUE" :
            (tchr == "R") ? "FOREIGN KEY" :
            "CHECK";

        QString sql = QString("ALTER TABLE %1%2 ADD CONSTRAINT %3 %4\n").
                      arg(schema).arg(QUOTE(table)).arg(QUOTE(name)).arg(type);
        if (PROMPT)
        {
            ret = "PROMPT ";
            ret += sql;
            ret += "\n";
        }
        ret += sql;
        if (tchr == "C")
        {
            ret += "    (";
            ret += search;
            ret += ")\n";
        }
        else
        {
            ret += constraintColumns(ext, owner, name);

            if (tchr == "R")
            {
                toQuery query(CONNECTION, SQLConstraintTable, rOwner, rName);
                ret += " REFERENCES ";
                ret += ext.intSchema(rOwner, false);
                ret += QUOTE(query.readValue());
                ret += constraintColumns(ext, rOwner, rName);

                if (delRule == "CASCADE")
                    ret += "ON DELETE CASCADE\n";
            }
        }
        if (CONNECTION.version() < "08")
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
        ret += ";\n\n";
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

QString toOracleExtract::createDBLink(toExtract &ext,
                                      const QString &,
                                      const QString &owner,
                                      const QString &name) const
{
    toQuery inf(CONNECTION, SQLDBLink, owner, name);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "DBLink %1.%2 doesn't exist").arg(owner).arg(name);
    QString user(inf.readValue());
    QString password(inf.readValue());
    QString host(inf.readValue());
    QString publ = (owner == "PUBLIC") ? " PUBLIC" : "";
    QString sql = QString("CREATE%1 DATABASE LINK %2").arg(publ).arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += QString("\nCONNECT TO %1 IDENTIFIED BY %2 USING '%3';\n\n").
           arg(user.toLower()).
           arg(password.toLower()).
           arg(prepareDB(host));
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

QString toOracleExtract::createExchangeIndex(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    QStringList str = name.split(":");
    if (str.count() != 2)
        throw ("When calling createExchangeIndex name should contain :");
    QString segment = str.first();
    QString partition = str.last();

    toQuery inf(CONNECTION, segments(ext, SQLPartitionSegmentType), segment, partition, owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Exchange index %1.%2 doesn't exist").arg(owner).arg(name);
    QString type = inf.readValue();
    QString blocks = inf.readValue();

    QString sql = toSQL::string(SQLExchangeIndex, CONNECTION).
                  arg(type).arg(blocks);
    toQList result = toQuery::readQueryNull(CONNECTION, sql, segment, partition, owner);
    QString degree = toShift(result);
    QString instances = toShift(result);
    QString table = toShift(result);
    QString unique = toShift(result);
    QString bitmap = toShift(result);

    toUnShift(result, toQValue(""));

    sql = QString("CREATE%1%2 INDEX %3%4 ON %3%5\n").
          arg(unique).arg(bitmap).arg(schema).arg(segment).arg(QUOTE(table));

    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n";
    }
    ret += sql;
    ret += indexColumns(ext, "", owner, segment);
    if (ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty() )
    {
        ret += "PARALLEL\n(\n  DEGREE            ";
        ret += degree;
        ret += "\n  INSTANCES         ";
        ret += instances;
        ret += ")\n";
    }
    ret += segmentAttributes(ext, result);
    ret += ";\n\n";
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

QString toOracleExtract::createExchangeTable(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    QStringList str = name.split(":");
    if (str.count() != 2)
        throw ("When calling createExchangeTable name should contain :");
    QString segment = str.first();
    QString partition = str.last();

    toQuery inf(CONNECTION, segments(ext, SQLPartitionSegmentType), segment, partition, owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Exchange table %1.%2 doesn't exist").arg(owner).arg(name);
    QString type = inf.readValue();
    QString blocks = inf.readValue();

    QString sql = toSQL::string(SQLExchangeTable, CONNECTION).arg(type).arg(blocks);
    toQList result = toQuery::readQueryNull(CONNECTION, sql, segment, partition, owner);
    QString ret = createTableText(ext, result, schema, owner, segment);
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::createFunction(toExtract &ext,
                                        const QString &schema,
                                        const QString &owner,
                                        const QString &name) const
{
    return displaySource(ext, schema, owner, name, "FUNCTION", false);
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

QString toOracleExtract::createIndex(toExtract &ext,
                                     const QString &schema,
                                     const QString &owner,
                                     const QString &name) const
{
    if (!ext.getIndexes())
        return "";

    toQList res = toQuery::readQueryNull(CONNECTION, SQLIndexInfo, name, owner);
    if (res.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find index %1.%2").arg(owner).arg(name);

    QString partitioned = toShift(res);
    QString table = toShift(res);
    QString tableOwner = toShift(res);
    QString reverse = toShift(res);
    QString unique = toShift(res);
    QString bitmap = toShift(res);
    QString domain = toShift(res);
    QString domOwner = toShift(res);
    QString domName = toShift(res);
    QString domParam = toShift(res);

    toQList storage = toQuery::readQueryNull(CONNECTION, segments(ext, SQLIndexSegment), name, owner);
    QString degree = toShift(storage);
    QString instances = toShift(storage);
    QString compressed = toShift(storage);

    QString schema2 = ext.intSchema(tableOwner, false);

    QString ret;
    QString sql = QString("CREATE%1%2 INDEX %3%4 ON %5%6\n").
                  arg(unique).
                  arg(bitmap).
                  arg(schema).
                  arg(QUOTE(name)).
                  arg(schema2).
                  arg(QUOTE(table));
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n";
    }
    ret += sql;
    ret += indexColumns(ext, "", owner, name);
    if (domain == "DOMAIN")
    {
        if (CONNECTION.version() >= "0801" && domOwner == "CTXSYS" && domName == "CONTEXT")
        {
            ret = createContextPrefs(ext, schema, owner, name, ret);
            return ret;
        }
        ret += QString("INDEXTYPE IS %1.%2\nPARAMETERS ('%3');\n\n").
               arg(QUOTE(domOwner)).arg(QUOTE(domName)).arg(prepareDB(domParam));
        return ret;
    }
    if (CONNECTION.version() >= "0800" && ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty() )
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
        return createPartitionedIndex(ext, schema, owner, name, ret);

    toUnShift(storage, toQValue(""));
    ret += segmentAttributes(ext, storage);
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

QString toOracleExtract::createMaterializedView(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    return createMView(ext, schema, owner, name, "MATERIALIZED VIEW");
}

QString toOracleExtract::createMaterializedViewLog(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    return createMViewLog(ext, schema, owner, name, "MATERIALIZED VIEW");
}

QString toOracleExtract::createPackage(toExtract &ext,
                                       const QString &schema,
                                       const QString &owner,
                                       const QString &name) const
{
    return displaySource(ext, schema, owner, name, "PACKAGE", false);
}

QString toOracleExtract::createPackageBody(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    return displaySource(ext, schema, owner, name, "PACKAGE BODY", false);
}

QString toOracleExtract::createProcedure(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    return displaySource(ext, schema, owner, name, "PROCEDURE", false);
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

QString toOracleExtract::createProfile(toExtract &ext,
                                       const QString &,
                                       const QString &,
                                       const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLProfileInfo,
                                          name);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find profile %1").arg(name);

    QString ret;
    if (PROMPT)
        ret = QString("PROMPT CREATE PROFILE %1\n\n").arg(QUOTE(name));
    ret += QString("CREATE PROFILE %1\n").arg(QUOTE(name));

    while (!info.empty())
    {
        ret += "   ";
        ret += toShift(info);
        ret += " ";
        ret += toShift(info);
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

QString toOracleExtract::createRole(toExtract &ext,
                                    const QString &,
                                    const QString &,
                                    const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLRoleInfo,
                                          name);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find role %1").arg(name);

    QString ret;
    if (PROMPT)
        ret = QString("PROMPT CREATE ROLE %1\n\n").arg(QUOTE(name));
    ret += QString("CREATE ROLE %1 %2;\n\n").arg(QUOTE(name)).arg(toShift(info));
    ret += grantedPrivs(ext, QUOTE(name), name, 1);
    return ret;
}

static toSQL SQLRollbackSegment("toOracleExtract:RollbackSegment",
                                "SELECT  DECODE(\n"
                                "                r.owner\n"
                                "               ,'PUBLIC',' PUBLIC '\n"
                                "               ,         ' '\n"
                                "              )                                  AS is_public\n"
                                "      , r.tablespace_name\n"
                                "      , NVL(r.initial_extent,t.initial_extent)   AS initial_extent\n"
                                "      , NVL(r.next_extent,t.next_extent)         AS next_extent\n"
                                "      , r.min_extents\n"
                                "      , DECODE(\n"
                                "                r.max_extents\n"
                                "               ,2147483645,'unlimited'\n"
                                "               ,           r.max_extents\n"
                                "              )                                  AS max_extents\n"
                                "  FROM  sys.dba_rollback_segs    r\n"
                                "      , sys.dba_tablespaces  t\n"
                                " WHERE\n"
                                "            r.segment_name    = :nam<char[100]>\n"
                                "        AND t.tablespace_name = r.tablespace_name",
                                "Get information about rollback segment, "
                                "must have same binds and columns");

QString toOracleExtract::createRollbackSegment(toExtract &ext,
        const QString &,
        const QString &,
        const QString &name) const
{
    toQList result = toQuery::readQueryNull(CONNECTION, SQLRollbackSegment, name);
    QString isPublic = toShift(result);
    QString tablespaceName = toShift(result);
    QString initialExtent = toShift(result);
    QString nextExtent = toShift(result);
    QString minExtent = toShift(result);
    QString maxExtent = toShift(result);

    QString ret;
    if (PROMPT)
        ret += QString("PROMPT CREATE%1ROLLBACK SEGMENT %2\n\n").arg(isPublic).arg(QUOTE(name));
    ret += QString("CREATE%1ROLLBACK SEGMENT %2\n\n").arg(isPublic).arg(QUOTE(name));
    if (ext.getStorage())
    {
        ret += QString("STORAGE\n(\n"
                       "  INITIAL      %1\n").arg(initialExtent);
        if (!nextExtent.isEmpty())
            ret += QString("  NEXT         %1\n").arg(nextExtent);
        ret += QString("  MINEXTENTS   %1\n"
                       "  MAXEXTENTS   %2\n"
                       ")\n"
                       "TABLESPACE     %3;\n\n").
               arg(minExtent).arg(maxExtent).arg(QUOTE(tablespaceName));
    }
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

QString toOracleExtract::createSequence(toExtract &ext,
                                        const QString &schema,
                                        const QString &owner,
                                        const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLSequenceInfo,
                                          name, owner);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find sequence %1").arg(name);

    QString ret;
    if (PROMPT)
        ret = QString("PROMPT CREATE SEQUENCE %1%2\n\n").arg(schema).arg(QUOTE(name));
    ret += QString("CREATE SEQUENCE %1%2\n").arg(schema).arg(QUOTE(name));

    while (!info.empty())
    {
        ret += "   ";
        ret += toShift(info);
        ret += "\n";
    }
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::createSnapshot(toExtract &ext,
                                        const QString &schema,
                                        const QString &owner,
                                        const QString &name) const
{
    return createMViewLog(ext, schema, owner, name, "SNAPSHOT");
}

QString toOracleExtract::createSnapshotLog(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    return createMViewLog(ext, schema, owner, name, "SNAPSHOT LOG");
}

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

QString toOracleExtract::createSynonym(toExtract &ext,
                                       const QString &schema,
                                       const QString &owner,
                                       const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLSynonymInfo,
                                          name, owner);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find synonym %1.%2").arg(owner).arg(name);

    QString tableOwner = toShift(info);
    QString tableName = toShift(info);
    QString dbLink = toShift(info);
    if (dbLink == "NULL")
        dbLink = "";
    else
        dbLink = "@" + QUOTE(dbLink);
    QString useSchema = (schema == "PUBLIC") ? QString("") : schema;
    QString isPublic = (owner == "PUBLIC") ? " PUBLIC" : "";
    QString tableSchema = ext.intSchema(tableOwner, false);

    QString sql = QString("CREATE%1 SYNONYM %2%3 FOR %4%5%6").
                  arg(isPublic).arg(useSchema).arg(QUOTE(name)).
                  arg(tableSchema).arg(QUOTE(tableName)).arg(dbLink);
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
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

QString toOracleExtract::createTable(toExtract &ext,
                                     const QString &schema,
                                     const QString &owner,
                                     const QString &name) const
{
    toQuery inf(CONNECTION, SQLTableType, name, owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Couldn't find table %1.%2").arg(owner).arg(name);

    QString partitioned(inf.readValue());
    QString iot_type(inf.readValue());

    if (iot_type == "IOT")
    {
        if (partitioned == "YES")
            return createPartitionedIOT(ext, schema, owner, name);
        else
            return createIOT(ext, schema, owner, name);
    }
    else if (partitioned == "YES")
        return createPartitionedTable(ext, schema, owner, name);

    toQList result = toQuery::readQueryNull(CONNECTION, segments(ext, SQLTableInfo), name, owner);
    QString ret = createTableText(ext, result, schema, owner, name);
    ret += ";\n\n";
    ret += createComments(ext, schema, owner, name);
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
                                  QString::null,
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
                                 QString::null,
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

QString toOracleExtract::createTableFamily(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    QString ret = createTable(ext, schema, owner, name);

    toQuery inf(CONNECTION, SQLTableType, name, owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Couldn't find table %1.%2").arg(owner).arg(name);

    inf.readValue();
    QString iotType(inf.readValue());

    toQList constraints = toQuery::readQueryNull(CONNECTION, SQLTableConstraints, name, owner);
    toQList indexes = toQuery::readQueryNull(CONNECTION, SQLIndexNames, name, owner);

    while (!indexes.empty())
    {
        QString indOwner(toShift(indexes));
        QString indName(toShift(indexes));
        bool add
        = true;
        if (iotType == "IOT")
        {
            for (toQList::iterator i = constraints.begin();i != constraints.end();i++)
            {
                QString consType = *i;
                i++;
                QString consName = *i;
                i++;
                if (consType == "P" && consName == indName && owner == indOwner)
                {
                    add
                    = false;
                    break;
                }
            }
        }
        if (add
           )
            ret += createIndex(ext, ext.intSchema(indOwner, false), indOwner, indName);
    }

    while (!constraints.empty())
    {
        QString type = toShift(constraints);
        QString name = toShift(constraints);
        QString search = toShift(constraints);
        if ((type != "P" || iotType != "IOT") && !search.contains(" IS NOT NULL"))
            ret += createConstraint(ext, schema, owner, name);
    }

    toQList triggers = toQuery::readQueryNull(CONNECTION, SQLTableTriggers, name, owner);
    while (!triggers.empty())
        ret += createTrigger(ext, schema, owner, toShift(triggers));
    return ret;
}

void toOracleExtract::createTableContents(toExtract &ext,
        QTextStream &stream,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    if (ext.getContents())
    {
        if (PROMPT)
            stream << QString("PROMPT CONTENTS OF %1%2\n\n").arg(schema).arg(QUOTE(name));

        toQuery query(CONNECTION, QString("SELECT * FROM %1.%2").arg(QUOTE(owner)).arg(QUOTE(name)));

        toQDescList desc = query.describe();
        int cols = query.columns();
        bool first = true;
        QString dateformat(toConfigurationSingle::Instance().dateFormat());

        QString beg = QString("INSERT INTO %1%2 (").arg(schema).arg(QUOTE(name));
        bool *dates = new bool[desc.size()];
        try
        {
            int num = 0;
            for (toQDescList::iterator i = desc.begin();i != desc.end();i++)
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

            QRegExp find("'");

            int row = 0;
            int maxRow = ext.getCommitDistance();

            while (!query.eof())
            {
                QString line = beg;
                first = true;
                for (int i = 0;i < cols;i++)
                {
                    if (first)
                        first = false;
                    else
                        line += ",";
                    QString val = query.readValueNull();
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

QString toOracleExtract::createTableReferences(toExtract &ext,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    QString ret;
    toQList constraints = toQuery::readQueryNull(CONNECTION, SQLTableReferences, name, owner);
    while (!constraints.empty())
        ret += createConstraint(ext, schema, owner, toShift(constraints));
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

QString toOracleExtract::createTablespace(toExtract &ext,
        const QString &,
        const QString &,
        const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLTablespaceInfo,
                                          QString::number(ext.getBlockSize()),
                                          name);

    if (info.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find tablespace %1").arg(name);

    QString initial = toShift(info);
    QString next = toShift(info);
    QString minExtents = toShift(info);
    QString maxExtents = toShift(info);
    QString pctIncrease = toShift(info);
    QString minExtlen = toShift(info);
    QString contents = toShift(info);
    QString logging = toShift(info);
    QString extentManagement = toShift(info);
    QString allocationType = toShift(info);

    QString ret;
    QString sql;

    if (extentManagement == "LOCAL" && contents == "TEMPORARY")
        sql = QString("CREATE TEMPORARY TABLESPACE %1\n").arg(QUOTE(name));
    else
        sql = QString("CREATE TABLESPACE %1\n").arg(QUOTE(name));

    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n";
    }
    ret += sql;

    toQList files = toQuery::readQueryNull(CONNECTION,
                                           SQLDatafileInfo,
                                           QString::number(ext.getBlockSize()),
                                           name);
    if (extentManagement == "LOCAL" && contents == "TEMPORARY")
        ret += "TEMPFILE\n";
    else
        ret += "DATAFILE\n";
    QString comma = "  ";
    while (!files.empty())
    {
        QString fileName = toShift(files);
        QString bytes = toShift(files);
        QString autoExtensible = toShift(files);
        QString maxBytes = toShift(files);
        QString incrementBy = toShift(files);

        ret += QString("%1 '%2' SIZE %3 REUSE\n").
               arg(comma).
               arg(prepareDB(fileName)).
               arg(bytes);
        if (CONNECTION.version() >= "0800" && ext.getStorage())
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
            if (CONNECTION.version() >= "0801")
                ret += "EXTENT MANAGEMENT DICTIONARY\n";
        }
        if (CONNECTION.version() >= "0800" && (contents != "TEMPORARY" || extentManagement != "LOCAL"))
        {
            ret += logging;
            ret += "\n";
        }
    }
    ret += ";\n\n";
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

QString toOracleExtract::createTrigger(toExtract &ext,
                                       const QString &schema,
                                       const QString &owner,
                                       const QString &name) const
{
    if (!ext.getCode())
        return "";
    toQList result = toQuery::readQueryNull(CONNECTION, SQLTriggerInfo, name, owner);
    if (result.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find trigger %1.%2").arg(owner).arg(name);
    QString triggerType = toShift(result);
    QString event = toShift(result);
    QString tableOwner = toShift(result);
    QString table = toShift(result);
    QString baseType = toShift(result);
    QString refNames = toShift(result);
    QString description = toShift(result);
    QString when = toShift(result);
    QString body = toShift(result);
    QString status = toShift(result);

    QString trgType;
    if (triggerType.indexOf("BEFORE") >= 0)
        trgType = "BEFORE";
    else if (triggerType.indexOf("AFTER") >= 0)
        trgType = "AFTER";
    else if (triggerType.indexOf("INSTEAD OF") >= 0)
        trgType = "INSTEAD OF";


    QString trgPart = trgType + " " + event;
    QRegExp src("\\s" + trgPart + "\\s", Qt::CaseInsensitive);
    description.replace(QRegExp("\nON"), QString("\n ON"));
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
    if (baseType == "TABLE")
    {
        object = schema2;
        object += table;
    }
    else if (baseType == "SCHEMA")
    {
        object = schema;
        object += "SCHEMA";
    }
    else
    {
        object = baseType;
    }
    QString sql = QString("CREATE OR REPLACE TRIGGER %1%2\n").arg(schema).arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n";
    }
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
    ret += "\n/\n\n";
    if (status != "ENABLE")
    {
        ret += QString("ALTER TRIGGER %1%2 DISABLE;\n\n").arg(schema).arg(QUOTE(name));
    }
    return ret;
}

QString toOracleExtract::createType(toExtract &ext,
                                    const QString &schema,
                                    const QString &owner,
                                    const QString &name) const
{
    return displaySource(ext, schema, owner, name, "TYPE", false);
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

QString toOracleExtract::createUser(toExtract &ext,
                                    const QString &,
                                    const QString &,
                                    const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLUserInfo,
                                          name);

    if (info.size() != 4)
        throw qApp->translate("toOracleExtract", "Couldn't find user %1").arg(name);

    QString password = toShift(info);
    QString profile = toShift(info);
    QString defaultTablespace = toShift(info);
    QString temporaryTablespace = toShift(info);

    QString ret;
    QString nam;
    if (ext.getSchema() != "1" && !ext.getSchema().isEmpty())
        nam = ext.getSchema().toLower();
    else
        nam = QUOTE(name);
    if (PROMPT)
        ret = QString("PROMPT CREATE USER %1\n\n").arg(nam);
    ret += QString("CREATE USER %1 IDENTIFIED %2\n"
                   "   PROFILE              %3\n"
                   "   DEFAULT TABLESPACE   %4\n"
                   "   TEMPORARY TABLESPACE %5\n").
           arg(nam).
           arg(password).
           arg(QUOTE(profile)).
           arg(QUOTE(defaultTablespace)).
           arg(QUOTE(temporaryTablespace));

    if (ext.getStorage())
    {
        toQList quota = toQuery::readQueryNull(CONNECTION, SQLUserQuotas, name);
        while (!quota.empty())
        {
            QString siz = toShift(quota);
            QString tab = toShift(quota);
            ret += QString("   QUOTA %1 ON %2\n").
                   arg(siz).
                   arg(QUOTE(tab));
        }
    }
    ret += ";\n\n";
    ret += grantedPrivs(ext, nam, name, 3);
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

QString toOracleExtract::createView(toExtract &ext,
                                    const QString &schema,
                                    const QString &owner,
                                    const QString &name) const
{
    if (!ext.getCode())
        return "";
    toQList source = toQuery::readQueryNull(CONNECTION,
                                            SQLViewSource,
                                            name, owner);
    if (source.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find view %1.%2").arg(QUOTE(owner)).arg(QUOTE(name));

    QString text = toShift(source);
    QString ret;
    QString sql = QString("CREATE OR REPLACE VIEW %1%2\n").arg(schema).arg(QUOTE(name));
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n";
    }
    ret += sql;
    toQList cols = toQuery::readQueryNull(CONNECTION,
                                          SQLViewColumns,
                                          name, owner);
    ret += "(";
    QString sep = "\n    ";
    while (!cols.empty())
    {
        QString str = toShift(cols);
        ret += sep;
        ret += QUOTE(str);
        sep = "\n  , ";
    }
    ret += "\n)\n";
    ret += "AS\n";
    ret += text;
    ret += ";\n\n";
    ret += createComments(ext, schema, owner, name);
    return ret;
}

// Implementation describe functions

void toOracleExtract::describeConstraint(toExtract &ext,
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    if (ext.getConstraints())
    {
        toQuery inf(CONNECTION, SQLListConstraint, owner, name);
        if (inf.eof())
            throw qApp->translate("toOracleExtract", "Constraint %1.%2 doesn't exist").arg(owner).arg(name);
        QString table(inf.readValue());
        QString tchr(inf.readValue());
        QString search(inf.readValue());
        QString rOwner(inf.readValue());
        QString rName(inf.readValue());
        QString delRule(inf.readValue());
        QString status(inf.readValue());
        QString defferable(inf.readValue());
        QString deffered(inf.readValue());

        QString type =
            (tchr == "P") ? "PRIMARY KEY" :
            (tchr == "U") ? "UNIQUE" :
            (tchr == "R") ? "FOREIGN KEY" :
            "CHECK";

        std::list<QString> ctx;
        ctx.insert(ctx.end(), schema);
        ctx.insert(ctx.end(), "TABLE");
        ctx.insert(ctx.end(), QUOTE(table));
        ctx.insert(ctx.end(), "CONSTRAINT");
        ctx.insert(ctx.end(), QUOTE(name));

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
            ret += constraintColumns(ext, owner, name).simplified();

            if (tchr == "R")
            {
                toQuery query(CONNECTION, SQLConstraintTable, rOwner, rName);
                ret += " REFERENCES ";
                ret += ext.intSchema(rOwner, false);
                ret += QUOTE(query.readValue());
                ret += constraintColumns(ext, rOwner, rName);

                if (delRule == "CASCADE")
                    ret += "ON DELETE CASCADE";
            }
        }
        addDescription(lst, ctx, "DEFINITION", ret);

        if (status.startsWith("ENABLE"))
            status = "ENABLED";

        if (CONNECTION.version() < "08")
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

void toOracleExtract::describeDBLink(toExtract &ext,
                                     std::list<QString> &lst,
                                     const QString &,
                                     const QString &owner,
                                     const QString &name) const
{
    toQuery inf(CONNECTION, SQLDBLink, owner, name);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "DBLink %1.%2 doesn't exist").arg(owner).arg(name);
    QString user(inf.readValue());
    QString password(inf.readValue());
    QString host(inf.readValue());
    QString publ = (owner == "PUBLIC") ? "PUBLIC" : "";
    QString sql = QString("CREATE%1 DATABASE LINK %2").arg(publ).arg(QUOTE(name));
    QString ret;

    std::list<QString> ctx;
    ctx.insert(ctx.end(), "DATABASE LINK");
    ctx.insert(ctx.end(), QUOTE(name));

    addDescription(lst, ctx, publ, QString("%1 IDENTIFIED BY %2 USING '%3'").
                   arg(user.toLower()).
                   arg(password.toLower()).
                   arg(prepareDB(host)));
}

void toOracleExtract::describeExchangeIndex(toExtract &ext,
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    QStringList str = name.split(":");
    if (str.count() != 2)
        throw ("When calling createExchangeIndex name should contain :");
    QString segment = str.first();
    QString partition = str.last();

    toQuery inf(CONNECTION, segments(ext, SQLPartitionSegmentType), segment, partition, owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Exchange index %1.%2 doesn't exist").arg(owner).arg(name);
    QString type = inf.readValue();
    QString blocks = inf.readValue();

    QString sql = toSQL::string(SQLExchangeIndex, CONNECTION).arg(type).arg(blocks);
    toQList result = toQuery::readQueryNull(CONNECTION, sql, segment, partition, owner);
    QString degree = toShift(result);
    QString instances = toShift(result);
    QString table = toShift(result);
    QString unique = toShift(result);
    QString bitmap = toShift(result);

    toUnShift(result, toQValue(""));

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), "TABLE");
    ctx.insert(ctx.end(), QUOTE(table));
    ctx.insert(ctx.end(), "EXCHANGE INDEX");
    ctx.insert(ctx.end(), segment);
    ctx.insert(ctx.end(), partition);

    describeIndexColumns(ext, lst, ctx, owner, segment);
    addDescription(lst, ctx, QString("%1%2").arg(unique).arg(bitmap));
    addDescription(lst, ctx, QString("%1%2").arg(segment).arg(table));
    if (ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty())
    {
        addDescription(lst, ctx, "PARALLEL", "DEGREE", degree);
        addDescription(lst, ctx, "PARALLEL", "INSTANCES", instances);
    }
    describeAttributes(ext, lst, ctx, result);
}

void toOracleExtract::describeExchangeTable(toExtract &ext,
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    QStringList str = name.split(":");
    if (str.count() != 2)
        throw ("When calling createExchangeTable name should contain :");
    QString segment = str.first();
    QString partition = str.last();

    toQuery inf(CONNECTION, segments(ext, SQLPartitionSegmentType), segment, partition, owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Exchange table %1.%2 doesn't exist").arg(owner).arg(name);
    QString type = inf.readValue();
    QString blocks = inf.readValue();

    QString sql = toSQL::string(SQLExchangeTable, CONNECTION).arg(type).arg(blocks);
    toQList result = toQuery::readQueryNull(CONNECTION, sql, segment, partition, owner);
    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), "EXCHANGE TABLE");
    ctx.insert(ctx.end(), segment);
    ctx.insert(ctx.end(), partition);
    describeTableText(ext, lst, ctx, result, schema, owner, segment);
}

void toOracleExtract::describeFunction(toExtract &ext,
                                       std::list<QString> &lst,
                                       const QString &schema,
                                       const QString &owner,
                                       const QString &name) const
{
    describeSource(ext, lst, schema, owner, name, "FUNCTION");
}

void toOracleExtract::describeIndex(toExtract &ext,
                                    std::list<QString> &lst,
                                    const QString &schema,
                                    const QString &owner,
                                    const QString &name) const
{
    if (!ext.getIndexes())
        return ;

    toQList res = toQuery::readQueryNull(CONNECTION, SQLIndexInfo, name, owner);
    if (res.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find index %1.%2").arg(owner).arg(name);

    QString partitioned = toShift(res);
    QString table = toShift(res);
    QString tableOwner = toShift(res);
    QString reverse = toShift(res);
    QString unique = toShift(res);
    QString bitmap = toShift(res);
    QString domain = toShift(res);
    QString domOwner = toShift(res);
    QString domName = toShift(res);
    QString domParam = toShift(res);

    toQList storage = toQuery::readQueryNull(CONNECTION, segments(ext, SQLIndexSegment), name, owner);
    QString degree = toShift(storage);
    QString instances = toShift(storage);
    QString compressed = toShift(storage);

    QString schema2 = ext.intSchema(tableOwner, false);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), QString("INDEX"));
    ctx.insert(ctx.end(), QUOTE(name));
    addDescription(lst, ctx, "ON", schema2 + QUOTE(table));
    addDescription(lst, ctx, QString("%1%2 INDEX").arg(unique).arg(bitmap).mid(1));

    addDescription(lst, ctx);
    if (!reverse.isEmpty())
        addDescription(lst, ctx, reverse);
    describeIndexColumns(ext, lst, ctx, owner, name);
    if (domain == "DOMAIN")
    {
        addDescription(lst, ctx, "DOMAIN", QString("INDEXTYPE IS DOMAINOWNER %1.%2").
                       arg(QUOTE(domOwner)).
                       arg(QUOTE(domName)));
        addDescription(lst, ctx, "DOMAIN", QString("PARAMETERS %1").arg(domParam));
        return ;
    }
    if (CONNECTION.version() >= "0800" && ext.getParallel() && ! degree.isEmpty() && ! instances.isEmpty())
    {
        addDescription(lst, ctx, "PARALLEL", QString("DEGREE %1").arg(degree));
        addDescription(lst, ctx, "PARALLEL", QString("INSTANCES %1").arg(instances));
    }
    if (partitioned == "YES")
    {
        describePartitionedIndex(ext, lst, ctx, schema, owner, name);
        return ;
    }

    toUnShift(storage, toQValue(""));
    describeAttributes(ext, lst, ctx, storage);
    if (!compressed.isEmpty() && compressed != "0" && ext.getStorage())
        addDescription(lst, ctx, "COMPRESS", compressed);
}

void toOracleExtract::describeMaterializedView(toExtract &ext,
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    describeMView(ext, lst, schema, owner, name, "MATERIALIZED VIEW");
}

void toOracleExtract::describeMaterializedViewLog(toExtract &ext,
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    describeMViewLog(ext, lst, schema, owner, name, "MATERIALIZED VIEW LOG");
}

void toOracleExtract::describePackage(toExtract &ext,
                                      std::list<QString> &lst,
                                      const QString &schema,
                                      const QString &owner,
                                      const QString &name) const
{
    describeSource(ext, lst, schema, owner, name, "PACKAGE");
}

void toOracleExtract::describePackageBody(toExtract &ext,
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    describeSource(ext, lst, schema, owner, name, "PACKAGE BODY");
}

void toOracleExtract::describeProcedure(toExtract &ext,
                                        std::list<QString> &lst,
                                        const QString &schema,
                                        const QString &owner,
                                        const QString &name) const
{
    describeSource(ext, lst, schema, owner, name, "PROCEDURE");
}

void toOracleExtract::describeProfile(toExtract &ext,
                                      std::list<QString> &lst,
                                      const QString &,
                                      const QString &,
                                      const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLProfileInfo,
                                          name);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find profile %1").arg(name);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), "NONE");
    ctx.insert(ctx.end(), "PROFILE");
    ctx.insert(ctx.end(), QUOTE(name));
    addDescription(lst, ctx);

    while (!info.empty())
    {
        QString nam = toShift(info);
        QString val = toShift(info);
        addDescription(lst, ctx, nam, val);
    }
}

void toOracleExtract::describeRole(toExtract &ext,
                                   std::list<QString> &lst,
                                   const QString &,
                                   const QString &,
                                   const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLRoleInfo,
                                          name);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find role %1").arg(name);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), "NONE");
    ctx.insert(ctx.end(), "ROLE");
    ctx.insert(ctx.end(), QUOTE(name));
    addDescription(lst, ctx);
    addDescription(lst, ctx, "INFO", toShift(info));
    describePrivs(ext, lst, ctx, name);
}

void toOracleExtract::describeRollbackSegment(toExtract &ext,
        std::list<QString> &lst,
        const QString &,
        const QString &,
        const QString &name) const
{
    toQList result = toQuery::readQueryNull(CONNECTION, SQLRollbackSegment, name);
    QString isPublic = toShift(result);
    QString tablespaceName = toShift(result);
    QString initialExtent = toShift(result);
    QString nextExtent = toShift(result);
    QString minExtent = toShift(result);
    QString maxExtent = toShift(result);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), "NONE");
    ctx.insert(ctx.end(), "ROLLBACK SEGMENT");
    ctx.insert(ctx.end(), QUOTE(name));

    addDescription(lst, ctx);
    addDescription(lst, ctx, QString("TABLESPACE %1").arg(QUOTE(tablespaceName)));

    if (ext.getStorage())
    {
        ctx.insert(ctx.end(), "STORAGE");
        addDescription(lst, ctx, QString("INITIAL %1").arg(initialExtent));
        if (!nextExtent.isEmpty())
            addDescription(lst, ctx, QString("NEXT %1").arg(nextExtent));
        addDescription(lst, ctx, QString("MINEXTENTS %1").arg(minExtent));
        addDescription(lst, ctx, QString("MAXEXTENTS %1").arg(maxExtent));
    }
}

void toOracleExtract::describeSequence(toExtract &ext,
                                       std::list<QString> &lst,
                                       const QString &schema,
                                       const QString &owner,
                                       const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLSequenceInfo,
                                          name, owner);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find sequence %1").arg(name);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), "SEQUENCE");
    ctx.insert(ctx.end(), QUOTE(name));

    addDescription(lst, ctx);

    while (!info.empty())
        addDescription(lst, ctx, toShift(info));
}

void toOracleExtract::describeSnapshot(toExtract &ext,
                                       std::list<QString> &lst,
                                       const QString &schema,
                                       const QString &owner,
                                       const QString &name) const
{
    describeMView(ext, lst, schema, owner, name, "SNAPSHOT");
}

void toOracleExtract::describeSnapshotLog(toExtract &ext,
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    describeMViewLog(ext, lst, schema, owner, name, "SNAPSHOT LOG");
}

void toOracleExtract::describeSynonym(toExtract &ext,
                                      std::list<QString> &lst,
                                      const QString &,
                                      const QString &owner,
                                      const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLSynonymInfo,
                                          name, owner);
    if (info.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find synonym %1.%2").arg(owner).arg(name);

    QString tableOwner = toShift(info);
    QString tableName = toShift(info);
    QString dbLink = toShift(info);
    if (dbLink == "NULL")
        dbLink = "";
    else
        dbLink = "@" + QUOTE(dbLink);
    QString tableSchema = ext.intSchema(tableOwner, true);

    std::list<QString> ctx;

    ctx.insert(ctx.end(), owner);
    addDescription(lst, ctx, "SYNONYM", QString("%1%2%3").
                   arg(tableSchema).arg(QUOTE(tableName)).arg(dbLink));
}

void toOracleExtract::describeTable(toExtract &ext,
                                    std::list<QString> &lst,
                                    const QString &schema,
                                    const QString &owner,
                                    const QString &name) const
{
    toQuery inf(CONNECTION, SQLTableType, name, owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Couldn't find table %1.%2").arg(owner).arg(name);

    QString partitioned(inf.readValue());
    QString iot_type(inf.readValue());

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), "TABLE");
    ctx.insert(ctx.end(), QUOTE(name));

    if (iot_type == "IOT" && ext.getIndexes())
    {
        if (partitioned == "YES")
            describePartitionedIOT(ext, lst, ctx, schema, owner, name);
        else
            describeIOT(ext, lst, ctx, schema, owner, name);
        return ;
    }
    else if (partitioned == "YES")
    {
        describePartitionedTable(ext, lst, ctx, schema, owner, name);
        return ;
    }

    toQList result = toQuery::readQueryNull(CONNECTION, segments(ext, SQLTableInfo), name, owner);
    describeTableText(ext, lst, ctx, result, schema, owner, name);
    describeComments(ext, lst, ctx, owner, name);
}

void toOracleExtract::describeTableFamily(toExtract &ext,
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    describeTable(ext, lst, schema, owner, name);

    toQList indexes = toQuery::readQueryNull(CONNECTION, SQLIndexNames, name, owner);
    while (!indexes.empty())
    {
        QString indOwner(toShift(indexes));
        describeIndex(ext, lst, ext.intSchema(indOwner, true), indOwner, toShift(indexes));
    }

    toQuery inf(CONNECTION, SQLTableType, name, owner);
    if (inf.eof())
        throw qApp->translate("toOracleExtract", "Couldn't find table %1.%2").arg(owner).arg(name);

    inf.readValue();
    QString iotType(inf.readValue());

    toQList constraints = toQuery::readQueryNull(CONNECTION, SQLTableConstraints, name, owner);
    while (!constraints.empty())
    {
        if (toShift(constraints).toUtf8() != "P" || iotType != "IOT")
            describeConstraint(ext, lst, schema, owner, toShift(constraints));
        else
            toShift(constraints);
        toShift(constraints);
    }

    toQList triggers = toQuery::readQueryNull(CONNECTION, SQLTableTriggers, name, owner);
    while (!triggers.empty())
        describeTrigger(ext, lst, schema, owner, toShift(triggers));
}

void toOracleExtract::describeTableReferences(toExtract &ext,
        std::list<QString> &lst,
        const QString &schema,
        const QString &owner,
        const QString &name) const
{
    toQList constraints = toQuery::readQueryNull(CONNECTION, SQLTableReferences, name, owner);
    while (!constraints.empty())
        describeConstraint(ext, lst, schema, owner, toShift(constraints));
}

void toOracleExtract::describeTablespace(toExtract &ext,
        std::list<QString> &lst,
        const QString &,
        const QString &,
        const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLTablespaceInfo,
                                          QString::number(ext.getBlockSize()),
                                          name);

    if (info.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find tablespace %1").arg(name);

    QString initial = toShift(info);
    QString next = toShift(info);
    QString minExtents = toShift(info);
    QString maxExtents = toShift(info);
    QString pctIncrease = toShift(info);
    QString minExtlen = toShift(info);
    QString contents = toShift(info);
    QString logging = toShift(info);
    QString extentManagement = toShift(info);
    QString allocationType = toShift(info);

    std::list<QString> ctx;

    ctx.insert(ctx.end(), "NONE");
    if (extentManagement == "LOCAL" && contents == "TEMPORARY")
        ctx.insert(ctx.end(), "TEMPORARY TABLESPACE");
    else
        ctx.insert(ctx.end(), "TABLESPACE");
    ctx.insert(ctx.end(), QUOTE(name));

    addDescription(lst, ctx);

    toQList files = toQuery::readQueryNull(CONNECTION,
                                           SQLDatafileInfo,
                                           QString::number(ext.getBlockSize()),
                                           name);
    while (!files.empty())
    {
        QString fileName = toShift(files);
        QString bytes = toShift(files);
        QString autoExtensible = toShift(files);
        QString maxBytes = toShift(files);
        QString incrementBy = toShift(files);

        std::list<QString> cctx = ctx;
        if (extentManagement == "LOCAL" && contents == "TEMPORARY")
            cctx.insert(cctx.end(), "TEMPFILE");
        else
            cctx.insert(cctx.end(), "DATAFILE");
        cctx.insert(cctx.end(), prepareDB(fileName));

        addDescription(lst, cctx);
        addDescription(lst, cctx, QString("SIZE %3").arg(bytes));

        if (CONNECTION.version() >= "0800" && ext.getStorage())
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
        if (CONNECTION.version() >= "0800" && (contents != "TEMPORARY" || extentManagement != "LOCAL"))
            addDescription(lst, ctx, logging);
    }
}

void toOracleExtract::describeTrigger(toExtract &ext,
                                      std::list<QString> &lst,
                                      const QString &schema,
                                      const QString &owner,
                                      const QString &name) const
{
    if (!ext.getCode())
        return ;

    toQList result = toQuery::readQueryNull(CONNECTION, SQLTriggerInfo, name, owner);
    if (result.size() != 10)
        throw qApp->translate("toOracleExtract", "Couldn't find trigger %1.%2").arg(owner).arg(name);
    QString triggerType = toShift(result);
    QString event = toShift(result);
    QString tableOwner = toShift(result);
    QString table = toShift(result);
    QString baseType = toShift(result);
    QString refNames = toShift(result);
    QString description = toShift(result);
    QString when = toShift(result);
    QString body = toShift(result);
    QString status = toShift(result);

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
    description.replace(QRegExp("\nON"), QString("\n ON"));
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
    if (baseType == "TABLE")
    {
        object = schema2;
        if (!schema2.isEmpty())
            object += ".";
        object += table;
    }
    else if (baseType == "SCHEMA")
    {
        object = schema;
        if (!schema.isEmpty())
            object += ".";
        object += "SCHEMA";
    }
    else
    {
        object = baseType;
    }

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), "TRIGGER");
    ctx.insert(ctx.end(), QUOTE(name));
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

void toOracleExtract::describeType(toExtract &ext,
                                   std::list<QString> &lst,
                                   const QString &schema,
                                   const QString &owner,
                                   const QString &name) const
{
    describeSource(ext, lst, schema, owner, name, "TYPE");
}

void toOracleExtract::describeUser(toExtract &ext,
                                   std::list<QString> &lst,
                                   const QString &,
                                   const QString &,
                                   const QString &name) const
{
    toQList info = toQuery::readQueryNull(CONNECTION,
                                          SQLUserInfo,
                                          name);

    if (info.size() != 4)
        throw qApp->translate("toOracleExtract", "Couldn't find user %1").arg(name);

    QString password = toShift(info);
    QString profile = toShift(info);
    QString defaultTablespace = toShift(info);
    QString temporaryTablespace = toShift(info);

    std::list<QString> ctx;
    ctx.insert(ctx.end(), "NONE");
    ctx.insert(ctx.end(), "USER");
    QString nam;
    if (ext.getSchema() != "1" && !ext.getSchema().isEmpty())
        nam = ext.getSchema().toLower();
    else
        nam = QUOTE(name);
    ctx.insert(ctx.end(), nam);

    addDescription(lst, ctx);
    addDescription(lst, ctx, QString("PROFILE %1").arg(QUOTE(profile)));
    addDescription(lst, ctx, QString("DEFAULT TABLESPACE %1").arg(QUOTE(defaultTablespace)));
    addDescription(lst, ctx, QString("TEMPORARY TABLESPACE %1").arg(QUOTE(temporaryTablespace)));

    if (ext.getStorage())
    {
        toQList quota = toQuery::readQueryNull(CONNECTION, SQLUserQuotas, name);
        while (!quota.empty())
        {
            QString siz = toShift(quota);
            QString tab = toShift(quota);
            addDescription(lst, ctx, "QUOTA", QString("%1 ON %2").
                           arg(siz).
                           arg(QUOTE(tab)));
        }
    }
    describePrivs(ext, lst, ctx, name);
}

void toOracleExtract::describeView(toExtract &ext,
                                   std::list<QString> &lst,
                                   const QString &schema,
                                   const QString &owner,
                                   const QString &name) const
{
    if (!ext.getCode())
        return ;
    toQList source = toQuery::readQueryNull(CONNECTION,
                                            SQLViewSource,
                                            name, owner);
    if (source.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find view %1.%2").arg(QUOTE(owner)).arg(QUOTE(name));

    std::list<QString> ctx;
    ctx.insert(ctx.end(), schema);
    ctx.insert(ctx.end(), "VIEW");
    ctx.insert(ctx.end(), QUOTE(name));

    addDescription(lst, ctx);
    QString text = toShift(source);
    addDescription(lst, ctx, "AS", text);
    describeComments(ext, lst, ctx, owner, name);

    toQuery query(CONNECTION, "SELECT * FROM " + QUOTE(owner) + "." + QUOTE(name) + " WHERE NULL = NULL");

    toQDescList desc = query.describe();
    int num = 1;
    for (toQDescList::iterator i = desc.begin();i != desc.end();i++)
    {
        addDescription(lst, ctx, "COLUMN", (*i).Name);
        addDescription(lst, ctx, "COLUMN", (*i).Name, (*i).Datatype);
        addDescription(lst, ctx, "COLUMN", (*i).Name, "ORDER", QString::number(num));
        num++;
    }
}

// Implementation drop functions

QString toOracleExtract::dropConstraint(toExtract &ext,
                                        const QString &schema,
                                        const QString &owner,
                                        const QString &,
                                        const QString &name) const
{
    toQList tableName = toQuery::readQueryNull(CONNECTION,
                        SQLConstraintTable,
                        owner, name);
    if (tableName.empty())
        throw qApp->translate("toOracleExtract", "Couldn't find constraint %1.%2").
        arg(QUOTE(owner)).arg(QUOTE(name));
    QString sql = QString("ALTER TABLE %1%2 DROP CONSTRAINT %3").
                  arg(schema).
                  arg(QUOTE(toShift(tableName))).
                  arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::dropDatabaseLink(toExtract &ext,
        const QString &,
        const QString &owner,
        const QString &,
        const QString &name) const
{
    QString sql = QString("DROP%1 DATABASE LINK %2").
                  arg((owner == "PUBLIC") ? " PUBLIC" : "").
                  arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::dropMViewLog(toExtract &ext,
                                      const QString &schema,
                                      const QString &,
                                      const QString &type,
                                      const QString &name) const
{
    QString sql = QString("DROP %1 ON %2%3").arg(type).arg(schema).arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += ";\n\n";

    return ret;
}

QString toOracleExtract::dropObject(toExtract &ext,
                                    const QString &,
                                    const QString &,
                                    const QString &type,
                                    const QString &name) const
{
    QString sql = QString("DROP %1 %2").arg(type).arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::dropProfile(toExtract &ext,
                                     const QString &,
                                     const QString &,
                                     const QString &,
                                     const QString &name) const
{
    QString sql = QString("DROP PROFILE %1 CASCADE").arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::dropSchemaObject(toExtract &ext,
        const QString &schema,
        const QString &,
        const QString &type,
        const QString &name) const
{
    QString sql = QString("DROP %1 %2%3").arg(type).arg(schema).arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::dropSynonym(toExtract &ext,
                                     const QString &schema,
                                     const QString &owner,
                                     const QString &,
                                     const QString &name) const
{
    QString sql = QString("DROP%1 SYNONYM %2%3").
                  arg((owner == "PUBLIC") ? " PUBLIC" : "").
                  arg((owner == "PUBLIC") ? schema : QString("")).
                  arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::dropTable(toExtract &ext,
                                   const QString &schema,
                                   const QString &,
                                   const QString &,
                                   const QString &name) const
{
    QString sql = QString("DROP TABLE %1%2 CASCADE CONSTRAINTS").
                  arg(schema).
                  arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::dropTablespace(toExtract &ext,
                                        const QString &,
                                        const QString &,
                                        const QString &,
                                        const QString &name) const
{
    QString sql = QString("DROP TABLESPACE %1 INCLUDING CONTENTS CASCADE CONSTRAINTS").
                  arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += ";\n\n";
    return ret;
}

QString toOracleExtract::dropUser(toExtract &ext,
                                  const QString &,
                                  const QString &,
                                  const QString &,
                                  const QString &name) const
{
    QString sql = QString("DROP USER %1 CASCADE").
                  arg(QUOTE(name));
    QString ret;
    if (PROMPT)
    {
        ret = "PROMPT ";
        ret += sql;
        ret += "\n\n";
    }
    ret += sql;
    ret += ";\n\n";
    return ret;
}

// Internal utility migration functions

static bool SameContext(const QString &str, const QString &str2)
{
    if (str.startsWith(str2))
    {
        if (str.length() == str2.length())
            return true;
        if (str.length() > str2.length() &&
                str.at(str2.length()) == QChar('\001'))
            return true;
    }
    return false;
}

static std::list<QString>::iterator FindItem(std::list<QString> &desc,
        std::list<QString>::iterator i,
        const QString &context,
        int level = -1,
        const QString & search = QString::null)
{
    if (search.isNull() || level < 0)
    {
        while (i != desc.end() && *i < context)
            i++;
        if (i == desc.end() || !SameContext(*i, context))
            return desc.end();
        return i;
    }
    else
    {
        while (i != desc.end())
        {
            if (!SameContext(*i, context) && !context.isEmpty())
                return desc.end();
            if (toExtract::partDescribe(*i, level) == search)
                return i;
            i++;
        }
        return desc.end();
    }
}

QString toOracleExtract::migrateAttributes(std::list<QString> &desc,
        std::list<QString>::iterator i,
        int level,
        const QString &indent) const
{
    QString after;
    QString storage;
    QString parallel;
    QString ret;
    QString partition;

    QString context = toExtract::contextDescribe(*i, level);
    if (context.isNull())
        return QString::null;

    while (i != desc.end())
    {
        if (!SameContext(*i, context))
            break;
        QString type = toExtract::partDescribe(*i, level);
        QString par = toExtract::partDescribe(*i, level + 1);
        if (type == "PARAMETER")
        {
            if (par.startsWith("TABLESPACE"))
                after += indent + par + "\n";
            else
                ret += indent + par + "\n";
        }
        else if (type == "STORAGE")
            storage += indent + "  " + par + "\n";
        else if (type == "PARALLEL")
            parallel += indent + "  " + par + "\n";
        else if (type == "LOCAL PARTITION")
        {
            if (partition.isEmpty())
            {
                partition += indent + "LOCAL\n";
                partition += migratePartitions(desc, i, level + 2, indent + "  ");
            }
        }
        else if (type == "GLOBAL PARTITION COLUMNS" ||
                 type.startsWith("PARTITION BY "))
            {}
        i++;
    }

    if (!parallel.isEmpty())
        ret.prepend(indent + "PARALLEL\n" + indent + "(\n" + parallel + indent + ")\n");

    if (!storage.isEmpty())
        ret += indent + "STORAGE\n" + indent + "(\n" + storage + indent + ")\n";

    ret += after;
    return ret;
}

QString toOracleExtract::migrateIndexColumns(std::list<QString> &destin,
        std::list<QString>::iterator i,
        const QString &context) const
{
    std::map<int, QString> cols;
    QString ret;
    do
    {
        if (!SameContext(*i, context))
            break;
        QString col = toExtract::partDescribe(*i, 4);
        QString ord = toExtract::partDescribe(*i, 5);
        if (!col.isNull() && !col.isNull())
            cols[ord.toInt()] = col;
        i++;
    }
    while (i != destin.end());
    ret += "(\n";
    for (int j = 1;!cols[j].isNull();j++)
    {
        if (j == 1)
            ret += "    ";
        else
            ret += "  , ";
        ret += cols[j] + "\n";
    }
    ret += ")\n";
    return ret;
}

QString toOracleExtract::migratePartitions(std::list<QString> &desc,
        std::list<QString>::iterator &i,
        int level,
        const QString &indent) const
{
    QString ret;

    QString context = toExtract::contextDescribe(*i, level);
    if (context.isNull())
        return QString::null;

    while (i != desc.end())
    {
        if (!SameContext(*i, context))
            break;
        QString partition = toExtract::partDescribe(*i, level);
        QString cc = toExtract::contextDescribe(*i, level + 1);
        ret += indent + "PARTITION " + partition;
        std::list<QString>::iterator t = FindItem(desc, i, cc, level + 1, "RANGE");
        if (t != desc.end())
            ret += toExtract::partDescribe(*t, level + 2);
        ret += "\n";
        ret += migrateAttributes(desc, i, level + 1, indent + "  ");
        t = FindItem(desc, i, cc, level + 1, "HASH");

        if (t != desc.end())
        {
            ret += indent + "  (\n";
            QString ind = indent + "    ";
            while (t != desc.end())
            {
                if (!SameContext(*t, cc))
                {
                    i = t;
                    break;
                }
                if (toExtract::partDescribe(*t, level + 1) != "HASH")
                    break;
                ret += ind + toExtract::partDescribe(*t, level + 2);
                ind = indent + "  , ";
                t++;
            }
            ret += indent + "  )\n";
        }

        while (i != desc.end())
        {
            if (!SameContext(*i, cc))
                break;
            i++;
        }
    }
    return ret;
}

QString toOracleExtract::migrateSource(toExtract &ext,
                                       std::list<QString> &source,
                                       std::list<QString> &destin,
                                       const QString &sourcetype) const
{
    std::list<QString> drop;
    std::list<QString> create;

    QString ret;

    toExtract::srcDst2DropCreate(source, destin, drop, create);

    QString lastOwner;
    QString lastName;

    {
        for (std::list<QString>::iterator i = drop.begin();i != drop.end();i++)
        {
            std::list<QString> ctx = toExtract::splitDescribe(*i);
            QString owner = toShift(ctx);
            QString type = toShift(ctx);
            if (type != sourcetype)
                continue;
            QString name = toShift(ctx);

            if (lastOwner != owner || name != lastName)
            {
                QString sql = "DROP " + sourcetype + " " + owner + "." + name;
                if (PROMPT)
                    ret += "PROMPT " + sql + "\n\n";
                ret += sql + ";\n\n";
                lastOwner = owner;
                lastName = name;
            }
        }
    }
    lastOwner = lastName = QString::null;

    for (std::list<QString>::iterator i = create.begin();i != create.end();i++)
    {
        std::list<QString> ctx = toExtract::splitDescribe(*i);

        QString owner = toShift(ctx);
        QString type = toShift(ctx);
        if (type != sourcetype)
            continue;
        QString name = toShift(ctx);
        QString source = toShift(ctx);

        if (lastOwner != owner || (name != lastName && !source.isEmpty()))
        {
            QString sql = "CREATE " + sourcetype + " " + owner + "." + name;
            if (PROMPT)
                ret += "PROMPT " + sql + "\n\n";
            ret += source + "\n\n";
            lastOwner = owner;
            lastName = name;
        }
    }

    return ret;
}

QString toOracleExtract::migratePrivs(toExtract &ext,
                                      std::list<QString> &source,
                                      std::list<QString> &destin,
                                      const QString &onlyGrantee) const
{
    std::list<QString> drop;
    std::list<QString> create;

    QString ret;

    toExtract::srcDst2DropCreate(source, destin, drop, create);

    {
        for (std::list<QString>::iterator i = drop.begin();i != drop.end();i++)
        {
            std::list<QString> ctx = toExtract::splitDescribe(*i);
            if (toShift(ctx) != "NONE")
                continue;
            QString grantee = toShift(ctx);
            if (!onlyGrantee.isEmpty() && grantee != onlyGrantee)
                continue;
            QString priv = toShift(ctx);
            QString admin = toShift(ctx);
            if (priv.isEmpty())
                continue;

            if (admin == "ON")
            {
                QString object = toShift(ctx);
                admin = toShift(ctx);
                QString sql = "REVOKE " + priv + " ON " + object + " FROM " + grantee;
                if (PROMPT)
                    ret += sql + "\n\n";
                ret += sql + ";\n\n";
            }
            else
            {
                QString sql = "REVOKE " + priv + " FROM " + grantee;
                if (PROMPT)
                    ret += sql + "\n\n";
                ret += sql + ";\n\n";
            }
        }
    }

    for (std::list<QString>::iterator i = create.begin();i != create.end();i++)
    {
        std::list<QString> ctx = toExtract::splitDescribe(*i);
        if (toShift(ctx) != "NONE")
            continue;
        QString grantee = toShift(ctx);
        if (!onlyGrantee.isEmpty() && grantee != onlyGrantee)
            continue;

        QString priv = toShift(ctx);
        QString admin = toShift(ctx);
        if (priv.isEmpty())
            continue;

        if (admin == "ON")
        {
            QString object = toShift(ctx);
            admin = toShift(ctx);
            QString sql = "GRANT " + priv + " ON " + object + " TO " + grantee;
            if (PROMPT)
                ret += sql + "\n\n";
            ret += sql + ";\n\n";
        }
        else
        {
            QString sql = "GRANT " + priv + " TO " + grantee;
            if (PROMPT)
                ret += sql + "\n\n";
            ret += sql + ";\n\n";
        }
    }

    return ret;
}

// Implementation of migration functions

QString toOracleExtract::migrateConstraint(toExtract &ext,
        std::list<QString> &source,
        std::list<QString> &destin) const
{
    QString lastSchema;
    QString lastTable;
    QString lastName;
    QString lastType;
    QString sql;

    std::list<QString> drop;
    std::list<QString> create;

    toExtract::srcDst2DropCreate(source, destin, drop, create);

    {
        for (std::list<QString>::iterator i = drop.begin();i != drop.end();i++)
        {
            std::list<QString> ctx = toExtract::splitDescribe(*i);
            QString schema = toShift(ctx);
            QString table = toShift(ctx);
            if (toShift(ctx) != "TABLE")
                continue;
            QString name = toShift(ctx);
            if (toShift(ctx) != "CONSTRAINT")
                continue;
            QString type = toShift(ctx);
            QString extra = toShift(ctx);
            if ((schema != lastSchema ||
                    table != lastTable ||
                    name != lastName ||
                    type != lastType) &&
                    extra.isEmpty())
            {
                if (PROMPT)
                    sql += QString("PROMPT ALTER TABLE %1%2 DROP CONSTRAINT %3\n\n").
                           arg(schema).arg(table).arg(name);
                sql += QString("ALTER TABLE %1%2 DROP CONSTRAINT %3;\n\n").
                       arg(schema).arg(table).arg(name);
                lastSchema = schema;
                lastTable = table;
                lastName = name;
                lastType = type;
            }
        }
    }

    lastSchema = lastTable = lastName = lastType = QString::null;

    for (std::list<QString>::iterator i = create.begin();i != create.end();i++)
    {
        std::list<QString> ctx = toExtract::splitDescribe(*i);
        QString schema = toShift(ctx);
        QString table = toShift(ctx);
        if (toShift(ctx) != "TABLE")
            continue;
        QString name = toShift(ctx);
        if (toShift(ctx) != "CONSTRAINT")
            continue;
        QString type = toShift(ctx);
        if (toShift(ctx) != "DEFINITION")
            continue;
        QString extra = toShift(ctx);
        if (schema == lastSchema &&
                table == lastTable &&
                name == lastName &&
                type == lastType)
        {
            sql += " " + extra;
        }
        else if (extra.isEmpty())
        {
            if (!sql.isEmpty())
                sql += ";\n\n";
            if (PROMPT)
                sql += QString("PROMPT ALTER TABLE %1%2 ADD CONSTRAINT %3\n\n").
                       arg(schema).arg(table).arg(name);
            sql += QString("ALTER TABLE %1%2 ADD CONSTRAINT %3 %4").
                   arg(schema).arg(table).arg(name).arg(type);
            lastSchema = schema;
            lastTable = table;
            lastName = name;
            lastType = type;
        }
        else
        {
            if (PROMPT)
                sql += QString("PROMPT ALTER TABLE %1%2 MODIFY CONSTRAINT %3\n\n").
                       arg(schema).arg(table).arg(name);
            sql += QString("ALTER TABLE %1%2 MODIFY CONSTRAINT %3 %4\n\n").
                   arg(schema).arg(table).arg(name).arg(extra);
        }
    }
    if (!sql.isEmpty())
        sql += ";\n\n";

    return sql;
}

QString toOracleExtract::migrateDBLink(toExtract &ext,
                                       std::list<QString> &source,
                                       std::list<QString> &destin) const
{
    std::list<QString> drop;
    std::list<QString> create;

    toExtract::srcDst2DropCreate(source, destin, drop, create);

    QString ret;

    {
        for (std::list<QString>::iterator i = drop.begin();i != drop.end();i++)
        {
            std::list<QString> ctx = toExtract::splitDescribe(*i);
            QString owner = toShift(ctx);
            if (toShift(ctx) != "DATABASE LINK")
                continue;
            QString sql;
            if (owner == "PUBLIC")
                sql = "DROP PUBLIC DATABASE LINK ";
            else
                sql = "DROP DATABASE LINK";
            sql += toShift(ctx);
            if (PROMPT)
                ret += "PROMPT " + sql + "\n\n";
            ret += sql;
            ret += ";\n\n";
        }
    }
    for (std::list<QString>::iterator i = create.begin();i != create.end();i++)
    {
        std::list<QString> ctx = toExtract::splitDescribe(*i);
        QString owner = toShift(ctx);
        if (toShift(ctx) != "DATABASE LINK")
            continue;
        QString sql;
        if (owner == "PUBLIC")
            sql = "CREATE PUBLIC DATABASE LINK ";
        else
            sql = "CREATE DATABASE LINK";
        sql += toShift(ctx);
        if (PROMPT)
            ret += "PROMPT " + sql + "\n\n";
        ret += sql;
        ret += toShift(ctx);
        ret += ";\n\n";
    }
    return ret;
}

QString toOracleExtract::migrateFunction(toExtract &ext,
        std::list<QString> &source,
        std::list<QString> &destin) const
{
    return migrateSource(ext, source, destin, "FUNCTION");
}

QString toOracleExtract::migrateIndex(toExtract &ext,
                                      std::list<QString> &source,
                                      std::list<QString> &destin) const
{
    std::list<QString> drop;
    std::list<QString> create;

    QString ret;

    toExtract::srcDst2DropCreate(source, destin, drop, create);
    QString lastOwner;
    QString lastName;

    {
        for (std::list<QString>::iterator i = drop.begin();i != drop.end();i++)
        {
            std::list<QString> ctx = toExtract::splitDescribe(*i);
            QString owner = toShift(ctx);
            QString type = toShift(ctx);
            if (type != "INDEX" && type != "UNIQUE INDEX" && type != "BITMAP INDEX")
                continue;
            QString name = toShift(ctx);

            if (lastOwner != owner || name != lastName)
            {
                QString sql = "DROP INDEX " + owner + "." + name;
                if (PROMPT)
                    ret += "PROMPT " + sql + "\n\n";
                ret += sql + ";\n\n";
                lastOwner = owner;
                lastName = name;
            }
        }
    }
    lastOwner = lastName = QString::null;

    std::list<QString>::iterator j = destin.begin();
    for (std::list<QString>::iterator i = create.begin();i != create.end();i++)
    {
        std::list<QString> ctx = toExtract::splitDescribe(*i);

        QString owner = toShift(ctx);
        QString type = toShift(ctx);
        if (type != "INDEX" && type != "UNIQUE INDEX" && type != "BITMAP INDEX")
            continue;
        QString name = toShift(ctx);

        if (lastOwner != owner || name != lastName)
        {
            QString context = toExtract::contextDescribe(*i, 3);
            j = FindItem(destin, j, context);
            if (j != destin.end())
            {
                std::list<QString>::iterator k = FindItem(destin, j, context, 3, "ON");
                if (k == destin.end())
                    throw qApp->translate("toOracleExtract", "Missing ON description item on index");
                QString on = toExtract::partDescribe(*k, 4);
                QString sql = "CREATE " + type + " " + owner + "." + name + " ON " + on;
                if (PROMPT)
                    ret += "PROMPT " + sql + "\n\n";
                ret += sql + "\n";
                k = FindItem(destin, j, context, 3, "COLUMN");
                if (k == destin.end())
                    throw qApp->translate("toOracleExtract", "Couldn't find any COLUMN on index");
                ret += migrateIndexColumns(destin, k, toExtract::contextDescribe(*k, 4));
                k = FindItem(destin, j, context, 3, "DOMAIN");
                if (k != destin.end())
                {
                    while (k != destin.end() && toExtract::partDescribe(*k, 3) == "DOMAIN")
                    {
                        ret += "\n" + toExtract::partDescribe(*k, 4);
                        k++;
                    }
                    ret += ";\n";
                    break;
                }
                else
                    ret += "\n";
                k = FindItem(destin, j, context, 3, "COMPRESS");
                if (k != destin.end())
                    ret += "COMPRESS " + toExtract::partDescribe(*k, 4) + "\n";
                ret += migrateAttributes(destin, j, 3, "");
                k = FindItem(destin, j, context, 3, "REVERSE");
                if (k != destin.end())
                    ret += "REVERSE\n";
                ret + ";\n\n";
            }
            lastOwner = owner;
            lastName = name;
        }
    }

    return ret;
}

QString toOracleExtract::migratePackage(toExtract &ext,
                                        std::list<QString> &source,
                                        std::list<QString> &destin) const
{
    return migrateSource(ext, source, destin, "PACKAGE");
}

QString toOracleExtract::migratePackageBody(toExtract &ext,
        std::list<QString> &source,
        std::list<QString> &destin) const
{
    return migrateSource(ext, source, destin, "PACKAGE BODY");
}

QString toOracleExtract::migrateProcedure(toExtract &ext,
        std::list<QString> &source,
        std::list<QString> &destin) const
{
    return migrateSource(ext, source, destin, "PROCEDURE");
}

QString toOracleExtract::migrateRole(toExtract &ext,
                                     std::list<QString> &source,
                                     std::list<QString> &destin) const
{
    std::list<QString> drop;
    std::list<QString> create;

    toExtract::srcDst2DropCreate(source, destin, drop, create);

    std::list<QString> privs;

    QString ret;
    QString lrole;

    {
        bool dropped = false;
        for (std::list<QString>::iterator i = drop.begin();i != drop.end();i++)
        {
            std::list<QString> ctx = toExtract::splitDescribe(*i);
            QString owner = toShift(ctx);
            if (toShift(ctx) != "ROLE")
                continue;
            QString role = toShift(ctx);
            if (role != lrole)
                dropped = false;
            if ( ctx.empty() )
            {
                QString sql = "DROP ROLE " + QUOTE(role) + ";";
                if (PROMPT)
                    ret += "PROMPT " + sql + "\n\n";
                ret += sql + ";\n\n";
                dropped = true;
            }
            else if (!dropped)
            {
                if (toShift(ctx) != "GRANT")
                    continue;
                QString type = toShift(ctx);
                QString on = toShift(ctx);
                QString what = toShift(ctx);
                QString sql = QString("REVOKE %1").arg(QUOTE(type));
                if (!on.isEmpty())
                    sql += QUOTE(on);
                if (!what.isEmpty())
                {
                    if (what == "ON")
                        sql += "FROM";
                    else
                        sql += QUOTE(what);
                }
                if (PROMPT)
                    ret += "PROMPT " + sql + "\n\n";
                ret += sql + ";\n\n";
            }
            lrole = role;
        }
    }
    lrole = QString::null;

    for (std::list<QString>::iterator i = create.begin();i != create.end();i++)
    {
        std::list<QString> ctx = toExtract::splitDescribe(*i);
        QString owner = toShift(ctx);
        if (toShift(ctx) != "ROLE")
            continue;
        QString role = toShift(ctx);
        if ( ctx.empty() )
            continue;
        else
        {
            QString extra = toShift(ctx);
            if (extra == "INFO")
            {
                QString sql = "CREATE ROLE " + QUOTE(role) + ";";
                if (PROMPT)
                    ret += "PROMPT " + sql + "\n\n";
                ret += sql + QString("%1;\n\n").arg(toShift(ctx));
            }
            else if (extra == "GRANT")
            {
                QString type = toShift(ctx);
                QString on = toShift(ctx);
                QString what = toShift(ctx);
                QString sql = QString("GRANT %1").arg(QUOTE(type));
                if (!on.isEmpty())
                    sql += QUOTE(on);
                if (!what.isEmpty())
                    sql += QUOTE(what);
                if (PROMPT)
                    ret += "PROMPT " + sql + "\n\n";
                ret += sql + ";\n\n";
            }
        }
        lrole = role;
    }
    return ret;
}

QString toOracleExtract::migrateSequence(toExtract &ext,
        std::list<QString> &source,
        std::list<QString> &destin) const
{
    QString ret;

    std::list<QString> drop;
    std::list<QString> create;

    toExtract::srcDst2DropCreate(source, destin, drop, create);
    {
        for (std::list<QString>::iterator i = drop.begin();i != drop.end();i++)
        {
            std::list<QString> ctx = toExtract::splitDescribe(*i);
            QString owner = toShift(ctx);
            if (toShift(ctx) != "SEQUENCE")
                continue;
            QString sequence = toShift(ctx);
            if ( ctx.empty() )
            {
                QString sql = QString("DROP SEQUENCE %1.%2").arg(QUOTE(owner)).arg(QUOTE(sequence));
                if (PROMPT)
                    ret += "PROMPT " + sql + "\n\n";
                ret += sql + ";\n\n";
            }
        }
    }
    bool created = false;
    QString lastOwner;
    QString lastSequence;
    QString sql;
    QString prompt;
    for (std::list<QString>::iterator i = create.begin();i != create.end();i++)
    {
        std::list<QString> ctx = toExtract::splitDescribe(*i);
        QString owner = toShift(ctx);
        if (toShift(ctx) != "SEQUENCE")
            continue;
        QString sequence = toShift(ctx);
        if (lastSequence != sequence || lastOwner != owner)
        {
            if (created)
            {
                prompt = QString("CREATE SEQUENCE %1.%2").arg(lastOwner).arg(lastSequence);
                sql.prepend(prompt);
            }
            else
            {
                prompt = QString("ALTER SEQUENCE %1.%2").arg(lastOwner).arg(lastSequence);
                sql.prepend(prompt);
            }
            if (PROMPT)
                ret += prompt + "\n\n";
            ret += sql + ";\n\n";

            prompt = sql =
                         created = false;
        }
        if ( ctx.empty() )
            created = true;
        else
            sql += " " + toShift(ctx);

        lastOwner = owner;
        lastSequence = sequence;
    }

    if (created)
    {
        prompt = QString("CREATE SEQUENCE %1.%2").arg(lastOwner).arg(lastSequence);
        sql.prepend(prompt);
    }
    else
    {
        prompt = QString("ALTER SEQUENCE %1.%2").arg(lastOwner).arg(lastSequence);
        sql.prepend(prompt);
    }
    if (PROMPT)
        ret += prompt + "\n\n";
    ret += sql + ";\n\n";
    return ret;
}

// Implementation public interface

toOracleExtract::toOracleExtract()
{
    // Supports Oracle
    registerExtract(ORACLE_NAME,
                    QString::null,
                    QString::null);

    // Register creates
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "CONSTRAINT");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "DATABASE LINK");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "EXCHANGE INDEX");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "EXCHANGE TABLE");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "FUNCTION");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "INDEX");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "MATERIALIZED VIEW");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "MATERIALIZED VIEW LOG");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "PACKAGE");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "PACKAGE BODY");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "PROCEDURE");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "PROFILE");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "ROLE");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "ROLE GRANTS");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "ROLLBACK SEGMENT");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "SEQUENCE");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "SNAPSHOT");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "SNAPSHOT LOG");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "SYNONYM");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "TABLE");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "TABLE FAMILY");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "TABLE CONTENTS");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "TABLE REFERENCES");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "TABLESPACE");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "TRIGGER");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "TYPE");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "USER");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "USER GRANTS");
    registerExtract(ORACLE_NAME,
                    "CREATE",
                    "VIEW");

    // Register describes
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "CONSTRAINT");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "DATABASE LINK");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "EXCHANGE INDEX");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "EXCHANGE TABLE");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "FUNCTION");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "INDEX");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "MATERIALIZED VIEW");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "MATERIALIZED VIEW LOG");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "PACKAGE");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "PACKAGE BODY");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "PROCEDURE");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "PROFILE");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "ROLE");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "ROLE GRANTS");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "ROLLBACK SEGMENT");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "SEQUENCE");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "SNAPSHOT");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "SNAPSHOT LOG");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "SYNONYM");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "TABLE");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "TABLE FAMILY");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "TABLE CONTENTS");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "TABLE REFERENCES");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "TABLESPACE");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "TRIGGER");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "TYPE");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "USER");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "USER GRANTS");
    registerExtract(ORACLE_NAME,
                    "DESCRIBE",
                    "VIEW");

    // Register drops
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "CONSTRAINT");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "DATABASE LINK");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "DIMENSION");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "DIRECTORY");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "FUNCTION");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "INDEX");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "MATERIALIZED VIEW");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "MATERIALIZED VIEW LOG");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "PACKAGE");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "PROCEDURE");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "PROFILE");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "ROLE");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "ROLLBACK SEGMENT");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "SEQUENCE");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "SNAPSHOT");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "SNAPSHOT LOG");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "SYNONYM");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "TABLE");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "TABLESPACE");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "TRIGGER");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "TYPE");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "USER");
    registerExtract(ORACLE_NAME,
                    "DROP",
                    "VIEW");
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

void toOracleExtract::initialize(toExtract &ext) const
{
    QString DbaSegments;
    try
    {
        toQuery(CONNECTION, "select * from sys.dba_segments where null=null");
        DbaSegments = "sys.dba_segments";
    }
    catch (...)
    {
        DbaSegments = QString("(select '%1' owner,user_segments.* from sys.user_segments)").
                      arg(CONNECTION.user().toUpper());
    }
    ext.setState("Segments", DbaSegments);

    try
    {
        toQList ret = toQuery::readQueryNull(CONNECTION, SQLSetSizing);
        ext.setBlockSize(toShift(ret).toInt());
    }
    catch (...)
    {
        toQList ret = toQuery::readQueryNull(CONNECTION, SQLSetSizingFallback);
        if (ret.empty())
            ret = toQuery::readQueryNull(CONNECTION, SQLSetSizingFallback2);
        ext.setBlockSize(toShift(ret).toInt());
    }
}

void toOracleExtract::create(toExtract &ext,
                             QTextStream &stream,
                             const QString &type,
                             const QString &schema,
                             const QString &owner,
                             const QString &name) const
{
    clearFlags(ext);

    if (type == "CONSTRAINT")
        stream << createConstraint(ext, schema, owner, name);
    else if (type == "DATABASE LINK")
        stream << createDBLink(ext, schema, owner, name);
    else if (type == "EXCHANGE INDEX")
        stream << createExchangeIndex(ext, schema, owner, name);
    else if (type == "EXCHANGE TABLE")
        stream << createExchangeTable(ext, schema, owner, name);
    else if (type == "FUNCTION")
        stream << createFunction(ext, schema, owner, name);
    else if (type == "INDEX")
        stream << createIndex(ext, schema, owner, name);
    else if (type == "MATERIALIZED VIEW")
        stream << createMaterializedView(ext, schema, owner, name);
    else if (type == "MATERIALIZED VIEW LOG")
        stream << createMaterializedViewLog(ext, schema, owner, name);
    else if (type == "PACKAGE")
        stream << createPackage(ext, schema, owner, name);
    else if (type == "PACKAGE BODY")
        stream << createPackageBody(ext, schema, owner, name);
    else if (type == "PROCEDURE")
        stream << createProcedure(ext, schema, owner, name);
    else if (type == "PROFILE")
        stream << createProfile(ext, schema, owner, name);
    else if (type == "ROLE")
        stream << createRole(ext, schema, owner, name);
    else if (type == "ROLE GRANTS")
        stream << grantedPrivs(ext, QUOTE(name), name, 6);
    else if (type == "ROLLBACK SEGMENT")
        stream << createRollbackSegment(ext, schema, owner, name);
    else if (type == "SEQUENCE")
        stream << createSequence(ext, schema, owner, name);
    else if (type == "SNAPSHOT")
        stream << createSnapshot(ext, schema, owner, name);
    else if (type == "SNAPSHOT LOG")
        stream << createSnapshotLog(ext, schema, owner, name);
    else if (type == "SYNONYM")
        stream << createSynonym(ext, schema, owner, name);
    else if (type == "TABLE")
        stream << createTable(ext, schema, owner, name);
    else if (type == "TABLE FAMILY")
        stream << createTableFamily(ext, schema, owner, name);
    else if (type == "TABLE CONTENTS")
        createTableContents(ext, stream, schema, owner, name);
    else if (type == "TABLE REFERENCES")
        stream << createTableReferences(ext, schema, owner, name);
    else if (type == "TABLESPACE")
        stream << createTablespace(ext, schema, owner, name);
    else if (type == "TRIGGER")
        stream << createTrigger(ext, schema, owner, name);
    else if (type == "TYPE")
        stream << createType(ext, schema, owner, name);
    else if (type == "USER")
        stream << createUser(ext, schema, owner, name);
    else if (type == "USER GRANTS")
    {
        QString nam;
        if (ext.getSchema() != "1" && !ext.getSchema().isEmpty())
            nam = ext.getSchema().toLower();
        else
            nam = QUOTE(name);
        stream << grantedPrivs(ext, nam, name, 4);
    }
    else if (type == "VIEW")
        stream << createView(ext, schema, owner, name);
    else
    {
        throw qApp->translate("toOracleExtract", "Invalid type %1 to create").arg(type);
    }
}

void toOracleExtract::describe(toExtract &ext,
                               std::list<QString> &lst,
                               const QString &type,
                               const QString &schema,
                               const QString &owner,
                               const QString &name) const
{
    clearFlags(ext);

    if (type == "CONSTRAINT")
        describeConstraint(ext, lst, schema, owner, name);
    else if (type == "DATABASE LINK")
        describeDBLink(ext, lst, schema, owner, name);
    else if (type == "EXCHANGE INDEX")
        describeExchangeIndex(ext, lst, schema, owner, name);
    else if (type == "EXCHANGE TABLE")
        describeExchangeTable(ext, lst, schema, owner, name);
    else if (type == "FUNCTION")
        describeFunction(ext, lst, schema, owner, name);
    else if (type == "INDEX")
        describeIndex(ext, lst, schema, owner, name);
    else if (type == "MATERIALIZED VIEW")
        describeMaterializedView(ext, lst, schema, owner, name);
    else if (type == "MATERIALIZED VIEW LOG")
        describeMaterializedViewLog(ext, lst, schema, owner, name);
    else if (type == "PACKAGE")
        describePackage(ext, lst, schema, owner, name);
    else if (type == "PACKAGE BODY")
        describePackageBody(ext, lst, schema, owner, name);
    else if (type == "PROCEDURE")
        describeProcedure(ext, lst, schema, owner, name);
    else if (type == "PROFILE")
        describeProfile(ext, lst, schema, owner, name);
    else if (type == "ROLE")
        describeRole(ext, lst, schema, owner, name);
    else if (type == "ROLE GRANTS")
    {
        // A nop, everything is done in describe role
    }
    else if (type == "ROLLBACK SEGMENT")
        describeRollbackSegment(ext, lst, schema, owner, name);
    else if (type == "SEQUENCE")
        describeSequence(ext, lst, schema, owner, name);
    else if (type == "SNAPSHOT")
        describeSnapshot(ext, lst, schema, owner, name);
    else if (type == "SNAPSHOT LOG")
        describeSnapshotLog(ext, lst, schema, owner, name);
    else if (type == "SYNONYM")
        describeSynonym(ext, lst, schema, owner, name);
    else if (type == "TABLE")
        describeTable(ext, lst, schema, owner, name);
    else if (type == "TABLE FAMILY")
        describeTableFamily(ext, lst, schema, owner, name);
    else if (type == "TABLE REFERENCES")
        describeTableReferences(ext, lst, schema, owner, name);
    else if (type == "TABLE CONTENTS")
    {
        // A nop, nothing is described of contents
    }
    else if (type == "TABLESPACE")
        describeTablespace(ext, lst, schema, owner, name);
    else if (type == "TRIGGER")
        describeTrigger(ext, lst, schema, owner, name);
    else if (type == "TYPE")
        describeType(ext, lst, schema, owner, name);
    else if (type == "USER")
        describeUser(ext, lst, schema, owner, name);
    else if (type == "USER GRANTS")
    {
        // A nop, everything is done in describe user
    }
    else if (type == "VIEW")
        describeView(ext, lst, schema, owner, name);
    else
    {
        throw qApp->translate("toOracleExtract", "Invalid type %1 to describe").arg(type);
    }
}

void toOracleExtract::drop(toExtract &ext,
                           QTextStream &stream,
                           const QString &type,
                           const QString &schema,
                           const QString &owner,
                           const QString &name) const
{
    clearFlags(ext);
    if (type == "CONSTRAINT")
        stream << dropConstraint(ext, schema, owner, type, name);
    else if (type == "DATABASE LINK")
        stream << dropDatabaseLink(ext, schema, owner, type, name);
    else if (type == "DIMENSION")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else if (type == "DIRECTORY")
        stream << dropObject(ext, schema, owner, type, name);
    else if (type == "FUNCTION")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else if (type == "INDEX")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else if (type == "MATERIALIZED VIEW")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else if (type == "MATERIALIZED VIEW LOG")
        stream << dropMViewLog(ext, schema, owner, type, name);
    else if (type == "PACKAGE")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else if (type == "PROCEDURE")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else if (type == "PROFILE")
        stream << dropProfile(ext, schema, owner, type, name);
    else if (type == "ROLE")
        stream << dropObject(ext, schema, owner, type, name);
    else if (type == "ROLLBACK SEGMENT")
        stream << dropObject(ext, schema, owner, type, name);
    else if (type == "SEQUENCE")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else if (type == "SNAPSHOT")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else if (type == "SNAPSHOT LOG")
        stream << dropMViewLog(ext, schema, owner, type, name);
    else if (type == "SYNONYM")
        stream << dropSynonym(ext, schema, owner, type, name);
    else if (type == "TABLE")
        stream << dropTable(ext, schema, owner, type, name);
    else if (type == "TABLESPACE")
        stream << dropTablespace(ext, schema, owner, type, name);
    else if (type == "TRIGGER")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else if (type == "TYPE")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else if (type == "USER")
        stream << dropUser(ext, schema, owner, type, name);
    else if (type == "VIEW")
        stream << dropSchemaObject(ext, schema, owner, type, name);
    else
    {
        throw qApp->translate("toOracleExtract", "Invalid type %1 to drop").arg(type);
    }
}
