
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

#include "core/totreemodel.h"

#include "core/toconfiguration.h"
#include "core/todatabaseconfig.h"
#include "core/toqvalue.h"
//#include "ts_log/ts_log_utils.h"

toTreeModelPriv::toTreeModelPriv(QObject *parent)
    : super(parent)
{
}

toTreeModelPriv::~toTreeModelPriv()
{
    cleanup();
}

int toTreeModelPriv::rowCount(QModelIndex const& parent) const
{
    if (parent.isValid())
        return 0;

    return Rows.size();
}

/**
 * Returns the data stored under the given role for the item
 * referred to by the index.
 */
QVariant toTreeModelPriv::data(QModelIndex const& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= Rows.size()) //
        return QVariant();

    toQValue const &data = Rows.at(index.row()).at(index.column());

    switch (role)
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
            if (data.isNull() && toConfigurationNewSingle::Instance().option(ToConfiguration::Database::IndicateEmptyBool).toBool())
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

bool toTreeModelPriv::setData( QModelIndex const& index,
                               QVariant const& value,
                               int role)
{
    throw QString("Not implemented yet: bool toTableModel::setData(...)");
}

QVariant toTreeModelPriv::headerData(int section,
                                     Qt::Orientation orientation,
                                     int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (Headers.empty())
            return QVariant();

        if (section >= Headers.size())
            return QVariant();

        switch(role)
        {
            case Qt::DisplayRole: return Headers[section].name;
            case Qt::UserRole:    return QVariant::fromValue(Headers[section]);
            default:              return QVariant();
        }
    }

    if (orientation == Qt::Vertical)
    {
        if (role == Qt::DisplayRole)
            return section + 1;
        else if (role == Qt::ForegroundRole)
        {
            if (section < 0 || section > Rows.size())
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

bool toTreeModelPriv::setHeaderData(int section,
                                    Qt::Orientation orientation,
                                    QVariant const& value,
                                    int role)
{
    if (role != Qt::DisplayRole)
        return false;

    if (orientation != Qt::Horizontal)
        return false;

    Headers[section].name = value.toString();
    Headers[section].name_orig = value.toString();
    Headers[section].hidden = false;
    return true;
}

int toTreeModelPriv::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return Headers.size();
}

bool toTreeModelPriv::canFetchMore(const QModelIndex &parent) const
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

void toTreeModelPriv::fetchMore(const QModelIndex &parent)
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

Qt::ItemFlags toTreeModelPriv::flags(QModelIndex const& index) const
{
    Qt::ItemFlags defaultFlags = super::flags(index);
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

void toTreeModelPriv::cleanup()
{

}

void toTreeModelPriv::beginInsertRows(const QModelIndex &parent, int first, int last)
{
    super::beginInsertRows(parent, first, last);
}

void toTreeModelPriv::endInsertRows()
{
    super::endInsertRows();
}

void toTreeModelPriv::appendRow(toQueryAbstr::Row const& r)
{
    int oldRowCount = rowCount();

    Rows << r;

    if (oldRowCount == 0)
        emit firstResultReceived();
}

void toTreeModelPriv::appendRows(toQueryAbstr::RowList const& r)
{
    int oldRowCount = rowCount();

    beginInsertRows(QModelIndex(), oldRowCount, oldRowCount + r.size() - 1);
    Rows << r;
    endInsertRows();

    if (oldRowCount == 0)
        emit firstResultReceived();
}

void toTreeModelPriv::setHeaders(toQueryAbstr::HeaderList const& h)
{
    if (!Headers.empty())
    {
        Q_ASSERT_X(false, qPrintable(__QHERE__), "Query already described");
    }

    beginInsertColumns(QModelIndex(), columnCount(), columnCount() + h.size() - 1);
    Headers = h;
    endInsertColumns();

    emit headersReceived();
}


