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

#include "addrollback.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

toRollbackDialog::toRollbackDialog(toConnection &Connection,QWidget* parent,const char* name,bool modal,WFlags fl)
  : QDialog(parent,name,modal,fl)
{
  if (!name)
    setName("Form1");
  resize(470,500); 
  setMinimumSize(QSize(470,500));
  setMaximumSize(QSize(470,500));
  setCaption(tr("Form1"));
  
  DialogTabs=new QTabWidget(this,"DialogTabs");
  DialogTabs->setGeometry(QRect(10,10,330,480)); 

  RollbackInfo=new QWidget(DialogTabs,"RollbackInfo");
  
  Name=new QLineEdit(RollbackInfo,"Name");
  Name->setGeometry(QRect(10,30,310,23)); 
  connect(Name,SIGNAL(textChanged(const QString &)),
	  this,SLOT(valueChanged(const QString &)));
  
  TextLabel1=new QLabel(RollbackInfo,"TextLabel1");
  TextLabel1->setGeometry(QRect(10,10,310,16)); 
  TextLabel1->setText(tr("&Name"));
  QToolTip::add( TextLabel1,tr("Name of this rollback segment."));
  
  Tablespace=new QComboBox(FALSE,RollbackInfo,"Tablespace");
  Tablespace->setGeometry(QRect(10,90,310,27)); 
  Tablespace->setEditable(TRUE);
  DialogTabs->addTab(RollbackInfo,tr("Rollback Segment"));
  connect(Tablespace,SIGNAL(textChanged(const QString &)),
	  this,SLOT(valueChanged(const QString &)));

  try {
    otl_stream q(1,
		 "select tablespace_name from dba_tablespaces order by tablespace_name",
		 Connection.connection());
    char buf[100];
    while(!q.eof()) {
      q>>buf;
      Tablespace->insertItem(buf);
    }
  } catch (...) {

  }
  
  CheckBox1=new QCheckBox(RollbackInfo,"CheckBox1");
  CheckBox1->setGeometry(QRect(10,130,310,19)); 
  CheckBox1->setText(tr("&Public"));
  QToolTip::add( CheckBox1,tr("If this should be a publicly available rollback segment."));
  
  TextLabel1_2=new QLabel(RollbackInfo,"TextLabel1_2");
  TextLabel1_2->setGeometry(QRect(10,70,310,16)); 
  TextLabel1_2->setText(tr("&Tablespace"));
  QToolTip::add( TextLabel1_2,tr("Name of this rollback segment."));
  
  // buddies
  TextLabel1->setBuddy(Name);
  TextLabel1_2->setBuddy(Tablespace);

  Storage=new toStorageDefinition(DialogTabs);
  DialogTabs->addTab(Storage,"Storage");

  OkButton = new QPushButton(this, "OkButton" );
  OkButton->move(350, 40); 
  OkButton->setText( tr( "&OK"  ) );
  OkButton->setDefault( TRUE );

  QPushButton *CancelButton = new QPushButton(this, "CancelButton" );
  CancelButton->move(350, 90); 
  CancelButton->setText( tr( "Cancel"  ) );
  CancelButton->setDefault( FALSE );
  
  connect(OkButton,SIGNAL(clicked()),this,SLOT(accept()));
  connect(CancelButton,SIGNAL(clicked()),this,SLOT(reject()));

  OkButton->setEnabled(false);
}

void toRollbackDialog::valueChanged(const QString &str)
{
  if (Name.isEmpty()||
      Tablespace.isEmpty())
    OkButton->setEnabled(false);
  else
    OkButton->setEnabled(true);
}

/*  
 *  Destroys the object and frees any allocated resources
 */
Form1::~Form1()
{
    // no need to delete child widgets,Qt does it all for us
}

