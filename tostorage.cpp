
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

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "tofilesize.h"
#include "tohelp.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "toresultstorage.h"
#include "tosql.h"
#include "tostorage.h"
#include "tostoragedefinition.h"
#include "tostorageprefsui.h"
#include "totool.h"

#ifdef TO_KDE
#include <kfiledialog.h>
#include <kmenubar.h>
#endif

#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qworkspace.h>

#include "tostorage.moc"
#include "tostoragedatafileui.moc"
#include "tostoragedialogui.moc"
#include "tostorageprefsui.moc"
#include "tostoragetablespaceui.moc"

#include "icons/addfile.xpm"
#include "icons/addtablespace.xpm"
#include "icons/coalesce.xpm"
#include "icons/eraselog.xpm"
#include "icons/logging.xpm"
#include "icons/modfile.xpm"
#include "icons/modtablespace.xpm"
#include "icons/movefile.xpm"
#include "icons/offline.xpm"
#include "icons/online.xpm"
#include "icons/readtablespace.xpm"
#include "icons/refresh.xpm"
#include "icons/storageextents.xpm"
#include "icons/tostorage.xpm"
#include "icons/writetablespace.xpm"

#define CONF_DISP_TABLESPACES "DispTablespaces"
#define CONF_DISP_COALESCED "DispCoalesced"
#define CONF_DISP_EXTENTS "DispExtents"
#define CONF_DISP_AVAILABLEGRAPH "AvailableGraph"

class toStoragePrefs : public toStoragePrefsUI, public toSettingTab
{
  toTool *Tool;

public:
  toStoragePrefs(toTool *tool,QWidget* parent = 0,const char* name = 0);
  virtual void saveSetting(void);
};

toStoragePrefs::toStoragePrefs(toTool *tool,QWidget* parent,const char* name)
  : toStoragePrefsUI(parent,name),toSettingTab("storage.html"),Tool(tool)
{
  DispCoalesced->setChecked(!tool->config(CONF_DISP_COALESCED,"").isEmpty());
  DispExtents->setChecked(!tool->config(CONF_DISP_EXTENTS,"").isEmpty());
  DispTablespaces->setChecked(!tool->config(CONF_DISP_TABLESPACES,"Yes").isEmpty());
  DispAvailableGraph->setChecked(!tool->config(CONF_DISP_AVAILABLEGRAPH,"Yes").isEmpty());
}

void toStoragePrefs::saveSetting(void)
{
  Tool->setConfig(CONF_DISP_COALESCED,DispCoalesced->isChecked()?"Yes":"");
  Tool->setConfig(CONF_DISP_EXTENTS,DispExtents->isChecked()?"Yes":"");
  Tool->setConfig(CONF_DISP_TABLESPACES,DispTablespaces->isChecked()?"Yes":"");
  Tool->setConfig(CONF_DISP_AVAILABLEGRAPH,DispAvailableGraph->isChecked()?"Yes":"");
}

class toStorageTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return tostorage_xpm; }
public:
  toStorageTool()
    : toTool(50,"Storage Manager")
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
  MinimumExtent->setTitle(tr("&Minimum Extent"));
  LocalUniform->setTitle(tr("Extent &Size"));
  LocalUniform->setValue(1024);
}

void toStorageTablespace::permanentToggle(bool val)
{
  try {
    if (toCurrentConnection(this).version()>="8") {
      emit tempFile(!val&&!Dictionary->isChecked());
      return;
    }
    if (!val)
      Dictionary->setChecked(true);
    LocalAuto->setEnabled(val);
    LocalSelect->setEnabled(val);
  } TOCATCH
}

void toStorageTablespace::dictionaryToggle(bool val)
{
  try {
    if (toCurrentConnection(this).version()<"8")
      Permanent->setEnabled(val);
    else
      emit tempFile(!Permanent->isChecked()&&!val);
    DefaultStorage->setEnabled(val);
    emit allowStorage(val);
  } TOCATCH
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
  QString str;
  if (ExtentGroup->isEnabled()&&Dictionary->isChecked())
    toPush(ret,QString::fromLatin1("MINIMUM EXTENT ")+MinimumExtent->sizeString());

  if (Logging->isChecked()!=LoggingOrig||!Modify) {
    if (Logging->isChecked())
      str=QString::fromLatin1("LOGGING");
    else
      str=QString::fromLatin1("NOLOGGING");
    toPush(ret,str);
  }
  if (Online->isChecked()!=OnlineOrig||!Modify) {
    if (Online->isChecked())
      str=QString::fromLatin1("ONLINE");
    else
      str=QString::fromLatin1("OFFLINE");
    toPush(ret,str);
  }
  if (Permanent->isChecked()!=PermanentOrig||!Modify) {
    if (Permanent->isChecked())
      str=QString::fromLatin1("PERMANENT");
    else
      str=QString::fromLatin1("TEMPORARY");
    toPush(ret,str);
  }
  if (ExtentGroup->isEnabled()) {
    str=QString::fromLatin1("EXTENT MANAGEMENT ");
    if (Dictionary->isChecked())
      str.append(QString::fromLatin1("DICTIONARY"));
    else {
      str.append(QString::fromLatin1("LOCAL "));
      if (LocalAuto->isChecked())
	str.append(QString::fromLatin1("AUTOALLOCATE"));
      else {
	str.append(QString::fromLatin1("UNIFORM SIZE "));
	str.append(LocalUniform->sizeString());
      }
    }
    toPush(ret,str);
  }
  return ret;
}

