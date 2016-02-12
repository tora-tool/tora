
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

#include "tools/tobrowsertablewidget.h"
#include "widgets/tobrowserbasewidget.h"
#include "tools/tostorage.h"
#include "widgets/toresultcols.h"
#include "widgets/toresultcode.h"
#include "widgets/toresultitem.h"
#include "widgets/toresulttableview.h"
#include "widgets/toresultgrants.h"
#include "widgets/toresultextent.h"
#include "core/utils.h"
//#include "core/toextract.h"
#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "core/toconnectiontraits.h"
#include "core/toconnectionsubloan.h"

#include "result/toresulttabledata.h"

#include "widgets/toresultdrawing.h"
#ifdef TORA_EXPERIMENTAL
#include "dotgraphview.h"
#include "dotgraph.h"
#endif


Util::RegisterInFactory<toBrowserTableWidget, toBrowserWidgetFactory, toCache::CacheEntryType> regToBrowserTableWidget(toCache::TABLE);

static toSQL SQLTableIndex("toBrowserTableWidget:TableIndex",
                           "SELECT IND.index_name AS \"Index Name\",\n"
                           "       ind.column_name AS \"Column Name\",\n"
                           "       al.uniqueness AS \"Unique\",\n"
                           "       AL.index_type AS \"Type\",\n"
                           "       EX.column_expression AS \"Column Expression\",\n"
                           "       AL.status\n"
                           "  FROM SYS.ALL_IND_COLUMNS IND,\n"
                           "       SYS.ALL_IND_EXPRESSIONS EX,\n"
                           "       sys.All_Indexes AL\n"
                           " WHERE IND.INDEX_OWNER = :own<char[101]>\n"
                           "   AND IND.TABLE_NAME = :nam<char[101]>\n"
                           "   AND EX.index_owner ( + ) = IND.index_owner\n"
                           "   AND EX.index_name ( + ) = IND.index_name\n"
                           "   AND IND.index_name = AL.index_name ( + )\n"
                           "   AND IND.index_owner = AL.owner ( + )\n"
                           "ORDER BY ind.index_name, ind.column_position",
                           "List the indexes on a table",
                           "");
static toSQL SQLTableIndexSapDB("toBrowserTableWidget:TableIndex",
                                "SELECT owner,\n"
                                "       indexname \"Index_Name\",\n"
                                "       'NORMAL',\n"
                                "       type\n"
                                " FROM indexes \n"
                                " WHERE owner = :f1<char[101]> and tablename = :f2<char[101]> \n"
                                " ORDER by indexname",
                                "",
                                "",
                                "SapDB");

static toSQL SQLTableIndexPG("toBrowserTableWidget:TableIndex",
                             "SELECT u.usename AS \"Owner\",\n"
                             "       c2.relname AS \"Index Name\",\n"
                             "       pg_get_indexdef(i.indexrelid) as \"Definition\"\n"
                             "  FROM pg_class c,\n"
                             "       pg_class c2,\n"
                             "       pg_index i,\n"
                             "       pg_user u,\n"
                             "       pg_namespace n\n"
                             " WHERE c.relowner = u.usesysid\n"
                             "   AND n.nspname = :f1\n"
                             "   AND c.relname = :f2\n"
                             "   AND c.relowner = u.usesysid\n"
                             "   AND n.OID = c.relnamespace\n"
                             "   AND c.OID = i.indrelid\n"
                             "   AND i.indexrelid = c2.OID",
                             "",
                             "",
                             "QPSQL");

static toSQL SQLTableIndexMySQL3("toBrowserTableWidget:TableIndex",
                                 "SHOW INDEX FROM `:f1<noquote>`.`:tab<noquote>`",
                                 "",
                                 "3.0",
                                 "QMYSQL");
static toSQL SQLTableIndexMySQL("toBrowserTableWidget:TableIndex",
                                "SELECT * FROM INFORMATION_SCHEMA.STATISTICS\n"
                                "    WHERE table_schema = :f1<char[101]>\n"
                                "        AND table_name = :f2<char[101]>\n",
                                "",
                                "5.0",
                                "QMYSQL");

