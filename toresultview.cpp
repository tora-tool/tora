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

#ifdef TO_KDE
#include <kfiledialog.h>
#endif

#include <qpopupmenu.h>
#include <qclipboard.h>
#include <qheader.h>
#include <qtooltip.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qpalette.h>
#include <qpaintdevicemetrics.h>
#include <qdragobject.h>
#include <qfiledialog.h>
#include <qregexp.h>

#include "tosearchreplace.h"
#include "tomain.h"
#include "toresultview.h"
#include "tosql.h"
#include "toconf.h"
#include "totool.h"
#include "tomemoeditor.h"
#include "toparamget.h"

#include "toresultview.moc"

#ifdef TO_HAS_KPRINT
#include <kprinter.h>
#endif

static int MaxColDisp;

void toResultViewMLine::setText (int col,const QString &text)
{
  QListViewItem::setText(col,text);
  int pos=0;
  int lines=0;
  do {
    pos=text.find("\n",pos);
    lines++;
    pos++;
  } while(pos>0);
  if (lines>Lines)
    Lines=lines;
}

void toResultViewMLine::setup(void)
{
  QListViewItem::setup();
  int margin=listView()->itemMargin()*2+1;
  setHeight((height()-margin)*Lines+margin);
}

void toResultViewMLine::paintCell (QPainter *pnt,const QColorGroup & cg,
				   int column,int width,int alignment)
{
  toResultViewItem::paintCell(pnt,cg,column,
			      max(QListViewItem::width(pnt->fontMetrics(),listView(),column),width),
			      alignment);
}

static int TextWidth(const QFontMetrics &fm,const QString &str)
{
  int lpos=0;
  int pos=0;
  int maxWidth=0;
  const char *strtext=str;
  do {
    pos=str.find("\n",lpos);
    QRect bounds=fm.boundingRect(strtext+lpos,pos-lpos);
    if (bounds.width()>maxWidth)
      maxWidth=bounds.width();
    lpos=pos+1;
  } while(pos>=0);
  return maxWidth;
}

int toResultViewMLine::width(const QFontMetrics &fm, const QListView *top, int column) const
{
  if (!MaxColDisp)
    MaxColDisp=toTool::globalConfig(CONF_MAX_COL_DISP,DEFAULT_MAX_COL_DISP).toInt();
  return min(TextWidth(fm,text(column)),MaxColDisp)+top->itemMargin()*2+2;
}

int toResultViewItem::width(const QFontMetrics &fm, const QListView *top, int column) const
{
  if (!MaxColDisp)
    MaxColDisp=toTool::globalConfig(CONF_MAX_COL_DISP,DEFAULT_MAX_COL_DISP).toInt();
  QRect bounds=fm.boundingRect(text(column));
  return min(bounds.width(),MaxColDisp)+top->itemMargin()*2+2;
}

void toResultViewItem::paintCell(QPainter * p,const QColorGroup & cg,int column,int width,int align)
{
  QListViewItem::paintCell(p,cg,column,width,align);
  toResultView *view=dynamic_cast<toResultView *>(listView());
  if (view&&(itemBelow()==NULL||itemBelow()->itemBelow()==NULL))
    view->addItem();
}

QString toResultViewItem::text(int col) const
{
  QString text=QListViewItem::text(col);
  int pos=text.find('\n');
  if (pos!=-1) {
    text.remove(pos,text.length());
    text.append("...");
  }
  
  return text;
}

QString toResultViewItem::key(int col,bool asc) const
{
  static QRegExp number("^\\d*\\.?\\d+E?-?\\d*.?.?$");

  QString val=text(col);
  if (number.match(val)>=0) {
    static char buf[100];
    sprintf(buf,"%015.5f",text(col).toFloat());
    return buf;
  }
  return val;
}

void toResultViewMLCheck::setText (int col,const QString &text)
{
  QCheckListItem::setText(col,text);
  int pos=0;
  int lines=0;
  do {
    pos=text.find("\n",pos);
    lines++;
    pos++;
  } while(pos>0);
  if (lines>Lines)
    Lines=lines;
}

void toResultViewMLCheck::setup(void)
{
  QCheckListItem::setup();
  int margin=listView()->itemMargin()*2+1;
  setHeight((height()-margin)*Lines+margin);
}

