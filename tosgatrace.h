//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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

#include <qstring.h>
#include "totool.h"

class toMain;
class toResultView;
class QTabWidget;
class QComboBox;
class toSGAStatement;
class toConnection;
class toTool;
class QListViewItem;

class toSGATrace : public toToolWidget {
  Q_OBJECT

  toResultView *Trace;
  QTabWidget *ResultTab;
  QComboBox *Schema;
  QComboBox *Type;

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
};

#endif
