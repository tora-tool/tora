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


#include "tonewconnection.h"

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

#include "tonewconnection.moc"
#include "totool.h"
#include "toconf.h"

toNewConnection::toNewConnection(QWidget* parent, const char* name,bool modal,WFlags fl)
  : QDialog(parent,name,modal,fl)
{
  if (!name)
    setName("toNewConnection");
  resize(241,201); 
  setMinimumSize(241,201);
  setMaximumSize(241,201);
  setCaption(tr("New database connection"));

  TextLabel1=new QLabel(this,"TextLabel1");
  TextLabel1->setGeometry(QRect(10,0,61,20)); 
  TextLabel1->setText(tr("Username"));

  TextLabel2=new QLabel(this,"TextLabel2");
  TextLabel2->setGeometry(QRect(10,50,61,20)); 
  TextLabel2->setText(tr("Password"));

  TextLabel3=new QLabel(this,"TextLabel3");
  TextLabel3->setGeometry(QRect(10,100,61,20)); 
  TextLabel3->setText(tr("Database"));

  User=new QLineEdit(this,"User");
  User->setGeometry(QRect(10,20,220,23)); 
  User->setText(toTool::globalConfig(CONF_USER,DEFAULT_USER));
  User->setFocus();

  Password=new QLineEdit(this,"Password");
  Password->setGeometry(QRect(10,70,220,23)); 
  Password->setAutoMask(false);
  if (toTool::globalConfig(CONF_PASSWORD,DEFAULT_PASSWORD).isEmpty())
    Password->setText(DEFAULT_PASSWORD);
  else
    Password->setText(toTool::globalConfig(CONF_PASSWORD,DEFAULT_PASSWORD));
  Password->setEchoMode(QLineEdit::Password);

  Database=new QComboBox(false,this,"Database");
  Database->setGeometry(QRect(10,120,220,27)); 
  QString defdb=toTool::globalConfig(CONF_DATABASE,DEFAULT_DATABASE);
  Database->setEditable(true);
  Database->setAutoCompletion(true);
  if (!defdb.isEmpty())
    Database->insertItem(defdb);

  OkButton=new QPushButton(this,"OkButton");
  OkButton->move(10,160); 
  OkButton->setText(tr("&Connect"));
  OkButton->setDefault(true);

  CancelButton=new QPushButton(this,"CancelButton");
  CancelButton->move(130,160); 
  CancelButton->setText(tr("Cancel"));

  connect(OkButton,SIGNAL(clicked()),SLOT(accept()));
  connect(CancelButton,SIGNAL(clicked()),SLOT(reject()));

  if (!getenv("ORACLE_HOME"))
    return;

  QString str(getenv("ORACLE_HOME"));
  str.append("/network/admin/tnsnames.ora");

  QFile file(str);
  if (!file.open(IO_ReadOnly)) {
    return;
  }
	    
  int size=file.size();
	    
  char buf[size+1];
  if (file.readBlock(buf,size)==-1)
    return;

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
}

toNewConnection::~toNewConnection()
{
}

toConnection *toNewConnection::makeConnection(void)
{
  try {
    toTool::globalSetConfig(CONF_USER,User->text());
    if (toTool::globalConfig(CONF_SAVE_PWD,DEFAULT_SAVE_PWD)!=DEFAULT_SAVE_PWD)
      toTool::globalSetConfig(CONF_PASSWORD,Password->text());
    else
      toTool::globalSetConfig(CONF_PASSWORD,DEFAULT_SAVE_PWD);
    toTool::globalSetConfig(CONF_DATABASE,Database->currentText());
    toConnection *retCon=new toConnection(User->text(),Password->text(),Database->currentText());
    return retCon;
  } catch (const otl_exception &exc) {
    QString str("Unable to connect to the database.\n");
    str.append((const char *)exc.msg);
    QMessageBox::information(this,"Unable to connect to the database",
			     str);
    return NULL;
  }
}

