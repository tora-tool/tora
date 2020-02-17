
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

#include "tools/toresultbar.h"

#include "core/utils.h"
#include "core/toeventquery.h"

#include <QMenu>
#include <QtCore/QObject>

toResultBar::toResultBar(QWidget *parent, const char *name)
    : toBarChart(parent, name)
    , Flow(true)
    , Started(false)
    , LastStamp(0)
    , First(true)
    , Query(NULL)
    , Columns(0)
{}

toResultBar::~toResultBar()
{
    delete Query;
}

void toResultBar::query(const QString &sql, toQueryParams const& param)
{
    if (!handled() || Query)
        return ;

    setSqlAndParams(sql, param);

    try
    {
        Query = new toEventQuery(this, connection(), sql, param, toEventQuery::READ_ALL);
        connect(Query, &toEventQuery::dataAvailable, this, &toResultBar::receiveData);
        connect(Query, SIGNAL(done(toEventQuery*, unsigned long)), this, SLOT(queryDone()));
        Query->start();
    }
    TOCATCH
}

void toResultBar::receiveData(toEventQuery*)
{
    try
    {
        if (Query)
        {
            toQColumnDescriptionList desc;
            if (!Columns)
            {
                desc = Query->describe();
                Columns = desc.size();
            }

            if (First)
            {
                if ( desc.empty() )
                    desc = Query->describe();
                clear();
                std::list<QString> labels;
                for (toQColumnDescriptionList::iterator i = desc.begin(); i != desc.end(); i++)
                    if (i != desc.begin())
                        labels.push_back((*i).Name);
                setLabels(labels);
                First = false;
            }

            while (Query->hasMore())
            {
                unsigned int num = 0;
                QString lab = (QString)Query->readValue();
                num++;
                std::list<double> vals;
                while (!Query->eof() && num < Columns)
                {
                    vals.insert(vals.end(), Query->readValue().toDouble());
                    num++;
                }

                if (Flow)
                {
                    time_t now = time(NULL);
                    if (now != LastStamp)
                    {
                        if (LastValues.size() > 0)
                        {
                            std::list<double> dispVal;
                            std::list<double>::iterator i = vals.begin();
                            std::list<double>::iterator j = LastValues.begin();
                            while (i != vals.end() && j != LastValues.end())
                            {
                                dispVal.insert(dispVal.end(), (*i - *j) / (now - LastStamp));
                                i++;
                                j++;
                            }
                            std::list<double> tmp = transform(dispVal);
                            addValues(tmp, lab);
                        }
                        LastValues = vals;
                        LastStamp = now;
                    }
                }
                else
                {
                    std::list<double> tmp = transform(vals);
                    addValues(tmp, lab);
                }
            }
        }
    }
    catch (const QString &exc)
    {
        Columns = 0;
        delete Query;
        Query = NULL;
        Utils::toStatusMessage(exc);
    }
}

void toResultBar::queryDone(void)
{
    Columns = 0;
    delete Query;
    Query = NULL;
    update();
    emit done();
}

std::list<double> toResultBar::transform(std::list<double> &input)
{
    return input;
}

void toResultBar::connectionChanged(void)
{
    toResult::connectionChanged();
    clear();
    First = true;
}

void toResultBar::addMenues(QMenu *popup)
{
    if (!sqlName().isEmpty())
    {
        popup->addSeparator();
        popup->addAction(tr("Edit SQL..."),
                         this,
                         SLOT(editSQL()));
    }
}

void toResultBar::editSQL(void)
{
    //toMainWindow::lookup()->editSQL(sqlName());
    Utils::toStatusMessage("Not yet implemented editSQL(Name).");
}
