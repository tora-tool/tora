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

#include "toconf.h"
#include "toconnection.h"
#include "tonewconnection.h"
#include "tomain.h"
#include "toresultview.h"
#include "totool.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwhatsthis.h>

#ifdef WIN32
#  include "windows/cregistry.h"
#endif

#include "tonewconnection.moc"
#include "tonewconnectionui.moc"

toNewConnection::toNewConnection(QWidget* parent, const char* name,bool modal,WFlags fl)
  : toNewConnectionUI(parent,name,modal,fl),
    toHelpContext(QString::fromLatin1("newconnection.html"))
{
  toHelp::connectDialog(this);

  QPopupMenu *menu = new QPopupMenu(Previous);
  Database->insertItem(toTool::globalConfig(CONF_DATABASE,DEFAULT_DATABASE));
  Previous->addColumn(tr("Provider"));
  Previous->addColumn(tr("Host"));
  Previous->addColumn(tr("Database"));
  Previous->addColumn(tr("Username"));
  Previous->setSelectionMode(QListView::Single);
  menu->insertItem("Delete",TONEWCONNECTION_DELETE);
  connect(menu,SIGNAL(activated(int)),this,SLOT(menuCallback(int)));
  Previous->setDisplayMenu(menu);
  std::list<QCString> lst=toConnectionProvider::providers();
  int sel=0,cur=0;
  QCString provider=toTool::globalConfig(CONF_PROVIDER,DEFAULT_PROVIDER).latin1();
  for(std::list<QCString>::iterator i=lst.begin();i!=lst.end();i++) {
    Provider->insertItem(QString::fromLatin1(*i));
    if (*i==provider)
      sel=cur;
    cur++;
  }
  if (cur==0) {
    TOMessageBox::information(this,
			      tr("No connection provider"),
			      tr("No available connection provider, plugins probably missing"));
    reject();
    return;
  }
  Provider->setCurrentItem(sel);
  changeProvider();

  SqlNet->setChecked(!toTool::globalConfig(CONF_HOST,DEFAULT_HOST).isEmpty());
  Host->lineEdit()->setText(toTool::globalConfig(CONF_HOST,DEFAULT_HOST));

  Username->setText(toTool::globalConfig(CONF_USER,DEFAULT_USER));
  Username->setFocus();

  bool pass=toTool::globalConfig(CONF_SAVE_PWD,DEFAULT_SAVE_PWD).isEmpty();
  if (pass)
    Password->setText(QString::fromLatin1(DEFAULT_PASSWORD));
  else
    Password->setText(toUnobfuscate(toTool::globalConfig(CONF_PASSWORD,DEFAULT_PASSWORD)));

  QString defdb=toTool::globalConfig(CONF_DATABASE,DEFAULT_DATABASE);
  Database->setEditable(true);

  {
    int maxHist=toTool::globalConfig(CONF_CONNECT_CURRENT,0).toInt();
    Previous->setSorting(-1);
    QListViewItem *last=NULL;
    for (int i=0;i<maxHist;i++) {
      QCString path=CONF_CONNECT_HISTORY;
      path+=":";
      path+=QString::number(i).latin1();
      QCString tmp=path;
      tmp+=CONF_USER;
      QString user=toTool::globalConfig(tmp,"");

      tmp=path;
      tmp+=CONF_PASSWORD;
      QString passstr=(pass?QString::fromLatin1(DEFAULT_PASSWORD):
                      (toUnobfuscate(toTool::globalConfig(tmp,DEFAULT_PASSWORD))));

      tmp=path;
      tmp+=CONF_HOST;
      QString host=toTool::globalConfig(tmp,DEFAULT_HOST);

      tmp=path;
      tmp+=CONF_DATABASE;
      QString database=toTool::globalConfig(tmp,DEFAULT_DATABASE);

      tmp=path;
      tmp+=CONF_PROVIDER;
      QString provider=toTool::globalConfig(tmp,DEFAULT_PROVIDER);

      last=new QListViewItem(Previous,last,provider,host,database,user,passstr);
    }
  }
}

toNewConnection::~toNewConnection() {
}

void toNewConnection::changeProvider(void)
{
  try {
    std::list<QString> hosts=toConnectionProvider::hosts(Provider->currentText().latin1());
    QString current=Host->currentText();

    Host->clear();
    bool sqlNet=false;
    for(std::list<QString>::iterator i=hosts.begin();i!=hosts.end();i++) {
      if ((*i).isEmpty())
	sqlNet=true;
      Host->insertItem(*i);
    }
    if (sqlNet) {
      HostLabel->hide();
      Host->hide();
      SqlNet->show();
    } else {
      HostLabel->show();
      Host->show();
      SqlNet->hide();
    }
    Host->lineEdit()->setText(current);
    Mode->clear();
    std::list<QString> modes=toConnectionProvider::modes(Provider->currentText().latin1());
    for(std::list<QString>::iterator j=modes.begin();j!=modes.end();j++)
      Mode->insertItem(*j);
  } catch (const QString &str) {
    Host->clear();
    toStatusMessage(str);
  }
}

void toNewConnection::changeHost(void)
{
  try {
    QString host;
    if (SqlNet->isHidden())
      host=Host->currentText();
    else
      host=(SqlNet->isChecked()?QString::fromLatin1("SQL*Net"):QString::null);
    std::list<QString> databases=toConnectionProvider::databases(Provider->currentText().latin1(),
								 host,
								 Username->text(),
								 Password->text());
    QString current=Database->currentText();

    Database->clear();
    for(std::list<QString>::iterator i=databases.begin();i!=databases.end();i++)
      Database->insertItem(*i);
    Database->lineEdit()->setText(current);
  } catch (const QString &str) {
    Database->clear();
    toStatusMessage(str);
  } 
}

