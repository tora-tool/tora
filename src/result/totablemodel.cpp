
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

#include "result/totablemodel.h"

#include "core/toconfiguration_new.h"
#include "core/todatabasesetting.h"
#include "core/toqvalue.h"
#include "ts_log/ts_log_utils.h"

#include <QtCore/QMimeData>

toTableModelPriv::toTableModelPriv(QObject *parent)
    : QAbstractTableModel(parent)
	, SortedOnColumn(-1)
	, SortOrder(Qt::AscendingOrder)
{
}

toTableModelPriv::~toTableModelPriv()
{
    cleanup();
}

int toTableModelPriv::rowCount(QModelIndex const& parent) const
{
    if (parent.isValid())
        return 0;

    return Rows.size();
}

/**
 * Returns the data stored under the given role for the item
 * referred to by the index.
 */
QVariant toTableModelPriv::data(QModelIndex const& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= Rows.size()) //
        return QVariant();

	int r = index.row();
	int c = index.column();
    toQValue const &data = Rows.at(index.row()).at(index.column());

    switch(role)
    {
    case Qt::ToolTipRole:
        if (data.isNull())
            return data.toQVariant();
        if (data.isComplexType())
        {
            toQValue::complexType *i = data.toQVariant().value<toQValue::complexType*>();
            return QVariant(i->tooltipData());
        }
        return data.toQVariant();
    case Qt::EditRole:
        if (data.isComplexType())
        {
            toQValue::complexType *i = data.toQVariant().value<toQValue::complexType*>();
            return QVariant(i->editData());
        }
        return QVariant(data.editData());
    case Qt::DisplayRole:
        if (data.isComplexType())
        {
            toQValue::complexType *i = data.toQVariant().value<toQValue::complexType*>();
            return QVariant(i->displayData());
        }
        return QVariant(data.displayData());
    case Qt::BackgroundRole:
        if (data.isNull() && toConfigurationNewSingle::Instance().option(ToConfiguration::Database::IndicateEmpty).toBool())
            return QVariant(QColor(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::IndicateEmptyColor).toString()));
        return QVariant();
//    case Qt::TextAlignmentRole:
//        return (int) Headers.at(index.column()).align;
    case Qt::UserRole:
        return data.toQVariant();
// TODO Heran's patch
//    case Qt::FontRole:
//        if (rowDesc.status == REMOVED)
//            fontRet.setStrikeOut(true);
//        else if (rowDesc.status == ADDED)
//            fontRet.setBold(true);
//        else if (rowDesc.status == MODIFIED)
//            fontRet.setItalic(true);
//        return fontRet;
    default:
        return QVariant();
    }
    return QVariant();
}

bool toTableModelPriv::setData( QModelIndex const& index,
                            QVariant const& value,
                            int role)
{
	throw QString("Not implemented yet: bool toTableModel::setData(...)");
}

QVariant toTableModelPriv::headerData(int section,
                                   Qt::Orientation orientation,
                                   int role) const
{
    /*if (role != Qt::DisplayRole)
        return QVariant();*/

    if (orientation == Qt::Horizontal)
    {
        if (Headers.empty())
            return QVariant();

        if (section >= Headers.size())
            return QVariant();

        if (role == Qt::DisplayRole)
            return Headers[section].name;
        else
            return QVariant();
    }

    if (orientation == Qt::Vertical)
    {
        if (role == Qt::DisplayRole)
            return section + 1;
        else if (role == Qt::ForegroundRole)
        {
            if(section < 0 || section > Rows.size())
                return QVariant();
// TODO
//            toRowDesc rowDesc = Rows[section][0].getRowDesc();
//            switch (rowDesc.status)
//            {
//            case REMOVED:
//                return QBrush(Qt::red);
//            case ADDED:
//                return QBrush(Qt::green);
//            case MODIFIED:
                return QBrush(Qt::blue);
//            case EXISTED:
//            default:
//                return QVariant();
//            }
        }
    }

    return QVariant();
}

bool toTableModelPriv::setHeaderData(int section,
                                  Qt::Orientation orientation,
                                  QVariant const& value,
                                  int role)
{
    if (role != Qt::DisplayRole)
        return false;

    if (orientation != Qt::Horizontal)
        return false;

    Headers[section].name = value.toString();
    return true;
}

int toTableModelPriv::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return Headers.size();
}

bool toTableModelPriv::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    // sometimes the view calls this before the query has even
    // run.
// TODO
//    if (First)
//        return false;
//
//    try
//    {
//        return Query && Query->hasMore();
//    }
//    catch (...)
//    {
//        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
//        // will catch later
//        ;
//    }

    return false;
}

void toTableModelPriv::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);

    // sometimes the view calls this before the query has even
    // run. don't actually increase max until we've hit it.
// TODO
//    if (MaxNumber < 0 || MaxNumber <= Rows.size())
//        MaxNumber += MaxRead;
//
//    slotReadData();
}

Qt::ItemFlags toTableModelPriv::flags(QModelIndex const& index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    Qt::ItemFlags fl = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;

    if (index.column() == 0)
        return fl;              // row number column

    if (!index.isValid() || index.row() >= Rows.size())
    {
// TODO
//        if(Editable)
//            return Qt::ItemIsDropEnabled | defaultFlags;
//        else
            return defaultFlags;
    }

    toQValue const &data = Rows.at(index.row()).at(index.column());
    // TODO
    // toRowDesc rowDesc = Rows.at(index.row()).at(0).getRowDesc();
    if (data.isComplexType())
    {
        return ( defaultFlags | fl ) & ~Qt::ItemIsEditable;
    }

// TODO
//    if (Editable)
//        fl |= defaultFlags | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
//    else
        fl |= defaultFlags;

    //Check the status of current record
// TODO Heran's patch
//    if (rowDesc.status == REMOVED)
//        fl &= ~Qt::ItemIsEditable;
    return fl;
}

