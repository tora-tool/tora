//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include <qdragobject.h>
#include <qclipboard.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qmessagebox.h>

#include "toconf.h"
#include "tomemoeditor.h"
#include "tomain.h"
#include "toresultcontent.h"
#include "toresultview.h"
#include "totool.h"
#include "tohighlightedtext.h"
#include "toresultcontentfilterui.h"
#include "toresultcols.h"
#include "toconnection.h"
#include "tosearchreplace.h"
#include "tonoblockquery.h"

#include "toresultcontent.moc"
#include "toresultcontentfilterui.moc"

#include "icons/filter.xpm"
#include "icons/nofilter.xpm"
#include "icons/addrecord.xpm"
#include "icons/saverecord.xpm"
#include "icons/canceledit.xpm"
#include "icons/trash.xpm"

#include "icons/forward.xpm"
#include "icons/next.xpm"
#include "icons/previous.xpm"
#include "icons/rewind.xpm"

#define INC_SIZE 50

void toResultContentEditor::editSearch(toSearchReplace *search)
{
  search->setTarget(this);
}

void toResultContentEditor::setCurrentCellFocus(int row, int col)
{
  QTable::setCurrentCell(row,col);
  viewport()->setFocus();
}

void toResultContentEditor::contentsMouseMoveEvent (QMouseEvent *e)
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

void toResultContentEditor::contentsMousePressEvent(QMouseEvent *e)
{
  LastMove=QPoint(-1,-1);
  if (e->button()==RightButton)
    displayMenu(e->globalPos());
  else
    QTable::contentsMousePressEvent(e);
  viewport()->setFocus();
}

void toResultContentEditor::contentsMouseReleaseEvent(QMouseEvent *e)
{
  LastMove=QPoint(-1,-1);
  QTable::contentsMouseReleaseEvent(e);
}

void toResultContentEditor::dragEnterEvent(QDragEnterEvent *e)
{
  e->accept(QTextDrag::canDecode(e));
}

void toResultContentEditor::dropEvent(QDropEvent *e)
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

toResultContentEditor::toResultContentEditor(QWidget *parent,const char *name)
  : QTable(parent,name),
    toEditWidget(false,true,true,
		 false,false,
		 false,false,false,
		 true,false,true)
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

  QString str=toTool::globalConfig(CONF_LIST,"");
  if (!str.isEmpty()) {
    QFont font(toStringToFont(str));
    setFont(font);
  }

  installEventFilter(this);

  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
}

toResultContentEditor::~toResultContentEditor()
{
  delete Query;
}

void toResultContentEditor::wrongUsage(void)
{
  throw QString("Can't use these on toResultContent");
}

void toResultContentEditor::changeSort(int col)
{
  if (col!=SortRow) {
    SortRow=col;
    SortRowAsc=true;
  } else
    SortRowAsc=!SortRowAsc;
  sortColumn(SortRow,SortRowAsc,true);
}

void toResultContentEditor::changeParams(const QString &Param1,const QString &Param2)
{
  if (Param1==Owner&&Param2==Table&&!toTool::globalConfig(CONF_DONT_REREAD,"").isEmpty())
    return;

  Owner=Param1;
  Table=Param2;
  if (AllFilter)
    FilterName="";
  else {
    FilterName=Owner;
    FilterName+=".";
    FilterName+=Table;
  }

  setNumRows(0);
  setNumCols(0);
  NewRecordRow = -1;
  MaxNumber=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER).toInt();

  delete Query;
  Query=NULL;

  try {
    QString sql;
    sql="SELECT * FROM ";
    sql+=table();
    if (!Criteria[FilterName].isEmpty()) {
      sql+=" WHERE ";
      sql+=Criteria[FilterName];
    }
    if (!Order[FilterName].isEmpty()) {
      sql+=" ORDER BY ";
      sql+=Order[FilterName];
    }
    toQList par;
    Query=new toNoBlockQuery(connection(),toQuery::Background,sql,par);
    Poll.start(100);
    OrigValues.clear();
    CurrentRow=-1;
  } TOCATCH
}

