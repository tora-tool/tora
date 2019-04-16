
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

#pragma once

#include "core/toresult.h"
#include "core/toeditwidget.h"
#include "core/toeventquery.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "core/utils.h"

#include "core/toproviderobserver.h"
#include "core/totablemodel.h"
#include "widgets/toworkingwidget.h"

#include <QtCore/QObject>
#include <QHeaderView>

namespace Views
{
    class toTableView;
}

/** toTableView traits */
struct MVCTraits
{
    typedef toTableModelPriv                       Model;
    typedef typename Views::toTableView            View;
    typedef toEventQuery                           Query;
    typedef toEventQueryObserverObject             ObserverObject;
    typedef toEventQueryObserverObject::Observer   Observer;
    typedef toWorkingWidgetNew                     WorkingWidget;

    enum RowNumberPolicy
    {
        NoRowNumber = 0,
        BuiltInRowNumber,
        TableRowNumber,
    };

    enum ColumnResizePolicy
    {
        NoColumnResize = 0,
        HeaderColumnResize,
        RowColumResize,
        CustomColumnResize
    };

    static const int  SelectionBehavior = QAbstractItemView::SelectItems;
    static const int  SelectionMode = QAbstractItemView::NoSelection;
    static const bool AlternatingRowColorsEnabled = false;
    static const int  ContextMenuPolicy = Qt::NoContextMenu; // DefaultContextMenu => the widget's QWidget::contextMenuEvent() handler is called
    static const int  ShowRowNumber = TableRowNumber;
    static const int  ColumnResize = NoColumnResize;
    static const bool ShowWorkingWidget = true;
    static const bool WorkingWidgetInteractive = true;
    static const bool SortingEnabled = true;
};

