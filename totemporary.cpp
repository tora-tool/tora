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

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "toextract.h"
#include "tohighlightedtext.h"
#include "totemporary.h"
#include "tomain.h"
#include "toresultextract.h"
#include "toresultlong.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <qlabel.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qprogressdialog.h>
#include <qsplitter.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include "totemporary.moc"

#include "icons/refresh.xpm"
#include "icons/totemporary.xpm"

static toSQL SQLListTemporarySortObjects(
			"toTemporary:ListTemporarySortObjects",
			"select s.sid || ',' || s.serial# sid," \
             		"s.username, u.tablespace," \
			"substr(a.sql_text, 1, 50) sql_text," \
			"round(((u.blocks*p.value)/1024/1024),2) size_mb " \
			"from v$sort_usage u, v$session s, v$sqlarea a, v$parameter p " \
			"where s.saddr = u.session_addr " \
			"and a.address (+) = s.sql_address "\
			"and a.hash_value (+) = s.sql_hash_value "\
			"and p.name = 'db_block_size' "\
			"group by " \
			"s.sid || ',' || s.serial#, " \
			"s.username, " \
			"substr(a.sql_text, 1, 50), "\
			"u.tablespace, " \
			"round(((u.blocks*p.value)/1024/1024),2)" ,
		    "Get temporary sort usage.");

static toSQL SQLListTemporaryObjects(
		"toTemporary:ListTermporaryObjects",
	        "select * from all_tables where DURATION = 'SYS$SESSION' or duration = 'SYS$TRANSACTION'",
		 "Get temporary object usage.");


class toTemporaryTool : public toTool {
  virtual char **pictureXPM(void)
  { return totemporary_xpm; }
public:
  toTemporaryTool()
    : toTool(250,"Temporary Objects")
  { }
  virtual const char *menuItem()
  { return "Temporary Objects"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toTemporary(parent,connection);
  }
};

static toTemporaryTool TemporaryTool;

toTemporary::toTemporary(QWidget *main,toConnection &connection)
  : toToolWidget(TemporaryTool,"temporary.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,tr("Temporary Objects"));

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  tr("Refresh list"),
		  tr("Refresh list"),
		  this,SLOT(refresh()),
		  toolbar);

  toolbar->setStretchableWidget(new QLabel(toolbar,TO_KDE_TOOLBAR_WIDGET));
  QSplitter *splitter=new QSplitter(Vertical,this);

  SortObjects =new toResultLong(false,false,toQuery::Background,splitter);
  SortObjects->setSQL(SQLListTemporarySortObjects);

  TmpObjects =new toResultLong(false,false,toQuery::Background,splitter);
  TmpObjects->setSQL(SQLListTemporaryObjects);
 
  {
    QValueList<int> sizes=splitter->sizes();
    sizes[0]=500;
    splitter->setSizes(sizes);
    splitter->setResizeMode(SortObjects,QSplitter::KeepSize);
  }

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  refresh();
  setFocusProxy(SortObjects);
}


void toTemporary::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),tr("&Refresh"),
			   this,SLOT(refresh(void)),Key_F5);

      toMainWidget()->menuBar()->insertItem(tr("&Temporary"),ToolMenu,-1,toToolMenuIndex());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

void toTemporary::refresh(void)
{
  SortObjects->refresh();
  TmpObjects->refresh();
}