void toResultContentEditor::poll(void)
{
  try {
    if (Query&&Query->poll()) {
      if (numRows()==0) {
	toQDescList desc=Query->describe();
	setNumCols(desc.size());

	QHeader *head=horizontalHeader();
	int col=0;
	for (toQDescList::iterator i=desc.begin();i!=desc.end();i++) {
	  head->setLabel(col,(*i).Name);
	  col++;
	}
	Row=0;
      }
      
      std::list<QString> data;

      for (int j=Row;(j<MaxNumber||MaxNumber<0)&&Query->poll()&&!Query->eof();j++)
	for (int k=0;k<numCols();k++)
	  data.insert(data.end(),Query->readValueNull());

      setNumRows(Row+data.size()/numCols()+1);

      while(data.size()>0) {
	verticalHeader()->setLabel(Row,QString::number(Row+1));
	for(int j=0;j<numCols();j++)
	  setText(Row,j,toShift(data));
	Row++;
      }

      if (Query->eof()) {
	delete Query;
	Query=NULL;
	Poll.stop();
      } else if (Row>=MaxNumber&&MaxNumber>=0)
	Poll.stop();
    }
  } catch(const QString &str) {
    delete Query;
    Query=NULL;
    Poll.stop();
    toStatusMessage(str);
  }
}

bool toResultContentEditor::eventFilter(QObject *o,QEvent *e)
{
  if (e&&o&&e->type()==QEvent::KeyPress) {
    QKeyEvent *ke = (QKeyEvent*)e;
    if ((ke->key()==Key_Tab&&ke->state()==0)||
	(ke->key()==Key_Backtab&&ke->state()==ShiftButton)) {
      keyPressEvent(ke);
      return true;
    }
  }
  return QTable::eventFilter(o,e);
}

void toResultContentEditor::keyPressEvent(QKeyEvent *e)
{
  if (e->key()==Key_PageDown) {
    if (Query&&!Query->eof()) {
      Poll.start(100);
      if (MaxNumber>=0) {
        int height=verticalHeader()->sectionSize(0);
	MaxNumber+=max(height+1,20);
      } else
	MaxNumber+=20;
    }
  } else if (e->key()==Key_Return) {
    if (e->state()==0) {
      gotoNextRecord();
      e->accept();
      return;
    } else if (e->state()==ShiftButton) {
      gotoPreviousRecord();
      e->accept();
      return;
    }
  } else if (e->key()==Key_Tab&&e->state()==0) {
    if (currentColumn()<numCols()-1)
      setCurrentCellFocus(currentRow(),currentColumn()+1);
    else if (currentRow()<numRows()-1)
      setCurrentCellFocus(currentRow()+1,0);
    e->accept();
    return;
  } else if (e->key()==Key_Backtab&&e->state()==ShiftButton) {
    if (currentColumn()>0)
      setCurrentCellFocus(currentRow(),currentColumn()-1);
    else if (currentRow()>0)
      setCurrentCellFocus(currentRow()-1,numCols()-1);
    e->accept();
    return;
  }
  QTable::keyPressEvent(e);
}

void toResultContentEditor::paintCell(QPainter *p,int row,int col,const QRect &cr,bool selected)
{
  if (row+1>=MaxNumber) {
    if (Query&&!Query->eof()) {
      Poll.start(100);
      if (MaxNumber>=0)
	MaxNumber+=5;
    }
  }
  QTable::paintCell(p,row,col,cr,selected);
}

QWidget *toResultContentEditor::beginEdit(int row,int col,bool replace)
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

void toResultContentEditor::gotoLastRecord()
{
  editReadAll();
  setNumRows(Row+1);
  setCurrentCellFocus(Row-1,currentColumn());
}
  
void toResultContentEditor::gotoFirstRecord()
{
  setCurrentCellFocus(0,currentColumn());
}
  