toStorageDatafile::toStorageDatafile(bool temp,bool dispName,QWidget* parent,const char* name,WFlags fl)
  : toStorageDatafileUI(parent,name,fl),Tempfile(temp)
{
  Modify=false;
  InitialSizeOrig=NextSizeOrig=MaximumSizeOrig=0;

  if (!name)
    setName("DataFile");
  setCaption(tr("Create datafile"));

  if (!dispName) {
    Name->hide();
    NameLabel->hide();
    Filename->setFocus();
  } else 
    Name->setFocus();

  InitialSize->setTitle(tr("&Size"));
  NextSize->setTitle(tr("&Next"));
  MaximumSize->setTitle(tr("&Maximum size"));
}

std::list<QString> toStorageDatafile::sql(void)
{
  std::list<QString> ret;
  if (!Modify) {
    QString str;
    if (Tempfile)
      str=QString::fromLatin1("TEMPFILE '");
    else
      str=QString::fromLatin1("DATAFILE '");
    QString filename(Filename->text());
    filename.replace(QRegExp(QString::fromLatin1("'")),QString::fromLatin1("''"));
    str.append(filename);
    str.append(QString::fromLatin1("' SIZE "));
    str.append(InitialSize->sizeString());
    str.append(QString::fromLatin1(" REUSE AUTOEXTEND "));
    if (AutoExtend->isChecked()) {
      str.append(QString::fromLatin1("ON NEXT "));
      str.append(NextSize->sizeString());
      str.append(QString::fromLatin1(" MAXSIZE "));
      if (UnlimitedMax->isChecked())
	str.append(QString::fromLatin1("UNLIMITED"));
      else {
	str.append(MaximumSize->sizeString());
      }
    } else
      str.append(QString::fromLatin1("OFF"));
    toPush(ret,str);
  } else {
    QString str;
    if (Filename->text()!=FilenameOrig) {
      str=QString::fromLatin1("RENAME FILE '");
      str+=FilenameOrig;
      str+=QString::fromLatin1("' TO '");
      str+=Filename->text();
      toPush(ret,str);
    }
    if (InitialSize->value()!=InitialSizeOrig) {
      if (Tempfile)
	str=QString::fromLatin1("TEMPFILE '");
      else
	str=QString::fromLatin1("DATAFILE '");
      str+=Filename->text();
      str+=QString::fromLatin1("' RESIZE ");
      str+=InitialSize->sizeString();
      toPush(ret,str);
    }
    if (Tempfile)
      str=QString::fromLatin1("TEMPFILE '");
    else
      str=QString::fromLatin1("DATAFILE '");
    str+=Filename->text();
    str+=QString::fromLatin1("' AUTOEXTEND ");
    if (AutoExtend->isChecked()) {
      str.append(QString::fromLatin1("ON NEXT "));
      str.append(NextSize->sizeString());
      str.append(QString::fromLatin1(" MAXSIZE "));
      if (UnlimitedMax->isChecked())
	str.append(QString::fromLatin1("UNLIMITED"));
      else {
	str.append(MaximumSize->sizeString());
      }
    } else
      str.append(QString::fromLatin1("OFF"));
    toPush(ret,str);
  }
  return ret;
}

void toStorageDatafile::browseFile(void)
{
  QString str=TOFileDialog::getSaveFileName(Filename->text(),QString::fromLatin1("*.dbf"),this);
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
    throw tr("No name available");
  return Name->text();
}

