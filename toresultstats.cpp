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
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include "tomain.h"
#include "toresultstats.h"
#include "tosql.h"

#include "toresultstats.moc"

toResultStats::toResultStats(bool onlyChanged,int ses,toConnection &conn,QWidget *parent,
			     const char *name)
  : toResultView(false,false,conn,parent,name),OnlyChanged(onlyChanged)
{
  SessionID=ses;
  setSQLName("toResultStats");
  setup();
}

static toSQL SQLSession("toResultStats:Session",
			"SELECT MIN(SID) FROM V$MYSTAT",
			"Get session id of current session");

toResultStats::toResultStats(bool onlyChanged,toConnection &conn,QWidget *parent,
			     const char *name)
  : toResultView(false,false,conn,parent,name),OnlyChanged(onlyChanged)
{
  try {
    otl_stream str(1,
		   SQLSession(Connection),
		   Connection.connection());
    str>>SessionID;
  } catch (otl_exc &) {
    SessionID=-1;
  }
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
			   "SELECT Statistic#,Value FROM V$SesStat WHERE SID = :f1<int>",
			   "Get statistics for session, must have same number of columns");
static toSQL SQLSessionIO("toResultStats:SessionIO",
			  "SELECT Block_Gets \"block gets\",\n"
			  "       Block_Changes \"block changes\",\n"
			  "       Consistent_Changes \"consistent changes\"\n"
			  "  FROM v$sess_io\n"
			  " WHERE SID = :f1<int>",
			  "Get session IO, must have same binds");

void toResultStats::resetStats(void)
{
  try {
    otl_stream str(1,
		   SQLStatistics(Connection),
		   Connection.connection());
    str<<SessionID;
    while(!str.eof()) {
      int id;
      double value;
      str>>id;
      str>>value;
      id+=TO_STAT_BLOCKS;
      if (id<TO_STAT_MAX+TO_STAT_BLOCKS)
	LastValues[id]=value;
    }
    otl_stream sesio(1,
		     SQLSessionIO(Connection),
		     Connection.connection());
    sesio<<SessionID;
    int id=0;
    while(!sesio.eof()) {
      double value;
      sesio>>value;
      LastValues[id]=value;
      id++;
    }
  } TOCATCH
}

void toResultStats::changeSession(otl_connect &conn)
{
  try {
    otl_stream str(1,
		   SQLSession(Connection),
		   conn);
    str>>SessionID;
    resetStats();
    refreshStats(true);
  } TOCATCH
}

void toResultStats::changeSession(int ses)
{
  if (SessionID!=ses) {
    SessionID=ses;
    resetStats();
  }
  refreshStats();
}

static toSQL SQLStatisticName("toResultStats:StatisticName",
			      "SELECT b.Name,a.Statistic#,a.Value\n"
			      "  FROM V$SesStat a,V$StatName b\n"
			      " WHERE a.SID = :f1<int> AND a.Statistic# = b.Statistic#\n"
			      " ORDER BY UPPER(b.Name) Desc",
			      "Get statistics and their names for session, must have same number of columns");

void toResultStats::refreshStats(bool reset)
{
  try {
    clear();
    otl_stream str(1,
		   SQLStatisticName(Connection),
		   Connection.connection());
    otl_stream sesio(1,
		     SQLSessionIO(Connection),
		     Connection.connection());
    sesio<<SessionID;

    str<<SessionID;
    int SesID=0;
    int Row=0;
    while(!str.eof()||!sesio.eof()) {
      int id;
      double value;
      char buffer[65];
      if (!sesio.eof()) {
	int len;
	otl_column_desc *description=sesio.describe_select(len);
	strcpy(buffer,description[SesID%len].name);
	id=SesID;
	SesID++;
	sesio>>value;
      } else {
	str>>buffer;
	str>>id;
	id+=TO_STAT_BLOCKS;
	str>>value;
      }
      QString delta;
      QString absVal;
      toResultViewItem *last=NULL;
      setQueryColumns(3);

      if (value!=0) {
	absVal.sprintf("%g",value);
	if (id<TO_STAT_MAX+TO_STAT_BLOCKS) {
	  delta.sprintf("%g",value-LastValues[id]);
	  toResultViewItem *item;
	  if (value!=LastValues[id]||!OnlyChanged) {
	    item=new toResultViewItem(this,last);
	    if (reset)
	      LastValues[id]=value;
	    item->setText(0,QString::fromUtf8(buffer));
	    if (OnlyChanged)
	      item->setText(1,delta);
	    else {
	      item->setText(1,absVal);
	      item->setText(2,delta);
	    }
	    item->setText(3,QString::number(++Row));
	    last=item;
	  }
	}
      }
    }
  } TOCATCH
}
