
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

#include "core/toresulttableviewedit.h"
#include "core/utils.h"
#include "core/toconfiguration.h"
#include "core/tomainwindow.h"
#include "core/toconnectiontraits.h"
#include "core/toconnectionsub.h"
#include "core/toglobalevent.h"

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QScrollBar>
#include <QtGui/QItemDelegate>
#include <QtCore/QSize>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QProgressDialog>

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


void toResultTableViewEdit::query(const QString &SQL, toQueryParams const& params, const std::list<QString> priKeys)
{
    revertChanges();

    setSqlAndParams(SQL, params);

    if (params.size() == 2)
    {
        toQueryParams::const_iterator par = params.begin();
        Owner = *par;
        par++;
        Table = *par;
    }

    toResultTableView::query(SQL, toQueryParams(), priKeys);

    emit changed(false);

    if (!Model)
        return;                 // error

    Model->setOwner(Owner);
    Model->setTable(Table);

    // must be reconnected after every query
    connect(Model,
            SIGNAL(columnChanged(const QModelIndex &,
                                 const toQValue &,
                                 const toQuery::Row &)),
            this,
            SLOT(recordChange(const QModelIndex &,
                              const toQValue &,
                              const toQuery::Row &)));

    connect(Model,
            SIGNAL(rowAdded(const toQuery::Row &)),
            this,
            SLOT(recordAdd(const toQuery::Row &)));

    connect(Model,
            SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this,
            SLOT(handleNewRows(const QModelIndex &, int, int)));

    connect(Model,
            SIGNAL(modelReset()),
            this,
            SLOT(revertChanges()));

    connect(Model,
            SIGNAL(rowDeleted(const toQuery::Row &)),
            this,
            SLOT(recordDelete(const toQuery::Row &)));

    verticalHeader()->setVisible(true);
}


