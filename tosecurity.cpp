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

#include <qlabel.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include "toconnection.h"
#include "tosecurity.h"
#include "tosql.h"
#include "totool.h"
#include "tomain.h"

#include "tosecurity.moc"

#include "icons/refresh.xpm"
#include "icons/tosecurity.xpm"

class toSecurityTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return tosecurity_xpm; }
public:
  toSecurityTool()
    : toTool(13,"Security Manager")
  { }
  virtual const char *menuItem()
  { return "Security Manager"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toSecurity(parent,connection);
  }
};

static toSecurityTool OutputTool;

static QPixmap *toRefreshPixmap;

toSecurity::toSecurity(QWidget *main,toConnection &connection)
  : QVBox(main,NULL,WDestructiveClose),Connection(connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);

  QToolBar *toolbar=new QToolBar("SQL Output",toMainWidget(),this);
  new QToolButton(*toRefreshPixmap,
		  "Poll for output now",
		  "Poll for output now",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel("",toolbar));

  Connection.addWidget(this);
}

toSecurity::~toSecurity()
{
  Connection.delWidget(this);
}

void toSecurity::refresh(void)
{
}