void toResultContentEditor::gotoPreviousRecord()
{
  setCurrentCellFocus(max(0,currentRow()-1),currentColumn());
}
  
void toResultContentEditor::gotoNextRecord()
{
  setCurrentCellFocus(min(numRows()-1,currentRow()+1),currentColumn());
}
  
void toResultContentEditor::addRecord()
{
  if(CurrentRow>0 || NewRecordRow>0) {
    return;
  }

  saveUnsaved();
  
  if(currentRow() > numRows()-2) {
    setCurrentCellFocus(numRows()-1,0);
  } else {
    setNumRows(numRows()+1);
    int crow=currentRow();
    if(crow < 0 || crow > numRows()-2)
      crow = 0;
    for (int row=numRows()-1;row>crow;row--)
      swapRows(row,row-1);

    for (int i=0;i<numCols();i++)
      setText(crow,i,"");

    NewRecordRow = crow;
    setNumRows(numRows());
    setCurrentCellFocus(crow,0);
  }
}

void toResultContentEditor::cancelEdit()
{

  if(CurrentRow < 0 && NewRecordRow < 0) {
    setCurrentCellFocus(currentRow(),currentColumn());
    return;
  }

  int crow=CurrentRow;
  endEdit(currentRow(),currentColumn(),false,false);

  if(NewRecordRow > -1) {
    crow=NewRecordRow;
    NewRecordRow = -1;
    for (int row=crow+1;row<numRows();row++)
      swapRows(row-1,row);
  } else {
    int col=0;
    for(std::list<QString>::iterator j=OrigValues.begin();j!=OrigValues.end();j++,col++)
      setText(CurrentRow,col,*j);
  }
  CurrentRow=-1;
  OrigValues.clear();
    
  setNumRows(Row+1);
  setCurrentCellFocus(crow,0);
  toStatusMessage("Edit cancelled",false,false);
}

void toResultContentEditor::deleteCurrent()
{
  if(currentRow()==NewRecordRow) {
    cancelEdit();
    return;
  }
  bool mysql=(connection().provider()=="MySQL");
  saveUnsaved();
  if (currentRow()<Row) {
    QString sql="DELETE FROM ";
    sql+=table();
    sql+=" WHERE ";
    
    QHeader *head=horizontalHeader();
    for(int i=0;i<numCols();i++) {
      if (!mysql)
	sql+="\"";
      sql+=head->label(i);
      if (!mysql)
	sql+="\" ";
      if (!text(currentRow(),i))
	sql+=" IS NULL";
      else {
	sql+="= :c";
	sql+=QString::number(i);
	sql+="<char[4000]>";
      }
      if (i+1<numCols())
	sql+=" AND ";
    }
    try {
      toConnection &conn=connection();
      
      toQList args;
      for(int i=0;i<numCols();i++) {
	QString str=text(currentRow(),i);
	if (!str.isNull())
	  toPush(args,toQValue(str));
      }
      conn.execute(sql,args);
      if (!toTool::globalConfig(CONF_AUTO_COMMIT,"").isEmpty())
	conn.commit();
      else
	toMainWidget()->setNeedCommit(conn);
    } TOCATCH
  }
  int crow=currentRow();
  for (int row=crow+1;row<numRows();row++)
    swapRows(row-1,row);
  if (crow<Row)
    Row--;
  else
    setNumRows(Row);

  setNumRows(Row+1);
  setCurrentCellFocus(crow,0);
}

