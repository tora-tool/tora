/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOCONNECTIONMODEL_H
#define TOCONNECTIONMODEL_H

#include <QAbstractTableModel>
#include "tonewconnection.h"


/*! \brief Display imported/available connections in
the Import dialog's view.
\author Petr Vanek <petr@scribus.info>
*/

class toConnectionModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        toConnectionModel();

        //! \brief Set the m_data and update all connected views.
        void setupData(QMap<int,toConnectionOptions> list);
        void append(int ix, toConnectionOptions conn);
//         bool removeRow(int row, const QModelIndex & parent = QModelIndex());
        //! \brief Bring m_data back to caller.
        QMap<int,toConnectionOptions> availableConnections() { return m_data; };
        toConnectionOptions availableConnection(int ix) { return m_data[ix]; };

        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        int columnCount(const QModelIndex & parent = QModelIndex()) const { return 6; };
        int rowCount(const QModelIndex & parent = QModelIndex()) const { return m_data.count(); };
        Qt::ItemFlags flags(const QModelIndex & index) const;

    private:
        QMap<int,toConnectionOptions> m_data;
};

#endif
