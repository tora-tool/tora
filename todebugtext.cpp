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
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
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

TO_NAMESPACE;

#include <qapplication.h>

#include "tomain.h"
#include "todebugtext.h"
#include "toconnection.h"
#include "todebug.h"
#include "tosql.h"

#include "icons/breakpoint.xpm"
#include "icons/disbreakpoint.xpm"

#define TO_BREAK_COL 5

toBreakpointItem::toBreakpointItem(QListView *parent,QListViewItem *after,toConnection &conn,
				   const QString &schema,const QString &type,
				   const QString &object,int line)
: QListViewItem(parent,after),Connection(conn)
{
  if (schema.isNull())
    setText(2,"");
  else
    setText(2,schema);
  if (object.isNull())
    setText(0,"");
  else
    setText(0,object);
  if (type.isNull())
    setText(3,"");
  else
    setText(3,type);
  setText(1,QString::number(line+1));
  if(type=="PACKAGE"||type=="PROCEDURE"||type=="FUNCTION"||type=="TYPE")
    Namespace=TO_NAME_TOPLEVEL;
  else if (type=="PACKAGE BODY"||type=="TYPE BODY")
    Namespace=TO_NAME_BODY;
  else
    Namespace=TO_NAME_NONE;
  Line=line;
  setText(4,"DEFERED");
}

static toSQL SQLBreakpoint("toDebug:SetBreakpoint",
			   "DECLARE\n"
			   "    proginf DBMS_DEBUG.PROGRAM_INFO;\n"
			   "    bnum BINARY_INTEGER;\n"
			   "    ret BINARY_INTEGER;\n"
			   "BEGIN\n"
			   "    proginf.Namespace:=:type<int,in>;\n"
			   "    proginf.Name:=:name<char[100],in>;\n"
			   "    proginf.Owner:=:schema<char[100],in>;\n"
			   "    proginf.DbLink:=NULL;\n"
			   "    proginf.LibUnitType:=DBMS_DEBUG.LibUnitType_Procedure;\n"
			   "    proginf.EntryPointName:=NULL;\n"
			   "    proginf.Line#:=:line<int,in>;\n"
			   "    ret:=DBMS_DEBUG.SET_BREAKPOINT(proginf,proginf.Line#,bnum,0,1);\n"
			   "    SELECT ret,bnum INTO :ret<int,out>,:bnum<int,out> FROM DUAL;\n"
			   "END;",
			   "Set breakpoint, must have same bindings");
		       

