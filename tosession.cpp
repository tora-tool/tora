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

#include <qworkspace.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qsplitter.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qworkspace.h>
#include <qmenubar.h>

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include "tochangeconnection.h"
#include "tomain.h"
#include "totool.h"
#include "tosession.h"
#include "toresultview.h"
#include "toconf.h"
#include "tosql.h"
#include "tosgastatement.h"
#include "toresultstats.h"
#include "toresultlock.h"
#include "toresultbar.h"
#include "toconnection.h"
#include "toresultlong.h"

#include "tosession.moc"

#include "icons/refresh.xpm"
#include "icons/tosession.xpm"
#include "icons/clock.xpm"
#include "icons/noclock.xpm"
#include "icons/disconnect.xpm"

class toSessionTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return tosession_xpm; }
public:
  toSessionTool()
    : toTool(101,"Sessions")
  { }
  virtual const char *menuItem()
  { return "Sessions"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toSession(parent,connection);
  }
};

static toSessionTool SessionTool;

static toSQL SQLConnectInfo("toSession:ConnectInfo",
			    "select authentication_type,osuser,network_service_banner\n"
			    "  from v$session_connect_info where sid = :f1<char[101]>",
			    "Get connection info for a session");
static toSQL SQLLockedObject("toSession:LockedObject",
			     "select b.Object_Name \"Object Name\",\n"
			     "       b.Object_Type \"Type\",\n"
			     "       DECODE(a.locked_mode,0,'None',1,'Null',2,'Row-S',\n"
			     "                            3,'Row-X',4,'Share',5,'S/Row-X',\n"
			     "                            6,'Exclusive',a.Locked_Mode) \"Locked Mode\"\n"
			     "  from v$locked_object a,sys.all_objects b\n"
			     " where a.object_id = b.object_id\n"
			     "   and a.session_id = :f1<char[101]>",
			     "Display info about objects locked by this session");
static toSQL SQLOpenCursors("toSession:OpenCursor",
			    "select SQL_Text \"SQL\", Address||':'||Hash_Value \" Address\"\n"
			    "  from v$open_cursor where sid = :f1<char[101]>",
			    "Display open cursors of this session");
static toSQL SQLSessionWait(TO_SESSION_WAIT,
			    "select sysdate,\n"
			    "       cpu \"CPU\",\n"
			    "       parallel \"Parallel execution\",\n"
			    "       filewrite \"DB File Write\",\n"
			    "       writecomplete \"Write Complete\",\n"
			    "       fileread \"DB File Read\",\n"
			    "       singleread \"DB Single File Read\",\n"
			    "       control \"Control File I/O\",\n"
			    "       direct \"Direct I/O\",\n"
			    "       log \"Log file\",\n"
			    "       net \"SQL*Net\",\n"
			    "       total-parallel-filewrite-writecomplete-fileread-singleread-control-direct-log-net \"Other\"\n"
			    "  from (select SUM(DECODE(SUBSTR(event,1,2),'PX',time_waited,0))-SUM(DECODE(event,'PX Idle Wait',time_waited,0)) parallel,\n"
			    "               SUM(DECODE(event,'db file parallel write',time_waited,'db file single write',time_waited,0)) filewrite,\n"
			    "               SUM(DECODE(event,'write complete waits',time_waited,NULL)) writecomplete,\n"
			    "               SUM(DECODE(event,'db file parallel read',time_waited,'db file sequential read',time_waited,0)) fileread,\n"
			    "               SUM(DECODE(event,'db file scattered read',time_waited,0)) singleread,\n"
			    "               SUM(DECODE(SUBSTR(event,1,12),'control file',time_waited,0)) control,\n"
			    "               SUM(DECODE(SUBSTR(event,1,11),'direct path',time_waited,0)) direct,\n"
			    "               SUM(DECODE(SUBSTR(event,1,3),'log',time_waited,0)) log,\n"
			    "               SUM(DECODE(SUBSTR(event,1,7),'SQL*Net',time_waited,0))-SUM(DECODE(event,'SQL*Net message from client',time_waited,0)) net,\n"
			    "		    SUM(DECODE(event,'PX Idle Wait',0,'SQL*Net message from client',0,time_waited)) total\n"
			    "          from v$session_event where sid in (select b.sid from v$session a,v$session b where a.sid = :f1<char[101]> and a.audsid = b.audsid)),\n"
			    "       (select value*10 cpu from v$sesstat a\n"
			    "         where statistic# = 12 and a.sid in (select b.sid from v$session a,v$session b where a.sid = :f1<char[101]> and a.audsid = b.audsid))",
			    "Used to generate chart for session wait time.");
