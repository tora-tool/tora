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

TO_NAMESPACE;

#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qlabel.h>

#include "toconnection.h"
#include "tomain.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"
#include "totuning.h"
#include "toresultstats.h"

#include "totuning.moc"

#include "icons/refresh.xpm"
#include "icons/totuning.xpm"

static toSQL SQLDictionaryMiss("toTuning:Important ratios:1DictionaryMiss",
			       "select to_char(round(sum(getmisses)/sum(gets)*100,2))||' %' from v$rowcache",
			       "Data dictionary row cache miss ratio (%). < 10%-15%");

static toSQL SQLLibraryCache("toTuning:Important ratios:2LibraryCache",
			     "select to_char(round(sum(reloads)/sum(pins)*100,2))||' %' from v$librarycache",
			     "Library cache (Shared SQL) miss ratio (%). < 1%");

static toSQL SQLDataCache("toTuning:Important ratios:3DataCache",
			  "SELECT TO_CHAR(ROUND((1-(physread/(dbblocks+consist)))*100,2))||' %'\n"
			  "  FROM (SELECT MAX(value) dbblocks FROM v$sysstat WHERE name = 'db block gets') a,\n"
			  "       (SELECT MAX(value) consist FROM v$sysstat WHERE name = 'consistent gets') b,\n"
			  "       (SELECT MAX(value) physread FROM v$sysstat WHERE name = 'physical reads') c",
			  "Data buffer cache hit ratio (%). > 60% - 70%");

static toSQL SQLLogRedo("toTuning:Redo log contention:1LogSpace",
			"select value from v$sysstat where name='redo log space requests'",
			"Redo log space requests. Close to 0");

static toSQL SQLSystemHeadUndo("toTuning:RBS contention:1SystemHeadUndo",
			       "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			       "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo header') a,\n"
			       "       (SELECT SUM(value) blocks FROM v$sysstat WHERE name IN ('consistent gets','db block gets')) b",
			       "System undo header waits (%). < 1%");

static toSQL SQLSystemBlockUndo("toTuning:RBS contention:2SystemBlockUndo",
				"SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
				"  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'system undo block') a,\n"
				"       (SELECT SUM(value) blocks FROM v$sysstat WHERE name IN ('consistent gets','db block gets')) b",
				"System undo block waits (%). < 1%");

static toSQL SQLHeadUndo("toTuning:RBS contention:3HeadUndo",
			 "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			 "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo header') a,\n"
			 "       (SELECT SUM(value) blocks FROM v$sysstat WHERE name IN ('consistent gets','db block gets')) b",
			 "Undo head waits (%). < 1%");

static toSQL SQLBlockUndo("toTuning:RBS contention:4BlockUndo",
			  "SELECT TO_CHAR(ROUND(count/blocks*100,2))||' %'\n"
			  "  FROM (SELECT MAX(count) count FROM v$waitstat WHERE class = 'undo block') a,\n"
			  "       (SELECT SUM(value) blocks FROM v$sysstat WHERE name IN ('consistent gets','db block gets')) b",
			  "Undo block waits (%). < 1%");

static toSQL SQLTotalWaits("toTuning:RBS contention:5TotalWaits",
			  "select to_char(round(sum(waits)/sum(gets)*100,2))||' %' from v$rollstat",
			  "Total RBS waits (%). < 1%");

static toSQL SQLBusyRate("toTuning:MTS contention:1BusyRate",
			 "SELECT TO_CHAR(ROUND(NVL(SUM(busy)/(SUM(busy)+SUM(idle)),0)*100,2))||' %' FROM v$dispatcher",
			 "Busy rate for dispatchers (%). < 50%");

static toSQL SQLDispQueue("toTuning:MTS contention:2DispQueue",
			  "SELECT DECODE(NVL(SUM(totalq),0),0, 'No responses', ROUND(SUM(wait)/SUM(totalq),3)) FROM v$queue WHERE type='DISPATCHER'",
			  "Dispatcher response queues waits (1/100s).");

static toSQL SQLServerQueue("toTuning:MTS contention:3ServerQueue",
			    "SELECT DECODE (NVL(SUM(totalq),0),0, 'No requests', ROUND(SUM(wait)/SUM(totalq),3)) FROM v$queue WHERE type='COMMON'",
			    "Server request queue waits (1/100s).");

static toSQL SQLParameters("toTuning:Parameters",
			   "select name,value,description from v$parameter",
			   "Display parameters of Oracle server");

class toTuningTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return totuning_xpm; }
public:
  toTuningTool()
    : toTool(10,"Server Tuning")
  { }
  virtual const char *menuItem()
  { return "Server Tuning"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    QWidget *window=new toTuning(parent,connection);
    window->setIcon(*toolbarImage());
    return window;
  }
};

static toTuningTool TuningTool;

static QPixmap *toRefreshPixmap;

toTuning::toTuning(QWidget *main,toConnection &connection)
  : toToolWidget("tuning.html",main,connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);

  QToolBar *toolbar=toAllocBar(this,"Server Tuning",connection.connectString());

  new QToolButton(*toRefreshPixmap,
		  "Refresh",
		  "Refresh",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel("",toolbar));

  QTabWidget *Tabs=new QTabWidget(this);
  Indicators=new toListView(Tabs);
  Indicators->setRootIsDecorated(true);
  Indicators->addColumn("Indicator");
  Indicators->addColumn("Value");
  Indicators->addColumn("Reference");
  Tabs->addTab(Indicators,"Indicators");

  Statistics=new toResultStats(connection,Tabs);
  Tabs->addTab(Statistics,"Statistics");

  Parameters=new toResultView(true,false,connection,Tabs);
  Parameters->setSQL(SQLParameters);
  Tabs->addTab(Parameters,"Parameters");

  refresh();
}

void toTuning::refresh(void)
{
  Indicators->clear();
  list<QString> val=toSQL::range("toTuning");
  QListViewItem *parent=NULL;
  QListViewItem *last=NULL;
  for(list<QString>::iterator i=val.begin();i!=val.end();i++) {
    try {
      list<QString> val=toReadQuery(connection(),toSQL::sql(*i,connection()));
      QStringList parts=QStringList::split(":",*i);
      if (!parent||parent->text(0)!=parts[1]) {
	parent=new toResultViewItem(Indicators,NULL,parts[1]);
	parent->setOpen(true);
	last=NULL;
      }
      QStringList dsc=QStringList::split(".",toSQL::description(*i));
      QString first=dsc[0];
      first+=".";
      last=new toResultViewItem(parent,last,first);
      last->setText(1,*(val.begin()));
      if (dsc.count()>1)
	last->setText(2,dsc[1]);
    } TOCATCH
  }
  Statistics->refreshStats();
  Parameters->refresh();
}
