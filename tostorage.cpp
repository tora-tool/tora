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
#include <kmenubar.h>
#endif

#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qworkspace.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qmenubar.h>

#include "tomemoeditor.h"
#include "tochangeconnection.h"
#include "tostorage.h"
#include "tostoragedefinition.h"
#include "toresultstorage.h"
#include "totool.h"
#include "tomain.h"
#include "tofilesize.h"
#include "tosql.h"
#include "tohelp.h"
#include "toconnection.h"

#include "tostorage.moc"
#include "tostoragetablespaceui.moc"
#include "tostoragedialogui.moc"
#include "tostoragedatafileui.moc"

#include "icons/refresh.xpm"
#include "icons/tostorage.xpm"
#include "icons/online.xpm"
#include "icons/offline.xpm"
#include "icons/logging.xpm"
#include "icons/eraselog.xpm"
#include "icons/movefile.xpm"
#include "icons/coalesce.xpm"
#include "icons/addfile.xpm"
#include "icons/addtablespace.xpm"
#include "icons/modtablespace.xpm"
#include "icons/modfile.xpm"
#include "icons/readtablespace.xpm"
#include "icons/writetablespace.xpm"

#define CONF_DISP_COALESCED "DispCoalesced"

class toStoragePrefs : public QGroupBox, public toSettingTab
{ 
  QCheckBox *DispCoalesced;
  toTool *Tool;

public:
  toStoragePrefs(toTool *tool,QWidget* parent = 0,const char* name = 0);
  virtual void saveSetting(void);
};

toStoragePrefs::toStoragePrefs(toTool *tool,QWidget* parent,const char* name)
  : QGroupBox(1,Horizontal,parent,name),toSettingTab("storage.html"),Tool(tool)
{
  setTitle(tr("Storage Manager" ));
  
  DispCoalesced = new QCheckBox(this,"DispCoalesced");
  DispCoalesced->setText(tr("&Display coalesced column"));
  QToolTip::add(DispCoalesced,tr("Can degrade performance noticably on large databases."));
  
  if (!tool->config(CONF_DISP_COALESCED,"Yes").isEmpty())
    DispCoalesced->setChecked(true);
}

void toStoragePrefs::saveSetting(void)
{
  Tool->setConfig(CONF_DISP_COALESCED,DispCoalesced->isChecked()?"Yes":"");
}

class toStorageTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return tostorage_xpm; }
public:
  toStorageTool()
    : toTool(20,"Storage Manager")
  { }
  virtual const char *menuItem()
  { return "Storage Manager"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toStorage(parent,connection);
  }
  virtual QWidget *configurationTab(QWidget *parent)
  {
    return new toStoragePrefs(this,parent);
  }
};

toStorageTablespace::toStorageTablespace(QWidget* parent,const char* name,WFlags fl)
  : toStorageTablespaceUI(parent,name,fl)
{
  Modify=false;
  if (!name)
    setName("toStorageTablespace");
  MinimumExtent->setTitle("&Minimum Extent");
  LocalUniform->setTitle("Extent &Size");
  LocalUniform->setValue(1024);
}

void toStorageTablespace::permanentToggle(bool val)
{
  if (!val)
    Dictionary->setChecked(true);
  LocalAuto->setEnabled(val);
  LocalSelect->setEnabled(val);
}

void toStorageTablespace::dictionaryToggle(bool val)
{
  Permanent->setEnabled(val);
  DefaultStorage->setEnabled(val);
  emit allowStorage(val);
}

void toStorageTablespace::allowDefault(bool val)
{
  LocalAuto->setEnabled(!val);
  LocalSelect->setEnabled(!val);
  emit allowStorage(val);
}

bool toStorageTablespace::allowStorage(void)
{
  return DefaultStorage->isChecked();
}

void toStorageTablespace::uniformToggle(bool val)
{
  LocalUniform->setEnabled(val);
}

