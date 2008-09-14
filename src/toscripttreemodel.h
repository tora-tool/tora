/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#ifndef TOSCRIPTTREEMODEL_H
#define TOSCRIPTTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class toScriptTreeItem;


/*! \brief A tree model for QTreeView used in toScriptSchemaWidget.
Read Qt4 documenation to understand MVC used here.
*/
class toScriptTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    toScriptTreeModel(QObject *parent = 0);
    ~toScriptTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /*! \brief Reset the model with newly readed data from database.
    \param connId a connection string indentifier.
    \param schema it can be null. When it's given, the SQL statements
    will use WHERE clause with this string.
    */
    void setupModelData(const QString & connId, const QString & schema = 0);

private:
    //! An universal root item. It's deleted and recreated in setupModelData()
    toScriptTreeItem *rootItem;
};

#endif