void toResultViewMLCheck::paintCell (QPainter *pnt,const QColorGroup & cg,
				   int column,int width,int alignment)
{
  toResultViewCheck::paintCell(pnt,cg,column,
			      max(QCheckListItem::width(pnt->fontMetrics(),listView(),column),width),
			      alignment);
}

int toResultViewMLCheck::width(const QFontMetrics &fm, const QListView *top, int column) const
{
  if (!MaxColDisp)
    MaxColDisp=toTool::globalConfig(CONF_MAX_COL_DISP,DEFAULT_MAX_COL_DISP).toInt();
  return min(TextWidth(fm,text(column)),MaxColDisp)+top->itemMargin()*2+2;
}

int toResultViewCheck::width(const QFontMetrics &fm, const QListView *top, int column) const
{
  if (!MaxColDisp)
    MaxColDisp=toTool::globalConfig(CONF_MAX_COL_DISP,DEFAULT_MAX_COL_DISP).toInt();
  QRect bounds=fm.boundingRect(text(column));
  return min(bounds.width(),MaxColDisp)+top->itemMargin()*2+2;
}


void toResultViewCheck::paintCell(QPainter * p,const QColorGroup & cg,int column,int width,int align)
{
  QCheckListItem::paintCell(p,cg,column,width,align);
  toResultView *view=dynamic_cast<toResultView *>(listView());
  if (view&&(itemBelow()==NULL||itemBelow()->itemBelow()==NULL))
    view->addItem();
}

QString toResultViewCheck::text(int col) const
{
  QString text=QCheckListItem::text(col);
  int pos=text.find('\n');
  if (pos!=-1) {
    text.remove(pos,text.length());
    text.append("...");
  }
  
  return text;
}

QString toResultViewCheck::key(int col,bool asc) const
{
  static QRegExp number("^\\d*\\.?\\d+$");
  QString val=text(col);
  if (number.match(val)>=0) {
    static char buf[100];
    sprintf(buf,"%015f",text(col).toFloat());
    return buf;
  }
  return val;
}

class toListTip : public QToolTip {
private:
  toListView *List;
public:
  toListTip(toListView *parent)
    : QToolTip(parent->viewport())
  {
    List=parent;
  }

  virtual void maybeTip(const QPoint &p)
  {
    QListViewItem *item=List->itemAt(p);
    toResultViewItem *resItem;
    try {
      resItem=dynamic_cast<toResultViewItem *>(item);
    } catch(...) {
      resItem=NULL;
    }
    toResultViewCheck *chkItem;
    try {
      chkItem=dynamic_cast<toResultViewCheck *>(item);
    } catch(...) {
      chkItem=NULL;
    }
    if (item) {
      int col=List->header()->sectionAt(List->viewportToContents(p).x());
      QPoint t(List->header()->sectionPos(col),0);
      int pos=List->contentsToViewport(t).x();
      int width=List->header()->sectionSize(col);
      QString key;
	
      QString text=item->text(col);
      if (resItem)
	key=resItem->tooltip(col);
      else if (chkItem)
	key=chkItem->tooltip(col);
      else
	key=text;
      int textWidth=TextWidth(List->fontMetrics(),text)+List->itemMargin()*2+2;
      if (key!=text||
	  width<textWidth) {
	QRect itemRect=List->itemRect(item);
	itemRect.setLeft(pos);
	itemRect.setRight(pos+width);
	tip(itemRect,key);
      }
    }
  }
};

toListView::toListView(QWidget *parent,const char *name)
  : QListView(parent,name),
    toEditWidget(false,true,true,
		 false,false,
		 false,false,false,
		 true,false,false)
{
  setAllColumnsShowFocus(true);
  AllTip=new toListTip(this);
  setShowSortIndicator(true);
  setSorting(-1);
  Menu=NULL;
  connect(this,SIGNAL(rightButtonPressed(QListViewItem *,const QPoint &,int)),
	  this,SLOT(displayMenu(QListViewItem *,const QPoint &,int)));
  QString str=toTool::globalConfig(CONF_LIST,"");
  if (!str.isEmpty()) {
    QFont font(toStringToFont(str));
    setFont(font);
  }
  LastMove=QPoint(-1,-1);
}

