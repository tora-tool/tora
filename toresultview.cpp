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

#include "utils.h"

#include "toconf.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "toparamget.h"
#include "toresultlistformatui.h"
#include "toresultview.h"
#include "tosearchreplace.h"
#include "tosql.h"
#include "totool.h"

#ifdef TO_KDE
#include <kfiledialog.h>
#endif
#ifdef TO_HAS_KPRINT
#include <kprinter.h>
#endif

#include <qapplication.h>
#include <qclipboard.h>
#include <qcombobox.h>
#include <qdragobject.h>
#include <qfiledialog.h>
#include <qheader.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpopupmenu.h>
#include <qprinter.h>
#include <qregexp.h>
#include <qtooltip.h>
#include <qworkspace.h>

#include "toresultlistformatui.moc"
#include "toresultview.moc"

static int MaxColDisp;

void toResultViewMLine::setText (int col,const QString &text)
{
  QListViewItem::setText(col,text);
  int pos=0;
  int lines=0;
  do {
    pos=text.find(QString::fromLatin1("\n"),pos);
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
  setHeight((listView()->fontMetrics().height()+1)*Lines+margin);
}

void toResultViewMLine::paintCell(QPainter *pnt,const QColorGroup & cg,
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
  do {
    pos=str.find(QString::fromLatin1("\n"),lpos);
    QRect bounds=fm.boundingRect(str.mid(lpos,pos-lpos));
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
    text.append(QString::fromLatin1("..."));
  }
  
  return text;
}

QString toResultViewItem::key(int col,bool asc) const
{
  static QRegExp number(QString::fromLatin1("^\\d*\\.?\\d+E?-?\\d*.?.?$"));

  QString val=text(col);
  if (number.match(val)>=0) {
    static char buf[100];
    sprintf(buf,"%015.5f",text(col).toFloat());
    return QString::fromLatin1(buf);
  } else if (val==QString::fromLatin1("N/A")) {
    if (asc)
      return QString::fromLatin1("\xff");
    else
      return QString::fromLatin1("\001");
  }
  return val;
}

void toResultViewMLCheck::setText (int col,const QString &text)
{
  QCheckListItem::setText(col,text);
  int pos=0;
  int lines=0;
  do {
    pos=text.find(QString::fromLatin1("\n"),pos);
    lines++;
    pos++;
  } while(pos>0);
  if (lines>Lines)
    Lines=lines;
}

void toResultViewMLCheck::setup(void)
{
  QCheckListItem::setup();
  int margin=listView()->itemMargin()*2;
  setHeight((listView()->fontMetrics().height()+1)*Lines+margin);
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
    text.append(QString::fromLatin1("..."));
  }
  
  return text;
}

