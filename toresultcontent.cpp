//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003 Quest Software, Inc
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "tonoblockquery.h"
#include "toparamget.h"
#include "toresultcols.h"
#include "toresultcontent.h"
#include "toresultcontentfilterui.h"
#include "toresultview.h"
#include "tosearchreplace.h"
#include "totool.h"

#include <qcheckbox.h>
#include <qclipboard.h>
#include <qdragobject.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <stdio.h>

#include "toresultcontent.moc"
#include "toresultcontentfilterui.moc"

#include "icons/addrecord.xpm"
#include "icons/canceledit.xpm"
#include "icons/duplicaterecord.xpm"
#include "icons/filter.xpm"
#include "icons/forward.xpm"
#include "icons/next.xpm"
#include "icons/nofilter.xpm"
#include "icons/previous.xpm"
#include "icons/rewind.xpm"
#include "icons/saverecord.xpm"
#include "icons/single.xpm"
#include "icons/trash.xpm"

std::map<QCString,QString> toResultContentEditor::Criteria;
std::map<QCString,QString> toResultContentEditor::Order;

toResultContentEditor *toResultContentMemo::contentEditor()
{
  return dynamic_cast<toResultContentEditor *>(parent());
}

toResultContentMemo::toResultContentMemo(QWidget *parent,const QString &data,int row,int col,
					 bool sql)
  : toMemoEditor(parent,data,row,col,sql,false,true)
{
  toResultContentEditor *cnt=contentEditor();
  if (cnt)
    label()->setText("<B>"+cnt->horizontalHeader()->label(col)+"</B>");
  connect(parent,SIGNAL(currentChanged(int,int)),this,SLOT(changePosition(int,int)));
}

void toResultContentMemo::changePosition(int row,int cols)
{
  toMemoEditor::changePosition(row,cols);
  toResultContentEditor *cnt=contentEditor();
  if (cnt) {
    cnt->setCurrentCell(row,cols); // Nasty workaround but it'll work I think.
    setText(cnt->text(row,cols));
    label()->setText("<B>"+cnt->horizontalHeader()->label(cols)+"</B>");
  }
}

void toResultContentMemo::firstColumn()
{
  toResultContentEditor *cnt=contentEditor();
  if (cnt) {
    int col=cnt->currentColumn();
    if (col==0)
      cnt->setCurrentCell(max(cnt->currentRow()-1,0),0);
    else
      cnt->setCurrentCell(cnt->currentRow(),0);
  }
}

void toResultContentMemo::lastColumn()
{
  toResultContentEditor *cnt=contentEditor();
  if (cnt) {
    int col=cnt->currentColumn();
    int maxCol=cnt->numCols()-1;
    if (col==maxCol)
      cnt->setCurrentCell(min(cnt->numRows()-1,cnt->currentRow()+1),maxCol);
    else
      cnt->setCurrentCell(cnt->currentRow(),maxCol);
  }
}

void toResultContentMemo::previousColumn()
{
  toResultContentEditor *cnt=contentEditor();
  if (cnt) {
    int col=cnt->currentColumn();
    if (col==0)
      cnt->setCurrentCell(max(cnt->currentRow()-1,0),cnt->numCols()-1);
    else
      cnt->setCurrentCell(cnt->currentRow(),col-1);
  }
}

void toResultContentMemo::nextColumn()
{
  toResultContentEditor *cnt=contentEditor();
  if (cnt) {
    int col=cnt->currentColumn();
    int maxCol=cnt->numCols()-1;
    if (col==maxCol)
      cnt->setCurrentCell(min(cnt->numRows()-1,cnt->currentRow()+1),0);
    else
      cnt->setCurrentCell(cnt->currentRow(),col+1);
  }
}

toConnection &toResultContentEditor::connection()
{
  return toCurrentConnection(this);
}

toResultContentEditor::contentItem::contentItem(QTable *table,const QString &text)
  : QTableItem(table,OnTyping,text)
{
}

QString toResultContentEditor::contentItem::key(void) const
{
  static QRegExp number(QString::fromLatin1("^\\d*\\.?\\d+E?-?\\d*.?.?$"));

  QString txt=text();
  if (number.match(txt)>=0) {
    QString ret;
    double val=txt.toFloat();
    if(val<0)
      ret.sprintf("\x01%015.5f",val);
    else
      ret.sprintf("%015.5f",val);
    return ret;
  }
  return txt;
}

void toResultContentEditor::setText(int row,int col,const QString &text)
{
  QTableItem *itm=item(row,col);
  if (itm)
    itm->setText(text);
  else
    setItem(row,col,new contentItem(this,text));
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

  saveRow(row);
  QString text;
  if ( QTextDrag::decode(e,text)) {
    setText(row,col,text);
    setCurrentCell(row,col);
  }
}

