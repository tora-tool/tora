
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
        QString           datatype;    /* data type from queryDescribe */
        bool              nullAllowed; /* data can be null */
        Qt::Alignment     align;       /* alignment */
    };

//    typedef QList<toQValue> Row; // NOTE: first (0th) value in a row is a row description (see variable currRowKey)
//    typedef QList<Row> RowList;
    typedef QList<HeaderDesc> HeaderList;

private:
    toEventQuery *Query;

    toQuery::RowList Rows;
    HeaderList Headers;

    // Following two variables hold information on how was data last sorted by sort() function.
    // This is used by sort() function in order not to waste CPU on resorting.
    int SortedOnColumn;
    Qt::SortOrder SortedOrder;

    // max rows to read until
    int MaxNumber;

    // how much to read at a time
    int MaxRead;

    // [0] column of each row contains a row number which is later used for insert/update/delete
    // operations. This variable is used to generate non repeating row keys (something like
    // oracle sequence).
    int CurrRowKey;

    // If column names are to be made more readable.
    bool ReadableColumns;

    // when to emit firstResult
    bool First;

    // return editable flag
    bool Editable;

    // headers read already?
    bool HeadersRead;

    // should read all data
    bool ReadAll;

    // helpers for sort implementation
    toQuery::RowList mergesort(toQuery::RowList&, int, Qt::SortOrder);
    toQuery::RowList merge(toQuery::RowList&, toQuery::RowList&, int, Qt::SortOrder);

    std::list<QString> PriKeys;

    QString Owner, Table;


private slots:
    // destroys query, stops timer, good things.
    // emits done()


    void slotQueryError(toEventQuery*, const toConnection::exception &);

public:
    explicit toResultModel(toEventQuery *query,
                           std::list<QString> priKeys,
                           QObject *parent = 0,
                           bool edit = false,
                           bool read = false);

    /** This constructor is used when model has to be filled in
     * from the cache rather than from the database.
     */
    toResultModel(const QString &owner,
                  const QString &type,
                  QObject *parent = 0,
                  bool read = false);

    ~toResultModel();


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
    virtual bool setData(const QModelIndex &,
                         const QVariant &,
                         int role = Qt::EditRole);

    /**
     * Convience function to return Qt::EditRole data for row and
     * column.
     */
    virtual QVariant data(int row, int column) const;


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
    This method changes data wchich are taken from DB (but it doesn't
    change it in the DB) so use it only if you know what are you doing.
    It's allowed for Horizontal orientation and DisplayRole only. Else
    it returns always false and it does not do anything.
    */
    bool setHeaderData(int section,
                       Qt::Orientation orientation,
                       const QVariant & value,
                       int role = Qt::EditRole);

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
     * stop running query
     */
    void stop(void)
    {
        cleanup();
    }

    /**
     * Return the headers used for this query
     *
     */
    const HeaderList& headers(void) const
    {
        return Headers;
    }

    /**
     * Adds a row internally. Emits rowAdded on success.
     *
     * This isn't part of any parent api. Qt provides insertRow which
     * needs a row number and index. This is provided to simply append
     * a new row.
     *
     * @param ind index of a selected cell when this action was called
     * @param duplicate - should the value of current row be copied/duplicated
     * @return added row
     */
    int addRow(QModelIndex ind = QModelIndex(), bool duplicate = false);

    /**
     * Mark to delete a row interally. Emits rowDeleted on success.
     *
     * This is not an overridden method.
     */
    void deleteRow(QModelIndex);

    /**
     * Clear the status of records.
     */
    void clearStatus();

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

    /** Get raw data of the data model. This is currently used to
     * prepare and send data to cache.
     */
    toQuery::RowList &getRawData(void);

    /**
     *  Get PriKeys
     */
    const std::list<QString> &getPriKeys()
    {
        return PriKeys;
    }

    void setOwner(const QString &owner)
    {
        this->Owner = owner;
    }
    void setTable(const QString &table)
    {
        this->Table = table;
    }
    /**
     * Update data
     */
    void commitChanges(toConnection &conn, unsigned int &updated, unsigned int &added, unsigned int &deleted);

protected:
    void cleanup(void);

    void commitUpdate(toConnection &conn, const toQuery::Row &row, unsigned int &updated);
    void commitAdd(toConnection &conn, const toQuery::Row &row, unsigned int &added);
    void commitDelete(toConnection &conn, const toQuery::Row &row, unsigned int &deleted);

public:

signals:

    /**
     * Emitted when query is finished.
     */
    void done(void);


    /**
     * Emitted when the first result is available.
     *
     * @param res String describing result.
     * @param error Error has occurred.
     */
    void firstResult(const toConnection::exception &res,
                     bool error);


    /**
     * Emitted when column data in the view changes.
     */
    void columnChanged(const QModelIndex &index,
                       const toQValue &newValue,
                       const toQuery::Row &row);


    /**
     * Emitted when a row is added to the model. The numbercolumn
     * passed which will not change over the life of the model.
     *
     * Note this is not emitted when query data is added.
     *
     */
    void rowAdded(const toQuery::Row &row);


    /**
     * Emitted when a row is deleted from the model.
     */
    void rowDeleted(const toQuery::Row &row);


private slots:
    /**
     * Load all data into model until end of query
     */
    void slotReadAll(void);

public:
    void readAll(void);

public slots:
    /**
     * Overloaded method. Called when query has data available.
     */
    void slotFetchMore(toEventQuery*);


protected slots:

    /**
     * reads ands sets up Rows and Columns
     */
    void slotReadData(void);

    /**
     * reads and sets up Headers
     */
    void slotReadHeaders(toEventQuery*);
};


#endif
