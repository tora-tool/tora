/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toconnection.h"
#include "toresultcols.h"
#include "toresultcolscomment.h"
#include "toresulttableview.h"
#include "tosql.h"

#include <QCheckBox>
#include <QLabel>
#include <qregexp.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>


static toSQL SQLComment(
    "toResultCols:Comments",
    "SELECT Column_name,Comments FROM sys.All_Col_Comments\n"
    " WHERE Owner = :f1<char[100]>\n"
    "   AND Table_Name = :f2<char[100]>",
    "Display column comments");

static toSQL SQLCommentPG(
    "toResultCols:Comments",
    "SELECT a.attname,\n"
    "       pg_catalog.col_description ( a.attrelid,\n"
    "                                    a.attnum )\n"
    "  FROM pg_catalog.pg_attribute a,\n"
    "       pg_class c,\n"
    "       pg_namespace n\n"
    " WHERE n.nspname = :f1<char[100]>\n"
    "   AND c.relname = :f2<char[100]>\n"
    "   AND c.relkind = 'r'::\"char\"\n"
    "   AND c.relnamespace = n.OID\n"
    "   AND a.attrelid = c.OID\n"
    "   AND a.attnum > 0\n"
    "   AND NOT a.attisdropped",
    "",
    "7.1",
    "PostgreSQL");

static toSQL SQLTableCommentMySQL(
    "toResultCols:TableComment",
    "TOAD 15 SHOW TABLE STATUS FROM :f1<noquote> LIKE :f2<char[100]>",
    "Display Table comment",
    "4.1",
    "MySQL");

static toSQL SQLTableComment(
    "toResultCols:TableComment",
    "SELECT Comments FROM sys.All_Tab_Comments\n"
    " WHERE Owner = :f1<char[100]>\n"
    "   AND Table_Name = :f2<char[100]>",
    "");

// sql must return a row even if there's no comment for the table
static toSQL SQLTableCommentPG(
    "toResultCols:TableComment",
    "SELECT d.description\n"
    "  FROM pg_class c\n"
    "  LEFT OUTER JOIN pg_description d\n"
    "    ON d.objoid = c.OID,\n"
    "       pg_namespace n\n"
    " WHERE n.nspname = :f1<char[100]>\n"
    "   AND c.relname = :f2<char[100]>\n"
    "   AND c.relkind = 'r'::\"char\"\n"
    "   AND c.relnamespace = n.OID\n"
    "   AND d.objsubid = 0",
    "",
    "7.1",
    "PostgreSQL");

