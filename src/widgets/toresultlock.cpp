
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

#include "widgets/toresultlock.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "core/toqvalue.h"
#include "core/tosql.h"
#include "core/totool.h"
#include "core/utils.h"

#include <map>


bool toResultLock::canHandle(const toConnection &conn)
{
    return conn.providerIs("Oracle");
}

toResultLock::toResultLock(QWidget *parent, const char *name)
    : toResultView(false, false, parent, name)
{
    setAllColumnsShowFocus(true);
    setSorting( -1);
    setRootIsDecorated(true);
    addColumn(tr("Session"));
    addColumn(tr("Schema"));
    addColumn(tr("Osuser"));
    addColumn(tr("Program"));
    addColumn(tr("Type"));
    addColumn(tr("Mode"));
    addColumn(tr("Request"));
    addColumn(tr("Object"));
    addColumn(tr("Grabbed"));
    addColumn(tr("Requested"));
    setSQLName(QString::fromLatin1("toResultLock"));

    Query = NULL;
}

toResultLock::~toResultLock()
{
    if (Query)
        delete Query;
}

static toSQL SQLBlockingLock("toResultLock:BlockingLocks",
                             "select b.sid,b.schemaname,b.osuser,b.program,\n"
                             "       decode(a.type,\n"
                             "              'MR', 'Media Recovery',\n"
                             "              'RT', 'Redo Thread',\n"
                             "              'UN', 'User Name',\n"
                             "              'TX', 'Transaction',\n"
                             "              'TM', 'DML',\n"
                             "              'UL', 'PL/SQL User Lock',\n"
                             "              'DX', 'Distributed Xaction',\n"
                             "              'CF', 'Control File',\n"
                             "              'IS', 'Instance State',\n"
                             "              'FS', 'File Set',\n"
                             "              'IR', 'Instance Recovery',\n"
                             "              'ST', 'Disk Space Transaction',\n"
                             "              'TS', 'Temp Segment',\n"
                             "              'IV', 'Library Cache Invalidation',\n"
                             "              'LS', 'Log Start or Switch',\n"
                             "              'RW', 'Row Wait',\n"
                             "              'SQ', 'Sequence Number',\n"
                             "              'TE', 'Extend Table',\n"
                             "              'TT', 'Temp Table',\n"
                             "              'Internal ('||a.type||')'),\n"
                             "       DECODE(a.lmode,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',TO_CHAR(a.lmode)),\n"
                             "       DECODE(a.request,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',TO_CHAR(a.request)),\n"
                             "       d.object_name,\n"
                             "       ' ',\n"
                             "       TO_CHAR(SYSDATE-a.CTIME/3600/24)\n"
                             "  from v$lock a,v$session b,v$locked_object c,sys.all_objects d\n"
                             " where a.sid = b.sid\n"
                             "   and c.session_id = a.sid\n"
                             "   and exists (select 'X'\n"
                             "                 from v$locked_object bb,\n"
                             "                      v$lock cc\n"
                             "                where bb.session_id = cc.sid\n"
                             "                  and cc.sid != a.sid\n"
                             "                  and cc.id1 = a.id1\n"
                             "                  and cc.id2 = a.id2\n"
                             "                  and bb.object_id = c.object_id)\n"
                             "   and d.object_id = c.object_id\n"
                             "   and a.request != 0",
                             "List session blocked by a lock");

static toSQL SQLLock("toResultLock:Locks",
                     "select b.sid,\n"
                     "       b.schemaname,\n"
                     "       b.osuser,\n"
                     "       b.program,\n"
                     "       decode(a.type,\n"
                     "              'MR', 'Media Recovery',\n"
                     "              'RT', 'Redo Thread',\n"
                     "              'UN', 'User Name',\n"
                     "              'TX', 'Transaction',\n"
                     "              'TM', 'DML',\n"
                     "              'UL', 'PL/SQL User Lock',\n"
                     "              'DX', 'Distributed Xaction',\n"
                     "              'CF', 'Control File',\n"
                     "              'IS', 'Instance State',\n"
                     "              'FS', 'File Set',\n"
                     "              'IR', 'Instance Recovery',\n"
                     "              'ST', 'Disk Space Transaction',\n"
                     "              'TS', 'Temp Segment',\n"
                     "              'IV', 'Library Cache Invalidation',\n"
                     "              'LS', 'Log Start or Switch',\n"
                     "              'RW', 'Row Wait',\n"
                     "              'SQ', 'Sequence Number',\n"
                     "              'TE', 'Extend Table',\n"
                     "              'TT', 'Temp Table',\n"
                     "              'Internal ('||a.type||')'),\n"
                     "       DECODE(a.lmode,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',TO_CHAR(a.lmode)),\n"
                     "       DECODE(e.request,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',TO_CHAR(e.request)),\n"
                     "       d.object_name,\n"
                     "       TO_CHAR(SYSDATE-a.CTIME/3600/24),\n"
                     "       TO_CHAR(SYSDATE-e.CTIME/3600/24)\n"
                     "  from v$lock a, v$session b,v$locked_object c,sys.all_objects d,v$lock e\n"
                     " where a.sid = b.sid\n"
                     "   and a.lmode != 0\n"
                     "   and c.session_id = a.sid\n"
                     "   and c.object_id = d.object_id\n"
                     "   and exists (select 'X'\n"
                     "                 from v$locked_object bb,\n"
                     "                      v$lock cc\n"
                     "                where bb.session_id = cc.sid\n"
                     "                  and cc.sid != a.sid\n"
                     "                  and cc.id1 = a.id1\n"
                     "                  and cc.id2 = a.id2\n"
                     "                  and bb.object_id = c.object_id)\n"
                     "   and a.id1 = e.id1\n"
                     "   and a.id2 = e.id2\n"
                     "   and e.sid = :f1<char[101]>\n"
                     "   and e.lmode != e.request\n"
                     "   and e.request != 0",
                     "List locks in a session");