static toSQL SQLSessionIO(TO_SESSION_IO,
			  "select sysdate,\n"
			  "       sum(block_gets) \"Block gets\",\n"
			  "       sum(consistent_gets) \"Consistent gets\",\n"
			  "       sum(physical_reads) \"Physical reads\",\n"
			  "       sum(block_changes) \"Block changes\",\n"
			  "       sum(consistent_changes) \"Consistent changes\"\n"
			  "  from v$sess_io where sid in (select b.sid from v$session a,v$session b where a.sid = :f1<char[101]> and a.audsid = b.audsid)",
			  "Display chart of session generated I/O");

toSession::toSession(QWidget *main,toConnection &connection)
  : toToolWidget(SessionTool,"session.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,"Session manager",connection.description());

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Update sessionlist",
		  "Update sessionlist",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)clock_xpm),
		  "Enable timed statistics",
		  "Enable timed statistics",
		  this,SLOT(enableStatistics(void)),
		  toolbar);
  new QToolButton(QPixmap((const char **)noclock_xpm),
		  "Disable timed statistics",
		  "Disable timed statistics",
		  this,SLOT(disableStatistics(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)disconnect_xpm),
		  "Disconnect selected session",
		  "Disconnect selected session",
		  this,SLOT(disconnectSession(void)),
		  toolbar);
  toolbar->addSeparator();
  new QLabel("Refresh ",toolbar);
  connect(Refresh=toRefreshCreate(toolbar),SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));

  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  QSplitter *splitter=new QSplitter(Vertical,this);
  Sessions=new toResultLong(false,false,toQuery::Normal,splitter);
  Sessions->setReadAll(true);
  connect(Sessions,SIGNAL(done()),this,SLOT(done()));

  ResultTab=new QTabWidget(splitter);
  StatisticSplitter=new QSplitter(Horizontal,ResultTab);
  SessionStatistics=new toResultStats(false,0,StatisticSplitter);
  WaitBar=new toResultBar(StatisticSplitter);
  WaitBar->setSQL(SQLSessionWait);
  WaitBar->setTitle("Session wait states");
  WaitBar->setYPostfix("ms/s");
  IOBar=new toResultBar(StatisticSplitter);
  IOBar->setSQL(SQLSessionIO);
  IOBar->setTitle("Session I/O");
  IOBar->setYPostfix("blocks/s");
  ResultTab->addTab(StatisticSplitter,"Statistics");

  ConnectInfo=new toResultLong(true,false,toQuery::Normal,ResultTab);
  ConnectInfo->setSQL(SQLConnectInfo);
  ResultTab->addTab(ConnectInfo,"Connect Info");
  PendingLocks=new toResultLock(ResultTab);
  ResultTab->addTab(PendingLocks,"Pending Locks");
  LockedObjects=new toResultLong(false,false,toQuery::Normal,ResultTab);
  ResultTab->addTab(LockedObjects,"Locked Objects");
  LockedObjects->setSQL(SQLLockedObject);
  CurrentStatement=new toSGAStatement(ResultTab);
  ResultTab->addTab(CurrentStatement,"Current Statement");
  PreviousStatement=new toSGAStatement(ResultTab);
  ResultTab->addTab(PreviousStatement,"Previous Statement");

  OpenSplitter=new QSplitter(Horizontal,ResultTab);
  ResultTab->addTab(OpenSplitter,"Open Cursors");
  OpenCursors=new toResultLong(false,true,toQuery::Normal,OpenSplitter);
  OpenCursors->setSQL(SQLOpenCursors);
  OpenStatement=new toSGAStatement(OpenSplitter);

  connect(Sessions,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  connect(OpenCursors,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeCursor(QListViewItem *)));
  connect(ResultTab,SIGNAL(currentChanged(QWidget *)),
	  this,SLOT(changeTab(QWidget *)));

  connect(timer(),SIGNAL(timeout(void)),this,SLOT(refresh(void)));
  toRefreshParse(timer());
  CurrentTab=StatisticSplitter;
  CurrentItem=NULL;

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));
  refresh();
}

void toSession::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),"&Refresh",
			   this,SLOT(refresh(void)),Key_F5);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)clock_xpm),"Enable timed statistics",
			   this,SLOT(enableStatistics(void)));
      ToolMenu->insertItem(QPixmap((const char **)noclock_xpm),"Disable timed statistics",
			   this,SLOT(disableStatistics(void)));
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)disconnect_xpm),"Disconnect session",
			   this,SLOT(disconnectSession(void)));
      ToolMenu->insertSeparator();
      ToolMenu->insertItem("&Change Refresh",Refresh,SLOT(setFocus(void)),
			   Key_R+ALT);
      toMainWidget()->menuBar()->insertItem("&Session",ToolMenu,-1,toToolMenuIndex());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

