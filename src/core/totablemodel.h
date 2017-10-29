
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

#pragma once

#include "core/toqvalue.h"
#include "core/toconnection.h"
#include "core/toquery.h"

#include <QtCore/QAbstractTableModel>
#include <QtCore/QList>

/** This is base class for all table based models.
 *  QT does not allow signals to be emitted by templates.
 *  See @ref ToTableModel
 */
class toTableModelPriv : public QAbstractTableModel
{
        Q_OBJECT;
        typedef QAbstractTableModel super;

    public:

        explicit toTableModelPriv(QObject *parent = 0);

        virtual ~toTableModelPriv();


        ///@{ --- begin overrides for @name QAbstractTableModel parent ---
        /**
         * Returns the number of rows under the given parent. When the
         * parent is valid it means that rowCount is returning the
         * number of children of parent.
         *
         * Tip: When implementing a table based model, rowCount()
         * should return 0 when the parent is valid.
         */
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

        /**
         * Returns the data stored under the given role for the item
         * referred to by the index.
         */
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        /**
         * Sets the role data for the item at index to value. Returns true
         * if successful; otherwise returns false.
         *
         * The dataChanged() signal should be emitted if the data was
         * successfully set.
         *
         * The base class implementation returns false. This function and
         * data() must be reimplemented for editable models. Note that the
         * dataChanged() signal must be emitted explicitly when
         * reimplementing this function.
         *
         */
        virtual bool setData(const QModelIndex &, const QVariant &, int role = Qt::EditRole);

        /**
         * Returns the data for the given role and section in the header
         * with the specified orientation.
         */
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

        /*! Update the header.name attribute for the horizontal header.
          This method changes data wchich are taken from DB (but it doesn't
          change it in the DB) so use it only if you know what are you doing.
          It's allowed for Horizontal orientation and DisplayRole only. Else
          it returns always false and it does not do anything.
        */
        bool setHeaderData(int section, Qt::Orientation orientation, const QVariant & value, int role = Qt::EditRole);

        /**
         * Returns the number of columns for the children of the given
         * parent. When the parent is valid it means that rowCount is
         * returning the number of children of parent.
         */
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

        /**
         * Returns true if there is more data available for parent,
         * otherwise false.
         *
         * overrides QAbstractTableModel::canFetchMore
         * not intended for toResultModel subclasses
         */
        virtual bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;

        /**
         * Fetches any available data for the items with the parent
         * specified by the parent index.
         *
         *  overrides QAbstractTableModel::fetchMore
         *  not intended for toResultModel subclasses
         */
        virtual void fetchMore(const QModelIndex &parent);

        /**
         * Returns the item flags for the given index.
         */
        virtual Qt::ItemFlags flags(const QModelIndex &index) const;

        /**
         * Sorts the model by column in the given order.
         */
        virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

        /**
         * Returns a list of MIME types that can be used to describe a
         * list of model indexes.
         */
        virtual QStringList mimeTypes() const;

        /**
         * Returns an object that contains serialized items of data
         * corresponding to the list of indexes specified. The formats
         * used to describe the encoded data is obtained from the
         * mimeTypes() function.
         *
         * If the list of indexes is empty, or there are no supported MIME
         * types, 0 is returned rather than a serialized empty list.
         */
        virtual QMimeData* mimeData(const QModelIndexList &indexes) const;

        /**
         * Handles the data supplied by a drag and drop operation that
         * ended with the given action. Returns true if the data and
         * action can be handled by the model; otherwise returns false.
         *
         * Although the specified row, column and parent indicate the
         * location of an item in the model where the operation ended, it
         * is the responsibility of the view to provide a suitable
         * location for where the data should be inserted.
         *
         * For instance, a drop action on an item in a QTreeView can
         * result in new items either being inserted as children of the
         * item specified by row, column, and parent, or as siblings of
         * the item.
         *
         * When row and column are -1 it means that it is up to the model
         * to decide where to place the data. This can occur in a tree
         * when data is dropped on a parent. Models will usually append
         * the data to the parent in this case.
         *
         * Returns true if the dropping was successful otherwise false.
         */
        virtual bool dropMimeData(const QMimeData *data,
                                  Qt::DropAction action,
                                  int row,
                                  int column,
                                  const QModelIndex &parent);

        /**
         * Returns the drop actions supported by this model.
         *
         * The default implementation returns Qt::CopyAction. Reimplement
         * this function if you wish to support additional actions. Note
         * that you must also reimplement the dropMimeData() function to
         * handle the additional operations.
         */
        virtual Qt::DropActions supportedDropActions() const;

        /// This method is not public in @ref QAbstractTableModel
        void beginInsertRows(const QModelIndex &parent, int first, int last);
        /// This method is not public in @ref QAbstractTableModel
        void endInsertRows();

        ///@} --- end overrides for QAbstractTableModel parent ---

        void appendRows(const toQueryAbstr::RowList &);
        void appendRow(const toQueryAbstr::Row &);
        void setHeaders(const toQueryAbstr::HeaderList &);

    protected:
        void clearAll();

    signals:

        void headersReceived();

        void firstResultReceived();

    private:

        // helpers for sort implementation
        toQueryAbstr::RowList mergesort(toQueryAbstr::RowList&, int, Qt::SortOrder);
        toQueryAbstr::RowList merge(toQueryAbstr::RowList&, toQueryAbstr::RowList&, int, Qt::SortOrder);

        toQueryAbstr::RowList Rows;
        toQueryAbstr::HeaderList Headers;

        // Following two variables hold information on how was data last sorted by sort() function.
        // This is used by sort() function in order not to waste CPU on resorting.
        int SortedOnColumn;
        Qt::SortOrder SortOrder;
};
