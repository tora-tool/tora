
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

#include "toconnection.h"
#include "tonoblockquery.h"
#include "toresultdepend.h"
#include "tosql.h"


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

bool toResultDepend::canHandle(toConnection &conn)
{
    return toIsOracle(conn);
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
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
}

toResultDepend::~toResultDepend()
{
    delete Query;
}

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

void toResultDepend::query(const QString &sql, const toQList &param)
{
    if (!handled())
        return ;

    delete Query;
    Query = NULL;
    Current = NULL;

    if (!setSQLParams(sql, param))
        return ;

    clear();

    try
    {
        Query = new toNoBlockQuery(connection(),
                                   toQuery::Background,
                                   toSQL::string(SQLResultDepend, connection()),
                                   param);
        Poll.start(100);
    }
    TOCATCH
}

void toResultDepend::poll(void)
{
    try
    {
        if (!toCheckModal(this))
            return ;
        if (Query && Query->poll())
        {
            int cols = Query->describe().size();
            while (Query->poll() && !Query->eof())
            {
                toTreeWidgetItem *item;
                QString owner = Query->readValue();
                QString name = Query->readValue();
                if (!exists(owner, name))
                {
                    if (!Current)
                        item = new toResultViewItem(this, NULL, owner);
                    else
                        item = new toResultViewItem(Current, NULL, owner);
                    item->setText(1, name);
                    for (int i = 2;i < cols;i++)
                        item->setText(i, Query->readValue());
                }
                else
                {
                    for (int i = 2;i < cols;i++)
                        Query->readValue();
                }
            }
            if (Query->eof())
            {
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
                delete Query;
                Query = NULL;
                if (Current)
                {
                    toQList param;
                    param.insert(param.end(), Current->text(0));
                    param.insert(param.end(), Current->text(1));
                    Query = new toNoBlockQuery(connection(),
                                               toQuery::Background,
                                               toSQL::string(SQLResultDepend, connection()),
                                               param);
                }
                else
                    Poll.stop();
            }
        }
        resizeColumnsToContents();
    }
    catch (const QString &exc)
    {
        delete Query;
        Query = NULL;
        Poll.stop();
        toStatusMessage(exc);
    }
}
