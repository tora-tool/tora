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
#include "toglobalsetting.h"
#include "tomain.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"

#ifdef TO_KDE
#include <kfiledialog.h>
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qdir.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtextcodec.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qvariant.h>
#include <qwhatsthis.h>

#include "todatabasesettingui.moc"
#include "toglobalsettingui.moc"
#include "totoolsettingui.moc"

toGlobalSetting::toGlobalSetting(QWidget *parent,const char *name,WFlags fl)
  : toGlobalSettingUI(parent,name,fl), toSettingTab("preferences.html#global")
{
  SavePassword->setChecked(!toTool::globalConfig(CONF_SAVE_PWD,"").isEmpty());
  DesktopAware->setChecked(!toTool::globalConfig(CONF_DESKTOP_AWARE,"Yes").isEmpty());
  ToolsLeft->setChecked(!toTool::globalConfig(CONF_TOOLS_LEFT,"Yes").isEmpty());
  toRefreshCreate(OptionGroup,TO_KDE_TOOLBAR_WIDGET,QString::null,Refresh);
  DefaultSession->setText(toTool::globalConfig(CONF_DEFAULT_SESSION,DEFAULT_SESSION));
  Status->setValue(toTool::globalConfig(CONF_STATUS_MESSAGE,
					DEFAULT_STATUS_MESSAGE).toInt());
  HistorySize->setValue(toTool::globalConfig(CONF_STATUS_SAVE,
					     DEFAULT_STATUS_SAVE).toInt());
  IncludeDB->setChecked(!toTool::globalConfig(CONF_DB_TITLE,"Yes").isEmpty());
  MaximizeMain->setChecked(!toTool::globalConfig(CONF_MAXIMIZE_MAIN,"Yes").isEmpty());
  Statusbar->setChecked(!toTool::globalConfig(CONF_MESSAGE_STATUSBAR,"").isEmpty());
  RestoreSession->setChecked(!toTool::globalConfig(CONF_RESTORE_SESSION,"").isEmpty());
  HelpDirectory->setText(toHelpPath());
  ChangeConnection->setChecked(!toTool::globalConfig(CONF_CHANGE_CONNECTION,"Yes").isEmpty());
  ConnectHistory->setValue(toTool::globalConfig(CONF_CONNECT_SIZE,DEFAULT_CONNECT_SIZE).toInt());
  int samples=toTool::globalConfig(CONF_CHART_SAMPLES,DEFAULT_CHART_SAMPLES).toInt();
  if (samples<0) {
    UnlimitedSamples->setChecked(true);
    ChartSamples->setValue(QString::fromLatin1(DEFAULT_CHART_SAMPLES).toInt());
  } else
    ChartSamples->setValue(samples);
  samples=toTool::globalConfig(CONF_DISPLAY_SAMPLES,DEFAULT_DISPLAY_SAMPLES).toInt();
  if (samples<0) {
    AllSamples->setChecked(true);
    DisplaySamples->setValue(ChartSamples->value());
  } else
    DisplaySamples->setValue(samples);
  DefaultFormat->setCurrentItem(toTool::globalConfig(CONF_DEFAULT_FORMAT,"").toInt());

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
  Style->insertStringList(toGetSessionTypes());
  QString str=toGetSessionType();
  for (int i=0;i<Style->count();i++) {
    if (str==Style->text(i)) {
      Style->setCurrentItem(i);
      break;
    }
  }
#else
  Style->hide();
  StyleLabel->hide();
#endif

#ifdef TO_KDE
  DockToolbar->hide();
  DesktopAware->hide();
#endif
#ifdef WIN32
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
  
  /** disk caching options
   */  
  
  QString home=QDir::homeDirPath();
  QString dirname = toTool::globalConfig(CONF_CACHE_DIR, "");
  if (dirname == "")
    dirname = QString(home+"/.tora_cache"); 
     
  CacheDirectory->setText(dirname);
  
  /** default is off
   */

  if (toTool::globalConfig(CONF_CACHE_DISK,"No") == "Yes") 
    DiskCaching->setChecked(true);   
  else 
    DiskCaching->setChecked(false);   
  
  CustomSQL->setText(toTool::globalConfig(CONF_SQL_FILE,
					  DEFAULT_SQL_FILE));
#if QT_VERSION >= 300
  UpgradeCheck->setChecked(!toTool::globalConfig(CONF_UPGRADE_CHECK,"").isEmpty());
#else
  UpgradeCheck->hide();
#endif

  Locale->setText(toTool::globalConfig(CONF_LOCALE,QTextCodec::locale()));
}

