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



#include <stdio.h>
#include <ctype.h>

#include <qclipboard.h>
#include <qheader.h>
#include <qtooltip.h>
#include "toresultview.h"
#include "toresultview.moc"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"

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
  return min(TextWidth(fm,text(column)),MaxColDisp)+top->itemMargin()*2+2;
}

int toResultViewItem::width(const QFontMetrics &fm, const QListView *top, int column) const
{
  QRect bounds=fm.boundingRect(text(column));
  return min(bounds.width(),MaxColDisp)+top->itemMargin()*2+2;
}


void toResultViewItem::paintCell(QPainter * p,const QColorGroup & cg,int column,int width,int align)
{
  QListViewItem::paintCell(p,cg,column,width,align);
  if (itemBelow()==NULL) {
    toResultView *view=(toResultView *)listView();
    view->addItem();
  }
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

class toResultTip : public QToolTip {
private:
  toResultView *Result;
public:
  toResultTip(toResultView *parent)
    : QToolTip(parent->viewport())
  {
    Result=parent;
  }

  virtual void maybeTip(const QPoint &p)
  {
    QListViewItem *item=Result->itemAt(p);
    toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
    if (item) {
      int col=Result->header()->sectionAt(Result->viewportToContents(p).x());
      QPoint t(Result->header()->sectionPos(col),0);
      int pos=Result->contentsToViewport(t).x();
      int width=Result->header()->sectionSize(col);
      QString key;
	
      QString text=item->text(col);
      if (resItem)
	key=resItem->allText(col);
      else
	key=text;
      int textWidth=TextWidth(qApp->fontMetrics(),text)+Result->itemMargin()*2+2;
      if (key!=text||
	  width<textWidth) {
	QRect itemRect=Result->itemRect(item);
	itemRect.setLeft(pos);
	itemRect.setRight(pos+width);
	tip(itemRect,key);
      }
    }
  }
};

void toResultView::setup(bool readable,bool dispCol)
{
  Query=NULL;
  ReadableColumns=readable;
  setAllColumnsShowFocus(true);
  setSorting(-1);
  NumberColumn=dispCol;
  if (NumberColumn)
    addColumn("#");
  AllResult=new toResultTip(this);
}

toResultView::toResultView(bool readable,bool dispCol,toConnection &conn,QWidget *parent,const char *name)
  : QListView(parent,name), Connection(conn)
{
  setup(readable,dispCol);
}

toResultView::toResultView(toConnection &conn,QWidget *parent,const char *name)
  : QListView(parent,name), Connection(conn)
{
  setup(false,true);
}

void toResultView::addItem(void)
{
  int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();
  char buffer[MaxColSize+1];
  buffer[MaxColSize]=0;
  MaxColDisp=toTool::globalConfig(CONF_MAX_COL_DISP,DEFAULT_MAX_COL_DISP).toInt();

  try {
    if (Query&&!Query->eof()) {
      RowNumber++;
      int disp=0;
      QListViewItem *item=createItem(this,LastItem,NULL);
      LastItem=item;
      if (NumberColumn) {
	sprintf(buffer,"%d",RowNumber);
	item->setText(0,buffer);
	disp=1;
      }
      for (int j=0;j<DescriptionLen&&!Query->eof();j++) {
	(*Query)>>buffer;
	if (Query->is_null())
	  item->setText(j+disp,"{null}");
	else
	  item->setText(j+disp,buffer);
      }
    }
  } catch (const QString &str) {
    toStatusMessage((const char *)str);
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
  }
}

void toResultView::contentsMouseDoubleClickEvent (QMouseEvent *e)
{
  QPoint p=e->pos();
  int col=header()->sectionAt(p.x());
  QListViewItem *item=itemAt(contentsToViewport(p));
  toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
  QString str;
  QClipboard *clip=qApp->clipboard();
  if (resItem)
    clip->setText(resItem->allText(col));
  else if (item)
    clip->setText(item->text(col));
  
}

QString toResultView::query(const QString &sql,const QString *Param1,const QString *Param2,const QString *Param3)
{
  delete Query;
  SQL=sql;
  Query=NULL;
  LastItem=NULL;
  RowNumber=0;

  clear();
  while(columns()>1) {
    removeColumn(1);
  }
  if (!NumberColumn)
    removeColumn(0);

  try {
    Query=new otl_stream;

    int MaxColNum=toTool::globalConfig(CONF_MAX_COL_NUM,DEFAULT_MAX_COL_NUM).toInt();
    int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();

    for (int i=0;i<MaxColNum;i++)
      Query->set_column_type(i+1,otl_var_char,MaxColSize);

    Query->open(1,
		(const char *)sql,
		Connection.connection());

    if (Param1)
      (*Query)<<(const char *)(*Param1);
    if (Param2)
      (*Query)<<(const char *)(*Param2);
    if (Param3)
      (*Query)<<(const char *)(*Param3);

    Description=Query->describe_select(DescriptionLen);

    if (DescriptionLen>MaxColNum) {
      toStatusMessage("Too many column in result, increase maximum column number");
      return "";
    }

    bool hidden=false;

    for (int i=0;i<DescriptionLen;i++) {
      if (ReadableColumns) {
	bool inWord=false;
	char *name=Description[i].name;
	for (unsigned int j=0;j<strlen(name);j++) {
	  if (name[j]=='_')
	    name[j]=' ';
	  if (isspace(name[j]))
	    inWord=false;
	  else if (isalpha(name[j])) {
	    if (inWord)
	      name[j]=tolower(name[j]);
	    else
	      name[j]=toupper(name[j]);
	    inWord=true;
	  }
	}
      }
      if (Description[i].name[0]&&Description[i].name[0]!=' ') {
	if (hidden)
	  throw QString("Can only hide last column in query");
	if (Description[i].name[0]=='-') {
	  addColumn(Description[i].name+1);
	  setColumnAlignment(columns()-1,AlignRight);
	} else
	  addColumn(Description[i].name);
      } else
	hidden=true;
    }

    int MaxNumber=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER).toInt();
    for (int i=0;(i<MaxNumber||ReadAll)&&!Query->eof();i++)
      addItem();

    char buffer[10];
    if (Query->get_rpc()>0)
      sprintf(buffer,"%d rows processed",(int)Query->get_rpc());
    else
      sprintf(buffer,"Query executed");
    toStatusMessage(buffer);
    updateContents();
    return QString(buffer);
  } catch (const QString &str) {
    toStatusMessage((const char *)str);
    return str;
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
    updateContents();
    return QString((const char *)exc.msg);
  }
}

void toResultView::readAll(void)
{
  while(Query&&!Query->eof())
    addItem();
}
