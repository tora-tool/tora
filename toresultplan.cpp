//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003 Quest Software, Inc
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tonoblockquery.h"
#include "toresultplan.h"
#include "tosql.h"
#include "totool.h"

#include <time.h>

#include <map>

#include <qmessagebox.h>

#include "toresultplan.moc"

toResultPlan::toResultPlan(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
{
  setSQLName(QString::fromLatin1("toResultPlan"));
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
  Query=NULL;
  oracleSetup();
}

static toSQL SQLViewPlan("toResultPlan:ViewPlan",
			 "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer, to_char(Cost), to_char(Bytes), to_char(Cardinality)\n"
			 "  FROM %1 WHERE Statement_ID = '%2' ORDER BY NVL(Parent_ID,0),ID",
			 "Get the contents of a plan table. Observe the %1 and %2 which must be present. Must return same columns");

bool toResultPlan::canHandle(toConnection &conn)
{
  return toIsOracle(conn)||toIsMySQL(conn);
}

toResultPlan::~toResultPlan()
{
  delete Query;
}

void toResultPlan::oracleSetup(void)
{
  clear();

  while(columns()>0) {
    removeColumn(0);
  }
  setAllColumnsShowFocus(true);
  setSorting(-1);
  setRootIsDecorated(true);
  addColumn(QString::fromLatin1("#"));
  addColumn(tr("Operation"));
  addColumn(tr("Options"));
  addColumn(tr("Object name"));
  addColumn(tr("Mode"));
  addColumn(tr("Cost"));
  addColumn(tr("Bytes"));
  addColumn(tr("Cardinality"));
  setColumnAlignment(5,AlignRight);
  setColumnAlignment(6,AlignRight);
  setColumnAlignment(7,AlignRight);
}

void toResultPlan::oracleNext(void)
{
  LastTop=NULL;
  Parents.clear();
  Last.clear();

  QString chkPoint=toTool::globalConfig(CONF_PLAN_CHECKPOINT,DEFAULT_PLAN_CHECKPOINT);
  
  toConnection &conn=connection();
  
  conn.execute(QString::fromLatin1("SAVEPOINT %1").arg(chkPoint));
  
  Ident=QString::fromLatin1("TOra ")+QString::number((int)time(NULL)+rand());
  
  QString planTable=toTool::globalConfig(CONF_PLAN_TABLE,DEFAULT_PLAN_TABLE);
  
  QString sql=toShift(Statements);
  if (sql.isNull()) {
    Poll.stop();
    return;
  }
  if (sql.length()>0&&sql.at(sql.length()-1).latin1()==';')
    sql=sql.mid(0,sql.length()-1);

  QString explain=QString::fromLatin1("EXPLAIN PLAN SET STATEMENT_ID = '%1' INTO %2 FOR %3").
    arg(Ident).arg(planTable).arg(toSQLStripSpecifier(sql));
  if (!User.isNull()&&User!=conn.user().upper()) {
    try {
      conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = %1").arg(User));
    } catch(...) {
    }
    try {
      conn.execute(explain);
    } catch(...) {
      try {
	conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = %2").arg(connection().user()));
      } catch(...) {
      }
      throw;
    }
    conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = %2").arg(connection().user()));
    toQList par;
    Query=new toNoBlockQuery(connection(),toQuery::Normal,
			     toSQL::string(SQLViewPlan,connection()).
			     arg(toTool::globalConfig(CONF_PLAN_TABLE,DEFAULT_PLAN_TABLE)).
			     arg(Ident),par);
    Reading=true;
  } else {
    Reading=false;
    toQList par;
    Query=new toNoBlockQuery(conn,toQuery::Normal,explain,par);
  }
  TopItem=new toResultViewItem(this,TopItem,QString::fromLatin1("DML"));
  TopItem->setText(1,sql);
  Poll.start(100);
}

static void StripInto(std::list<toSQLParse::statement> &stats)
{
  std::list<toSQLParse::statement> res;
  bool into=false;
  bool add=true;
  for(std::list<toSQLParse::statement>::iterator i=stats.begin();i!=stats.end();i++) {
    if(into) {
      if(!add&&(*i).String.upper()==QString::fromLatin1("FROM"))
	add=true;
    } else if ((*i).String.upper()==QString::fromLatin1("INTO")) {
      add=false;
      into=true;
    }
    if (add)
      res.insert(res.end(),*i);
  }
  stats=res;
}

void toResultPlan::addStatements(std::list<toSQLParse::statement> &stats)
{
  for(std::list<toSQLParse::statement>::iterator i=stats.begin();i!=stats.end();i++) {
    if ((*i).Type==toSQLParse::statement::Block)
      addStatements((*i).subTokens());
    else if ((*i).Type==toSQLParse::statement::Statement) {
      if ((*i).subTokens().begin()!=(*i).subTokens().end()) {
	QString t=(*((*i).subTokens().begin())).String.upper();
	if (t==QString::fromLatin1("SELECT"))
	  StripInto((*i).subTokens());

	if (t==QString::fromLatin1("SELECT")||
	    t==QString::fromLatin1("INSERT")||
	    t==QString::fromLatin1("UPDATE")||
	    t==QString::fromLatin1("DELETE"))
	  Statements.insert(Statements.end(),
			    toSQLParse::indentStatement(*i).stripWhiteSpace());
      }
    }
  }
}

void toResultPlan::query(const QString &sql,
			 const toQList &param)
{
  if (!handled())
    return;

  try {
    if (!setSQLParams(sql,param))
      return;

    if (Query) {
      delete Query;
      Query=NULL;
    }
    if (connection().provider()=="MySQL") {
      setRootIsDecorated(false);
      setSorting(0);
      toResultView::query(QString::fromLatin1("EXPLAIN ")+toSQLStripBind(sql),param);
      return;
    }

    toQList::iterator cp=((toQList &)param).begin();
    if (cp!=((toQList &)param).end())
      User=*cp;
    else
      User=QString::null;
    
    oracleSetup();

    QString planTable=toTool::globalConfig(CONF_PLAN_TABLE,DEFAULT_PLAN_TABLE);

    Statements.clear();
    if (sql.startsWith(QString::fromLatin1("SAVED:"))) {
      Ident=sql.mid(6);
      toQList par;
      Query=new toNoBlockQuery(connection(),toQuery::Background,
			       toSQL::string(SQLViewPlan,connection()).
			       arg(planTable).arg(Ident),
			       par);
      Reading=true;
      LastTop=NULL;
      Parents.clear();
      Last.clear();
      TopItem=new toResultViewItem(this,NULL,QString::fromLatin1("DML"));
      TopItem->setText(1,QString::fromLatin1("Saved plan"));
    } else {
      TopItem=NULL;
      std::list<toSQLParse::statement> ret=toSQLParse::parse(sql);
      addStatements(ret);
      oracleNext();
    }
  } catch (const QString &str) {
    checkException(str);
  }
}

void toResultPlan::poll(void)
{
  try {
    if (!toCheckModal(this))
      return;
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
	    item=new QListViewItem(TopItem,LastTop,
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
	  QString chkPoint=toTool::globalConfig(CONF_PLAN_CHECKPOINT,DEFAULT_PLAN_CHECKPOINT);
	  if (!sql().startsWith(QString::fromLatin1("SAVED:"))) {
	    if (toTool::globalConfig(CONF_KEEP_PLANS,"").isEmpty())
	      connection().execute(QString::fromLatin1("ROLLBACK TO SAVEPOINT %1").arg(chkPoint));
	    else
	      toMainWidget()->setNeedCommit(connection());
	  }
	  oracleNext();
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
    if (str.startsWith(QString::fromLatin1("ORA-02404"))) {
      QString planTable=toTool::globalConfig(CONF_PLAN_TABLE,DEFAULT_PLAN_TABLE);
      int ret=TOMessageBox::warning(this,
				    tr("Plan table doesn't exist"),
				    tr("Specified plan table %1 didn't exist.\n"
				       "Should TOra try to create it?").arg(planTable),
				    tr("&Yes"),tr("&No"),QString::null,0,1);
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