void toResultLock::startQuery(void)
{
    connect(Query, SIGNAL(dataAvailable(toEventQuery*)), this, SLOT(poll()));
    connect(Query, SIGNAL(done(toEventQuery*)), this, SLOT(queryDone()));
    Query->start();
} // startQuery

void toResultLock::query(const QString &sql,
                         const toQueryParams &param)
{
    if (!handled())
        return ;

    if (!setSqlAndParams(sql, param))
        return ;

    if (Query)
    {
        delete Query;
        Query = NULL;
    }
    clear();
    Checked.clear();

    try
    {
        LastItem = NULL;
        if (!sql.isEmpty())
        {
            Query = new toEventQuery(this, connection(), toSQL::string(SQLLock, connection()), toQueryParams() << sql, toEventQuery::READ_ALL);
        }
        else
        {
            Query = new toEventQuery(this, connection(), toSQL::string(SQLBlockingLock, connection()), toQueryParams(), toEventQuery::READ_ALL);
        }
        startQuery();
    }
    TOCATCH
} // query

void toResultLock::poll(void)
{
    try
    {
        if (!Utils::toCheckModal(this))
            return ;
        if (Query)
        {
            while (Query->hasMore())
            {
                toTreeWidgetItem *item;
                // Check if item has to be added to top level...
                if (!LastItem)
                    item = new toResultViewItem(this, NULL);
                // ...or attached as a child record to some parent.
                else
                    item = new toResultViewItem(LastItem, NULL);
                toQColumnDescriptionList desc = Query->describe();
                for (unsigned int pos = 0; pos < desc.size(); pos++)
                    item->setText(int(pos), (QString)Query->readValue());
            }
        }
    }
    catch (const QString &exc)
    {
        delete Query;
        Query = NULL;
        Utils::toStatusMessage(exc);
    }
} // poll

// Which column is used as a marker for records processed for children. This
// column will get value "Yes" so that it is not processed on on subsequent pass.
#define MARK_COL 20

void toResultLock::queryDone(void)
{
    if (Query)
    {
        delete Query;
        Query = NULL;
    }

    LastItem = NULL;
    toTreeWidgetItem *next = NULL;
    for (toTreeWidgetItem *item = firstChild(); item; item = next)
    {
        int sid = item->text(0).toInt();
        if (item->text(MARK_COL).isEmpty())
        {
            item->setText(MARK_COL, QString::fromLatin1("Yes"));
            item->setOpen(true);
            if (!Checked[sid])
            {
                Checked[sid] = true;
                LastItem = item;
                toQueryParams par = toQueryParams() << LastItem->text(0);
                Query = new toEventQuery(this, connection(), toSQL::string(SQLLock, connection()), par, toEventQuery::READ_ALL);
                startQuery();
            }
            else
            {
                toTreeWidgetItem *cn = NULL;
                for (toTreeWidgetItem *ci = firstChild(); ci; ci = cn)
                {
                    if (ci != item && ci->text(0) == item->text(0))
                    {
                        if (ci->firstChild())
                        {
                            ci = ci->firstChild();
                            cn = new toResultViewItem(item, NULL);
                            for (int i = 0; i < columns(); i++)
                                cn->setText(i, ci->text(i));
                        }
                        break;
                    }
                    if (ci->firstChild())
                    {
                        cn = ci->firstChild();
                    }
                    else if (ci->nextSibling())
                        cn = ci->nextSibling();
                    else
                    {
                        cn = ci;
                        do
                        {
                            cn = cn->parent();
                        }
                        while (cn && !cn->nextSibling());
                        if (cn)
                            cn = cn->nextSibling();
                    }
                }
            }
            break;
        }
        if (item->firstChild())
        {
            next = item->firstChild();
        }
        else if (item->nextSibling())
            next = item->nextSibling();
        else
        {
            next = item;
            do
            {
                next = next->parent();
            }
            while (next && !next->nextSibling());
            if (next)
                next = next->nextSibling();
        }
    }
} // queryDone
