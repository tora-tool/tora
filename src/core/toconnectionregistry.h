
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

#ifndef TOCONNECTION_REGISTRY_H
#define TOCONNECTION_REGISTRY_H

#include "core/tora_export.h"
#include "core/toconnectionoptions.h"
#include "loki/Singleton.h"

#include <QtCore/QAbstractListModel>

class toConnection;
class QAction;

/** The connections that have been opened in TOra. */
class TORA_EXPORT toConnectionRegistry : public QAbstractListModel
{
        Q_OBJECT;
    public:
        toConnectionRegistry() {};
        ~toConnectionRegistry();

        bool isEmpty() const;

        /** Change current connection
         */
        // see void toChangeConnection::changeConnection(QAction *act)
        void changeConnection(toConnection &);

        /**
         * Get a list of currently open connections.
         * @return List of connections. The returned list can then be used by
         */
        QList<toConnection*> const& connections(void) const;

        /** Get a connection identified by a string.
         * @return A reference to a connection.
         * @exception QString If connection wasn't found.
         */
        toConnection& connection(const toConnectionOptions &);

        /**
         * Get the current database connection
         * @return Reference to current connection.
         */
        toConnection& currentConnection(void);

        void removeConnection(toConnection*);

        void addConnection(toConnection*);

        // QAbstractListModel interface
        int rowCount(const QModelIndex &) const;
        QVariant data(const QModelIndex &,int) const;

    signals:
        void activeConnectionChanged(QModelIndex);
        void activeConnectionChanged(int);

    private slots:
        void slotViewIndexChanged(int);

    private:
        QModelIndex m_currentConnection;
        QMap<toConnectionOptions, toConnection *> m_ConnectionsMap;
        QList<toConnection *> m_ConnectionsList;
};

typedef Loki::SingletonHolder<toConnectionRegistry> toConnectionRegistrySing;

#endif
