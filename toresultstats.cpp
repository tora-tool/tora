//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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

#include "tomain.h"
#include "toresultstats.h"
#include "tosql.h"
#include "toconnection.h"
#include "tonoblockquery.h"

#include "toresultstats.moc"

toResultStats::toResultStats(bool onlyChanged,int ses,QWidget *parent,
			     const char *name)
  : toResultView(false,false,parent,name),OnlyChanged(onlyChanged)
{
  if (!handled())
    return;

  SessionID=ses;
  setSQLName("toResultStats");
  System=false;
  setup();
}

toResultStats::~toResultStats()
{
  delete Query;
  delete SessionIO;
}

static toSQL SQLSession("toResultStats:Session",
			"SELECT MIN(SID) FROM V$MYSTAT",
			"Get session id of current session");

toResultStats::toResultStats(bool onlyChanged,QWidget *parent,
			     const char *name)
  : toResultView(false,false,parent,name),OnlyChanged(onlyChanged)
{
  if (!handled())
    return;

  try {
    toQuery query(connection(),SQLSession);
    SessionID=query.readValue().toInt();
  } catch (...) {
    SessionID=-1;
  }
  System=false;

  setup();
}

toResultStats::toResultStats(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name),OnlyChanged(false)
{
  if (!handled())
    return;

  System=true;
  setup();
}

void toResultStats::setup(void)
{
  for (int i=0;i<TO_STAT_MAX;i++)
    LastValues[i]=0;

  addColumn("Name");
  if (!OnlyChanged)
    addColumn("Value");
  addColumn("Delta");
  setSorting(0);

  setColumnAlignment(1,AlignRight);
  setColumnAlignment(2,AlignRight);

  Query=SessionIO=NULL;
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
}

static toSQL SQLStatistics("toResultStats:Statistics",
			   "SELECT Statistic#,Value FROM V$SesStat WHERE SID = :f1<int>",
			   "Get statistics for session, must have same number of columns");
static toSQL SQLSessionIO("toResultStats:SessionIO",
			  "SELECT Block_Gets \"block gets\",\n"
			  "       Block_Changes \"block changes\",\n"
			  "       Consistent_Changes \"consistent changes\"\n"
			  "  FROM v$sess_io\n"
			  " WHERE SID = :f1<int>",
			  "Get session IO, must have same binds");
static toSQL SQLSystemStatistics("toResultStats:SystemStatistics",
				 "SELECT Statistic#,Value FROM v$sysstat",
				 "Get system statistics, must have same number of columns");

void toResultStats::resetStats(void)
{
  if (!handled())
    return;

  toBusy busy;
  try {
    toConnection &conn=connection();
    toQList args;
    if (!System)
      args.insert(args.end(),SessionID);
    toQuery query(conn,System?SQLSystemStatistics:SQLStatistics,args);
    while(!query.eof()) {
      int id;
      double value;
      id=query.readValue().toInt();
      value=query.readValue().toDouble();
      id+=TO_STAT_BLOCKS;
      if (id<TO_STAT_MAX+TO_STAT_BLOCKS)
	LastValues[id]=value;
    }
    if (!System) {
      toQuery queryio(conn,SQLSessionIO,args);
      int id=0;
      while(!queryio.eof()) {
	double value;
	value=queryio.readValue().toDouble();
	LastValues[id]=value;
	id++;
      }
    }
  } TOCATCH
}

void toResultStats::changeSession(toQuery &query)
{
  if (!handled())
    return;

  if (System)
    throw QString("Can't change session on system statistics");
  try {
    toQList args;
    query.execute(SQLSession,args);
    SessionID=query.readValue().toInt();
    emit sessionChanged(SessionID);
    emit sessionChanged(QString::number(SessionID));
    resetStats();
    refreshStats(true);
  } TOCATCH
}

void toResultStats::changeSession(int ses)
{
  if (!handled())
    return;

  if (System)
    throw QString("Can't change session on system statistics");
  if (SessionID!=ses) {
    SessionID=ses;
    emit sessionChanged(SessionID);
    emit sessionChanged(QString::number(SessionID));
    resetStats();
  }
  refreshStats();
}

static toSQL SQLStatisticName("toResultStats:StatisticName",
			      "SELECT b.Name,a.Statistic#,a.Value\n"
			      "  FROM V$SesStat a,V$StatName b\n"
			      " WHERE a.SID = :f1<int> AND a.statistic# = b.statistic#\n",
			      "Get statistics and their names for session, must have same number of columns");
static toSQL SQLSystemStatisticName("toResultStats:SystemStatisticName",
				    "SELECT Name,Statistic#,Value\n"
				    "  FROM v$sysstat\n",
				    "Get statistics and their names for system statistics, must have same number of columns");

void toResultStats::addValue(bool reset,int id,const QString &name,double value)
{
  QString delta;
  QString absVal;

  if (value!=0) {
    absVal.sprintf("%.15g",value);
    if (id<TO_STAT_MAX+TO_STAT_BLOCKS) {
      delta.sprintf("%.15g",value-LastValues[id]);
      if (value!=LastValues[id]||!OnlyChanged) {
	toResultViewItem *item=new toResultViewItem(this,NULL);
	if (reset)
	  LastValues[id]=value;
	item->setText(0,name);
	if (OnlyChanged)
	  item->setText(1,delta);
	else {
	  item->setText(1,absVal);
	  item->setText(2,delta);
	}
	item->setText(3,QString::number(++Row));
      }
    }
  }
}

void toResultStats::refreshStats(bool reset)
{
  if (!handled()||Query||SessionIO)
    return;

  try {
    clear();
    Row=0;
    toConnection &conn=connection();
    toQList args;
    if (!System)
      args.insert(args.end(),SessionID);
    Query=new toNoBlockQuery(conn,
			     toQuery::Normal,
			     toSQL::string(System?SQLSystemStatisticName:SQLStatisticName,
					   connection()),
			     args);
    if (!System)
      SessionIO=new toNoBlockQuery(conn,toQuery::Normal,
				   toSQL::string(SQLSessionIO,connection()),
				   args);
    Poll.start(100);
    Reset=reset;

  } TOCATCH
}

void toResultStats::poll(void)
{
  try {
    bool done=true;

    if (Query&&Query->poll()) {
      while(Query->poll()&&!Query->eof()) {
	QString name=Query->readValue();
	int id=Query->readValue().toInt();
	double value=Query->readValue().toDouble();
	addValue(Reset,id,name,value);
      }

      if (Query->eof()) {
	delete Query;
	Query=NULL;
      } else
	done=false;
    } else if (Query)
      done=false;

    if (SessionIO&&SessionIO->poll()) {
      int id=0;
      toQDescList description=SessionIO->describe();
      toQDescList::iterator i=description.begin();
      while(!SessionIO->eof()) {
	addValue(Reset,id,(*i).Name,SessionIO->readValue().toDouble());
	id++;
	if (i==description.end())
	  i=description.begin();
	else
	  i++;
      }
      delete SessionIO;
      SessionIO=NULL;
    } else
      done=false;

    if (done)
      Poll.stop();
  } TOCATCH
}
