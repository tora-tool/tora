
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

#include "core/tosql.h"

static toSQL SQLCurrentSchema("Global:CurrentSchema",
                              "select sys_context('userenv', 'current_schema') from dual",
                              "Get address of an SQL statement");

static toSQL SQLConnectionHasTransaction("toConnection:HasTransaction"
        , "select nvl2(dbms_transaction.local_transaction_id, 1, 0) from dual"
        , "Returns 1 if transaction is active"
        , "0900"
        , "Oracle");

static toSQL SQLListObjectsInDatabase("toConnection:ListObjectsInDatabase",
                                      "select a.owner,a.object_name,a.object_type,b.comments\n"
                                      "  from sys.all_objects a,\n"
                                      "       sys.all_tab_comments b\n"
                                      " where a.owner = b.owner(+) and a.object_name = b.table_name(+)\n"
                                      , "List all the objects to cache for a connection"
                                      , "0800"
                                      , "Oracle");

static toSQL SQLListObjectInSchema("toConnection:ListObjectsInSchema",
                                   "select a.owner,a.object_name,a.object_type,b.comments\n"
                                   "  from sys.all_objects a,\n"
                                   "       sys.all_tab_comments b\n"
                                   " where a.owner = b.owner(+) and a.object_name = b.table_name(+)\n"
                                   "   and a.owner = :owner<char[101]> \n"
                                   , "List all the objects to cache for a connection"
                                   , "0800"
                                   , "Oracle");

/*
** 11g version, see $ORACLE_HOME/rdbms/admin/utlxplan.sql
*/
static toSQL SQLCreatePlanTable(toSQL::TOSQL_CREATEPLAN,
                                "CREATE TABLE %1 (\n"
                                "STATEMENT_ID        VARCHAR2(30),\n"
                                "PLAN_ID             NUMBER,\n"
                                "TIMESTAMP           DATE,\n"
                                "REMARKS             VARCHAR2(4000),\n"
                                "OPERATION           VARCHAR2(30),\n"
                                "OPTIONS             VARCHAR2(255),\n"
                                "OBJECT_NODE         VARCHAR2(128),\n"
                                "OBJECT_OWNER        VARCHAR2(30),\n"
                                "OBJECT_NAME         VARCHAR2(30),\n"
                                "OBJECT_ALIAS        VARCHAR2(65),\n"
                                "OBJECT_INSTANCE     NUMERIC,\n"
                                "OBJECT_TYPE         VARCHAR2(30),\n"
                                "OPTIMIZER           VARCHAR2(255),\n"
                                "SEARCH_COLUMNS      NUMBER,\n"
                                "ID                  NUMERIC,\n"
                                "PARENT_ID           NUMERIC,\n"
                                "DEPTH               NUMERIC,\n"
                                "POSITION            NUMERIC,\n"
                                "COST                NUMERIC,\n"
                                "CARDINALITY         NUMERIC,\n"
                                "BYTES               NUMERIC,\n"
                                "OTHER_TAG           VARCHAR2(255),\n"
                                "PARTITION_START     VARCHAR2(255),\n"
                                "PARTITION_STOP      VARCHAR2(255),\n"
                                "PARTITION_ID        NUMERIC,\n"
                                "OTHER               LONG,\n"
                                "DISTRIBUTION        VARCHAR2(30),\n"
                                "CPU_COST            NUMERIC,\n"
                                "IO_COST             NUMERIC,\n"
                                "TEMP_SPACE          NUMERIC,\n"
                                "ACCESS_PREDICATES   VARCHAR2(4000),\n"
                                "FILTER_PREDICATES   VARCHAR2(4000),\n"
                                "PROJECTION          VARCHAR2(4000),\n"
                                "TIME                NUMERIC,\n"
                                "QBLOCK_NAME         VARCHAR2(30),\n"
                                "OTHER_XML           CLOB\n"
                                ")",
                                "Create plan table, must have same % signs");
