
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

#pragma once

//#include "core/utils.h"
//#include "core/toconnection.h"
#include "core/toextract.h"
#include "core/tosql.h"
#include "core/toqvalue.h"
#include "core/toquery.h"
//#include "core/totool.h"

#include <QApplication>
#include <QtCore/QRegExp>

// Some convenient defines

static const QString ORACLE_NAME("Oracle");
#define PROMPT (ext.getPrompt())

#define addDescription toExtract::addDescription

class toOracleExtract : public toExtract::extractor
{
        std::list<toExtract::datatype> oracle_datatypes;
        // Misc utility functions

        void clearFlags                 ();
        QString displaySource           (const QString &schema, const QString &owner, const QString &name, const QString &type, bool describe);
        QString keyColumns              (const QString &owner, const QString &name, const QString &type, const QString &table);
        QString partitionKeyColumns     (const QString &owner, const QString &name, const QString &type);
        static QString prepareDB        (const QString &data);
        QString segments                ();
        QString segments                (const toSQL &sql);
        QString subPartitionKeyColumns  (const QString &owner, const QString &name, const QString &type);

        // Create utility functions
        QString constraintColumns       (const QString &owner, const QString &name);
        QString createComments          (const QString &schema, const QString &owner, const QString &name);
        QString createContextPrefs      (const QString &schema, const QString &owner, const QString &name, const QString &sql);
        QString createIOT               (const QString &schema, const QString &owner, const QString &name);
        QString createMView             (const QString &schema, const QString &owner, const QString &name, const QString &type);
        QString createMViewIndex        (const QString &schema, const QString &owner, const QString &name);
        QString createMViewLog          (const QString &schema, const QString &owner, const QString &name, const QString &type);
        QString createMViewTable        (const QString &schema, const QString &owner, const QString &name);
        QString createPartitionedIOT    (const QString &schema, const QString &owner, const QString &name);
        QString createPartitionedIndex  (const QString &schema, const QString &owner, const QString &name, const QString &sql);
        QString createPartitionedTable  (const QString &schema, const QString &owner, const QString &name);
        QString createTableText         (toQList &result, const QString &schema, const QString &owner, const QString &name);
        QString grantedPrivs            (const QString &dest, const QString &name, int typ);
        QString indexColumns            (const QString &indent, const QString &owner, const QString &name);
        QString rangePartitions         (const QString &owner, const QString &name, const QString &subPartitionType, const QString &caller);
        QString segmentAttributes       (toQList &result);
        QString tableColumns            (const QString &owner, const QString &name);

        // Describe utility functions
        void describeAttributes         (std::list<QString> &dsp, std::list<QString> &ctx, toQList &result);
        void describeComments           (std::list<QString> &lst, std::list<QString> &ctx, const QString &owner, const QString &name);
        void describePrivs              (std::list<QString> &lst, std::list<QString> &ctx, const QString &name);
        void describeIOT                (std::list<QString> &lst, std::list<QString> &ctx, const QString &schema, const QString &owner, const QString &name);
        void describeIndexColumns       (std::list<QString> &lst, std::list<QString> &ctx, const QString &owner, const QString &name);
        void describeMView              (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name, const QString &type);
        void describeMViewIndex         (std::list<QString> &lst, std::list<QString> &ctx, const QString &schema, const QString &owner, const QString &name);
        void describeMViewLog           (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name, const QString &type);
        void describeMViewTable         (std::list<QString> &lst, std::list<QString> &ctx, const QString &schema, const QString &owner, const QString &name);
        void describePartitions         (std::list<QString> &lst, std::list<QString> &ctx, const QString &owner, const QString &name, const QString &subPartitionType, const QString &caller);
        void describePartitionedIOT     (std::list<QString> &lst, std::list<QString> &ctx, const QString &schema, const QString &owner, const QString &name);
        void describePartitionedIndex   (std::list<QString> &lst, std::list<QString> &ctx, const QString &schema, const QString &owner, const QString &name);
        void describePartitionedTable   (std::list<QString> &lst, std::list<QString> &ctx, const QString &schema, const QString &owner, const QString &name);
        void describeSource             (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name, const QString &type);
        void describeTableColumns       (std::list<QString> &lst, std::list<QString> &ctx, const QString &owner, const QString &name);
        void describeTableText          (std::list<QString> &lst, std::list<QString> &ctx, toQList &result, const QString &schema, const QString &owner, const QString &name);

