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

#ifndef TOSGATRACE_H
#define TOSGATRACE_H

#include "totool.h"

#include <qstring.h>

#define TOSQL_LONGOPS "toSGATrace:LongOps"

class QComboBox;
class QListViewItem;
class QPopupMenu;
class QTabWidget;
class toConnection;
class toMain;
class toResultCombo;
class toResultLong;
class toSGAStatement;
class toTool;

class toSGATrace : public toToolWidget {
  Q_OBJECT

  toResultLong *Trace;
  QTabWidget *ResultTab;
  toResultCombo *Schema;
  QComboBox *Type;
  QComboBox *Refresh;
  QComboBox *Limit;
  QPopupMenu *ToolMenu;

  toSGAStatement *Statement;
  QString CurrentSchema;

  void updateSchemas(void);

public:
  toSGATrace(QWidget *parent,toConnection &connection);
public slots:
  void changeSchema(const QString &str);
  void changeItem(QListViewItem *item);
  void changeRefresh(const QString &str);
  void refresh(void);
  void windowActivated(QWidget *widget);
};

#endif
