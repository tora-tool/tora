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

#include "tochangeconnection.h"
#include "toconnection.h"
#include "tomain.h"
#include "totool.h"

#include <qpopupmenu.h>
#include <qtooltip.h>

#include "tochangeconnection.moc"

#include "icons/changeconnect.xpm"

toChangeConnection::toChangeConnection(QToolBar *parent,const char *name)
  : toPopupButton(QPixmap((const char **)changeconnect_xpm),
		  tr("Change the connection of the tool."),
		  tr("Change the connection of the tool."),
		  parent,name)
{
  setPopup(new QPopupMenu(this));
  connect(popup(),SIGNAL(aboutToShow()),this,SLOT(popupMenu()));
  connect(popup(),SIGNAL(activated(int)),this,SLOT(changeConnection(int)));
}

void toChangeConnection::popupMenu(void)
{
  try {
    popup()->clear();
    toConnection &conn=toCurrentConnection(this);
    std::list<QString> cons=toMainWidget()->connections();
    int idx=0;
    for (std::list<QString>::iterator i=cons.begin();i!=cons.end();i++,idx++) {
      if (toCurrentTool(this)->canHandle(toMainWidget()->connection(*i))) {
	popup()->insertItem(*i,idx);
	if (conn.description()==*i)
	  popup()->setItemChecked(idx,true);
      }
    }
  } TOCATCH
}

void toChangeConnection::changeConnection(int val)
{
  if (popup()->isItemChecked(val))
    return;
  std::list<QString> cons=toMainWidget()->connections();
  std::list<QString>::iterator i=cons.begin();
  while (i!=cons.end()&&val>0) {
    i++;
    val--;
  }
  try {
    if (i==cons.end())
      throw tr("Couldn't find selected connection");
    QWidget *cur=parentWidget();
    while(cur) {
      toToolWidget *tool=dynamic_cast<toToolWidget *>(cur);
      if (tool) {
	tool->setConnection(toMainWidget()->connection(*i));
	toMainWidget()->windowActivated(tool);
	toMainWidget()->changeConnection();
	return;
      }
      cur=cur->parentWidget();
    }
    throw tr("Couldn't find parent connection. Internal error.");
  } TOCATCH
}