#define TORESULT_COPY_FIELD	1
#define TORESULT_PASTE    	2
#define TORESULT_COPY_SEL	3
#define TORESULT_COPY_SEL_HEAD	4
#define TORESULT_COPY_TRANS	5
#define TORESULT_MEMO     	6
#define TORESULT_READ_ALL 	7
#define TORESULT_EXPORT   	8
#define TORESULT_DELETE	  	9
#define TORESULT_SELECT_ALL	10

toResultContentEditor::toResultContentEditor(QWidget *parent,const char *name)
  : QTable(parent,name),
    toEditWidget(false,true,true,
		 false,false,
		 false,false,false,
		 true,true,true)
{
  CurrentEditor=NULL;
  SearchStart=SearchEnd=0;
  NoUseReturning=false;
  AllFilter=false;
  MaxColDisp=toTool::globalConfig(CONF_MAX_COL_DISP,DEFAULT_MAX_COL_DISP).toInt();
  Query=NULL;
  SingleEdit=NULL;
  connect(this,SIGNAL(currentChanged(int,int)),this,SLOT(changePosition(int,int)));
  CurrentRow=-1;
  setFocusPolicy(StrongFocus);
  setSelectionMode(Single);
  connect(horizontalHeader(),SIGNAL(clicked(int)),this,SLOT(changeSort(int)));
  SortRow=-1;
  setAcceptDrops(true);
  LastMove=QPoint(-1,-1);
  MenuColumn=MenuRow=-1;

  Menu=new QPopupMenu(this);
  Menu->insertItem(tr("&Display in editor..."),TORESULT_MEMO);
  Menu->insertSeparator();
  Menu->insertItem(tr("&Copy field"),TORESULT_COPY_FIELD);
  Menu->insertItem(tr("&Paste field"),TORESULT_PASTE);
  Menu->insertSeparator();
  Menu->insertItem(tr("Copy selection"),TORESULT_COPY_SEL);
  Menu->insertItem(tr("Copy selection with header"),TORESULT_COPY_SEL_HEAD);
  Menu->insertItem(tr("Copy transposed"),TORESULT_COPY_TRANS);
  Menu->insertSeparator();
  Menu->insertItem(tr("&Delete record"),TORESULT_DELETE);
  Menu->insertSeparator();
  Menu->insertItem(tr("Select all"),TORESULT_SELECT_ALL);
  Menu->setAccel(CTRL+Key_A,TORESULT_SELECT_ALL);
  Menu->insertSeparator();
  Menu->insertItem(tr("Export to file..."),TORESULT_EXPORT);
  Menu->insertItem(tr("Read all"),TORESULT_READ_ALL);
  connect(Menu,SIGNAL(activated(int)),this,SLOT(menuCallback(int)));

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
  toStatusMessage(tr("Can't use these on toResultContent"));
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

void toResultContentEditor::query(const QString &,const toQList &params)
{
  if (params.size()!=2)
    wrongUsage();
  toQList::const_iterator par=params.begin();
  QString Param1=*par;
  par++;
  QString Param2=*par;

  Owner=Param1;
  Table=Param2;
  if (AllFilter)
    FilterName=("");
  else {
    FilterName=Owner;
    FilterName+=(".");
    FilterName+=Table;
  }

  setNumRows(0);
  setNumCols(0);
  NewRecordRow = -1;
  MaxNumber=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER).toInt();

  delete Query;
  Query=NULL;
  GotoEnd=false;

  try {
    SQL=("SELECT * FROM ");
    SQL+=table();
    bool where=false;
    if (!Criteria[FilterName.utf8()].isEmpty()) {
      SQL+=(" WHERE ");
      SQL+=Criteria[FilterName.utf8()];
      SQL+=(" ");
      where=true;
    }
    emit filterEnabled(where);

    toQList par;

    QString order;
    if (!Order[FilterName.utf8()].isEmpty()) {
      order=(" ORDER BY ");
      order+=Order[FilterName.utf8()];
    }

    QString init=SQL;
    SkipNumber=toTool::globalConfig(CONF_MAX_CONTENT,DEFAULT_MAX_CONTENT).toInt();

    if (SkipNumber>0) {
      if (connection().provider()=="Oracle")
	init="SELECT * FROM ("+SQL+order+") WHERE ROWNUM <= "+QString::number(SkipNumber);
      else if (connection().provider()=="MySQL")
	init=SQL+order+" LIMIT "+QString::number(SkipNumber);
      else if (connection().provider()=="SapDB") {
	init=SQL;
	if (where)
	  init+=" AND ";
	else
	  init+=" WHERE ";
	init+="ROWNO <= "+QString::number(SkipNumber)+order;
      }
    }
    SQL+=order;
    Query=new toNoBlockQuery(connection(),toQuery::Background,init,par);
    Poll.start(100);
    OrigValues.clear();
    CurrentRow=-1;
  } TOCATCH
}