static toSQL SQLTableConstraint(
    "toBrowser:TableConstraint",
//     "SELECT sub.constraint_name AS \"Constraint Name\",\n"
//     "       srch.search_condition AS \"Search Condition\",\n"
//     "       sub.condition AS \"Condition\",\n"
//     "       sub.status AS \"Status\",\n"
//     "       sub.constraint_type AS \"Type\",\n"
//     "       sub.delete_rule AS \"Delete Rule\",\n"
//     "       sub.GENERATED AS \"Generated\"\n"
//     "  FROM ( SELECT c.Constraint_Name,\n"
//     "                DECODE ( constraint_type,\n"
//     "                         'R',\n"
//     "                         'foreign key (' || a.column_name || ') references ' "
//     "|| a2.owner || '.' || a2.table_name || '(' || a2.column_name || ')',\n"
//     "                         'P',\n"
//     "                         'primary key (' || a.column_name || ')',\n"
//     "                         'U',\n"
//     "                         'unique (' || a.column_name || ')',\n"
//     "                         NULL ) AS condition,\n"
//     "                c.Status,\n"
//     "                c.Constraint_Type,\n"
//     "                c.Delete_Rule,\n"
//     "                c.GENERATED\n"
//     "           FROM sys.All_Constraints c,\n"
//     "                sys.all_cons_columns a,\n"
//     "                sys.all_cons_columns a2\n"
//     "          WHERE c.constraint_name = a.constraint_name\n"
//     "            AND c.Owner = :f1<char[101]>\n"
//     "            AND c.Table_Name = :f2<char[101]>\n"
//     "            AND c.r_constraint_name = a2.constraint_name ( + ) ) sub,\n"
//     "       sys.all_constraints srch\n"
//     " WHERE sub.constraint_name = srch.constraint_name",
    "SELECT main.constraint_name AS \"Constraint Name\",\n"
    "   main.column_name AS \"Column Name\",\n"
    "   main.search_condition AS \"Search Condition\",\n"
    "   main.status AS \"Status\",\n"
    "   main.constraint_type AS \"Type\",\n"
    "   main.delete_rule AS \"Delete Rule\",\n"
    "   main.generated AS \"Generated\",\n"
    "   DECODE ( main.constraint_type,\n"
    "            'R',\n"
    "            'FK (' || main.column_name || ') ref. ' || refs.owner || '.'\n"
    "                   || refs.table_name || '(' || refs.column_name || ')',\n"
    "            'P',\n"
    "            'PK (' || main.column_name || ')',\n"
    "            'U',\n"
    "            'unique (' || main.column_name || ')',\n"
    "            NULL ) AS \"Condition\"\n"
    "   FROM\n"
    "       (\n"
    "       SELECT\n"
    "           c.constraint_name,\n"
    "           a.column_name,\n"
    "           c.constraint_type,\n"
    "           c.r_constraint_name,\n"
    "           c.search_condition,\n"
    "           c.status,\n"
    "           c.delete_rule,\n"
    "           c.generated,\n"
    "           c.owner,\n"
    "           c.r_owner,\n"
    "           a.position\n"
    "       FROM\n"
    "           sys.All_Constraints c,\n"
    "           sys.all_cons_columns a\n"
    "       WHERE\n"
    "               c.constraint_name = a.constraint_name\n"
    "           AND c.Owner = :f1<char[101]>\n"
    "           AND c.Owner = a.Owner\n"
    "           AND c.Table_Name = :f2<char[101]>\n"
    "       ORDER BY\n"
    "           c.constraint_name, a.constraint_name, a.position\n"
    "       ) main,\n"
    "       sys.all_cons_columns refs\n"
    "   WHERE\n"
    "           main.r_constraint_name = refs.constraint_name (+)\n"
    "       AND main.r_owner = refs.owner (+)\n"
    "       AND main.position = refs.position (+)\n",
    "List the constraints on a table",
    "");

