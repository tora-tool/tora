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

toSession::toSession(QWidget *main,toConnection &connection)
  : QVBox(main,NULL,WDestructiveClose),Connection(connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);
  if (!toClockPixmap)
    toClockPixmap=new QPixmap((const char **)clock_xpm);
  if (!toNoClockPixmap)
    toNoClockPixmap=new QPixmap((const char **)noclock_xpm);
  if (!toDisconnectPixmap)
    toDisconnectPixmap=new QPixmap((const char **)disconnect_xpm);

  QToolBar *toolbar=new QToolBar("SGA Trace",toMainWidget(),this);
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
  SessionStatistics=new toResultStats(0,Connection,ResultTab);
  ResultTab->addTab(SessionStatistics,"Statistics");
  ConnectInfo=new toResultView(true,false,Connection,ResultTab);
  ConnectInfo->setSQL("select authentication_type,osuser,network_service_banner"
		      "  from v$session_connect_info where sid = :f1<char[31]>");
  ResultTab->addTab(ConnectInfo,"Connect Info");
  PendingLocks=new toResultLock(Connection,ResultTab);
  ResultTab->addTab(PendingLocks,"Pending Locks");
  LockedObjects=new toResultView(false,false,Connection,ResultTab);
  ResultTab->addTab(LockedObjects,"Locked Objects");
  LockedObjects->setSQL("select b.Object_Name \"Object Name\","
			"       b.Object_Type \"Type\","
			"       DECODE(a.locked_mode,0,'None',1,'Null',2,'Row-S',3,'Row-X',4,'Share',5,'S/Row-X',6,'Exclusive',a.Locked_Mode) \"Locked Mode\""
			"  from v$locked_object a,all_objects b"
			" where a.object_id = b.object_id"
			"   and a.session_id = :f1<char[31]>");
  CurrentStatement=new toSGAStatement(ResultTab,Connection);
  ResultTab->addTab(CurrentStatement,"Current Statement");
  PreviousStatement=new toSGAStatement(ResultTab,Connection);
  ResultTab->addTab(PreviousStatement,"Previous Statement");

  OpenSplitter=new QSplitter(Horizontal,ResultTab);
  ResultTab->addTab(OpenSplitter,"Open Cursors");
  OpenCursors=new toResultView(false,true,Connection,OpenSplitter);
  OpenCursors->setSQL("select SQL_Text \"SQL\", Address||':'||Hash_Value \" Address\""
		      "  from v$open_cursor where sid = :f1<char[31]>");
  OpenStatement=new toSGAStatement(OpenSplitter,Connection);

  connect(Sessions,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  connect(OpenCursors,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeCursor(QListViewItem *)));
  connect(ResultTab,SIGNAL(currentChanged(QWidget *)),
	  this,SLOT(changeTab(QWidget *)));

  Connection.addWidget(this);

  Timer=new QTimer(this);
  connect(Timer,SIGNAL(timeout(void)),this,SLOT(refresh(void)));
  toRefreshParse(Timer,toTool::globalConfig(CONF_REFRESH,DEFAULT_REFRESH));
  CurrentTab=SessionStatistics;
  CurrentItem=NULL;
  refresh();
}

toSession::~toSession()
{
  Connection.delWidget(this);
}

void toSession::refresh(void)
{
  QString session;
  QString serial;
  if (CurrentItem) {
    session=CurrentItem->text(0);
    serial=CurrentItem->text(1);
  }
  Sessions->query("SELECT Sid \"-Id\","
		  "       Serial# \"-Serial#\","
		  "       SchemaName \"Schema\","
		  "       Status \"Status\","
		  "       Server \"Server\","
		  "       OsUser \"Osuser\","
		  "       Machine \"Machine\","
		  "       Program \"Program\","
		  "       Type \"Type\","
		  "       Module \"Module\","
		  "       Action \"Action\","
		  "       Client_Info \"Client Info\","
		  "       Process \"-Process\","
		  "       SQL_Address||':'||SQL_Hash_Value \" SQL Address\","
		  "       Prev_SQL_Addr||':'||Prev_Hash_Value \" Prev SQl Address\""
		  "  FROM v$session"
		  " ORDER BY Sid");
  for (CurrentItem=Sessions->firstChild();CurrentItem;CurrentItem=CurrentItem->nextSibling())
    if (CurrentItem->text(0)==session&&
	CurrentItem->text(1)==serial) {
      Sessions->setSelected(CurrentItem,true);
      break;
    }
  changeTab(CurrentTab);
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
		   sql,
		   Connection.connection());
  } catch (...) {
    toStatusMessage("No access to timed statistics flags");
  }
}

#include <stdio.h>

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
    switch(QMessageBox::warning(this,"Commit work?",str,"&Yes","&No","&Cancel")) {
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
		     sql,
		     Connection.connection());
    } catch (const QString &str) {
      toStatusMessage((const char *)str);
    } catch (const otl_exception &exc) {
      toStatusMessage((const char *)exc.msg);
    }
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
