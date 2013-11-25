
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

#include "core/toresultmodeledit.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconf.h"
#include "core/toconfiguration.h"
#include "core/toqvalue.h"
#include "core/toeventquery.h"
#include "core/toconnectiontraits.h"

#include <QtCore/QDebug>
#include <QtCore/QMimeData>

toResultModelEdit::toResultModelEdit(toEventQuery *query,
                             std::list<QString> priKeys,
                             QObject *parent,
                             bool read)
    : toResultModel(query, priKeys, parent, read)
{
	setSupportedDragActions(Qt::CopyAction | Qt::MoveAction);
}

toResultModelEdit::~toResultModelEdit()
{
}

int toResultModelEdit::addRow(QModelIndex ind, bool duplicate)
{
    int newRowPos;
    if (ind.isValid())
        newRowPos = ind.row() + 1; // new row is inserted right after the current one
    else
    {
        if (!duplicate || Rows.size() > 0)
            newRowPos = Rows.size() + 1; // new row is appended at the end
        else
            return -1; // unable to duplicate a record if there are no records
    }
    beginInsertRows(QModelIndex(), newRowPos, newRowPos);

    toQuery::Row row;
    toRowDesc rowDesc;
    rowDesc.key = CurrRowKey++;
    rowDesc.status = ADDED;

    if (duplicate)
    {
        // Create a duplicate of current row
        row = Rows[ind.row()];
        // Reset a 0'th column
        row[0] = rowDesc;
    }
    else
    {
        // Create a new empty row
        row.append(toQValue(rowDesc));

        // null out the rest of the row
        int cols = Headers.size();
        for (int j = 1; j < cols; j++)
            row.append(toQValue());
    }

    Rows.insert(newRowPos, row);
    endInsertRows();
    emit rowAdded(row);
    return newRowPos;
} // addRow


void toResultModelEdit::deleteRow(QModelIndex index)
{
    if (!index.isValid() || index.row() >= Rows.size())
        return;

    toQuery::Row deleted = Rows[index.row()];
    toRowDesc rowDesc = deleted[0].getRowDesc();

    if(rowDesc.status == REMOVED)
    {
        //Make sure removed row can't be removed twice
        return;
    }
    else if(rowDesc.status == ADDED)
    {
        //Newly added record can be removed regularly
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        Rows.takeAt(index.row());
        endRemoveRows();
    }
    else  //Existed and Modified
    {
        rowDesc.status = REMOVED;
        Rows[index.row()][0] = toQValue(rowDesc);
    }
    emit rowDeleted(deleted);
}

void toResultModelEdit::clearStatus()
{
    // Go through all records and set their status to be existed
    for(toQuery::RowList::iterator ite = Rows.begin(); ite != Rows.end(); ite++)
    {
        toRowDesc rowDesc = ite->at(0).getRowDesc();
        if(rowDesc.status == REMOVED)
        {
            ite = Rows.erase(ite);
            if(ite == Rows.end())
                break;
        }
        else if(rowDesc.status != EXISTED)
        {
            rowDesc.status = EXISTED;
            (*ite)[0] = toQValue(rowDesc);
        }
    }
    emit headerDataChanged(Qt::Vertical, 0, Rows.size() - 1);
}

bool toResultModelEdit::changed(void)
{
    return !Changes.isEmpty();
}

bool toResultModelEdit::dropMimeData(const QMimeData *data,
                                 Qt::DropAction action,
                                 int row,
                                 int column,
                                 const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;
    if (column == 0)
        return false;           // can't change row number

    if(row < 0 || column < 0)
    {
        // friggen qt expects me to figure it out.  you'd think -1
        // wouldn't happen often but you'd be wrong.
        row = parent.row();
        column = parent.column();
    }

    if(row < 0)
    {
        row = addRow();
        if(row < 0)
            return false;
    }

    if (data->hasText())
    {
        // can't do anything with an item without a valid column
        if(column < 0)
        {
            if (data->hasFormat("application/vnd.int.list"))
            {
                QByteArray source = data->data("application/vnd.int.list");
                QDataStream stream(&source, QIODevice::ReadOnly);
                int sourceRow = 0;
                int sourceCol = 0;
                stream >> sourceRow;
                stream >> sourceCol;

                if(sourceCol > 0)
                    column = sourceCol;
                else
                    return false;
            }
            else
                return false;
        }

        QModelIndex ind = index(row, column);
        setData(ind, QVariant(data->text()));
        return true;
    }
    else if (data->hasFormat("application/vnd.tomodel.list"))
    {
        QByteArray source = data->data("application/vnd.tomodel.list");
        QDataStream stream(&source, QIODevice::ReadOnly);

        if (column < 0)
            column = 1;

        int rows;
        int columns;
        stream >> rows;
        stream >> columns;

        // count of rows down we've gone so far
        int counter = 0;
        while (!stream.atEnd() && column <= columnCount())
        {
            QString text;
            stream >> text;

            QModelIndex ind = index(row, column);
            setData(ind, QVariant(text));

            row++;
            counter++;
            if (counter >= rows)
            {
                row -= counter;
                counter = 0;
                column++;
            }

            if (!stream.atEnd() && row >= rowCount())
                row = addRow();

            if (column >= columnCount())
                return true;    // drop data past end of columns
        }

        return true;
    }

    return false;
}