static toSQL SQLTableColumns(
    "toResultCols:ListCols",
    "SELECT column_name AS \"Column Name\",\n"
    "       RPAD ( DECODE ( data_type,\n"
    "                       'NUMBER',\n"
    "                       DECODE ( data_precision,\n"
    "                                NULL,\n"
    "                                DECODE ( data_scale,\n"
    "                                         0,\n"
    "                                         'INTEGER',\n"
    "                                         'NUMBER   ' ),\n"
    "                                'NUMBER   ' ),\n"
    "                       'RAW',\n"
    "                       'RAW      ',\n"
    "                       'CHAR',\n"
    "                       'CHAR     ',\n"
    "                       'NCHAR',\n"
    "                       'NCHAR    ',\n"
    "                       'UROWID',\n"
    "                       'UROWID   ',\n"
    "                       'VARCHAR2',\n"
    "                       'VARCHAR2 ',\n"
    "                       data_type ) ||\n"
    "              DECODE ( data_type,\n"
    "                       'DATE',\n"
    "                       NULL,\n"
    "                       'LONG',\n"
    "                       NULL,\n"
    "                       'NUMBER',\n"
    "                       DECODE ( data_precision,\n"
    "                                NULL,\n"
    "                                NULL,\n"
    "                                '(' ),\n"
    "                       'RAW',\n"
    "                       '(',\n"
    "                       'CHAR',\n"
    "                       '(',\n"
    "                       'NCHAR',\n"
    "                       '(',\n"
    "                       'UROWID',\n"
    "                       '(',\n"
    "                       'VARCHAR2',\n"
    "                       '(',\n"
    "                       'NVARCHAR2',\n"
    "                       '(',\n"
    "                       NULL ) ||\n"
    "              DECODE ( data_type,\n"
    "                       'RAW',\n"
    "                       data_length,\n"
    "                       'CHAR',\n"
    "                       data_length,\n"
    "                       'NCHAR',\n"
    "                       char_length,\n"
    "                       'UROWID',\n"
    "                       data_length,\n"
    "                       'VARCHAR2',\n"
    "                       data_length,\n"
    "                       'NVARCHAR2',\n"
    "                       char_length,\n"
    "                       'NUMBER',\n"
    "                       data_precision,\n"
    "                       NULL ) ||\n"
    "              DECODE ( data_type,\n"
    "                       'NUMBER',\n"
    "                       DECODE ( TO_CHAR ( data_precision ),\n"
    "                                NULL,\n"
    "                                NULL,\n"
    "                                DECODE ( TO_CHAR ( data_scale ),\n"
    "                                         NULL,\n"
    "                                         NULL,\n"
    "                                         0,\n"
    "                                         NULL,\n"
    "                                         ',' || data_scale ) ) ) ||\n"
    "              DECODE ( data_type,\n"
    "                       'DATE',\n"
    "                       NULL,\n"
    "                       'LONG',\n"
    "                       NULL,\n"
    "                       'NUMBER',\n"
    "                       DECODE ( data_precision,\n"
    "                                NULL,\n"
    "                                NULL,\n"
    "                                ')' ),\n"
    "                       'RAW',\n"
    "                       ')',\n"
    "                       'CHAR',\n"
    "                       ')',\n"
    "                       'NCHAR',\n"
    "                       ')',\n"
    "                       'UROWID',\n"
    "                       ')',\n"
    "                       'VARCHAR2',\n"
    "                       ')',\n"
    "                       'NVARCHAR2',\n"
    "                       ')',\n"
    "                       NULL ),\n"
    "                     32 ) AS \"Data Type\",\n"
    "       data_default AS \"Default\",\n"
    "       DECODE ( nullable,\n"
    "                'N',\n"
    "                'NOT NULL',\n"
    "                NULL ) AS \"NULL\",\n"
    "       ( SELECT comments\n"
    "           FROM sys.All_Col_Comments c\n"
    "          WHERE c.owner = tc.owner\n"
    "            AND c.table_name = tc.table_name\n"
    "            AND c.column_name = tc.column_name ) AS \"Comment\"\n"
    "  FROM sys.all_tab_columns tc\n"
    " WHERE owner = :f1<char[100]>\n"
    "   AND table_name = :f2<char[100]>\n"
    " ORDER BY column_id\n",
    "List table columns and defaults.",
    "1000");