std::list<QString> toStorageTablespace::sql()
{
  std::list<QString> ret;
  QString str("MINIMUM EXTENT ");
  str.append(MinimumExtent->sizeString());
  toPush(ret,str);
  if (Logging->isChecked()!=LoggingOrig||!Modify) {
    if (Logging->isChecked())
      str="LOGGING";
    else
      str="NOLOGGING";
    toPush(ret,str);
  }
  if (Online->isChecked()!=OnlineOrig||!Modify) {
    if (Online->isChecked())
      str="ONLINE";
    else
      str="OFFLINE";
    toPush(ret,str);
  }
  if (Permanent->isChecked()!=PermanentOrig||!Modify) {
    if (Permanent->isChecked())
      str="PERMANENT";
    else
      str="TEMPORARY";
    toPush(ret,str);
  }
  if (ExtentGroup->isEnabled()) {
    str="EXTENT MANAGEMENT ";
    if (Dictionary->isChecked())
      str.append("DICTIONARY");
    else {
      str.append("LOCAL ");
      if (LocalAuto->isChecked())
	str.append("AUTOALLOCATED");
      else {
	str.append("UNIFORM SIZE ");
	str.append(LocalUniform->sizeString());
      }
    }
    toPush(ret,str);
  }
  return ret;
}

toStorageDatafile::toStorageDatafile(bool dispName,QWidget* parent,const char* name,WFlags fl)
  : toStorageDatafileUI(parent,name,fl)
{
  Modify=false;
  InitialSizeOrig=NextSizeOrig=MaximumSizeOrig=0;

  if (!name)
    setName("DataFile");
  setCaption(tr("Create datafile"));

  if (!dispName) {
    Name->hide();
    NameLabel->hide();
    Name->setFocus();
  } else 
    Filename->setFocus();

  InitialSize->setTitle("&Size");
  NextSize->setTitle("&Next");
  MaximumSize->setTitle("&Maximum size");
}

std::list<QString> toStorageDatafile::sql(void)
{
  std::list<QString> ret;
  if (!Modify) {
    QString str("'");
    QString filename(Filename->text());
    filename.replace(QRegExp("'"),"''");
    str.append(filename);
    str.append("' SIZE ");
    str.append(InitialSize->sizeString());
    str.append(" REUSE AUTOEXTEND ");
    if (AutoExtend->isChecked()) {
      str.append("ON NEXT ");
      str.append(NextSize->sizeString());
      str.append(" MAXSIZE ");
      if (UnlimitedMax->isChecked())
	str.append("UNLIMITED");
      else {
	str.append(MaximumSize->sizeString());
      }
    } else
      str.append("OFF");
    toPush(ret,str);
  } else {
    QString str;
    if (Filename->text()!=FilenameOrig) {
      str="RENAME FILE '";
      str+=FilenameOrig;
      str+="' TO '";
      str+=Filename->text();
      toPush(ret,str);
    }
    if (InitialSize->value()!=InitialSizeOrig) {
      str="DATAFILE '";
      str+=Filename->text();
      str+="' RESIZE ";
      str+=InitialSize->sizeString();
      toPush(ret,str);
    }
    str="DATAFILE '";
    str+=Filename->text();
    str+="' AUTOEXTEND ";
    if (AutoExtend->isChecked()) {
      str.append("ON NEXT ");
      str.append(NextSize->sizeString());
      str.append(" MAXSIZE ");
      if (UnlimitedMax->isChecked())
	str.append("UNLIMITED");
      else {
	str.append(MaximumSize->sizeString());
      }
    } else
      str.append("OFF");
    toPush(ret,str);
  }
  return ret;
}

void toStorageDatafile::browseFile(void)
{
  QString str=TOFileDialog::getSaveFileName(Filename->text(),"*.dbf",this);
  if (!str.isEmpty())
    Filename->setText(str);
}

void toStorageDatafile::autoExtend(bool val)
{
  MaximumSize->setEnabled(!UnlimitedMax->isChecked());
  UnlimitedMax->setEnabled(val);
  NextSize->setEnabled(val);
}

void toStorageDatafile::maximumSize(bool val)
{
  MaximumSize->setEnabled(!val);
}

QString toStorageDatafile::getName(void)
{
  if (!Name)
    throw QString("No name available");
  return Name->text();
}

void toStorageDatafile::valueChanged(const QString &str)
{
  if ((!Name||!Name->text().isEmpty())&&
      !Filename->text().isEmpty())
    emit validContent(true);
  else
    emit validContent(false);
}

void toStorageDialog::Setup(void)
{
  DialogTab->removePage(DefaultPage);
  toHelp::connectDialog(this);
}