        // Create functions
        QString createConstraint        (const QString &schema, const QString &owner, const QString &name);
        QString createDBLink            (const QString &schema, const QString &owner, const QString &name);
        QString createDirectory         (const QString &schema, const QString &owner, const QString &name);
        QString createExchangeIndex     (const QString &schema, const QString &owner, const QString &name);
        QString createExchangeTable     (const QString &schema, const QString &owner, const QString &name);
        QString createFunction          (const QString &schema, const QString &owner, const QString &name);
        QString createIndex             (const QString &schema, const QString &owner, const QString &name);
        QString createMaterializedView  (const QString &schema, const QString &owner, const QString &name);
        QString createMaterializedViewLog(const QString &schema, const QString &owner, const QString &name);
        QString createPackage           (const QString &schema, const QString &owner, const QString &name);
        QString createPackageBody       (const QString &schema, const QString &owner, const QString &name);
        QString createProcedure         (const QString &schema, const QString &owner, const QString &name);
        QString createProfile           (const QString &schema, const QString &owner, const QString &name);
        QString createRole              (const QString &schema, const QString &owner, const QString &name);
        QString createRollbackSegment   (const QString &schema, const QString &owner, const QString &name);
        QString createSequence          (const QString &schema, const QString &owner, const QString &name);
        QString createSnapshot          (const QString &schema, const QString &owner, const QString &name);
        QString createSnapshotLog       (const QString &schema, const QString &owner, const QString &name);
        QString createSynonym           (const QString &schema, const QString &owner, const QString &name);
        QString createTable             (const QString &schema, const QString &owner, const QString &name);
        QString createTableFamily       (const QString &schema, const QString &owner, const QString &name);
        void createTableContents        (QTextStream &stream, const QString &schema, const QString &owner, const QString &name);
        QString createTableReferences   (const QString &schema, const QString &owner, const QString &name);
        QString createTablespace        (const QString &schema, const QString &owner, const QString &name);
        QString createTrigger           (const QString &schema, const QString &owner, const QString &name);
        QString createType              (const QString &schema, const QString &owner, const QString &name);
        QString createUser              (const QString &schema, const QString &owner, const QString &name);
        QString createView              (const QString &schema, const QString &owner, const QString &name);

        // Describe functions
        void describeConstraint         (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeDBLink             (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeExchangeIndex      (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeExchangeTable      (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeFunction           (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeIndex              (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeMaterializedView   (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeMaterializedViewLog(std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describePackage            (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describePackageBody        (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeProcedure          (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeProfile            (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeRole               (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeRollbackSegment    (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeSequence           (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeSnapshot           (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeSnapshotLog        (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeSynonym            (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeTable              (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeTableFamily        (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeTableReferences    (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeTablespace         (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeTrigger            (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeType               (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeUser               (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);
        void describeView               (std::list<QString> &lst, const QString &schema, const QString &owner, const QString &name);

        // DBMS_METADATA
        QString createMetadata(const QString &owner, const QString &name, toExtract::ObjectType type);

    public:
        // Public interface

        toOracleExtract(toExtract &parent);
        virtual ~toOracleExtract();

        void initialize() override;

        void create(QTextStream &stream, toExtract::ObjectType type, const QString &schema, const QString &owner, const QString &name) override;

        void describe(std::list<QString> &lst, toExtract::ObjectType type, const QString &schema, const QString &owner, const QString &name) override;

        std::list<toExtract::datatype> datatypes() const override;
};

