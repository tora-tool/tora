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

TO_NAMESPACE;

#include <qprinter.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qpalette.h>
#include <qpaintdevicemetrics.h>

#include "tomain.h"
#include "tomarkedtext.h"
#include "totool.h"
#include "toconf.h"

#include "tomarkedtext.moc"

#if QT_VERSION >= 300
#include "qtlegacy/qtmultilineedit.cpp"
#include "qtlegacy/qttableview.cpp"
#include "qtlegacy/qtmultilineedit.moc"
#include "qtlegacy/qttableview.moc"
#include "tomarkedtext.3.moc"
#else
#include "tomarkedtext.2.moc"
#endif

#ifdef TO_HAS_KPRINT
#include <kprinter.h>
#endif

toMarkedText::toMarkedText(QWidget *parent,const char *name)
: toMultiLineEdit(parent,name)
{
  setFont(toStringToFont(toTool::globalConfig(CONF_TEXT,"")));

  UndoAvailable=false;
  RedoAvailable=false;
  connect(this,SIGNAL(redoAvailable(bool)),this,SLOT(setRedoAvailable(bool)));
  connect(this,SIGNAL(undoAvailable(bool)),this,SLOT(setUndoAvailable(bool)));
  connect(this,SIGNAL(copyAvailable(bool)),this,SLOT(setCopyAvailable(bool)));
}

void toMarkedText::setEdit(void)
{
  if (isReadOnly()) {
    toMain::editEnable(false,true,true,
		       false,false,
		       hasMarkedText(),hasMarkedText(),false,true);
  } else {
    toMain::editEnable(true,true,true,
		       getUndoAvailable(),getRedoAvailable(),
		       hasMarkedText(),hasMarkedText(),true,true);
  }
}

void toMarkedText::focusInEvent (QFocusEvent *e)
{
  setEdit();
  toMultiLineEdit::focusInEvent(e);
}

void toMarkedText::focusOutEvent (QFocusEvent *e)
{
  toMain::editDisable();
  toMultiLineEdit::focusOutEvent(e);
}

toMarkedText::~toMarkedText()
{
  if (qApp->focusWidget()==this)
    toMain::editDisable();
}

void toMarkedText::paintEvent(QPaintEvent *pe)
{
  toMultiLineEdit::paintEvent(pe);
  if (hasFocus()) {
    int curline,curcol;
    getCursorPosition (&curline,&curcol);
    toMainWidget()->setCoordinates(curline+1,curcol+1);
  }
}

void toMarkedText::print(void)
{
  TOPrinter printer;
  printer.setMinMax(1,1000);
  if (printer.setup()) {
    printer.setCreator("TOra");
    QPainter painter(&printer);
    int line=0;
    int offset=0;
    int page=1;
    while(page<printer.fromPage()&&
	  (line=printPage(&printer,&painter,line,offset,page++,false)))
      painter.resetXForm();
    while((line=printPage(&printer,&painter,line,offset,page++))&&
	  line<numLines()&&
	  (printer.toPage()==0||page<=printer.toPage())) {
      printer.newPage();
      painter.resetXForm();
      qApp->processEvents();
      QString str("Printing page ");
      str+=QString::number(page);
      toStatusMessage(str);
    }
    painter.end();
    toStatusMessage("Done printing");
  }
}

int toMarkedText::printPage(TOPrinter *printer,QPainter *painter,int line,int &offset,
			    int pageNo,bool paint)
{
  QPaintDeviceMetrics metrics(printer);
  painter->drawLine(0,0,metrics.width(),0);
  QRect size=painter->boundingRect(0,0,metrics.width(),metrics.height(),
				   AlignLeft|AlignTop|ExpandTabs|SingleLine,
				   Filename);
  QString str("Page: ");
  str+=QString::number(pageNo);
  painter->drawText(0,metrics.height()-size.height(),size.width(),size.height(),
		    AlignLeft|AlignTop|ExpandTabs|SingleLine,
		    Filename);
  painter->drawText(size.width(),metrics.height()-size.height(),metrics.width()-size.width(),
		    size.height(),
		    AlignRight|AlignTop|SingleLine,
		    str);
  painter->drawLine(0,0,metrics.width(),0);
  int margin=size.height()+2;

  QFont defFont=painter->font();
  painter->setFont(font());
  size=painter->boundingRect(0,0,metrics.width(),metrics.height(),
			     AlignLeft|AlignTop,
			     "x");
  int height=size.height();
  int totalHeight=(metrics.height()-margin)/height*height;
  painter->drawLine(0,totalHeight+2,metrics.width(),totalHeight+2);
  painter->setClipRect(0,2,metrics.width(),totalHeight);
  int pos=1+offset;
  do {
    QRect bound;
    painter->drawText(0,pos,
		      metrics.width(),metrics.height(),
		      AlignLeft|AlignTop|ExpandTabs|WordBreak,
		      textLine(line),-1,&bound);
    int cheight=bound.height()?bound.height():height;
    totalHeight-=cheight;
    pos+=cheight;
    if (totalHeight>=0)
      line++;
  } while(totalHeight>0&&line<numLines());
  painter->setClipping(false);
  offset=totalHeight;
  painter->setFont(defFont);
  return line;
}
