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

#ifndef __TOSTORAGE_H
#define __TOSTORAGE_H

#include <list>

#include <qdialog.h>

#include "totool.h"
#include "tostoragetablespaceui.h"
#include "tostoragedialogui.h"
#include "tostoragedatafileui.h"

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

class toStorageTablespace : public toStorageTablespaceUI
{ 
  Q_OBJECT

  bool Modify;
  bool LoggingOrig;
  bool OnlineOrig;
  bool PermanentOrig;

public:
  toStorageTablespace(QWidget *parent=0,const char *name=0,WFlags fl=0);

  std::list<QString> sql(void);
  bool allowStorage(void);

  friend class toStorageDialog;
signals:
  void allowStorage(bool);

public slots:
  virtual void permanentToggle(bool);
  virtual void dictionaryToggle(bool); 
  virtual void uniformToggle(bool);
  virtual void allowDefault(bool);
};

class toStorageDatafile : public toStorageDatafileUI
{ 
  Q_OBJECT

  bool Modify;
  int InitialSizeOrig;
  int NextSizeOrig;
  int MaximumSizeOrig;
  QString FilenameOrig;

public:
  toStorageDatafile(bool dispName,QWidget *parent = 0,const char *name=0,WFlags fl=0);

  QString getName(void);
  std::list<QString> sql(void);

  friend class toStorageDialog;
signals:
  void validContent(bool);

public slots:
  virtual void browseFile(void);
  virtual void autoExtend(bool);
  virtual void maximumSize(bool);
  virtual void valueChanged(const QString &);
};

class toStorageDialog : public toStorageDialogUI {
  Q_OBJECT
  enum {
    NewTablespace,
    NewDatafile,
    ModifyTablespace,
    ModifyDatafile
  } Mode;
  QString TablespaceOrig;
  void Setup(void);
public:
  toStorageDatafile *Datafile;
  toStorageTablespace *Tablespace;
  toStorageDefinition *Default;

  toStorageDialog(toConnection &conn,const QString &Tablespace,QWidget *parent);
  toStorageDialog(toConnection &conn,const QString &Tablespace,
		  const QString &file,QWidget *parent);
  toStorageDialog(const QString &tablespace,QWidget *parent);

  std::list<QString> sql(void);
public slots:
  void validContent(bool val);
  void allowStorage(bool val);
  virtual void displaySQL(void);
};

class toStorage : public toToolWidget {
  Q_OBJECT

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
