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

#ifndef __TOSQLEDIT_H
#define __TOSQLEDIT_H

#include "totool.h"

class toWorksheet;
class toMarkedText;
class QComboBox;
class QListView;
class QLineEdit;
class QToolButton;

class toSQLEdit : public toToolWidget {
  Q_OBJECT

  QListView *Statements;
  QLineEdit *Name;
  toMarkedText *Description;
  QComboBox *Version;
  toWorksheet *Editor;
  QToolButton *TrashButton;
  QToolButton *CommitButton;
  QString LastVersion;
  QString Filename;

protected:
  void updateStatements(const QString &def=QString::null);

  bool checkStore(bool);
  virtual bool close(bool del);

  void selectionChanged(const QString &ver);
  void changeSQL(const QString &name,const QString &ver);
public:
  toSQLEdit(QWidget *parent,toConnection &connection);
  virtual ~toSQLEdit();

public slots:
  void loadSQL(void);
  void saveSQL(void);
  void deleteVersion(void);
  void selectionChanged(void);
  void changeVersion(const QString &);
  void commitChanges(void);
  void editSQL(const QString &);
  void newSQL();
};

#endif