toStorageDialog::toStorageDialog(const QString &tablespace,QWidget *parent)
  : toStorageDialogUI(parent,"Storage Dialog",true)
{
  Setup();
  OkButton->setEnabled(false);

  if (!tablespace.isNull()) {
    Mode=NewDatafile;
    TablespaceOrig=tablespace;
    setCaption("Add datafile");
    Tablespace=NULL;
    Default=NULL;
    Datafile=new toStorageDatafile(false,DialogTab);
    DialogTab->addTab(Datafile,"Datafile");
  } else {
    Mode=NewTablespace;
    setCaption("Add tablespace");
    Datafile=new toStorageDatafile(true,DialogTab);
    DialogTab->addTab(Datafile,"Datafile");
    Tablespace=new toStorageTablespace(DialogTab);
    DialogTab->addTab(Tablespace,"Tablespace");
    Default=new toStorageDefinition(DialogTab);
    DialogTab->addTab(Default,"Default Storage");
    Default->setEnabled(false);
    connect(Tablespace,SIGNAL(allowStorage(bool)),this,SLOT(allowStorage(bool)));
  }
  connect(Datafile,SIGNAL(validContent(bool)),this,SLOT(validContent(bool)));
}

static toSQL SQLTablespaceInfo("toStorage:TablespaceInfo",
			       "SELECT min_extlen/1024,\n"
			       "       extent_management,\n"
			       "       contents,\n"
			       "       logging,\n"
			       "       status,\n"
			       "       initial_extent/1024,\n"
			       "       next_extent/1024,\n"
			       "       min_extents,\n"
			       "       max_extents,\n"
			       "       pct_increase\n"
			       "  FROM dba_tablespaces\n"
			       " WHERE tablespace_name = :nam<char[70]>",
			       "Get information about a tablespace for the modify dialog, "
			       "must have same columns and bindings");

toStorageDialog::toStorageDialog(toConnection &conn,const QString &tablespace,QWidget *parent)
  : toStorageDialogUI(parent,"Storage Dialog",true)
{
  Setup();
  Mode=ModifyTablespace;
  Datafile=NULL;
  Tablespace=new toStorageTablespace(DialogTab);
  DialogTab->addTab(Tablespace,"Tablespace");
  Default=new toStorageDefinition(DialogTab);
  DialogTab->addTab(Default,"Default Storage");
  connect(Tablespace,SIGNAL(allowStorage(bool)),this,SLOT(allowStorage(bool)));

  toQList result=toQuery::readQuery(conn,SQLTablespaceInfo,tablespace);
  if (result.size()!=10)
    throw QString("Invalid response from query");
  Tablespace->MinimumExtent->setValue(toShift(result).toInt());

  TablespaceOrig=tablespace;
  Tablespace->Modify=true;
  Default->setEnabled(false);
  if (toShift(result)=="DICTIONARY")
    Tablespace->Dictionary->setChecked(true);
  else {
    Tablespace->DefaultStorage->setEnabled(false);
    Tablespace->LocalAuto->setChecked(true);
  }
  Tablespace->ExtentGroup->setEnabled(false);

  Tablespace->Permanent->setChecked(toShift(result)=="PERMANENT");
  Tablespace->Logging->setChecked(toShift(result)=="LOGGING");
  Tablespace->Online->setChecked(toShift(result)=="ONLINE");
  Tablespace->OnlineOrig=Tablespace->Online->isChecked();
  Tablespace->PermanentOrig=Tablespace->Permanent->isChecked();
  Tablespace->LoggingOrig=Tablespace->Logging->isChecked();

  Default->InitialSize->setValue(toShift(result).toInt());
  Default->NextSize->setValue(toShift(result).toInt());
  Default->InitialExtent->setValue(toShift(result).toInt());
  int num=toShift(result).toInt();
  if (num==0)
    Default->UnlimitedExtent->setChecked(true);
  else
    Default->MaximumExtent->setValue(num);
  Default->PCTIncrease->setValue(toShift(result).toInt());
}

static toSQL SQLDatafileInfo("toStorage:DatafileInfo",
			     "SELECT bytes/1024,\n"
			     "       autoextensible,\n"
			     "       bytes/blocks*increment_by/1024,\n"
			     "       maxbytes/1024\n"
			     "  FROM dba_data_files\n"
			     " WHERE tablespace_name = :nam<char[70]>"
			     "   AND file_name = :fil<char[1500]>",
			     "Get information about a datafile for the modify dialog, "
			     "must have same columns and bindings");