void toListView::contentsMouseDoubleClickEvent (QMouseEvent *e)
{
  QPoint p=e->pos();
  int col=header()->sectionAt(p.x());
  QListViewItem *item=itemAt(contentsToViewport(p));
  toResultViewItem *resItem;
  try {
    resItem=dynamic_cast<toResultViewItem *>(item);
  } catch(...) {
    resItem=NULL;
  }
  toResultViewCheck *chkItem;
  try {
    chkItem=dynamic_cast<toResultViewCheck *>(item);
  } catch(...) {
    chkItem=NULL;
  }
  QClipboard *clip=qApp->clipboard();
  if (resItem)
    clip->setText(resItem->allText(col));
  else if (chkItem)
    clip->setText(chkItem->allText(col));
  else if (item)
    clip->setText(item->text(col));

  QHeader *head=header();
  for (int i=0;i<columns();i++) {
    QString str;
    if (resItem)
      str=resItem->allText(i);
    else if (chkItem)
      str=chkItem->allText(col);
    else if (item)
      str=item->text(i);

    if (str=="{null}")
      str=QString::null;
    toParamGet::setDefault(head->label(i).lower(),str);
  }
}

void toListView::contentsMouseMoveEvent (QMouseEvent *e)
{
  if (e->state()==LeftButton&&
      e->stateAfter()==LeftButton&&
      LastMove.x()>0&&
      LastMove!=e->pos()) {
    QPoint p=e->pos();
    int col=header()->sectionAt(p.x());
    QListViewItem *item=itemAt(contentsToViewport(p));
    toResultViewItem *resItem;
    try {
      resItem=dynamic_cast<toResultViewItem *>(item);
    } catch(...) {
      resItem=NULL;
    }
    toResultViewCheck *chkItem;
    try {
      chkItem=dynamic_cast<toResultViewCheck *>(item);
    } catch(...) {
      chkItem=NULL;
    }
    QString str;
    if (resItem)
      str=resItem->allText(col);
    else if (chkItem)
      str=chkItem->allText(col);
    else if (item)
      str=item->text(col);
    if (str.length()) {
      QDragObject *d=new QTextDrag(str,this);
      d->dragCopy();
    }
  } else {
    LastMove=e->pos();
    QListView::contentsMouseMoveEvent(e);
  }
}

void toListView::contentsMousePressEvent(QMouseEvent *e)
{
  LastMove=QPoint(-1,-1);
  QListView::contentsMousePressEvent(e);
}

void toListView::contentsMouseReleaseEvent(QMouseEvent *e)
{
  LastMove=QPoint(-1,-1);
  QListView::contentsMouseReleaseEvent(e);
}