Qt::DropActions toResultModelEdit::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction;
}

bool toResultModelEdit::setData(const QModelIndex &index,
                            const QVariant &_value,
                            int role)
{
    if (role != Qt::EditRole)
        return false;

    if (index.column() == 0)
        return false;           // can't change number column

    if (index.row() > Rows.size() - 1 || index.column() > Headers.size() - PriKeys.size() - 1)
        return false;

    toQuery::Row &row = Rows[index.row()];
    toQValue newValue = toQValue::fromVariant(_value);
    toRowDesc rowDesc = Rows.at(index.row())[0].getRowDesc();
    if (rowDesc.status == EXISTED && !(row[index.column()] == newValue))
    {
        // leave row that's added as in status added
        rowDesc.status = MODIFIED;
        Rows[index.row()][0] = toQValue(rowDesc);
    }
    if(PriKeys.size() == 0)
    {
        // If no prikey is used, data is recorded in change list
    	toQuery::Row oldRow = row;           // keep old version
        row[index.column() + PriKeys.size()] = newValue;
        // for writing to the database
        emit columnChanged(index, newValue, oldRow);
    }
    else
    {
        if(!row[index.column() + PriKeys.size()].updateNewValue(newValue))
            return false;
        qDebug() << "Value is changed from " << row[index.column() + PriKeys.size()] << " to " << newValue << "At " << index;
    }

    // for the view
    emit dataChanged(index, index);

    return true;
}

Qt::ItemFlags toResultModelEdit::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    Qt::ItemFlags fl = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;

    if (index.column() == 0)
        return fl;              // row number column

    if (!index.isValid() || index.row() >= Rows.size())
    {
    	return Qt::ItemIsDropEnabled | defaultFlags;
    }

    toQuery::Row const& row = Rows.at(index.row());
    if (index.column() + PriKeys.size() >= row.size())
        return defaultFlags;

    toQValue const &data = row.at(index.column() + PriKeys.size());
    if (data.isComplexType())
    {
        return ( defaultFlags | fl ) & ~Qt::ItemIsEditable;
    }

    fl |= defaultFlags | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;

    //Check the status of current record
    toRowDesc rowDesc = row.at(0).getRowDesc();
    if (rowDesc.status == REMOVED)
        fl &= ~Qt::ItemIsEditable;
    return fl;
}

