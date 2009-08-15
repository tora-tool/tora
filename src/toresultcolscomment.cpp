
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

