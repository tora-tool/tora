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

#ifndef __TOSEARCHREPLACE_H
#define __TOSEARCHREPLACE_H

#include "tosearchreplaceui.h"
#include "tohelp.h"

class toMarkedText;
class toListView;
class toResultContent;

class toSearchReplace : public toSearchReplaceUI, public toHelpContext {
  toMarkedText *Text;
  toListView *List;
  toResultContent *Content;
  bool FromTop;

  int findIndex(const QString &str,int line,int col);
  void findPosition(const QString &str,int index,int &line,int &col);
  bool findString(const QString &str,int &startPos,int &endPos);
  void release(void);

public:
  toSearchReplace(QWidget *parent);

  void setTarget(toMarkedText *parent);
  void setTarget(toListView *parent);
  void setTarget(toResultContent *parent);
  bool searchNextAvailable()
  { return SearchNext->isEnabled(); }
public slots:
  virtual void destroyed();
  virtual void replaceAll();
  virtual void replace();
  virtual void search();
  virtual void searchNext();
  virtual void searchChanged();
  virtual void displayHelp();
};

#endif
