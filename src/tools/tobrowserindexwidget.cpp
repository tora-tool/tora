
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

#include "tools/tobrowserindexwidget.h"
#include "widgets/tobrowserbasewidget.h"
#include "tools/tostorage.h"
#include "widgets/toresultcode.h"
#include "widgets/toresultitem.h"
#include "widgets/toresulttableview.h"
#include "core/utils.h"

Util::RegisterInFactory<toBrowserIndexWidget, toBrowserWidgetFactory, toCache::CacheEntryType> regToBrowserIndexWidget(toCache::INDEX);

static toSQL SQLIndexColsMySQL3("toBrowser:IndexCols",
                                "SHOW INDEX FROM `:f1<noquote>`.`:f2<noquote>`",
                                "Display columns on which an index is built",
                                "3.23",
                                "QMYSQL");
static toSQL SQLIndexColsMySQL("toBrowser:IndexCols",
                               "SELECT * FROM INFORMATION_SCHEMA.STATISTICS\n"
                               "    WHERE table_schema = :f1<char[101]>\n"
                               "        AND table_name = :f2<char[101]>\n",
                               "",
                               "5.0",
                               "QMYSQL");
static toSQL SQLIndexCols("toBrowser:IndexCols",
                          "SELECT a.Table_Name,a.Column_Name,a.Column_Length,a.Descend,b.Column_Expression \" \"\n"
                          "  FROM sys.All_Ind_Columns a,sys.All_Ind_Expressions b\n"
                          " WHERE a.Index_Owner = :f1<char[101]> AND a.Index_Name = :f2<char[101]>\n"
                          "   AND a.Index_Owner = b.Index_Owner(+) AND a.Index_Name = b.Index_Name(+)\n"
                          "   AND a.column_Position = b.Column_Position(+)\n"
                          " ORDER BY a.Column_Position",
                          "",
                          "0801");
static toSQL SQLIndexCols8("toBrowser:IndexCols",
                           "SELECT Table_Name,Column_Name,Column_Length,Descend\n"
                           "  FROM SYS.ALL_IND_COLUMNS\n"
                           " WHERE Index_Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>\n"
                           " ORDER BY Column_Position",
                           "",
                           "0800");
static toSQL SQLIndexCols7("toBrowser:IndexCols",
                           "SELECT Table_Name,Column_Name,Column_Length\n"
                           "  FROM SYS.ALL_IND_COLUMNS\n"
                           " WHERE Index_Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>\n"
                           " ORDER BY Column_Position",
                           "",
                           "0703");
static toSQL SQLIndexColsPgSQL("toBrowser:IndexCols",
                               "SELECT a.attname,\n"
                               "       format_type(a.atttypid, a.atttypmod) as FORMAT,\n"
                               "       a.attnotnull,\n"
                               "       a.atthasdef\n"
                               "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid,\n"
                               "       pg_attribute a\n"
                               " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                               "   AND a.attrelid = c.oid AND c.relname = :f2\n"
                               "   AND a.attnum > 0\n"
                               " ORDER BY a.attnum\n",
                               "",
                               "7.1",
                               "QPSQL");
static toSQL SQLIndexColsSapDb("toBrowser:IndexCols",
                               "SELECT tablename,columnname,len \"Length\",DataType,Sort \n"
                               " FROM indexcolumns \n"
                               " WHERE  owner = upper(:f1<char[101]>) and indexname = upper(:f2<char[101]>)\n"
                               " ORDER BY indexname,columnno",
                               "",
                               "",
                               "SapDB");

static toSQL SQLIndexInfoSapDb("toBrowser:IndexInformation",
                               "SELECT  INDEXNAME,TABLENAME, TYPE, CREATEDATE,CREATETIME,INDEX_USED, DISABLED \n"
                               " FROM indexes\n"
                               " WHERE  owner = upper(:f1<char[101]>) and indexname = :f2<char[101]>\n",
                               "Display information about an index",
                               "",
                               "SapDB");
static toSQL SQLIndexInfo("toBrowser:IndexInformation",
                          "SELECT * FROM SYS.ALL_INDEXES\n"
                          " WHERE Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>",
                          "");
static toSQL SQLIndexStatistic("toBrowser:IndexStatstics",
                               "SELECT description \"Description\", value(char_value,numeric_value) \"Value\" \n"
                               " FROM indexstatistics \n"
                               " WHERE owner = upper(:f1<char[101]>) and indexname = :f2<char[101]>",
                               "Index Statistics",
                               "",
                               "SapDB");



toBrowserIndexWidget::toBrowserIndexWidget(QWidget * parent)
    : toBrowserBaseWidget(parent)
{
    setObjectName("toBrowserIndexWidget");

    columnView = new toResultTableView(this);
    columnView->setObjectName("columnView");
    columnView->setSQL(SQLIndexCols);
    columnView->setReadAll(true);

    resultInfo = new toResultItem(this);
    resultInfo->setObjectName("resultInfo");
    resultInfo->setSQL(SQLIndexInfo);

    extentsView = new toResultExtent(this);
    extentsView->setObjectName("extentsView");

    extractView = new toResultCode(this);
    extractView->setObjectName("extractView");

    statisticView = new toResultTableView(this);
    statisticView->setObjectName("statisticView");
    statisticView->setSQL(SQLIndexStatistic);
    statisticView->setReadAll(true);

    changeConnection();
}

void toBrowserIndexWidget::changeConnection()
{
    toBrowserBaseWidget::changeConnection();
    // changeConnection() clears type field which should be set so
    // that extractor knows what type of object TOra is handling.
    setType("INDEX");

    toConnection & c = toConnection::currentConnection(this);

    addTab(columnView, "&Columns");

    if ( c.providerIs("Oracle") || c.providerIs("SapDB"))
        addTab(resultInfo, "Information");
    else
        resultInfo->hide();

    if ( c.providerIs("Oracle"))
        addTab(extentsView, "Extents");
    else
        extentsView->hide();

    if (c.providerIs("SapDB"))
        addTab(statisticView, tr("&Statistic"));
    else
        statisticView->hide();

    if (c.providerIs("Oracle"))
        addTab(extractView, "Script");
    else
        extractView->hide();
}
