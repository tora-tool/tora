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

#include <qworkspace.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qsplitter.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qlabel.h>

#include "tomain.h"
#include "totool.h"
#include "tosession.h"
#include "toresultview.h"
#include "toconf.h"
#include "tosql.h"
#include "tosgastatement.h"
#include "toresultstats.h"
#include "toresultlock.h"

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
    QWidget *window=new toSession(parent,connection);
    window->setIcon(*toolbarImage());
    return window;
  }
};

static toSessionTool SessionTool;

static QPixmap *toRefreshPixmap;
static QPixmap *toClockPixmap;
static QPixmap *toNoClockPixmap;
static QPixmap *toDisconnectPixmap;

static toSQL SQLConnectInfo("toSession:ConnectInfo",
			    "select authentication_type,osuser,network_service_banner\n"
			    "  from v$session_connect_info where sid = :f1<char[31]>",
			    "Get connection info for a session");
static toSQL SQLLockedObject("toSession:LockedObject",
			     "select b.Object_Name \"Object Name\",\n"
			     "       b.Object_Type \"Type\",\n"
			     "       DECODE(a.locked_mode,0,'None',1,'Null',2,'Row-S',\n"
			     "                            3,'Row-X',4,'Share',5,'S/Row-X',\n"
			     "                            6,'Exclusive',a.Locked_Mode) \"Locked Mode\"\n"
			     "  from v$locked_object a,all_objects b\n"
			     " where a.object_id = b.object_id\n"
			     "   and a.session_id = :f1<char[31]>",
			     "Display info about objects locked by this session");
static toSQL SQLOpenCursors("toSession:OpenCursor",
			    "select SQL_Text \"SQL\", Address||':'||Hash_Value \" Address\"\n"
			    "  from v$open_cursor where sid = :f1<char[31]>",
			    "Display open cursors of this session");


toSession::toSession(QWidget *main,toConnection &connection)
  : toToolWidget(main,connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);
  if (!toClockPixmap)
    toClockPixmap=new QPixmap((const char **)clock_xpm);
  if (!toNoClockPixmap)
    toNoClockPixmap=new QPixmap((const char **)noclock_xpm);
  if (!toDisconnectPixmap)
    toDisconnectPixmap=new QPixmap((const char **)disconnect_xpm);

  QToolBar *toolbar=toAllocBar(this,"Session manager",connection.connectString());

  new QToolButton(*toRefreshPixmap,
		  "Update sessionlist",
		  "Update sessionlist",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(*toClockPixmap,
		  "Enable timed statistics",
		  "Enable timed statistics",
		  this,SLOT(enableStatistics(void)),
		  toolbar);
  new QToolButton(*toNoClockPixmap,
		  "Disable timed statistics",
		  "Disable timed statistics",
		  this,SLOT(disableStatistics(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(*toDisconnectPixmap,
		  "Disconnect selected session",
		  "Disconnect selected session",
		  this,SLOT(disconnectSession(void)),
		  toolbar);
  toolbar->addSeparator();
  new QLabel("Refresh",toolbar);
  connect(toRefreshCreate(toolbar),SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));

  toolbar->setStretchableWidget(new QLabel("",toolbar));

  QSplitter *splitter=new QSplitter(Vertical,this);
  Sessions=new toResultView(false,false,connection,splitter);
  ResultTab=new QTabWidget(splitter);
  SessionStatistics=new toResultStats(false,0,connection,ResultTab);
  ResultTab->addTab(SessionStatistics,"Statistics");
  ConnectInfo=new toResultView(true,false,connection,ResultTab);
  ConnectInfo->setSQL(SQLConnectInfo);
  ResultTab->addTab(ConnectInfo,"Connect Info");
  PendingLocks=new toResultLock(connection,ResultTab);
  ResultTab->addTab(PendingLocks,"Pending Locks");
  LockedObjects=new toResultView(false,false,connection,ResultTab);
  ResultTab->addTab(LockedObjects,"Locked Objects");
  LockedObjects->setSQL(SQLLockedObject);
  CurrentStatement=new toSGAStatement(ResultTab,connection);
  ResultTab->addTab(CurrentStatement,"Current Statement");
  PreviousStatement=new toSGAStatement(ResultTab,connection);
  ResultTab->addTab(PreviousStatement,"Previous Statement");

  OpenSplitter=new QSplitter(Horizontal,ResultTab);
  ResultTab->addTab(OpenSplitter,"Open Cursors");
  OpenCursors=new toResultView(false,true,connection,OpenSplitter);
  OpenCursors->setSQL(SQLOpenCursors);
  OpenStatement=new toSGAStatement(OpenSplitter,connection);

  connect(Sessions,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  connect(OpenCursors,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeCursor(QListViewItem *)));
  connect(ResultTab,SIGNAL(currentChanged(QWidget *)),
	  this,SLOT(changeTab(QWidget *)));

  Timer=new QTimer(this);
  connect(Timer,SIGNAL(timeout(void)),this,SLOT(refresh(void)));
  toRefreshParse(Timer,toTool::globalConfig(CONF_REFRESH,DEFAULT_REFRESH));
  CurrentTab=SessionStatistics;
  CurrentItem=NULL;
  refresh();
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
  QString session;
  QString serial;
  if (CurrentItem) {
    session=CurrentItem->text(0);
    serial=CurrentItem->text(1);
  }
  Sessions->query(SQLSessions);
  for (CurrentItem=Sessions->firstChild();CurrentItem;CurrentItem=CurrentItem->nextSibling())
    if (CurrentItem->text(0)==session&&
	CurrentItem->text(1)==serial) {
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
    otl_stream str(1,
		   sql.utf8(),
		   otlConnect());
  } catch (...) {
    toStatusMessage("No access to timed statistics flags");
  }
}

void toSession::changeTab(QWidget *tab)
{
  CurrentTab=tab;
  if (CurrentItem) {
    if (CurrentTab==SessionStatistics) {
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
      otl_stream str(1,
		     sql.utf8(),
		     otlConnect());
    } TOCATCH
  }
}

void toSession::changeRefresh(const QString &str)
{
  toRefreshParse(Timer,str);
}

void toSession::changeItem(QListViewItem *item)
{
  CurrentItem=item;
  changeTab(CurrentTab);
}
