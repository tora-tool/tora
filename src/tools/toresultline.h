
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
#include <time.h>

#include <list>
#include "tools/tolinechart.h"

class QMenu;
class toEventQuery;
class toSQL;

/** Display the result of a query in a piechart. The first column of the query should
 * contain the x value and the rest of the columns should be values of the diagram. The
 * legend is the column name. Connects to the tool timer for updates automatically.
 */
class toResultLine : public toLineChart, public toResult
{
        Q_OBJECT;
    public:
        /** Create widget.
         * @param parent Parent of list.
         * @param name Name of widget.
         */
        toResultLine(QWidget *parent, const char *name = NULL);
        /** Destroy chart
         */
        ~toResultLine();
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

        /** override to public */
        void setParams(toQueryParams const& par);

        /** Reimplemented for internal reasons.
         */
        void query(const QString &sql, const toQueryParams &param) override;

        /** Reimplemented for internal reasons.
         */
        void clear(void)
        {
            LastStamp = 0;
            LastValues.clear();
            toLineChart::clear();
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
        bool canHandle(const toConnection &conn) override
        {
            return conn.providerIs("Oracle");
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
        void queryDone(void);
        void editSQL(void);

    private:
        bool Flow; // Display flow in change per second instead of actual values.
        bool Started;
        time_t LastStamp; // Timestamp of last fetch
        std::list<double> LastValues; // Last read values
        bool First;
        toEventQuery *Query;
        unsigned int Columns;
};
