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

#include <qframe.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include "topreferences.h"
#include "toglobalsetting.h"
#include "tosyntaxsetup.h"
#include "tohelp.h"

#include "topreferences.moc"

#define TAB_PLACE_X 170
#define TAB_PLACE_Y 10
#define TAB_WIDTH 400
#define TAB_HEIGHT 400

void toPreferences::addWidget(QListBoxItem *item,QWidget *widget)
{
  widget->setGeometry(TAB_PLACE_X,TAB_PLACE_Y,TAB_WIDTH,TAB_HEIGHT);
  Tabs[item]=widget;
  if (!Shown)
    Shown=widget;
  else
    widget->hide();
}

void toPreferences::selectTab(QListBoxItem *item)
{
  QWidget *tab=Tabs[item];
  if (tab) {
    if (Shown)
      Shown->hide();
    tab->show();
    Shown=tab;
  }
}

void toPreferences::displayPreferences(QWidget *parent)
{
  toPreferences *dialog=new toPreferences(parent,"Options",true);
  if (dialog->exec())
    dialog->saveSetting();
  delete dialog;
}

void toPreferences::saveSetting(void)
{
  
  for (std::map<QListBoxItem *,QWidget *>::iterator i=Tabs.begin();i!=Tabs.end();i++) {
    toSettingTab *tab=dynamic_cast<toSettingTab *>((*i).second);
    if (tab)
      tab->saveSetting();
  }
  toTool::saveConfig();
}

toPreferences::toPreferences(QWidget* parent,const char* name,bool modal,WFlags fl)
  : QDialog(parent,name,modal,fl),toHelpContext("preferences.html")
{
  toHelp::connectDialog(this);
  Shown=NULL;
  if (!name)
    setName("toPreferences");
  resize(580,470); 
  setCaption(tr("Settings" ));
  setMinimumSize(QSize(580,470));
  setMaximumSize(QSize(580,470));
  
  TabSelection = new QListBox(this,"TabSelection");
  TabSelection->setGeometry(QRect(10,10,141,401)); 
  
  QListBoxText *item;
  item=new QListBoxText(TabSelection,"Global Settings");
  addWidget(item,new toGlobalSetting(this));

  item=new QListBoxText(TabSelection,"Database Settings");
  addWidget(item,new toDatabaseSetting(this));

  item=new QListBoxText(TabSelection,"Font Settings");
  addWidget(item,new toSyntaxSetup(this));
  
  item=new QListBoxText(TabSelection,"Tools");
  addWidget(item,new toToolSetting(this));

  TabSelection->setCurrentItem(0);

  std::map<QString,toTool *> tools=toTool::tools();
  std::map<QString,toTool *> newSort;
  {
    for (std::map<QString,toTool *>::iterator i=tools.begin();i!=tools.end();i++)
      newSort[(*i).second->name()]=(*i).second;
  }

  {
    for (std::map<QString,toTool *>::iterator i=newSort.begin();i!=newSort.end();i++) {
      QWidget *tab=(*i).second->configurationTab(this);
      if (tab) {
	QString str(" ");
	str.append((*i).first);
	addWidget(new QListBoxText(TabSelection,str),tab);
      }
    }
  }

  CancelButton = new QPushButton(this,"CancelButton");
  CancelButton->move(460,430); 
  CancelButton->setText(tr("Cancel" ));
  CancelButton->setDefault(false);
  
  OkButton = new QPushButton(this,"OkButton");
  OkButton->move(330,430); 
  OkButton->setText(tr("&OK" ));
  OkButton->setDefault(true);
  
  // tab order
  setTabOrder(TabSelection,OkButton);
  setTabOrder(OkButton,CancelButton);
  
  connect(OkButton,SIGNAL(clicked()),this,SLOT(accept()));
  connect(CancelButton,SIGNAL(clicked()),this,SLOT(reject()));
  connect(TabSelection,SIGNAL(currentChanged(QListBoxItem *)),this,SLOT(selectTab(QListBoxItem *)));
}
