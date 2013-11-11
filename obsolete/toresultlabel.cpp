
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

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "toeventquery.h"
#include "toresultlabel.h"
#include "tosql.h"
#include "totool.h"

//Added by qt3to4:
#include <QtGui/QLabel>

toResultLabel::toResultLabel(QWidget *parent, const char *name)
    : QLabel(parent)
{
    setObjectName(name);
    Query = NULL;
}

toResultLabel::~toResultLabel()
{
    if (Query)
        delete Query;
}

void toResultLabel::query(const QString &sql, const toQList &param)
{
    if (!setSQLParams(sql, param))
        return ;

    try
    {
        clear();
        if (Query)
        {
            delete Query;
            Query = NULL;
        }

        Query = new toEventQuery(connection(), toQuery::Background, sql, param);
        res.clear();
        connect(Query, SIGNAL(dataAvailable()), this, SLOT(poll()));
        connect(Query, SIGNAL(done()), this, SLOT(queryDone()));
        Query->slotReadAll(); // indicate that all records should be fetched
        Query->start();
    }
    TOCATCH
}

#define THRESHOLD 10240

void toResultLabel::poll(void)
{
    try
    {
        if (!toCheckModal(this))
            return ;

        if (Query)
        {
            while (Query->hasMore())
            {
                res << Query->readValue();
            }
            setText(res.join(QString::fromLatin1("/")));
        }
    }
    catch (const QString &exc)
    {
        delete Query;
        Query = NULL;
        toStatusMessage(exc);
    }
}

void toResultLabel::queryDone(void)
{
    delete Query;
    Query = NULL;
    res.clear();
} // queryDone