static toSQL SQLTableColumns8(
    "toResultCols:ListCols",
    "SELECT column_name AS \"Column Name\",\n"
    "       RPAD ( DECODE ( data_type,\n"
    "		       'NUMBER',\n"
    "		       DECODE ( data_precision,\n"
    "				NULL,\n"
    "				DECODE ( data_scale,\n"
    "					 0,\n"
    "					 'INTEGER',\n"
    "					 'NUMBER   ' ),\n"
    "				'NUMBER   ' ),\n"
    "		       'RAW',\n"
    "		       'RAW      ',\n"
    "		       'CHAR',\n"
    "		       'CHAR     ',\n"
    "		       'NCHAR',\n"
    "		       'NCHAR    ',\n"
    "		       'UROWID',\n"
    "		       'UROWID   ',\n"
    "		       'VARCHAR2',\n"
    "		       'VARCHAR2 ',\n"
    "		       data_type ) ||\n"
    "	      DECODE ( data_type,\n"
    "		       'DATE',\n"
    "		       NULL,\n"
    "		       'LONG',\n"
    "		       NULL,\n"
    "		       'NUMBER',\n"
    "		       DECODE ( data_precision,\n"
    "				NULL,\n"
    "				NULL,\n"
    "				'(' ),\n"
    "		       'RAW',\n"
    "		       '(',\n"
    "		       'CHAR',\n"
    "		       '(',\n"
    "		       'NCHAR',\n"
    "		       '(',\n"
    "		       'UROWID',\n"
    "		       '(',\n"
    "		       'VARCHAR2',\n"
    "		       '(',\n"
    "		       'NVARCHAR2',\n"
    "		       '(',\n"
    "		       NULL ) ||\n"
    "	      DECODE ( data_type,\n"
    "		       'RAW',\n"
    "		       data_length,\n"
    "		       'CHAR',\n"
    "		       data_length,\n"
    "		       'UROWID',\n"
    "		       data_length,\n"
    "		       'VARCHAR2',\n"
    "		       data_length,\n"
    "		       'NUMBER',\n"
    "		       data_precision,\n"
    "		       NULL ) ||\n"
    "	      DECODE ( data_type,\n"
    "		       'NUMBER',\n"
    "		       DECODE ( TO_CHAR ( data_precision ),\n"
    "				NULL,\n"
    "				NULL,\n"
    "				DECODE ( TO_CHAR ( data_scale ),\n"
    "					 NULL,\n"
    "					 NULL,\n"
    "					 0,\n"
    "					 NULL,\n"
    "					 ',' || data_scale ) ) ) ||\n"
    "	      DECODE ( data_type,\n"
    "		       'DATE',\n"
    "		       NULL,\n"
    "		       'LONG',\n"
    "		       NULL,\n"
    "		       'NUMBER',\n"
    "		       DECODE ( data_precision,\n"
    "				NULL,\n"
    "				NULL,\n"
    "				')' ),\n"
    "		       'RAW',\n"
    "		       ')',\n"
    "		       'CHAR',\n"
    "		       ')',\n"
    "		       'NCHAR',\n"
    "		       ')',\n"
    "		       'UROWID',\n"
    "		       ')',\n"
    "		       'VARCHAR2',\n"
    "		       ')',\n"
    "		       'NVARCHAR2',\n"
    "		       ')',\n"
    "		       NULL ),\n"
    "		     32 ) AS \"Data Type\",\n"
    "       data_default AS \"Default\",\n"
    "       DECODE ( nullable,\n"
    "		'N',\n"
    "		'NOT NULL',\n"
    "		NULL ) AS \"NULL\",\n"
    "       ( SELECT comments\n"
    "	   FROM sys.All_Col_Comments c\n"
    "	  WHERE c.owner = tc.owner\n"
    "	    AND c.table_name = tc.table_name\n"
    "	    AND c.column_name = tc.column_name ) AS \"Comment\"\n"
    "  FROM sys.all_tab_columns tc\n"
    " WHERE owner = :f1<char[100]>\n"
    "   AND table_name = :f2<char[100]>\n"
    " ORDER BY column_id",
    "",
    "0800");

static toSQL SQLTableColumnsPG(
    "toResultCols:ListCols",
    "SELECT a.attname AS \"Column Name\",\n"
    "       pg_catalog.format_type ( a.atttypid,\n"
    "                                a.atttypmod ) AS \"Format Type\",\n"
    "       ( SELECT substring ( pg_catalog.pg_get_expr ( d.adbin,\n"
    "                                                     d.adrelid ) FOR 128 )\n"
    "           FROM pg_catalog.pg_attrdef d\n"
    "          WHERE d.adrelid = a.attrelid\n"
    "            AND d.adnum = a.attnum\n"
    "            AND a.atthasdef ) AS \"Modifiers\",\n"
    "       a.attnotnull AS \"NULL\",\n"
    "       pg_catalog.col_description ( a.attrelid,\n"
    "                                    a.attnum ) AS \"Comment\"\n"
    "  FROM pg_catalog.pg_attribute a,\n"
    "       pg_class c,\n"
    "       pg_namespace n\n"
    " WHERE n.nspname = :f1<char[100]>\n"
    "   AND c.relname = :f2<char[100]>\n"
    "   AND c.relkind = 'r'::\"char\"\n"
    "   AND c.OID = a.attrelid\n"
    "   AND c.relnamespace = n.OID\n"
    "   AND a.attnum > 0\n"
    "   AND NOT a.attisdropped\n"
    " ORDER BY a.attnum",
    "",
    "7.1",
    "PostgreSQL");

