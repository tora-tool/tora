
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

void toConnectionRegistry::changeConnection(QAction *act)
{
	changeConnection(act->text());
}

void toConnectionRegistry::changeConnection(QString description)
{
	if (description.isEmpty()) // All connections were closed
		m_currentConnectionDescription = description;
	else if( m_ConnectionsMap.contains(description))
		m_currentConnectionDescription = description;
	else
		throw QString("Unregistered connection(change): %1").arg(description);
}

int toConnectionRegistry::rowCount(const QModelIndex &) const
{
	return m_ConnectionsList.size();
}

void toConnectionRegistry::addConnection(toConnection *conn)
{
	QString const& description = conn->description();
	conn->setParent(this);

	if(m_ConnectionsMap.contains(description))
	{
		delete conn;
		throw QString("Duplicit connection: %1").arg(description);
	}

	if(m_ConnectionsList.empty())
		m_currentConnectionDescription = description;

	beginInsertRows(QModelIndex(), m_ConnectionsList.size(), m_ConnectionsList.size());
	m_ConnectionsMap.insert(description, conn);
	m_ConnectionsList.append(conn);
	endInsertRows();
}

void toConnectionRegistry::removeConnection(toConnection *conn)
{
	QString const& description = conn->description();

	QList<QString> descriptions = m_ConnectionsMap.keys(conn);
	if(descriptions.size() != 1 || !m_ConnectionsList.contains(conn))
	{
		conn->setParent(this);
		throw QString("Unregistered connection: %1").arg(description);
	}
	delete conn;

	int pos = m_ConnectionsList.indexOf(conn);
	beginRemoveRows(QModelIndex(), pos, pos);
	int mRemoved = m_ConnectionsMap.remove(descriptions.at(0));
	//int lRemoved = 
	m_ConnectionsList.removeAt(pos);
	endRemoveRows();
}

QVariant toConnectionRegistry::data(const QModelIndex &idx, int role) const
{
	static QVariant nothing;
	switch(role)
	{
		case Qt::DisplayRole:
			return QVariant(m_ConnectionsList.at(idx.row())->description());
		case Qt::DecorationRole:
		{
			QString const& color = m_ConnectionsList.at(idx.row())->color();
			return QVariant(Utils::connectionColorPixmap(color));
		}
		default:
			return nothing;	
	}
}

toConnection& toConnectionRegistry::currentConnection()
{
	if( m_ConnectionsMap.contains(m_currentConnectionDescription))
		return *m_ConnectionsMap.value(m_currentConnectionDescription);
    throw tr("Can't find active connection");
}

toConnection& toConnectionRegistry::connection(const QString &str)
{
	if( m_ConnectionsMap.contains(str))
		return *m_ConnectionsMap.value(str);
    throw tr("Couldn't find specified connectionts (%1)").arg(str);
}

QList<toConnection*> const& toConnectionRegistry::connections(void) const
{
    return m_ConnectionsList;
}
