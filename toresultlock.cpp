/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include <stdio.h>
#include <map>

#include "toresultlock.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"
#include "tosql.h"

#include "toresultlock.moc"


toResultLock::toResultLock(toConnection &conn,QWidget *parent,const char *name)
  : toResultView(false,false,conn,parent,name)
{
  setAllColumnsShowFocus(true);
  setSorting(-1);
  setRootIsDecorated(true);
  addColumn("Session");
  addColumn("Lock ID:s");
  addColumn("Schema");
  addColumn("Osuser");
  addColumn("Program");
  addColumn("Type");
  addColumn("Mode");
  addColumn("Request");
  addColumn("Object");
  addColumn("Grabbed");
  addColumn("Requested");
  setSQLName("toResultLock");
}

static toSQL SQLLock("toResultLock:Locks",
		     "select TO_CHAR(b.sid),\n"
		     "       a.id1||':'||a.id2,\n"
		     "       b.schemaname,\n"
		     "       b.osuser,\n"
		     "       b.program,\n"
		     "       DECODE(e.type,'TM','DML enqueue','TX','Transaction enqueue','UL','User supplied','Internal ('||a.type||')'),\n"
		     "       DECODE(a.lmode,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',TO_CHAR(a.lmode)),\n"
		     "       DECODE(e.request,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',TO_CHAR(e.request)),\n"
		     "       d.object_name,\n"
		     "       TO_CHAR(SYSDATE-a.CTIME/3600/24),\n"
		     "       TO_CHAR(SYSDATE-e.CTIME/3600/24)\n"
		     "  from v$lock a, v$session b,v$locked_object c,all_objects d,v$lock e\n"
		     " where a.sid = b.sid\n"
		     "   and a.lmode != 0\n"
		     "   and c.session_id = a.sid\n"
		     "   and c.object_id = d.object_id\n"
		     "   and exists (select 'X'\n"
		     "                 from v$locked_object bb,\n\n"
		     "                      v$lock cc\n"
		     "                where bb.session_id = cc.sid\n"
		     "                  and cc.sid != a.sid\n"
		     "                  and cc.id1 = a.id1\n"
		     "                  and cc.id2 = a.id2\n"
		     "                  and bb.object_id = c.object_id)\n"
		     "   and a.id1 = e.id1\n"
		     "   and a.id2 = e.id2\n"
		     "   and (e.id1,e.id2,e.sid) in (select aa.id1,aa.id2,aa.sid from v$lock aa where aa.sid = :f1<char[31]> and aa.lmode != aa.request and aa.request != 0)",
		     "List locks in a session");

void toResultLock::query(const QString &sql,
			 const list<QString> &param)
{
  clear();

  try {
    QString chkPoint=toTool::globalConfig(CONF_PLAN_CHECKPOINT,DEFAULT_PLAN_CHECKPOINT);

    otl_stream query(1,
		     SQLLock(Connection),
		     Connection.connection());

    {
      toResultViewItem *lastItem=NULL;
      QString session=sql;
      query<<session.utf8();
      while(!query.eof()) {
	toResultViewItem *item;
	if (!lastItem)
	  item=new toResultViewItem(this,NULL);
	else {
	  item=new toResultViewItem(lastItem,NULL);
	  setOpen(lastItem,true);
	}
	lastItem=item;
	for (int pos=0;!query.eof();pos++) {
	  char buffer[1024];
	  query>>buffer;
	  item->setText(pos,QString::fromUtf8(buffer));
	}
	session=item->text(0);
	query<<session.utf8();
      }
    }
  } TOCATCH
  updateContents();
}
