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


#ifndef __TORESULTVIEW_H
#define __TORESULTVIEW_H

#include <qlistview.h>
#include "toconnection.h"
#include "toresult.h"
#include "otlv32.h"

class QListViewItem;
class toResultView;
class toResultTip;

class toResultViewItem : public QListViewItem {
public:
  toResultViewItem(QListView *parent,QListViewItem *after,const char *buffer=NULL)
    : QListViewItem(parent,after,NULL)
  { if (buffer) setText(0,buffer); }
  toResultViewItem(QListViewItem *parent,QListViewItem *after,const char *buffer=NULL)
    : QListViewItem(parent,after,NULL)
  { if (buffer) setText(0,buffer); }
  virtual void paintCell(QPainter * p,const QColorGroup & cg,int column,int width,int align);
  virtual QString text(int col) const;
  virtual int width(const QFontMetrics &fm, const QListView *top, int column) const;
  virtual QString allText(int col) const
  { return QListViewItem::text(col); }
};

class toResultViewMLine : public toResultViewItem {
private:
  int Lines;
public:
  toResultViewMLine(QListView *parent,QListViewItem *after,const char *buffer=NULL)
    : toResultViewItem(parent,after,NULL)
  { Lines=1; if (buffer) setText(0,buffer); }
  toResultViewMLine(QListViewItem *parent,QListViewItem *after,const char *buffer=NULL)
    : toResultViewItem(parent,after,NULL)
  { Lines=1; if (buffer) setText(0,buffer); }
  virtual void setup(void);
  virtual void setText (int,const QString &);
  virtual QString text(int col) const
  { return QListViewItem::text(col); }
  virtual QString allText(int col) const
  { return QListViewItem::text(col); }
  virtual int width(const QFontMetrics &fm, const QListView *top, int column) const;

  virtual void paintCell (QPainter *pnt,const QColorGroup & cg,int column,int width,int alignment);
};

class toResultView : public QListView, public toResult {
  Q_OBJECT

protected:
  QString SQL;
  toConnection &Connection;
  otl_stream *Query;
  int DescriptionLen;
  otl_column_desc *Description;
  QListViewItem *LastItem;

  toResultTip *AllResult;

  int RowNumber;
  bool ReadableColumns;
  bool NumberColumn;
  bool ReadAll;
  void setup(bool readable,bool dispCol);

  virtual void contentsMouseDoubleClickEvent (QMouseEvent *e);
  virtual bool eof(void)
  { return !Query||Query->eof(); }

  virtual void keyPressEvent (QKeyEvent * e);
public:
  toResultView(bool readable,bool numCol,toConnection &conn,QWidget *parent,const char *name=NULL);
  toResultView(toConnection &conn,QWidget *parent,const char *name=NULL);
  ~toResultView()
  { delete Query; }

  void setReadAll(bool all)
  { ReadAll=all;}

  void readAll(void);

  virtual QListViewItem *createItem(QListView *parent,QListViewItem *last,const char *str)
  { return new toResultViewItem(parent,last,str); }

  void setSQL(const QString &sql)
    { SQL=sql; }
  virtual void query(const QString &sql,const list<QString> &param);
  void query(const QString &sql)
  { list<QString> p; query(sql,p); }
public slots:
  virtual void refresh(void)
  { query(SQL); }
  virtual void changeParams(const QString &Param1)
  { list<QString> p; p.insert(p.end(),Param1); query(SQL,p); }
  virtual void changeParams(const QString &Param1,const QString &Param2)
  { list<QString> p; p.insert(p.end(),Param1); p.insert(p.end(),Param2); query(SQL,p); }
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3)
  { list<QString> p; p.insert(p.end(),Param1); p.insert(p.end(),Param2); p.insert(p.end(),Param3); query(SQL,p); }
  virtual void addItem(void);
};

#endif
