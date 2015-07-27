
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOCONNECTIONMODEL_H
#define TOCONNECTIONMODEL_H

#include "core/toconnectionoptions.h"

#include <QtCore/QAbstractTableModel>

/*! \brief Display imported/available connections in
the Import dialog's view.
\author Petr Vanek <petr@scribus.info>
*/

class toConnectionModel : public QAbstractTableModel
{
        Q_OBJECT

    public:
        toConnectionModel(QObject *parent = NULL);

        //! \brief Pull connections from QSettings
        void readConfig();
        //! \brief Set the m_data and update all connected views.
        void setupData(QMap<int, toConnectionOptions> list);
        void append(int ix, toConnectionOptions conn);
        bool removeRow(int row, const QModelIndex & parent = QModelIndex());
        //! \brief Bring m_data back to caller.
        QMap<int, toConnectionOptions> availableConnections()
        {
            return m_data;
        };
        toConnectionOptions availableConnection(int ix)
        {
            return m_data[ix];
        };

        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        int columnCount(const QModelIndex & parent = QModelIndex()) const
        {
            return 6;
        };
        int rowCount(const QModelIndex & parent = QModelIndex()) const
        {
            return m_data.count();
        };
        Qt::ItemFlags flags(const QModelIndex & index) const;

    private:
        QMap<int, toConnectionOptions> m_data;
};

#endif