void toResultModelEdit::commitUpdate(toConnection &conn, const toQuery::Row &row, unsigned int &updated)
{
    QString sql = QString("UPDATE %1.%2 SET ").arg(conn.getTraits().quote(Owner)).arg(conn.getTraits().quote(Table));
    int num = 0;
    toQueryParams args;
    for (int i = PriKeys.size() + 1; i < Headers.size(); i++)
    {
        if (row[i].isModified())
        {
            // Only append columns that is not null
            if (num > 0)
            {
                sql += ',';
            }
            num++;
            // Construct place holder
            sql += conn.getTraits().quote(Headers[i].name) + "=:f" + QString::number(num);

            if (row[i].isBinary())
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
            // Construct value list
            args << row[i];
        }
    }
    sql += " WHERE ";
    std::list<QString>::iterator ite;
    int i;
    for (i = 1, ite = PriKeys.begin(); ite != PriKeys.end(); ite++, i++)
    {
        if(i > 1)
        {
            sql += " AND ";
        }
        sql += *ite + "=:k" + QString::number(i);
        sql += "<char[4000],in>";
        args << row[i];
    }
    qDebug() << sql;
	{
		    	toConnectionSubLoan c(conn);
				toQuery q(c, sql, args);
				updated += q.rowsProcessed();
	}
}
void toResultModelEdit::commitAdd(toConnection &conn, const toQuery::Row &row, unsigned int &added)
{
    QString sql = QString("INSERT INTO %1.%2 (").arg(conn.getTraits().quote(Owner)).arg(conn.getTraits().quote(Table));
    QString sqlColumns, sqlValuePlaceHolders;
    int num = 0;
    toQueryParams args;
    for (int i = PriKeys.size() + 1; i < Headers.size(); i++)
    {
        if (!row[i].isNull())
        {
            // Only append columns that is not null
            if (num > 0)
            {
                sqlColumns += ',';
                sqlValuePlaceHolders += ',';
            }
            num++;
            sqlColumns += conn.getTraits().quote(Headers[i].name);
            // Construct place holder
            sqlValuePlaceHolders += (":f");
            sqlValuePlaceHolders += QString::number(num);

            if (row[i].isBinary())
            {
                if (Headers[i].datatype.toUpper().contains("LOB"))
                    sqlValuePlaceHolders += ("<blob,in>");
                else
                    sqlValuePlaceHolders += ("<raw_long,in>");
            }
            else
            {
                if (Headers[i].datatype.toUpper().contains("LOB"))
                    sqlValuePlaceHolders += ("<varchar_long,in>");
                else
                    sqlValuePlaceHolders += ("<char[4000],in>");
            }
            // Construct value list
            args << row[i];
        }
    }
    sql = sql + sqlColumns + ") VALUES (" + sqlValuePlaceHolders + ")";
    qDebug() << sql;
    {
    	toConnectionSubLoan c(conn);
    	toQuery q(c, sql, args);
    	added += q.rowsProcessed();
    }
}
void toResultModelEdit::commitDelete(toConnection &conn, const toQuery::Row &row, unsigned int &deleted)
{
    QString sql = QString("DELETE FROM %1.%2 WHERE ").arg(conn.getTraits().quote(Owner)).arg(conn.getTraits().quote(Table));
    std::list<QString>::iterator ite;
    int num = 0;
    toQueryParams args;

    for(ite = PriKeys.begin(); ite != PriKeys.end(); ite++)
    {
        if(num > 0)
        {
            sql += " AND ";
        }
        num++;
        sql += *ite + "=:v" + QString::number(num);
        sql += "<char[4000],in>";
        args << row[num];
    }
    qDebug() << sql;
    {
    	toConnectionSubLoan c(conn);
    	toQuery q(c, sql, args);
    	deleted += q.rowsProcessed();
    }
}
void toResultModelEdit::commitChanges(toConnection &conn, unsigned int &updated, unsigned int &added, unsigned int &deleted)
{
	toQuery::RowList::const_iterator ite;
    for(ite = Rows.constBegin(); ite != Rows.constEnd(); ite++)
    {
        //Scan the whole row and commit change when necessary
        const toQuery::Row &row = *ite;
        const toRowDesc &rowDesc = row[0].getRowDesc();
        switch(rowDesc.status)
        {
        case MODIFIED:
            commitUpdate(conn, row, updated);
            break;
        case ADDED:
            commitAdd(conn, row, added);
            break;
        case REMOVED:
            commitDelete(conn, row, deleted);
            break;
        case EXISTED:
            break;
        default:
            qDebug() << "Undefined row type " << rowDesc.status;
            break;
        }
    }
}

QList<struct toResultModelEdit::ChangeSet>& toResultModelEdit::changes()
{
	return Changes;
}

void toResultModelEdit::revertChanges()
{
	bool c = changed();
    Changes.clear();
    emit changed(changed());
}

unsigned toResultModelEdit::commitDelete(ChangeSet &change, toConnection &conn)
{
    const toResultModel::HeaderList Headers = headers();
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


unsigned toResultModelEdit::commitAdd(ChangeSet &change, toConnection &conn)
{
    const toResultModel::HeaderList Headers = headers();

    QString sql = QString("INSERT INTO %1.%2 (").arg(conn.getTraits().quote(Owner)).arg(conn.getTraits().quote(Table));

    int num = 0;
    for (int i = 1; i < change.row.size(); i++)
    {
        if (num > 0)
            sql += ",";
        sql += conn.getTraits().quote(headerData(
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

unsigned toResultModelEdit::commitUpdate(ChangeSet &change, toConnection &conn)
{
    const toResultModel::HeaderList Headers = headers();
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

        QString columnName = conn.getTraits().quote(headerData(
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

void toResultModelEdit::recordChange(const QModelIndex &index,
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

    change.columnName = headerData(index.column(),
                                            Qt::Horizontal,
                                            Qt::DisplayRole).toString();
    change.newValue = newValue;
    change.row      = row;
    change.column   = index.column();
    change.kind     = Update;

    Changes.append(change);
    emit changed(changed());
}


void toResultModelEdit::recordAdd(const toQuery::Row &row)
{
    struct ChangeSet change;

    change.row      = row;
    change.kind     = Add;

    Changes.append(change);
    emit changed(changed());
}


void toResultModelEdit::recordDelete(const toQuery::Row &row)
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