void toResultTableViewEdit::recordChange(const QModelIndex &index,
        const toQValue &newValue,
        const toQuery::Row &row)
{
    // first, if it was an added row, find and update the ChangeSet so
    // they all get inserted as one.
    toQValue rowDesc = row[0];
    for (int changeIndex = 0; changeIndex < Changes.size(); changeIndex++)
    {
        if (Changes[changeIndex].kind == Add && Changes[changeIndex].row[0].getRowDesc().key == rowDesc.getRowDesc().key)
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


void toResultTableViewEdit::recordAdd(const toQuery::Row &row)
{
    struct ChangeSet change;

    change.row      = row;
    change.kind     = Add;

    Changes.append(change);
    emit changed(changed());
}


void toResultTableViewEdit::recordDelete(const toQuery::Row &row)
{
    // Loop through all previously recorded changes. If there is an insert (add)
    // statement for the row being deleted - remove it (as there is no point of
    // trying to insert a possibly bad row and throw exceptions then that row
    // must be deleted).
    QMutableListIterator<struct ChangeSet> j(Changes);
    struct ChangeSet cs;
    bool insertFound = false;
    while (j.hasNext() && !insertFound)
    {
        cs = j.next();
        if ((cs.row[0].getRowDesc().key == row[0].getRowDesc().key) &&
                (cs.kind == Add))
        {
            j.remove();
            insertFound = true;
        }
    }

    if (!insertFound)
    {
        struct ChangeSet change;

        change.row      = row;
        change.kind     = Delete;

        Changes.append(change);
    }
    emit changed(changed());
}


unsigned toResultTableViewEdit::commitDelete(ChangeSet &change, toConnection &conn)
{
    const toResultModel::HeaderList Headers = Model->headers();
    bool oracle = conn.providerIs("Oracle");

    QString sql = QString("DELETE FROM %1.%2 ").arg(conn.getTraits().quote(Owner)).arg(conn.getTraits().quote(Table));
    sql += (" WHERE ");
    int col = 1;
    bool where = false;
    for (toQuery::Row::iterator j = change.row.begin() + 1;
            j != change.row.end();
            j++, col++)
    {
        if ((*j).isComplexType())
        {
            Utils::toStatusMessage(tr("This table contains complex/user defined columns "
                                      "and can not be edited"));
            return 0;
        }

        if (!oracle || (!Headers[col].datatype.toUpper().startsWith(("LONG")) &&
                        !Headers[col].datatype.toUpper().contains(("LOB"))))
        {
            if (where)
                sql += " AND ";
            else
                where = true;

            sql += conn.getTraits().quote(Headers[col].name);

            if ((*j).isNull())
                sql += " IS NULL";
            else
            {
                sql += "= :c";
                sql += QString::number(col);
                if ((*j).isBinary())
                    sql += "<raw_long>";
                else
                    sql += "<char[4000]>";
            }
        }
    }

    if (!where)
    {
        Utils::toStatusMessage(tr("This table contains only LOB/LONG columns and can not be edited"));
        return 0;
    }

    toQueryParams args;
    for (int i = 1; i < change.row.size(); i++)
    {
        if (!change.row[i].isNull() && (!oracle || (!Headers[i].datatype.startsWith(("LONG")) &&
                                        !Headers[i].datatype.toUpper().contains(("LOB")))))
        {
            args << change.row[i];
        }
    }

	{
		toConnectionSubLoan c(conn);
		toQuery q(c, sql, args);
		
		if (toConfigurationSingle::Instance().autoCommit())
			c->commit();
		else
		{
			throw QString("Not implemented yet. %1").arg(__QHERE__);
			//// TODO toGlobalEventSingle::Instance().setNeedCommit(conn);
		}
		return q.rowsProcessed();
	}
}


unsigned toResultTableViewEdit::commitAdd(ChangeSet &change, toConnection &conn)
{
    const toResultModel::HeaderList Headers = Model->headers();

    QString sql = QString("INSERT INTO %1.%2 (").arg(conn.getTraits().quote(Owner)).arg(conn.getTraits().quote(Table));

    int num = 0;
    for (int i = 1; i < change.row.size(); i++)
    {
        if (num > 0)
            sql += ",";
        sql += conn.getTraits().quote(Model->headerData(
                                          i,
                                          Qt::Horizontal,
                                          Qt::DisplayRole).toString());
        num++;
    }

    sql += ") VALUES (";
    int col = 1;
    for (toQuery::Row::iterator j = change.row.begin() + 1;
            j != change.row.end();
            j++, col++)
    {
        if ((*j).isComplexType())
        {
            Utils::toStatusMessage(tr("This table contains complex/user defined columns "
                                      "and can not be edited"));
            return 0;
        }

        if (col > 1)
            sql += (",");
        sql += (":f");
        sql += QString::number(col);

        if ((*j).isBinary())
        {
            if (Headers[col].datatype.toUpper().contains("LOB"))
                sql += ("<blob,in>");
            else
                sql += ("<raw_long,in>");
        }
        else
        {
            if (Headers[col].datatype.toUpper().contains("LOB"))
                sql += ("<varchar_long,in>");
            else
                sql += ("<char[4000],in>");
        }
    }

    sql += (")");

    toQueryParams args;
    for (int i = 1; i < change.row.size(); i++)
        args << change.row[i];

	{
		toConnectionSubLoan c(conn);
		toQuery q(c, sql, args);

		if (toConfigurationSingle::Instance().autoCommit())
			c->commit();
		else
		{
			throw QString("Not implemented yet. %1").arg(__QHERE__);
			//toGlobalEventSingle::Instance().setNeedCommit(conn);
		}
		return q.rowsProcessed();
	}
}

unsigned toResultTableViewEdit::commitUpdate(ChangeSet &change, toConnection &conn)
{
    const toResultModel::HeaderList Headers = Model->headers();
    bool oracle = conn.providerIs("Oracle");

    QString sql = QString("UPDATE %1.%2 SET ").arg(conn.getTraits().quote(Owner)).arg(conn.getTraits().quote(Table));
    sql += conn.getTraits().quote(change.columnName);

    // set new value in update statement
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

    // set where clause for update statement
    sql += (" WHERE (");
    int col = 1;
    bool where = false;

    for (toQuery::Row::iterator j = change.row.begin() + 1;
            j != change.row.end();
            j++, col++)
    {

        QString columnName = conn.getTraits().quote(Model->headerData(
                                 col,
                                 Qt::Horizontal,
                                 Qt::DisplayRole).toString());

        if ((*j).isComplexType())
        {
            Utils::toStatusMessage(tr("This table contains complex/user defined columns "
                                      "and can not be edited"));
            return 0;
        }

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

                // QVariant cannot identify the type when value is null therefore
                // we use the actual database type for this check.
                if (Headers[col].datatype.startsWith("NUMBER") ||
                        Headers[col].datatype.startsWith("INT") ||
                        Headers[col].datatype.startsWith("DATE"))
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
        Utils::toStatusMessage(tr("This table contains only LOB/LONG "
                                  "columns and can not be edited"));
        return 0;
    }

    toQueryParams args;

    // the "SET = " value
    if (!change.newValue.isNull())
        args << change.newValue;

    col = 1;
    for (toQuery::Row::iterator j = change.row.begin() + 1;
            j != change.row.end();
            j++, col++)
    {
        if (!oracle || (!Headers[col].datatype.toUpper().startsWith(("LONG")) &&
                        !Headers[col].datatype.toUpper().contains(("LOB"))))
        {
            if ((*j).isNull())
            {
                if (!Headers[col].datatype.startsWith("NUMBER") &&
                        !Headers[col].datatype.startsWith("INT") &&
                        !Headers[col].datatype.startsWith("DATE"))
                    args << toQValue(QString(""));
                // else don't push null for numbers
            }
            else
                args << (*j);
        }
    }

	{
		toConnectionSubLoan c(conn);
		toQuery q(c, sql, args);
		if (toConfigurationSingle::Instance().autoCommit())
			c->commit();
		else
		{
			throw QString("Not implemented yet. %1").arg(__QHERE__);
			///toGlobalEventSingle::Instance().setNeedCommit(conn);
		}
		return q.rowsProcessed();
	}
}


bool toResultTableViewEdit::commitChanges(bool status)
{
    // Check to make sure some changes were actually made
	if (!Model)
		return false;

    if (Changes.size() < 1 && Model->getPriKeys().size() == 0)
    {
        if (status)
            Utils::toStatusMessage(tr("No changes made"), false, false);
        return false;
    }

    toConnection &conn = connection();

    QProgressDialog progress(tr("Performing changes"),
                             tr("Cancel"),
                             0,
                             Changes.size(),
                             this);

    bool error = false;
    unsigned updated = 0, added = 0, deleted = 0;
    if(Model->getPriKeys().size() == 0)
    {
        // No primary keys
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
                    deleted += commitDelete(change, conn);
                    break;
                case Add:
                    added += commitAdd(change, conn);
                    break;
                case Update:
                    updated += commitUpdate(change, conn);
                    break;
                default:
                    Utils::toStatusMessage(tr("Internal error."));
                    break;
                }
            }
            catch (const QString &str)
            {
                Utils::toStatusMessage(str);
                error = true;
                break;
            }
        }
    }
    else
    {
        try
        {
            Model->commitChanges(conn, updated, added, deleted);
            if (toConfigurationSingle::Instance().autoCommit())
                conn.commit();
            else
            {
    			throw QString("Not implemented yet. %1").arg(__QHERE__);
            	///toGlobalEventSingle::Instance().setNeedCommit(conn);
            }
        }
        catch(...)
        {
            conn.rollback();
            throw;
        }
    }

    Utils::toStatusMessage(tr("Saved %1 changes(updated %2, added %3, deleted %4)")
                           .arg(Changes.size(), 0, 10)
                           .arg(updated, 0, 10)
                           .arg(added, 0, 10)
                           .arg(deleted, 0, 10)
                           , false, false);
    if (!error)
        Changes.clear();

    emit changed(changed());
    return !error;
}


