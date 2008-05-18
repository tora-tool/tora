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

#include "config.h"
#include "toresultmodel.h"
#include "toconf.h"
#include "utils.h"
#include "toconfiguration.h"
#include "toqvalue.h"
#include "toeventquery.h"


toResultModel::toResultModel(toEventQuery *query,
                             QObject *parent,
                             bool edit,
                             bool read)
        : QAbstractTableModel(parent)
{
    ReadableColumns = read;
    HeadersRead     = false;
    First           = true;
    Editable        = edit;
    ReadAll         = false;

    MaxRead = MaxNumber = toConfigurationSingle::Instance().maxNumber();

    CurrentRow = 0;

    Query = query;
    Query->setParent(this);

    qRegisterMetaType<toConnection::exception>("toConnection::exception");

    connect(query,
            SIGNAL(descriptionAvailable()),
            this,
            SLOT(readHeaders()));
    connect(query,
            SIGNAL(dataAvailable()),
            this,
            SLOT(fetchMore()));
    connect(query,
            SIGNAL(error(const toConnection::exception &)),
            this,
            SLOT(queryError(const toConnection::exception &)));
    connect(query,
            SIGNAL(done()),
            this,
            SLOT(fetchMore()));

    query->start();
}


toResultModel::~toResultModel()
{
    cleanup();
}


void toResultModel::cleanup()
{
    if (Query)
    {
        disconnect(Query, 0, this, 0);

        Query->stop();
        delete Query;
        emit done();
    }

    Query = 0;
}


void toResultModel::queryError(const toConnection::exception &err) {
    if(First) {
        emit firstResult(err, true);
        First = !First;
        toStatusMessage(err);
        cleanup();
    }
}


void toResultModel::readAll()
{
    ReadAll = true;
    if(Query)
        Query->readAll();
}


void toResultModel::readData()
{
    if (!Query)
    {
        cleanup();
        return;
    }

    try
    {
        // must check for errors
        Query->eof();

        int cols = Headers.size();
        if(cols < 1)
            return;

        // don't actually modify any data until we can call
        // beginInsertRows(). but to do that, we have to know how many
        // records we're going to add.
        RowList tmp;
        int     current = CurrentRow;

        while(Query->hasMore() &&
              (MaxNumber < 0 || MaxNumber > current))
        {
            Row row;

            // the number column. should never change
            row.append(toQValue(current + 1));

            for (int j = 1; (j < cols || j == 0) && Query->hasMore(); j++)
                row.append(Query->readValueNull());

            tmp.append(row);
            current++;
        }

        // if we read some data, then go ahead and insert them now.
        if (tmp.size() > 0)
        {
            beginInsertRows(QModelIndex(), CurrentRow, current - 1);
            Rows << tmp;
            CurrentRow = current;
            endInsertRows();
        }

        // not really first, but just be sure to emit before done()
        // must be emitted even if there's no data....
        if (First)
        {
            if (tmp.size() > 0 || !Query || Query->eof())
            {
                First = !First;

                // need to reset view(s) since we have to poll for data
                reset();
                emit firstResult(tr("Statement executed"), false);
            }
        }

        if (!Query)
            return;
        if (!Query->hasMore() && Query->eof())
        {
            cleanup();
            return;
        }
    }
    catch (const toConnection::exception &str)
    {
        if (First)
        {
            First = !First;
            emit firstResult(str, true);
        }
        toStatusMessage(str);
        cleanup();
        return;
    }
    catch (const QString &str)
    {
        if (First)
        {
            First = !First;
            emit firstResult(str, true);
        }
        toStatusMessage(str);
        cleanup();
        return;
    }
}


int toResultModel::addRow(QModelIndex ind)
{
    if (!Editable)
        return -1;

    CurrentRow++;
    beginInsertRows(QModelIndex(), CurrentRow, CurrentRow);

    Row row;
    if (ind.isValid())
    {
        row = Rows[ind.row()];
        row[0] = CurrentRow;
    }
    else
    {
        row.append(toQValue(CurrentRow));

        // null out the rest of the row
        int cols = Headers.size();
        for (int j = 1; j < cols; j++)
            row.append(toQValue());
    }

    Rows.append(row);
    endInsertRows();
    emit rowAdded(row);
    return CurrentRow;
}


void toResultModel::deleteRow(QModelIndex index)
{
    if (!Editable)
        return;

    if (!index.isValid() && index.row() >= Rows.size())
        return;

    beginRemoveRows(QModelIndex(), index.row(), index.row());

    Row deleted = Rows.takeAt(index.row());
    endRemoveRows();
    emit rowDeleted(deleted);
}


int toResultModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return CurrentRow;
}


/**
 * Returns the data stored under the given role for the item
 * referred to by the index.
 */
QVariant toResultModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() > Rows.size() - 1 || index.column() > Headers.size() - 1)
        return QVariant();

    toQValue data = Rows.at(index.row()).at(index.column());

    if (role == Qt::DisplayRole)
    {
        if (data.isNull())
            return toNull(data).toQVariant();

        return data.toQVariant();
    }
    if (role == Qt::EditRole)
        return data.toQVariant();
    if (role == Qt::ToolTipRole)
        return data.toQVariant();
    if (role == Qt::BackgroundRole)
    {
        if (data.isNull())
            return QVariant(QColor("#f2ffbc"));
        return QVariant();
    }
    if (role == Qt::TextAlignmentRole)
        return (int) Headers.at(index.column()).align;

    return QVariant();
}


