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

TO_NAMESPACE;

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

#include "tostorage.h"
#include "tostoragedefinition.h"
#include "toresultstorage.h"
#include "totool.h"
#include "tomain.h"
#include "tofilesize.h"
#include "tosql.h"
#include "tohelp.h"

#include "tostorage.moc"

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
  : QFrame(parent,name),Tool(tool)
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
  : QWidget(parent,name,fl)
{	
  Modify=false;
  if (!name)
    setName("toStorageTablespace");
  setCaption(tr("Form1"));
  QToolTip::add( this,tr("Specify a system manage tablespace. User can not specify extent allocation."));
  
  MinimumExtent=new toFilesize("&Minimum Extent",this,"MinimumExtent");
  MinimumExtent->setGeometry(QRect(10,10,310,60)); 
  MinimumExtent->setValue(50);
  
  GroupBox3=new QGroupBox(this,"GroupBox3");
  GroupBox3->setGeometry(QRect(10,80,310,145)); 
  GroupBox3->setTitle(tr("&Options"));
  
  Logging=new QCheckBox(GroupBox3,"Logging");
  Logging->setGeometry(QRect(10,25,290,19)); 
  Logging->setText(tr("&Logging"));
  Logging->setChecked(true);
  QToolTip::add( Logging,tr("Should objects created in the tablespace be logging by default."));
  
  Permanent=new QCheckBox(GroupBox3,"Permanent");
  Permanent->setGeometry(QRect(10,55,290,19)); 
  Permanent->setText(tr("&Permanent"));
  Permanent->setChecked(true);
  QToolTip::add( Permanent,tr("Should the contents of the tablespace be permanent after the end of a session."));
  connect(Permanent,SIGNAL(toggled(bool)),this,SLOT(permanentToggle(bool)));

  Online=new QCheckBox(GroupBox3,"Online");
  Online->setGeometry(QRect(10,85,290,19)); 
  Online->setText(tr("&Online"));
  Online->setChecked(true);
  QToolTip::add( Online,tr("Should the contents of the tablespace be permanent after the end of a session."));

  DefaultStorage=new QCheckBox(GroupBox3,"DefaultStorage");
  DefaultStorage->setGeometry(QRect(10,115,290,19));
  DefaultStorage->setText("&Specify Default Storage");
  DefaultStorage->setChecked(false);
  QToolTip::add( DefaultStorage,tr("Allow specification of default storage (Separate tab)") );
  connect(DefaultStorage,SIGNAL(toggled(bool)),this,SLOT(allowDefault(bool)));

  ExtentGroup=new QButtonGroup(this,"ExtentGroup");
  ExtentGroup->setGeometry(QRect(10,245,310,185)); 
  ExtentGroup->setTitle(tr("&Extent allocation"));
  QToolTip::add( ExtentGroup,tr("Specify how space allocation is managed in the tablespace."));
  
  Dictionary=new QRadioButton(ExtentGroup,"Dictionary");
  Dictionary->setGeometry(QRect(10,25,140,19)); 
  Dictionary->setText(tr("&Dictionary"));
  Dictionary->setChecked(true);
  QToolTip::add( Dictionary,tr("Manage tablespace usiing dictionary tables."));
  connect(Dictionary,SIGNAL(toggled(bool)),this,SLOT(dictionaryToggle(bool)));

  LocalAuto=new QRadioButton(ExtentGroup,"LocalAuto");
  LocalAuto->setGeometry(QRect(10,55,140,19)); 
  LocalAuto->setText(tr("Local &Autoallocation"));
  
  LocalSelect=new QRadioButton(ExtentGroup,"LocalSelect");
  LocalSelect->setGeometry(QRect(10,85,100,19)); 
  LocalSelect->setText(tr("Local &Uniform"));
  QToolTip::add( LocalSelect,tr("The tablespace is locally managed with fixed extent sizes of specified size."));
  connect(LocalSelect,SIGNAL(toggled(bool)),this,SLOT(uniformToggle(bool)));
  
  LocalUniform=new toFilesize("Extent &Size",ExtentGroup,"LocalUniform");
  LocalUniform->setGeometry(QRect(10,110,291,61)); 
  LocalUniform->setEnabled(false);
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

QStringList toStorageTablespace::getSQL()
{
  QStringList ret;
  QString str("MINIMUM EXTENT ");
  str.append(MinimumExtent->sizeString());
  ret+=str;
  if (Logging->isChecked()!=LoggingOrig||!Modify) {
    if (Logging->isChecked())
      str="LOGGING";
    else
      str="NOLOGGING";
    ret+=str;
  }
  if (Online->isChecked()!=OnlineOrig||!Modify) {
    if (Online->isChecked())
      str="ONLINE";
    else
      str="OFFLINE";
    ret+=str;
  }
  if (Permanent->isChecked()!=PermanentOrig||!Modify) {
    if (Permanent->isChecked())
      str="PERMANENT";
    else
      str="TEMPORARY";
    ret+=str;
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
    ret+=str;
  }
  return ret;
}

toStorageDatafile::toStorageDatafile(bool dispName,QWidget* parent,const char* name,WFlags fl)
  : QWidget(parent,name,fl)
{
  Modify=false;
  if (!name)
    setName("DataFile");
  setCaption(tr("Create datafile"));

  int indent=0;

  if (dispName) {
    Name=new QLineEdit(this,"Name");
    Name->setGeometry(QRect(10,30,310,23)); 
    Name->setFocus();
    connect(Name,SIGNAL(textChanged(const QString &)),this,SLOT(valueChanged(const QString &)));

    TextLabel2=new QLabel(this,"TextLabel2");
    TextLabel2->setGeometry(QRect(10,10,310,15)); 
    TextLabel2->setText(tr("&Tablespace Name"));
    TextLabel2->setBuddy(Name);
    indent=60;
  } else
    Name=NULL;

  Filename=new QLineEdit(this,"Filename");
  Filename->setGeometry(QRect(10,30+indent,210,23)); 
  if (!dispName)
    Filename->setFocus();
  connect(Filename,SIGNAL(textChanged(const QString &)),this,SLOT(valueChanged(const QString &)));

  TextLabel1=new QLabel(this,"TextLabel1");
  TextLabel1->setGeometry(QRect(10,10+indent,61,15)); 
  TextLabel1->setText(tr("&Filename"));
  TextLabel1->setBuddy(Filename);

  BrowseFile=new QPushButton(this,"BrowseFile");
  BrowseFile->setGeometry(QRect(240,25+indent,80,32)); 
  BrowseFile->setText(tr("&Browse"));
  
  InitialSize=new toFilesize("&Size",this,"InitialSize");
  InitialSize->setGeometry(QRect(10,70+indent,310,60)); 
  
  GroupBox1=new QGroupBox(this,"GroupBox1");
  GroupBox1->setGeometry(QRect(10,140+indent,311,230)); 
  GroupBox1->setTitle(tr("&Auto extend"));

  AutoExtend=new QCheckBox(GroupBox1,"Autoextend");
  AutoExtend->setGeometry(QRect(10,25,290,19)); 
  AutoExtend->setText(tr("Auto&extend datafile"));
  connect(AutoExtend,SIGNAL(toggled(bool)),this,SLOT(autoExtend(bool)));

  NextSize=new toFilesize("&Next",GroupBox1,"NextSize");
  NextSize->setGeometry(QRect(10,55,291,60)); 
  NextSize->setEnabled(false);

  UnlimitedMax=new QCheckBox(GroupBox1,"UnlimitedMax");
  UnlimitedMax->setGeometry(QRect(10,125,290,19)); 
  UnlimitedMax->setText(tr("&Unlimited max size"));
  UnlimitedMax->setEnabled(false);
  connect(UnlimitedMax,SIGNAL(toggled(bool)),this,SLOT(maximumSize(bool)));
  
  MaximumSize=new toFilesize("&Maximum size",GroupBox1,"MaximumSize");
  MaximumSize->setGeometry(QRect(10,155,291,60)); 
  MaximumSize->setEnabled(false);
  
  connect(BrowseFile,SIGNAL(clicked(void)),this,SLOT(browseFile(void)));
  
}

QStringList toStorageDatafile::getSQL(void)
{
  QStringList ret;
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
    ret+=str;
  } else {
    QString str;
    if (Filename->text()!=FilenameOrig) {
      str="RENAME FILE '";
      str+=FilenameOrig;
      str+="' TO '";
      str+=Filename->text();
      ret+=str;
    }
    if (InitialSize->value()!=InitialSizeOrig) {
      str="DATAFILE '";
      str+=Filename->text();
      str+="' RESIZE ";
      str+=InitialSize->sizeString();
      ret+=str;
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
    ret+=str;
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
  resize(470,500);
  setMinimumSize(QSize(470,500));
  setMaximumSize(QSize(470,500));
  DialogTab=new QTabWidget(this);
  DialogTab->setGeometry(10,10,330,480);

  OkButton = new QPushButton(this,"OkButton");
  OkButton->move(350,40); 
  OkButton->setText(tr("&OK"));
  OkButton->setDefault(true);

  QPushButton *CancelButton=new QPushButton(this,"CancelButton");
  CancelButton->move(350,90); 
  CancelButton->setText(tr("Cancel"));
  CancelButton->setDefault(false);
  
  connect(OkButton,SIGNAL(clicked()),this,SLOT(accept()));
  connect(CancelButton,SIGNAL(clicked()),this,SLOT(reject()));
  toHelp::connectDialog(this);
}

toStorageDialog::toStorageDialog(bool datafile,QWidget *parent)
  : QDialog(parent,"Storage Dialog",true)
{
  Setup();
  OkButton->setEnabled(false);

  if (datafile) {
    setCaption("Add datafile");
    Tablespace=NULL;
    Default=NULL;
    Datafile=new toStorageDatafile(false,DialogTab);
    DialogTab->addTab(Datafile,"Datafile");
  } else {
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
  : QDialog(parent,"Storage Dialog",true)
{
  Setup();
  Datafile=NULL;
  setCaption("Modify tablespace");
  Tablespace=new toStorageTablespace(DialogTab);
  DialogTab->addTab(Tablespace,"Tablespace");
  Default=new toStorageDefinition(DialogTab);
  DialogTab->addTab(Default,"Default Storage");
  connect(Tablespace,SIGNAL(allowStorage(bool)),this,SLOT(allowStorage(bool)));

  list<QString> result=toReadQuery(conn,SQLTablespaceInfo(conn),
				   tablespace);
  if (result.size()!=10)
    throw QString("Invalid response from query");
  Tablespace->MinimumExtent->setValue(toShift(result).toInt());

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
  : QDialog(parent,"Storage Dialog",true)
{
  Setup();
  Datafile=new toStorageDatafile(true,DialogTab);
  DialogTab->addTab(Datafile,"Datafile");
  setCaption("Modify datafile");
  Tablespace=NULL;
  Default=NULL;

  list<QString> result=toReadQuery(conn,SQLDatafileInfo(conn),
				   tablespace,filename);
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
  : toToolWidget(main,connection)
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

  QToolBar *toolbar=toAllocBar(this,"Storage manager",connection.connectString());

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

  Storage=new toResultStorage(connection,this);

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
    otl_cursor::direct_exec(otlConnect(),
			    str.utf8());
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
    otl_cursor::direct_exec(otlConnect(),
			    str.utf8());
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
    otl_cursor::direct_exec(otlConnect(),
			    str.utf8());
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
    otl_cursor::direct_exec(otlConnect(),
			    str.utf8());
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
    otl_cursor::direct_exec(otlConnect(),
			    str.utf8());
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
    otl_cursor::direct_exec(otlConnect(),
			    str.utf8());
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
    otl_cursor::direct_exec(otlConnect(),
			    str.utf8());
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
    toStorageDialog newFile(true,this);

    if (newFile.exec()) {
      QString str;
      str="ALTER TABLESPACE \"";
      str.append(Storage->currentTablespace());
      str.append("\" ADD DATAFILE ");
      str.append(newFile.Datafile->getSQL().join(" "));
      otl_cursor::direct_exec(otlConnect(),
			      str.utf8());
      refresh();
    }
  } TOCATCH
}

void toStorage::newTablespace(void)
{
  try {
    toStorageDialog newSpace(false,this);

    if (newSpace.exec()) {
      QString str;
      str="CREATE TABLESPACE \"";
      str.append(newSpace.Datafile->getName());
      str.append("\" DATAFILE ");
      str.append(newSpace.Datafile->getSQL().join(" "));
      str.append(" ");
      str.append(newSpace.Tablespace->getSQL().join(" "));
      if (newSpace.Tablespace->allowStorage()) {
	str.append(" DEFAULT ");
	str.append(newSpace.Default->getSQL());
      }
      otl_cursor::direct_exec(otlConnect(),
			      str.utf8());
      refresh();
    }
  } TOCATCH
}

void toStorage::modifyTablespace(void)
{
  try {
    toStorageDialog modifySpace(connection(),Storage->currentTablespace(),this);

    if (modifySpace.exec()) {
      QString start;
      start="ALTER TABLESPACE \"";
      start.append(Storage->currentTablespace());
      start.append("\" ");
      QStringList opr=modifySpace.Tablespace->getSQL();
      for (unsigned int i=0;i<opr.count();i++) {
	QString str=start;
	str+=opr[i];
	otl_cursor::direct_exec(otlConnect(),
				str.utf8());
      }
      if (modifySpace.Tablespace->allowStorage()) {
	QString str=start;
	str.append("DEFAULT ");
	str.append(modifySpace.Default->getSQL());
	otl_cursor::direct_exec(otlConnect(),
				str.utf8());
      }
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
      QString start="ALTER DATABASE ";
      QStringList opr=modifySpace.Datafile->getSQL();
      for (unsigned int i=0;i<opr.count();i++) {
	QString str=start;
	str+=opr[i];
	otl_cursor::direct_exec(otlConnect(),
				str.utf8());
      }
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
      otl_cursor::direct_exec(otlConnect(),
			      str.utf8());
      refresh();
    }
  } TOCATCH
}
