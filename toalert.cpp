//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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

#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qlabel.h>

#include "totool.h"
#include "toresultview.h"
#include "tomain.h"
#include "tosql.h"
#include "toconnection.h"
#include "tonoblockquery.h"
#include "toalert.h"
#include "toresultview.h"

#include "toalert.moc"

#include "icons/toalert.xpm"
#include "icons/toworksheet.xpm"
#include "icons/return.xpm"
#include "icons/trash.xpm"

class toAlertTool : public toTool {
protected:
  std::map<toConnection *,QWidget *> Windows;

  virtual char **pictureXPM(void)
  { return toalert_xpm; }
public:
  toAlertTool()
    : toTool(104,"Alert Tool")
  { }
  virtual const char *menuItem()
  { return "Alert Tool"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    std::map<toConnection *,QWidget *>::iterator i=Windows.find(&connection);
    if (i!=Windows.end()) {
      (*i).second->setFocus();
      return NULL;
    } else {
      QWidget *window=new toAlert(parent,connection);
      Windows[&connection]=window;
      window->setIcon(*toolbarImage());
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

static toAlertTool AlertTool;

toAlert::toAlert(QWidget *main,toConnection &connection)
  : toToolWidget(AlertTool,"alert.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,"Alert Tool",connection.description());

  new QLabel("Registered",toolbar);
  Registered=new QComboBox(toolbar);
  Registered->insertItem("TOra");
  Registered->setEditable(true);
  connect(Registered,SIGNAL(activated(int)),this,SLOT(add()));

  new QToolButton(QPixmap((const char **)trash_xpm),
		  "Remove registered",
		  "Remove registered",
		  this,SLOT(remove()),
		  toolbar);

  toolbar->addSeparator();

  new QLabel("Name",toolbar);
  Name=new QLineEdit(toolbar);
  Name->setText("TOra");
  new QLabel("Message",toolbar);
  Message=new QLineEdit(toolbar);
  new QToolButton(QPixmap((const char **)toworksheet_xpm),
		  "Edit message in memo",
		  "Edit message in memo",
		  this,SLOT(memo()),
		  toolbar);
  toolbar->setStretchableWidget(Message);
  new QToolButton(QPixmap((const char **)return_xpm),
		  "Send alert",
		  "Send alert",
		  this,SLOT(send()),
		  toolbar);

  Alerts=new toListView(this);
  Alerts->addColumn("Name");
  Alerts->addColumn("Message");
}

toAlert::~toAlert()
{
  AlertTool.closeWindow(connection());
}

void toAlert::poll(void)
{

}

void toAlert::send(void)
{

}

void toAlert::memo(void)
{

}

void toAlert::remove(void)
{
  if (Registered->count()>0)
    Registered->removeItem(Registered->currentItem());
  if (Registered->count()>0)
    Registered->setCurrentItem(0);
}

void toAlert::add(void)
{
  Registered->lineEdit()->selectAll();
}
