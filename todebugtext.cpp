#include <qapplication.h>

#include "tomain.h"
#include "todebugtext.h"
#include "toconnection.h"
#include "todebug.h"
#include "icons/breakpoint.xpm"

#define TO_BREAK_COL 5

toBreakpointItem::toBreakpointItem(QListView *parent,QListViewItem *after,toConnection &conn,
				   const QString &schema,const QString &type,
				   const QString &object,int line)
: QListViewItem(parent,after),Connection(conn)
{
  setText(0,schema);
  setText(1,object);
  setText(2,QString::number(line+1));
  setText(3,type);
  if(type=="PACKAGE"||type=="PROCEDURE"||type=="FUNCTION"||type=="TYPE")
    Namespace=TO_NAME_TOPLEVEL;
  else if (type=="PACKAGE BODY"||type=="TYPE BODY")
    Namespace=TO_NAME_BODY;
  else
    Namespace=TO_NAME_NONE;
  Line=line;
  setBreakpoint();
}

void toBreakpointItem::setBreakpoint(void)
{
  bool ok=false;
  try {
    clearBreakpoint();
    otl_stream str(1,
		   "
DECLARE
    proginf DBMS_DEBUG.PROGRAM_INFO;
    bnum BINARY_INTEGER;
    ret BINARY_INTEGER;
BEGIN
    proginf.Namespace:=:type<int,in>;
    proginf.Name:=:name<char[100],in>;
    proginf.Owner:=:schema<char[100],in>;
    proginf.DbLink:=NULL;
    proginf.LibUnitType:=DBMS_DEBUG.LibUnitType_Procedure;
    proginf.EntryPointName:=NULL;
    proginf.Line#:=:line<int,in>;
    ret:=DBMS_DEBUG.SET_BREAKPOINT(proginf,proginf.Line#,bnum,0,1);
    SELECT ret,bnum INTO :ret<int,out>,:bnum<int,out> FROM DUAL;
END;",
		   Connection.connection());
    str<<Namespace;
    str<<(const char *)text(1);
    str<<(const char *)text(0);
    str<<Line+1;
    int ret;
    str>>ret;
    if (ret==TO_SUCCESS) {
      int bnum;
      str>>bnum;
      if (bnum!=0) {
	setText(TO_BREAK_COL,QString::number(bnum));
	setText(4,"ENABLED");
      } else
	setText(4,"DEFERED");
      ok=true;
    } else if (ret==TO_ERROR_ILLEGAL_LINE) {
      toStatusMessage("Can not enable breakpoint, not a valid line. Perhaps needs to recompile.");
    } else if (ret==TO_ERROR_BAD_HANDLE) {
      toStatusMessage("Can not enable breakpoint, not a valid object. Perhaps needs to compile.");
    }
  } TOCATCH
  if (!ok)
    setText(4,"NOT SET");
}

void toBreakpointItem::clearBreakpoint(void)
{
  if (text(4)=="ENABLED"||text(4)=="DISABLED") {
    setText(4,"NOT SET");
    try {
      otl_stream str(1,
		     "
DECLARE
    bnum BINARY_INTEGER;
    ret BINARY_INTEGER;
BEGIN
    ret:=DBMS_DEBUG.DELETE_BREAKPOINT(:bnum<char[100],in>);
    SELECT ret INTO :ret<int,out> FROM DUAL;
END;",
		     Connection.connection());
      str<<(const char *)text(TO_BREAK_COL);
      int res;
      str>>res;
      if (res!=TO_SUCCESS) {
	QString str("Failed to remove breakpoint (Reason ");
	str+=QString::number(res);
	str+=")";
	toStatusMessage(str);
      }
    } TOCATCH
    setText(TO_BREAK_COL,NULL);
  }
}

#define DEBUG_INDENT 10

static QPixmap *toBreakpointPixmap;

void toDebugText::setData(const QString &schema,const QString &type,const QString &object)
{
  Schema=schema;
  Type=type;
  Object=object;
  FirstItem=NULL;
  NoBreakpoints=false;
  repaint();
}

toDebugText::toDebugText(QListView *breakpoints,
			 toConnection &connection,
			 QWidget *parent,
			 const char *name)
  : toHighlightedText(parent,name),
    Connection(connection),
    Breakpoints(breakpoints)
{
  setLeftIgnore(DEBUG_INDENT);
  setHMargin(DEBUG_INDENT+hMargin());
  setMouseTracking(true);
  QRect view=viewRect();
  LastX=DEBUG_INDENT+view.left();
  CurrentItem=FirstItem=NULL;
  NoBreakpoints=false;
  if (!toBreakpointPixmap)
    toBreakpointPixmap=new QPixmap((const char **)breakpoint_xpm);
}

bool toDebugText::checkItem(toBreakpointItem *item)
{
  if (!item)
    return false;
  if (item->text(0)==Schema&&
      item->text(3)==Type&&
      item->text(1)==Object)
    return true;
  return false;
}

void toDebugText::clear(void)
{
  setData("","","");
  FirstItem=CurrentItem=NULL;
  NoBreakpoints=false;
  toHighlightedText::clear();
}

bool toDebugText::hasBreakpoint(int row)
{
  if (!FirstItem&&!NoBreakpoints) {
    FirstItem=dynamic_cast<toBreakpointItem *>(Breakpoints->firstChild());
    while(!checkItem(FirstItem)&&FirstItem)
      FirstItem=dynamic_cast<toBreakpointItem *>(FirstItem->nextSibling());
    if (!FirstItem)
      NoBreakpoints=true;
    CurrentItem=FirstItem;
  }

  if (!NoBreakpoints) {
    toBreakpointItem *next=dynamic_cast<toBreakpointItem *>(CurrentItem->nextSibling());
    bool hasNext=checkItem(next);
    int nextLine=hasNext?next->line():row+1;

    if (CurrentItem->line()==row)
      return true;
    if (row==nextLine) {
      CurrentItem=next;
      return true;
    }
    if (!hasNext&&row>CurrentItem->line())
      return false;
    if (row<CurrentItem->line()) {
      if (CurrentItem==FirstItem)
	return false;
      CurrentItem=FirstItem;
      return hasBreakpoint(row);
    }
    if (row>nextLine) {
      CurrentItem=next;
      return hasBreakpoint(row);
    }
  }
  return false;
}

void toDebugText::paintCell(QPainter *painter,int row,int col)
{
  toHighlightedText::paintCell(painter,row,col);
  QPalette cp=qApp->palette();
  painter->fillRect(0,0,DEBUG_INDENT-2,cellHeight(),cp.active().background());
  painter->fillRect(DEBUG_INDENT-2,0,1,cellHeight(),cp.active().midlight());
  painter->fillRect(DEBUG_INDENT-1,0,1,cellHeight(),cp.active().dark());
  if (hasBreakpoint(row)) {
    int h=max((cellHeight()-toBreakpointPixmap->height())/2,0);
    painter->drawPixmap(0,h,*toBreakpointPixmap);
  }
}

void toDebugText::paintEvent(QPaintEvent *pe)
{
  toHighlightedText::paintEvent(pe);
  int fromY=0;
  if (numRows()==0||rowYPos(numRows()-1,&fromY)) {
    if (numRows())
      fromY+=cellHeight();
    QRect view=viewRect();
    if (fromY+view.top()<view.height()) {
      QPainter painter(this);
      QPalette cp=qApp->palette();
      painter.fillRect(view.left(),fromY,DEBUG_INDENT-2,view.bottom()-fromY,cp.active().background());
      painter.fillRect(view.left()+DEBUG_INDENT-2,fromY,1,view.bottom()-fromY,cp.active().midlight());
      painter.fillRect(view.left()+DEBUG_INDENT-1,fromY,1,view.bottom()-fromY,cp.active().dark());
    }
  }
}

void toDebugText::mouseMoveEvent(QMouseEvent *me)
{
  QRect view=viewRect();
  if (me->x()>DEBUG_INDENT+view.left()) {
    if (LastX<=DEBUG_INDENT+view.left())
      setCursor(Qt::ibeamCursor);
    if(me->state()!=0)
      toHighlightedText::mouseMoveEvent(me);
  } else {
    if (LastX>DEBUG_INDENT+view.left())
      setCursor(Qt::ibeamCursor);
    setCursor(Qt::arrowCursor);
  }
  LastX=me->x();
}

void toDebugText::mouseDoubleClickEvent(QMouseEvent *me)
{
  if (me->x()+xOffset()>DEBUG_INDENT)
    toHighlightedText::mouseDoubleClickEvent(me);
  else {
    int row=findRow(me->y());
    if (row>=0) {
      if (hasBreakpoint(row)) {
	CurrentItem->clearBreakpoint();
	delete CurrentItem;
	if (FirstItem==CurrentItem) {
	  NoBreakpoints=false;
	  CurrentItem=FirstItem=NULL;
	} else
	  CurrentItem=FirstItem;
      } else {
	if (CurrentItem&&CurrentItem->line()>row)
	  new toBreakpointItem(Breakpoints,NULL,Connection,
			       Schema,Type,Object,row);
	else
	  new toBreakpointItem(Breakpoints,CurrentItem,Connection,
			       Schema,Type,Object,row);
	FirstItem=CurrentItem=NULL;
	NoBreakpoints=false;
      }
      repaint();
    }
  }
}