class toResult2
{
public:
    virtual void refreshWithParams(toQueryParams const& params) = 0;
    virtual void clear() = 0;
    virtual ~toResult2(){};
    static toResult2* fromQWidget(QWidget *w)
    {
        QAbstractItemView *view = dynamic_cast<QAbstractItemView*>(w);
        if (view == NULL)
            return NULL;
        return dynamic_cast<toResult2*>(view->model());
    }
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
    , public toResult2
{
    public:
        typedef _T                     Traits;
        typedef typename Traits::View  View;
        typedef typename Traits::Model Model;
        typedef typename Traits::Query Query;
        typedef typename Traits::Observer       Observer;
        typedef typename Traits::ObserverObject ObserverObject;
        typedef typename Traits::WorkingWidget  WorkingWidget;
        typedef _VP<Traits>           ViewPolicy;
        typedef _DP<Traits>           DataProviderPolicy;

        TOMVC(QWidget *parent);
        virtual ~TOMVC();

        void setQuery(Query*);

        View* view();
        QWidget* widget();

        /**
         * @name toEventQueryObserverObject::Observer
         * toEventQueryObserverObject::Observer interface implementation
         */
        ///@{
        virtual void observeHeaders(const toQueryAbstr::HeaderList&);
        virtual void observeBeginData();
        virtual void observeRow(toQueryAbstr::Row&);
        virtual void observeData(toQueryAbstr::RowList&);
        virtual void observeData(QObject*); // toQuery or toEventQuery
        virtual void observeEndData();
        virtual void observeDone();
        virtual void observeError(const toConnection::exception &);
        ///@}

        /**
         * toResult like interface
         */
        ///@{
        /** Get the current connection from the closest tool.
         * @return Reference to connection.
         * NOTE: View must inherit from QWidget
         */
        toConnection& connection();

        /** Set the SQL statement of this list
         * @param sql String containing statement.
         */
        void setSQL(const QString &sql);

        /** Set the SQL statement of this list. This will also affect @ref Name.
         * @param sql SQL containing statement.
         */
        void setSQL(const toSQL &sql);

        /** Set SQL name of list.
         */
        void setSQLName(const QString &name);

        /** Reexecute with changed parameters.
         * @param list of query parameters
         */
        void refreshWithParams(toQueryParams const& params) override;
        void clear() override;

    protected:
        /** Perform a query - can be re-implemented by subclasses
         */
        virtual void query();

        void setParams(toQueryParams const& params);

        ///@}
    private:
        View  *m_view;
        WorkingWidget *m_workingWidget;

        ObserverObject *m_observerObject;

        QString m_SQL, m_SQLName;
        toQueryParams m_Params;
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
    , m_observerObject(new ObserverObject(*this, parent))
    , m_workingWidget(NULL)
{
    m_view->setModel(this);
    ViewPolicy::setup(m_view);
    DataProviderPolicy::setup();

    if (_T::ShowWorkingWidget)
    {
        m_workingWidget = new WorkingWidget(m_view);
    }
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

    if ( query->parent() == this)
    {
        // small hack, do not share toEventQuery's ownership with the observer class
        // hand over the ownership to observer
        query->setParent(m_observerObject);
    }

    Model::clearAll();

    m_observerObject->setQuery(query);

    retval = QObject::connect(query, SIGNAL(descriptionAvailable(toEventQuery*))
                              , m_observerObject, SLOT(eqDescriptionAvailable(toEventQuery*)));
    Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: descriptionAvailable");

    //retval = QObject::connect(query, SIGNAL(dataAvailable(toEventQuery*)), m_observerObject, SLOT(eqDataAvailable(toEventQuery*)));
    //Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: dataAvailable");
    connect(query, &Query::dataAvailable, m_observerObject, &ObserverObject::eqDataAvailable);

    retval = QObject::connect(query, SIGNAL(error(toEventQuery*, const toConnection::exception &))
                              , m_observerObject, SLOT(eqError(toEventQuery*, const toConnection::exception &)));
    Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: error");
    retval = QObject::connect(query, SIGNAL(done(toEventQuery*,unsigned long))
                              , m_observerObject, SLOT(eqDone(toEventQuery*)));
    Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: done");

    if (_T::ShowWorkingWidget)
    {
        retval = QObject::connect(query, SIGNAL(descriptionAvailable(toEventQuery*))
                                  , m_workingWidget, SLOT(undisplay()));
        Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: descriptionAvailable");
        retval = QObject::connect(query, SIGNAL(dataAvailable(toEventQuery*))
                                  , m_workingWidget, SLOT(undisplay()));
        Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: dataAvailable");
        retval = QObject::connect(query, SIGNAL(error(toEventQuery*, const toConnection::exception &))
                                  , m_workingWidget, SLOT(undisplay()));
        Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: error");
        retval = QObject::connect(query, SIGNAL(done(toEventQuery*,unsigned long))
                                  , m_workingWidget, SLOT(undisplay()));
        Q_ASSERT_X(retval, qPrintable(__QHERE__), "connect failed: done");

        if (_T::WorkingWidgetInteractive)
        {
            retval = QObject::connect(m_workingWidget, SIGNAL(stop()), query, SLOT(stop()));
        }
        m_workingWidget->display();
    }

    query->start();
}

template<
typename _T,
         template <class> class _VP,
         template <class> class _DP
         >
typename TOMVC< _T, _VP, _DP>::View* TOMVC< _T, _VP, _DP>::view()
{
    return m_view;
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
void TOMVC< _T, _VP, _DP>::observeHeaders(const toQueryAbstr::HeaderList &headers)
{
    toQueryAbstr::HeaderList h(headers);
    if ( Traits::ShowRowNumber == MVCTraits::TableRowNumber )
    {
        struct toQueryAbstr::HeaderDesc d;

        d.name = QString("#");
        d.name_orig = QString("#");
        d.datatype = QString("RowNumber");
        d.hidden = false;
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
void TOMVC< _T, _VP, _DP>::observeRow(toQueryAbstr::Row &row)
{
    if ( Traits::ShowRowNumber == MVCTraits::TableRowNumber )
    {
        row << 1;
    }
}

template<
typename _T,
         template <class> class _VP,
         template <class> class _DP
         >
void TOMVC< _T, _VP, _DP>::observeData(toQueryAbstr::RowList &rows)
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
        Q_ASSERT_X(m_observerObject->query() != NULL , qPrintable(__QHERE__), " phantom data");
        Q_ASSERT_X(m_observerObject->query() == query, qPrintable(__QHERE__), " unknown data source");
        Q_ASSERT_X(m_observerObject->query()->columnCount() >0, qPrintable(__QHERE__), " not described yet");

        // TODO to be moved into Policy class (tomvc.h)
        int columns = query->columnCount();
        //toQueryAbstr::RowList rows;
        int oldRowCount = Model::rowCount();
        unsigned int rows = query->rowsAvaiable();
        Model::beginInsertRows(QModelIndex(), oldRowCount, oldRowCount + rows - 1);
        while (query->hasMore())
        {
            toQueryAbstr::Row row;
            for (int i=0; i < columns; i++)
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

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
>
toConnection& TOMVC< _T, _VP, _DP>::connection()
{
    return toConnection::currentConnection(m_view);
}

template<
    typename _T,
    template <class> class _VP,
    template <class> class _DP
>
void TOMVC< _T, _VP, _DP>::setSQL(const QString &sql)
{
    m_SQL = sql;
}

template<
    typename _T,
    template <class> class _VP,
    template <class> class _DP
>
void TOMVC< _T, _VP, _DP>::setSQL(const toSQL &sql)
{
    setSQLName(sql.name());
    try
    {
        m_Params.clear();
        setSQL(toSQL::string(sql, connection()));
    }
    catch (QString const& e)
    {
        TLOG(8, toDecorator, __HERE__) << e << std::endl;
        m_SQL.clear();
        widget()->setEnabled(false);
    }
}

template<
    typename _T,
    template <class> class _VP,
    template <class> class _DP
>
void TOMVC< _T, _VP, _DP>::setSQLName(const QString &name)
{
    m_SQLName = name;
}

template<
    typename _T,
    template <class> class _VP,
    template <class> class _DP
>
void TOMVC< _T, _VP, _DP>::refreshWithParams(toQueryParams const& params)
{
    setParams(params);
    QString sql(m_SQL);
    if(sql.isEmpty())
    {
        sql = toSQL::string(m_SQLName, connection());
    }
    toEventQuery *Query = new toEventQuery(this, connection(), sql, m_Params, toEventQuery::READ_ALL);
    setQuery(Query);
}

template<
	typename _T,
	template <class> class _VP,
	template <class> class _DP
>
void TOMVC< _T, _VP, _DP>::clear()
{
    Model::clearAll();
}

template<
    typename _T,
    template <class> class _VP,
    template <class> class _DP
>
void TOMVC< _T, _VP, _DP>::query()
{
    QString sql(m_SQL);
    if(sql.isEmpty())
    {
        sql = toSQL::string(m_SQLName, connection());
    }
    toEventQuery *Query = new toEventQuery(this, connection(), sql, m_Params, toEventQuery::READ_ALL);
    setQuery(Query);
}

template<
    typename _T,
    template <class> class _VP,
    template <class> class _DP
>
void TOMVC< _T, _VP, _DP>::setParams(toQueryParams const& params)
{
    m_Params = params;
}
