//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include <time.h>

#include <map>

#include <qmessagebox.h>

#include "tomain.h"
#include "toresultplan.h"
#include "toconnection.h"
#include "toconf.h"
#include "totool.h"
#include "tosql.h"
#include "tonoblockquery.h"

#include "toresultplan.moc"

toResultPlan::toResultPlan(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
{
  setSQLName("toResultPlan");
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
  Query=NULL;
}

static toSQL SQLViewPlan("toResultPlan:ViewPlan",
			 "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer, to_char(Cost), to_char(Bytes), to_char(Cardinality)\n"
			 "  FROM %1 WHERE Statement_ID = 'Tora %2' ORDER BY NVL(Parent_ID,0),ID",
			 "Get the contents of a plan table. Observe the %1 and %2 which must be present. Must return same columns");

bool toResultPlan::canHandle(toConnection &conn)
{
  return toIsOracle(conn)||conn.provider()=="MySQL";
}

toResultPlan::~toResultPlan()
{
  delete Query;
}

void toResultPlan::query(const QString &sql,
			 const toQList &param)
{
  if (!handled())
    return;

  if (!setSQLParams(sql,param))
    return;

  if (Query) {
    delete Query;
    Query=NULL;
  }
  if (connection().provider()=="MySQL") {
    setRootIsDecorated(false);
    setSorting(0);
    toResultView::query("EXPLAIN "+toSQLStripBind(sql),param);
    return;
  }

  clear();

  while(columns()>0) {
    removeColumn(0);
  }
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

  QString planTable=toTool::globalConfig(CONF_PLAN_TABLE,DEFAULT_PLAN_TABLE);

  try {
    QString chkPoint=toTool::globalConfig(CONF_PLAN_CHECKPOINT,DEFAULT_PLAN_CHECKPOINT);

    toConnection &conn=connection();

    conn.execute(QString("SAVEPOINT %1").arg(chkPoint));

    Ident=(int)time(NULL);

    QString explain=QString("EXPLAIN PLAN SET STATEMENT_ID = 'Tora %1' INTO %2 FOR %3").
      arg(Ident).arg(planTable).arg(toSQLStripSpecifier(sql));
    Reading=false;
    toQList par;
    Query=new toNoBlockQuery(conn,toQuery::Normal,explain,par);
    Parents.clear();
    Last.clear();
    Poll.start(100);

    LastTop=NULL;

  } catch (const QString &str) {
    checkException(str);
  }
  updateContents();
}

void toResultPlan::poll(void)
{
  try {
    if (Query&&Query->poll()) {
      if (!Reading) {
	toQList par;
	delete Query;
	Query=NULL;
	Query=new toNoBlockQuery(connection(),toQuery::Normal,
				 toSQL::string(SQLViewPlan,connection()).
				 arg(toTool::globalConfig(CONF_PLAN_TABLE,DEFAULT_PLAN_TABLE)).
				 arg(Ident),par);
	Reading=true;
      } else {
	while(Query->poll()&&!Query->eof()) {
	  QString id=Query->readValueNull();
	  QString parentid=Query->readValueNull();
	  QString operation=Query->readValueNull();
	  QString options=Query->readValueNull();
	  QString object=Query->readValueNull();
	  QString optimizer=Query->readValueNull();
	  QString cost=Query->readValueNull();
	  QString bytes=Query->readValueNull();
	  QString cardinality=Query->readValueNull();

	  QListViewItem *item;
	  if (parentid&&Parents[parentid]) {
	    item=new QListViewItem(Parents[parentid],Last[parentid],
				   id,
				   operation,
				   options,
				   object,
				   optimizer,
				   cost,
				   bytes,
				   cardinality);
	    setOpen(Parents[parentid],true);
	    Parents[id]=item;
	    Last[parentid]=item;
	  } else {
	    item=new QListViewItem(this,LastTop,
				   id,
				   operation,
				   options,
				   object,
				   optimizer,
				   cost,
				   bytes,
				   cardinality);
	    Parents[id]=item;
	    LastTop=item;
	  }
	}
	if (Query->eof()) {
	  delete Query;
	  Query=NULL;
	  Poll.stop();
	  QString chkPoint=toTool::globalConfig(CONF_PLAN_CHECKPOINT,DEFAULT_PLAN_CHECKPOINT);
	  connection().execute(QString("ROLLBACK TO SAVEPOINT %1").arg(chkPoint));
	}
      }
    }
  } catch (const QString &str) {
    delete Query;
    Query=NULL;
    Poll.stop();
    checkException(str);
  }
}

void toResultPlan::checkException(const QString &str)
{
  try {
    if (str.startsWith("ORA-02404")) {
      QString planTable=toTool::globalConfig(CONF_PLAN_TABLE,DEFAULT_PLAN_TABLE);
      int ret=TOMessageBox::warning(this,
				    "Plan table doesn't exist",
				    QString("Specified plan table %1 didn't exist.\n"
					    "Should TOra try to create it?").arg(planTable),
				    "&Yes","&No",0,1);
      if (ret==0) {
	connection().execute(toSQL::string(toSQL::TOSQL_CREATEPLAN,
					   connection()).arg(planTable));
	QString t=sql();
	setSQL(QString::null);
	query(t,params());
      }
    } else
      toStatusMessage(str);
  } TOCATCH
}