QListViewItem *toListView::printPage(TOPrinter *printer,QPainter *painter,QListViewItem *top,int &column,int &level,int pageNo,bool paint)
{
  QPaintDeviceMetrics wmetr(this);
  QPaintDeviceMetrics metrics(printer);
  double scale=toTool::globalConfig(CONF_LIST_SCALE,DEFAULT_LIST_SCALE).toFloat();
  if (scale<0.001)
    scale=1;

  double wpscalex=(double(metrics.width())*wmetr.widthMM()/metrics.widthMM()/wmetr.width());
  double wpscaley=(double(metrics.height())*wmetr.heightMM()/metrics.heightMM()/wmetr.height());

  painter->save();
  QFont font=painter->font();
  font.setPointSizeFloat(font.pointSizeFloat()/max(wpscalex,wpscaley));
  painter->setFont(font);

  painter->scale(wpscalex,wpscaley);

  double mwidth=metrics.width()/scale/wpscalex;
  double mheight=metrics.height()/scale/wpscaley;
  double x=0;
  if (paint) {
    QString numPage("Page: ");
    numPage+=QString::number(pageNo);
    painter->drawText(0,int(metrics.height()/wpscaley)-header()->height(),int(metrics.width()/wpscalex),
		      header()->height(),
		      SingleLine|AlignRight|AlignVCenter,
		      numPage);
    painter->drawText(0,int(metrics.height()/wpscaley)-header()->height(),int(metrics.width()/wpscalex),
		      header()->height(),
		      SingleLine|AlignHCenter|AlignVCenter,
		      middleString());
    painter->drawText(0,int(metrics.height()/wpscaley)-header()->height(),int(metrics.width()/wpscalex),
		      header()->height(),
		      SingleLine|AlignLeft|AlignVCenter,
		      sqlName());
    painter->scale(scale,scale);
    painter->drawLine(0,header()->height()-1,int(mwidth),header()->height()-1);
  }
  font=toListView::font();
  font.setPointSizeFloat(font.pointSizeFloat()/max(wpscalex,wpscaley));
  painter->setFont(font);

  for (int i=column;i<columns();i++) {
    double width=columnWidth(i);
    if (width+x>=mwidth) {
      if (i==column)
	width=mwidth-x-1;
      else
	break;
    }
    if (paint)
      painter->drawText(int(x),0,int(width),
			header()->height(),
			SingleLine|AlignLeft|AlignVCenter,header()->label(i));
    x+=width;
  }
  if (paint)
    painter->translate(0,header()->height());

  double y=(header()->height()+1)/scale+header()->height();
  int curLevel=level;
  int tree=rootIsDecorated()?treeStepSize():0;
  int newCol=-1;
  QListViewItem *item=top;
  while(item&&(y<mheight||item==top)) {
    if (column==0)
      x=curLevel;
    else
      x=0;
    painter->translate(x,0);
    for (int i=column;i<columns();i++) {
      double width=columnWidth(i);
      if (width+x>=mwidth) {
	if (i==column)
	  width=mwidth-x-1;
	else {
	  newCol=i;
	  break;
	}
      }
      if (i==0)
	width-=curLevel;
      if (paint) {
	item->paintCell(painter,qApp->palette().active(),i,int(width),columnAlignment(i));
	painter->translate(width,0);
      }
      x+=width;
    }
    if (paint)
      painter->translate(-x,item->height());
    y+=item->height();
    if (item->firstChild()) {
      item=item->firstChild();
      curLevel+=tree;
    } else if (item->nextSibling())
      item=item->nextSibling();
    else {
      do {
	item=item->parent();
	curLevel-=tree;
      } while(item&&!item->nextSibling());
      if (item)
	item=item->nextSibling();
    }
  }
  if (paint)
    painter->drawLine(0,0,int(mwidth),0);
  painter->restore();
  if (newCol>=0) {
    column=newCol;
    return top;
  }
  column=0;
  level=curLevel;
  return item;
}

void toListView::editPrint(void)
{
  TOPrinter printer;
  printer.setMinMax(1,1000);
  if (printer.setup()) {
    printer.setCreator("TOra");
    QPainter painter(&printer);

    QListViewItem *item=firstChild();
    int column=0;
    int tree=rootIsDecorated()?treeStepSize():0;
    int page=1;
    while(page<printer.fromPage()&&
	  (item=printPage(&printer,&painter,item,column,tree,page++,false)))
      painter.resetXForm();
    while((item=printPage(&printer,&painter,item,column,tree,page++))&&
	  (printer.toPage()==0||page<=printer.toPage())) {
      printer.newPage();
      painter.resetXForm();
      qApp->processEvents();
      QString str("Printing page ");
      str+=QString::number(page);
      toStatusMessage(str,false,false);
    }
    painter.end();
    toStatusMessage("Done printing");
  }
}

#define TORESULT_COPY     1
#define TORESULT_MEMO     2
#define TORESULT_SQL      3
#define TORESULT_READ_ALL 4
#define TORESULT_EXPORT   5

void toListView::displayMenu(QListViewItem *item,const QPoint &p,int col)
{
  if (item) {
    if (!Menu) {
      Menu=new QPopupMenu(this);
      Menu->insertItem("&Copy",TORESULT_COPY);
      Menu->insertItem("Display in editor",TORESULT_MEMO);
      Menu->insertItem("Export to file",TORESULT_EXPORT);
      if (!Name.isEmpty()) {
	Menu->insertSeparator();
	Menu->insertItem("Edit SQL",TORESULT_SQL);
      }
      connect(Menu,SIGNAL(activated(int)),this,SLOT(menuCallback(int)));
      addMenues(Menu);
    }
    MenuItem=item;
    MenuColumn=col;
    Menu->popup(p);
  }
}

void toListView::displayMemo(void)
{
  QString str=menuText();
  if (!str.isEmpty())
    new toMemoEditor(this,str);
}

void toListView::menuCallback(int cmd)
{
  switch(cmd) {
  case TORESULT_COPY:
    {
      QClipboard *clip=qApp->clipboard();
      clip->setText(menuText());
    }
    break;
  case TORESULT_MEMO:
    displayMemo();
    break;
  case TORESULT_SQL:
    toMainWidget()->editSQL(Name);
    break;
  case TORESULT_EXPORT:
    editSave(false);
    break;
  }
}

