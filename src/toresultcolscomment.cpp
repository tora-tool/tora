//
// C++ Implementation: toresultcolscomment
//
// Description: 
//
//
// Author: Thomas Porschberg, core <pberg@porschberg>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "toresultcolscomment.h"

#include <qregexp.h>

#include "toconnection.h"
#include "tosql.h"
#include "utils.h"

#include "toresultcolscomment.moc"

namespace {
	toSQL SQLChangeTableCommentMySQL("toResultCols:ChangeTableComment",
                                        "ALTER TABLE %1 COMMENT = %2",
                                        "Set a comment on a table. Must have same % signs",
                                        "4.1",
                                        "MySQL");

	toSQL SQLChangeTableComment("toResultCols:ChangeTableComment",
                                   "COMMENT ON TABLE %1 IS %2",
                                   "");

	toSQL SQLChangeColumnComment("toResultCols:ChangeColumnComment",
                                    "COMMENT ON COLUMN %1 IS %2",
                                    "Set a comment on a column. Must have same % signs");
}

toResultColsComment::toResultColsComment(QWidget *parent)
        : QLineEdit(parent), Changed(false), Cached(0)
{
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(commentChanged()));    
}

	void toResultColsComment::setComment(bool table, const QString &name, const QString &comment)
{
    saveUnchanged();
    disconnect(this, SIGNAL(textChanged(const QString &)), this, SLOT(commentChanged()));
    Table = table;
    Name = name;
    Cached = NULL;
    Changed = false;
    setText(comment);
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(commentChanged()));
}

void toResultColsComment::setCachedComment(bool table, const QString &name, QString &comment)
{
    saveUnchanged();
    disconnect(this, SIGNAL(textChanged(const QString &)), this, SLOT(commentChanged()));
    Table = table;
    Name = name;
    Cached = &comment;
    Changed = false;
    setText(comment);
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(commentChanged()));
}

void toResultColsComment::commentChanged()
{
    Changed = true;
}

void toResultColsComment::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    saveUnchanged();
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
            if (Cached)
                *Cached = text();
        }
    }
    TOCATCH
}

