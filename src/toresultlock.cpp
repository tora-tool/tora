
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
#include "toresultlock.h"
#include "tosql.h"
#include "totool.h"

#include <map>


bool toResultLock::canHandle(toConnection &conn)
{
    return toIsOracle(conn);
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
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
}

toResultLock::~toResultLock()
{
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

void toResultLock::query(const QString &sql,
                         const toQList &param)
{
    if (!handled())
        return ;

    if (!setSQLParams(sql, param))
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
        toQList par;
        if (!sql.isEmpty())
        {
            par.insert(par.end(), sql);
            Query = new toNoBlockQuery(connection(), toQuery::Background,
                                       toSQL::string(SQLLock, connection()), par);
        }
        else
        {
            Query = new toNoBlockQuery(connection(), toQuery::Background,
                                       toSQL::string(SQLBlockingLock, connection()), par);
        }
        Poll.start(100);
    }
    TOCATCH
}

#define MARK_COL 20

void toResultLock::poll(void)
{
    try
    {
        if (!toCheckModal(this))
            return ;
        if (Query && Query->poll())
        {
            if (!Query->eof())
            {
                do
                {
                    toTreeWidgetItem *item;
                    if (!LastItem)
                        item = new toResultViewItem(this, NULL);
                    else
                        item = new toResultViewItem(LastItem, NULL);
                    toQDescList desc = Query->describe();
                    for (unsigned int pos = 0;pos < desc.size();pos++)
                        item->setText(int(pos), Query->readValue());
                }
                while (!Query->eof());
            }

            delete Query;
            Query = NULL;

            LastItem = NULL;
            toTreeWidgetItem *next = NULL;
            for (toTreeWidgetItem *item = firstChild();item;item = next)
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
                        toQList par;
                        par.insert(par.end(), LastItem->text(0));
                        Query = new toNoBlockQuery(connection(), toQuery::Background,
                                                   toSQL::string(SQLLock, connection()), par);
                    }
                    else
                    {
                        toTreeWidgetItem *cn = NULL;
                        for (toTreeWidgetItem *ci = firstChild();ci;ci = cn)
                        {
                            if (ci != item && ci->text(0) == item->text(0))
                            {
                                if (ci->firstChild())
                                {
                                    ci = ci->firstChild();
                                    cn = new toResultViewItem(item, NULL);
                                    for (int i = 0;i < columns();i++)
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
            if (!LastItem)
                Poll.stop();
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
