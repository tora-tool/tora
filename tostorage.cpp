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

#ifdef TO_KDE
#include <kfiledialog.h>
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

class toStoragePrefs : public QFrame, public toSettingTab
{ 
  QGroupBox *GroupBox1;
  QCheckBox *DispCoalesced;
  toTool *Tool;

public:
  toStoragePrefs(toTool *tool,QWidget* parent = 0,const char* name = 0);
  virtual void saveSetting(void);
};

toStoragePrefs::toStoragePrefs(toTool *tool,QWidget* parent,const char* name)
  : QFrame(parent,name),toSettingTab("storage.html"),Tool(tool)
{
  GroupBox1 = new QGroupBox( this, "GroupBox1" );
  GroupBox1->setGeometry( QRect( 10, 10, 380, 380 ) ); 
  GroupBox1->setTitle( tr( "Storage Tool"  ) );
  
  DispCoalesced = new QCheckBox( GroupBox1, "DispCoalesced" );
  DispCoalesced->setGeometry( QRect( 20, 30, 340, 20 ) ); 
  DispCoalesced->setText( tr( "&Display coalesced column"  ) );
  QToolTip::add( DispCoalesced, tr( "Can degrade performance noticably on large databases." ) );
  
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
    QWidget *window=new toStorage(parent,connection);
    window->setIcon(*toolbarImage());
    return window;
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
      start+=Datafile->getName();
      start+="\" DATAFILE ";
      std::list<QString> lst=Datafile->sql();
      for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
	start+=" ";
	start+=*i;
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

static QPixmap *toRefreshPixmap;
static QPixmap *toOnlinePixmap;
static QPixmap *toOfflinePixmap;
static QPixmap *toEraseLogPixmap;
static QPixmap *toLoggingPixmap;
static QPixmap *toCoalescePixmap;
static QPixmap *toMovePixmap;
static QPixmap *toNewTablespacePixmap;
static QPixmap *toNewFilePixmap;
static QPixmap *toModTablespacePixmap;
static QPixmap *toReadTablespacePixmap;
static QPixmap *toWriteTablespacePixmap;
static QPixmap *toModFilePixmap;

toStorage::toStorage(QWidget *main,toConnection &connection)
  : toToolWidget(StorageTool,"storage.html",main,connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);
  if (!toOnlinePixmap)
    toOnlinePixmap=new QPixmap((const char **)online_xpm);
  if (!toOfflinePixmap)
    toOfflinePixmap=new QPixmap((const char **)offline_xpm);
  if (!toLoggingPixmap)
    toLoggingPixmap=new QPixmap((const char **)logging_xpm);
  if (!toEraseLogPixmap)
    toEraseLogPixmap=new QPixmap((const char **)eraselog_xpm);
  if (!toMovePixmap)
    toMovePixmap=new QPixmap((const char **)movefile_xpm);
  if (!toCoalescePixmap)
    toCoalescePixmap=new QPixmap((const char **)coalesce_xpm);
  if (!toNewFilePixmap)
    toNewFilePixmap=new QPixmap((const char **)addfile_xpm);
  if (!toNewTablespacePixmap)
    toNewTablespacePixmap=new QPixmap((const char **)addtablespace_xpm);
  if (!toModTablespacePixmap)
    toModTablespacePixmap=new QPixmap((const char **)modtablespace_xpm);
  if (!toReadTablespacePixmap)
    toReadTablespacePixmap=new QPixmap((const char **)readtablespace_xpm);
  if (!toWriteTablespacePixmap)
    toWriteTablespacePixmap=new QPixmap((const char **)writetablespace_xpm);
  if (!toModFilePixmap)
    toModFilePixmap=new QPixmap((const char **)modfile_xpm);

  QToolBar *toolbar=toAllocBar(this,"Storage manager",connection.description());

  new QToolButton(*toRefreshPixmap,
		  "Update",
		  "Update",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  OnlineButton=new QToolButton(*toOnlinePixmap,
			       "Take tablespace online",
			       "Take tablespace online",
			       this,SLOT(online(void)),
			       toolbar);
  OfflineButton=new QToolButton(*toOfflinePixmap,
				"Take tablespace offline",
				"Take tablespace offline",
				this,SLOT(offline(void)),
				toolbar);
  toolbar->addSeparator(); 
  LoggingButton=new QToolButton(*toLoggingPixmap,
				"Set tablespace default to logging",
				"Set tablespace default to logging",
				this,SLOT(logging(void)),
				toolbar);
  EraseLogButton=new QToolButton(*toEraseLogPixmap,
				 "Set tablespace default to no logging",
				 "Set tablespace default to no logging",
				 this,SLOT(noLogging(void)),
				 toolbar);
  toolbar->addSeparator(); 
  ReadWriteButton=new QToolButton(*toWriteTablespacePixmap,
				  "Allow read write access to tablespace",
				  "Allow read write access to tablespace",
				  this,SLOT(readWrite(void)),
				  toolbar);
  ReadOnlyButton=new QToolButton(*toReadTablespacePixmap,
				 "Set tablespace to read only",
				 "Set tablespace to read only",
				 this,SLOT(readOnly(void)),
				 toolbar);
  toolbar->addSeparator(); 
  ModTablespaceButton=new QToolButton(*toModTablespacePixmap,
				      "Modify tablespace",
				      "Modify tablespace",
				      this,SLOT(modifyTablespace(void)),
				      toolbar);
  ModFileButton=new QToolButton(*toModFilePixmap,
				"Modify file",
				"Modify file",
				this,SLOT(modifyDatafile(void)),
				toolbar);
  toolbar->addSeparator(); 
  new QToolButton(*toNewTablespacePixmap,
		  "Create new tablespace",
		  "Create new tablespace",
		  this,SLOT(newTablespace(void)),
		  toolbar);
  NewFileButton=new QToolButton(*toNewFilePixmap,
				"Add datafile to tablespace",
				"Add datafile to tablespace",
				this,SLOT(newDatafile(void)),
				toolbar);
  toolbar->addSeparator(); 
  CoalesceButton=new QToolButton(*toCoalescePixmap,
				 "Coalesce tablespace",
				 "Coalesce tablespace",
				 this,SLOT(coalesce(void)),
				 toolbar);
  MoveFileButton=new QToolButton(*toMovePixmap,
				 "Move datafile",
				 "Move datafile",
				 this,SLOT(moveFile(void)),
				 toolbar);
 
  toolbar->setStretchableWidget(new QLabel("",toolbar));
  new toChangeConnection(toolbar);

  Storage=new toResultStorage(this);

  connect(Storage,SIGNAL(selectionChanged(void)),this,SLOT(selectionChanged(void)));

  refresh();
  selectionChanged();
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
