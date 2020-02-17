
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

#include "tools/toresultdepend.h"

#include "core/utils.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "core/tosql.h"


static toSQL SQLResultDepend("toResultDepend:Depends",
                             "SELECT DISTINCT\n"
                             "       referenced_owner \"Owner\",\n"
                             "       referenced_name \"Name\",\n"
                             "       referenced_type \"Type\",\n"
                             "       dependency_type \"Dependency Type\"\n"
                             "  FROM sys.all_dependencies\n"
                             " WHERE owner = :owner<char[101]>\n"
                             "   AND name = :name<char[101]>\n"
                             " ORDER BY referenced_owner,referenced_type,referenced_name",
                             "Display dependencies on an object, must have first two "
                             "columns same columns and same bindings",
                             "0800");
static toSQL SQLResultDepend7("toResultDepend:Depends",
                              "SELECT DISTINCT\n"
                              "       referenced_owner \"Owner\",\n"
                              "       referenced_name \"Name\",\n"
                              "       referenced_type \"Type\",\n"
                              "       'N/A' \"Dependency Type\"\n"
                              "  FROM sys.all_dependencies\n"
                              " WHERE owner = :owner<char[101]>\n"
                              "   AND name = :name<char[101]>\n"
                              " ORDER BY referenced_owner,referenced_type,referenced_name",
                              "",
                              "0703");

bool toResultDepend::canHandle(const toConnection &conn)
{
    return conn.providerIs("Oracle");
}

toResultDepend::toResultDepend(QWidget *parent, const char *name)
    : toResultView(false, false, parent, name)
{
    addColumn(tr("Owner"));
    addColumn(tr("Name"));
    addColumn(tr("Type"));
    addColumn(tr("Dependency"));

    setRootIsDecorated(true);
    setReadAll(true);
    setSQLName(QString::fromLatin1("toResultDepend"));

    Query = NULL;
    Current = NULL;
}

toResultDepend::~toResultDepend()
{
    delete Query;
}

void toResultDepend::query(const QString &sql, toQueryParams const& param)
{
    if (!handled())
        return ;

    delete Query;
    Query = NULL;
    Current = NULL;

    if (!setSqlAndParams(sql, param))
        return ;

    clear();

    try
    {
        Query = new toEventQuery(this
                                 , connection()
                                 , toSQL::string(SQLResultDepend, connection())
                                 , param
                                 , toEventQuery::READ_ALL);
        auto c1 = connect(Query, &toEventQuery::dataAvailable, this, &toResultDepend::receiveData);
        connect(Query, SIGNAL(done(toEventQuery*,unsigned long)), this, SLOT(slotQueryDone()));
        Query->start();
    }
    TOCATCH
}

void toResultDepend::clearData()
{
    clear();
}

void toResultDepend::receiveData(toEventQuery*)
{
    try
    {
        if (!Utils::toCheckModal(this))
            return ;
        if (Query)
        {
            int cols = Query->describe().size();
            while (Query->hasMore())
            {
                toTreeWidgetItem *item;
                QString owner = (QString)Query->readValue();
                QString name = (QString)Query->readValue();
                if (!exists(owner, name))
                {
                    if (!Current)
                        item = new toResultViewItem(this, NULL, owner);
                    else
                        item = new toResultViewItem(Current, NULL, owner);
                    item->setText(1, name);
                    for (int i = 2; i < cols; i++)
                        item->setText(i, (QString)Query->readValue());
                }
                else
                {
                    for (int i = 2; i < cols; i++)
                        Query->readValue();
                }
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

void toResultDepend::slotQueryDone(void)
{
    delete Query;
    Query = NULL;

    if (!Current)
        Current = firstChild();
    else if (Current->firstChild())
        Current = Current->firstChild();
    else if (Current->nextSibling())
        Current = Current->nextSibling();
    else
    {
        do
        {
            Current = Current->parent();
        }
        while (Current && !Current->nextSibling());
        if (Current)
            Current = Current->nextSibling();
    }

    if (Current)
    {
        toQueryParams param;
        param << Current->text(0);
        param << Current->text(1);
        Query = new toEventQuery(this
                                 , connection()
                                 , toSQL::string(SQLResultDepend, connection())
                                 , param
                                 , toEventQuery::READ_ALL);
        auto c1 = connect(Query, &toEventQuery::dataAvailable, this, &toResultDepend::receiveData);
        connect(Query, SIGNAL(done(toEventQuery*,unsigned long)), this, SLOT(slotQueryDone()));
        Query->start();
    }
    resizeColumnsToContents();
} // queryDone

bool toResultDepend::exists(const QString &owner, const QString &name)
{
    toTreeWidgetItem *item = firstChild();
    while (item)
    {
        if (item->text(0) == owner && item->text(1) == name)
            return true;
        if (item->firstChild())
            item = item->firstChild();
        else if (item->nextSibling())
            item = item->nextSibling();
        else
        {
            do
            {
                item = item->parent();
            }
            while (item && !item->nextSibling());
            if (item)
                item = item->nextSibling();
        }
    }
    return false;
}