#define STOP_RESIZE_ROW 200

void toResultContentEditor::poll(void)
{
  try {
    if (!toCheckModal(this))
      return;
    if (Query&&Query->poll()) {
      bool first=false;
      if (numRows()==0) {
	Description=Query->describe();
	setNumCols(Description.size());

	QHeader *head=horizontalHeader();
	int col=0;
	for (toQDescList::iterator i=Description.begin();i!=Description.end();i++) {
	  head->setLabel(col,(*i).Name);
	  col++;
	}
	Row=0;
	first=true;
      }
      
      std::list<QString> data;
      int dataSize=0;

      for (int j=Row;(j<MaxNumber||MaxNumber<0)&&Query->poll()&&!Query->eof();j++) {
	for (int k=0;k<numCols();k++)
	  if (SkipNumber<=0||j<SkipNumber) {
	    toQValue val=Query->readValueNull();
	    if (val.isDouble())
	      data.insert(data.end(),QString::number(val.toDouble()));
	    else
	      data.insert(data.end(),val);
	    dataSize++;
	  } else
	    Query->readValueNull();
	if (SkipNumber>0&&j>=SkipNumber)
	  SkipNumber--;
      }

      int rows=Row+dataSize/numCols()+1;
      if (numRows()!=rows) {
	setUpdatesEnabled(false);
	setNumRows(rows);

#if QT_VERSION >= 300
	int origRow=Row;
#endif
	while(dataSize>0) {
	  verticalHeader()->setLabel(Row,QString::number(Row+1));
	  for(int j=0;j<numCols();j++) {
	    setText(Row,j,toShift(data));
	    dataSize--;
	  }
	  Row++;
	}
#if QT_VERSION >= 300
	if (numRows()<STOP_RESIZE_ROW)
	  for(int j=0;j<numCols();j++) {
	    int width=columnWidth(j);
	    for(int k=origRow;k<Row;k++) {
	      QRect bounds=fontMetrics().boundingRect(text(k,j));
	      int cw=min(bounds.width(),MaxColDisp);
	      if (cw>width)
		width=cw;
	    }
	    if (width!=columnWidth(j))
	      setColumnWidth(j,width);
	  }
#endif
	setUpdatesEnabled(true);
      }
      if (first&&SingleEdit) {
	SingleEdit->changeSource(this);
	saveRow(currentRow());
      }

      if (Query->eof()) {
	delete Query;
	Query=NULL;

	if (SkipNumber>0&&Row==SkipNumber) {
	  toQList par;
	  Query=new toNoBlockQuery(connection(),toQuery::Background,SQL,par);
	} else {
	  Poll.stop();
	  if (GotoEnd)
	    setCurrentCellFocus(Row-1,currentColumn());
	}
      } else if (Row>=MaxNumber&&MaxNumber>=0)
	Poll.stop();
    }
  } catch(const QString &str) {
    delete Query;
    Query=NULL;
    Poll.stop();
    if (GotoEnd)
      setCurrentCellFocus(Row-1,currentColumn());
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

  QString txt=text(row,col);
  int nl=txt.find(("\n"));
  if (nl>=0) {
    txt=txt.mid(0,nl);
    txt+=("...");
  }

  toQDescList::iterator i=Description.begin();
  for(;col>0&&i!=Description.end();i++,col--)
    ;
  int align=AlignLeft;
  if (i!=Description.end()&&(*i).AlignRight)
    align=AlignRight;
  align|=ExpandTabs|AlignVCenter;

  p->setBrush(selected?colorGroup().highlight():colorGroup().base());
  p->setPen(colorGroup().foreground());
  p->drawRect(-1,-1,cr.width()+1,cr.height()+1);
  if (selected)
    p->setPen(colorGroup().highlightedText());
  p->drawText(1,1,cr.width()-3,cr.height()-3,align,txt);
}

QWidget *toResultContentEditor::beginEdit(int row,int col,bool replace)
{
  SearchStart=SearchEnd=0;
  if (CurrentRow!=row)
    toStatusMessage(tr("Unsaved data in contents, select other row to store"),true);
  saveRow(row);

  QString txt=text(row,col);
  if (txt.contains(("\n"))) {
    toMemoEditor *edit=new toResultContentMemo(this,txt,row,col);
    connect(edit,SIGNAL(changeData(int,int,const QString &)),
	    this,SLOT(changeData(int,int,const QString &)));
  }

  QWidget *ret=QTable::beginEdit(row,col,replace);
  if (ret->isA("QLineEdit"))
    CurrentEditor=(QLineEdit *)ret;
  return ret;
}

void toResultContentEditor::endEdit(int row,int col,bool accept,bool replace)
{
  CurrentEditor=NULL;
  QTable::endEdit(row,col,accept,replace);
}

void toResultContentEditor::gotoLastRecord()
{
  editReadAll();
  if (!Query||Query->eof()) {
    setNumRows(Row+1);
    setCurrentCellFocus(Row-1,currentColumn());
  } else {
    toStatusMessage(tr("Reading all values, moving cursor to end when done"),false,false);
    GotoEnd=true;
  }
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
      setText(crow,i,(""));

    NewRecordRow = crow;
    setNumRows(numRows());
    setCurrentCellFocus(crow,0);
  }
}