void toResultContentEditor::saveUnsaved()
{
  endEdit(currentRow(),currentColumn(),true,true);
  if (OrigValues.size()>0) {
    QString rowid = "";
    bool mysql=(connection().provider()=="MySQL");
    bool oracle=(connection().provider()=="Oracle");
    toStatusMessage("Saved row",false,false);
    if (CurrentRow>=Row || CurrentRow==NewRecordRow) {
      QString sql="INSERT INTO ";
      sql+=table();
      sql+=" VALUES (";
      for (int i=0;i<numCols();i++) {
	sql+=":f";
	sql+=QString::number(i);
	sql+="<char[4000],in>";
	if (i+1<numCols())
	  sql+=",";
      }
      sql+=")";
      if(oracle) 
	sql+=" RETURNING ROWID INTO :r<char[32],out>";
	
      try {
	toConnection &conn = connection();
	toQList args;
	toQValue null;
	for (int i=0;i<numCols();i++) {
	  QString str=text(CurrentRow,i);
	  if (str.isNull())
	    toPush(args,null);
	  else
	    toPush(args,toQValue(str));
	}
	toQuery q(conn,sql,args);
	if(oracle)
	  rowid = q.readValueNull();
	Row++;
	setNumRows(Row+1);
	if (!toTool::globalConfig(CONF_AUTO_COMMIT,"").isEmpty())
	  conn.commit();
	else
	  toMainWidget()->setNeedCommit(conn);
      } catch (const QString &str) {
	cancelEdit();
	toStatusMessage(str);
	oracle = false;
      }
    } else {
      QString sql="UPDATE ";
      sql+=table();
      sql+=" SET ";
      QHeader *head=horizontalHeader();
      std::list<QString>::iterator k=OrigValues.begin();
      bool first=false;
      for(int i=0;i<numCols();i++,k++) {
	if (*k!=text(CurrentRow,i)) {
	  if (!first)
	    first=true;
	  else
	    sql+=", ";
	  if (!mysql)
	    sql+="\"";
	  sql+=head->label(i);
	  if (!mysql)
	    sql+="\" ";
	  if (text(CurrentRow,i).isNull())
	    sql+=" = NULL";
	  else {
	    sql+="= :f";
	    sql+=QString::number(i);
	    sql+="<char[4000],in>";
	  }
	}
      }
      if (first) {
	sql+=" WHERE ";
	int col=0;
	for(std::list<QString>::iterator j=OrigValues.begin();j!=OrigValues.end();j++,col++) {
	  if (!mysql)
	    sql+="\"";
	  sql+=head->label(col);
	  if (!mysql)
	    sql+="\" ";
	  if ((*j).isNull())
	    sql+=" IS NULL";
	  else {
	    sql+="= :c";
	    sql+=QString::number(col);
	    sql+="<char[4000],in>";
	  }
	  if (col+1<numCols())
	    sql+=" AND ";
	}
	if(oracle)
	  sql+=" RETURNING ROWID INTO :r<char[32],out>";
	try {
	  toConnection &conn = connection();
	  toQList args;

	  std::list<QString>::iterator k=OrigValues.begin();
	  for (int i=0;i<numCols();i++,k++) {
	    QString str=text(CurrentRow,i);
	    if (str!=*k&&!str.isNull())
	      toPush(args,toQValue(str));
	  }
	  for(std::list<QString>::iterator j=OrigValues.begin();j!=OrigValues.end();j++,col++) {
	    QString str=(*j);
	    if (!str.isNull())
	      toPush(args,toQValue(str));
	  }
	  toQuery q(conn,sql,args);
	  if(oracle)
	    rowid = q.readValueNull();
	  if (!toTool::globalConfig(CONF_AUTO_COMMIT,"").isEmpty())
	    conn.commit();
	  else
	    toMainWidget()->setNeedCommit(conn);
	} catch (const QString &str) {
	  int col=0;
	  for(std::list<QString>::iterator j=OrigValues.begin();j!=OrigValues.end();j++,col++)
	    setText(CurrentRow,col,*j);
	  toStatusMessage(str);
	  oracle = false;
	}
      }
    }
    OrigValues.clear();
    if(oracle) {
      try {
	QString sql;
	sql="SELECT * FROM ";
	sql+=table();
	sql+=" WHERE rowid = :r<char[32]>";
	toQuery q(connection(),sql,rowid);
	for (int j=0;j<numCols()&&!q.eof();j++)
	  setText(CurrentRow,j,q.readValueNull());
      } TOCATCH
    }
    CurrentRow=-1;
    NewRecordRow=-1;
  }
}

