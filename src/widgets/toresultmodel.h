
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

#ifndef TORESULTMODEL_H
#define TORESULTMODEL_H

#include "core/tosql.h"
#include "core/toresult.h"
#include "core/toconnection.h"
#include "core/toqvalue.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractTableModel>
#include <QtCore/QModelIndex>
#include <QtCore/QList>
#include <QtCore/QMap>


class toEventQuery;

class toResultModel : public QAbstractTableModel
{
        Q_OBJECT;

    public:
        struct HeaderDesc
        {
            QString           name;        /* column name */
            QString           name_orig;   /* original column name */
            QString           datatype;    /* data type from queryDescribe */
            bool              nullAllowed; /* data can be null */
            Qt::Alignment     align;       /* alignment */
        };

        typedef QList<HeaderDesc> HeaderList;

        toResultModel(toEventQuery *query,
                      QObject *parent = 0,
                      bool read = false);

        /** This constructor is used when model has to be filled in
         * from the cache rather than from the database.
         */
        toResultModel(const QString &owner,
                      const QString &type,
                      QObject *parent = 0,
                      bool read = false);

        virtual ~toResultModel();

        // ------------------------------ overrides ItemModel parent

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
         * Convience function to return Qt::EditRole data for row and
         * column.
         */
        virtual QVariant data(int row, int column, int role = Qt::EditRole) const; // TODO change it to Qt::DisplayRole


        /**
         * Convience function to return Qt::EditRole data for the row and
         * column.
         */
        virtual QVariant data(int row, QString column) const;


        /**
         * Returns the data for the given role and section in the header
         * with the specified orientation.
         */
        QVariant headerData(int section,
                            Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const;

        /*! Update the header.name attribute for the horizontal header.
        This method changes data which are taken from DB (but it doesn't
        change it in the DB) so use it only if you know what are you doing.
        It's allowed for Horizontal orientation and DisplayRole only. Else
        it returns always false and it does not do anything.
        */
        bool setHeaderData(int section,
                           Qt::Orientation orientation,
                           const QVariant & value,
                           int role = Qt::EditRole) override;

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
         * Set the column alignment for given column
         */
        virtual void setAlignment(int col, Qt::AlignmentFlag fl);

        /**
         * Sorts the model by column in the given order.
         */
        virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

        /**
         * override parent to make public
         */
        QModelIndex createIndex(int row, int column, void *ptr = 0) const
        {
            return QAbstractTableModel::createIndex(row, column, ptr);
        }

        /**
         * override parent to make it traceable
         */
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const
        {
            return QAbstractTableModel::index(row, column, parent);
        }

        /**
         * stop running query
         */
        void stop(void)
        {
            cleanup();
        }

        void readAll(void);

        /**
         * Return the headers used for this query
         */
        HeaderList & headers(void) { return Headers; }
        HeaderList const& headers(void) const { return Headers; }

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
         * Returns the drop actions supported by this model.
         *
         * The default implementation returns Qt::CopyAction. Reimplement
         * this function if you wish to support additional actions. Note
         * that you must also reimplement the dropMimeData() function to
         * handle the additional operations.
         */
        virtual Qt::DropActions supportedDropActions() const;

        /** Get raw data of the data model. This is currently used to
         * prepare and send data to cache.
         */
        toQueryAbstr::RowList &getRawData(void);

        void setInitialRows(int);
    signals:

        /**
         * Emitted when query is finished.
         */
        void done(void);

        /**
         * Emitted when the first result piece is available.
         *
         * @param res String describing result.
         * @param error Error has occurred.
         */
        void firstResult(const toConnection::exception &res, bool error);

        /**
         * Emitted when the last result piece is available.
         *
         * @param res String describing result.
         * @param error Error has occurred.
         */
        void lastResult(const QString &message, bool error);

    protected slots:
        /**
         * reads and sets up Headers
         */
        void receiveHeaders(toEventQuery*);

        /**
         * Called when query has data available.
         */
        void receiveData(toEventQuery*);

        void queryError(toEventQuery*, const toConnection::exception &);

        /**
         * Called when last data piece available.
         */
        void slotFetchLast(toEventQuery*, unsigned long);

        /**
         * reads ands sets up Rows and Columns
         */
        void slotReadData(void);

        /**
         * Load all data into model until end of query
         */
        void slotReadAll(void);

    protected:
        void cleanup(void);

        // helpers for sort implementation
        toQueryAbstr::RowList mergesort(toQueryAbstr::RowList&, int, Qt::SortOrder);
        toQueryAbstr::RowList merge(toQueryAbstr::RowList&, toQueryAbstr::RowList&, int, Qt::SortOrder);

        toEventQuery *Query;

        toQueryAbstr::RowList Rows;
        HeaderList Headers;

        // Following two variables hold information on how was data last sorted by sort() function.
        // This is used by sort() function in order not to waste CPU on resorting.
        int SortedOnColumn;
        Qt::SortOrder SortedOrder;

        // max rows to read until
        int MaxRows;

        // how much to read at a time
        int MaxRowsToAdd;

        // [0] column of each row contains a row number which is later used for insert/update/delete
        // operations. This variable is used to generate non repeating row keys (something like
        // oracle sequence).
        int CurrRowKey;

        // If column names are to be made more readable.
        bool ReadableColumns;

        // when to emit firstResult
        bool First;

        // headers read already?
        bool HeadersRead;

        // should read all data
        bool ReadAll;
};


#endif
