
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

#include "config.h"
#include "toresulttableviewedit.h"
#include "toresultmodel.h"
#include "toconf.h"
#include "utils.h"
#include "toqvalue.h"
#include "toconfiguration.h"
#include "toconnection.h"
#include "tomemoeditor.h"
#include "tomain.h"

#include <QClipboard>
#include <QScrollBar>
#include <QItemDelegate>
#include <QSize>
#include <QFont>
#include <QFontMetrics>
#include <QProgressDialog>

toResultTableViewEdit::toResultTableViewEdit(bool readable,
        bool numberColumn,
        QWidget *parent,
        const char *name)
        : toResultTableView(readable, numberColumn, parent, name, true)
{

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ContiguousSelection);
}


toResultTableViewEdit::~toResultTableViewEdit()
{
}


void toResultTableViewEdit::query(const QString &SQL, const toQList &params)
{
    revertChanges();

    if (params.size() != 2)
        ;                       // assume it's a refresh
    else
    {
        toQList::const_iterator par = params.begin();
        Owner = *par;
        par++;
        Table = *par;
    }

    toQList empty;
    toResultTableView::query(SQL, empty);

    emit changed(false);

    if (!Model)
        return;                 // error

    // must be reconnected after every query
    connect(Model,
            SIGNAL(columnChanged(const QModelIndex &,
                                 const toQValue &,
                                 const toResultModel::Row &)),
            this,
            SLOT(recordChange(const QModelIndex &,
                              const toQValue &,
                              const toResultModel::Row &)));

    connect(Model,
            SIGNAL(rowAdded(const toResultModel::Row &)),
            this,
            SLOT(recordAdd(const toResultModel::Row &)));

    connect(Model,
            SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this,
            SLOT(handleNewRows(const QModelIndex &, int, int)));

    connect(Model,
            SIGNAL(modelReset()),
            this,
            SLOT(revertChanges()));

    connect(Model,
            SIGNAL(rowDeleted(const toResultModel::Row &)),
            this,
            SLOT(recordDelete(const toResultModel::Row &)));

    connect(toMainWidget(),
            SIGNAL(willCommit(toConnection &, bool)),
            this,
            SLOT(commitChanges(toConnection &, bool)));

    verticalHeader()->setVisible(true);
}


void toResultTableViewEdit::recordChange(const QModelIndex &index,
        const toQValue &newValue,
        const toResultModel::Row &row)
{
    // first, if it was an added row, find and update the ChangeSet so
    // they all get inserted as one.
    toQValue rowid = row[0];
    for (int changeIndex = 0; changeIndex < Changes.size(); changeIndex++)
    {
        if (Changes[changeIndex].kind == Add && Changes[changeIndex].row[0] == rowid)
        {
            Changes[changeIndex].row[index.column()] = newValue;
            return;
        }
    }

    // don't record if not changed
    if (newValue == row[index.column()])
        return;

    struct ChangeSet change;

    change.columnName = model()->headerData(index.column(),
                                            Qt::Horizontal,
                                            Qt::DisplayRole).toString();
    change.newValue = newValue;
    change.row      = row;
    change.column   = index.column();
    change.kind     = Update;

    Changes.append(change);
    emit changed(changed());
}


void toResultTableViewEdit::recordAdd(const toResultModel::Row &row)
{
    struct ChangeSet change;

    change.row      = row;
    change.kind     = Add;

    Changes.append(change);
    emit changed(changed());
}


void toResultTableViewEdit::recordDelete(const toResultModel::Row &row)
{
    struct ChangeSet change;

    change.row      = row;
    change.kind     = Delete;

    Changes.append(change);
    emit changed(changed());
}


