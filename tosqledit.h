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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef __TOSQLEDIT_H
#define __TOSQLEDIT_H

#include <qvbox.h>

class toWorksheet;
class toMarkedText;
class QComboBox;
class QListView;
class QLineEdit;
class QToolButton;

class toSQLEdit : public QVBox {
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
  toConnection &Connection;
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
};

#endif