toStorageDialog::toStorageDialog(toConnection &conn,const QString &tablespace,
				 const QString &filename,QWidget *parent)
  : toStorageDialogUI(parent,"Storage Dialog",true)
{
  Setup();
  Mode=ModifyDatafile;
  Datafile=new toStorageDatafile(true,DialogTab);
  DialogTab->addTab(Datafile,"Datafile");
  setCaption("Modify datafile");
  Tablespace=NULL;
  Default=NULL;

  toQList result=toQuery::readQuery(conn,SQLDatafileInfo,tablespace,filename);

  if (result.size()!=4)
    throw QString("Invalid response from query (Wanted 4, got %1 entries").arg(result.size());
  Datafile->Name->setText(tablespace);
  Datafile->Name->setEnabled(false);
  Datafile->Modify=true;
  Datafile->FilenameOrig=filename;
  Datafile->Filename->setText(filename);
  Datafile->InitialSize->setValue(Datafile->InitialSizeOrig=toShift(result).toInt());
  if (toShift(result)!="NO") {
    Datafile->AutoExtend->setChecked(true);
    Datafile->NextSize->setValue(Datafile->NextSizeOrig=toShift(result).toInt());
  } else {
    Datafile->NextSizeOrig=0;
  }

  int num=toShift(result).toInt();
  if (num==0) {
    Datafile->UnlimitedMax->setChecked(true);
    Datafile->MaximumSize->setEnabled(false);
  } else {
    Datafile->UnlimitedMax->setChecked(false);
    Datafile->MaximumSize->setValue(num);
  }
  Datafile->MaximumSizeOrig=num;
}

void toStorageDialog::validContent(bool val)
{
  OkButton->setEnabled(val);
}

void toStorageDialog::allowStorage(bool val)
{
  Default->setEnabled(val);
}

void toStorageDialog::displaySQL(void)
{
  std::list<QString> lines=sql();
  QString res;
  for(std::list<QString>::iterator i=lines.begin();i!=lines.end();i++) {
    res+=*i;
    res+=";\n";
  }
  if (res.length()>0) {
    toMemoEditor memo(this,res,-1,-1,true,true);
    memo.exec();
  } else
    toStatusMessage("No changes made");
}

std::list<QString> toStorageDialog::sql(void)
{
  std::list<QString> ret;
  switch(Mode) {
  case ModifyDatafile:
    {
      QString start="ALTER DATABASE ";
      std::list<QString> lst=Datafile->sql();
      for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	QString t=start;
	t+=*i;
	toPush(ret,t);
      }
    }
    break;
  case ModifyTablespace:
    {
      QString start="ALTER TABLESPACE \"";
      start+=TablespaceOrig;
      start+="\" ";
      std::list<QString> lst=Tablespace->sql();
      for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	QString t=start;
	t+=*i;
	toPush(ret,t);
      }
      if (Tablespace->allowStorage()) {
	start+="DEFAULT ";
	std::list<QString> lst=Default->sql();
	for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	  QString t=start;
	  t+=*i;
	  toPush(ret,t);
	}
      }
    }
    break;
  case NewDatafile:
    {
      QString start="ALTER TABLESPACE \"";
      start+=TablespaceOrig;
      start+="\" ADD DATAFILE";
      std::list<QString> lst=Datafile->sql();
      for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	start+=" ";
	start+=*i;
      }
      toPush(ret,start);
    }
    break;
  case NewTablespace:
    {
      QString start="ALTER TABLESPACE \"";
      start+=Datafile->getName().upper();
      start+="\" DATAFILE ";
      std::list<QString> lst=Datafile->sql();
      {
	for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	  start+=" ";
	  start+=*i;
	}
      }
      lst=Tablespace->sql();
      for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	start+=" ";
	start+=*i;
      }
      if (Tablespace->allowStorage()) {
	start+=" DEFAULT";
	lst=Default->sql();
	for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	  start+=" ";
	  start+=*i;
	}
      }
      toPush(ret,start);
    }
  }
  return ret;
}

static toStorageTool StorageTool;