bool toResultModel::setData(const QModelIndex &index,
                            const QVariant &_value,
                            int role)
{
    if (!Editable)
        return false;

    if (role != Qt::EditRole)
        return false;

    if (index.column() == 0)
        return false;           // can't change number column

    if (index.row() > Rows.size() - 1 || index.column() > Headers.size() - 1)
        return false;

    Row &row = Rows[index.row()];
    Row oldRow = row;           // keep old version
    toQValue newValue = toUnnull(toQValue::fromVariant(_value));

    row[index.column()] = newValue;

    // for writing to the database
    emit columnChanged(index, newValue, oldRow);

    // for the view
    emit dataChanged(index, index);
    return true;
}


QVariant toResultModel::data(int row, int column) const
{
    QModelIndex ind = index(row, column);
    return data(ind, Qt::EditRole);
}


QVariant toResultModel::data(int row, QString column) const
{
    int c;
    for (c = 0; c < Headers.size(); c++)
    {
        if (Headers[c].name == column)
            break;
    }

    if (c < Headers.size())
        return data(row, c);
    return QVariant();
}


/**
 * Returns the data for the given role and section in the header
 * with the specified orientation.
 *
 */
QVariant toResultModel::headerData(int section,
                                   Qt::Orientation orientation,
                                   int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        if (!HeadersRead)
            return QVariant();

        if (section > Headers.size() - 1)
            return QVariant();

        return Headers[section].name;
    }

    if (orientation == Qt::Vertical)
        return section;

    return QVariant();
}



void toResultModel::readHeaders()
{
    if (HeadersRead)
        return;
    if (!Query)
        return;

    // always add the number column. this makes adjusting for it in
    // the row data easier. it is not always displayed.
    struct HeaderDesc d;
    d.name     = "#";
    d.align    = Qt::AlignRight;
    d.datatype = "INT";
    Headers.append(d);

    toQDescList desc = Query->describe();
    for (toQDescList::iterator i = desc.begin(); i != desc.end(); i++)
    {
        struct HeaderDesc d;

        d.name = (*i).Name;
        if (ReadableColumns)
            toReadableColumn(d.name);

        d.datatype = (*i).Datatype;

        d.nullAllowed = (*i).Null;

        if ((*i).AlignRight)
            d.align = Qt::AlignRight | Qt::AlignVCenter;
        else
            d.align = Qt::AlignLeft | Qt::AlignVCenter;

        Headers.append(d);
    }

    HeadersRead = true;
}


/**
 * Returns the number of columns for the children of the given
 * parent. When the parent is valid it means that rowCount is
 * returning the number of children of parent.
 */
int toResultModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return Headers.size();
}


bool toResultModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    // sometimes the view calls this before the query has even
    // run.
    if (First)
        return false;

    try
    {
        return Query && Query->hasMore();
    }
    catch (...)
    {
        // will catch later
        ;
    }

    return false;
}


void toResultModel::fetchMore()
{
    if(ReadAll) {
        MaxNumber = -1;
        readData();
    }
    else if(CurrentRow < MaxNumber) {
        QModelIndex ind;
        fetchMore(ind);
    }
}


void toResultModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);

    // sometimes the view calls this before the query has even
    // run. don't actually increase max until we've hit it.
    if (MaxNumber < 0 || MaxNumber <= CurrentRow)
        MaxNumber += MaxRead;

    readData();
}


Qt::ItemFlags toResultModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags fl = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (Editable)
        fl = fl | Qt::ItemIsEditable;
    return fl;
}


void toResultModel::setAlignment(int col, Qt::AlignmentFlag fl)
{
    Headers[col].align = fl;
}


int toResultModel::partition(int left,
                             int right,
                             int pivot,
                             int column,
                             Qt::SortOrder order)
{
    Rows.move(pivot, right);        // move to end

    int store = left;
    toQValue key = Rows.at(right).at(column); // pivot value for
    // comparisons

    for (int i = left; i < right; i++)
    {
        if (order == Qt::AscendingOrder && (Rows.at(i).at(column) < key) ||
                order == Qt::DescendingOrder && (Rows.at(i).at(column) > key))
            Rows.swap(i, store++);
    }

    Rows.swap(store, right);
    return store;
}


void toResultModel::qsort(int left,
                          int right,
                          int column,
                          Qt::SortOrder order)
{
    // quick sort

    // mrj: i'm not an algorithm guy and i don't really want to do
    // this, but there's no good way to do this within qt.

    // i did a quick 'n dirty implementation following
    // http://en.wikipedia.org/wiki/Quicksort

    if (right <= left)
        return;

    int pivotIndex = left;
    int index = partition(left, right, pivotIndex, column, order);
    qsort(left, index - 1, column, order);
    qsort(index + 1, right, column, order);
}


void toResultModel::sort(int column, Qt::SortOrder order)
{
    if (column > Headers.size() - 1)
        return;

    qsort(0, Rows.size() - 1, column, order);
    emit dataChanged(createIndex(0, 0),
                     createIndex(rowCount(), columnCount()));
}