void toResultTableViewEdit::commitChanges(toConnection &conn)
{
    // make sure this is the same connection, we don't want to commit
    // this connection when somebody clicked 'commit' when in another
    // tool and another database.
    if (&conn != &connection())
        return;

    try {
    	bool success = commitChanges(false);
    	connection().commit();  // make sure to commit connection
    	// where our changes are.
    	// some cleanup work
    	if (success) Model->clearStatus();
    }
    TOCATCH
}

void toResultTableViewEdit::rollbackChanges(toConnection &conn)
{
    // make sure this is the same connection, we don't want to commit
    // this connection when somebody clicked 'commit' when in another
    // tool and another database.
    if (&conn != &connection())
        return;

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
    // mrj: this was a work around for a scrolling bug. Don't think we
    // need this anymore.

    // int col = selectionModel()->currentIndex().column();
    // if (col < 1)
    //     col = 1;
    // QModelIndex index = Model->index(start - 1, col);

    // selectionModel()->select(QItemSelection(index, index),
    //                          QItemSelectionModel::ClearAndSelect);
    // setCurrentIndex(index);
}


void toResultTableViewEdit::addRecord(void)
{
    Model->addRow(selectionModel()->currentIndex(), false);
}


void toResultTableViewEdit::duplicateRecord(void)
{
    Model->addRow(selectionModel()->currentIndex(), true);
}


void toResultTableViewEdit::deleteRecord(void)
{
    if (!selectionModel())
        return;

    QModelIndex ind = selectionModel()->currentIndex();
    if (ind.isValid())
        Model->deleteRow(ind);
}
