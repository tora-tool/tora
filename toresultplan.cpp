//***************************************************************************
/*
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
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

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
	QString id=query.readValue();
	QString parentid=query.readValue();
	QString operation=query.readValue();
	QString options=query.readValue();
	QString object=query.readValue();
	QString optimizer=query.readValue();
	QString cost=query.readValue();
	QString bytes=query.readValue();
	QString cardinality=query.readValue();

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
