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

#include <stdio.h>
#include <map>

#ifdef WIN32
#include <time.h>
#endif

#include <qmessagebox.h>

#include "tomain.h"
#include "toresultplan.h"
#include "toconnection.h"
#include "toconf.h"
#include "totool.h"
#include "tosql.h"

#include "toresultplan.moc"

toResultPlan::toResultPlan(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
{
  setAllColumnsShowFocus(true);
  setSorting(-1);
  setRootIsDecorated(true);
  addColumn("#");
  addColumn("Operation");
  addColumn("Options");
  addColumn("Object name");
  addColumn("Mode");
  addColumn("Cost");
  addColumn("Bytes");
  addColumn("Cardinality");
  setSQLName("toResultPlan");
}

static toSQL SQLViewPlan("toResultPlan:ViewPlan",
			 "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer, to_char(Cost), to_char(Bytes), to_char(Cardinality)\n"
			 "  FROM %1 WHERE Statement_ID = 'Tora %2' ORDER BY NVL(Parent_ID,0),ID",
			 "Get the contents of a plan table. Observe the %1 and %2 which must be present. Must return same columns");

void toResultPlan::query(const QString &sql,
			 const toQList &param)
{
  if (!handled())
    return;

  clear();

  QString planTable=toTool::globalConfig(CONF_PLAN_TABLE,DEFAULT_PLAN_TABLE);

  try {
    QString chkPoint=toTool::globalConfig(CONF_PLAN_CHECKPOINT,DEFAULT_PLAN_CHECKPOINT);

    toConnection &conn=connection();

    conn.execute(QString("SAVEPOINT %1").arg(chkPoint));

    int ident=(int)time(NULL);

    QString explain=QString("EXPLAIN PLAN SET STATEMENT_ID = 'Tora %1' INTO %2 FOR %3").
      arg(ident).arg(planTable).arg(sql);
    conn.execute(explain);

    {
      std::map <QString,QListViewItem *> parents;
      std::map <QString,QListViewItem *> last;
      QListViewItem *lastTop=NULL;
      toQuery query(conn,toSQL::string(SQLViewPlan,conn).arg(planTable).arg(ident));
      while(!query.eof()) {
	QString id=query.readValueNull();
	QString parentid=query.readValueNull();
	QString operation=query.readValueNull();
	QString options=query.readValueNull();
	QString object=query.readValueNull();
	QString optimizer=query.readValueNull();
	QString cost=query.readValueNull();
	QString bytes=query.readValueNull();
	QString cardinality=query.readValueNull();

	QListViewItem *item;
	if (parentid&&parents[parentid]) {
	  item=new QListViewItem(parents[parentid],last[parentid],
				 id,
				 operation,
				 options,
				 object,
				 optimizer,
				 cost,
				 bytes,
				 cardinality);
	  setOpen(parents[parentid],true);
	  parents[id]=item;
	  last[parentid]=item;
	} else {
	  item=new QListViewItem(this,lastTop,
				 id,
				 operation,
				 options,
				 object,
				 optimizer,
				 cost,
				 bytes,
				 cardinality);
	  parents[id]=item;
	  lastTop=item;
	}
      }
    }

    conn.execute(QString("ROLLBACK TO SAVEPOINT %1").arg(chkPoint));
  } catch (const QString &str) {
    try {
      if (str.contains("2404")) {
	int ret=TOMessageBox::warning(this,
				      "Plan table doesn't exist",
				      QString("Specified plan table %1 didn't exist.\n"
					      "Should TOra try to create it?").arg(planTable),
				      "&Yes","&No",0,1);
	if (ret==0) {
	  connection().execute(toSQL::string(toSQL::TOSQL_CREATEPLAN,
					     connection()).arg(planTable));
	  query(sql,param);
	}
      } else
	throw;
    } TOCATCH
  }
  updateContents();
}
