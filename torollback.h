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

#ifndef __TOSGATRACE_H
#define __TOSGATRACE_H

#include <qdialog.h>
#include "totool.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTabWidget;
class QTimer;
class QToolButton;

class toConnection;
class toResultItem;
class toResultView;
class toSGAStatement;
class toStorageDefinition;

class toRollbackDialog : public QDialog
{ 
  Q_OBJECT
    
  QTabWidget *DialogTabs;
  QWidget *RollbackInfo;
  QLineEdit *Name;
  QLabel *TextLabel1;
  QCheckBox *Public;
  QLabel *TextLabel1_2;
  QComboBox *Tablespace;
  toStorageDefinition *Storage;
  QPushButton *OkButton;

public:
  toRollbackDialog(toConnection &conn,QWidget *parent=0,const char *name=0);

  QString getSQL(void);

public slots:
  void valueChanged(const QString &str);
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
