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

#include "utils.h"

#include "tobackup.h"
#include "tochangeconnection.h"
#include "tomain.h"
#include "toresultlong.h"
#include "tosql.h"
#include "totool.h"

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <qlabel.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include "tobackup.moc"

#include "icons/refresh.xpm"
#include "icons/tobackup.xpm"

class toBackupTool : public toTool {
protected:
  std::map<toConnection *,QWidget *> Windows;

  virtual char **pictureXPM(void)
  { return tobackup_xpm; }
public:
  toBackupTool()
    : toTool(240,"Backup Manager")
  { }
  virtual const char *menuItem()
  { return "Backup Manager"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    std::map<toConnection *,QWidget *>::iterator i=Windows.find(&connection);
    if (i!=Windows.end()) {
      (*i).second->setFocus();
      return NULL;
    } else {
      QWidget *window=new toBackup(parent,connection);
      Windows[&connection]=window;
      return window;
    }
  }
  void closeWindow(toConnection &connection)
  {
    std::map<toConnection *,QWidget *>::iterator i=Windows.find(&connection);
    if (i!=Windows.end())
      Windows.erase(i);
  }
};

static toBackupTool BackupTool;

static toSQL SQLLogHistory("toBackup:LogHistory",
			   "SELECT trunc(first_time)\"Date\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               0,\n"
			   "                               1))),\n"
			   "           ' ')\"-00\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               1,\n"
			   "                               1))),\n"
			   "           ' ')\"-01\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               2,\n"
			   "                               1))),\n"
			   "           ' ')\"-02\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               3,\n"
			   "                               1))),\n"
			   "           ' ')\"-03\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               4,\n"
			   "                               1))),\n"
			   "           ' ')\"-04\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               5,\n"
			   "                               1))),\n"
			   "           ' ')\"-05\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               6,\n"
			   "                               1))),\n"
			   "           ' ')\"-06\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               7,\n"
			   "                               1))),\n"
			   "           ' ')\"-07\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               8,\n"
			   "                               1))),\n"
			   "           ' ')\"-08\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               9,\n"
			   "                               1))),\n"
			   "           ' ')\"-09\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               10,\n"
			   "                               1))),\n"
			   "           ' ')\"-10\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               11,\n"
			   "                               1))),\n"
			   "           ' ')\"-11\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               12,\n"
			   "                               1))),\n"
			   "           ' ')\"-12\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               13,\n"
			   "                               1))),\n"
			   "           ' ')\"-13\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               14,\n"
			   "                               1))),\n"
			   "           ' ')\"-14\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               15,\n"
			   "                               1))),\n"
			   "           ' ')\"-15\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               16,\n"
			   "                               1))),\n"
			   "           ' ')\"-16\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               17,\n"
			   "                               1))),\n"
			   "           ' ')\"-17\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               18,\n"
			   "                               1))),\n"
			   "           ' ')\"-18\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               19,\n"
			   "                               1))),\n"
			   "           ' ')\"-19\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               20,\n"
			   "                               1))),\n"
			   "           ' ')\"-20\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               21,\n"
			   "                               1))),\n"
			   "           ' ')\"-21\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               22,\n"
			   "                               1))),\n"
			   "           ' ')\"-22\",\n"
			   "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
			   "                                                 'HH24')),\n"
			   "                               23,\n"
			   "                               1))),\n"
			   "           ' ')\"-23\"\n"
			   "  FROM v$log_history\n"
			   " GROUP BY trunc(first_time)\n"
			   " ORDER BY trunc(first_time)DESC",
			   "Log switch history");


toBackup::toBackup(QWidget *main,toConnection &connection)
  : toToolWidget(BackupTool,"backup.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,"Backup Manager",connection.description());

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Update",
		  "Update",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  Tabs=new QTabWidget(this);

  QVBox *box=new QVBox(Tabs,"history");
  new QLabel("Logswitches per day and hour",box);
  LogHistory=new toResultLong(true,false,toQuery::Background,box);
  LogHistory->setSQL(SQLLogHistory);
  Tabs->addTab(box,"Redo Switches");

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  refresh();

  setFocusProxy(Tabs);
}

void toBackup::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),"&Refresh",
			   this,SLOT(refresh(void)),Key_F5);
      toMainWidget()->menuBar()->insertItem("&Backup Manager",ToolMenu,-1,toToolMenuIndex());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

toBackup::~toBackup()
{
  BackupTool.closeWindow(connection());
}

void toBackup::refresh()
{
  LogHistory->refresh();
}

