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

#ifndef __TOSGATRACE_H
#define __TOSGATRACE_H

#include <qdialog.h>

#include "totool.h"
#include "torollbackdialogui.h"

class toConnection;
class toResultItem;
class toResultView;
class toSGAStatement;
class toStorageDefinition;

class toRollbackDialog : public toRollbackDialogUI
{ 
  Q_OBJECT
    
  toStorageDefinition *Storage;

public:
  toRollbackDialog(toConnection &conn,QWidget *parent=0,const char *name=0);

  std::list<QString> sql(void);

public slots:
  void valueChanged(const QString &str);
  void displaySQL(void); 
};

class toRollback : public toToolWidget {
  Q_OBJECT

  toResultView *Segments;
  toResultView *Statements;

  QWidget *CurrentTab;

  toSGAStatement *CurrentStatement;
  toResultItem *CurrentTransaction;

  QToolButton *OnlineButton;
  QToolButton *OfflineButton;
  QToolButton *DropButton;

  QString currentSegment(void);

public:
  toRollback(QWidget *parent,toConnection &connection);
public slots:
  void changeStatement(QListViewItem *item);
  void changeItem(QListViewItem *item);
  void changeRefresh(const QString &str);

  void refresh(void);
  void online(void);
  void offline(void);
  void addSegment(void);
  void dropSegment(void);
};

#endif