void toGlobalSetting::pluginBrowse(void)
{
  QString str=TOFileDialog::getExistingDirectory(PluginDirectory->text(),this);
  if (!str.isEmpty())
    PluginDirectory->setText(str);
}

void toGlobalSetting::sqlBrowse(void)
{
  QString str=toOpenFilename(CustomSQL->text(),QString::null,this);
  if (!str.isEmpty())
    CustomSQL->setText(str);
}

void toGlobalSetting::sessionBrowse(void)
{
  QString str=toOpenFilename(DefaultSession->text(),QString::fromLatin1("*.tse"),this);
  if (!str.isEmpty())
    DefaultSession->setText(str);
}

void toGlobalSetting::helpBrowse(void)
{
  QString str=toOpenFilename(HelpDirectory->text(),QString::fromLatin1("toc.htm*"),this);
  if (!str.isEmpty())
    HelpDirectory->setText(str);
}

void toGlobalSetting::saveSetting(void)
{
  if (!toMonolithic())
    toTool::globalSetConfig(CONF_PLUGIN_DIR,PluginDirectory->text());
  toTool::globalSetConfig(CONF_CACHE_DISK,DiskCaching->isChecked()?"Yes":"No");
  toTool::globalSetConfig(CONF_CACHE_DIR,CacheDirectory->text());
  toTool::globalSetConfig(CONF_SQL_FILE,CustomSQL->text());
  toTool::globalSetConfig(CONF_DEFAULT_SESSION,DefaultSession->text());
  toTool::globalSetConfig(CONF_REFRESH,Refresh->currentText());
  toTool::globalSetConfig(CONF_SAVE_PWD,SavePassword->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_DESKTOP_AWARE,DesktopAware->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_STATUS_MESSAGE,QString::number(Status->value()));
  toTool::globalSetConfig(CONF_STATUS_SAVE,QString::number(HistorySize->value()));
  toTool::globalSetConfig(CONF_CHART_SAMPLES,QString::number(ChartSamples->value()));
  toTool::globalSetConfig(CONF_CONNECT_SIZE,QString::number(ConnectHistory->value()));
  toTool::globalSetConfig(CONF_MAXIMIZE_MAIN,MaximizeMain->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_MESSAGE_STATUSBAR,Statusbar->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_RESTORE_SESSION,RestoreSession->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_TOOLS_LEFT,ToolsLeft->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_DEFAULT_FORMAT,
			  QString::number(DefaultFormat->currentItem()));
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
  if (AllSamples->isChecked())
    toTool::globalSetConfig(CONF_DISPLAY_SAMPLES,"-1");
  else
    toTool::globalSetConfig(CONF_DISPLAY_SAMPLES,QString::number(DisplaySamples->value()));
  if (UnlimitedSamples->isChecked())
    toTool::globalSetConfig(CONF_CHART_SAMPLES,"-1");
  else
    toTool::globalSetConfig(CONF_CHART_SAMPLES,QString::number(ChartSamples->value()));

#if QT_VERSION >= 300
  toTool::globalSetConfig(CONF_UPGRADE_CHECK,UpgradeCheck->isChecked()?"Yes":"");
#endif

  toTool::globalSetConfig(CONF_LOCALE,Locale->text());
}

void toDatabaseSetting::numberFormatChange()
{
  Decimals->setEnabled(NumberFormat->currentItem()==2);
}

toDatabaseSetting::toDatabaseSetting(QWidget *parent,const char *name,WFlags fl)
  : toDatabaseSettingUI(parent,name,fl),toSettingTab("database.html")
{
  MaxColDisp->setText(toTool::globalConfig(CONF_MAX_COL_DISP,
					   DEFAULT_MAX_COL_DISP));
  QString str=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER);
  if (str.toInt()<=0)
    ReadAll->setChecked(true);
  else
    InitialFetch->setText(str);

  str=toTool::globalConfig(CONF_MAX_CONTENT,DEFAULT_MAX_CONTENT);
  if (str.toInt()<=0) {
    MaxContent->setText(InitialFetch->text());
    UnlimitedContent->setChecked(true);
  } else
    MaxContent->setText(str);

  MaxColDisp->setValidator(new QIntValidator(MaxColDisp));
  InitialFetch->setValidator(new QIntValidator(InitialFetch));
  MaxContent->setValidator(new QIntValidator(InitialFetch));

  Decimals->setValue(toTool::globalConfig(CONF_NUMBER_DECIMALS,DEFAULT_NUMBER_DECIMALS).toInt());
  NumberFormat->setCurrentItem(toTool::globalConfig(CONF_NUMBER_FORMAT,DEFAULT_NUMBER_FORMAT).toInt());
  if (NumberFormat->currentItem()==2)
    Decimals->setEnabled(true);

  AutoCommit->setChecked(!toTool::globalConfig(CONF_AUTO_COMMIT,"").isEmpty());
  DontReread->setChecked(!toTool::globalConfig(CONF_DONT_REREAD,"Yes").isEmpty());
  ObjectCache->setCurrentItem(toTool::globalConfig(CONF_OBJECT_CACHE,"0").toInt());
  BkgndConnect->setChecked(!toTool::globalConfig(CONF_BKGND_CONNECT,"").isEmpty());
  IndicateEmpty->setChecked(!toTool::globalConfig(CONF_INDICATE_EMPTY,"").isEmpty());
  int val=toTool::globalConfig(CONF_AUTO_LONG,"0").toInt();
  AutoLong->setChecked(val);
  MoveAfter->setValue(val);
  KeepAlive->setChecked(!toTool::globalConfig(CONF_KEEP_ALIVE,"").isEmpty());
}

