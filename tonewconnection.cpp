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

#ifdef WIN32
#  include "windows/cregistry.h"
#endif

#include <qfile.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qlistview.h>

#include "totool.h"
#include "toconf.h"
#include "tonewconnection.h"
#include "tomain.h"

#include "tonewconnection.moc"
#include "tonewconnectionui.moc"

toNewConnection::toNewConnection(QWidget* parent, const char* name,bool modal,WFlags fl)
  : toNewConnectionUI(parent,name,modal,fl),toHelpContext("newconnection.html")
{
  toHelp::connectDialog(this);

  Username->setText(toTool::globalConfig(CONF_USER,DEFAULT_USER));
  Username->setFocus();

  bool pass=toTool::globalConfig(CONF_PASSWORD,DEFAULT_PASSWORD).isEmpty();
  if (pass)
    Password->setText(DEFAULT_PASSWORD);
  else
    Password->setText(toTool::globalConfig(CONF_PASSWORD,DEFAULT_PASSWORD));

  SqlNet->setChecked(!toTool::globalConfig(CONF_SQLNET,"Yes").isEmpty());

  QString defdb=toTool::globalConfig(CONF_DATABASE,DEFAULT_DATABASE);
  Database->setEditable(true);
  if (!defdb.isEmpty())
    Database->insertItem(defdb);

  {
    int maxHist=toTool::globalConfig(CONF_CONNECT_CURRENT,0).toInt();
    Previous->setSorting(-1);
    QListViewItem *last=NULL;
    for (int i=0;i<maxHist;i++) {
      QString path=CONF_CONNECT_HISTORY;
      path+=":";
      path+=QString::number(i);
      QString tmp=path;
      tmp+=CONF_USER;
      QString user=toTool::globalConfig(tmp,QString::null);

      tmp=path;
      tmp+=CONF_PASSWORD;
      QString passstr=(pass?QString(DEFAULT_PASSWORD):
		       (toTool::globalConfig(tmp,DEFAULT_PASSWORD)));

      tmp=path;
      tmp+=CONF_SQLNET;
      QString check=toTool::globalConfig(tmp,"SQL*Net");

      tmp=path;
      tmp+=CONF_DATABASE;
      QString database=toTool::globalConfig(tmp,DEFAULT_DATABASE);

      last=new QListViewItem(Previous,last,database,user,check,passstr);
    }
  }

  QString str;
#ifdef WIN32
  CRegistry registry;
  DWORD siz=1024;
  char buffer[1024];
  try {
    if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
				"SOFTWARE\\ORACLE\\HOME0",
				"TNS_ADMIN",
				buffer,siz)) {
      if (siz>0)
	str=buffer;
      else
	throw 0;
    } else
      throw 0;
  } catch(...) {
    try {
      if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
				  "SOFTWARE\\ORACLE\\HOME0",
				  "ORACLE_HOME",
				  buffer,siz)) {
	if (siz>0) {
	  str=buffer;
	  str+="\\network\\admin";
	}
      }
    } catch(...) {
    }
  }
#else
  if (!getenv("ORACLE_HOME"))
    throw QString("ORACLE_HOME environment variable not set");
  if (getenv("TNS_ADMIN")) {
    str=getenv("TNS_ADMIN");
  } else {
    str=getenv("ORACLE_HOME");
    str.append("/network/admin");
  }
#endif
  str.append("/tnsnames.ora");


  QFile file(str);
  if (!file.open(IO_ReadOnly)) {
    return;
  }
	    
  int size=file.size();
	    
  char *buf=new char[size+1];
  if (file.readBlock(buf,size)==-1) {
    delete buf;
    return;
  }

  buf[size]=0;

  char *begname=NULL;
  int pos=0;
  int param=0;
  while(pos<size) {
    if (buf[pos]=='#') {
      buf[pos]=0;
      while(pos<size&&buf[pos]!='\n')
	pos++;
    } else if (isspace(buf[pos])) {
      buf[pos]=0;
    } else if (buf[pos]=='=') {
      if (param==0) {
	buf[pos]=0;
	if (begname&&begname!=defdb) {
	  Database->insertItem(begname);
	}
      }
    } else if (buf[pos]=='(') {
      begname=NULL;
      param++;
    } else if (buf[pos]==')') {
      begname=NULL;
      param--;
    } else if (!begname) {
      begname=buf+pos;
    }
    pos++;
  }
  delete buf;
}

toConnection *toNewConnection::makeConnection(void)
{
  try {
    toTool::globalSetConfig(CONF_USER,Username->text());
    QString pass;
    if (toTool::globalConfig(CONF_SAVE_PWD,DEFAULT_SAVE_PWD)!=DEFAULT_SAVE_PWD)
      pass=Password->text();
    else
      pass=DEFAULT_SAVE_PWD;
    toTool::globalSetConfig(CONF_PASSWORD,pass);
    toTool::globalSetConfig(CONF_DATABASE,Database->currentText());
    toTool::globalSetConfig(CONF_SQLNET,SqlNet->isChecked()?"Yes":"");
    toConnection *retCon=new toConnection(SqlNet->isChecked(),
					  Username->text(),
					  Password->text(),
					  Database->currentText(),
					  Mode->currentText());
    {
      for(QListViewItem *item=Previous->firstChild();item;item=item->nextSibling()) {
	if (item->text(0)==Database->currentText()&&
	    item->text(1)==Username->text()&&
	    !item->text(2).isEmpty()==SqlNet->isChecked()) {
	  delete item;
	  break;
	}
      }
    }
    new QListViewItem(Previous,NULL,
		      Database->currentText(),
		      Username->text(),
		      SqlNet->isChecked()?"SQL*Net":"",
		      pass);
    {
      int siz=toTool::globalConfig(CONF_CONNECT_SIZE,DEFAULT_CONNECT_SIZE).toInt();
      int i=0;
      for(QListViewItem *item=Previous->firstChild();item&&i<siz;item=item->nextSibling()) {
	QString path=CONF_CONNECT_HISTORY;
	path+=":";
	path+=QString::number(i);
	QString tmp=path;
	tmp+=CONF_USER;
	toTool::globalSetConfig(tmp,item->text(1));

	tmp=path;
	tmp+=CONF_PASSWORD;
	toTool::globalSetConfig(tmp,item->text(3));

	tmp=path;
	tmp+=CONF_SQLNET;
	toTool::globalSetConfig(tmp,item->text(2));

	tmp=path;
	tmp+=CONF_DATABASE;
	toTool::globalSetConfig(tmp,item->text(0));
	i++;
      }
      toTool::globalSetConfig(CONF_CONNECT_CURRENT,QString::number(i));
    }

    return retCon;
  } catch (const otl_exception &exc) {
    QString str("Unable to connect to the database.\n");
    str.append(QString::fromUtf8((const char *)exc.msg));
    TOMessageBox::information(this,
			      "Unable to connect to the database",
			      str);
    return NULL;
  }
}

void toNewConnection::historySelection(void)
{
  QListViewItem *item=Previous->selectedItem();
  if (item) {
    Username->setText(item->text(1));
    if(item->text(3)!=DEFAULT_PASSWORD)
      Password->setText(item->text(3));
    Database->lineEdit()->setText(item->text(0));
    SqlNet->setChecked(!item->text(2).isEmpty());
  }
}