void toResultTableViewEdit::commitDelete(ChangeSet &change, toConnection &conn)
{
    const toResultModel::HeaderList Headers = Model->headers();
    bool oracle = toIsOracle(conn);

    QString sql = QString("DELETE FROM %1.%2 ").arg(conn.quote(Owner)).arg(conn.quote(Table));
    sql += (" WHERE ");

    bool where = false;
    for (int i = 1; i < change.row.size(); i++)
    {
        if (!oracle || (!Headers[i].datatype.toUpper().startsWith(("LONG")) &&
                        !Headers[i].datatype.toUpper().contains(("LOB"))))
        {
            if (where)
                sql += " AND ";
            else
                where = true;

            sql += conn.quote(Headers[i].name);

            if (change.row[i].isNull())
                sql += " IS NULL";
            else
            {
                sql += "= :c";
                sql += QString::number(i);
                if (change.row[i].isBinary())
                    sql += "<raw_long>";
                else
                    sql += "<char[4000]>";
            }
        }
    }

    if (!where)
    {
        toStatusMessage(tr("This table contains only LOB/LONG columns and can not be edited"));
        return;
    }

    toQList args;
    for (int i = 1; i < change.row.size(); i++)
    {
        if (!change.row[i].isNull() && (!oracle || (!Headers[i].datatype.startsWith(("LONG")) &&
                                        !Headers[i].datatype.toUpper().contains(("LOB")))))
        {
            toPush(args, change.row[i]);
        }
    }

    conn.execute(sql, args);

    if (toConfigurationSingle::Instance().autoCommit())
        conn.commit();
    else
        toMainWidget()->setNeedCommit(conn);
}


void toResultTableViewEdit::commitAdd(ChangeSet &change, toConnection &conn)
{
    const toResultModel::HeaderList Headers = Model->headers();

    QString sql = QString("INSERT INTO %1.%2 (").arg(conn.quote(Owner)).arg(conn.quote(Table));

    int num = 0;
    for (int i = 1; i < change.row.size(); i++)
    {
        if (num > 0)
            sql += ",";
        sql += conn.quote(Model->headerData(
                              i,
                              Qt::Horizontal,
                              Qt::DisplayRole).toString());
        num++;
    }

    sql += ") VALUES (";

    num = 0;
    for (int i = 1; i < change.row.size(); i++, num++)
    {
        if (num > 0)
            sql += (",");
        sql += (":f");
        sql += QString::number(num + 1);

        if (change.row[i].isBinary())
        {
            if (Headers[i].datatype.toUpper().contains("LOB"))
                sql += ("<blob,in>");
            else
                sql += ("<raw_long,in>");
        }
        else
        {
            if (Headers[i].datatype.toUpper().contains("LOB"))
                sql += ("<varchar_long,in>");
            else
                sql += ("<char[4000],in>");
        }
    }

    sql += (")");

    toQList args;
    for (int i = 1; i < change.row.size(); i++)
        toPush(args, change.row[i]);

    toQuery q(conn, sql, args);

    if (toConfigurationSingle::Instance().autoCommit())
        conn.commit();
    else
        toMainWidget()->setNeedCommit(conn);
}


void toResultTableViewEdit::commitUpdate(ChangeSet &change, toConnection &conn)
{
    const toResultModel::HeaderList Headers = Model->headers();
    bool oracle = toIsOracle(conn);

    QString sql = QString("UPDATE %1.%2 SET ").arg(conn.quote(Owner)).arg(conn.quote(Table));
    sql += conn.quote(change.columnName);

    if (change.newValue.isNull())
        sql += (" = NULL");
    else
    {
        sql += ("= :f0");

        if (change.row[change.column].isBinary())
        {
            if (Headers[change.column].datatype.toUpper().contains("LOB"))
                sql += ("<blob,in>");
            else
                sql += ("<raw_long,in>");
        }
        else
        {
            if (Headers[change.column].datatype.toUpper().contains("LOB"))
                sql += ("<varchar_long,in>");
            else
                sql += ("<char[4000],in>");
        }
    }

    sql += (" WHERE (");
    int col = 1;
    bool where = false;

    for (toResultModel::Row::iterator j = change.row.begin() + 1;
            j != change.row.end();
            j++, col++)
    {

        QString columnName = conn.quote(Model->headerData(
                                            col,
                                            Qt::Horizontal,
                                            Qt::DisplayRole).toString());

        if (!oracle || (!Headers[col].datatype.toUpper().startsWith(("LONG")) &&
                        !Headers[col].datatype.toUpper().contains(("LOB"))))
        {
            if (where)
                sql += (" AND (");
            else
                where = true;

            sql += columnName;

            if ((*j).isNull())
            {
                sql += " IS NULL ";

                if ((*j).isNumber())
                    sql += ")";
                else
                {
                    sql += " OR " + columnName + " = :c";
                    sql += QString::number(col);
                    if ((*j).isBinary())
                        sql += ("<raw_long,in>)");
                    else
                        sql += ("<char[4000],in>)");
                }
            }
            else
            {
                sql += " = :c";
                sql += QString::number(col);
                if ((*j).isBinary())
                    sql += ("<raw_long,in>)");
                else
                    sql += ("<char[4000],in>)");
            }
        }
    }

    if (!where)
    {
        toStatusMessage(tr("This table contains only LOB/LONG "
                           "columns and can not be edited"));
        return;
    }

    toQList args;

    // the "SET = " value
    if (!change.newValue.isNull())
        toPush(args, change.newValue);

    col = 1;
    for (toResultModel::Row::iterator j = change.row.begin() + 1;
            j != change.row.end();
            j++, col++)
    {
        if (!oracle || (!Headers[col].datatype.toUpper().startsWith(("LONG")) &&
                        !Headers[col].datatype.toUpper().contains(("LOB"))))
        {
            if ((*j).isNull())
            {
                if (!(*j).isNumber())
                    toPush(args, toQValue(QString("")));
                // else don't push null for numbers
            }
            else
                toPush(args, (*j));
        }
    }

    toQuery q(conn, sql, args);
    if (toConfigurationSingle::Instance().autoCommit())
        conn.commit();
    else
        toMainWidget()->setNeedCommit(conn);
}


