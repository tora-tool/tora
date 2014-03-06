
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
#include "core/toqvalue.h"
#include "core/toeventquery.h"
#include "core/toconnectiontraits.h"

#include <QtCore/QDebug>
#include <QtCore/QMimeData>

toResultModelEdit::toResultModelEdit(toEventQuery *query,
                             QList<QString> priKeys,
                             QObject *parent,
                             bool read)
    : toResultModel(query, parent, read)
    , PriKeys(priKeys)
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
    recordAdd(row);
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
    recordDelete(deleted);
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

    if (index.row() >= Rows.size() || index.column() >= Headers.size())
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

    {
        // If no prikey is used, data is recorded in change list
    	toQuery::Row oldRow = row;           // keep old version
        row[index.column()] = newValue;
        // for writing to the database
        recordChange(index, newValue, oldRow);

        if(!row[index.column()].updateNewValue(newValue))
            return false;
        qDebug() << "Value is changed from " << row[index.column()] << " to " << newValue << "At " << index;
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
    if (index.column() >= row.size())
        return defaultFlags;

    toQValue const &data = row.at(index.column());
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
