
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

#ifndef TOTREEMODEL_H
#define TOTREEMODEL_H

#include "core/toqvalue.h"
#include "core/toconnection.h"
#include "core/toquery.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QList>

#include <list>

/** This is base class for all table based models.
 *  QT does not allow signals to be emitted by templates.
 *  See @ref ToTableModel
 */
class toTreeModelPriv : public QAbstractItemModel
{
	Q_OBJECT;
	typedef QAbstractItemModel super;

public:

	explicit toTreeModelPriv(QObject *parent = 0);

	virtual ~toTreeModelPriv();


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

	/// This method is not public in @ref QAbstractTableModel
    void beginInsertRows(const QModelIndex &parent, int first, int last);
    /// This method is not public in @ref QAbstractTableModel
    void endInsertRows();

	///@} --- end overrides for QAbstractTableModel parent ---

	void appendRows(const toQuery::RowList &);
	void appendRow(const toQuery::Row &);
	void setHeaders(const toQuery::HeaderList &);

signals:

	void headersReceived();

	void firstResultReceived();

private:
	void cleanup();

	toQuery::RowList Rows;
    toQuery::HeaderList Headers;
};

#endif
