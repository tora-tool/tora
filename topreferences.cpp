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

#include <qframe.h>
#include <qlistbox.h>
#include <qlayout.h>

#include "topreferences.h"
#include "toglobalsetting.h"
#include "tosyntaxsetup.h"
#include "tohelp.h"
#include "topreferencesui.h"

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
  : toPreferencesUI(parent,name,modal,fl),toHelpContext("preferences.html")
{
  toHelp::connectDialog(this);
  Shown=NULL;
  
  Layout=new QVBoxLayout(Parent);

  QListBoxText *item;
  item=new QListBoxText(TabSelection,"Global Settings");
  addWidget(item,new toGlobalSetting(Parent));

  item=new QListBoxText(TabSelection,"Database Settings");
  addWidget(item,new toDatabaseSetting(Parent));

  item=new QListBoxText(TabSelection,"Font Settings");
  addWidget(item,new toSyntaxSetup(Parent));
  
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
