
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

#ifndef TOSECURITYTREEITEM_H
#define TOSECURITYTREEITEM_H

#include <QList>
#include <QVariant>


/*! \brief A leaf item for toSecurity, toSecurityObject.
DB object GRANTs are handled in this model.
Read Qt4 documentation to understand undocumented
methods in this class.
*/
class toSecurityTreeItem
{
public:
    /*!
    \param parent a parent item. When it's 0, it's a root one.
    \param data main data - object name usually.
    \param type object type. See SQL statements for its content.
    \param schema a schema/owner name. See SQL statements for its content.
    */
    toSecurityTreeItem(
             toSecurityTreeItem *parent = 0,
             const QString & data = 0,
             const QString & name = 0,
             const QString & type = 0,
             const QString & schema = 0
             );
    ~toSecurityTreeItem();

    void appendChild(toSecurityTreeItem *child);

    toSecurityTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    toSecurityTreeItem *parent();

    //! Object display string
    QString data() const;
    //! Object name
    QString name() const;
    //! Object type
    QString type() const;
    //! Object owner/its schema name
    QString schema() const;

    //! "Can be checked?" property handlign
    bool isCheckable() { return m_checkable; };
    void setCheckable(bool state) { m_checkable = state; };
    //! Handle its check state
    Qt::CheckState state() { return m_checked; };
    void setState(Qt::CheckState state) { m_checked = state; };
    //! "Is it modified by user?" property.
    bool changed() { return m_changed; };
    void setChanged(bool v) { m_changed = v; };

private:
    QList<toSecurityTreeItem*> childItems;
    QString itemData;
    QString itemName;
    QString itemType;
    QString itemSchema;
    toSecurityTreeItem *parentItem;

    bool m_changed;
    bool m_checkable;
    Qt::CheckState m_checked;
};

#endif
