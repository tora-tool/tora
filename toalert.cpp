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

#include <stdio.h>
#include <unistd.h>

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
#include "tomemoeditor.h"

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

#define TIMEOUT 1

toAlert::toAlert(QWidget *main,toConnection &connection)
  : toToolWidget(AlertTool,"alert.html",main,connection),Connection(connection)
{
  QToolBar *toolbar=toAllocBar(this,"Alert Tool",connection.description());

  new QLabel("Registered",toolbar);
  Registered=new QComboBox(toolbar);
  Registered->insertItem("TOra");
  Registered->setCurrentItem(0);
  Registered->setEditable(true);
  Registered->setDuplicatesEnabled(false);
  connect(Registered,SIGNAL(activated(int)),this,SLOT(add()));
  AddNames.insert(AddNames.end(),"TOra");

  new QToolButton(QPixmap((const char **)trash_xpm),
		  "Remove registered",
		  "Remove registered",
		  this,SLOT(remove()),
		  toolbar);

  toolbar->addSeparator();

  new QLabel("Name",toolbar);
  Name=new QLineEdit(toolbar);
  Name->setText("TOra");
  connect(Name,SIGNAL(returnPressed()),this,SLOT(send()));
  new QLabel("Message",toolbar);
  Message=new QLineEdit(toolbar);
  connect(Message,SIGNAL(returnPressed()),this,SLOT(send()));
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

  connect(&Timer,SIGNAL(timeout()),this,SLOT(poll()));
  Timer.start(TIMEOUT*1000);

  Alerts=new toListView(this);
  Alerts->addColumn("Time");
  Alerts->addColumn("Name");
  Alerts->addColumn("Message");

  State=Started;
  toThread *thread=new toThread(new pollTask(*this));
  thread->start();
}

toAlert::~toAlert()
{
  Lock.lock();
  State=Quit;
  do {
    Lock.unlock();
    sleep(1);
    Lock.lock();
  } while(State==Quit);
  AlertTool.closeWindow(connection());
}

static toSQL SQLRegister("toAlert:Register",
			 "BEGIN DBMS_ALERT.REGISTER(:name<char[4000]>); END;",
			 "Register an alert name, must have same binds");
static toSQL SQLRemove("toAlert:Remove",
		       "BEGIN DBMS_ALERT.REMOVE(:name<char[4000]>); END;",
		       "Remove a registered an alert name, must have same binds");
static toSQL SQLRemoveAll("toAlert:RemoveAll",
			  "BEGIN DBMS_ALERT.REMOVEALL; END;",
			  "Remove all registered alerts");
static toSQL SQLPoll("toAlert:PollAlert",
		     "BEGIN\n"
		     "  DBMS_ALERT.WAITANY(:name<char[4000],out>,:msg<char[4000],out>,\n"
		     "                     :stat<in,out>,:tim<char[10],in>);\n"
		     "END;",
		     "Poll for alert, must have same binds");
static toSQL SQLSignal("toAlert:Signal",
		     "BEGIN\n"
		     "  DBMS_ALERT.SIGNAL(:name<char[4000],in>,:msg<char[4000],in>);\n"
		     "END;",
		     "Signal alert, must have same binds");

