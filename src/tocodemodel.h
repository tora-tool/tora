
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

#ifndef TOCODEMODEL_H
#define TOCODEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "config.h"
#include "toconnection.h"

class toEventQuery;


/*! \brief A leaf item for toCodeModel.
It displays one item in the tree structure of the code model.
*/
class toCodeModelItem
{
public:
    /*!
    \param parent a parent item. When it's 0, it's a root one.
    \param display a text to display - item caption or object name
    \param type item type. "NULL" for headers and non-db items.
                and e.g. "PROCEDURE" for procedures. See its toSQL.
    \param status a (in)valid state of the DB object. Use "VALID" for
                  non-db items.
    */
    toCodeModelItem(
             toCodeModelItem *parent = 0,
             const QString & display = 0,
             const QString & type = "NULL",
             const QString & status = "VALID"
             );
    ~toCodeModelItem();

    /*! \brief It appends a child to the tree for this item.
    \warning Do not call this method if you provide
    a parent in constructor!
    */
    void appendChild(toCodeModelItem *child);

    toCodeModelItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    toCodeModelItem *parent();

    //! Object name
    QString display() const;
    //! Object type
    QString type() const;
    //! Object status (validity)
    QString status() const;
    void setStatus(const QString & s);

private:
    QList<toCodeModelItem*> childItems;
    QString itemDisplay;
    QString itemType;
    QString itemStatus;
    toCodeModelItem *parentItem;
};



/*! \brief A tree model for QTreeView used in the sql editor and sql
 *  debugger tools and toBrowser as well.
 */
class toCodeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    toCodeModel(QObject *parent = 0);
    ~toCodeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * Run query for model
     *
     */
    void refresh(toConnection &conn, const QString &owner);

public slots:
    void addChildContent(const QModelIndex & index);

signals:
    void dataReady();

private slots:
    void cleanup(void);
    void readData(void);
    void queryError(const toConnection::exception &);

private:
    //! An universal root item. It's deleted and recreated in setupModelData()
    toCodeModelItem *rootItem;
    toCodeModelItem *packageItem;
    toCodeModelItem *procItem;
    toCodeModelItem *funcItem;
    toCodeModelItem *typeItem;
    toEventQuery    *query;
    QString m_owner;
};

#endif
