
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

#include "tools/tobarchart.h"
#include "core/toresult.h"

#include <time.h>
#include <list>

class QMenu;
class toEventQuery;
class toSQL;

/** Display the result of a query in a barchart. The first column of the query should
 * contain the x value and the rest of the columns should be values of the diagram. The
 * legend is the column name. Connects to the tool timer for updates automatically.
 */

class toResultBar : public toBarChart, public toResult
{
    private:
        Q_OBJECT
        /** Display flow in change per second instead of actual values.
         */
        bool Flow;
        bool Started;
        /** Timestamp of last fetch.
         */
        time_t LastStamp;
        /** Last read values.
         */
        std::list<double> LastValues;
        bool First;
        toEventQuery *Query;
        unsigned int Columns;

    public:
        /** Create widget.
         * @param parent Parent of list.
         * @param name Name of widget.
         */
        toResultBar(QWidget *parent, const char *name = NULL);

        /** Destroy chart
         */
        ~toResultBar();

        /** Display actual values or flow/s.
         * @param on Display flow or absolute values.
         */
        void setFlow(bool on)
        {
            Flow = on;
        }
        /** Return if flow is displayed.
         * @return If flow is used.
         */
        bool flow(void)
        {
            return Flow;
        }

        /** Reimplemented abstract method
         */
        void query(const QString &sql, toQueryParams const& param) override;

        /** Reimplemented for internal reasons.
         */
        void clear(void)
        {
            LastStamp = 0;
            LastValues.clear();
            toBarChart::clear();
            First = true;
        }

        /** Transform valueset. Make it possible to perform more complex transformation.
         * called directly before adding the valueset to the chart. After flow transformation.
         * Default is passthrough.
         * @param input The untransformed valueset.
         * @return The valueset actually added to the chart.
         */
        virtual std::list<double> transform(std::list<double> &input);
        /** Handle any connection
         */
	bool canHandle(const toConnection &) override
        {
            return true;
        }

        /** override setParams to public */
        void setParams(toQueryParams const& par)
        {
            toResult::setParams(par);
        }

    signals:
        void done();

    public slots:
        /** Reimplemented for internal reasons.
         */
        void refresh(void) override
        {
            query(sql(), params());
        }

    protected slots:
        /** Reimplemented for internal reasons.
         */
        void connectionChanged(void) override;
        /** Reimplemented for internal reasons.
         */
        void addMenues(QMenu *) override;
    private slots:
        void receiveData(toEventQuery*);
        void editSQL(void);
        void queryDone(void);
};