void toTableModelPriv::sort(int column, Qt::SortOrder order)
{
    if (column > Headers.size() - 1)
        return;

    // Do nothing if data was already sorted in the requested way
    if (SortedOnColumn == column && SortOrder == order)
        return;

    Rows = mergesort(Rows, column, order);
    SortedOnColumn = column;
    SortOrder = order;
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
}

QStringList toTableModelPriv::mimeTypes() const
{
    QStringList types;
    types << "text/plain";
    types << "application/vnd.tomodel.list";
    return types;
}

QMimeData* toTableModelPriv::mimeData(const QModelIndexList &indexes) const
{
    QMimeData   *mimeData = new QMimeData();
    QByteArray   encodedData;
    QDataStream  stream(&encodedData, QIODevice::WriteOnly);
    QByteArray   stringData;
    QDataStream  ss(&stringData, QIODevice::WriteOnly);

    int         valid = 0;
    QModelIndex validIndex;
    QString     text;

    // qt sends list by column (all indexes from column 1, then column
    // column2). Need to figure out the number of columns and
    // rows. It'd be awesome if we could get the selection but there's
    // no good way to do that from the model.
    int rows       = 0;
    int columns    = 0;
    int currentRow = -1;
    int currentCol = -1;

    foreach (QModelIndex index, indexes)
    {
        if (index.isValid())
        {
            if(index.row() > currentRow)
            {
                currentRow = index.row();
                rows++;
            }

            if(currentCol != index.column())
            {
                currentCol = index.column();
                columns++;
            }

            valid++;
            validIndex = index;
            text = data(index, Qt::DisplayRole).toString();
            ss << text;
        }
    }

    if(valid < 1)
        return 0;

    if(valid == 1)
    {
        mimeData->setText(text);

        // set row and column in data so we can try to preserve the
        // columns, if needed

        QByteArray sourceData;
        QDataStream sourceStream(&sourceData, QIODevice::WriteOnly);
        sourceStream << validIndex.row();
        sourceStream << validIndex.column();
        mimeData->setData("application/vnd.int.list", sourceData);
    }
    else
    {
        // serialize selection shape
        stream << rows;
        stream << columns;
        mimeData->setData("application/vnd.tomodel.list", encodedData + stringData);
    }

    return mimeData;
}

bool toTableModelPriv::dropMimeData(const QMimeData *data,
                                 Qt::DropAction action,
                                 int row,
                                 int column,
                                 const QModelIndex &parent)
{
    throw QString("Not implemented yet: bool toTableModel::dropMimeData(...)");
}

Qt::DropActions toTableModelPriv::supportedDropActions() const
{
// TODO
//    if(Editable)
//        return Qt::CopyAction | Qt::MoveAction;
    return Qt::IgnoreAction;
}

void toTableModelPriv::cleanup()
{

}

void toTableModelPriv::beginInsertRows(const QModelIndex &parent, int first, int last)
{
	super::beginInsertRows(parent, first, last);
}

void toTableModelPriv::endInsertRows()
{
	super::endInsertRows();
}

void toTableModelPriv::appendRow(toQuery::Row const& r)
{
	int oldRowCount = rowCount();

	Rows << r;

	if (oldRowCount == 0)
		emit firstResultReceived();
}

void toTableModelPriv::appendRows(toQuery::RowList const& r)
{
	int oldRowCount = rowCount();

	beginInsertRows(QModelIndex(), oldRowCount, oldRowCount + r.size() - 1); 
	Rows << r;
	endInsertRows();

	if (oldRowCount == 0)
		emit firstResultReceived();
}

void toTableModelPriv::setHeaders(toQuery::HeaderList const& h)
{
	if(!Headers.empty())
	{
		Q_ASSERT_X(false, qPrintable(__QHERE__), "Query already described");
	}

	beginInsertColumns(QModelIndex(), columnCount(), columnCount() + h.size() - 1);
	Headers = h;
	endInsertColumns();

	emit headersReceived();
}

toQuery::RowList toTableModelPriv::mergesort(toQuery::RowList &rows,
        int column,
        Qt::SortOrder order)
{
    if(rows.size() <= 1)
        return rows;

    toQuery::RowList left, right;

    int middle = (int) (rows.size() / 2);
    left = rows.mid(0, middle);
    right = rows.mid(middle);

    left = mergesort(left, column, order);
    right = mergesort(right, column, order);

    return merge(left, right, column, order);
}


toQuery::RowList toTableModelPriv::merge(toQuery::RowList &left,
        toQuery::RowList &right,
        int column,
        Qt::SortOrder order)
{
    toQuery::RowList result;

    while(left.size() > 0 && right.size() > 0)
    {
        if((order == Qt::AscendingOrder && left.at(0).at(column) <= right.at(0).at(column)) ||
                (order == Qt::DescendingOrder && left.at(0).at(column) >= right.at(0).at(column)))
            result.append(left.takeAt(0));
        else
            result.append(right.takeAt(0));
    }

    if(left.size() > 0)
        result << left;
    if(right.size() > 0)
        result << right;
    return result;
}

