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

#include <qpopupmenu.h>
#include <qtooltip.h>

#include "tomain.h"
#include "toconnection.h"
#include "tochangeconnection.h"
#include "totool.h"

#include "tochangeconnection.moc"
#include "icons/changeconnect.xpm"

static QPixmap *Pixmap;

toChangeConnection::toChangeConnection(QToolBar *parent,const char *name)
  : QToolButton(parent,name)
{
  if (!Pixmap)
    Pixmap=new QPixmap((const char **)changeconnect_xpm);

  setPopup(new QPopupMenu(this));
  connect(popup(),SIGNAL(aboutToShow()),this,SLOT(popupMenu()));
  connect(popup(),SIGNAL(activated(int)),this,SLOT(changeConnection(int)));
  setPopupDelay(0);
  setOffIconSet(*Pixmap);
  QToolTip::add(this,"Change the connection of the tool.");
}

void toChangeConnection::popupMenu(void)
{
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
}

void toChangeConnection::changeConnection(int val)
{
  std::list<QString> cons=toMainWidget()->connections();
  std::list<QString>::iterator i=cons.begin();
  while (i!=cons.end()&&val>0) {
    i++;
    val--;
  }
  if (i==cons.end())
    throw QString("Couldn't find selected connection");
  QWidget *cur=parentWidget();
  while(cur) {
    try {
      toToolWidget *tool=dynamic_cast<toToolWidget *>(cur);
      if (tool) {
	tool->setConnection(toMainWidget()->connection(*i));
	toMainWidget()->windowActivated(tool);
	return;
      }
    } catch(...) {
      // Catch problems with Visual C++ missing RTTI
    }
    cur=cur->parentWidget();
  }
  throw QString("Couldn't find parent connection. Internal error.");
}