static toSQL SQLTableConstraintPG(
    "toBrowser:TableConstraint",
    "SELECT conname as \"Constraint Name\",\n"
    "       pg_catalog.pg_get_constraintdef ( r.OID,\n"
    "                                         TRUE ) AS \"Description\"\n"
    "  FROM pg_catalog.pg_constraint r,\n"
    "       pg_catalog.pg_class c,\n"
    "       pg_catalog.pg_namespace n\n"
    " WHERE n.nspname = :f1\n"
    "   AND c.relname = :f2\n"
    "   AND c.relnamespace = n.OID\n"
    "   AND c.OID = r.conrelid\n"
    " ORDER BY 1",
    "",
    "",
    "QPSQL");

static toSQL SQLTableReferences(
    "toBrowser:TableReferences",
    "SELECT a.Owner AS \"Owner\",\n"
    "       a.Table_Name AS \"Object\",\n"
    "       a.Constraint_Name AS \"Constraint\",\n"
    "       'foreign key (' || c.column_name || ') references ' || r.owner || "
    "'.' || r.table_name || '(' || r.column_name || ')' AS \"Condition\",\n"
    "       a.Status AS \"Status\",\n"
    "       a.Delete_Rule AS \"Delete Rule\"\n"
    "  FROM sys.all_constraints a,\n"
    "       sys.all_cons_columns r,\n"
    "       sys.all_cons_columns c\n"
    " WHERE a.constraint_type = 'R'\n"
    "   AND (a.r_owner, a.r_constraint_name) IN ( SELECT b.owner, b.constraint_name\n"
    "                                              FROM sys.all_constraints b\n"
    "                                             WHERE b.OWNER = :owner<char[101]>\n"
    "                                               AND b.TABLE_NAME = :tab<char[101]> )\n"
    "   AND a.r_owner = r.owner\n"
    "   AND a.r_constraint_name = r.constraint_name\n"
    "   AND a.owner = c.owner\n"
    "   AND a.constraint_name = c.constraint_name\n"
    " UNION SELECT owner,\n"
    "       name,\n"
    "       NULL,\n"
    "       TYPE || ' ' || dependency_type,\n"
    "       'DEPENDENCY',\n"
    "       NULL\n"
    "  FROM sys.all_dependencies\n"
    " WHERE referenced_owner = :owner<char[101]>\n"
    "   AND referenced_name = :tab<char[101]>",
    "List the references on a table",
    "");

static toSQL SQLTableTriggerSapDB("toBrowser:TableTrigger",
                                  "SELECT TriggerName,'UPDATE' \"Event\",''\"Column\",'ENABLED' \"Status\",''\"Description\"\n"
                                  " FROM triggers \n"
                                  " WHERE owner = upper(:f1<char[101]>) and tablename = :f2<char[101]>\n"
                                  "  and update='YES'\n"
                                  "UNION\n"
                                  "SELECT TriggerName,'INSERT','','ENABLED',''\n"
                                  " FROM triggers \n"
                                  " WHERE owner = upper(:f1<char[101]>) and  tablename = :f2<char[101]>\n"
                                  "  and insert='YES'\n"
                                  "UNION\n"
                                  "SELECT TriggerName,'DELETE','','ENABLED',''\n"
                                  " FROM triggers \n"
                                  " WHERE owner = upper(:f1<char[101]>) and  tablename = :f2<char[101]>\n"
                                  "  and delete='YES'\n"
                                  " ORDER by 1 ",
                                  "Display the triggers operating on a table",
                                  "",
                                  "SapDB");
static toSQL SQLTableTrigger("toBrowser:TableTrigger",
                             "SELECT Trigger_Name,Triggering_Event,Column_Name,Status,Description \n"
                             "  FROM SYS.ALL_TRIGGERS\n"
                             " WHERE Table_Owner = :f1<char[101]> AND Table_Name = :f2<char[101]>",
                             "",
                             "0801");
static toSQL SQLTableTrigger8("toBrowser:TableTrigger",
                              "SELECT Trigger_Name,Triggering_Event,Status,Description \n"
                              "  FROM SYS.ALL_TRIGGERS\n"
                              " WHERE Table_Owner = :f1<char[101]> AND Table_Name = :f2<char[101]>");
