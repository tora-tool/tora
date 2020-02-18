
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

#include "core/toproviderobserver.h"
#include "core/utils.h"

toEventQueryObserverObject::toEventQueryObserverObject(Observer &o, QObject *parent)
    : QObject(parent)
    , toEventQuery::Client()
    , m_eventQuery(NULL)
    , m_observer(o)
{}

toEventQueryObserverObject::~toEventQueryObserverObject()
{
    if (m_eventQuery && m_eventQuery->parent() == this)
        delete m_eventQuery;
    m_eventQuery = NULL;
}

void toEventQueryObserverObject::setQuery(toEventQuery *query)
{
    if ( m_eventQuery != NULL && m_eventQuery->parent() == this)
    {
        delete m_eventQuery;
    }

    m_eventQuery = query;
}

toEventQuery* toEventQueryObserverObject::query()
{
    return m_eventQuery;
}

void toEventQueryObserverObject::eqDescriptionAvailable(toEventQuery *query)
{
    Q_ASSERT_X(m_eventQuery != NULL , qPrintable(__QHERE__), " phantom data");
    Q_ASSERT_X(m_eventQuery == query, qPrintable(__QHERE__), " unknown data source");

    toQColumnDescriptionList const&desc = query->describe();

    // TODO to be moved into Policy class (tomvc.h)
    toQueryAbstr::HeaderList headers;
    for (toQColumnDescriptionList::const_iterator i = desc.constBegin(); i != desc.constEnd(); i++)
    {
        struct toQueryAbstr::HeaderDesc d;

        d.name = (*i).Name;
        d.name_orig = (*i).Name;
        d.datatype = (*i).Datatype;
        d.hidden = false;
        headers << d;
    }

    m_observer.observeHeaders(headers);
}

void toEventQueryObserverObject::eqDataAvailable(toEventQuery *query)
{
    m_observer.observeData(query);
//	try
//	{
//		Q_ASSERT_X(m_eventQuery != NULL , qPrintable(__QHERE__), " phantom data");
//		Q_ASSERT_X(m_eventQuery == query, qPrintable(__QHERE__), " unknown data source");
//		Q_ASSERT_X(m_eventQuery->columnCount() >0, qPrintable(__QHERE__), " not described yet");
//
//		// TODO to be moved into Policy class (tomvc.h)
//		int columns = query->columnCount();
//		toQueryAbstr::RowList m_rows;
//		while(query->hasMore())
//		{
//			toQueryAbstr::Row row;
//			m_observer.observeRow(row);
//			for(int i=0; i < columns; i++)
//			{
//				row << query->readValue();
//			}
//			m_rows << row;
//			row.clear();
//		}
//		m_observer.observeBeginData();
//		m_observer.observeData(m_rows);
//		m_observer.observeEndData();
//	}
//	TOCATCH
}

void toEventQueryObserverObject::eqDone(toEventQuery*)
{
    m_observer.observeDone();

    bool retval = m_eventQuery->disconnect(this);
    if (retval == false)
        throw __QHERE__ + " disconnect failed";
    m_eventQuery->deleteLater();
    m_eventQuery = NULL;
}

void toEventQueryObserverObject::eqError(toEventQuery *query, const toConnection::exception &e)
{
    if (m_eventQuery && query == m_eventQuery)
    {
        m_observer.observeError(e);
		Utils::toStatusMessage(e);
	} else {
		Q_ASSERT_X(false, qPrintable(__QHERE__), "unknown data source");
	}
}

void toEventQueryObserverObject::connectionChanged(toConnection &connection)
{
    m_observer.observeConnectionChanged(connection);
}

