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

#include "tomain.h"
#include "toresultstats.h"
#include "tosql.h"
#include "toconnection.h"

#include "toresultstats.moc"

toResultStats::toResultStats(bool onlyChanged,int ses,QWidget *parent,
			     const char *name)
  : toResultView(false,false,parent,name),OnlyChanged(onlyChanged)
{
  SessionID=ses;
  setSQLName("toResultStats");
  System=false;
  setup();
}

static toSQL SQLSession("toResultStats:Session",
			"SELECT MIN(SID) FROM V$MYSTAT",
			"Get session id of current session");

toResultStats::toResultStats(bool onlyChanged,QWidget *parent,
			     const char *name)
  : toResultView(false,false,parent,name),OnlyChanged(onlyChanged)
{
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
}

static toSQL SQLStatistics("toResultStats:Statistics",
			   "SELECT Statistic#,SUM(Value) FROM V$SesStat WHERE SID = :f1<int>\n"
			   " GROUP BY Statistic#",
			   "Get statistics for session, must have same number of columns");
static toSQL SQLSessionIO("toResultStats:SessionIO",
			  "SELECT SUM(Block_Gets) \"block gets\",\n"
			  "       SUM(Block_Changes) \"block changes\",\n"
			  "       SUM(Consistent_Changes) \"consistent changes\"\n"
			  "  FROM v$sess_io\n"
			  " WHERE SID = :f1<int>",
			  "Get session IO, must have same binds");
static toSQL SQLSystemStatistics("toResultStats:SystemStatistics",
				 "SELECT Statistic#,Value FROM v$sysstat",
				 "Get system statistics, must have same number of columns");

void toResultStats::resetStats(void)
{
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
			      "SELECT b.Name,a.Statistic#,SUM(a.Value)\n"
			      "  FROM V$SesStat a,V$StatName b\n"
			      " WHERE a.SID = :f1<int> AND a.statistic# = b.statistic#\n"
			      " GROUP BY b.name,a.Statistic#",
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
  toBusy busy;
  try {
    clear();
    Row=0;
    toConnection &conn=connection();
    toQList args;
    if (!System)
      args.insert(args.end(),SessionID);
    toQuery query(conn,System?SQLSystemStatisticName:SQLStatisticName,args);
    while(!query.eof()) {
      QString name=query.readValue();
      int id=query.readValue().toInt();
      double value=query.readValue().toInt();
      addValue(reset,id,name,value);
    }

    if (!System) {
      int id=0;
      toQuery query(conn,SQLSessionIO,args);
      toQDescList description=query.describe();
      toQDescList::iterator i=description.begin();
      while(!query.eof()) {
	addValue(reset,id,(*i).Name,query.readValue().toDouble());
	id++;
	if (i==description.end())
	  i=description.begin();
	else
	  i++;
      }
    }
  } TOCATCH
  updateContents();
}
