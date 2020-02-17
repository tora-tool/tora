
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

#include "result/toresultlock.h"

#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "core/toqvalue.h"
#include "core/tosql.h"
#include "core/totool.h"
#include "core/utils.h"

#include <QWidget>
#include <QVBoxLayout>

//bool toResultLock::canHandle(const toConnection &conn)
//{
//    return conn.providerIs("Oracle");
//}

toResultLock::toResultLock(QWidget *parent, const char *name)
    : ResutLock::MVC(parent)
{
//    if (name)
//        QWidget::setObjectName(name);
//    QVBoxLayout *vbox = new QVBoxLayout;
//    vbox->setSpacing(0);
//    vbox->setContentsMargins(0, 0, 0, 0);
//    QWidget::setLayout(vbox);

//    mvc = new ResutLock::MVC(this);
//    QWidget::layout()->addWidget(mvc->widget());

//    setAllColumnsShowFocus(true);
//    setSorting( -1);
//    setRootIsDecorated(true);
    setSQLName(QString::fromLatin1("toResultLock:Locks"));
}

toResultLock::~toResultLock()
{
}

static toSQL SQLLock("toResultLock:Locks",
                     "select b.sid                                                                                 as \"Session\", \n"
                     "       b.schemaname                                                                          as Schema,  \n"
                     "       b.osuser                                                                              as Osuser,  \n"
                     "       b.program                                                                             as Program, \n"
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
                     "              'Internal ('||a.type||')')                                                     as Type, \n"
                     "       DECODE(a.lmode,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',TO_CHAR(a.lmode)) as \"Mode\", \n"
                     "       DECODE(e.request,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',TO_CHAR(e.request)) as Request,\n"
                     "       d.object_name                                                                         as Object,   \n"
                     "       TO_CHAR(SYSDATE-a.CTIME/3600/24)                                                      as Grabbed,  \n"
                     "       TO_CHAR(SYSDATE-e.CTIME/3600/24)                                                      as Requested \n"
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

//SELECT decode( a.blocker_sid , NULL , '<chain id#' ||a.chain_id||'>' ) chain_id,
//       RPAD( '+' , LEVEL , '-' ) ||a.sid sid,
//       RPAD( ' ' , LEVEL , ' ' ) ||a.wait_event_text wait_event,
//        'Current Process: '||osid W_PROC,
//        --'SID '||i.instance_name INSTANCE,
//        'INST #: '||instance INST,'Blocking Process: '||decode(blocker_osid,null,'',blocker_osid)|| ' from Instance '||blocker_instance BLOCKER_PROC,
//        'Number of waiters: '||num_waiters waiters,
//        'Wait Event: ' ||wait_event_text wait_event, 'P1: '||p1 p1, 'P2:'||p2 p2, 'P3: '||p3 p3,
//        'Seconds in Wait: '||in_wait_secs Seconds,
//        'SecondsSince Last Wait: '||time_since_last_wait_secs sincelw,
//        'Wait Chain:'||chain_id ||': '||chain_signature chain_signature,
//        'Blocking Wait Chain:'||decode(blocker_chain_id,null, '',blocker_chain_id) blocker_chain,
//        o.owner ||'.'||o.object_name,
//        ROW_WAIT_OBJ#,
//        ROW_WAIT_ROW#
//FROM   V$WAIT_CHAINS  a
//LEFT OUTER JOIN DBA_OBJECTS o ON(o.object_id = a.ROW_WAIT_OBJ#)
//CONNECT BY 1=1
//AND    PRIOR a.instance     = a.blocker_instance
//AND    PRIOR a.osid         = a.blocker_osid
//AND    PRIOR a.pid          = a.blocker_pid
//AND    PRIOR a.sid          = a.blocker_sid
//AND    PRIOR a.sess_serial# = a.blocker_sess_serial#
//START WITH a.blocker_is_valid='FALSE'
//ORDER  BY a.chain_id , LEVEL