QString toListView::menuText(void)
{
  toResultViewItem *resItem;
  try {
    resItem=dynamic_cast<toResultViewItem *>(MenuItem);
  } catch(...) {
    resItem=NULL;
  }
  toResultViewCheck *chkItem;
  try {
    chkItem=dynamic_cast<toResultViewCheck *>(MenuItem);
  } catch(...) {
    chkItem=NULL;
  }
  QString str;
  if (resItem)
    str=resItem->allText(MenuColumn);
  else if (chkItem)
    str=chkItem->allText(MenuColumn);
  else if (MenuItem)
    str=MenuItem->text(MenuColumn);
  return str;
}

void toListView::focusInEvent (QFocusEvent *e)
{
  receivedFocus();
  QListView::focusInEvent(e);
}

static QString QuoteString(const QString &str)
{
  static QRegExp quote("\"");
  QString t=str;
  t.replace(quote,"\"\"");
  return t;
}

void toListView::editSearch(toSearchReplace *search)
{
  search->setTarget(this);
}

void toListView::editSave(bool ask)
{
  int type=TOMessageBox::information(this,"Export format",
				     "Select format to use for exporting to file",
				     "Text","Tab delimited","CSV");

  QString nam;
  switch(type) {
  default:
    nam="*.txt";
    break;
  case 2:
    nam="*.csv";
    break;
  }

  QString filename=toSaveFilename(QString::null,nam,this);
  if (filename.isEmpty())
    return;
  
  int *sizes=NULL;
  try {
    if (type==0) {
      sizes=new int[columns()];
      int level=0;
      for (int i=0;i<columns();i++)
	sizes[i]=header()->label(i).length();

      {
	QListViewItem *next=NULL;
	for (QListViewItem *item=firstChild();item;item=next) {

	  for (int i=0;i<columns();i++) {
	    int csiz=item->text(i).length();
	    if (i==0)
	      csiz+=level;
	    if (sizes[i]<csiz)
	      sizes[i]=csiz;
	  }

	  if (item->firstChild()) {
	    level++;
	    next=item->firstChild();
	  } else if (item->nextSibling())
	    next=item->nextSibling();
	  else {
	    next=item;
	    do {
	      next=next->parent();
	      level--;
	    } while(next&&!next->nextSibling());
	    if (next)
	      next=next->nextSibling();
	  }
	}
      }
    }

    QString output;

    QString indent;

    for (int j=0;j<columns();j++)
      switch(type) {
      case 0:
	output+=QString("%1 ").arg(header()->label(j),-sizes[j]);
	break;
      case 1:
	output+=QString("%1\t").arg(header()->label(j));
	break;
      case 2:
	output+=QString("\"%1\";").arg(QuoteString(header()->label(j)));
	break;
      }
    if (output.length()>0)
      output=output.left(output.length()-1);
    output+="\n";
    if (type==0) {
      for (int k=0;k<columns();k++) {
	for (int l=0;l<sizes[k];l++)
	  output+="=";
	if (k!=columns()-1)
	  output+=" ";
      }
      output+="\n";
    }

    QListViewItem *next=NULL;
    for (QListViewItem *item=firstChild();item;item=next) {

      QString line=indent;

      for (int i=0;i<columns();i++)
	switch(type) {
	case 0:
	  line+=QString("%1 ").arg(item->text(i),(i==0?indent.length():0)-sizes[i]);
	  break;
	case 1:
	  line+=QString("%1\t").arg(item->text(i));
	  break;
	case 2:
	  line+=QString("\"%1\";").arg(QuoteString(item->text(i)));
	  break;
	}
      line=line.left(line.length()-1);
      line+="\n";
      output+=line;

      if (item->firstChild()) {
	indent+=" ";
	next=item->firstChild();
      } else if (item->nextSibling())
	next=item->nextSibling();
      else {
	next=item;
	do {
	  next=next->parent();
	  indent.truncate(indent.length()-1);
	} while(next&&!next->nextSibling());
	if (next)
	  next=next->nextSibling();
      }
    }
    toWriteFile(filename,output);

  } catch(...) {
    delete sizes;
    throw;
  }
  delete sizes;
}


