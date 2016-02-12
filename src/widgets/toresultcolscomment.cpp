
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

// Author: Thomas Porschberg, core <pberg@porschberg>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "widgets/toresultcolscomment.h"
#include "core/toconnection.h"
#include "core/tosql.h"
#include "core/utils.h"

#include <QtCore/QRegExp>
#include <QtGui/QFocusEvent>


static toSQL SQLChangeTableCommentMySQL(
    "toResultCols:ChangeTableComment",
    "ALTER TABLE %1 COMMENT = %2",
    "Set a comment on a table. Must have same % signs",
    "4.1",
    "QMYSQL");

static toSQL SQLChangeTableComment(
    "toResultCols:ChangeTableComment",
    "COMMENT ON TABLE %1 IS %2",
    "");

static toSQL SQLChangeTableCommentPG(
    "toResultCols:ChangeTableComment",
    "COMMENT ON TABLE %1 IS %2",
    "",
    "7.1",
    "QPSQL");

static toSQL SQLChangeTableCommentTD(
    "toResultCols:ChangeTableComment",
    "COMMENT ON TABLE %1 IS %2",
    "",
    "",
    "Teradata");

static toSQL SQLChangeColumnComment(
    "toResultCols:ChangeColumnComment",
    "COMMENT ON COLUMN %1 IS %2",
    "Set a comment on a column. Must have same % signs");

static toSQL SQLChangeColumnCommentPG(
    "toResultCols:ChangeColumnComment",
    "COMMENT ON COLUMN %1 IS %2",
    "",
    "7.1",
    "QPSQL");

static toSQL SQLChangeColumnCommentTD(
    "toResultCols:ChangeColumnComment",
    "COMMENT ON COLUMN %1 IS %2",
    "",
    "",
    "Teradata");


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
            toConnection &conn = toConnection::currentConnection(this);
            QString sql;
            if (Table)
                sql = SQLChangeTableComment(conn);
            else
                sql = SQLChangeColumnComment(conn);
            QString comment = text();
            comment.replace(QRegExp("'"), "''");
            comment = "'" + comment + "'";

#ifdef TORA3_QUERY
            conn.execute(sql.arg(Name).arg(comment));
#endif
        }
    }
    TOCATCH;
}