void toStorageDatafile::valueChanged(const QString &str)
{
  if ((Name->isHidden()||!Name->text().isEmpty())&&
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
			       "  FROM sys.dba_tablespaces\n"
			       " WHERE tablespace_name = :nam<char[70]>",
			       "Get information about a tablespace for the modify dialog, "
			       "must have same columns and bindings");

toStorageDialog::toStorageDialog(const QString &tablespace,QWidget *parent)
  : toStorageDialogUI(parent,"Storage Dialog",true)
{
  Setup();
  OkButton->setEnabled(false);

  if (!tablespace.isNull()) {
    try {
      toQList result;
      result=toQuery::readQuery(toCurrentConnection(this),
				SQLTablespaceInfo,tablespace);
      if (result.size()!=10)
	throw tr("Invalid response from query");

      toShift(result);
      QString dict=toShift(result);
      QString temp=toShift(result);
      
      Mode=NewDatafile;
      TablespaceOrig=tablespace;
      setCaption(tr("Add datafile"));
      Tablespace=NULL;
      Default=NULL;
      Datafile=new toStorageDatafile(dict!=QString::fromLatin1("DICTIONARY")&&
				     temp!=QString::fromLatin1("PERMANENT"),false,DialogTab);
      DialogTab->addTab(Datafile,tr("Datafile"));
    } TOCATCH
  } else {
    Mode=NewTablespace;
    setCaption(tr("Add tablespace"));
    Datafile=new toStorageDatafile(false,true,DialogTab);
    DialogTab->addTab(Datafile,tr("Datafile"));
    Tablespace=new toStorageTablespace(DialogTab);
    DialogTab->addTab(Tablespace,tr("Tablespace"));
    Default=new toStorageDefinition(DialogTab);
    DialogTab->addTab(Default,tr("Default Storage"));
    Default->setEnabled(false);
    connect(Tablespace,SIGNAL(allowStorage(bool)),this,SLOT(allowStorage(bool)));
    connect(Tablespace,SIGNAL(tempFile(bool)),Datafile,SLOT(setTempFile(bool)));
  }
  connect(Datafile,SIGNAL(validContent(bool)),this,SLOT(validContent(bool)));
}

toStorageDialog::toStorageDialog(toConnection &conn,const QString &tablespace,QWidget *parent)
  : toStorageDialogUI(parent,"Storage Dialog",true)
{
  Setup();
  try {
    Mode=ModifyTablespace;
    Datafile=NULL;
    Tablespace=new toStorageTablespace(DialogTab);
    DialogTab->addTab(Tablespace,tr("Tablespace"));
    Default=new toStorageDefinition(DialogTab);
    DialogTab->addTab(Default,tr("Default Storage"));
    connect(Tablespace,SIGNAL(allowStorage(bool)),this,SLOT(allowStorage(bool)));

    toQList result=toQuery::readQuery(conn,SQLTablespaceInfo,tablespace);
    if (result.size()!=10)
      throw tr("Invalid response from query");
    Tablespace->MinimumExtent->setValue(toShift(result).toInt());

    TablespaceOrig=tablespace;
    Tablespace->Modify=true;
    Default->setEnabled(false);
    if (toShift(result)==QString::fromLatin1("DICTIONARY"))
      Tablespace->Dictionary->setChecked(true);
    else {
      Tablespace->DefaultStorage->setEnabled(false);
      Tablespace->LocalAuto->setChecked(true);
    }
    Tablespace->ExtentGroup->setEnabled(false);

    Tablespace->Permanent->setChecked(toShift(result)==QString::fromLatin1("PERMANENT"));
    Tablespace->Logging->setChecked(toShift(result)==QString::fromLatin1("LOGGING"));
    Tablespace->Online->setChecked(toShift(result)==QString::fromLatin1("ONLINE"));
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
  } catch(const QString &str) {
    toStatusMessage(str);
    reject();
  }
}

static toSQL SQLDatafileInfo("toStorage:DatafileInfo",
			     "SELECT bytes/1024,\n"
			     "       autoextensible,\n"
			     "       bytes/blocks*increment_by/1024,\n"
			     "       maxbytes/1024\n"
			     "  FROM (SELECT * FROM sys.dba_data_files UNION SELECT * FROM sys.dba_temp_files)\n"
			     " WHERE tablespace_name = :nam<char[70]>"
			     "   AND file_name = :fil<char[1500]>",
			     "Get information about a datafile for the modify dialog, "
			     "must have same columns and bindings",
			     "8.1");

static toSQL SQLDatafileInfo8("toStorage:DatafileInfo",
			      "SELECT bytes/1024,\n"
			      "       autoextensible,\n"
			      "       bytes/blocks*increment_by/1024,\n"
			      "       maxbytes/1024\n"
			      "  FROM SELECT * sys.dba_data_files\n"
			      " WHERE tablespace_name = :nam<char[70]>"
			      "   AND file_name = :fil<char[1500]>",
			      "",
			      "8.0");

toStorageDialog::toStorageDialog(toConnection &conn,const QString &tablespace,
				 const QString &filename,QWidget *parent)
  : toStorageDialogUI(parent,"Storage Dialog",true)
{
  Setup();
  try {
    Mode=ModifyDatafile;

    toQList result;
    result=toQuery::readQuery(toCurrentConnection(this),
			      SQLTablespaceInfo,tablespace);
    if (result.size()!=10)
      throw tr("Invalid response from query");

    toShift(result);
    QString dict=toShift(result);
    QString temp=toShift(result);

    Datafile=new toStorageDatafile(dict!=QString::fromLatin1("DICTIONARY")&&
				   temp!=QString::fromLatin1("PERMANENT"),true,DialogTab);
    DialogTab->addTab(Datafile,tr("Datafile"));
    setCaption(tr("Modify datafile"));
    Tablespace=NULL;
    Default=NULL;

    result=toQuery::readQuery(conn,SQLDatafileInfo,tablespace,filename);

    if (result.size()!=4)
      throw tr("Invalid response from query (Wanted 4, got %1 entries) for %2.%3").
	arg(result.size()).arg(tablespace).arg(filename);
    Datafile->Name->setText(tablespace);
    Datafile->Name->setEnabled(false);
    Datafile->Modify=true;
    Datafile->FilenameOrig=filename;
    Datafile->Filename->setText(filename);
    Datafile->InitialSize->setValue(Datafile->InitialSizeOrig=toShift(result).toInt());
    if (toShift(result)!=QString::fromLatin1("NO")) {
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
  } catch(const QString &str) {
    toStatusMessage(str);
    reject();
  }
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
    res+=QString::fromLatin1(";\n");
  }
  if (res.length()>0) {
    toMemoEditor memo(this,res,-1,-1,true,true);
    memo.exec();
  } else
    toStatusMessage(tr("No changes made"));
}

std::list<QString> toStorageDialog::sql(void)
{
  try {
    std::list<QString> ret;
    switch(Mode) {
    case ModifyDatafile:
      {
	QString start=QString::fromLatin1("ALTER DATABASE ");
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
	QString start=QString::fromLatin1("ALTER TABLESPACE \"");
	start+=TablespaceOrig;
	start+=QString::fromLatin1("\" ");
	std::list<QString> lst=Tablespace->sql();
	for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	  QString t=start;
	  t+=*i;
	  toPush(ret,t);
	}
	if (Tablespace->allowStorage()) {
	  start+=QString::fromLatin1("DEFAULT ");
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
	QString start=QString::fromLatin1("ALTER TABLESPACE \"");
	start+=TablespaceOrig;
	start+=QString::fromLatin1("\" ADD ");
	std::list<QString> lst=Datafile->sql();
	for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	  start+=QString::fromLatin1(" ");
	  start+=*i;
	}
	toPush(ret,start);
      }
      break;
    case NewTablespace:
      {
	QString start=QString::fromLatin1("CREATE TABLESPACE \"");
	start+=Datafile->getName().upper();
	start+=QString::fromLatin1("\" ");
	std::list<QString> lst=Datafile->sql();
	{
	  for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	    start+=QString::fromLatin1(" ");
	    start+=*i;
	  }
	}
	lst=Tablespace->sql();
	for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	  start+=QString::fromLatin1(" ");
	  start+=*i;
	}
	if (Tablespace->allowStorage()) {
	  start+=QString::fromLatin1(" DEFAULT");
	  lst=Default->sql();
	  for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	    start+=QString::fromLatin1(" ");
	    start+=*i;
	  }
	}
	toPush(ret,start);
      }
    }
    return ret;
  } catch(const QString &str) {
    toStatusMessage(str);
    std::list<QString> ret;
    return ret;
  }
}

