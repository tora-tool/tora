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

#ifndef __TOCURRENT_H
#define __TOCURRENT_H

#include "totool.h"

class toResultView;
class QTabWidget;
class toResultStats;

class toCurrent : public toToolWidget {
  Q_OBJECT

  QTabWidget *Tabs;
  toResultView *Version;
  toListView *Grants;
  toResultView *Parameters;
  toResultStats *Statistics;
  virtual void addRole(QListViewItem *parent);
  virtual void addList(QListViewItem *parent,const QString &typ,toSQL &sql,const QString &role=QString::null);
public:
  toCurrent(QWidget *parent,toConnection &connection);
  virtual ~toCurrent();
public slots:
  void refresh(void);
};

#endif
