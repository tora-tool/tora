
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

#include "result/toresultwaitchains.h"

#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "core/toqvalue.h"
#include "core/tosql.h"
#include "core/totool.h"
#include "core/utils.h"

#include <QWidget>
#include <QVBoxLayout>


toResultWaitChains::toResultWaitChains(QWidget *parent, const char *name)
    : ResutWaitSchains::MVC(parent)
    , Query(NULL)
{
    setSQLName(QString::fromLatin1("toResultWaitChains:Chains"));
}

toResultWaitChains::~toResultWaitChains()
{
}

static toSQL SQLLock("toResultWaitChains:Chains",
                     "SELECT                                                                                                                                    \n"
                     " 'Current Process: '||osid W_PROC,                                                                                                        \n"
                     " 'SID '||i.instance_name INSTANCE,                                                                                                        \n"
                     " 'INST #: '||instance INST,                                                                                                               \n"
                     " 'Blocking Process: '||decode(blocker_osid,null,'<none>',blocker_osid)||' from Instance '||blocker_instance BLOCKER_PROC,                 \n"
                     " 'Number of waiters: '||num_waiters waiters,                                                                                              \n"
                     " 'Final Blocking Process: '||decode(p.spid,null,'<none>', p.spid)||' from Instance '||s.final_blocking_instance FBLOCKER_PROC,            \n"
                     " 'Program: '||p.program image,                                                                                                            \n"
                     " 'Wait Event: ' ||wait_event_text wait_event,                                                                                             \n"
                     " 'P1: '||wc.p1 p1,                                                                                                                        \n"
                     " 'P2: '||wc.p2 p2,                                                                                                                        \n"
                     " 'P3: '||wc.p3 p3,                                                                                                                        \n"
                     " 'Seconds in Wait: '||in_wait_secs Seconds,                                                                                               \n"
                     " 'Seconds Since Last Wait: '||time_since_last_wait_secs sincelw,                                                                          \n"
                     " 'Wait Chain: '||chain_id ||': '||chain_signature chain_signature,                                                                        \n"
                     " 'Blocking Wait Chain: '||decode(blocker_chain_id,null,'<none>',blocker_chain_id) blocker_chain                                           \n"
                     "FROM v$wait_chains wc,                                                                                                                    \n"
                     " gv$session s,                                                                                                                            \n"
                     " gv$session bs,                                                                                                                           \n"
                     " gv$instance i,                                                                                                                           \n"
                     " gv$process p                                                                                                                             \n"
                     "WHERE wc.instance = i.instance_number (+)                                                                                                 \n"
                     " AND (wc.instance = s.inst_id (+) and wc.sid = s.sid (+)                                                                                  \n"
                     " AND wc.sess_serial# = s.serial# (+))                                                                                                     \n"
                     " AND (s.final_blocking_instance = bs.inst_id (+) and s.final_blocking_session = bs.sid (+))                                               \n"
                     " AND (bs.inst_id = p.inst_id (+) and bs.paddr = p.addr (+))                                                                               \n"
                     " AND ( num_waiters > 0 OR ( blocker_osid IS NOT NULL AND in_wait_secs > 10 ) )                                                            \n"
                     "ORDER BY chain_id, num_waiters DESC                                                                                                       \n"
                     ,
                     "List session blockers and waiters");

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