static toStorageTool StorageTool;

toStorage::toStorage(QWidget *main,toConnection &connection)
  : toToolWidget(StorageTool,"storage.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,tr("Storage manager"));

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  tr("Update"),
		  tr("Update"),
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  ExtentButton=new QToolButton(toolbar);
  ExtentButton->setToggleButton(true);
  ExtentButton->setIconSet(QIconSet(QPixmap((const char **)storageextents_xpm)));
  bool extents=!StorageTool.config(CONF_DISP_EXTENTS,"").isEmpty();
  if (extents)
    ExtentButton->setOn(true);
  connect(ExtentButton,SIGNAL(toggled(bool)),this,SLOT(showExtent(bool)));
  QToolTip::add(ExtentButton,tr("Show extent view."));

  TablespaceButton=new QToolButton(toolbar);
  TablespaceButton->setToggleButton(true);
  TablespaceButton->setIconSet(QIconSet(QPixmap((const char **)tostorage_xpm)));
  bool tablespaces=!StorageTool.config(CONF_DISP_TABLESPACES,"Yes").isEmpty();
  if (tablespaces)
    TablespaceButton->setOn(true);
  connect(TablespaceButton,SIGNAL(toggled(bool)),this,SLOT(showTablespaces(bool)));
  QToolTip::add(TablespaceButton,tr("Show tablespaces or just datafiles."));

  toolbar->addSeparator();

  OnlineButton=new QToolButton(QPixmap((const char **)online_xpm),
			       tr("Take tablespace online"),
			       tr("Take tablespace online"),
			       this,SLOT(online(void)),
			       toolbar);
  OfflineButton=new QToolButton(QPixmap((const char **)offline_xpm),
				tr("Take tablespace offline"),
				tr("Take tablespace offline"),
				this,SLOT(offline(void)),
				toolbar);
  toolbar->addSeparator(); 
  LoggingButton=new QToolButton(QPixmap((const char **)logging_xpm),
				tr("Set tablespace default to logging"),
				tr("Set tablespace default to logging"),
				this,SLOT(logging(void)),
				toolbar);
  EraseLogButton=new QToolButton(QPixmap((const char **)eraselog_xpm),
				 tr("Set tablespace default to no logging"),
				 tr("Set tablespace default to no logging"),
				 this,SLOT(noLogging(void)),
				 toolbar);
  toolbar->addSeparator(); 
  ReadWriteButton=new QToolButton(QPixmap((const char **)writetablespace_xpm),
				  tr("Allow read write access to tablespace"),
				  tr("Allow read write access to tablespace"),
				  this,SLOT(readWrite(void)),
				  toolbar);
  ReadOnlyButton=new QToolButton(QPixmap((const char **)readtablespace_xpm),
				 tr("Set tablespace to read only"),
				 tr("Set tablespace to read only"),
				 this,SLOT(readOnly(void)),
				 toolbar);
  toolbar->addSeparator(); 
  ModTablespaceButton=new QToolButton(QPixmap((const char **)modtablespace_xpm),
				      tr("Modify tablespace"),
				      tr("Modify tablespace"),
				      this,SLOT(modifyTablespace(void)),
				      toolbar);
  ModFileButton=new QToolButton(QPixmap((const char **)modfile_xpm),
				tr("Modify file"),
				tr("Modify file"),
				this,SLOT(modifyDatafile(void)),
				toolbar);
  toolbar->addSeparator(); 
  new QToolButton(QPixmap((const char **)addtablespace_xpm),
		  tr("Create new tablespace"),
		  tr("Create new tablespace"),
		  this,SLOT(newTablespace(void)),
		  toolbar);
  NewFileButton=new QToolButton(QPixmap((const char **)addfile_xpm),
				tr("Add datafile to tablespace"),
				tr("Add datafile to tablespace"),
				this,SLOT(newDatafile(void)),
				toolbar);
  toolbar->addSeparator(); 
  CoalesceButton=new QToolButton(QPixmap((const char **)coalesce_xpm),
				 tr("Coalesce tablespace"),
				 tr("Coalesce tablespace"),
				 this,SLOT(coalesce(void)),
				 toolbar);
  MoveFileButton=new QToolButton(QPixmap((const char **)movefile_xpm),
				 tr("Move datafile"),
				 tr("Move datafile"),
				 this,SLOT(moveFile(void)),
				 toolbar);
 
  toolbar->setStretchableWidget(new QLabel(toolbar,TO_KDE_TOOLBAR_WIDGET));
  new toChangeConnection(toolbar,TO_KDE_TOOLBAR_WIDGET);

  QSplitter *splitter=new QSplitter(Vertical,this);
  Storage=new toResultStorage(!StorageTool.config(CONF_DISP_AVAILABLEGRAPH,"Yes").isEmpty(),
			      splitter);
  ExtentParent=new QSplitter(Horizontal,splitter);
  Objects=new toListView(ExtentParent);
  Objects->addColumn(tr("Owner"));
  Objects->addColumn(tr("Object"));
  Objects->addColumn(tr("Partition"));
  Objects->addColumn(tr("Extents"));
  Objects->setColumnAlignment(3,AlignRight);

  Extents=new toStorageExtent(ExtentParent);
  Objects->setSelectionMode(QListView::Single);
  Storage->setSelectionMode(QListView::Single);
  connect(Objects,SIGNAL(selectionChanged(void)),this,SLOT(selectObject(void)));

  if (!extents)
    ExtentParent->hide();

  if (!tablespaces)
    Storage->setOnlyFiles(true);

  connect(Storage,SIGNAL(selectionChanged(void)),this,SLOT(selectionChanged(void)));

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  refresh();
  selectionChanged();
  setFocusProxy(Storage);
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
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),tr("&Refresh"),
			   this,SLOT(refresh(void)),Key_F5);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)online_xpm),tr("Tablespace online"),
			   this,SLOT(online()),0,TO_ID_ONLINE);
      ToolMenu->insertItem(QPixmap((const char **)offline_xpm),tr("Tablespace offline"),
			   this,SLOT(offline()),0,TO_ID_OFFLINE);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)logging_xpm),tr("Default logging"),
			   this,SLOT(logging()),0,TO_ID_LOGGING);
      ToolMenu->insertItem(QPixmap((const char **)eraselog_xpm),tr("Default not logging"),
			   this,SLOT(noLogging()),0,TO_ID_NOLOGGING);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)writetablespace_xpm),tr("Read write access"),
			   this,SLOT(readWrite()),0,TO_ID_READ_WRITE);
      ToolMenu->insertItem(QPixmap((const char **)readtablespace_xpm),tr("Read only access"),
			   this,SLOT(readOnly()),0,TO_ID_READ_ONLY);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)modtablespace_xpm),tr("Modify tablespace..."),
			   this,SLOT(modifyTablespace()),0,TO_ID_MODIFY_TABLESPACE);
      ToolMenu->insertItem(QPixmap((const char **)modfile_xpm),tr("Modify datafile..."),
			   this,SLOT(modifyDatafile()),0,TO_ID_MODIFY_DATAFILE);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)addtablespace_xpm),tr("New tablespace..."),
			   this,SLOT(newTablespace()),0,TO_ID_NEW_TABLESPACE);
      ToolMenu->insertItem(QPixmap((const char **)addfile_xpm),tr("Add datafile..."),
			   this,SLOT(newDatafile()),0,TO_ID_ADD_DATAFILE);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)coalesce_xpm),tr("Coalesce tablespace"),
			   this,SLOT(coalesce()),0,TO_ID_COALESCE);
      ToolMenu->insertItem(QPixmap((const char **)movefile_xpm),tr("Move datafile..."),
			   this,SLOT(moveFile()),0,TO_ID_MOVE_FILE);

      toMainWidget()->menuBar()->insertItem(tr("&Storage"),ToolMenu,-1,toToolMenuIndex());

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
  Storage->showCoalesced(!StorageTool.config(CONF_DISP_COALESCED,"").isEmpty());
  Storage->query();
}

