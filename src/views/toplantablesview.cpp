
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

#include "views/toplantablesview.h"

#if 0
select
sp.OBJECT#      , --NUMBER      Object number of the table or the index
sp.OBJECT_OWNER         , --VARCHAR2(30)        Name of the user who owns the schema containing the table or index
sp.OBJECT_NAME  , --VARCHAR2(30)        Name of the table or index
sp.OBJECT_ALIAS         , --VARCHAR2(65)        Alias for the object
sp.OBJECT_TYPE  ,  --VARCHAR2(20)       Type of the object
do.object_type  ,
dt.table_name,
di.table_name
from v$sql_plan sp
join dba_objects do on (sp.object# = do.object_id)
left outer join dba_tables dt on (do.owner = dt.owner and do.object_name = dt.table_name and do.object_type like 'TABLE%')
left outer join dba_indexes di on (do.owner = di.owner and do.object_name = di.index_name and do.object_type like 'INDEX%')

select --p.*,
        case when i.index_name is not null then i.owner||'.'||i.index_name else null end as idx
        , case when nvl(t.table_name, i.table_name) is not null
               then nvl(t.owner, i.owner)||'.'||nvl(t.table_name, i.table_name)
               else null
          end as tbl
        , t.num_rows
        , i.num_rows
        , case when t.table_name is not null then
          'dbms_stats.gather_table_stats(''"'||t.owner||'"'',''"'||t.table_name||'"'');'
          else null end as stat
from v$sql_plan p
left outer join dba_tables  t on (p.object_owner = t.owner and p.object_name = t.table_name)
left outer join dba_indexes i on (p.object_owner = i.owner and p.object_name = i.index_name)
where 1=1
and object# is not null
and sql_id = 'caqct1vtd84w9'
and child_number = 0

#endif

static toSQL toStatementTables("toStatementTables:ListTables",
"select                                                                                          \n"
"        case when i.index_name is not null then i.owner||'.'||i.index_name else null end as idx \n"
"        , case when nvl(t.table_name, i.table_name) is not null                                 \n"
"               then nvl(t.owner, i.owner)||'.'||nvl(t.table_name, i.table_name)                 \n"
"               else null \n"
"          end as tbl     \n"
"        , t.num_rows TABLE_ROWS \n"
"        , i.num_rows INDEX_ROWS \n"
"        , case when t.table_name is not null then                                               \n"
"          'dbms_stats.gather_table_stats(''\"'||t.owner||'\"'',''\"'||t.table_name||'\"'');'    \n"
"          else null end as stat                                                                 \n"
"from v$sql_plan p        \n"
"left outer join dba_tables  t on (p.object_owner = t.owner and p.object_name = t.table_name)    \n"
"left outer join dba_indexes i on (p.object_owner = i.owner and p.object_name = i.index_name)    \n"
"where 1=1                 \n"
" and object# is not null  \n"
" and sql_id = :sql_id<char[20],in> \n"
" and child_number = :child<int,in> \n"
,"List locks in a session"
,"1000"
,"Oracle"
);


toPlanTablesView::toPlanTablesView(QWidget *parent)
    : super(parent)
{
    setSQLName(QString::fromLatin1("toStatementTables:ListTables"));
}

#if 0
#include "parsing/tsqlparse.h"
#include <QtGui/QStandardItemModel>

void toPlanTablesView::setSQL(QString const& sql)
{
    QSet<QString> tableNames;
    unsigned emptyAliasCnt = 0;
    std::unique_ptr <SQLParser::Statement> source = StatementFactTwoParmSing::Instance().create("OracleDML", sql, "");

    SQLParser::Statement::token_const_iterator node;
    for (node = source->begin(); node != source->end(); ++node)
    {
        switch (node->getTokenType())
        {
            case SQLParser::Token::S_SUBQUERY_FACTORED:
            case SQLParser::Token::S_SUBQUERY_NESTED:
            {
                SQLParser::TokenSubquery const* snode = static_cast<SQLParser::TokenSubquery const*>(&*node);
                QList<SQLParser::Token*> tables = snode->nodeTables().values();

                foreach(SQLParser::Token * table, tables)
                {
                    if ( table->getTokenType() != SQLParser::Token::S_TABLE_REF)
                        continue;
                    // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
                    if ( source->translateAlias(table->child(0)->toStringRecursive(false).toUpper(), &*node))
                        continue;

                    SQLParser::TokenTable *tt = static_cast<SQLParser::TokenTable*>(table);
                    QString tableName = tt->toStringRecursive(false);
                    tableNames.insert(tableName);
                }
            }
            break;
            case SQLParser::Token::X_ROOT:
            {
                SQLParser::TokenSubquery const* snode = static_cast<SQLParser::TokenSubquery const*>(&*node);
                QList<SQLParser::Token*> tables = snode->nodeTables().values();
                foreach(SQLParser::Token * table, tables)
                {
                    if ( table->getTokenType() != SQLParser::Token::S_TABLE_REF)
                        continue;
                    // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
                    if ( source->translateAlias(table->child(0)->toStringRecursive(false).toUpper(), &*node))
                        continue;
                    SQLParser::TokenTable *tt = static_cast<SQLParser::TokenTable*>(table);
                    QString tableName = tt->toStringRecursive(false);

                    tableNames.insert(tableName);
                }
            }
            break;
        } // switch getTokenType
    } // for each node
#if 0
    QStandardItemModel *m = static_cast<QStandardItemModel*>(model());
    m->clear();
    for( int r=0; r < tableNames.size(); r++ )
    {
        QStandardItem *item = new QStandardItem(tableNames.values().at(r));
        m->insertRow(r, item);
    }
#endif
}
#endif
