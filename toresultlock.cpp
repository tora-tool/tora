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


#include <stdio.h>
#include <map>

#include "toresultlock.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"

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
}

QString toResultLock::query(const QString &sql,
			    const list<QString> &param)
{
  clear();

  try {
    QString chkPoint=toTool::globalConfig(CONF_PLAN_CHECKPOINT,DEFAULT_PLAN_CHECKPOINT);

    otl_stream query(1,
		     "select TO_CHAR(b.sid),"
		     "       a.id1||':'||a.id2,"
		     "       b.schemaname,"
		     "       b.osuser,"
		     "       b.program,"
		     "       DECODE(e.type,'TM','DML enqueue','TX','Transaction enqueue','UL','User supplied','Internal ('||a.type||')'),"
		     "       DECODE(a.lmode,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',TO_CHAR(a.lmode)),"
		     "       DECODE(e.request,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',TO_CHAR(e.request)),"
		     "       d.object_name,"
		     "       TO_CHAR(SYSDATE-a.CTIME/3600/24),"
		     "       TO_CHAR(SYSDATE-e.CTIME/3600/24)"
		     "  from v$lock a, v$session b,v$locked_object c,all_objects d,v$lock e"
		     " where a.sid = b.sid"
		     "   and a.lmode != 0"
		     "   and c.session_id = a.sid"
		     "   and c.object_id = d.object_id"
		     "   and exists (select 'X'"
		     "                 from v$locked_object bb,"
		     "                      v$lock cc"
		     "                where bb.session_id = cc.sid"
		     "                  and cc.sid != a.sid"
		     "                  and cc.id1 = a.id1"
		     "                  and cc.id2 = a.id2"
		     "                  and bb.object_id = c.object_id)"
		     "   and a.id1 = e.id1"
		     "   and a.id2 = e.id2"
		     "   and (e.id1,e.id2,e.sid) in (select aa.id1,aa.id2,aa.sid from v$lock aa where aa.sid = :f1<char[31]> and aa.lmode != aa.request and aa.request != 0)",
		     Connection.connection());

    {
      toResultViewItem *lastItem=NULL;
      QString session=sql;
      query<<session;
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
	  item->setText(pos,buffer);
	}
	session=item->text(0);
	query<<session;
      }
    }
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
    return QString((const char *)exc.msg);
  } catch (const QString &str) {
    toStatusMessage((const char *)str);
    return str;
  }
  updateContents();
  return "";
}