toStorage::toStorage(QWidget *main,toConnection &connection)
  : toToolWidget(StorageTool,"storage.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,"Storage manager",connection.description());

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Update",
		  "Update",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  OnlineButton=new QToolButton(QPixmap((const char **)online_xpm),
			       "Take tablespace online",
			       "Take tablespace online",
			       this,SLOT(online(void)),
			       toolbar);
  OfflineButton=new QToolButton(QPixmap((const char **)offline_xpm),
				"Take tablespace offline",
				"Take tablespace offline",
				this,SLOT(offline(void)),
				toolbar);
  toolbar->addSeparator(); 
  LoggingButton=new QToolButton(QPixmap((const char **)logging_xpm),
				"Set tablespace default to logging",
				"Set tablespace default to logging",
				this,SLOT(logging(void)),
				toolbar);
  EraseLogButton=new QToolButton(QPixmap((const char **)eraselog_xpm),
				 "Set tablespace default to no logging",
				 "Set tablespace default to no logging",
				 this,SLOT(noLogging(void)),
				 toolbar);
  toolbar->addSeparator(); 
  ReadWriteButton=new QToolButton(QPixmap((const char **)writetablespace_xpm),
				  "Allow read write access to tablespace",
				  "Allow read write access to tablespace",
				  this,SLOT(readWrite(void)),
				  toolbar);
  ReadOnlyButton=new QToolButton(QPixmap((const char **)readtablespace_xpm),
				 "Set tablespace to read only",
				 "Set tablespace to read only",
				 this,SLOT(readOnly(void)),
				 toolbar);
  toolbar->addSeparator(); 
  ModTablespaceButton=new QToolButton(QPixmap((const char **)modtablespace_xpm),
				      "Modify tablespace",
				      "Modify tablespace",
				      this,SLOT(modifyTablespace(void)),
				      toolbar);
  ModFileButton=new QToolButton(QPixmap((const char **)modfile_xpm),
				"Modify file",
				"Modify file",
				this,SLOT(modifyDatafile(void)),
				toolbar);
  toolbar->addSeparator(); 
  new QToolButton(QPixmap((const char **)addtablespace_xpm),
		  "Create new tablespace",
		  "Create new tablespace",
		  this,SLOT(newTablespace(void)),
		  toolbar);
  NewFileButton=new QToolButton(QPixmap((const char **)addfile_xpm),
				"Add datafile to tablespace",
				"Add datafile to tablespace",
				this,SLOT(newDatafile(void)),
				toolbar);
  toolbar->addSeparator(); 
  CoalesceButton=new QToolButton(QPixmap((const char **)coalesce_xpm),
				 "Coalesce tablespace",
				 "Coalesce tablespace",
				 this,SLOT(coalesce(void)),
				 toolbar);
  MoveFileButton=new QToolButton(QPixmap((const char **)movefile_xpm),
				 "Move datafile",
				 "Move datafile",
				 this,SLOT(moveFile(void)),
				 toolbar);
 
  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  Storage=new toResultStorage(this);

  connect(Storage,SIGNAL(selectionChanged(void)),this,SLOT(selectionChanged(void)));
  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  refresh();
  selectionChanged();
}

#define TO_ID_ONLINE		(toMain::TO_TOOL_MENU_ID+ 0)
#define TO_ID_OFFLINE		(toMain::TO_TOOL_MENU_ID+ 1)
#define TO_ID_LOGGING		(toMain::TO_TOOL_MENU_ID+ 2)
#define TO_ID_NOLOGGING		(toMain::TO_TOOL_MENU_ID+ 3)
#define TO_ID_READ_WRITE	(toMain::TO_TOOL_MENU_ID+ 4)
#define TO_ID_READ_ONLY		(toMain::TO_TOOL_MENU_ID+ 5)
#define TO_ID_MODIFY_TABLESPACE	(toMain::TO_TOOL_MENU_ID+ 6)
#define TO_ID_MODIFY_DATAFILE	(toMain::TO_TOOL_MENU_ID+ 7)
#define TO_ID_NEW_TABLESPACE	(toMain::TO_TOOL_MENU_ID+ 8)
#define TO_ID_ADD_DATAFILE	(toMain::TO_TOOL_MENU_ID+ 9)
#define TO_ID_COALESCE		(toMain::TO_TOOL_MENU_ID+ 10)
#define TO_ID_MOVE_FILE		(toMain::TO_TOOL_MENU_ID+ 11)

