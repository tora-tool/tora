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


#ifndef __TORESULTCONTENT_H
#define __TORESULTCONTENT_H

#include <list>
#include <qtable.h>
#include "toresult.h"
#include "toconnection.h"

class toResultContent : public QTable,public toResult {
  Q_OBJECT

  toConnection &Connection;
  QString Owner;
  QString Table;
  list<QString> OrigValues;
  otl_stream *Query;
  otl_column_desc *Description;
  int Row;
  int CurrentRow;
  bool AddRow;

  void addRow(void);
  void wrongUsage(void);

  virtual void drawContents(QPainter * p,int cx,int cy,int cw,int ch);
  virtual QWidget *beginEdit(int row,int col,bool replace);
  virtual void paintCell(QPainter *p,int row,int col,const QRect &cr,bool selected);
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void focusInEvent (QFocusEvent *e);
  virtual void focusOutEvent (QFocusEvent *e); 
  virtual void activateNextCell();
public:
  toResultContent(toConnection &conn,QWidget *parent,const char *name=NULL);
  virtual void query(const QString &sql,const list<QString> &param)
  { wrongUsage(); }
  virtual void changeParams(const QString &Param1)
  { wrongUsage(); }
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3)
  { wrongUsage(); }

  void readAll(void);
  void print(void);
public slots:
  virtual void refresh(void)
  { changeParams(Owner,Table); }
  virtual void changeParams(const QString &Param1,const QString &Param2);
  void changePosition(int col,int row);
};

#endif
