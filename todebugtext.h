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

#ifndef __TODEBUGTEXT_H
#define __TODEBUGTEXT_H

#include <qlistview.h>

#include "tohighlightedtext.h"
class toConnection;
class toDebug;

class toBreakpointItem : public QListViewItem {
  int Line;
  int Namespace;
public:
  toBreakpointItem(QListView *parent,QListViewItem *after,
		   const QString &schema,const QString &type,const QString &object,int line);
  void setBreakpoint(void);
  void clearBreakpoint(void);
  void disableBreakpoint(void);
  void enableBreakpoint(void);
  int line()
  { return Line; }
};

class toDebugText : public toHighlightedText {
  QString Schema;
  QString Object;
  QString Type;
  int LastX;
  toDebug *Debugger;

  QListView *Breakpoints;
  bool NoBreakpoints;
  toBreakpointItem *FirstItem;
  toBreakpointItem *CurrentItem;

  bool checkItem(toBreakpointItem *item);
  bool hasBreakpoint(int row);
public:
  toDebugText(QListView *breakpoints,
	      QWidget *parent,
	      toDebug *debugger,
	      const char *name=NULL);

  void toggleBreakpoint(int row=-1,bool enable=false);

  void setData(const QString &schema,const QString &type,const QString &data);
  const QString &schema(void) const
  { return Schema; }
  const QString &object(void) const
  { return Object; }
  void setType(const QString &type)
  { setData(Schema,type,Object); }
  void setSchema(const QString &schema)
  { setData(schema,Type,Object); }
  const QString &type(void) const
  { return Type; }
  void clear(void);

  bool readData(toConnection &connection,QListView *);
  bool readErrors(toConnection &connection);
  bool compile(void);
protected:
  virtual void paintCell(QPainter *painter,int row,int col);
  virtual void paintEvent(QPaintEvent *pe);
  virtual void mouseDoubleClickEvent (QMouseEvent *me);
  virtual void mouseMoveEvent (QMouseEvent *me);
};

#endif
