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
