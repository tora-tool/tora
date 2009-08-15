
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

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tonoblockquery.h"
#include "toresultpie.h"
#include "tosql.h"


toResultPie::toResultPie(QWidget *parent, const char *name)
        : toPieChart(parent, name)
{
    Query = NULL;
    Columns = 0;
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
    Started = false;
    LabelFirst = false;
}

void toResultPie::start(void)
{
    if (!Started)
    {
        try
        {
            connect(timer(), SIGNAL(timeout()), this, SLOT(refresh()));
        }
        TOCATCH
        Started = true;
    }
}

void toResultPie::stop(void)
{
    if (Started)
    {
        try
        {
            disconnect(timer(), SIGNAL(timeout()), this, SLOT(refresh()));
        }
        TOCATCH
        Started = false;
    }
}

void toResultPie::query(const QString &sql, const toQList &param)
{
    if (!handled() || Query)
        return ;

    start();
    if (!setSQLParams(sql, param))
        return ;

    try
    {
        Query = new toNoBlockQuery(connection(), toQuery::Background, sql, param);
        Poll.start(100);
    }
    TOCATCH
}

void toResultPie::poll(void)
{
    try
    {
        if (Query && Query->poll())
        {
            if (!Columns)
                Columns = Query->describe().size();
            while (Query->poll() && !Query->eof())
            {
                QString val;
                QString lab;
                if (Columns > 1)
                {
                    if (LabelFirst)
                    {
                        lab = Query->readValueNull();
                        val = Query->readValueNull();
                    }
                    else
                    {
                        val = Query->readValueNull();
                        lab = Query->readValueNull();
                    }
                    for (int i = 2;i < Columns;i++)
                        Query->readValueNull();
                }
                else
                    val = Query->readValueNull();
                if (!Filter.isEmpty() && !Filter.exactMatch(lab))
                    continue;
                if (!ValueFilter.isEmpty() && !ValueFilter.exactMatch(val))
                    continue;
                Values.insert(Values.end(), val.toDouble());
                if (Columns > 1)
                    Labels.insert(Labels.end(), lab);
            }
            if (Query->eof())
            {
                setValues(Values, Labels);
                Values.clear();
                Labels.clear();
                delete Query;
                Query = NULL;
                Columns = 0;
                Poll.stop();
            }
        }
    }
    catch (const QString &exc)
    {
        delete Query;
        Query = NULL;
        Poll.stop();
        toStatusMessage(exc);
    }
}
