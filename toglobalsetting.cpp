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

#ifdef TO_KDE
#include <kfiledialog.h>
#endif

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfontdialog.h>
#include <qcombobox.h>
#include <qvalidator.h>
#include <qfiledialog.h>

#include "toconf.h"
#include "totool.h"
#include "tomain.h"
#include "toglobalsetting.h"
#include "tosql.h"
#include "toconnection.h"
#include "toresultview.h"

#include "toglobalsettingui.moc"
#include "todatabasesettingui.moc"
#include "totoolsettingui.moc"

toGlobalSetting::toGlobalSetting(QWidget *parent,const char *name,WFlags fl)
  : toGlobalSettingUI(parent,name,fl), toSettingTab("preferences.html#global")
{
  SavePassword->setChecked(!toTool::globalConfig(CONF_SAVE_PWD,"").isEmpty());
  DesktopAware->setChecked(!toTool::globalConfig(CONF_DESKTOP_AWARE,"Yes").isEmpty());
  toRefreshCreate(OptionGroup,NULL,NULL,Refresh);
  ListScale->setText(toTool::globalConfig(CONF_LIST_SCALE,DEFAULT_LIST_SCALE));
  Status->setValue(toTool::globalConfig(CONF_STATUS_MESSAGE,
					DEFAULT_STATUS_MESSAGE).toInt());
  HistorySize->setValue(toTool::globalConfig(CONF_STATUS_SAVE,
					     DEFAULT_STATUS_SAVE).toInt());
  IncludeDB->setChecked(!toTool::globalConfig(CONF_DB_TITLE,"Yes").isEmpty());
  MaximizeMain->setChecked(!toTool::globalConfig(CONF_MAXIMIZE_MAIN,"Yes").isEmpty());
  Statusbar->setChecked(!toTool::globalConfig(CONF_MESSAGE_STATUSBAR,"").isEmpty());
  HelpDirectory->setText(toHelpPath());
  ChangeConnection->setChecked(!toTool::globalConfig(CONF_CHANGE_CONNECTION,"Yes").isEmpty());
  ConnectHistory->setValue(toTool::globalConfig(CONF_CONNECT_SIZE,DEFAULT_CONNECT_SIZE).toInt());
  ChartSamples->setValue(toTool::globalConfig(CONF_CHART_SAMPLES,DEFAULT_CHART_SAMPLES).toInt());

  QString typ=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
  if (typ=="KB")
    SizeUnit->setCurrentItem(1);
  else if (typ=="MB")
    SizeUnit->setCurrentItem(2);

#ifdef ENABLE_QT_XFT
  AntialiaseFonts->setChecked(toTool::globalConfig(CONF_QT_XFT,DEFAULT_QT_XFT)=="true");
#else
  AntialiaseFonts->hide();
#endif

#ifdef ENABLE_STYLE
  Style->insertItem("CDE");
  Style->insertItem("Motif");
  Style->insertItem("Motif Plus");
  Style->insertItem("Platinum");
  Style->insertItem("SGI");
  Style->insertItem("Windows");
  QString str=toGetSessionType();
  if (str=="CDE")
    Style->setCurrentItem(0);
  else if (str=="Motif")
    Style->setCurrentItem(1);
  else if (str=="Motif Plus")
    Style->setCurrentItem(2);
  else if (str=="Platinum")
    Style->setCurrentItem(3);
  else if (str=="SGI")
    Style->setCurrentItem(4);
  else if (str=="Windows")
    Style->setCurrentItem(5);
#else
  Style->hide();
  StyleLabel->hide();
#endif

#ifdef TO_KDE
  DockToolbar->hide();
  DesktopAware->hide();
#endif
#if QT_VERSION < 300
  DockToolbar->setChecked(toTool::globalConfig(CONF_DOCK_TOOLBAR,"Yes"));
#else
  DockToolbar->hide();
#endif

  if (toMonolithic()) {
    PluginLabel->hide();
    PluginDirectory->hide();
    PluginBrowse->hide();
  } else {
    PluginDirectory->setText(toTool::globalConfig(CONF_PLUGIN_DIR,
						  DEFAULT_PLUGIN_DIR));
  }

  CustomSQL->setText(toTool::globalConfig(CONF_SQL_FILE,
					  DEFAULT_SQL_FILE));

  ListScale->setValidator(new QDoubleValidator(ListScale));
}

void toGlobalSetting::pluginBrowse(void)
{
  QString str=TOFileDialog::getExistingDirectory(PluginDirectory->text(),this);
  if (!str.isEmpty())
    PluginDirectory->setText(str);
}

void toGlobalSetting::sqlBrowse(void)
{
  QString str=toOpenFilename(CustomSQL->text(),"*.sql",this);
  if (!str.isEmpty())
    CustomSQL->setText(str);
}

void toGlobalSetting::helpBrowse(void)
{
  QString str=toOpenFilename(HelpDirectory->text(),"toc.htm*",this);
  if (!str.isEmpty())
    HelpDirectory->setText(str);
}

