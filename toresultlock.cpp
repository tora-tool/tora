//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <map>

#include "toresultlock.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"
#include "tosql.h"
#include "toconnection.h"

#include "toresultlock.moc"


toResultLock::toResultLock(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
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
		     "   and (e.id1,e.id2,e.sid) in (select aa.id1,aa.id2,aa.sid from v$lock aa where aa.sid = :f1<char[101]> and aa.lmode != aa.request and aa.request != 0)",
		     "List locks in a session");

void toResultLock::query(const QString &sql,
			 const toQList &param)
{
  clear();

  toQuery *query=NULL;
  try {
    QString chkPoint=toTool::globalConfig(CONF_PLAN_CHECKPOINT,DEFAULT_PLAN_CHECKPOINT);

    query=new toQuery(connection(),SQLLock,sql);

    {
      toResultViewItem *lastItem=NULL;
      while(!query->eof()) {
	toResultViewItem *item;
	if (!lastItem)
	  item=new toResultViewItem(this,NULL);
	else {
	  item=new toResultViewItem(lastItem,NULL);
	  setOpen(lastItem,true);
	}
	lastItem=item;
	for (int pos=0;!query->eof();pos++)
	  item->setText(pos,query->readValue());
	QString session=item->text(0);
	delete query;
	query=new toQuery(connection(),SQLLock,session);
      }
    }
  } catch(const QString &exc) {
    delete query;
    toStatusMessage(exc);
  }
  updateContents();
}
