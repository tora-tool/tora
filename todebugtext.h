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
