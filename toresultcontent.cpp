//***************************************************************************
/*
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

#include <qdragobject.h>
#include <qclipboard.h>
#include <qpopupmenu.h>

#include "toconf.h"
#include "tomemoeditor.h"
#include "tomain.h"
#include "toresultcontent.h"
#include "toresultview.h"

#include "toresultcontent.moc"

#define INC_SIZE 50

#include <stdio.h>

void toResultContent::contentsMouseMoveEvent (QMouseEvent *e)
{
  if (e->state()==LeftButton&&
      e->stateAfter()==LeftButton&&
      LastMove.x()>0&&
      LastMove!=e->pos()) {

    QPoint p=e->pos();
    int col=columnAt(p.x());
    int row=rowAt(p.y());
    QString str=text(row,col);

    if (str.length()) {
      QDragObject *d=new QTextDrag(str,this);
      d->dragCopy();
    }
  } else {
    LastMove=e->pos();
    QTable::contentsMouseMoveEvent(e);
  }
}

void toResultContent::contentsMousePressEvent(QMouseEvent *e)
{
  LastMove=QPoint(-1,-1);
  if (e->button()==RightButton)
    displayMenu(e->globalPos());
  else
    QTable::contentsMousePressEvent(e);
}

void toResultContent::contentsMouseReleaseEvent(QMouseEvent *e)
{
  LastMove=QPoint(-1,-1);
  QTable::contentsMouseReleaseEvent(e);
}

void toResultContent::dragEnterEvent(QDragEnterEvent *e)
{
  e->accept(QTextDrag::canDecode(e));
}

void toResultContent::dropEvent(QDropEvent *e)
{
  QPoint p(e->pos().x()+contentsX()-verticalHeader()->width(),
	   e->pos().y()+contentsY()-horizontalHeader()->height());
  int col=columnAt(p.x());
  int row=rowAt(p.y());

  changePosition(col,row);

  if (CurrentRow!=row) {
    OrigValues.clear();
    for (int i=0;i<numCols();i++)
      OrigValues.insert(OrigValues.end(),text(row,i));
    CurrentRow=row;
  }
  QString text;
  if ( QTextDrag::decode(e,text)) {
    setText(row,col,text);
    setCurrentCell(row,col);
  }
}

toResultContent::toResultContent(toConnection &conn,QWidget *parent,const char *name)
  : QTable(parent,name),Connection(conn)
{
  Query=NULL;
  connect(this,SIGNAL(currentChanged(int,int)),this,SLOT(changePosition(int,int)));
  CurrentRow=-1;
  setFocusPolicy(StrongFocus);
  setSelectionMode(NoSelection);
  connect(horizontalHeader(),SIGNAL(clicked(int)),this,SLOT(changeSort(int)));
  SortRow=-1;
  setAcceptDrops(true);
  LastMove=QPoint(-1,-1);
  MenuColumn=MenuRow=-1;
  Menu=NULL;
}

void toResultContent::wrongUsage(void)
{
  throw QString("Can't use these on toResultContent");
}

void toResultContent::changeSort(int col)
{
  if (col!=SortRow) {
    SortRow=col;
    SortRowAsc=true;
  } else
    SortRowAsc=!SortRowAsc;
  sortColumn(SortRow,SortRowAsc,true);
}

void toResultContent::changeParams(const QString &Param1,const QString &Param2)
{
  Owner=Param1;
  Table=Param2;
  setNumRows(0);
  setNumCols(0);

  delete Query;
  Query=NULL;

  try {
    QString sql;
    sql="SELECT * FROM \"";
    sql+=Owner;
    sql+="\".\"";
    sql+=Table;
    sql+="\"";
    Query=new otl_stream;
    Query->set_all_column_types(otl_all_num2str|otl_all_date2str);
    Query->open(1,
		sql.utf8(),
		Connection.connection());

    int descriptionLen;
    Description=Query->describe_select(descriptionLen);

    setNumCols(descriptionLen);
    QHeader *head=horizontalHeader();
    for (int i=0;i<descriptionLen;i++) {
      QString name(QString::fromUtf8(Description[i].name));
      head->setLabel(i,name);
    }
    int MaxNumber=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER).toInt();
    Row=0;
    setNumRows(INC_SIZE);
    for (int j=0;j<MaxNumber&&!Query->eof();j++) {
      if (Row+2>=numRows())
	setNumRows(numRows()+INC_SIZE);
      addRow();
    }
    if (MaxNumber<0)
      readAll();
    setNumRows(Row+1);
  } TOCATCH
  OrigValues.clear();
  CurrentRow=-1;
}

void toResultContent::addRow(void)
{
  AddRow=false;
  int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();
  try {
    if (Query&&!Query->eof()) {
      if (Row+1>=numRows())
	setNumRows(Row+3);
      verticalHeader()->setLabel(Row,QString::number(Row+1));
      for (int j=0;j<numCols()&&!Query->eof();j++)
	setText(Row,j,toReadValue(Description[j],*Query,MaxColSize));
      Row++;
    }
  } TOCATCH
}

void toResultContent::keyPressEvent(QKeyEvent *e)
{
  if (e->key()==Key_PageDown) {
    int height=verticalHeader()->sectionSize(0);
    if (!Query->eof()&&height>0) {
      int num=visibleHeight()/height;
      setNumRows(Row+num+1);
      for (int i=0;i<num&&!Query->eof();i++) {
	if (Row+2>=numRows())
	  setNumRows(numRows()+INC_SIZE);
	addRow();
      }
      if (numRows()!=Row+1)
	setNumRows(Row+1);
    }
  }
  QTable::keyPressEvent(e);
}

void toResultContent::paintCell(QPainter *p,int row,int col,const QRect &cr,bool selected)
{
  if (row+1>=Row)
    AddRow=true;
  QTable::paintCell(p,row,col,cr,selected);
}

QWidget *toResultContent::beginEdit(int row,int col,bool replace)
{
  if (CurrentRow!=row) {
    OrigValues.clear();
    for (int i=0;i<numCols();i++)
      OrigValues.insert(OrigValues.end(),text(row,i));
    CurrentRow=row;
    toStatusMessage("Unsaved data in contents, select other row to store",true);
  }

  return QTable::beginEdit(row,col,replace);
}

static bool nullString(const QString &str)
{
  return str=="{null}"||str.isNull();
}

#include <stdio.h>

void toResultContent::changePosition(int row,int col)
{
  if (CurrentRow!=row&&OrigValues.size()>0) {
    toStatusMessage("Saved row");
    if (CurrentRow>=Row) {
      QString sql="INSERT INTO \"";
      sql+=Owner;
      sql+="\".\"";
      sql+=Table;
      sql+="\" VALUES (";
      for (int i=0;i<numCols();i++) {
	sql+=":f";
	sql+=QString::number(i);
	sql+="<char[4000]>";
	if (i+1<numCols())
	  sql+=",";
      }
      sql+=")";
      try {
	otl_stream exec(1,
			sql.utf8(),
			Connection.connection());
	otl_null null;
	for (int i=0;i<numCols();i++) {
	  QString str=text(CurrentRow,i);
	  otl_null null;
	  if (nullString(str))
	    exec<<null;
	  else
	    exec<<str.utf8();
	}
	Row++;
	setNumRows(Row+1);
	Connection.setNeedCommit();
      } TOCATCH
    } else {
      QString sql="UPDATE \"";
      sql+=Owner;
      sql+="\".\"";
      sql+=Table;
      sql+="\" SET ";
      QHeader *head=horizontalHeader();
      list<QString>::iterator k=OrigValues.begin();
      bool first=false;
      for(int i=0;i<numCols();i++,k++) {
	if (*k!=text(CurrentRow,i)) {
	  if (!first)
	    first=true;
	  else
	    sql+=", ";
	  sql+="\"";
	  sql+=head->label(i);
	  sql+="\" ";
	  if (nullString(text(CurrentRow,i)))
	    sql+=" = NULL";
	  else {
	    sql+="= :f";
	    sql+=QString::number(i);
	    sql+="<char[4000]>";
	  }
	}
      }
      if (first) {
	sql+=" WHERE ";
	int col=0;
	for(list<QString>::iterator j=OrigValues.begin();j!=OrigValues.end();j++,col++) {
	  sql+="\"";
	  sql+=head->label(col);
	  sql+="\" ";
	  if (nullString(*j))
	    sql+=" IS NULL";
	  else {
	    sql+="= :c";
	    sql+=QString::number(col);
	    sql+="<char[4000]>";
	  }
	  if (col+1<numCols())
	    sql+=" AND ";
	}
	try {
	  otl_stream exec(1,
			  sql.utf8(),
			  Connection.connection());
      
	  otl_null null;
	  list<QString>::iterator k=OrigValues.begin();
	  for (int i=0;i<numCols();i++,k++) {
	    QString str=text(CurrentRow,i);
	    if (str!=*k&&!nullString(str))
	      exec<<str.utf8();
	  }
	  for(list<QString>::iterator j=OrigValues.begin();j!=OrigValues.end();j++,col++) {
	    QString str=(*j);
	    if (!nullString(str))
	      exec<<str.utf8();
	  }
	  Connection.setNeedCommit();
	} catch (const otl_exception &exc) {
	  int col=0;
	  for(list<QString>::iterator j=OrigValues.begin();j!=OrigValues.end();j++,col++)
	    setText(CurrentRow,col,*j);
	  toStatusMessage(QString::fromUtf8((const char *)exc.msg));
	} catch (const QString &str) {
	  toStatusMessage(str);
	}
      }
    }
    OrigValues.clear();
    CurrentRow=-1;
  }
}

void toResultContent::drawContents(QPainter * p,int cx,int cy,int cw,int ch)
{
  QTable::drawContents(p,cx,cy,cw,ch);
  if (AddRow)
    addRow();
}

void toResultContent::readAll(void)
{
  while (Query&&!Query->eof()) {
    if (Row+2>=numRows())
      setNumRows(numRows()+INC_SIZE);
    addRow();
  }
  setNumRows(Row+1);
}

void toResultContent::print(void)
{
  toResultView print(false,true,Connection,this);
  print.hide();
  QString name="Content of ";
  name+=Owner;
  name+=".";
  name+=Table;
  print.setSQLName(name);
  QString sql="SELECT * FROM \"";
  sql+=Owner;
  sql+="\".\"";
  sql+=Table;
  sql+="\"";
  print.query(sql);
  print.print();
}

void toResultContent::exportFile(void)
{
  toResultView list(false,true,Connection,this);
  list.hide();
  QString name="Content of ";
  name+=Owner;
  name+=".";
  name+=Table;
  list.setSQLName(name);
  QString sql="SELECT * FROM \"";
  sql+=Owner;
  sql+="\".\"";
  sql+=Table;
  sql+="\"";
  list.query(sql);
  list.readAll();
  list.exportFile();
}

void toResultContent::activateNextCell()
{
  if (currentColumn()+1<numCols())
    setCurrentCell(currentRow(),currentColumn()+1);
  else {
    if (currentRow()+1>=numRows())
      setNumRows(Row+2);
    setCurrentCell(currentRow()+1,0);
  }
}

void toResultContent::focusInEvent (QFocusEvent *e)
{
  toMain::editEnable(false,false,true,
		     false,false,
		     false,false,false,true);
  QTable::focusInEvent(e);
}

void toResultContent::focusOutEvent (QFocusEvent *e)
{
  toMain::editDisable();
  QTable::focusOutEvent(e);
}

#define TORESULT_COPY     1
#define TORESULT_PASTE    2
#define TORESULT_MEMO     3
#define TORESULT_READ_ALL 4
#define TORESULT_EXPORT   5

void toResultContent::displayMenu(const QPoint &p)
{
  QPoint lp=mapFromGlobal(p);
  lp=QPoint(lp.x()+contentsX()-verticalHeader()->width(),
	    lp.y()+contentsY()-horizontalHeader()->height());
  MenuColumn=columnAt(lp.x());
  MenuRow=rowAt(lp.y());
  if (MenuColumn>=0&&MenuRow>=0) {
    if (!Menu) {
      Menu=new QPopupMenu(this);
      Menu->insertItem("&Copy",TORESULT_COPY);
      Menu->insertItem("&Paste",TORESULT_PASTE);
      Menu->insertItem("&Display in editor",TORESULT_MEMO);
      Menu->insertItem("Export to file",TORESULT_EXPORT);
      Menu->insertSeparator();
      Menu->insertItem("Read all",TORESULT_READ_ALL);
      connect(Menu,SIGNAL(activated(int)),this,SLOT(menuCallback(int)));
    }
    setCurrentCell(MenuRow,MenuColumn);
    Menu->popup(p);
    QClipboard *clip=qApp->clipboard();
    Menu->setItemEnabled(TORESULT_PASTE,!clip->text().isEmpty());
  }
}

void toResultContent::displayMemo(void)
{
  toMemoEditor *edit=new toMemoEditor(this,text(MenuRow,MenuColumn),MenuRow,MenuColumn);
  connect(edit,SIGNAL(changeData(int,int,const QString &)),
	  this,SLOT(changeData(int,int,const QString &)));
}

void toResultContent::changeData(int row,int col,const QString &str)
{
  changePosition(col,row);
  if (CurrentRow!=row) {
    OrigValues.clear();
    for (int i=0;i<numCols();i++)
      OrigValues.insert(OrigValues.end(),text(MenuRow,i));
    CurrentRow=row;
  }
  setText(row,col,str);
  setCurrentCell(row,col);
}

void toResultContent::menuCallback(int cmd)
{
  switch(cmd) {
  case TORESULT_COPY:
    {
      QClipboard *clip=qApp->clipboard();
      clip->setText(text(MenuRow,MenuColumn));
    }
    break;
  case TORESULT_PASTE:
    {
      QClipboard *clip=qApp->clipboard();
      if (CurrentRow!=MenuRow) {
	OrigValues.clear();
	for (int i=0;i<numCols();i++)
	  OrigValues.insert(OrigValues.end(),text(MenuRow,i));
	CurrentRow=MenuRow;
      }
      setText(MenuRow,MenuColumn,clip->text());
    }
    break;
  case TORESULT_MEMO:
    displayMemo();
    break;
  case TORESULT_READ_ALL:
    readAll();
    break;
  case TORESULT_EXPORT:
    exportFile();
    break;
  }
}
