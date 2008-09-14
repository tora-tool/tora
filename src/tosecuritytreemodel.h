
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#ifndef TOSECURITYTREEMODEL_H
#define TOSECURITYTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include "tosecuritytreeitem.h"


/*! \brief A tree model for QTreeView used in toSecurity, toSecurityObject
Read Qt4 documenation to understand MVC used here.
*/
class toSecurityTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    toSecurityTreeModel(QObject *parent = 0);
    ~toSecurityTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /*! \brief Reset the model with newly readed data from database.
    \param user a user name. It will use WHERE clause with this string.
    The tree is built with (un)checkable items. Only the "grants" related
    items are set as checkable. These chanckable items with set isChanged()
    to true are used for SQL creation in the sql() method.
    */
    void setupModelData(const QString & user);

    /*! \brief Construct GRANT/REVOKE script based on user's changes.
    */
    void sql(const QString &user, std::list<QString> &sqlLst);

private:
    //! An universal root item. It's deleted and recreated in setupModelData()
    toSecurityTreeItem *rootItem;

    //! Mapping of the OBJECT TYPE - its available privileges
    QMap<QString,QStringList> m_SQLObjectPrivs;
    //! A list with privilege items (only th 3rd level is stored there.
    QList<toSecurityTreeItem*> m_objectList;
};

#endif