void toResultContentEditor::changePosition(int row,int col)
{
  if (CurrentRow!=row)
    saveUnsaved();
  if (NewRecordRow>0 && NewRecordRow!=row) {
    cancelEdit();
    setCurrentCell(row,col);
  }
}

void toResultContentEditor::drawContents(QPainter * p,int cx,int cy,int cw,int ch)
{
  QTable::drawContents(p,cx,cy,cw,ch);
}

void toResultContentEditor::editReadAll(void)
{
  if (Query) {
    MaxNumber=-1;
    Poll.start(100);
  }
}

void toResultContentEditor::editPrint(void)
{
  toResultView print(false,true,this);
  print.hide();
  QString name="Content of ";
  name+=Owner;
  name+=".";
  name+=Table;
  print.setSQLName(name);
  QString sql="SELECT * FROM ";
  sql+=table();
  print.query(sql);
  print.editPrint();
}

void toResultContentEditor::editSave(bool ask)
{
  toResultView list(false,true,this);
  list.hide();
  QString name="Content of ";
  name+=Owner;
  name+=".";
  name+=Table;
  list.setSQLName(name);
  QString sql="SELECT * FROM ";
  sql+=table();
  list.query(sql);
  list.editReadAll();
  list.editSave(ask);
}

void toResultContentEditor::activateNextCell()
{
  if (currentColumn()+1<numCols())
    setCurrentCell(currentRow(),currentColumn()+1);
  else {
    if (currentRow()+1>=numRows())
      setNumRows(Row+2);
    setCurrentCell(currentRow()+1,0);
  }
}

void toResultContentEditor::focusInEvent (QFocusEvent *e)
{
  receivedFocus();
  QTable::focusInEvent(e);
}

#define TORESULT_COPY     1
#define TORESULT_PASTE    2
#define TORESULT_MEMO     3
#define TORESULT_READ_ALL 4
#define TORESULT_EXPORT   5
#define TORESULT_DELETE	  6

void toResultContentEditor::displayMenu(const QPoint &p)
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
      Menu->insertSeparator();
      Menu->insertItem("&Delete record",TORESULT_DELETE);
      Menu->insertSeparator();
      Menu->insertItem("Export to file",TORESULT_EXPORT);
      Menu->insertItem("Read all",TORESULT_READ_ALL);
      connect(Menu,SIGNAL(activated(int)),this,SLOT(menuCallback(int)));
    }
    setCurrentCell(MenuRow,MenuColumn);
    Menu->popup(p);
    QClipboard *clip=qApp->clipboard();
    Menu->setItemEnabled(TORESULT_PASTE,!clip->text().isEmpty());
  }
}

void toResultContentEditor::displayMemo(void)
{
  toMemoEditor *edit=new toMemoEditor(this,text(MenuRow,MenuColumn),MenuRow,MenuColumn);
  connect(edit,SIGNAL(changeData(int,int,const QString &)),
	  this,SLOT(changeData(int,int,const QString &)));
}

void toResultContentEditor::changeData(int row,int col,const QString &str)
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

void toResultContentEditor::menuCallback(int cmd)
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
  case TORESULT_DELETE:
    setCurrentCell(MenuRow,0);
    deleteCurrent();
    break;
  case TORESULT_MEMO:
    displayMemo();
    break;
  case TORESULT_READ_ALL:
    editReadAll();
    break;
  case TORESULT_EXPORT:
    editSave(false);
    break;
  }
}

QString toResultContentEditor::table(void)
{
  QString sql;
  if (connection().provider()=="MySQL")
    return Table;
  else {
    sql="\"";
    sql+=Owner;
    sql+="\".\"";
    sql+=Table;
    sql+="\"";
    return sql;
  }
}

