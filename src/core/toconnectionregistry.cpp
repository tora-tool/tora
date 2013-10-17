
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

#include "core/toconnectionregistry.h"
#include "core/toconnection.h"
#include "core/utils.h"
#include "ts_log/ts_log_utils.h"

#include <QtGui/QAction>

toConnectionRegistry::~toConnectionRegistry()
{
	Utils::toBusy::m_enabled = false;
	int size = m_ConnectionsList.size();
}

bool toConnectionRegistry::isEmpty() const
{
	return m_ConnectionsList.empty();
}

//void toConnectionRegistry::changeConnection(const toConnectionOptions &opts)
//{
//	if( m_ConnectionsMap.contains(opts))
//		m_currentConnection = opts;
//	else
//		throw QString("Unregistered connection(change): %1").arg(opts.toString());
//}

int toConnectionRegistry::rowCount(const QModelIndex &) const
{
	return m_ConnectionsList.size();
}

void toConnectionRegistry::addConnection(toConnection *conn)
{
	conn->setParent(this);

	if(m_ConnectionsMap.contains(conn->connectionOptions()))
	{
		QString opsStr = conn->connectionOptions().toString();
		delete conn;
		throw QString("Duplicit connection: %1").arg(opsStr);
	}

	//	if(m_ConnectionsList.empty())
	//		m_currentConnection = conn->connectionOptions();

	beginInsertRows(QModelIndex(), m_ConnectionsList.size(), m_ConnectionsList.size());
	m_ConnectionsMap.insert(conn->connectionOptions(), conn);
	m_ConnectionsList.append(conn);
	m_currentConnection = index(m_ConnectionsList.size()-1, 0);
	endInsertRows();
}

void toConnectionRegistry::removeConnection(toConnection *conn)
{
//	QString const& description = conn->description();

	QList<toConnectionOptions> conns = m_ConnectionsMap.keys(conn);
	if(conns.size() != 1 || !m_ConnectionsList.contains(conn))
	{
		conn->setParent(this);
		throw QString("Unregistered connection for removal: %1").arg(conn->description());
	}

	int pos = m_ConnectionsList.indexOf(conn);

	// TODO if TCP connection is lost this can hang "forever" - preventing application exit
	// There must be some ugly way of doing this asynchronously in bg thread:
	// conn->setParent(NULL); conn->setThread(<something>);
	// emit something and then wait for threads response
	delete conn;

	beginRemoveRows(QModelIndex(), pos, pos);
	int mRemoved = m_ConnectionsMap.remove(conns.at(0));
	m_ConnectionsList.removeAt(pos);
	m_currentConnection = index(m_ConnectionsList.size()-1, 0);
	endRemoveRows();
}

QVariant toConnectionRegistry::data(const QModelIndex &idx, int role) const
{
	static QVariant nothing;

	Q_ASSERT_X(idx.row() < m_ConnectionsList.size(), qPrintable(__QHERE__), "index out of range");

	switch(role)
	{
		case Qt::DisplayRole:
			return QVariant(m_ConnectionsList.at(idx.row())->description());
		case Qt::DecorationRole:
		{
			QString const& color = m_ConnectionsList.at(idx.row())->color();
			return QVariant(Utils::connectionColorPixmap(color));
		}
		case Qt::UserRole:
			return QVariant::fromValue(m_ConnectionsList.at(idx.row())->connectionOptions());
		default:
			return nothing;	
	}
}

toConnection& toConnectionRegistry::currentConnection()
{
	if (m_ConnectionsList.empty())
		throw tr("Can't find active connection");
	return *m_ConnectionsList.at(m_currentConnection.row());
}

toConnection& toConnectionRegistry::connection(const toConnectionOptions &opt)
{
	if( m_ConnectionsMap.contains(opt))
		return *m_ConnectionsMap.value(opt);
	throw tr("Couldn't find specified connectionts (%1)").arg(opt.toString());
}

void toConnectionRegistry::slotViewIndexChanged(int idx)
{
	if (idx == -1)
	{
		m_currentConnection = QModelIndex();
		return; // No connection
	}
	Q_ASSERT_X(idx < m_ConnectionsList.size(), qPrintable(__QHERE__), qPrintable(QString("Connection index out of range: %1").arg(idx)));
	QModelIndex i = index(idx, 0);
	QVariant val = data(i, Qt::UserRole);
	if (m_ConnectionsMap.contains(val.value<toConnectionOptions>()))
		m_currentConnection = i;
	else
		throw tr("Can't find active connection");
}

QList<toConnection*> const& toConnectionRegistry::connections(void) const
{
    return m_ConnectionsList;
}

