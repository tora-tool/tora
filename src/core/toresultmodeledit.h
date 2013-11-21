
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

#ifndef TORESULTMODEL_EDIT_H
#define TORESULTMODEL_EDIT_H

#include "core/toresultmodel.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractTableModel>
#include <QtCore/QModelIndex>
#include <QtCore/QList>
#include <QtCore/QMap>


class toEventQuery;

class toResultModelEdit : public toResultModel
{
    Q_OBJECT;

public:
    toResultModelEdit(toEventQuery *query,
    		std::list<QString> priKeys,
    		QObject *parent = 0,
    		bool read = false);

    ~toResultModelEdit();

    /**
     * Returns the item flags for the given index.
     */
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

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

    void commitUpdate(toConnection &conn, const toQuery::Row &row, unsigned int &updated);
    void commitAdd(toConnection &conn, const toQuery::Row &row, unsigned int &added);
    void commitDelete(toConnection &conn, const toQuery::Row &row, unsigned int &deleted);

signals:

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

private:
    //std::list<QString> PriKeys;
    QString Owner, Table;
};

#endif