static toSQL SQLTableTriggerPG("toBrowser:TableTrigger",
                               " SELECT t.tgname AS \"Trigger Name\",\n"
                               "        pg_catalog.pg_get_triggerdef ( t.OID ) AS \"Condition\"\n"
                               "   FROM pg_catalog.pg_trigger t,\n"
                               "        pg_class c,\n"
                               "        pg_tables tab\n"
                               "  WHERE lower ( tab.schemaname ) = lower ( :f1<char[101]> )\n"
                               "    AND c.relname = tab.tablename\n"
                               "    AND lower ( c.relname ) = lower ( :f2<char[101]> )\n"
                               "    AND c.OID = t.tgrelid\n"
                               "    AND ( NOT tgisconstraint OR NOT EXISTS ( SELECT 1\n"
                               "                                               FROM pg_catalog.pg_depend d\n"
                               "                                               JOIN pg_catalog.pg_constraint c\n"
                               "                                                 ON ( d.refclassid = c.tableoid AND d.refobjid = c.OID )\n"
                               "                                              WHERE d.classid = t.tableoid\n"
                               "                                                AND d.objid = t.OID\n"
                               "                                                AND d.deptype = 'i'\n"
                               "                                                AND c.contype = 'f' ) )\n"
                               "  ORDER BY 1\n",
                               "",
                               "",
                               "QPSQL");
static toSQL SQLTableTriggerMysql("toBrowser:TableTrigger",
                                  "SELECT * FROM INFORMATION_SCHEMA.TRIGGERS\n"
                                  "   WHERE event_object_schema = :f1<char[101]>\n"
                                  "       AND event_object_table = :f2<char[101]>\n"
                                  "   ORDER BY trigger_name\n",
                                  "",
                                  "5.0",
                                  "QMYSQL");
static toSQL SQLTableInfoMysql("toBrowser:TableInformation",
                               "show table status from `:own<noquote>` like :tab",
                               "Display information about a table",
                               "3.0",
                               "QMYSQL");
static toSQL SQLTableInfo("toBrowser:TableInformation",
                          "SELECT *\n"
                          "  FROM SYS.ALL_TABLES\n"
                          " WHERE OWNER = :f1<char[101]> AND Table_Name = :f2<char[101]>",
                          "");
static toSQL SQLTableInfoPgSQL("toBrowser:TableInformation",
                               "SELECT c.*\n"
                               "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                               " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                               "   AND c.relkind = 'r'\n"
                               "   AND c.relname = :f2",
                               "",
                               "7.1",
                               "QPSQL");
static toSQL SQLTableInfoSapDB("toBrowser:TableInformation",
                               "SELECT TABLENAME,PRIVILEGES,CREATEDATE,CREATETIME,UPDSTATDATE,UPDSTATTIME,ALTERDATE,ALTERTIME,TABLEID \n"
                               " FROM tables \n"
                               " WHERE tabletype = 'TABLE' and owner = upper(:f1<char[101]>) and tablename = :f2<char[101]>",
                               "",
                               "",
                               "SapDB");
static toSQL SQLTableStatistic("toBrowser:TableStatstics",
                               "SELECT description \"Description\", value(char_value,numeric_value) \"Value\" \n"
                               " FROM tablestatistics \n"
                               " WHERE owner = upper(:f1<char[101]>) and tablename = :f2<char[101]>",
                               "Table Statistics",
                               "",
                               "SapDB");
static toSQL SQLTablePartition("toBrowser:TablePartitions",
                               "select    p.partition_name \"Partition\"\n"
                               " , p.composite \"Composite\"\n"
                               " , p.num_rows \"Partition rows\"\n"
                               " , p.high_value \"High value\"\n"
                               " , p.subpartition_count \"Subpartitions\"\n"
                               " , p.partition_position \"Position\"\n"
                               " , s.subpartition_name \"Subpartition\"\n"
                               " , s.num_rows \"Subpartition rows\"\n"
                               " , s.subpartition_position \"Subpartition position\"\n"
                               "  from all_tab_partitions p,\n"
                               "       all_tab_subpartitions s\n"
                               " where p.table_owner = s.table_owner(+)\n"
                               "   and p.table_name = s.table_name(+)\n"
                               "   and p.partition_name = s.partition_name(+)\n"
                               "   and p.table_owner like upper(:table_owner<char[101]>)\n"
                               "   and p.table_name like upper(:table_name<char[101]>)\n"
                               " order by p.partition_name\n"
                               " , s.subpartition_name\n",
                               "Table partitions",
                               "0801");