void toBreakpointItem::setBreakpoint(void)
{
  bool ok=false;
  try {
    clearBreakpoint();
    otl_stream str(1,
		   SQLBreakpoint(Connection),
		   Connection.connection());
    str<<Namespace;
    str<<text(0).utf8();
    str<<text(2).utf8();
    str<<Line+1;
    int ret;
    str>>ret;
    if (ret==TO_SUCCESS) {
      int bnum;
      str>>bnum;
      setText(TO_BREAK_COL,QString::number(bnum));
      setText(4,"ENABLED");
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

static toSQL SQLClearBreakpoint("toDebug:ClearBreakpoint",
				"DECLARE\n"
				"    bnum BINARY_INTEGER;\n"
				"    ret BINARY_INTEGER;\n"
				"BEGIN\n"
				"    bnum:=:bnum<int,in>;\n"
				"    ret:=DBMS_DEBUG.DELETE_BREAKPOINT(bnum);\n"
				"    SELECT ret INTO :ret<int,out> FROM DUAL;\n"
				"END;",
				"Clear breakpoint, must have same bindings");

void toBreakpointItem::clearBreakpoint()
{
  if (text(4)=="ENABLED"&&!text(TO_BREAK_COL).isEmpty()) {
    try {
      otl_stream str(1,
		     SQLClearBreakpoint(Connection),
		     Connection.connection());
      str<<text(TO_BREAK_COL).toInt();
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
  setText(4,"DISABLED");
}

#define DEBUG_INDENT 10

static QPixmap *toBreakpointPixmap;
static QPixmap *toDisBreakpointPixmap;

static toSQL SQLReadSource("toDebug:ReadSource",
			   "SELECT Text FROM All_Source\n"
			   " WHERE OWNER = :f1<char[31]>\n"
			   "   AND NAME = :f2<char[31]>\n"
			   "   AND TYPE = :f3<char[31]>\n"
			   " ORDER BY Type,Line",
			   "Read sourcecode for object");
static toSQL SQLReadErrors("toDebug:ReadErrors",
			   "SELECT Line-1,Text FROM All_Errors\n"
			   " WHERE OWNER = :f1<char[31]>\n"
			   "   AND NAME = :f2<char[31]>\n"
			   "   AND TYPE = :f3<char[31]>\n"
			   " ORDER BY Type,Line",
			   "Get lines with errors in object (Observe first line 0)");

bool toDebugText::readData(toConnection &conn,QListView *Stack)
{
  QListViewItem *item=NULL;
  if (Stack&&Stack->firstChild())
    for(item=Stack->firstChild();item->firstChild();item=item->firstChild())
      ;
  try {
    otl_stream lines(1,
		     SQLReadSource(Connection),
		     conn.connection());
    otl_stream errors(1,
		      SQLReadErrors(Connection),
		      conn.connection());

    map<int,QString> Errors;

    lines<<Schema.utf8();
    lines<<Object.utf8();
    lines<<Type.utf8();
    QString str;
    while(!lines.eof()) {
      char buffer[4001];
      lines>>buffer;
      str+=QString::fromUtf8(buffer);
    }
    setText(str);
    setEdited(false);
    setCurrent(-1);
    if (str.isEmpty())
      return false;
    else {
      errors<<Schema.utf8();
      errors<<Object.utf8();
      errors<<Type.utf8();
      while(!errors.eof()) {
	char buffer[4001];
	int line;
	errors>>line;
	errors>>buffer;
	Errors[line]+=" ";
	Errors[line]+=QString::fromUtf8(buffer);
      }
      setErrors(Errors);
      if (item&&
	  Schema==item->text(2)&&
	  Object==item->text(0)&&
	  Type==item->text(3))
	setCurrent(item->text(1).toInt()-1);
      return true;
    }
  } TOCATCH
  return false;
}

void toDebugText::setData(const QString &schema,const QString &type,const QString &object)
{
  Schema=schema;
  Type=type;
  Object=object;
  FirstItem=NULL;
  NoBreakpoints=false;
  update();
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
  if (!toDisBreakpointPixmap)
    toDisBreakpointPixmap=new QPixmap((const char **)disbreakpoint_xpm);
}

bool toDebugText::checkItem(toBreakpointItem *item)
{
  if (!item)
    return false;
  if (item->text(2)==Schema&&
      item->text(3)==Type&&
      item->text(0)==Object)
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

bool toDebugText::hasBreakpoint(int row) // This has to leave CurrentItem on the breakpoint
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
  if (col==0) {
    QPalette cp=qApp->palette();
    painter->fillRect(0,0,DEBUG_INDENT-2,cellHeight(),cp.active().background());
    painter->fillRect(DEBUG_INDENT-2,0,1,cellHeight(),cp.active().midlight());
    painter->fillRect(DEBUG_INDENT-1,0,1,cellHeight(),cp.active().dark());

    if (hasBreakpoint(row)) {
      int h=max((cellHeight()-toBreakpointPixmap->height())/2,0);
      if (CurrentItem->text(4)=="DISABLED")
	painter->drawPixmap(0,h,*toDisBreakpointPixmap);
      else
	painter->drawPixmap(0,h,*toBreakpointPixmap);
    }
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
      if (xOffset()<DEBUG_INDENT-3)
	painter.fillRect(view.left(),fromY,DEBUG_INDENT-2-xOffset(),view.bottom()-fromY,cp.active().background());
      if (xOffset()<DEBUG_INDENT-2)
	painter.fillRect(view.left()+DEBUG_INDENT-2-xOffset(),fromY,1,view.bottom()-fromY,cp.active().midlight());
      if (xOffset()<DEBUG_INDENT-1)
	painter.fillRect(view.left()+DEBUG_INDENT-1-xOffset(),fromY,1,view.bottom()-fromY,cp.active().dark());
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

void toDebugText::toggleBreakpoint(int row,bool enable)
{
  if (Schema.isEmpty()||
      Type.isEmpty()||
      Object.isEmpty())
    return;

  int curcol;
  if (row<0)
    getCursorPosition (&row,&curcol);
  if (row>=0) {
    if (hasBreakpoint(row)) {
      if (enable) {
	if (CurrentItem->text(4)=="DISABLED")
	  CurrentItem->setText(4,"DEFERED");
	else
	  CurrentItem->clearBreakpoint();
      } else {
	CurrentItem->clearBreakpoint();
	delete CurrentItem;
	if (FirstItem==CurrentItem) {
	  NoBreakpoints=false;
	  CurrentItem=FirstItem=NULL;
	} else
	  CurrentItem=FirstItem;
      }
    } else if (!enable) {
      if (CurrentItem&&CurrentItem->line()>row)
	new toBreakpointItem(Breakpoints,NULL,Connection,
			     Schema,Type,Object,row);
      else
	new toBreakpointItem(Breakpoints,CurrentItem,Connection,
			     Schema,Type,Object,row);
      FirstItem=CurrentItem=NULL;
      NoBreakpoints=false;
    }
    updateCell(row,0,false);
  }
}

void toDebugText::mouseDoubleClickEvent(QMouseEvent *me)
{
  if (me->x()+xOffset()>DEBUG_INDENT)
    toHighlightedText::mouseDoubleClickEvent(me);
  else {
    int row=findRow(me->y());
    toggleBreakpoint(row);
  }
}