bool toResultTableViewEdit::commitChanges(bool status)
{
    // Check to make sure some changes were actually made
    if (Changes.size() < 1)
    {
        if (status)
            toStatusMessage(tr("No changes made"), false, false);
        return false;
    }

    toConnection &conn = connection();

    QProgressDialog progress(tr("Performing changes"),
                             tr("Cancel"),
                             0,
                             Changes.size(),
                             this);

    bool error = false;
    for (int changeIndex = 0; changeIndex < Changes.size(); changeIndex++)
    {
        progress.setValue(changeIndex);
        qApp->processEvents();
        if (progress.wasCanceled())
            break;

        try
        {
            struct ChangeSet &change = Changes[changeIndex];

            switch (change.kind)
            {
            case Delete:
                commitDelete(change, conn);
                break;
            case Add:
                commitAdd(change, conn);
                break;
            case Update:
                commitUpdate(change, conn);
                break;
            default:
                toStatusMessage(tr("Internal error."));
            }
        }
        catch (const QString &str)
        {
            toStatusMessage(str);
            error = true;
            break;
        }
    }

    toStatusMessage(tr("Saved %1 changes").arg(Changes.size(), 0, 10), false, false);

    if (error)
        refresh();
    else
        Changes.clear();

    emit changed(changed());
    return !error;
}


void toResultTableViewEdit::commitChanges(toConnection &conn, bool cmt)
{
    // make sure this is the same connection, we don't want to commit
    // this connection when somebody clicked 'commit' when in another
    // tool and another database.
    if (&conn != &connection())
        return;

    if (cmt)
    {
        commitChanges(false);
        connection().commit();  // make sure to commit connection
        // where our changes are.
    }
    else
        refresh();
}


void toResultTableViewEdit::revertChanges()
{
    Changes.clear();
    emit changed(changed());
}


void toResultTableViewEdit::handleNewRows(const QModelIndex &parent,
        int start,
        int end)
{
    int col = selectionModel()->currentIndex().column();
    if (col < 1)
        col = 1;
    QModelIndex index = Model->index(start - 1, col);

    selectionModel()->select(QItemSelection(index, index),
                             QItemSelectionModel::ClearAndSelect);
    setCurrentIndex(index);
}


void toResultTableViewEdit::addRecord(void)
{
    Model->addRow();
}


void toResultTableViewEdit::duplicateRecord(void)
{
    Model->addRow(selectionModel()->currentIndex());
}


void toResultTableViewEdit::deleteRecord(void)
{
    if (!selectionModel())
        return;

    QModelIndex ind = selectionModel()->currentIndex();
    if (ind.isValid())
        Model->deleteRow(ind);
}
