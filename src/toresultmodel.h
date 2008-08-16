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

#ifndef TORESULTMODEL_H
#define TORESULTMODEL_H

#include "config.h"
#include "tosql.h"
#include "toresult.h"
#include "toconnection.h"
#include "tobackground.h"

#include <QObject>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QList>
#include <QMap>

class toQValue;
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

    typedef QList<toQValue> Row;
    typedef QList<Row> RowList;
    typedef QList<HeaderDesc> HeaderList;

private:
    QPointer<toEventQuery> Query;

    RowList Rows;
    HeaderList Headers;

    // max rows to read until
    int MaxNumber;

    // how much to read at a time
    int MaxRead;

    int CurrentRow;

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
    RowList mergesort(RowList&, int, Qt::SortOrder);
    RowList merge(RowList&, RowList&, int, Qt::SortOrder);


private slots:

    // destroys query, stops timer, good things.
    // emits done()
    void cleanup(void);

    void queryError(const toConnection::exception &);

public:
    toResultModel(toEventQuery *query,
                  QObject *parent = 0,
                  bool edit = false,
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
    virtual QVariant data(const QModelIndex &index, int role) const;


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
     *
     */
    virtual QVariant data(int row, int column) const;


    /**
     * Convience function to return Qt::EditRole data for the row and
     * column.
     *
     */
    virtual QVariant data(int row, QString column) const;


    /**
     * Returns the data for the given role and section in the header
     * with the specified orientation.
     *
     */
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

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
     */
    virtual bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;


    /**
     * Fetches any available data for the items with the parent
     * specified by the parent index.
     *
     */
    virtual void fetchMore(const QModelIndex &parent);


    /**
     * Returns the item flags for the given index.
     *
     */
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;


    /**
     * Set the column alignment for given column
     *
     */
    virtual void setAlignment(int col, Qt::AlignmentFlag fl);


    /**
     * Sorts the model by column in the given order.
     *
     */
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);


    /**
     * override parent to make public
     *
     */
    QModelIndex createIndex(int row, int column, void *ptr = 0) const
    {
        return QAbstractTableModel::createIndex(row, column, ptr);
    }


    /**
     * stop running query
     *
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
     * @param ind if valid, the row to duplicate
     * @returns the row added or -1
     */
    int addRow(QModelIndex ind = QModelIndex());


    /**
     * Delete a row interally. Emits rowDeleted on success.
     *
     * This is not an overriden method.
     */
    void deleteRow(QModelIndex);

signals:

    /**
     * Emitted when query is finished.
     *
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
     *
     */
    void columnChanged(const QModelIndex &index,
                       const toQValue &newValue,
                       const toResultModel::Row &row);


    /**
     * Emitted when a row is added to the model. The numbercolumn
     * passed which will not change over the life of the model.
     *
     * Note this is not emitted when query data is added.
     *
     */
    void rowAdded(const toResultModel::Row &row);


    /**
     * Emitted when a row is deleted from the model.
     *
     */
    void rowDeleted(const toResultModel::Row &row);


public slots:
    /**
     * Load all data into model until end of query
     *
     */
    void readAll(void);


    /**
     * Overloaded method. Called when query has data available. 
     *
     */
    void fetchMore(void);


protected slots:

    /**
     * reads ands sets up Rows and Columns
     *
     */
    void readData(void);

    /**
     * reads and sets up Headers
     *
     */
    void readHeaders(void);
};


#endif
