
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

#ifndef TO_ORACLEEXTRACT
#define TO_ORACLEEXTRACT

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
std::list<toExtract::datatype> oracle_datatypes;
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
    QString createTable(std::list<QString> &destin) const;
    QString alterTable(std::list<QString> &source, std::list<QString> &destin) const;

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
    QString migrateTable(toExtract &ext, std::list<QString> &source,
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
    virtual void migrate(toExtract &ext,
                         QTextStream &stream,
                         const QString &type,
                         std::list<QString> &src,
                         std::list<QString> &dst) const;

virtual std::list<toExtract::datatype> datatypes() const;
};

#endif
