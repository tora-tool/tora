/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */
#ifndef TOCODEMODEL_H
#define TOCODEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QTreeWidgetItem>

#include "config.h"
#include "toconnection.h"

class toEventQuery;


/*! \brief A tree model for QTreeView used in the sql editor and sql
 *  debugger tools.
 *
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

private slots:
    void cleanup(void);
    void readData(void);
    void queryError(const toConnection::exception &);

private:
    //! An universal root item. It's deleted and recreated in setupModelData()
    QTreeWidgetItem *rootItem;
    QTreeWidgetItem *packageItem;
    QTreeWidgetItem *procItem;
    QTreeWidgetItem *typeItem;
    toEventQuery    *query;
};

#endif
