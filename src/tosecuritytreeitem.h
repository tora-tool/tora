/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