QString toResultViewCheck::key(int col,bool asc) const
{
  static QRegExp number(QString::fromLatin1("^\\d*\\.?\\d+$"));
  QString val=text(col);
  if (number.match(val)>=0) {
    static char buf[100];
    sprintf(buf,"%015f",text(col).toFloat());
    return QString::fromLatin1(buf);
  } else if (val=="N/A") {
    if (asc)
      return QString::fromLatin1("\xff");
    else
      return QString::fromLatin1("\001");
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
    toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
    toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(item);
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
  FirstSearch=false;
  setTreeStepSize(15);
  setSelectionMode(Extended);
  setAllColumnsShowFocus(true);
  AllTip=new toListTip(this);
#if 0
  setShowSortIndicator(true);
#endif
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
  toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
  toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(item);
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

    try {
      toParamGet::setDefault(toCurrentConnection(this),
			     head->label(i).lower(),toUnnull(str));
    } catch(...) {
    }
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
    toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
    toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(item);
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

  double wpscalex=(double(metrics.width())*wmetr.widthMM()/metrics.widthMM()/wmetr.width());
  double wpscaley=(double(metrics.height())*wmetr.heightMM()/metrics.heightMM()/wmetr.height());

  painter->save();
  QFont font=painter->font();
  font.setPointSizeFloat(font.pointSizeFloat()/max(wpscalex,wpscaley));
  painter->setFont(font);

  painter->scale(wpscalex,wpscaley);

  double mwidth=metrics.width()/wpscalex;
  double mheight=metrics.height()/wpscaley;
  double x=0;
  if (paint) {
    QString numPage(tr("Page: %1").arg(pageNo));
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

  double y=(header()->height()+1)+header()->height();
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
	item->setSelected(false);
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
  printer.setFromTo(1,1000);
  if (printer.setup()) {
    printer.setCreator(tr("TOra"));
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
      QString str=tr("Printing page %1").arg(page);
      toStatusMessage(str,false,false);
    }
    painter.end();
    toStatusMessage(tr("Done printing"),false,false);
  }
}

void toListView::setDisplayMenu(QPopupMenu *m) {
	if (Menu)
		delete Menu;
	Menu=m;
}

#define TORESULT_COPY_FIELD     1
#define TORESULT_COPY_SEL	2
#define TORESULT_COPY_SEL_HEAD	3
#define TORESULT_MEMO     	4
#define TORESULT_SQL      	5
#define TORESULT_READ_ALL 	6
#define TORESULT_EXPORT   	7
#define TORESULT_SELECT_ALL	8
#define TORESULT_COPY_TRANS	9

void toListView::displayMenu(QListViewItem *item,const QPoint &p,int col)
{
  if (item) {
    if (!Menu) {
      Menu=new QPopupMenu(this);
      Menu->insertItem(tr("Display in editor..."),TORESULT_MEMO);
      Menu->insertSeparator();
      Menu->insertItem(tr("&Copy field"),TORESULT_COPY_FIELD);
      if (selectionMode()==Multi||selectionMode()==Extended) {
	Menu->insertItem(tr("Copy selection"),TORESULT_COPY_SEL);
	Menu->insertItem(tr("Copy selection with header"),TORESULT_COPY_SEL_HEAD);
      }
      Menu->insertItem(tr("Copy transposed"),TORESULT_COPY_TRANS);
      if (selectionMode()==Multi||selectionMode()==Extended) {
	Menu->insertSeparator();
	Menu->insertItem(tr("Select all"),TORESULT_SELECT_ALL);
	Menu->setAccel(CTRL+Key_A,TORESULT_SELECT_ALL);
      }
      Menu->insertSeparator();
      Menu->insertItem(tr("Export to file..."),TORESULT_EXPORT);
      if (!Name.isEmpty()) {
	Menu->insertSeparator();
	Menu->insertItem(tr("Edit SQL..."),TORESULT_SQL);
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
    new toMemoEditor(this,str,0,MenuColumn);
}

void toListView::menuCallback(int cmd)
{
  switch(cmd) {
  case TORESULT_COPY_FIELD:
    {
      QClipboard *clip=qApp->clipboard();
      clip->setText(menuText());
    }
    break;
  case TORESULT_COPY_SEL:
    try {
      QString str=exportAsText(false,true);
      QClipboard *clip=qApp->clipboard();
      clip->setText(str);
    } TOCATCH
    break;
  case TORESULT_COPY_SEL_HEAD:
    try {
      QString str=exportAsText(true,true);
      QClipboard *clip=qApp->clipboard();
      clip->setText(str);
    } TOCATCH
    break;
  case TORESULT_SELECT_ALL:
    selectAll(true);
    break;
  case TORESULT_MEMO:
    displayMemo();
    break;
  case TORESULT_COPY_TRANS:
    copyTransposed();
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
  toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(MenuItem);
  toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(MenuItem);
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
  static QRegExp quote(QString::fromLatin1("\""));
  QString t=str;
  t.replace(quote,QString::fromLatin1("\"\""));
  return t;
}

bool toListView::searchNext(toSearchReplace *search)
{
  QListViewItem *item=currentItem();

  bool first=FirstSearch;
  FirstSearch=false;

  for (QListViewItem *next=NULL;item;item=next) {
    if (!first)
      first=true;
    else {
      for (int i=0;i<columns();i++) {
	int pos=0;
	int endPos=0;

	toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
	toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(item);
	QString txt;
	if (resItem)
	  txt=resItem->allText(i);
	else if (chkItem)
	  txt=chkItem->allText(i);
	else if (item)
	  txt=item->text(i);
	
	if (search->findString(item->text(0),pos,endPos)) {
	  setCurrentItem(item);
	  return true;
	}
      }
    }

    if (item->firstChild())
      next=item->firstChild();
    else if (item->nextSibling())
      next=item->nextSibling();
    else {
      next=item;
      do {
	next=next->parent();
      } while(next&&!next->nextSibling());
      if (next)
	next=next->nextSibling();
    }
  }
  return false;
}

toListView *toListView::copyTransposed(void)
{
  toListView *lst=new toListView(toMainWidget()->workspace());
  lst->setWFlags(lst->getWFlags()|WDestructiveClose);
  lst->Name=Name;

  QListViewItem *next=NULL;
  for(int i=1;i<columns();i++) {
    next=new toResultViewItem(lst,next);
    next->setText(0,header()->label(i));
  }

  next=NULL;
  int col=1;
  lst->addColumn(header()->label(0));
  for (QListViewItem *item=firstChild();item;item=next) {

    lst->addColumn(item->text(0));
    QListViewItem *ci=lst->firstChild();
    for(int i=1;i<columns()&&ci;i++) {
      ci->setText(col,item->text(i));
      ci=ci->nextSibling();
    }

    if (item->firstChild()) {
      next=item->firstChild();
    } else if (item->nextSibling())
      next=item->nextSibling();
    else {
      next=item;
      do {
	next=next->parent();
      } while(next&&!next->nextSibling());
      if (next)
	next=next->nextSibling();
    }
    col++;
  }
  lst->setCaption(Name);
  lst->show();
  toMainWidget()->windowsMenu();
  return lst;
}

bool toListView::editSave(bool)
{
  try {
    QString delimiter;
    QString separator;
    int type=exportType(separator,delimiter);

    QString nam;
    switch(type) {
    case -1:
      return false;
    default:
      nam="*.txt";
      break;
    case 2:
      nam="*.csv";
      break;
    case 3:
      nam="*.html";
      break;
    }

    QString filename=toSaveFilename(QString::null,nam,this);
    if (filename.isEmpty())
      return false;
  
    return toWriteFile(filename,exportAsText(true,false,type,separator,delimiter));
  } TOCATCH
  return false;
}

void toListView::addMenues(QPopupMenu *)
{
}

bool toListView::searchCanReplace(bool)
{
  return false;
}

int toListView::exportType(QString &separator,QString &delimiter)
{
  toResultListFormat format(this,NULL);
  if (!format.exec())
    return -1;

  format.saveDefault();

  separator=format.Separator->text();
  delimiter=format.Delimiter->text();

  return format.Format->currentItem();

}

QString toListView::exportAsText(bool includeHeader,bool onlySelection,int type,
				 const QString &sep,const QString &del)
{
  QString separator=sep;
  QString delimiter=del;
  if (type<0)
    type=exportType(separator,delimiter);
  if (type<0)
    throw QString::null;

  int *sizes=NULL;
  try {
    if (type==0) {
      sizes=new int[columns()];
      int level=0;
      for (int i=0;i<columns();i++)
	if (includeHeader)
	  sizes[i]=header()->label(i).length();
	else
	  sizes[i]=0;

      {
	QListViewItem *next=NULL;
	for (QListViewItem *item=firstChild();item;item=next) {
	  toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
	  toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(item);

	  if (!onlySelection||item->isSelected()) {
	    for (int i=0;i<columns();i++) {
	      int csiz;
	      if (resItem)
		csiz=resItem->text(i).length();
	      else if (chkItem)
		csiz=chkItem->text(i).length();
	      else
		csiz=item->text(i).length();
	      if (i==0)
		csiz+=level;
	      if (sizes[i]<csiz)
		sizes[i]=csiz;
	    }
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
    if (type==3) {
      output=QString::fromLatin1("<HTML><HEAD><TITLE>%1</TITLE></HEAD><BODY><TABLE CELLSPACING=0 BORDER=0>").
	arg(sqlName());
    }

    QString indent;

    QString bgcolor;
    if (includeHeader) {
      if (bgcolor.isEmpty())
	bgcolor=QString::fromLatin1("nonull");
      else
	bgcolor=QString::null;
      if (type==3)
	output+=QString::fromLatin1("<TR BGCOLOR=#7f7f7f>");
      for (int j=0;j<columns();j++)
	switch(type) {
	case 0:
	  output+=QString::fromLatin1("%1 ").arg(header()->label(j),-sizes[j]);
	  break;
	case 1:
	  output+=QString::fromLatin1("%1\t").arg(header()->label(j));
	  break;
	case 2:
	  output+=QString::fromLatin1("%1%2%3%4").
	    arg(delimiter).
	    arg(QuoteString(header()->label(j))).
	    arg(delimiter).
	    arg(separator);
	  break;
	case 3:
	  output+=QString::fromLatin1("<TH ALIGN=LEFT BGCOLOR=#cfcfcf>");
	  output+=header()->label(j);
	  output+=QString::fromLatin1("</TH>");
	  break;
	}
      if (output.length()>0&&type==2)
	output=output.left(output.length()-separator.length());
      else if (output.length()>0&&type!=3)
	output=output.left(output.length()-1);
      else if (type==3&&includeHeader)
	output+=QString::fromLatin1("</TR>");
      output+=QString::fromLatin1("\n");
      if (type==0) {
	for (int k=0;k<columns();k++) {
	  for (int l=0;l<sizes[k];l++)
	    output+=QString::fromLatin1("=");
	  if (k!=columns()-1)
	    output+=QString::fromLatin1(" ");
	}
	output+=QString::fromLatin1("\n");
      }
    }

    QListViewItem *next=NULL;
    for (QListViewItem *item=firstChild();item;item=next) {

      if (!onlySelection||item->isSelected()) {

	toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
	toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(item);

	if (bgcolor.isEmpty())
	  bgcolor=QString::fromLatin1(" BGCOLOR=#cfcfff");
	else
	  bgcolor=QString::null;
	QString line;
	if (type==3)
	  line=QString::fromLatin1("<TR%1>").arg(bgcolor);      

	for (int i=0;i<columns();i++) {
	  QString text;

	  if (resItem)
	    text=resItem->text(i);
	  else if (chkItem)
	    text=chkItem->text(i);
	  else
	    text=item->text(i);

	  switch(type) {
	  case 0:
	    line+=indent;
	    line+=QString::fromLatin1("%1 ").arg(text,(i==0?indent.length():0)-sizes[i]);
	    break;
	  case 1:
	    line+=indent;
	    line+=QString::fromLatin1("%1\t").arg(text);
	    break;
	  case 2:
	    line+=indent;
	    line+=QString::fromLatin1("%1%2%3%4").
	      arg(delimiter).
	      arg(QuoteString(text)).
	      arg(delimiter).
	      arg(separator);

	    break;
	  case 3:
	    line+=QString::fromLatin1("<TD%1>").arg(bgcolor);
	    if (i==0)
	      line+=indent;
	    line+=text;
	    line+=QString::fromLatin1("</TD>");
	    break;
	  }
	}
	if (type==3)
	  line+=QString::fromLatin1("</TR>");
	else if (type==2)
	  line=line.left(line.length()-separator.length());
	else 
	  line=line.left(line.length()-1);
	line+=QString::fromLatin1("\n");
	output+=line;
      }

      if (item->firstChild()) {
	if (type!=3)
	  indent+=QString::fromLatin1(" ");
	else
	  indent+=QString::fromLatin1("&nbsp;");
	next=item->firstChild();
      } else if (item->nextSibling())
	next=item->nextSibling();
      else {
	next=item;
	do {
	  next=next->parent();
	  if (type==3)
	    indent.truncate(indent.length()-5);
	  else
	    indent.truncate(indent.length()-1);
	} while(next&&!next->nextSibling());
	if (next)
	  next=next->nextSibling();
      }
    }
    if (type==3)
      output+=QString::fromLatin1("</TABLE></BODY></HTML>");
    delete[] sizes;
    return output;
  } catch(...) {
    delete[] sizes;
    throw;
  }
}

void toListView::exportData(std::map<QCString,QString> &ret,const QCString &prefix)
{
  int id=0;
  for(int i=0;i<columns();i++) {
    id++;
    ret[prefix+":Labels:"+QString::number(id).latin1()]=header()->label(i);
  }
  std::map<QListViewItem *,int> itemMap;
  QListViewItem *next=NULL;
  id=0;
  if (rootIsDecorated())
    ret[prefix+":Decorated"]=QString::fromLatin1("Yes");
  for (QListViewItem *item=firstChild();item;item=next) {
    id++;
    QCString nam=prefix;
    nam+=":Items:";
    nam+=QString::number(id).latin1();
    nam+=":";
    itemMap[item]=id;
    if (item->parent())
      ret[nam+"Parent"]=QString::number(itemMap[item->parent()]);
    else
      ret[nam+"Parent"]=QString::fromLatin1("0");
    if (item->isOpen())
      ret[nam+"Open"]=QString::fromLatin1("Yes");
    for(int i=0;i<columns();i++) {
      toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
      toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(item);
      QString val;
      if (resItem)
	val=resItem->allText(i);
      else if (chkItem)
	val=resItem->allText(i);
      else
	val=item->text(i);
      ret[nam+QString::number(i).latin1()]=val;
    }

    if (item->firstChild())
      next=item->firstChild();
    else if (item->nextSibling())
      next=item->nextSibling();
    else {
      next=item;
      do {
	next=next->parent();
      } while(next&&!next->nextSibling());
      if (next)
	next=next->nextSibling();
    }
  }
}

void toListView::importData(std::map<QCString,QString> &ret,const QCString &prefix)
{
  int id;
  std::map<QCString,QString>::iterator i;
  clear();

  id=1;
  while((i=ret.find(prefix+":Labels:"+QString::number(id).latin1()))!=ret.end()) {
    addColumn((*i).second);
    id++;
  }

  setRootIsDecorated(ret.find(prefix+":Decorated")!=ret.end());
  setSorting(0);

  std::map<int,QListViewItem *> itemMap;

  id=1;
  while((i=ret.find(prefix+":Items:"+QString::number(id).latin1()+":Parent"))!=ret.end()) {
    QCString nam=prefix+":Items:"+QString::number(id).latin1()+":";
    int parent=(*i).second.toInt();
    toResultViewItem *item;
    if (parent)
      item=new toResultViewItem(itemMap[parent],NULL);
    else
      item=new toResultViewItem(this,NULL);
    if (!ret[nam+"Open"].isEmpty())
      item->setOpen(true);
    itemMap[id]=item;
    for(int j=0;j<columns();j++)
      item->setText(j,ret[nam+QString::number(j).latin1()]);
    id++;
  }
}

bool toResultView::eof(void)
{
  return !Query||Query->eof();
}

QString toResultView::middleString()
{
  try {
    return connection().description();
  } catch(...) {
    return QString::null;
  }
}

void toResultView::setup(bool readable,bool dispCol)
{
  Query=NULL;
  ReadableColumns=readable;
  NumberColumn=dispCol;
  SortConnected=false;
  if (NumberColumn) {
    addColumn(QString::fromLatin1("#"));
    setColumnAlignment(0,AlignRight);
  }
  Filter=NULL;
  readAllEnabled(true);
  ReadAll=false;
  SortColumn=-2;
  SortAscending=true;
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

#define STOP_RESIZE_ROW 200

QListViewItem *toResultView::createItem(QListViewItem *last,const QString &str)
{
  if (childCount()==STOP_RESIZE_ROW)
    for (int i=0;i<columns();i++)
      setColumnWidthMode(i,Manual);

  return new toResultViewItem(this,last,str);
}

void toResultView::addItem(void)
{
  MaxColDisp=toTool::globalConfig(CONF_MAX_COL_DISP,DEFAULT_MAX_COL_DISP).toInt();

  try {
    if (Query&&!Query->eof()) {
      RowNumber++;
      int disp=0;
      QListViewItem *last=LastItem;
      LastItem=createItem(LastItem,QString::null);
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

  if (!setSQLParams(sql,param))
    return;

  delete Query;
  Query=NULL;
  LastItem=NULL;
  RowNumber=0;

  clear();
  while(columns()>0) {
    removeColumn(0);
  }
  if (NumberColumn) {
    addColumn(QString::fromLatin1("#"));
    setColumnAlignment(0,AlignRight);
  }

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
	  throw tr("Can only hide last column in query");
	if (name.at(0)=='-') {
	  addColumn(toTranslateMayby(sqlName(),name.right(name.length()-1)));
	  setColumnAlignment(columns()-1,AlignRight);
	} else {
	  addColumn(toTranslateMayby(sqlName(),name));
	  if ((*i).AlignRight)
	    setColumnAlignment(columns()-1,AlignRight);
	}
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
  } TOCATCH
  updateContents();
}

void toResultView::editReadAll(void)
{
  if (!ReadAll)
    toStatusMessage(tr("Reading all entries"),false,false);
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
  menu->insertItem(tr("Read All"),TORESULT_READ_ALL);
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

void toResultView::setSorting(int col,bool asc)
{
  if (col==SortColumn&&asc==SortAscending)
    return;
  SortColumn=col;
  SortAscending=asc;
  if (((col==0&&NumberColumn)||(col==columns()&&!NumberColumn))&&asc==true) {
    col=-1;
    toListView::setSorting(0,true);
    sort();
    QTimer::singleShot(1,this,SLOT(checkHeading()));
  } else if (SortConnected) {
    SortConnected=false;
    disconnect(header(),SIGNAL(clicked(int)),this,SLOT(headingClicked(int)));
  }
  toListView::setSorting(col,asc);
}

void toResultView::headingClicked(int col)
{
  if (col==SortColumn)
    setSorting(col,!SortAscending);
  else
    setSorting(col,true);
}

void toResultView::checkHeading(void)
{
  SortConnected=true;
  connect(header(),SIGNAL(clicked(int)),this,SLOT(headingClicked(int)));
}

toResultListFormat::toResultListFormat(QWidget *parent,const char *name)
  : toResultListFormatUI(parent,name,true)
{
  Format->insertItem(tr("Text"));
  Format->insertItem(tr("Tab delimited"));
  Format->insertItem(tr("CSV"));
  Format->insertItem(tr("HTML"));
  int num=toTool::globalConfig(CONF_DEFAULT_FORMAT,"").toInt();
  Format->setCurrentItem(num);
  formatChanged(num);

  Delimiter->setText(toTool::globalConfig(CONF_CSV_DELIMITER,DEFAULT_CSV_DELIMITER));
  Separator->setText(toTool::globalConfig(CONF_CSV_SEPARATOR,DEFAULT_CSV_SEPARATOR));
}

void toResultListFormat::formatChanged(int pos)
{
  Separator->setEnabled(pos==2);
  Delimiter->setEnabled(pos==2);
}

void toResultListFormat::saveDefault(void)
{
  toTool::globalSetConfig(CONF_CSV_DELIMITER,Delimiter->text());
  toTool::globalSetConfig(CONF_CSV_SEPARATOR,Separator->text());
  toTool::globalSetConfig(CONF_DEFAULT_FORMAT,QString::number(Format->currentItem()));
}

void toResultFilter::exportData(std::map<QCString,QString> &,const QCString &)
{
}

void toResultFilter::importData(std::map<QCString,QString> &,const QCString &)
{
}
