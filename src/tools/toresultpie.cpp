
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

#include "tools/toresultpie.h"
#include "core/utils.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "core/tosql.h"

toResultPie::toResultPie(QWidget *parent, const char *name)
    : toPieChart(parent, name)
{
    Query = NULL;
    Columns = 0;
    Started = false;
    LabelFirst = false;
}

void toResultPie::query(const QString &sql, const toQueryParams &param)
{
    if (!handled() || Query)
        return ;

	if (!setSqlAndParams(sql, param))
		return;

    try
    {
        Query = new toEventQuery(this, connection(), sql, param, toEventQuery::READ_ALL);
        connect(Query, &toEventQuery::dataAvailable, this, &toResultPie::receiveData);
        connect(Query, SIGNAL(done(toEventQuery*, unsigned long)), this, SLOT(queryDone()));
        Query->start();
    }
    TOCATCH
}

void toResultPie::receiveData(toEventQuery*)
{
    try
    {
        if (Query)
        {
            if (!Columns)
                Columns = Query->describe().size();
            while (Query->hasMore())
            {
                QString val;
                QString lab;
                if (Columns > 1)
                {
                    if (LabelFirst)
                    {
                        lab = Query->readValue();
                        val = Query->readValue();
                    }
                    else
                    {
                        val = Query->readValue();
                        lab = Query->readValue();
                    }
                    for (int i = 2; i < Columns; i++)
                        Query->readValue();
                }
                else
                    val = Query->readValue();
                if (!Filter.isEmpty() && !Filter.exactMatch(lab))
                    continue;
                if (!ValueFilter.isEmpty() && !ValueFilter.exactMatch(val))
                    continue;
                Values.insert(Values.end(), val.toDouble());
                if (Columns > 1)
                    Labels.insert(Labels.end(), lab);
            }
        }
    }
    catch (const QString &exc)
    {
        delete Query;
        Query = NULL;
        Utils::toStatusMessage(exc);
    }
}

void toResultPie::queryDone(void)
{
    setValues(Values, Labels);
    Values.clear();
    Labels.clear();
    delete Query;
    Query = NULL;
    Columns = 0;
	emit done();
}