void toGlobalSetting::saveSetting(void)
{
  if (!toMonolithic())
    toTool::globalSetConfig(CONF_PLUGIN_DIR,PluginDirectory->text());
  toTool::globalSetConfig(CONF_SQL_FILE,CustomSQL->text());
  toTool::globalSetConfig(CONF_REFRESH,Refresh->currentText());
  toTool::globalSetConfig(CONF_LIST_SCALE,ListScale->text());
  toTool::globalSetConfig(CONF_SAVE_PWD,SavePassword->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_DESKTOP_AWARE,DesktopAware->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_STATUS_MESSAGE,QString::number(Status->value()));
  toTool::globalSetConfig(CONF_STATUS_SAVE,QString::number(HistorySize->value()));
  toTool::globalSetConfig(CONF_CHART_SAMPLES,QString::number(ChartSamples->value()));
  toTool::globalSetConfig(CONF_CONNECT_SIZE,QString::number(ConnectHistory->value()));
  toTool::globalSetConfig(CONF_MAXIMIZE_MAIN,MaximizeMain->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_MESSAGE_STATUSBAR,Statusbar->isChecked()?"Yes":"");
#if QT_VERSION < 300
  toTool::globalSetConfig(CONF_DOCK_TOOLBAR,DockToolbar->isChecked()?"Yes":"");
#endif
  toTool::globalSetConfig(CONF_CHANGE_CONNECTION,ChangeConnection->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_DB_TITLE,IncludeDB->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_SIZE_UNIT,SizeUnit->currentText());
  toTool::globalSetConfig(CONF_HELP_PATH,HelpDirectory->text());
#ifdef ENABLE_STYLE
  toTool::globalSetConfig(CONF_STYLE,Style->currentText());
  toSetSessionType(Style->currentText());
#endif
#ifdef ENABLE_QT_XFT
  toTool::globalSetConfig(CONF_QT_XFT,AntialiaseFonts->isChecked()?"true":"false");
#endif
}

toDatabaseSetting::toDatabaseSetting(QWidget *parent,const char *name,WFlags fl)
  : toDatabaseSettingUI(parent,name,fl),toSettingTab("database.html")
{
  MaxColDisp->setText(toTool::globalConfig(CONF_MAX_COL_DISP,
					   DEFAULT_MAX_COL_DISP));
  QString str=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER);
  if (str.toInt()<0)
    ReadAll->setChecked(true);
  else
    InitialFetch->setText(str);

  MaxColDisp->setValidator(new QIntValidator(MaxColDisp));
  InitialFetch->setValidator(new QIntValidator(InitialFetch));

  AutoCommit->setChecked(!toTool::globalConfig(CONF_AUTO_COMMIT,"").isEmpty());
  DontReread->setChecked(!toTool::globalConfig(CONF_DONT_REREAD,"").isEmpty());
  int val=toTool::globalConfig(CONF_AUTO_LONG,"0").toInt();
  AutoLong->setChecked(val);
  MoveAfter->setValue(val);
}

void toDatabaseSetting::saveSetting(void)
{
  toTool::globalSetConfig(CONF_MAX_COL_DISP,MaxColDisp->text());
  if (ReadAll->isChecked())
    toTool::globalSetConfig(CONF_MAX_NUMBER,"-1");
  else
    toTool::globalSetConfig(CONF_MAX_NUMBER,InitialFetch->text());
  toTool::globalSetConfig(CONF_AUTO_COMMIT,AutoCommit->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_DONT_REREAD,DontReread->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_AUTO_LONG,
			  AutoLong->isChecked()?MoveAfter->cleanText():QString("0"));
}

toToolSetting::toToolSetting(QWidget *parent,const char *name,WFlags fl)
  : toToolSettingUI(parent,name,fl),toSettingTab("toolsetting.html")
{
  std::map<QString,toTool *> &tools=toTool::tools();

  Enabled->setSorting(0);
  for (std::map<QString,toTool *>::iterator i=tools.begin();i!=tools.end();i++) {
    const char *menuName=(*i).second->menuItem();
    if(FirstTool.isEmpty())
      FirstTool=menuName;

    if (menuName)
      new QListViewItem(Enabled,menuName,(*i).second->name());
  }

  for(QListViewItem *item=Enabled->firstChild();item;item=item->nextSibling()) {
    QString tmp=item->text(1);
    tmp+=CONF_TOOL_ENABLE;
    if(!toTool::globalConfig(tmp,"Yes").isEmpty())
      item->setSelected(true);
  }

  changeEnable();
}

void toToolSetting::changeEnable(void)
{
  QString str=DefaultTool->currentText();
  if (str.isEmpty()) {
    str=toTool::globalConfig(CONF_DEFAULT_TOOL,FirstTool);
  }

  DefaultTool->clear();
  int id=0;
  int sel=0;
  for(QListViewItem *item=Enabled->firstChild();item;item=item->nextSibling()) {
    if (item->isSelected()) {
      DefaultTool->insertItem(item->text(0),id);
      if (item->text(0)==str)
	sel=id;
      id++;
    }
  }
  DefaultTool->setCurrentItem(sel);
}

void toToolSetting::saveSetting(void)
{
  for(QListViewItem *item=Enabled->firstChild();item;item=item->nextSibling()) {
    QString str=item->text(1);
    str+=CONF_TOOL_ENABLE;
    toTool::globalSetConfig(str,item->isSelected()?"Yes":"");
  }
  toTool::globalSetConfig(CONF_DEFAULT_TOOL,DefaultTool->currentText());
}