void toAlert::pollTask::run(void)
{
  Parent.Lock.lock();
  while(Parent.State!=Quit) {
    Parent.Lock.unlock();
    try {
      {
	toLocker lock(Parent.Lock);
	for(std::list<QString>::iterator i=Parent.AddNames.begin();
	    i!=Parent.AddNames.end();
	    i++) {
	  Parent.Names.insert(Parent.Names.end(),*i);
	  Parent.Connection.execute(SQLRegister,*i);
	}
	Parent.AddNames.clear();
	for(std::list<QString>::iterator i=Parent.DelNames.begin();
	    i!=Parent.DelNames.end();
	    i++) {
	  std::list<QString>::iterator j=toFind(Parent.Names,*i);
	  if (j!=Parent.Names.end()) {
	    Parent.Names.erase(j);
	    Parent.Connection.execute(SQLRemove,*i);
	  }
	}
	Parent.DelNames.clear();
      }

      {
	toLocker lock(Parent.Lock);
	std::list<QString>::iterator i=Parent.SendAlerts.begin();
	std::list<QString>::iterator j=Parent.SendMessages.begin();
	while(i!=Parent.SendAlerts.end()&&j!=Parent.SendMessages.end()) {
	  Parent.Connection.execute(SQLSignal,*i,*j);
	  i++;
	  j++;
	}
	Parent.SendAlerts.clear();
	Parent.SendMessages.clear();
	Parent.Connection.commit();
      }

      toQuery query(Parent.Connection,SQLPoll,QString::number(TIMEOUT));
      QString name=query.readValue();
      QString msg=query.readValue();
      if (query.readValue().toInt()==0) {
	toLocker lock(Parent.Lock);
	Parent.NewAlerts.insert(Parent.NewAlerts.end(),name);
	Parent.NewMessages.insert(Parent.NewMessages.end(),msg);
      }
    } catch(const QString &str) {
      printf("Exception in alert polling:\n%s\n",(const char *)str);
    } catch(...) {
      printf("Unexpected alert in polling.\n");
    }

    Parent.Lock.lock();
  }
  try {
    Parent.Connection.execute(SQLRemoveAll);
  } catch(...) {
  }
  Parent.State=Done;
  Parent.Lock.unlock();
}

void toAlert::poll(void)
{
  toLocker lock(Lock);
  std::list<QString>::iterator i=NewAlerts.begin();
  std::list<QString>::iterator j=NewMessages.begin();
  while(i!=NewAlerts.end()&&j!=NewMessages.end()) {
    QListViewItem *item=new toResultViewMLine(Alerts,NULL,toNow(connection()));
    item->setText(1,*i);
    item->setText(2,*j);
    i++;
    j++;
  }
  NewAlerts.clear();
  NewMessages.clear();
}

void toAlert::send(void)
{
  if (!Name->text().isEmpty()&&!Message->text().isEmpty()) {
    toLocker lock(Lock);
    SendAlerts.insert(SendAlerts.end(),Name->text());
    SendMessages.insert(SendMessages.end(),Message->text());
    Message->clear();
  }
}

void toAlert::memo(void)
{
  toMemoEditor *memo=new toMemoEditor(this,Message->text(),0,0);
  connect(memo,SIGNAL(changeData(int,int,const QString &)),
	  this,SLOT(changeMessage(int,int,const QString &)));
}

void toAlert::changeMessage(int,int,const QString &str)
{
  Message->setText(str);
}

void toAlert::remove(void)
{
  toLocker lock(Lock);
  QString name=Registered->currentText();
  if (!name.isEmpty()) {
    std::list<QString>::iterator i=toFind(AddNames,name);
    if (i!=AddNames.end())
      AddNames.erase(i);
    i=toFind(Names,name);
    if (i!=Names.end()) {
      if (toFind(DelNames,name)==DelNames.end())
	DelNames.insert(DelNames.end(),name);
    }
  }

  if (Registered->count()>0)
    Registered->removeItem(Registered->currentItem());
  if (Registered->count()>0)
    Registered->setCurrentItem(0);
}

void toAlert::add(void)
{
  Registered->lineEdit()->selectAll();
  QString name=Registered->currentText();
  if (!name.isEmpty()) {
    std::list<QString>::iterator i=toFind(DelNames,name);
    if(i==DelNames.end()) {
      i=toFind(Names,name);
      if (i==Names.end()) {
	i=toFind(AddNames,name);
	if (i==AddNames.end())
	  AddNames.insert(AddNames.end(),name);
      }
    } else {
      DelNames.erase(i);
    }
  }
}