void toUpdateIndicateEmpty(void);

void toDatabaseSetting::saveSetting(void)
{
  toTool::globalSetConfig(CONF_MAX_COL_DISP,MaxColDisp->text());
  if (ReadAll->isChecked())
    toTool::globalSetConfig(CONF_MAX_NUMBER,"-1");
  else
    toTool::globalSetConfig(CONF_MAX_NUMBER,InitialFetch->text());
  if (UnlimitedContent->isChecked())
    toTool::globalSetConfig(CONF_MAX_CONTENT,"-1");
  else {
    int num=InitialFetch->text().toInt();
    int maxnum=MaxContent->text().toInt();
    if (num<0)
      maxnum=num;
    else if (num>=maxnum)
      maxnum=num+1;
    if (maxnum!=MaxContent->text().toInt())
      TOMessageBox::information(this,tr("Invalid values"),
				tr("Doesn't make sense to have max content less than initial\n"
				   "fetch size. Will adjust value to be higher."),
				tr("&Ok"));
    toTool::globalSetConfig(CONF_MAX_CONTENT,QString::number(maxnum));
  }
  toTool::globalSetConfig(CONF_AUTO_COMMIT,AutoCommit->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_DONT_REREAD,DontReread->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_OBJECT_CACHE,QString::number(ObjectCache->currentItem()));
  toTool::globalSetConfig(CONF_BKGND_CONNECT,BkgndConnect->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_AUTO_LONG,
			  AutoLong->isChecked()?MoveAfter->cleanText():QString::fromLatin1("0"));
  toTool::globalSetConfig(CONF_INDICATE_EMPTY,IndicateEmpty->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_KEEP_ALIVE,KeepAlive->isChecked()?DEFAULT_KEEP_ALIVE:"");

  toTool::globalSetConfig(CONF_NUMBER_FORMAT,QString::number(NumberFormat->currentItem()));
  toTool::globalSetConfig(CONF_NUMBER_DECIMALS,QString::number(Decimals->value()));
  toQValue::setNumberFormat(NumberFormat->currentItem(),Decimals->value());

  toMainWidget()->updateKeepAlive();
  toUpdateIndicateEmpty();
}

toToolSetting::toToolSetting(QWidget *parent,const char *name,WFlags fl)
  : toToolSettingUI(parent,name,fl),toSettingTab("toolsetting.html")
{
  std::map<QCString,toTool *> &tools=toTool::tools();

  QString first;
  Enabled->setSorting(0);
  for (std::map<QCString,toTool *>::iterator i=tools.begin();i!=tools.end();i++) {
    if ((*i).second->menuItem()) {
      QString menuName=qApp->translate("toTool",(*i).second->menuItem());
      if(first.isEmpty())
	first=menuName;

      new QListViewItem(Enabled,menuName,(*i).second->name(),(*i).first);
    }
  }

  for(QListViewItem *item=Enabled->firstChild();item;item=item->nextSibling()) {
    QCString tmp=item->text(2).latin1();
    tmp+=CONF_TOOL_ENABLE;
    if(!toTool::globalConfig(tmp,"Yes").isEmpty())
      item->setSelected(true);
  }

  DefaultTool->insertItem(first);
  changeEnable();
}

void toToolSetting::changeEnable(void)
{
  QString str=DefaultTool->currentText();

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
    QCString str=item->text(2).latin1();
    str+=CONF_TOOL_ENABLE;
    toTool::globalSetConfig(str,item->isSelected()?"Yes":"");

    if (DefaultTool->currentText()==item->text(0))
      toTool::globalSetConfig(CONF_DEFAULT_TOOL,item->text(2));
  }
}
