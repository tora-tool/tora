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

  bool readData(toConnection &connection);
  bool compile(void);
protected:
  virtual void paintCell(QPainter *painter,int row,int col);
  virtual void paintEvent(QPaintEvent *pe);
  virtual void mouseDoubleClickEvent (QMouseEvent *me);
  virtual void mouseMoveEvent (QMouseEvent *me);
};

#endif