// NOTE: this query is not used yet we need to implement some kind of flag in connection
// saying: "User has 'admin' rights and can see dba_* v$* tables"
static toSQL SQLTablePartitionDBA("toBrowser:TablePartitions",
                                  "select    p.partition_name \"Partition\"\n"
                                  " , p.composite \"Composite\"\n"
                                  " , p.num_rows \"Partition rows\"\n"
                                  " , round(seg.bytes/1024/1024) \"MB\"\n"
                                  " , p.high_value \"High value\"\n"
                                  " , p.subpartition_count \"Subpartitions\"\n"
                                  " , p.partition_position \"Position\"\n"
                                  " , s.subpartition_name \"Subpartition\"\n"
                                  " , s.num_rows \"Subpartition rows\"\n"
                                  " , s.subpartition_position \"Subpartition position\"\n"
                                  "  from all_tab_partitions p,\n"
                                  "       all_tab_subpartitions s,\n"
                                  "       dba_segments seg\n"
                                  " where p.table_owner = s.table_owner(+)\n"
                                  "   and p.table_name = s.table_name(+)\n"
                                  "   and p.partition_name = s.partition_name(+)\n"
                                  "   and p.table_owner like upper(:table_owner<char[101]>)\n"
                                  "   and p.table_name like upper(:table_name<char[101]>)\n"
                                  "   and ( \n"
                                  "         (p.partition_name = seg.partition_name and seg.segment_type='TABLE PARTITION') \n"
                                  "         or \n"
                                  "         (s.subpartition_name = seg.partition_name and seg.segment_type='TABLE SUBPARTITION') \n"
                                  "       ) \n"
                                  "   and p.table_name = seg.segment_name \n"
                                  " order by p.partition_name\n"
                                  " , s.subpartition_name\n",
                                  "",
                                  "0801");


toBrowserTableWidget::toBrowserTableWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
    setObjectName("toBrowserTableWidget");

    columnsWidget = new toResultCols(this);
    columnsWidget->setObjectName("columnsWidget");

    indexView = new toResultTableView(this);
    indexView->setObjectName("indexView");
    indexView->setSQL(SQLTableIndex);
    indexView->setReadAll(true);

    constraintsView = new toResultTableView(this);
    constraintsView->setObjectName("constraintsView");
    constraintsView->setSQL(SQLTableConstraint);
    constraintsView->setReadAll(true);

    referencesView = new toResultTableView(this);
    referencesView->setObjectName("referencesView");
    referencesView->setSQL(SQLTableReferences);
    referencesView->setReadAll(true);

#ifdef TORA_EXPERIMENTAL
    schemaView = new toResultDrawing(this);
    schemaView->setObjectName("schemaView");
    connect(schemaView->m_dotGraphView, SIGNAL(selected(QString const&)), this, SLOT(slotSelected(QString const&)), Qt::QueuedConnection);
#endif

    grantsView = new toResultGrants(this);
    grantsView->setObjectName("grantsView");

    triggersView = new toResultTableView(this);
    triggersView->setObjectName("triggersView");
    triggersView->setSQL(SQLTableTrigger);
    triggersView->setReadAll(true);

    resultData = new toResultTableData(this);
    resultData->setObjectName("resultData");

    resultInfo = new toResultItem(this);
    resultInfo->setObjectName("resultInfo");
    resultInfo->setSQL(SQLTableInfo);

    statisticsView = new toResultTableView(this);
    statisticsView->setObjectName("statisticsView");
    statisticsView->setSQL(SQLTableStatistic);
    statisticsView->setReadAll(true);

    partitionsView = new toResultTableView(this);
    partitionsView->setObjectName("partitionsView");
    partitionsView->setSQL(SQLTablePartition);
    partitionsView->setReadAll(true);

    extentsView = new toResultExtent(this);
    extentsView->setObjectName("extentsView");

    extractView = new toResultCode(this);
    extractView->setObjectName("extractView");

    changeConnection();
}

void toBrowserTableWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();
    // changeConnection() clears type field which should be set so
    // that extractor knows what type of object TOra is handling.
    setType("TABLE");

    toConnection & c = toConnection::currentConnection(this);

    addTab(columnsWidget, "&Columns");
    addTab(indexView, "&Indexes");

    if (c.providerIs("Oracle") || c.providerIs("QPSQL"))
        addTab(constraintsView, "C&onstraints");
    else
        constraintsView->hide();

    if (c.providerIs("Oracle"))
        addTab(referencesView, "&References");
    else
        referencesView->hide();

#ifdef TORA_EXPERIMENTAL
    {
        int pos = 0;
        if (c.providerIs("Oracle"))
            pos = addTab(schemaView, "&Schema");
        else
            schemaView->hide();
        if (!DotGraph::hasValidPath())
        {
            schemaView->setDisabled(true);
            schemaView->blockSignals(true);
            if (pos)
                setTabEnabled(pos, false);
        }
    }
#endif

    if (c.providerIs("Oracle"))
        addTab(grantsView, "&Grants");
    else
        grantsView->hide();

    addTab(triggersView, "Triggers");

    addTab(resultData, "&Data");

    addTab(resultInfo, "Information");

    if (c.providerIs("SapDB"))
        addTab(statisticsView, "Statistics");
    else
        statisticsView->hide();

    if (c.providerIs("Oracle"))
    {
        addTab(partitionsView, "Partitions");
        addTab(extentsView, "Extents");
    }
    else
    {
        partitionsView->hide();
        extentsView->hide();
    }

    if (c.providerIs("Oracle"))
        addTab(extractView, "Script");
    else
        extractView->hide();
}

void toBrowserTableWidget::slotSelected(const QString& object)
{
    emit selected(object);
}

//void toBrowserTableWidget::enableConstraints(bool enable)
//{
//    QString what(enable ? "ENABLE" : "DISABLE");
//    std::list<QString> migrate;
//    toConnection &conn = toConnection::currentConnection(this);
//
//    try
//    {
//        if (currentWidget() == constraintsView)
//        {
//            for (toResultTableView::iterator it(constraintsView); (*it).isValid(); it++)
//            {
//                if (constraintsView->isRowSelected(*it))
//                {
//                    Utils::toPush(migrate,
//                                  conn.getTraits().quote(schema()) + ":" +
//                                  "TABLE:" +
//                                  conn.getTraits().quote(object()) + ":" +
//                                  "CONSTRAINT:" +
//                                  conn.getTraits().quote((*it).data(Qt::EditRole).toString()) + ":" +
//                                  "DEFINITION:" +
//                                  what);
//                }
//            }
//        }
//        else if (currentWidget() == referencesView)
//        {
//            toResultModel *model = referencesView->model();
//            for (toResultTableView::iterator it(referencesView); (*it).isValid(); it++)
//            {
//                if (referencesView->isRowSelected(*it))
//                {
//                    Utils::toPush(migrate,
//                                  conn.getTraits().quote(model->data((*it).row(), 0).toString()) + ":" +
//                                  "TABLE:" +
//                                  conn.getTraits().quote(model->data((*it).row(), 1).toString()) + ":" +
//                                  "CONSTRAINT:" +
//                                  conn.getTraits().quote(model->data((*it).row(), 2).toString()) + ":" +
//                                  "DEFINITION:" +
//                                  what);
//                }
//            }
//        }
//        else
//        {
//            qDebug("toBrowserTableWidget::enableConstraints - unimplemented");
//            //             toResultView *lst = dynamic_cast<toResultView *>(SecondTab);
//            //             if (lst && lst->sqlName() == "toBrowser:TableTrigger") {
//            // Need work
//            //             }
//        }
//
//        if (migrate.begin() != migrate.end())
//        {
//            //obsolete std::list<QString> drop;
//            // toConnectionSubLoan connSub(conn);
//            // toExtract extract(conn, this);
//            // extract.setPrompt(false);
//            // extract.setHeading(false);
//            // QString sql = extract.migrate(drop, migrate);
//            // connSub->execute("BEGIN\n" + sql + "\nEND;");
//	    throw tr("toBrowserTableWidget::enableConstraints(bool enable) -> toExtract not implemented yet");
//        }
//    }
//    TOCATCH;
//}