bool toResultView::eof(void)
{
  return !Query||Query->eof();
}

QString toResultView::middleString()
{
  return connection().description();
}

void toResultView::setup(bool readable,bool dispCol)
{
  Query=NULL;
  ReadableColumns=readable;
  NumberColumn=dispCol;
  if (NumberColumn)
    addColumn("#");
  Filter=NULL;
  readAllEnabled(true);
}

toResultView::toResultView(bool readable,bool dispCol,QWidget *parent,const char *name)
  : toListView(parent,name)
{
  setup(readable,dispCol);
}

toResultView::toResultView(QWidget *parent,const char *name)
  : toListView(parent,name)
{
  setup(false,true);
}

void toResultView::addItem(void)
{
  MaxColDisp=toTool::globalConfig(CONF_MAX_COL_DISP,DEFAULT_MAX_COL_DISP).toInt();

  try {
    if (Query&&!Query->eof()) {
      RowNumber++;
      int disp=0;
      QListViewItem *last=LastItem;
      LastItem=createItem(LastItem,NULL);
      if (NumberColumn) {
	LastItem->setText(0,QString::number(RowNumber));
	disp=1;
      } else
	LastItem->setText(columns(),QString::number(RowNumber));
      for (int j=0;(j<Query->columns()||j==0)&&!Query->eof();j++)
	LastItem->setText(j+disp,Query->readValue());
      if (Filter&&!Filter->check(LastItem)) {
	delete LastItem;
	LastItem=last;
	RowNumber--;
      }
    }
  } TOCATCH
}

void toResultView::query(const QString &sql,const toQList &param)
{
  if (!handled())
    return;

  delete Query;
  Query=NULL;
  setSQL(sql);
  setParams(param);
  LastItem=NULL;
  RowNumber=0;

  clear();
  while(columns()>0) {
    removeColumn(0);
  }
  if (NumberColumn)
    addColumn("#");

  try {
    Query=new toQuery(connection(),sql,param);

    toQDescList description=Query->describe();

    bool hidden=false;

    for (toQDescList::iterator i=description.begin();i!=description.end();i++) {
      QString name=(*i).Name;
      if (ReadableColumns)
	toReadableColumn(name);

      if (name.length()>0&&name.at(0)!=' ') {
	if (hidden)
	  throw QString("Can only hide last column in query");
	if (name.at(0)=='-') {
	  addColumn(name.right(name.length()-1));
	  setColumnAlignment(columns()-1,AlignRight);
	} else
	  addColumn(name);
      } else
	hidden=true;
    }

    if (NumberColumn)
      setSorting(0);
    else
      setSorting(Query->columns());

    int MaxNumber=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER).toInt();
    for (int j=0;j<MaxNumber&&!Query->eof();j++)
      addItem();
    if (ReadAll||MaxNumber<0)
      editReadAll();

    char buffer[100];
    if (Query->rowsProcessed()>0)
      sprintf(buffer,"%d rows processed",(int)Query->rowsProcessed());
    else
      sprintf(buffer,"Query executed");
  } TOCATCH
  updateContents();
}

void toResultView::editReadAll(void)
{
  if (!ReadAll)
    toStatusMessage("Reading all entries",false,false);
  int i=0;
  while(!eof()) {
    addItem();
    i++;
    if (i%100==0)
      qApp->processEvents();
  }
}

toResultView::~toResultView()
{
  delete Query;
  delete Filter;
}

void toResultView::keyPressEvent(QKeyEvent *e)
{
  if (e->key()==Key_PageDown) {
    QListViewItem *item=firstChild();
    if (item&&!eof()&&item->height()) {
      int num=visibleHeight()/item->height();
      while(num>0) {
	addItem();
	num--;
      }
    }
  }
  QListView::keyPressEvent(e);
}

void toResultView::addMenues(QPopupMenu *menu)
{
  menu->insertSeparator();
  menu->insertItem("Read All",TORESULT_READ_ALL);
}

void toResultView::menuCallback(int cmd)
{
  if (cmd==TORESULT_READ_ALL)
    editReadAll();
  else {
    if (cmd==TORESULT_EXPORT)
      editReadAll();
    toListView::menuCallback(cmd);
  }
}

int toResultView::queryColumns(void) const
{
  return Query?Query->columns():0;
}