void toStorage::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),"&Refresh",
			   this,SLOT(refresh(void)),Key_F5);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)online_xpm),"Tablespace online",
			   this,SLOT(online()),0,TO_ID_ONLINE);
      ToolMenu->insertItem(QPixmap((const char **)offline_xpm),"Tablespace offline",
			   this,SLOT(offline()),0,TO_ID_OFFLINE);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)logging_xpm),"Default logging",
			   this,SLOT(logging()),0,TO_ID_LOGGING);
      ToolMenu->insertItem(QPixmap((const char **)eraselog_xpm),"Default not logging",
			   this,SLOT(noLogging()),0,TO_ID_NOLOGGING);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)writetablespace_xpm),"Read write access",
			   this,SLOT(readWrite()),0,TO_ID_READ_WRITE);
      ToolMenu->insertItem(QPixmap((const char **)readtablespace_xpm),"Read only access",
			   this,SLOT(readOnly()),0,TO_ID_READ_ONLY);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)modtablespace_xpm),"Modify tablespace",
			   this,SLOT(modifyTablespace()),0,TO_ID_MODIFY_TABLESPACE);
      ToolMenu->insertItem(QPixmap((const char **)modfile_xpm),"Modify datafile",
			   this,SLOT(modifyDatafile()),0,TO_ID_MODIFY_DATAFILE);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)addtablespace_xpm),"New tablespace",
			   this,SLOT(newTablespace()),0,TO_ID_NEW_TABLESPACE);
      ToolMenu->insertItem(QPixmap((const char **)addfile_xpm),"Add datafile",
			   this,SLOT(newDatafile()),0,TO_ID_ADD_DATAFILE);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)coalesce_xpm),"Coalesce tablespace",
			   this,SLOT(coalesce()),0,TO_ID_COALESCE);
      ToolMenu->insertItem(QPixmap((const char **)movefile_xpm),"Move datafile",
			   this,SLOT(moveFile()),0,TO_ID_MOVE_FILE);

      toMainWidget()->menuBar()->insertItem("&Storage",ToolMenu,-1,toToolMenuIndex());

      toMainWidget()->menuBar()->setItemEnabled(TO_ID_ONLINE,OnlineButton->isEnabled());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_OFFLINE,OfflineButton->isEnabled());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_LOGGING,LoggingButton->isEnabled());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_NOLOGGING,EraseLogButton->isEnabled());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_READ_WRITE,ReadWriteButton->isEnabled());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_READ_ONLY,ReadOnlyButton->isEnabled());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_MODIFY_TABLESPACE,
						ModTablespaceButton->isEnabled());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_MODIFY_DATAFILE,
						ModFileButton->isEnabled());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_ADD_DATAFILE,NewFileButton->isEnabled());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_COALESCE,CoalesceButton->isEnabled());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_MOVE_FILE,MoveFileButton->isEnabled());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

void toStorage::refresh(void)
{
  Storage->showCoalesced(!StorageTool.config(CONF_DISP_COALESCED,"Yes").isEmpty());
  Storage->query();
}