void toStorage::coalesce(void)
{
  try {
    QString str;
    str=QString::fromLatin1("ALTER TABLESPACE \"");
    str.append(Storage->currentTablespace());
    str.append(QString::fromLatin1("\" COALESCE"));
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::online(void)
{
  try {
    QString str;
    str=QString::fromLatin1("ALTER TABLESPACE \"");
    str.append(Storage->currentTablespace());
    str.append(QString::fromLatin1("\" ONLINE"));
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::logging(void)
{
  try {
    QString str;
    str=QString::fromLatin1("ALTER TABLESPACE \"");
    str.append(Storage->currentTablespace());
    str.append(QString::fromLatin1("\" LOGGING"));
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::noLogging(void)
{
  try {
    QString str;
    str=QString::fromLatin1("ALTER TABLESPACE \"");
    str.append(Storage->currentTablespace());
    str.append(QString::fromLatin1("\" NOLOGGING"));
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::readOnly(void)
{
  try {
    QString str;
    str=QString::fromLatin1("ALTER TABLESPACE \"");;
    str.append(Storage->currentTablespace());
    str.append(QString::fromLatin1("\" READ ONLY"));
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::readWrite(void)
{
  try {
    QString str;
    str=QString::fromLatin1("ALTER TABLESPACE \"");
    str.append(Storage->currentTablespace());
    str.append(QString::fromLatin1("\" READ WRITE"));
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toStorage::offline(void)
{
  try {
    QString reason;
    QString dsc=tr("Select mode to take tablespace %1 offline.").arg(Storage->currentTablespace());
    switch (TOMessageBox::information(this,
				      tr("Tablespace offline"),
				      dsc,
				      tr("Normal"),tr("Temporary"),tr("Cancel"))) {
    case 0:
      reason=QString::fromLatin1("NORMAL");
      break;
    case 1:
      reason=QString::fromLatin1("TEMPORARY");
      break;
    default:
      return;
    }
    QString str;
    str=QString::fromLatin1("ALTER TABLESPACE \"");
    str.append(Storage->currentTablespace());
    str.append(QString::fromLatin1("\" OFFLINE "));
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
    if (item->parent()||Storage->onlyFiles()) {
      if (!ExtentParent->isHidden())
	Extents->setFile(item->text(12),item->text(13).toInt());
      item=item->parent();
      MoveFileButton->setEnabled(true);
      ModFileButton->setEnabled(true);
    } else if (!ExtentParent->isHidden())
      Extents->setTablespace(item->text(0));

    if (!ExtentParent->isHidden()) {
      std::list<toStorageExtent::extentName> obj=Extents->objects();
      QListViewItem *objItem=NULL;
      Objects->clear();
      for (std::list<toStorageExtent::extentName>::iterator i=obj.begin();i!=obj.end();i++) {
	objItem=new toResultViewItem(Objects,objItem,(*i).Owner);
	objItem->setText(1,(*i).Table);
	objItem->setText(2,(*i).Partition);
	objItem->setText(3,QString::number((*i).Size));
      }
    }

    if (item) {
      QListViewItem *child=item->firstChild();
      if (!child) {
	OnlineButton->setEnabled(true);
	OfflineButton->setEnabled(true);
	ReadWriteButton->setEnabled(true);
	ReadOnlyButton->setEnabled(true);
      } else {
	if (child->text(1)==QString::fromLatin1("OFFLINE"))
	  OnlineButton->setEnabled(true);
	else if (child->text(1)==QString::fromLatin1("ONLINE")) {
	  OfflineButton->setEnabled(true);
	  if (child->text(2)==QString::fromLatin1("READ ONLY"))
	    ReadWriteButton->setEnabled(true);
	  else
	    ReadOnlyButton->setEnabled(true);
	}
      }
      if (item->text(4)==QString::fromLatin1("LOGGING"))
	EraseLogButton->setEnabled(true);
      else
	LoggingButton->setEnabled(true);

      if (item->text(10)!=QString::fromLatin1("100%"))
	CoalesceButton->setEnabled(true);
    }
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
    QString file=TOFileDialog::getSaveFileName(orig,QString::fromLatin1("*.dbf"),this);
    if (!file.isEmpty()&&file!=orig) {
      QString str;
      str=QString::fromLatin1("ALTER TABLESPACE \"");
      str.append(Storage->currentTablespace());
      str.append(QString::fromLatin1("\" RENAME DATAFILE '"));
      orig.replace(QRegExp(QString::fromLatin1("'")),QString::fromLatin1("''"));
      file.replace(QRegExp(QString::fromLatin1("'")),QString::fromLatin1("''"));
      str.append(orig);
      str.append(QString::fromLatin1("' TO '"));
      str.append(file);
      str.append(QString::fromLatin1("'"));
      connection().execute(str);
      refresh();
    }
  } TOCATCH
}

void toStorage::selectObject(void)
{
  QListViewItem *item=Objects->selectedItem();
  if (item) {
    toResultViewItem *res=dynamic_cast<toResultViewItem *>(item);
    if (res)
      Extents->highlight(res->allText(0),res->allText(1),res->allText(2));
  }
}

toStorageExtent::extentName::extentName(const QString &owner,const QString &table,
					const QString &partition,int size)
  : Owner(owner),Table(table),Partition(partition)
{
  Size=size;
}

bool toStorageExtent::extentName::operator < (const toStorageExtent::extentName &ext) const
{
  if (Owner<ext.Owner)
    return true;
  if (Owner>ext.Owner)
    return false;
  if (Table<ext.Table)
    return true;
  if (Table>ext.Table)
    return false;
  if (Partition<ext.Partition)
    return true;
  return false;
}

bool toStorageExtent::extentName::operator == (const toStorageExtent::extentName &ext) const
{
  return Owner==ext.Owner&&Table==ext.Table&&Partition==ext.Partition;
}

toStorageExtent::extent::extent(const QString &owner,const QString &table,
				const QString &partition,
				int file,int block,int size)
  : extentName(owner,table,partition,size)
{
  File=file;
  Block=block;
}

bool toStorageExtent::extent::operator < (const toStorageExtent::extent &ext) const
{
  if (File<ext.File)
    return true;
  if (File>ext.File)
    return false;
  if (Block<ext.Block)
    return true;
  return false;
}

bool toStorageExtent::extent::operator == (const toStorageExtent::extent &ext) const
{
  return Owner==ext.Owner&&Table==ext.Table&&Partition==ext.Partition&&
    File==ext.File&&Block==ext.Block&&Size==ext.Size;
}

toStorageExtent::toStorageExtent(QWidget *parent)
  : QWidget(parent)
{
  setBackgroundColor(Qt::white);
}

void toStorageExtent::highlight(const QString &owner,const QString &table,
				const QString &partition)
{
  Highlight.Owner=owner;
  Highlight.Table=table;
  Highlight.Partition=partition;
  update();
}

static toSQL SQLObjectsFile("toStorage:ObjectsFile",
			    "SELECT owner,\n"
			    "       segment_name,\n"
			    "       partition_name,\n"
			    "       file_id,\n"
			    "       block_id,\n"
			    "       blocks\n"
			    "  FROM sys.dba_extents\n"
			    " WHERE tablespace_name = :tab<char[101]>\n"
			    "   AND file_id = :fil<int>",
			    "Get objects in a datafile, must have same columns and binds",
			    "8.0");

static toSQL SQLObjectsFile7("toStorage:ObjectsFile",
			     "SELECT owner,\n"
			     "       segment_name,\n"
			     "       NULL,\n"
			     "       file_id,\n"
			     "       block_id,\n"
			     "       blocks\n"
			     "  FROM sys.dba_extents\n"
			     " WHERE tablespace_name = :tab<char[101]>\n"
			     "   AND file_id = :fil<int>",
			     "",
			     "7.3");

static toSQL SQLObjectsTablespace("toStorage:ObjectsTablespace",
				  "SELECT owner,\n"
				  "       segment_name,\n"
				  "       partition_name,\n"
				  "       file_id,\n"
				  "       block_id,\n"
				  "       blocks\n"
				  "  FROM sys.dba_extents WHERE tablespace_name = :tab<char[101]>",
				  "Get objects in a tablespace, must have same columns and binds",
				  "8.0");

static toSQL SQLObjectsTablespace7("toStorage:ObjectsTablespace",
				   "SELECT owner,\n"
				   "       segment_name,\n"
				   "       NULL,\n"
				   "       file_id,\n"
				   "       block_id,\n"
				   "       blocks\n"
				   "  FROM sys.dba_extents WHERE tablespace_name = :tab<char[101]>",
				   "",
				   "7.3");

static toSQL SQLFileBlocks("toStorage:FileSize",
			   "SELECT file_id,blocks FROM sys.dba_data_files\n"
			   " WHERE tablespace_name = :tab<char[101]>\n"
			   "   AND file_id = :fil<int>\n"
			   " ORDER BY file_id",
			   "Get blocks for datafiles, must have same columns and binds");

static toSQL SQLTablespaceBlocks("toStorage:TablespaceSize",
				 "SELECT file_id,blocks FROM sys.dba_data_files\n"
				 " WHERE tablespace_name = :tab<char[101]>"
				 " ORDER BY file_id",
				 "Get blocks for tablespace datafiles, must have same columns and binds");

void toStorageExtent::setTablespace(const QString &tablespace)
{
  try {
    toBusy busy;
    Extents.clear();
    FileOffset.clear();
    toQuery query(toCurrentConnection(this),SQLObjectsTablespace,tablespace);
    extent cur;
    while(!query.eof()) {
      cur.Owner=query.readValueNull();
      cur.Table=query.readValueNull();
      cur.Partition=query.readValueNull();
      cur.File=query.readValueNull().toInt();
      cur.Block=query.readValueNull().toInt();
      cur.Size=query.readValueNull().toInt();
      toPush(Extents,cur);
    }
    toQuery blocks(toCurrentConnection(this),SQLTablespaceBlocks,tablespace);
    Total=0;
    while(!blocks.eof()) {
      int id=blocks.readValueNull().toInt();
      FileOffset[id]=Total;
      Total+=blocks.readValueNull().toInt();
    }
  } TOCATCH
  Extents.sort();
  update();
}

void toStorageExtent::setFile(const QString &tablespace,int file)
{
  try {
    toBusy busy;
    Extents.clear();
    FileOffset.clear();
    toQuery query(toCurrentConnection(this),SQLObjectsFile,tablespace,QString::number(file));
    extent cur;
    while(!query.eof()) {
      cur.Owner=query.readValueNull();
      cur.Table=query.readValueNull();
      cur.Partition=query.readValueNull();
      cur.File=query.readValueNull().toInt();
      cur.Block=query.readValueNull().toInt();
      cur.Size=query.readValueNull().toInt();
      toPush(Extents,cur);
    }
    toQuery blocks(toCurrentConnection(this),SQLFileBlocks,tablespace,QString::number(file));
    Total=0;
    while(!blocks.eof()) {
      int id=blocks.readValueNull().toInt();
      FileOffset[id]=Total;
      Total+=blocks.readValueNull().toInt();
    }
  } TOCATCH
  Extents.sort();
  update();
}

void toStorageExtent::paintEvent(QPaintEvent *e)
{
  QPainter paint(this);
  if (FileOffset.begin()==FileOffset.end())
    return;
  QFontMetrics fm=paint.fontMetrics();

  int offset=2*fm.lineSpacing();
  double lineblocks=Total/(height()-offset-FileOffset.size()+1);

  paint.fillRect(0,0,width(),offset,colorGroup().background());
  paint.drawText(0,0,width(),offset,AlignLeft|AlignTop,tr("Files: %1").arg(FileOffset.size()));
  paint.drawText(0,0,width(),offset,AlignRight|AlignTop,tr("Extents: %1").arg(Extents.size()));
  paint.drawText(0,0,width(),offset,AlignLeft|AlignBottom,tr("Blocks: %1").arg(Total));
  paint.drawText(0,0,width(),offset,AlignRight|AlignBottom,tr("Blocks/line: %1").arg(int(lineblocks)));

  for(std::list<extent>::iterator i=Extents.begin();i!=Extents.end();i++) {
    QColor col=Qt::green;
    if (extentName(*i)==Highlight)
      col=Qt::red;
    int fileo=0;
    for (std::map<int,int>::iterator j=FileOffset.begin();j!=FileOffset.end();j++,fileo++)
      if ((*j).first==(*i).File)
	break;
    int block=FileOffset[(*i).File]+(*i).Block;

    int y1=int(block/lineblocks);
    int x1=int((block/lineblocks-y1)*width());
    block+=(*i).Size;
    int y2=int(block/lineblocks);
    int x2=int((block/lineblocks-y2)*width());
    paint.setPen(col);
    if (y1!=y2) {
      paint.drawLine(x1,y1+offset+fileo,width()-1,y1+offset+fileo);
      paint.drawLine(0,y2+offset+fileo,x2-1,y2+offset+fileo);
      if (y1+1!=y2)
	paint.fillRect(0,y1+1+offset+fileo,width(),y2-y1-1,col);
    } else
      paint.drawLine(x1,y1+offset+fileo,x2,y2+offset+fileo);
  }
  std::map<int,int>::iterator j=FileOffset.begin();
  j++;
  paint.setPen(Qt::black);
  int fileo=offset;
  while(j!=FileOffset.end()) {
    int block=(*j).second;
    int y1=int(block/lineblocks);
    int x1=int((block/lineblocks-y1)*width());
    paint.drawLine(x1,y1+fileo,width()-1,y1+fileo);
    if (x1!=0)
      paint.drawLine(0,y1+1+fileo,x1-1,y1+fileo+1);
    j++;
    fileo++;
  }
}


std::list<toStorageExtent::extentName> toStorageExtent::objects(void)
{
  std::list<extentName> ret;

  for(std::list<extent>::iterator i=Extents.begin();i!=Extents.end();i++) {
    bool dup=false;
    for (std::list<extentName>::iterator j=ret.begin();j!=ret.end();j++) {
      if ((*j)==(*i)) {
	(*j).Size++;
	dup=true;
	break;
      }
    }
    if (!dup)
      toPush(ret,extentName((*i).Owner,(*i).Table,(*i).Partition,1));
  }

  ret.sort();

  return ret;
}

void toStorage::showExtent(bool ena)
{
  if (ena) {
    ExtentParent->show();
    selectionChanged();
  } else {
    ExtentParent->hide();
  }
}

void toStorage::showTablespaces(bool tab)
{
  Storage->setOnlyFiles(!tab);
}
