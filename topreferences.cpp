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

#include <qframe.h>
#include <qlistbox.h>
#include <qlayout.h>

#include "topreferences.h"
#include "toglobalsetting.h"
#include "tosyntaxsetup.h"
#include "tohelp.h"
#include "topreferencesui.h"
#include "toconnection.h"

#include "topreferences.moc"
#include "topreferencesui.moc"

void toPreferences::addWidget(QListBoxItem *item,QWidget *widget)
{
  Layout->addWidget(widget);
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
  toPreferences dialog(parent,"Options",true);
  if (dialog.exec())
    dialog.saveSetting();
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
  : toPreferencesUI(parent,name,modal,fl),toHelpContext("preferences.html")
{
  toHelp::connectDialog(this);
  Shown=NULL;
  
  Layout=new QVBoxLayout(Parent);

  QListBoxText *item;
  item=new QListBoxText(TabSelection,"Global Settings");
  addWidget(item,new toGlobalSetting(Parent));

  item=new QListBoxText(TabSelection,"Editor Settings");
  addWidget(item,new toSyntaxSetup(Parent));
  
  item=new QListBoxText(TabSelection,"Database Settings");
  addWidget(item,new toDatabaseSetting(Parent));

  std::list<QString> prov=toConnectionProvider::providers();
  for (std::list<QString>::iterator i=prov.begin();i!=prov.end();i++) {
    QWidget *tab=toConnectionProvider::configurationTab(*i,Parent);
    if (tab) {
      QString str(" ");
      str.append(*i);
      addWidget(new QListBoxText(TabSelection,str),tab);
    }
  }

  item=new QListBoxText(TabSelection,"Tools");
  addWidget(item,new toToolSetting(Parent));

  TabSelection->setCurrentItem(0);

  std::map<QString,toTool *> tools=toTool::tools();
  std::map<QString,toTool *> newSort;
  {
    for (std::map<QString,toTool *>::iterator i=tools.begin();i!=tools.end();i++)
      newSort[(*i).second->name()]=(*i).second;
  }

  {
    for (std::map<QString,toTool *>::iterator i=newSort.begin();i!=newSort.end();i++) {
      QWidget *tab=(*i).second->configurationTab(Parent);
      if (tab) {
	QString str(" ");
	str.append((*i).first);
	addWidget(new QListBoxText(TabSelection,str),tab);
      }
    }
  }
}

void toPreferences::help(void)
{
  if (Shown)
    Shown->setFocus();
  toHelp::displayHelp(this);
}