static toSQL SQLTableColumnsMySql(
    "toResultCols:ListCols",
    "SHOW FULL COLUMNS FROM :f1<noquote>",
    "",
    "3.23",
    "MySQL");


toResultCols::toResultCols(QWidget *parent, const char *name, Qt::WFlags f)
    : QWidget(parent, f) {

    if(name)
        setObjectName(name);

    QVBoxLayout *vbox = new QVBoxLayout;
    QWidget     *box  = new QWidget(this);
    QHBoxLayout *hbox = new QHBoxLayout;

    Title = new QLabel(box);
    hbox->addWidget(Title);
    Title->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,
                                     QSizePolicy::Maximum));

    Comment = new QLabel(box);
    hbox->addWidget(Comment);
    Comment->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                       QSizePolicy::Maximum));

    EditComment = new toResultColsComment(box);
    hbox->addWidget(EditComment);
    EditComment->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                           QSizePolicy::Maximum));
    EditComment->hide();

    Edit = new QCheckBox("Edit comments", box);
    hbox->addWidget(Edit);
    Edit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
                                    QSizePolicy::Maximum));
    connect(Edit, SIGNAL(toggled(bool)), this, SLOT(editComment(bool)));

    hbox->setContentsMargins(2, 2, 2, 2);
    hbox->setSpacing(0);
    box->setLayout(hbox);
    vbox->addWidget(box);

    Columns = new toResultTableView(true, true, this);
    Columns->setDisableTab(false);
    Columns->setReadAll(true);
    vbox->addWidget(Columns);
    connect(Columns, SIGNAL(done()), this, SLOT(done()));

    MySQLColumns = new toResultTableView(true, true, this);
    MySQLColumns->hide();
    MySQLColumns->setDisableTab(false);
    MySQLColumns->setReadAll(true);
    MySQLColumns->setSQL(SQLTableColumns);
    vbox->addWidget(MySQLColumns);
    connect(MySQLColumns, SIGNAL(done()), this, SLOT(done()));

    ColumnComments = NULL;

    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    this->setLayout(vbox);

    Header = true;
    resize(640, 300);
}


void toResultCols::displayHeader(bool display) {
    Header = display;

    Title->setVisible(display);
    Comment->setVisible(display && !Edit->isChecked());
    EditComment->setVisible(display && Edit->isChecked());
    Edit->setVisible(display);
}


