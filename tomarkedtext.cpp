//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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