void toStorage::coalesce(void)
{
  try {
    QString str;
    str="ALTER TABLESPACE \"";
    str.append(Storage->currentTablespace());
    str.append("\" COALESCE");
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::online(void)
{
  try {
    QString str;
    str="ALTER TABLESPACE \"";
    str.append(Storage->currentTablespace());
    str.append("\" ONLINE");
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::logging(void)
{
  try {
    QString str;
    str="ALTER TABLESPACE \"";
    str.append(Storage->currentTablespace());
    str.append("\" LOGGING");
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::noLogging(void)
{
  try {
    QString str;
    str="ALTER TABLESPACE \"";
    str.append(Storage->currentTablespace());
    str.append("\" NOLOGGING");
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::readOnly(void)
{
  try {
    QString str;
    str="ALTER TABLESPACE \"";
    str.append(Storage->currentTablespace());
    str.append("\" READ ONLY");
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::readWrite(void)
{
  try {
    QString str;
    str="ALTER TABLESPACE \"";
    str.append(Storage->currentTablespace());
    str.append("\" READ WRITE");
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::offline(void)
{
  try {
    QString reason;
    QString dsc("Select mode to take tablespace ");
    dsc.append(Storage->currentTablespace());
    dsc.append(" offline for.");
    switch (TOMessageBox::information(this,"Tablespace offline",dsc,"Normal","Temporary","Cancel")) {
    case 0:
      reason="NORMAL";
      break;
    case 1:
      reason="TEMPORARY";
      break;
    default:
      return;
    }
    QString str;
    str="ALTER TABLESPACE \"";
    str.append(Storage->currentTablespace());
    str.append("\" OFFLINE ");
    str.append(reason);
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::selectionChanged(void)
{
  OfflineButton->setEnabled(false);
  OnlineButton->setEnabled(false);
  CoalesceButton->setEnabled(false);
  LoggingButton->setEnabled(false);
  EraseLogButton->setEnabled(false);
  ModTablespaceButton->setEnabled(false);
  NewFileButton->setEnabled(false);
  MoveFileButton->setEnabled(false);
  ModFileButton->setEnabled(false);
  ReadOnlyButton->setEnabled(false);
  ReadWriteButton->setEnabled(false);

  QListViewItem *item=Storage->selectedItem();
  if (item) {
    if (item->parent()) {
      item=item->parent();
      MoveFileButton->setEnabled(true);
      ModFileButton->setEnabled(true);
    }
    QListViewItem *child=item->firstChild();
    if (!child) {
      OnlineButton->setEnabled(true);
      OfflineButton->setEnabled(true);
      ReadWriteButton->setEnabled(true);
      ReadOnlyButton->setEnabled(true);
    } else {
      if (child->text(1)=="OFFLINE")
	OnlineButton->setEnabled(true);
      else if (child->text(1)=="ONLINE") {
	OfflineButton->setEnabled(true);
	if (child->text(2)=="READ ONLY")
	  ReadWriteButton->setEnabled(true);
	else
	  ReadOnlyButton->setEnabled(true);
      }
    }
    if (item->text(4)=="LOGGING")
      EraseLogButton->setEnabled(true);
    else
      LoggingButton->setEnabled(true);

    if (item->text(10)!="100%")
      CoalesceButton->setEnabled(true);
    NewFileButton->setEnabled(true);
    ModTablespaceButton->setEnabled(true);
  }
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_ONLINE,OnlineButton->isEnabled());
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_OFFLINE,OfflineButton->isEnabled());
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_LOGGING,LoggingButton->isEnabled());
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_NOLOGGING,EraseLogButton->isEnabled());
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_READ_WRITE,ReadWriteButton->isEnabled());
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_READ_ONLY,ReadOnlyButton->isEnabled());
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_MODIFY_TABLESPACE,
					    ModTablespaceButton->isEnabled());
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_MODIFY_DATAFILE,
					    ModFileButton->isEnabled());
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_ADD_DATAFILE,NewFileButton->isEnabled());
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_COALESCE,CoalesceButton->isEnabled());
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_MOVE_FILE,MoveFileButton->isEnabled());
}

void toStorage::newDatafile(void)
{
  try {
    toStorageDialog newFile(Storage->currentTablespace(),this);

    if (newFile.exec()) {
      std::list<QString> lst=newFile.sql();
      for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++)
	connection().execute(*i);

      refresh();
    }
  } TOCATCH
}

void toStorage::newTablespace(void)
{
  try {
    toStorageDialog newSpace(QString::null,this);

    if (newSpace.exec()) {
      std::list<QString> lst=newSpace.sql();
      for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++)
	connection().execute(*i);

      refresh();
    }
  } TOCATCH
}

void toStorage::modifyTablespace(void)
{
  try {
    toStorageDialog modifySpace(connection(),Storage->currentTablespace(),this);

    if (modifySpace.exec()) {
      std::list<QString> lst=modifySpace.sql();
      for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++)
	connection().execute(*i);

      refresh();
    }
  } TOCATCH
}

void toStorage::modifyDatafile(void)
{
  try {
    toStorageDialog modifySpace(connection(),Storage->currentTablespace(),
				Storage->currentFilename(),this);

    if (modifySpace.exec()) {
      std::list<QString> lst=modifySpace.sql();
      for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++)
	connection().execute(*i);

      refresh();
    }
  } TOCATCH
}

void toStorage::moveFile(void)
{
  try {
    QString orig=Storage->currentFilename();
    QString file=TOFileDialog::getSaveFileName(orig,"*.dbf",this);
    if (!file.isEmpty()&&file!=orig) {
      QString str;
      str="ALTER TABLESPACE \"";
      str.append(Storage->currentTablespace());
      str.append("\" RENAME DATAFILE '");
      orig.replace(QRegExp("'"),"''");
      file.replace(QRegExp("'"),"''");
      str.append(orig);
      str.append("' TO '");
      str.append(file);
      str.append("'");
      connection().execute(str);
      refresh();
    }
  } TOCATCH
}
