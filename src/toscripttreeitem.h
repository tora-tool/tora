/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSCRIPTTREEITEM_H
#define TOSCRIPTTREEITEM_H

#include <QList>
#include <QVariant>


/*! \brief A leaf item for toScriptTreeModel.
It contains required data for DB objects re-creation.
Read Qt4 documentation to understand undocumented
methods in this class.
*/
class toScriptTreeItem
{
public:
    /*!
    \param parent a parent item. When it's 0, it's a root one.
    \param data main data - object name usually.
    \param type object type. See SQL statements for its content.
    \param schema a schema/owner name. See SQL statements for its content.
    */
    toScriptTreeItem(
             toScriptTreeItem *parent = 0,
             const QString & data = 0,
             const QString & type = 0,
             const QString & schema = 0
             );
    ~toScriptTreeItem();

    void appendChild(toScriptTreeItem *child);

    toScriptTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    int row() const;
    toScriptTreeItem *parent();

    //! Object name
    QString data() const;
    //! Object type
    QString type() const;
    //! Object owner/its schema name
    QString schema() const;

private:
    QList<toScriptTreeItem*> childItems;
    QString itemData;
    QString itemType;
    QString itemSchema;
    toScriptTreeItem *parentItem;
};

#endif