toConnection *toNewConnection::makeConnection(void)
{
  try {
    toTool::globalSetConfig(CONF_PROVIDER,Provider->currentText());
    toTool::globalSetConfig(CONF_USER,Username->text());
    QString pass;
    QString host;
    if (SqlNet->isHidden())
      host=Host->currentText();
    else
      host=SqlNet->isChecked()?QString::fromLatin1("SQL*Net"):QString::null;
    toTool::globalSetConfig(CONF_HOST,host);

    std::list<QString> con=toMainWidget()->connections();
    for(std::list<QString>::iterator i=con.begin();i!=con.end();i++) {
      try {
	toConnection &conn=toMainWidget()->connection(*i);
	if(conn.user()==Username->text()&&
	   conn.provider()==Provider->currentText().latin1()&&
	   conn.host()==host&&
	   conn.database()==Database->currentText())
	  return &conn;
      } catch(...) {
      }
    }

    toConnection *retCon=new toConnection(Provider->currentText().latin1(),
					  Username->text(),
					  Password->text(),
					  host,
					  Database->currentText(),
					  Mode->currentText());
    {
      for(QListViewItem *item=Previous->firstChild();item;item=item->nextSibling()) {
	if (item->text(0)==Provider->currentText()&&
	    ((item->text(1)==host)||(item->text(1).isEmpty()==host.isEmpty()))&&
	    item->text(2)==Database->currentText()&&
	    item->text(3)==Username->text()) {
	  delete item;
	  break;
	}
      }
    }

    if (toTool::globalConfig(CONF_SAVE_PWD,DEFAULT_SAVE_PWD)!=DEFAULT_SAVE_PWD)
      pass=retCon->password();
    else
      pass=DEFAULT_SAVE_PWD;
    toTool::globalSetConfig(CONF_PASSWORD,toObfuscate(pass));
    toTool::globalSetConfig(CONF_DATABASE,Database->currentText());
    toTool::globalSetConfig(CONF_PASSWORD,toObfuscate(retCon->password()));

    new QListViewItem(Previous,NULL,
		      Provider->currentText(),
		      host,
		      Database->currentText(),
		      Username->text(),
		      retCon->password());
    historySave();
    return retCon;
  } catch (const QString &exc) {
    QString str=tr("Unable to connect to the database.\n");
    str.append(exc);
    TOMessageBox::information(this,
			      tr("Unable to connect to the database"),
			      str);
    return NULL;
  }
}
void toNewConnection::historySave(void) {
  int siz=toTool::globalConfig(CONF_CONNECT_SIZE,DEFAULT_CONNECT_SIZE).toInt();
  int i=0;
  int j=0;

  for(QListViewItem *item=Previous->firstChild();i<siz;item=(item?item=item->nextSibling():0)) {
    QCString path=CONF_CONNECT_HISTORY;
    path+=":";
    path+=QString::number(i).latin1();
    
    QCString tmp=path;
    tmp+=CONF_PROVIDER;
    if (i<siz&&item)
      toTool::globalSetConfig(tmp,item->text(0));
    else
      toTool::globalEraseConfig(tmp);
    
    tmp=path;
    tmp+=CONF_HOST;
    if (i<siz&&item)
      toTool::globalSetConfig(tmp,item->text(1));
    else
      toTool::globalEraseConfig(tmp);
    
    tmp=path;
    tmp+=CONF_DATABASE;
    if (i<siz&&item)
      toTool::globalSetConfig(tmp,item->text(2));
    else
      toTool::globalEraseConfig(tmp);
    
    tmp=path;
    tmp+=CONF_USER;
    if (i<siz&&item)
      toTool::globalSetConfig(tmp,item->text(3));
    else
      toTool::globalEraseConfig(tmp);
    
    tmp=path;
    tmp+=CONF_PASSWORD;
    if (i<siz&&item)
      toTool::globalSetConfig(tmp,toObfuscate(item->text(4)));
    else
      toTool::globalEraseConfig(tmp);
    
    i++;
    if (i<siz&&item)
      j++;
  }
  toTool::globalSetConfig(CONF_CONNECT_CURRENT,QString::number(j));
  toTool::saveConfig();
}

void toNewConnection::historySelection(void)
{
  QListViewItem *item=Previous->selectedItem();
  if (item) {
    for (int i=0;i<Provider->count();i++)
      if (Provider->text(i)==item->text(0)) {
	Provider->setCurrentItem(i);
	break;
      }
    changeProvider();
    if (SqlNet->isHidden())
      Host->lineEdit()->setText(item->text(1));
    else
      SqlNet->setChecked(!item->text(1).isEmpty());
    Database->lineEdit()->setText(item->text(2));
    Username->setText(item->text(3));
    if(item->text(4)!=DEFAULT_PASSWORD)
      Password->setText(item->text(4));
  }
}

void toNewConnection::historyConnect(void)
{
  bool ok=true;
  if(Password->text()==DEFAULT_PASSWORD) {
    ok=false;
    QString name=QInputDialog::getText(tr("Enter password"),
				       tr("Enter password to use for connection."),
				       QLineEdit::Password,QString::null,&ok,this);
    if (ok)
      Password->setText(name);
  }
  if (ok)
    accept();
}
void toNewConnection::menuCallback(int cmd)
{
  switch(cmd) {
  case TONEWCONNECTION_DELETE:
    {
      historyDelete();
      break;
    }
  }
}

void toNewConnection::historyDelete() {
  QListViewItem *item=Previous->selectedItem();
  if (item) {
        Previous->takeItem(item);
	delete item;
        historySave();
  }
}
