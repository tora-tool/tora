
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

#include "core/toeventquery.h"
#include "core/totablemodel.h"

#include <QtCore/QObject>

class toEventQueryObserverObject : public QObject, toEventQuery::Client
{
    Q_OBJECT;
    public:
        class Observer
        {
            public:
                virtual void observeHeaders(const toQueryAbstr::HeaderList&) {};
                virtual void observeBeginData() {};
                virtual void observeRow(toQueryAbstr::Row&) {};
                virtual void observeData(toQueryAbstr::RowList&) {};
                virtual void observeData(QObject*) {};
                virtual void observeDone() {};
                virtual void observeEndData() {};
                virtual void observeError(const toConnection::exception &) {};
                virtual void observeConnectionChanged(toConnection &) {};
        };

        toEventQueryObserverObject(Observer &, QObject *parent = 0);
        ~toEventQueryObserverObject();

        void setQuery(toEventQuery *query);

        toEventQuery *query();

//	template<class Traits>
//	void setModel(typename Traits::Model *model);
//
//	template<class Traits>
//	typename Traits::Model* model() const;

    public slots:
        /**
         * @name toEventQuery::Client
         * toEventQuery::Client interface implementation
         */
        ///@{
        /**
         * Emitted when header descriptions are available
         */
        virtual void eqDescriptionAvailable(toEventQuery*);

        /**
         * Emitted when data has been read.
         * @param rows Number of rows to be read
         */
        virtual void eqDataAvailable(toEventQuery*);

        /**
         * Emitted with error string
         */
        virtual void eqError(toEventQuery*, const toConnection::exception &);

        /**
         * Emitted when done
         */
        virtual void eqDone(toEventQuery*);
        ///@}

        // "stolen from to toResult
        virtual void connectionChanged(toConnection&);

    protected:
        // toTableModelPriv::RowList m_rows;
    private:
        toEventQuery *m_eventQuery;
        Observer &m_observer;
};

template<typename _T>
class DefaultDataProviderPolicy
{
    public:
        typedef _T Traits;
        typedef typename Traits::Query Query;
        typedef typename Traits::Observer Observer;
        void setup() {};

        void fetchData(Query*);
};
