/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
// Author: Thomas Porschberg, core <pberg@porschberg>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "toresultcolscomment.h"
#include "toconnection.h"
#include "tosql.h"
#include "utils.h"

#include <qregexp.h>
#include <QFocusEvent>


static toSQL SQLChangeTableCommentMySQL(
    "toResultCols:ChangeTableComment",
    "ALTER TABLE %1 COMMENT = %2",
    "Set a comment on a table. Must have same % signs",
    "4.1",
    "MySQL");

static toSQL SQLChangeTableComment(
    "toResultCols:ChangeTableComment",
    "COMMENT ON TABLE %1 IS %2",
    "");

static toSQL SQLChangeTableCommentPG(
    "toResultCols:ChangeTableComment",
    "COMMENT ON TABLE %1 IS %2",
    "",
    "7.1",
    "PostgreSQL");

static toSQL SQLChangeColumnComment(
    "toResultCols:ChangeColumnComment",
    "COMMENT ON COLUMN %1 IS %2",
    "Set a comment on a column. Must have same % signs");

static toSQL SQLChangeColumnCommentPG(
    "toResultCols:ChangeColumnComment",
    "COMMENT ON COLUMN %1 IS %2",
    "",
    "7.1",
    "PostgreSQL");


toResultColsComment::toResultColsComment(QWidget *parent)
        : QLineEdit(parent), Changed(false)
{
    connect(this,
            SIGNAL(textChanged(const QString &)),
            this,
            SLOT(commentChanged()));
    connect(this,
            SIGNAL(editingFinished()),
            this,
            SLOT(saveUnchanged()));
}


void toResultColsComment::setComment(bool table,
                                     const QString &name,
                                     const QString &comment)
{
    saveUnchanged();
    disconnect(this,
               SIGNAL(textChanged(const QString &)),
               this,
               SLOT(commentChanged()));
    Table = table;
    Name = name;
    Changed = false;
    setText(comment);
    connect(this,
            SIGNAL(textChanged(const QString &)),
            this,
            SLOT(commentChanged()));
}


void toResultColsComment::commentChanged()
{
    Changed = true;
}


void toResultColsComment::saveUnchanged()
{
    try
    {
        if (!Name.isEmpty() && Changed)
        {
            Changed = false;
            toConnection &conn = toCurrentConnection(this);
            QString sql;
            if (Table)
                sql = SQLChangeTableComment(conn);
            else
                sql = SQLChangeColumnComment(conn);
            QString comment = text();
            comment.replace(QRegExp("'"), "''");
            comment = "'" + comment + "'";
            conn.execute(sql.arg(Name).arg(comment));
        }
    }
    TOCATCH;
}

