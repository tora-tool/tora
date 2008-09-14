/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
