
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

#include <QSettings>
#include <QHideEvent>

#include "todescribe.h"


// TODO/FIXME: these SQL statements are same as in tobrowser.cpp
// Is there any way how to reuse them without any new subclassing
// of special widgets for indexes/constraints etc.
// It returns strange errors if SQL statements are moved into
// standalone include file to include in both places.
// TODO: I'll propably break toBrowser GUI into smaller chunks
// to allow reuse in widgets such this one... later.
static toSQL SQLTableIndex("toDescribe:TableIndex",
                           "SELECT IND.index_name AS \"Index Name\",\n"
                           "       ind.column_name AS \"Column Name\",\n"
                           "       al.uniqueness AS \"Unique\",\n"
                           "       AL.index_type AS \"Type\",\n"
                           "       EX.column_expression AS \"Column Expression\"\n"
                           "  FROM SYS.ALL_IND_COLUMNS IND,\n"
                           "       SYS.ALL_IND_EXPRESSIONS EX,\n"
                           "       sys.All_Indexes AL\n"
                           " WHERE IND.INDEX_OWNER = :own<char[101]>\n"
                           "   AND IND.TABLE_NAME = :nam<char[101]>\n"
                           "   AND EX.index_owner ( + ) = IND.index_owner\n"
                           "   AND EX.index_name ( + ) = IND.index_name\n"
                           "   AND IND.index_name = AL.index_name ( + )\n"
                           "   AND IND.index_owner = AL.owner ( + )",
                           "List the indexes on a table",
                           "");
static toSQL SQLTableIndexSapDB("toDescribe:TableIndex",
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

static toSQL SQLTableIndexPG("toDescribe:TableIndex",
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
                             "PostgreSQL");

static toSQL SQLTableIndexMySQL("toDescribe:TableIndex",
                                "SHOW INDEX FROM `:f1<noquote>`.`:tab<noquote>`",
                                "",
                                "",
                                "MySQL");

static toSQL SQLTableConstraint(
    "toDescribe:TableConstraint",
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
       "           c.generated\n"
       "       FROM\n"
       "           sys.All_Constraints c,\n"
       "           sys.all_cons_columns a\n"
       "       WHERE\n"
       "               c.constraint_name = a.constraint_name\n"
       "           AND c.Owner = :f1<char[101]>\n"
       "           AND c.Table_Name = :f2<char[101]>\n"
       "       ORDER BY\n"
       "           c.constraint_name, a.constraint_name, a.position\n"
       "       ) main,\n"
       "       sys.all_cons_columns refs\n"
       "   WHERE\n"
       "           main.r_constraint_name = refs.constraint_name (+)\n"
       "       AND main.column_name = refs.column_name (+)",
    "List the constraints on a table",
    "");

static toSQL SQLTableConstraintPG(
    "toDescribe:TableConstraint",
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
    "PostgreSQL");


toDescribe::toDescribe(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);

    indexView->setSQL(SQLTableIndex);
    indexView->setReadAll(true);
    constraintsView->setSQL(SQLTableConstraint);
    constraintsView->setReadAll(true);

    QSettings s;
    s.beginGroup("toDescribe");
    restoreGeometry(s.value("geometry", QByteArray()).toByteArray());
    s.endGroup();
}

void toDescribe::hideEvent(QHideEvent * event)
{
    QSettings s;
    s.beginGroup("toDescribe");
    s.setValue("geometry", saveGeometry());
    s.endGroup();
    event->accept();
}

void toDescribe::changeParams(const QString & owner, const QString & table)
{
    columnsWidget->changeParams(owner, table);
    indexView->changeParams(owner, table);
    constraintsView->changeParams(owner, table);
}
