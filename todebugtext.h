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
 ****************************************************************************/


#ifndef __TODEBUGTEXT_H
#define __TODEBUGTEXT_H

#include <qlistview.h>

#include "tohighlightedtext.h"
class toConnection;

class toBreakpointItem : public QListViewItem {
  int Line;
  int Namespace;
  toConnection &Connection;
public:
  toBreakpointItem(QListView *parent,QListViewItem *after,toConnection &conn,
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
  toConnection &Connection;
  int LastX;

  QListView *Breakpoints;
  bool NoBreakpoints;
  toBreakpointItem *FirstItem;
  toBreakpointItem *CurrentItem;

  bool checkItem(toBreakpointItem *item);
  bool hasBreakpoint(int row);
public:
  toDebugText(QListView *breakpoints,
	      toConnection &connection,
	      QWidget *parent,
	      const char *name=NULL);

  void toggleBreakpoint(int row=-1,bool enable=false);

  void setData(const QString &schema,const QString &type,const QString &data);
  const QString &schema(void) const
  { return Schema; }
  const QString &object(void) const
  { return Object; }
  void setType(const QString &type)
  { Type=type; }
  void setSchema(const QString &schema)
  { Schema=schema; }
  const QString &type(void) const
  { return Type; }
  void clear(void);

  bool readData(toConnection &connection,QListView *);
  bool compile(void);
protected:
  virtual void paintCell(QPainter *painter,int row,int col);
  virtual void paintEvent(QPaintEvent *pe);
  virtual void mouseDoubleClickEvent (QMouseEvent *me);
  virtual void mouseMoveEvent (QMouseEvent *me);
};

#endif
