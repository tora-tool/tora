
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

#ifndef __TOMVC_H__
#define __TOMVC_H__

#include "core/toresult.h"
#include "core/toeditwidget.h"
#include "core/toeventquery.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "core/utils.h"

#include "result/toproviderobserver.h"
#include "result/totablemodel.h"
#include "result/totableview.h"

#include <QtCore/QObject>
#include <QHeaderView>

/** toTableView traits */
struct MVCTraits
{
	typedef toTableModelPriv   Model;
	typedef toTableViewPriv    View;
	typedef toEventQuery       Query;
	typedef toEventQueryObserverObject             ObserverObject;
	typedef toEventQueryObserverObject::Observer   Observer;

	enum RowNumberPolicy {
		NoRowNumber = 0,
		BuiltInRowNumber,
		TableRowNumber,
	};

	enum ColumnResizePolicy {
		NoColumnResize = 0,
		HeaderColumnResize,
		RowColumResize,
		CustomColumnResize
	};

	enum
	{
		SelectionBehavior = QAbstractItemView::SelectItems,
		SelectionMode = QAbstractItemView::NoSelection,
		AlternatingRowColorsEnabled = false,
		ContextMenuPolicy = Qt::NoContextMenu,
		ShowRowNumber = TableRowNumber,
		ColumnResize = NoColumnResize
	};
};

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
class TOMVC
	: public _T::Model
	, public _T::Observer
	, public _VP< _T>
	, public _DP< _T>
{
public:
	typedef _T                    Traits;
	typedef typename Traits::View  View;
	typedef typename Traits::Model Model;
	typedef typename Traits::Query Query;
	typedef typename Traits::Observer       Observer;
	typedef typename Traits::ObserverObject ObserverObject;
	typedef _VP<Traits>           ViewPolicy;
	typedef _DP<Traits>           DataProviderPolicy;

	TOMVC(QWidget *parent);
	virtual ~TOMVC();

	void setQuery(Query*);

	QWidget *widget();

	/**
	 * @name toEventQueryObserverObject::Observer
	 * toEventQueryObserverObject::Observer interface implementation
	 */
	///@{
	virtual void observeHeaders(const toQuery::HeaderList&);
	virtual void observeBeginData();
	virtual void observeRow(toQuery::Row&);
	virtual void observeData(toQuery::RowList&);
	virtual void observeData(QObject*); // toQuery or toEventQuery
	virtual void observeEndData();
	virtual void observeDone();
	virtual void observeError(const toConnection::exception &);
	///@}

private:
	View  *m_view;
	Query *m_query;
	ObserverObject *m_observerObject;
};

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
TOMVC<_T, _VP, _DP>::TOMVC(QWidget *parent)
	: Model(parent)
	, Observer()
	, ViewPolicy()
	, DataProviderPolicy()
	, m_view(new View(parent))
	, m_query(NULL)
	, m_observerObject(new ObserverObject(*this, parent))
{
	m_view->setModel(this);
	ViewPolicy::setup(m_view);
	DataProviderPolicy::setup();
}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
TOMVC<_T, _VP, _DP>::~TOMVC()
{

};

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
void TOMVC<_T, _VP, _DP>::setQuery(Query *query)
{
	bool retval;

	if( m_query != NULL && m_query->parent() == this)
	{
		delete m_query;
		m_query = NULL;
	}

	m_query = query;
	m_observerObject->setQuery(query);

	retval = QObject::connect(m_query, SIGNAL(descriptionAvailable(toEventQuery*, const toQColumnDescriptionList &))
			, m_observerObject, SLOT(eqDescriptionAvailable(toEventQuery*, const toQColumnDescriptionList &)));
	Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: descriptionAvailable");
	retval = QObject::connect(m_query, SIGNAL(dataAvailable(toEventQuery*))
			, m_observerObject, SLOT(eqDataAvailable(toEventQuery*)));
	Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: dataAvailable");
	retval = QObject::connect(m_query, SIGNAL(error(toEventQuery*, const toConnection::exception &))
			, m_observerObject, SLOT(eqError(toEventQuery*, const toConnection::exception &)));
	Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: error");
	retval = QObject::connect(m_query, SIGNAL(done(toEventQuery*))
			, m_observerObject, SLOT(eqDone(toEventQuery*)));
	Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: done");
	m_query->start();
}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
QWidget* TOMVC< _T, _VP, _DP>::widget()
{
	return m_view;
}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
void TOMVC< _T, _VP, _DP>::observeHeaders(const toQuery::HeaderList &headers)
{
	toQuery::HeaderList h(headers);
	if( Traits::ShowRowNumber == MVCTraits::TableRowNumber )
	{
		struct toQuery::HeaderDesc d;

		d.name = QString::fromLatin1("#");
		d.datatype = QString::fromLatin1("RowNumber");
		h.prepend(d);
	}
	Model::setHeaders(h);
}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
void TOMVC< _T, _VP, _DP>::observeBeginData()
{
	//Model::beginInsertRows(QModelIndex(), rowCount(), rowCount());
}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
void TOMVC< _T, _VP, _DP>::observeEndData()
{
	//Model::endInsertRows();
}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
void TOMVC< _T, _VP, _DP>::observeRow(toQuery::Row &row)
{
	if( Traits::ShowRowNumber == MVCTraits::TableRowNumber )
	{
		row << 1;
	}
}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
void TOMVC< _T, _VP, _DP>::observeData(toQuery::RowList &rows)
{
	Model::appendRows(rows);
}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
void TOMVC< _T, _VP, _DP>::observeData(QObject *q)
{
	Query *query = dynamic_cast<Query*>(q);

	try
	{
		Q_ASSERT_X(m_query != NULL , qPrintable(__QHERE__), " phantom data");
		Q_ASSERT_X(m_query == query, qPrintable(__QHERE__), " unknown data source");
		Q_ASSERT_X(m_query->columnCount() >0, qPrintable(__QHERE__), " not described yet");

		// TODO to be moved into Policy class (tomvc.h)
		int columns = query->columnCount();
		//toQuery::RowList rows;
		int oldRowCount = Model::rowCount();
		Model::beginInsertRows(QModelIndex(), oldRowCount, oldRowCount + 49 /*rows.size() - 1*/);
		while(query->hasMore())
		{
			toQuery::Row row;
			for(int i=0; i < columns; i++)
			{
				row << query->readValue();
			}
			//rows << row;
			Model::appendRow(row);
			row.clear();			
		}
		Model::endInsertRows();
	}
	TOCATCH
}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
void TOMVC< _T, _VP, _DP>::observeDone()
{

}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
	>
void TOMVC< _T, _VP, _DP>::observeError(const toConnection::exception &e)
{

}

#endif
