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

#ifndef __TOSCRIPT_H
#define __TOSCRIPT_H

#include <list>

#include "tohelp.h"
#include "toscriptui.h"

class toWorksheet;
class toConnection;
class toScriptUI;
class toResultView;
class toExtract;

class toScript : public toScriptUI, public toHelpContext {
  Q_OBJECT

  toScriptUI *ScriptUI;
  toWorksheet *Worksheet;
  toResultView *DropList;
  toResultView *CreateList;
  toConnection &Connection;

  void setupExtract(toExtract &);
  void changeConnection(int,bool source);
  void changeSchema(int,bool source);
  std::list<QString> createObjectList(QListView *);
  void fillDifference(std::list<QString> &objects,QListView *list);
public:
  toScript(QWidget *parent,toConnection &connection);
  virtual ~toScript();


public slots:
  void execute(void);
  void changeMode(int);
  void changeSource(int val)
  { changeConnection(val,true); }
  void changeDestination(int val)
  { changeConnection(val,false); }
  void objectClicked(QListViewItem *item);
  void changeSourceSchema(int val)
  { changeSchema(val,true); }
  void changeDestinationSchema(int val)
  { changeSchema(val,false); }
  void newSize(void);
  void removeSize(void);
};

#endif
