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


#ifndef __TORESULTITEM_H
#define __TORESULTITEM_H

#include <qscrollview.h>
#include <qfont.h>
#include "toconnection.h"
#include "toresult.h"
#include "otlv32.h"

class QGrid;

class toResultItem : public QScrollView, public toResult {
  Q_OBJECT
  QString SQL;

  QGrid *Result;

  int NumWidgets;
  int WidgetPos;
  QWidget **Widgets;

  bool ReadableColumns;
  bool ShowTitle;
  bool AlignRight;
  QFont DataFont;

  void setup(int num,bool readable);
protected:
  toConnection &Connection;
  void start(void);
  void addItem(const QString &title,const QString &value);
  void done(void);
public:
  toResultItem(int num,bool readable,toConnection &conn,QWidget *parent,const char *name=NULL);
  toResultItem(int num,toConnection &conn,QWidget *parent,const char *name=NULL);

  void setSQL(const QString &sql)
  { SQL=sql; }
  void showTitle(bool val)
  { ShowTitle=val; }
  void alignRight(bool val)
  { AlignRight=val; }
  void dataFont(const QFont &val)
  { DataFont=val; }

  virtual QString query(const QString &sql,const QString *Param1=NULL,const QString *Param2=NULL,const QString *Param3=NULL);
public slots:
  virtual void refresh(void)
  { query(SQL); }
  virtual void changeParams(const QString &Param1)
  { query(SQL,&Param1); }
  virtual void changeParams(const QString &Param1,const QString &Param2)
  { query(SQL,&Param1,&Param2); }
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3)
  { query(SQL,&Param1,&Param2,&Param3); }
};

#endif
