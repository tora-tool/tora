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
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
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

#ifndef __TOSTORAGE_H
#define __TOSTORAGE_H

#include <qvbox.h>
#include <qdialog.h>

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QRadioButton;
class QTabWidget;

class toStorageDefinition;
class toConnection;
class toResultStorage;
class toFilesize;
class toStorageDialog;

class toStorageTablespace : public QWidget
{ 
  Q_OBJECT

  toFilesize *MinimumExtent;
  QButtonGroup *ExtentGroup;
  toFilesize *LocalUniform;
  QRadioButton *Dictionary;
  QRadioButton *LocalAuto;
  QRadioButton *LocalSelect;
  QGroupBox *GroupBox3;
  QCheckBox *Logging;
  QCheckBox *Online;
  QCheckBox *Permanent;
  QCheckBox *DefaultStorage;

  bool Modify;
  bool LoggingOrig;
  bool OnlineOrig;
  bool PermanentOrig;

public:
  toStorageTablespace(QWidget *parent=0,const char *name=0,WFlags fl=0);

  QStringList getSQL(void);
  bool allowStorage(void);

  friend class toStorageDialog;
signals:
  void allowStorage(bool);

public slots:
  void permanentToggle(bool);
  void dictionaryToggle(bool); 
  void uniformToggle(bool);
  void allowDefault(bool);
};

class toStorageDatafile : public QWidget
{ 
  Q_OBJECT

  QLabel *TextLabel1;
  QLabel *TextLabel2;
  QGroupBox *GroupBox1;
  QCheckBox *AutoExtend;
  toFilesize *NextSize;
  QCheckBox *UnlimitedMax;
  toFilesize *MaximumSize;
  QPushButton *BrowseFile;
  QLineEdit *Filename;
  QLineEdit *Name;
  toFilesize *InitialSize;

  bool Modify;
  int InitialSizeOrig;
  int NextSizeOrig;
  int MaximumSizeOrig;
  QString FilenameOrig;

public:
  toStorageDatafile(bool dispName,QWidget *parent = 0,const char *name=0,WFlags fl=0);

  QString getName(void);
  QStringList getSQL(void);

  friend class toStorageDialog;
signals:
  void validContent(bool);

public slots:
  void browseFile(void);
  void autoExtend(bool);
  void maximumSize(bool);
  void valueChanged(const QString &);
};

class toStorageDialog : public QDialog {
  Q_OBJECT
  
  QPushButton *OkButton;

  void Setup(void);

  QTabWidget *DialogTab;
public:
  toStorageDatafile *Datafile;
  toStorageTablespace *Tablespace;
  toStorageDefinition *Default;

  toStorageDialog(toConnection &conn,const QString &Tablespace,QWidget *parent);
  toStorageDialog(toConnection &conn,const QString &Tablespace,
		  const QString &file,QWidget *parent);
  toStorageDialog(bool datafile,QWidget *parent);
public slots:
  void validContent(bool val);
  void allowStorage(bool val);
};

class toStorage : public QVBox {
  Q_OBJECT

  toConnection &Connection;

  toResultStorage *Storage;

  QToolButton *OnlineButton;
  QToolButton *OfflineButton;
  QToolButton *CoalesceButton;
  QToolButton *LoggingButton;
  QToolButton *EraseLogButton;
  QToolButton *ModTablespaceButton;
  QToolButton *ReadOnlyButton;
  QToolButton *ReadWriteButton;
  QToolButton *NewFileButton;
  QToolButton *MoveFileButton;
  QToolButton *ModFileButton;
public:
  toStorage(QWidget *parent,toConnection &connection);
  ~toStorage();

public slots:
  void refresh(void);

  void coalesce(void); 

  void online(void); 
  void offline(void); 

  void logging(void);
  void noLogging(void);

  void readWrite(void);
  void readOnly(void);

  void newDatafile(void);
  void newTablespace(void);
  void modifyTablespace(void);
  void modifyDatafile(void);
  void moveFile(void);

  void selectionChanged(void);
};

#endif