toResultContent::toResultContent(QWidget *parent,const char *name)
  : QVBox(parent,name)
{
  QToolBar *toolbar=toAllocBar(this,"Content editor",connection().description());
  Editor=new toResultContentEditor(this,name);

  new QToolButton(QPixmap((const char **)filter_xpm),
		  "Define filter for editor",
		  "Define filter for editor",
		  this,SLOT(changeFilter()),toolbar);
  new QToolButton(QPixmap((const char **)nofilter_xpm),
		  "Remove any filter",
		  "Remove any filter",
		  this,SLOT(removeFilter()),toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)addrecord_xpm),
		  "Add a new record",
		  "Add a new record",
		  Editor,SLOT(addRecord()),toolbar);
  new QToolButton(QPixmap((const char **)saverecord_xpm),
		  "Save changes",
		  "Save changes",
		  Editor,SLOT(saveUnsaved()),toolbar);
  new QToolButton(QPixmap((const char **)canceledit_xpm),
		  "Discard changes",
		  "Discard changes",
		  Editor,SLOT(cancelEdit()),toolbar);
  new QToolButton(QPixmap((const char **)trash_xpm),
		  "Delete current record from table",
		  "Delete current record from table",
		  Editor,SLOT(deleteCurrent()),toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)rewind_xpm),
		  "Go to first row",
		  "Go to first row",
		  Editor,SLOT(gotoFirstRecord()),toolbar);
  new QToolButton(QPixmap((const char **)previous_xpm),
		  "Go to previous row",
		  "Go to previous row",
		  Editor,SLOT(gotoPreviousRecord()),toolbar);
  new QToolButton(QPixmap((const char **)next_xpm),
		  "Go to next row",
		  "Go to next row",
		  Editor,SLOT(gotoNextRecord()),toolbar);
  new QToolButton(QPixmap((const char **)forward_xpm),
		  "Go to last row",
		  "Go to last row",
		  Editor,SLOT(gotoLastRecord()),toolbar);
  toolbar->setStretchableWidget(new QLabel(toolbar));
  connect(toMainWidget(),SIGNAL(willCommit(toConnection &,bool)),
	  this,SLOT(saveUnsaved(toConnection &,bool)));
}

void toResultContent::changeFilter(void)
{
  toResultContentFilterUI filter(this,"FilterSetup",true);
  filter.AllTables->setChecked(Editor->allFilter());
  filter.Order->setText(Editor->Order[Editor->FilterName]);
  filter.Criteria->setText(Editor->Criteria[Editor->FilterName]);
  filter.Columns->changeParams(Editor->Owner,Editor->Table);
  if(filter.exec())
    Editor->changeFilter(filter.AllTables->isChecked(),
			 filter.Criteria->text(),
			 filter.Order->text());
}

void toResultContentEditor::changeFilter(bool all,const QString &crit,const QString &ord)
{
  AllFilter=all;
  QString nam;
  if (AllFilter)
    nam="";
  else {
    nam=Owner;
    nam+=".";
    nam+=Table;
  }
  Criteria[nam]=crit;
  Order[nam]=ord;
  saveUnsaved();
  changeParams(Owner,Table);
}

void toResultContent::saveUnsaved(toConnection &conn,bool cmt)
{
  toConnection &mycon=connection();
  if (&mycon==&conn) // Is this same connection
    saveUnsaved();
}

bool toResultContent::canHandle(toConnection &conn)
{
  return conn.provider()=="Oracle"||conn.provider()=="MySQL";
}

void toResultContent::removeFilter(void)
{
  if (!Editor->allFilter()) {
    switch(TOMessageBox::information(this,"Remove filter",
				     "Remove the filter for this table only or for all tables.",
				     "&All","&This","&Cancel",0)) {
    case 0:
      Editor->Criteria.clear();
      Editor->Order.clear();
      // Intentionally no break
    case 1:
      Editor->changeFilter(false,QString::null,QString::null);
      break;
    case 2:
      return;
    }
  } else
    Editor->changeFilter(Editor->allFilter(),QString::null,QString::null);
}