void toResultCols::query(const QString &sql, const toQList &param) {
    QString Owner;
    QString Name;
    QString object;

    toConnection &conn = connection();

    try {
        toQList subp;

        toQList::iterator cp = ((toQList &) param).begin();
        if(cp != ((toQList &) param).end()) {
            object = conn.quote(*cp);
            Owner = *cp;
        }
        cp++;
        if(cp != ((toQList &) param).end()) {
            object += QString::fromLatin1(".");
            object += conn.quote(*cp);
            Name = *cp;
        }
        else {
            Name = Owner;
            if(toIsMySQL(conn))
                Owner = connection().database();
            else
                Owner = connection().user().toUpper();
        }

        TableName = conn.quote(Owner) + "." + conn.quote(Name);

        if(toIsMySQL(conn)) {
            MySQLColumns->query(sql, param);
            MySQLColumns->show();
            Columns->hide();
            MySQLColumns->setSQL(SQLTableColumns);

            if(Owner.isEmpty())
                MySQLColumns->changeParams(Name);
            else
                MySQLColumns->changeParams(Owner + "." + Name);
        }
        else {
            Columns->setSQL(SQLTableColumns);
            Columns->changeParams(Owner, Name);
            Columns->show();
            MySQLColumns->hide();
        }
    }
    TOCATCH;

    try {
        QString synonym;
        const toConnection::objectName &name = conn.realName(
            object,
            synonym,
            false);

        QString label = QString::fromLatin1("<B>");
        if(!synonym.isEmpty()) {
            label += conn.quote(synonym);
            label += tr("</B> synonym for <B>");
        }
        label += conn.quote(name.Owner);
        if (label != QString::fromLatin1("<B>"))
            label += QString::fromLatin1(".");
        label += conn.quote(name.Name);

        label += QString::fromLatin1("</B>");
        if (!name.Comment.isNull()) {
            Comment->setText(QString::fromLatin1(" - ") + name.Comment);
            EditComment->setComment(
                true,
                conn.quote(name.Owner) + "." + conn.quote(name.Name),
                const_cast<QString &>(name.Comment));
        }
        else {
            Comment->setText(QString::null);
            EditComment->setComment(
                true,
                conn.quote(name.Owner) + "." + conn.quote(name.Name),
                const_cast<QString &>(name.Comment));
        }

        Title->setText(label);
    }
    catch(const QString &) {
        // expected for databases that don't support synonyms
        ;
    }

    try {
        QString label = QString::fromLatin1("<B>");
        label += object;
        label += QString::fromLatin1("</B>");

        if(connection().provider() == "Oracle" ||
           connection().provider() == "MySQL" ||
           connection().provider() == "PostgreSQL")
        {
            toConnection &conn = connection();
            toQuery query(conn, SQLTableComment, Owner, Name);
            QString t;
            if(!query.eof()) {
                t += QString::fromLatin1(" - ");
                QString comment = query.readValueNull();
                EditComment->setComment(true, TableName, comment);
                t += comment;
            }
            Comment->setText(t);
            editComment(Edit->isChecked());
            Edit->setEnabled(true);
        }
        else {
            editComment(false);
            Edit->setEnabled(false);
        }

        Title->setText(label);
    }
    catch(const QString &str) {
        Title->setText(str);
        toStatusMessage(str);
    }
}


void toResultCols::done() {
    setUpdatesEnabled(false);

    // setup the comments
    bool visible = false;
    if(ColumnComments) {
        visible = ColumnComments->isVisible();
        delete ColumnComments;
    }
    QScrollArea *scroll = new QScrollArea(this);
    ColumnComments = scroll;
    ColumnComments->setVisible(visible);
    layout()->addWidget(ColumnComments);

    QWidget *container = new QWidget(scroll);
    scroll->setWidget(container);
    scroll->setWidgetResizable(true);

    toResultTableView *tv;
    if(!toIsMySQL(connection()))
        tv = Columns;
    else
        tv = MySQLColumns;
    toResultTableView::iterator it(tv);

    QGridLayout *grid = new QGridLayout;
    grid->setContentsMargins(2, 2, 2, 2);
    grid->setSpacing(5);

    int row;
    for(row = 0; (*it).isValid(); row++, it++) {
        QString column  = tv->model()->data(row, 1).toString();
        QString comment = tv->model()->data(row, "Comment").toString();

        toResultColsComment *com = new toResultColsComment(container);
        com->setComment(false,
                        TableName + "." + connection().quote(column),
                        comment);
        grid->addWidget(new QLabel(column));
        grid->addWidget(com, row, 1);
    }

    // add widget at bottom of grid that can resize
    grid->addWidget(new QWidget(container), row, 0);
    grid->setRowStretch(row, 1);

    container->setLayout(grid);
    setUpdatesEnabled(true);
}


void toResultCols::editComment(bool val) {
    if(!toIsMySQL(connection())) {
        Columns->setVisible(!val);
        MySQLColumns->setVisible(false);
    }
    else {
        Columns->setVisible(false);
        MySQLColumns->setVisible(!val);
    }

    if(ColumnComments)
        ColumnComments->setVisible(val);
    EditComment->setVisible(val);
    Comment->setVisible(!val);
}