static toSQL SQLSessions("toSession:ListSession",
			 "SELECT Sid \"-Id\",\n"
			 "       Serial# \"-Serial#\",\n"
			 "       SchemaName \"Schema\",\n"
			 "       Status \"Status\",\n"
			 "       Server \"Server\",\n"
			 "       OsUser \"Osuser\",\n"
			 "       Machine \"Machine\",\n"
			 "       Program \"Program\",\n"
			 "       Type \"Type\",\n"
			 "       Module \"Module\",\n"
			 "       Action \"Action\",\n"
			 "       Client_Info \"Client Info\",\n"
			 "       Process \"-Process\",\n"
			 "       SQL_Address||':'||SQL_Hash_Value \" SQL Address\",\n"
			 "       Prev_SQL_Addr||':'||Prev_Hash_Value \" Prev SQl Address\"\n"
			 "  FROM v$session\n"
			 " ORDER BY Sid",
			 "List sessions, must have same number of culumns and the last 2 must be "
			 "the same");			 

void toSession::refresh(void)
{
  if (CurrentItem) {
    Session=CurrentItem->text(0);
    Serial=CurrentItem->text(1);
  } else {
    Session=Serial=QString::null;
  }
  toQList par;
  Sessions->query(toSQL::string(SQLSessions,connection()),par);
}

void toSession::done(void)
{
  for (CurrentItem=Sessions->firstChild();CurrentItem;CurrentItem=CurrentItem->nextSibling())
    if (CurrentItem->text(0)==Session&&
	CurrentItem->text(1)==Serial) {
      Sessions->setSelected(CurrentItem,true);
      break;
    }
}

void toSession::enableStatistics(bool enable)
{
  QString sql;
  if (enable)
    sql="ALTER SYSTEM SET TIMED_STATISTICS = TRUE";
  else
    sql="ALTER SYSTEM SET TIMED_STATISTICS = FALSE";
  try {
    connection().execute(sql);
  } catch (...) {
    toStatusMessage("No access to timed statistics flags");
  }
}

void toSession::changeTab(QWidget *tab)
{
  CurrentTab=tab;
  if (CurrentItem) {
    if (CurrentTab==StatisticSplitter) {
      int ses=CurrentItem->text(0).toInt();
      SessionStatistics->changeSession(ses);
    } else if (CurrentTab==ConnectInfo)
      ConnectInfo->changeParams(CurrentItem->text(0));
    else if (CurrentTab==PendingLocks)
      PendingLocks->query(CurrentItem->text(0));
    else if (CurrentTab==OpenSplitter) {
      QListViewItem *item=OpenCursors->currentItem();
      QString address;
      if (item)
	address=item->text(2);
      OpenCursors->changeParams(CurrentItem->text(0));
      if (!address.isEmpty())
	for (QListViewItem *item=OpenCursors->firstChild();
	     item;item=item->nextSibling())
	  if (address==item->text(2)) {
	    OpenCursors->setSelected(item,true);
	    break;
	  }
    } else if (CurrentTab==CurrentStatement)
      CurrentStatement->changeAddress(CurrentItem->text(13));
    else if (CurrentTab==LockedObjects)
      LockedObjects->changeParams(CurrentItem->text(0));
    else if (CurrentTab==PreviousStatement)
      PreviousStatement->changeAddress(CurrentItem->text(14));
  }
}

void toSession::changeCursor(QListViewItem *item)
{
  if (item)
    OpenStatement->changeAddress(item->text(2));
}

void toSession::disconnectSession(void)
{
  if (CurrentItem) {
    QString sess="'";
    sess.append(CurrentItem->text(0));
    sess.append(",");
    sess.append(CurrentItem->text(1));
    sess.append("'");
    QString str("Let current transaction finnish before disconnecting session?");
    QString sql;
    switch(TOMessageBox::warning(this,"Commit work?",str,"&Yes","&No","&Cancel")) {
    case 0:
      sql="ALTER SYSTEM DISCONNECT SESSION ";
      sql.append(sess);
      sql.append(" POST_TRANSACTION");
      break;
    case 1:
      sql="ALTER SYSTEM KILL SESSION ";
      sql.append(sess);
      break;
    case 2:
      return;
    }
    try {
      connection().execute(sql);
    } TOCATCH
  }
}

void toSession::changeRefresh(const QString &str)
{
  toRefreshParse(timer(),str);
}

void toSession::changeItem(QListViewItem *item)
{
  CurrentItem=item;
  if (CurrentItem&&LastSession!=CurrentItem->text(0)) {
    if (!CurrentItem->text(0).isEmpty()) {
      WaitBar->changeParams(CurrentItem->text(0));
      IOBar->changeParams(CurrentItem->text(0));
    }
    LastSession=CurrentItem->text(0);
  }
  changeTab(CurrentTab);
}