void toResultContentEditor::duplicateRecord()
{
  if(CurrentRow>0 || NewRecordRow>0) {
    return;
  }

  int fromrow=currentRow();
  addRecord();
  if (currentRow()+1<numRows())
    fromrow++;
  if (fromrow!=currentRow())
    for(int i=0;i<numCols();i++)
      setText(currentRow(),i,text(fromrow,i));
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
  if (SingleEdit)
    setCurrentCellFocus(crow,!currentColumn()); // Must change position
  else
    setCurrentCellFocus(crow,0);
  toStatusMessage(tr("Edit cancelled"),false,false);
}

void toResultContentEditor::deleteCurrent()
{
  if(currentRow()==NewRecordRow) {
    cancelEdit();
    return;
  }
  saveUnsaved();
  if (currentRow()<Row) {
    try {
      QString sql=("DELETE FROM ");
      sql+=table();
      sql+=(" WHERE ");
    
      QHeader *head=horizontalHeader();
      toQDescList::iterator di=Description.begin();
      bool where=false;
      toConnection &conn=connection();
      
      bool oracle=(connection().provider()=="Oracle");
      {
        for(int i=0;i<numCols();i++) {
	  if (!oracle||(!(*di).Datatype.startsWith(("LONG"))&&
			!(*di).Datatype.contains(("LOB")))) {
	    if (where)
	      sql+=(" AND ");
	    else
	      where=true;
	    sql+=conn.quote(head->label(i));
	    if (!text(currentRow(),i))
	      sql+=(" IS NULL");
	    else {
	      sql+=("= :c");
	      sql+=QString::number(i);
	      sql+=("<char[4000]>");
	    }
	  }
	  di++;
	}
      }
      if (!where) {
	toStatusMessage(tr("This table contains only LOB/LONG columns and can not be edited"));
	return;
      }
      toQList args;
      di=Description.begin();
      for(int i=0;i<numCols();i++) {
	QString str=text(currentRow(),i);
	if (!str.isNull()&&(!oracle||(!(*di).Datatype.startsWith(("LONG"))&&
				      !(*di).Datatype.contains(("LOB")))))
	  toPush(args,toQValue(str));
	di++;
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
  OrigValues.clear();
  if (SingleEdit)
    setCurrentCellFocus(crow,!currentColumn()); // Must change position
  else
    setCurrentCellFocus(crow,0);
}

void toResultContentEditor::saveUnsaved(void)
{
  if (SingleEdit&&CurrentRow>=0)
    SingleEdit->saveRow(this,CurrentRow);

  endEdit(currentRow(),currentColumn(),true,true);

  if (OrigValues.size()>0) {

    // Check to make sure some changes were actually made

    std::list<QString>::iterator j=OrigValues.begin();
    for (int i=0;i<numCols()&&j!=OrigValues.end();i++,j++)
      if (*j!=text(CurrentRow,i))
	break;
    if (j==OrigValues.end()) {
      toStatusMessage(tr("No changes made"),false,false);
      return;
    }

    bool oracle=false;
    QString rowid;
    try {
      toConnection &conn = connection();
      oracle=(connection().provider()=="Oracle");
      toStatusMessage(tr("Saved row"),false,false);
      if (CurrentRow>=Row || CurrentRow==NewRecordRow) {
	QString sql=("INSERT INTO ");
	sql+=table();
	sql+=" (";
	int num=0;
	QHeader *head=horizontalHeader();
	for (int i=0;i<numCols();i++) {
	  if (!text(CurrentRow,i).isNull()) {
	    if (num>0)
	      sql+=",";
	    sql+=conn.quote(head->label(i));
	    num++;
	  }
	}
	sql+=") VALUES (";
	num=0;
	for (int i=0;i<numCols();i++) {
	  if (!text(CurrentRow,i).isNull()) {
	    if (num>0)
	      sql+=(",");
	    sql+=(":f");
	    sql+=QString::number(num+1);
	    sql+=("<char[4000],in>");
	    num++;
	  }
	}
	sql+=(")");
	if(oracle&&!NoUseReturning) 
	  sql+=(" RETURNING ROWID INTO :r<char[101],out>");
	
	try {
	  toQList args;
	  for (int i=0;i<numCols();i++) {
	    QString str=text(CurrentRow,i);
	    if (!str.isNull())
	      toPush(args,toQValue(str));
	  }
	  toQuery q(conn,sql,args);
	  if(oracle&&!NoUseReturning)
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
	QString sql=("UPDATE ");
	sql+=table();
	sql+=(" SET ");
	QHeader *head=horizontalHeader();
	std::list<QString>::iterator k=OrigValues.begin();
	bool first=false;
	for(int i=0;i<numCols();i++,k++) {
	  QString fld=text(CurrentRow,i);
	  if (*k!=fld) {
	    if (!first)
	      first=true;
	    else
	      sql+=(", ");
	    sql+=conn.quote(head->label(i));
	    if (fld.isNull())
	      sql+=(" = NULL");
	    else {
	      sql+=("= :f");
	      sql+=QString::number(i);
	      sql+=("<char[4000],in>");
	    }
	  }
	}
	if (first) {
	  sql+=(" WHERE ");
	  int col=0;
	  bool where=false;
	  toQDescList::iterator di=Description.begin();
	  for(std::list<QString>::iterator j=OrigValues.begin();j!=OrigValues.end();j++,col++) {
	    if (!oracle||(!(*di).Datatype.startsWith(("LONG"))&&
			  !(*di).Datatype.contains(("LOB")))) {
	      if (where)
		sql+=(" AND ");
	      else
		where=true;
	      sql+=conn.quote((*di).Name);
	      if ((*j).isNull())
		sql+=(" IS NULL");
	      else {
		sql+=("= :c");
		sql+=QString::number(col);
		sql+=("<char[4000],in>");
	      }
	    }
	    di++;
	  }
	  if (!where) {
	    toStatusMessage(tr("This table contains only LOB/LONG columns and can not be edited"));
	    return;
	  }
	  if(oracle&&!NoUseReturning)
	    sql+=(" RETURNING ROWID INTO :r<char[101],out>");
	  try {
	    toQList args;

	    std::list<QString>::iterator k=OrigValues.begin();
	    for (int i=0;i<numCols();i++,k++) {
	      QString str=text(CurrentRow,i);
	      if (str!=*k&&!str.isNull())
		toPush(args,toQValue(str));
	    }

	    toQDescList::iterator di=Description.begin();
	    for(std::list<QString>::iterator j=OrigValues.begin();j!=OrigValues.end();j++,col++) {
	      QString str=(*j);
	      if (!str.isNull()&&(!oracle||(!(*di).Datatype.startsWith(("LONG"))&&
					    !(*di).Datatype.contains(("LOB")))))
		toPush(args,toQValue(str));
	      di++;
	    }
	    toQuery q(conn,sql,args);
	    if(oracle&&!NoUseReturning)
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
    } TOCATCH
    OrigValues.clear();
    if(oracle) {
      try {
	QString sql;
	sql=("SELECT * FROM ");
	sql+=table();
	sql+=(" WHERE rowid = :r<char[101]>");
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
  SearchStart=SearchEnd=0;
  if (CurrentRow!=row&&CurrentRow>=0)
    saveUnsaved();

  if (NewRecordRow>0 && NewRecordRow!=row) {
    cancelEdit();
    setCurrentCell(row,col);
  } else if (SingleEdit) {
    saveRow(currentRow()); // Don't use row here since row can have changed
    SingleEdit->changeRow(this,currentRow());
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
  QString name=tr("Content of %1.%2").arg(Owner).arg(Table);
  print.setSQLName(name);
  print.query(SQL);
  print.editPrint();
}

bool toResultContentEditor::editSave(bool ask)
{
  toResultView list(false,true,this);
  list.hide();
  QString name=tr("Content of %1.%2").arg(Owner).arg(Table);
  list.setSQLName(name);
  list.query(SQL);
  list.editReadAll();
  return list.editSave(ask);
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

void toResultContentEditor::displayMenu(const QPoint &p)
{
  QPoint lp=mapFromGlobal(p);
  lp=QPoint(lp.x()+contentsX()-verticalHeader()->width(),
	    lp.y()+contentsY()-horizontalHeader()->height());
  MenuColumn=columnAt(lp.x());
  MenuRow=rowAt(lp.y());
  if (MenuColumn>=0&&MenuRow>=0) {
    setCurrentCell(MenuRow,MenuColumn);
    Menu->popup(p);
    QClipboard *clip=qApp->clipboard();
    Menu->setItemEnabled(TORESULT_PASTE,!clip->text().isEmpty());
  }
}

void toResultContentEditor::displayMemo(void)
{
  toMemoEditor *edit=new toResultContentMemo(this,text(MenuRow,MenuColumn),MenuRow,MenuColumn);
  connect(edit,SIGNAL(changeData(int,int,const QString &)),
	  this,SLOT(changeData(int,int,const QString &)));
}

void toResultContentEditor::changeData(int row,int col,const QString &str)
{
  changePosition(col,row);
  saveRow(row);
  setText(row,col,str);
  setCurrentCell(row,col);
}

void toResultContentEditor::editSelectAll(void)
{
  removeSelection(0);
  QTableSelection sel;
  sel.init(0,0);
  sel.expandTo(numRows()-1,numCols()-1);
}

void toResultContentEditor::menuCallback(int cmd)
{
  switch(cmd) {
  case TORESULT_COPY_FIELD:
    {
      QClipboard *clip=qApp->clipboard();
      clip->setText(text(MenuRow,MenuColumn));
    }
    break;
  case TORESULT_PASTE:
    {
      QClipboard *clip=qApp->clipboard();
      saveRow(MenuRow);
      setText(MenuRow,MenuColumn,clip->text());
    }
    break;
  case TORESULT_COPY_SEL:
    {
      toListView *lst=copySelection(false);
      if (lst) {
	try {
	  QClipboard *clip=qApp->clipboard();
	  clip->setText(lst->exportAsText(false,false));
	} TOCATCH
	delete lst;
      }
    }
    break;
  case TORESULT_COPY_SEL_HEAD:
    {
      toListView *lst=copySelection(true);
      if (lst) {
	try {
	  QClipboard *clip=qApp->clipboard();
	  clip->setText(lst->exportAsText(true,false));
	} TOCATCH
        delete lst;
      }
    }
    break;
  case TORESULT_SELECT_ALL:
    editSelectAll();
    break;
  case TORESULT_COPY_TRANS:
    {
      editSelectAll();
      toListView *lst=copySelection(true);
      if (lst) {
	lst->copyTransposed();
	delete lst;
      }
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
  try {
    QString sql;
    if (connection().provider()!="PostgreSQL") {
      sql=connection().quote(Owner);
      sql+=(".");
    }
    sql+=connection().quote(Table);
    return sql;
  } catch(const QString &str) {
    toStatusMessage(str);
    return Table;
  }
}

bool toResultContentEditor::searchNext(toSearchReplace *search)
{
  int row=currentRow();
  int col=currentColumn();
  while(row<numRows()) {
    int pos=SearchEnd;
    int endPos;

    if (search->findString(text(row,col),pos,endPos)) {
      setCurrentCell(row,col);

#if QT_VERSION >= 300
      editCell(row,col);
      if (CurrentEditor)
	CurrentEditor->setSelection(pos,endPos-pos);
#endif

      SearchEnd=endPos;
      SearchStart=pos;
      return true;
    }
    col++;
    if (col>=numCols()) {
      row++;
      col=0;
    }
    SearchEnd=0;
  }
  return false;
}

void toResultContentEditor::searchReplace(const QString &newData)
{
  if (SearchEnd>0) {
    int row=currentRow();
    int col=currentColumn();
    QTableItem *itm=item(row,col);
    if (itm) {
      QString txt=itm->text();
      txt.replace(SearchStart,SearchEnd-SearchStart,newData);
      saveRow(row);
      itm->setText(txt);
    }
  }
}

bool toResultContentEditor::searchCanReplace(bool all)
{
  if (all||(SearchEnd>0))
    return true;
  return false;
}

toResultContent::toResultContent(QWidget *parent,const char *name)
  : QVBox(parent,name)
{
  QToolBar *toolbar=toAllocBar(this,tr("Content editor"));
  Editor=new toResultContentEditor(this,name);

  QToolButton *btn=new QToolButton(QPixmap((const char **)filter_xpm),
				   tr("Define filter for editor"),
				   tr("Define filter for editor"),
				   this,SLOT(changeFilter()),toolbar);
  connect(Editor,
	  SIGNAL(filterEnabled(bool)),
	  btn,
	  SLOT(setOn(bool)));
  btn->setToggleButton(true);

  new QToolButton(QPixmap((const char **)nofilter_xpm),
		  tr("Remove any filter"),
		  tr("Remove any filter"),
		  this,SLOT(removeFilter()),toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)addrecord_xpm),
		  tr("Add a new record"),
		  tr("Add a new record"),
		  Editor,SLOT(addRecord()),toolbar);
  new QToolButton(QPixmap((const char **)duplicaterecord_xpm),
		  tr("Duplicate an existing record"),
		  tr("Duplicate an existing record"),
		  Editor,SLOT(duplicateRecord()),toolbar);
  new QToolButton(QPixmap((const char **)saverecord_xpm),
		  tr("Save changes"),
		  tr("Save changes"),
		  Editor,SLOT(saveUnsaved()),toolbar);
  new QToolButton(QPixmap((const char **)canceledit_xpm),
		  tr("Discard changes"),
		  tr("Discard changes"),
		  Editor,SLOT(cancelEdit()),toolbar);
  new QToolButton(QPixmap((const char **)trash_xpm),
		  tr("Delete current record from table"),
		  tr("Delete current record from table"),
		  Editor,SLOT(deleteCurrent()),toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)rewind_xpm),
		  tr("Go to first row"),
		  tr("Go to first row"),
		  Editor,SLOT(gotoFirstRecord()),toolbar);
  new QToolButton(QPixmap((const char **)previous_xpm),
		  tr("Go to previous row"),
		  tr("Go to previous row"),
		  Editor,SLOT(gotoPreviousRecord()),toolbar);
  new QToolButton(QPixmap((const char **)next_xpm),
		  tr("Go to next row"),
		  tr("Go to next row"),
		  Editor,SLOT(gotoNextRecord()),toolbar);
  new QToolButton(QPixmap((const char **)forward_xpm),
		  tr("Go to last row"),
		  tr("Go to last row"),
		  Editor,SLOT(gotoLastRecord()),toolbar);
  toolbar->addSeparator();

  btn=new QToolButton(toolbar);
  btn->setToggleButton(true);
  btn->setIconSet(QIconSet(QPixmap((const char **)single_xpm)));
  connect(btn,SIGNAL(toggled(bool)),Editor,SLOT(singleRecordForm(bool)));
  QToolTip::add(btn,tr("Toggle between table or single record editing"));
  
  toolbar->setStretchableWidget(new QLabel(toolbar,TO_KDE_TOOLBAR_WIDGET));
  connect(toMainWidget(),SIGNAL(willCommit(toConnection &,bool)),
	  this,SLOT(saveUnsaved(toConnection &,bool)));
}

void toResultContent::changeFilter(void)
{
  toResultContentFilterUI filter(this,"FilterSetup",true);
  filter.AllTables->setChecked(Editor->allFilter());
  filter.Order->setText(Editor->Order[Editor->FilterName.utf8()]);
  filter.Criteria->setText(Editor->Criteria[Editor->FilterName.utf8()]);
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
    nam=("");
  else {
    nam=Owner;
    nam+=(".");
    nam+=Table;
  }
  Criteria[nam.utf8()]=crit;
  Order[nam.utf8()]=ord;
  saveUnsaved();

  QString t=Owner;
  Owner=QString::null;
  changeParams(t,Table);
}

void toResultContent::saveUnsaved(toConnection &conn,bool)
{
  try {
    toConnection &mycon=connection();
    if (&mycon==&conn) // Is this same connection
      saveUnsaved();
  } TOCATCH
}

bool toResultContent::canHandle(toConnection &)
{
  return true;
}

void toResultContent::removeFilter(void)
{
  if (!Editor->allFilter()) {
    switch(TOMessageBox::information(this,tr("Remove filter"),
				     tr("Remove the filter for this table only or for all tables."),
				     tr("&All"),tr("&This"),tr("Cancel"),0)) {
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

void toResultContentEditor::saveRow(int row)
{
  if (row!=CurrentRow) {
    OrigValues.clear();
    for (int i=0;i<numCols();i++)
      OrigValues.insert(OrigValues.end(),text(row,i));
    CurrentRow=row;
  }
}

void toResultContentEditor::singleRecordForm(bool display)
{
  if (display&&!SingleEdit) {
    SingleEdit=new toResultContentSingle(parentWidget());
    SingleEdit->changeSource(this);
    saveRow(currentRow());
    SingleEdit->show();
    hide();
  } else if (SingleEdit) {
    show();
    SingleEdit->saveRow(this,currentRow());
    delete SingleEdit;
    SingleEdit=NULL;
  }
}

void toResultContentEditor::exportData(std::map<QCString,QString> &data,const QCString &prefix)
{
  if (AllFilter)
    data[prefix+":All"]="Yes";
  toMapExport(data,prefix+":Crit",Criteria);
  toMapExport(data,prefix+":Order",Order);
}

void toResultContentEditor::importData(std::map<QCString,QString> &data,const QCString &prefix)
{
  AllFilter=!(data[prefix+":All"].isEmpty());
  toMapImport(data,prefix+":Crit",Criteria);
  toMapImport(data,prefix+":Order",Order);
}

toListView *toResultContentEditor::copySelection(bool header)
{
  QTableSelection sel=selection(0);
  if (sel.isActive()) {
    toListView *lst=new toListView(this);
    int row;
    int col;
    if (header) {
      lst->addColumn(("#"));
      lst->setColumnAlignment(0,AlignRight);
    }
    for (col=sel.leftCol();col<=sel.rightCol();col++) {
      QString lab=horizontalHeader()->label(col);
      lst->addColumn(lab);
      for(toQDescList::iterator i=Description.begin();i!=Description.end();i++) {
	if ((*i).Name==lab) {
	  if ((*i).AlignRight)
	    lst->setColumnAlignment(lst->columns()-1,AlignRight);
	  break;
	}
      }
    }
    QListViewItem *item=NULL;
    for (row=sel.topRow();row<=sel.bottomRow();row++) {
      item=new toResultViewItem(lst,item);
      if (header)
	item->setText(0,verticalHeader()->label(row));
      for (col=sel.leftCol();col<=sel.rightCol();col++) {
	item->setText(col-sel.leftCol()+(header?1:0),
		      text(row,col));
      }
    }
    QString name=tr("Content of %1.%2").arg(Owner).arg(Table);
    lst->setSQLName(name);
    return lst;
  }
  return NULL;
}

toResultContentSingle::toResultContentSingle(QWidget *parent)
  : QScrollView(parent)
{
  enableClipper(true);
  Container=NULL;
  Row=-1;
  viewport()->setBackgroundColor(qApp->palette().active().background());
}

void toResultContentSingle::changeSource(QTable *table)
{
  delete Container;
  Container=new QGrid(4,viewport());
  addChild(Container,5,5);
  Container->setSpacing(10);
  Value.clear();
  Null.clear();

  QHeader *head=table->horizontalHeader();
  for(int i=0;i<table->numCols();i++) {
    new QLabel(head->label(i),Container);
    QLineEdit *edit=new QLineEdit(Container,QString::number(i));
    edit->setFixedWidth(300);
    QCheckBox *box=new QCheckBox(("NULL"),Container);
    connect(box,SIGNAL(toggled(bool)),edit,SLOT(setDisabled(bool)));

    toParamGetButton *btn=new toParamGetButton(i,Container);
    btn->setText(tr("Edit"));
    btn->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed));
    connect(btn,SIGNAL(clicked(int)),this,SLOT(showMemo(int)));
    connect(box,SIGNAL(toggled(bool)),btn,SLOT(setDisabled(bool)));
    Value.insert(Value.end(),edit);
    Null.insert(Null.end(),box);
  }
  Row=table->currentRow();
  Container->show();
  changeRow(table,Row);
}

void toResultContentSingle::changeRow(QTable *table,int row)
{
  bool any=false;
  std::list<QCheckBox *>::iterator chk=Null.begin();
  std::list<QLineEdit *>::iterator val=Value.begin();
  for(int i=0;i<table->numCols()&&chk!=Null.end()&&val!=Value.end();i++) {
    QString str=table->text(row,i);
    if (!str.isNull())
      any=true;
    (*chk)->setChecked(str.isNull());
    (*val)->setText(str);
    chk++;
    val++;
  }
  if (!any)
    for(chk=Null.begin();chk!=Null.end();chk++)
      (*chk)->setChecked(false);
  Row=row;
}

void toResultContentSingle::saveRow(QTable *table,int row)
{
  if (Row==row) {
    std::list<QLineEdit *>::iterator val=Value.begin();
    for(int i=0;i<table->numCols()&&val!=Value.end();i++) {
      table->setText(row,i,(*val)->isEnabled()?(*val)->text():QString::null);
      val++;
    }
  } else
    toStatusMessage(tr("Internal error, save different row than current in content editor"));
}

void toResultContentSingle::showMemo(int row)
{
  QObject *obj=child(QString::number(row));
  if (obj) {
    toMemoEditor *memo=new toMemoEditor(this,((QLineEdit *)obj)->text(),row,0,false,true);
    if (memo->exec())
      ((QLineEdit *)obj)->setText(memo->text());
  }
}
