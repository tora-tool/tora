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
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tomarkedtext.h"
#include "tosearchreplace.h"
#include "totool.h"

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

#include <qapplication.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qprinter.h>

#include "tomarkedtext.moc"

toMarkedText::toMarkedText(QWidget *parent,const char *name)
  : toMultiLineEdit(parent,name),toEditWidget()
{
  setEdit();
  Search=false;
  setFont(toStringToFont(toTool::globalConfig(CONF_TEXT,"")));

  connect(this,SIGNAL(redoAvailable(bool)),this,SLOT(setRedoAvailable(bool)));
  connect(this,SIGNAL(undoAvailable(bool)),this,SLOT(setUndoAvailable(bool)));
  connect(this,SIGNAL(copyAvailable(bool)),this,SLOT(setCopyAvailable(bool)));

  QPalette pal=palette();
  pal.setColor(QColorGroup::Base,
	       toSyntaxAnalyzer::defaultAnalyzer().getColor(toSyntaxAnalyzer::NormalBkg));
  pal.setColor(QColorGroup::Foreground,
	       toSyntaxAnalyzer::defaultAnalyzer().getColor(toSyntaxAnalyzer::Normal));
  setPalette(pal);

  CursorTimerID=-1;
}

void toMarkedText::setEdit(void)
{
  if (isReadOnly()) {
    toEditWidget::setEdit(false,true,true,
			  false,false,
			  false,hasMarkedText(),false,
			  true,true,false);
  } else {
    toEditWidget::setEdit(true,true,true,
			  undoEnabled(),redoEnabled(),
			  hasMarkedText(),hasMarkedText(),true,
			  true,true,false);
  }
  toMain::editEnable(this);
}

void toMarkedText::focusInEvent (QFocusEvent *e)
{
  receivedFocus();
  int curline,curcol;
  getCursorPosition (&curline,&curcol);
  toMainWidget()->setCoordinates(curline+1,curcol+1);
  toMultiLineEdit::focusInEvent(e);
  if (CursorTimerID<0)
    CursorTimerID=startTimer(500);
}

void toMarkedText::timerEvent(QTimerEvent *e)
{
  if (CursorTimerID==e->timerId()) {
    int curline,curcol;
    getCursorPosition (&curline,&curcol);
    toMainWidget()->setCoordinates(curline+1,curcol+1);
  } else
    toMultiLineEdit::timerEvent(e);
}

void toMarkedText::focusOutEvent (QFocusEvent *e)
{
  if (Search) {
    Search=false;
    LastSearch=SearchString;
    toStatusMessage(QString::null);
  }
  if (CursorTimerID>=0)
    killTimer(CursorTimerID);
  toMultiLineEdit::focusOutEvent(e);
}

void toMarkedText::dropEvent(QDropEvent *e)
{
  toMultiLineEdit::dropEvent(e);
  setFocus();
}

void toMarkedText::paintEvent(QPaintEvent *pe)
{
  toMultiLineEdit::paintEvent(pe);
}

void toMarkedText::editPrint(void)
{
  TOPrinter printer;
  printer.setMinMax(1,1000);
  printer.setFromTo(1,1000);
  if (printer.setup()) {
    printer.setCreator(tr(TOAPPNAME));
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
      toStatusMessage(tr("Printing page %1").arg(page),false,false);
    }
    painter.end();
    toStatusMessage(tr("Done printing"),false,false);
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
  QString str=tr("Page: %1").arg(pageNo);
  if (paint) {
    painter->drawText(0,metrics.height()-size.height(),size.width(),size.height(),
		      AlignLeft|AlignTop|ExpandTabs|SingleLine,
		      Filename);
    painter->drawText(size.width(),metrics.height()-size.height(),metrics.width()-size.width(),
		      size.height(),
		      AlignRight|AlignTop|SingleLine,
		      str);
    painter->drawLine(0,0,metrics.width(),0);
  }
  int margin=size.height()+2;

  QFont defFont=painter->font();
  painter->setFont(font());
  size=painter->boundingRect(0,0,metrics.width(),metrics.height(),
			     AlignLeft|AlignTop,
			     QString::fromLatin1("x"));
  int height=size.height();
  int totalHeight=(metrics.height()-margin)/height*height;
  if (paint)
    painter->drawLine(0,totalHeight+2,metrics.width(),totalHeight+2);
  painter->setClipRect(0,2,metrics.width(),totalHeight);
  int pos=1+offset;
  do {
    QRect bound;
    if (paint) {
      painter->drawText(0,pos,
			metrics.width(),metrics.height(),
			AlignLeft|AlignTop|ExpandTabs|WordBreak,
			textLine(line),-1,&bound);
    } else
      bound=painter->boundingRect(0,pos,
				  metrics.width(),metrics.height(),
				  AlignLeft|AlignTop|ExpandTabs|WordBreak,
				  textLine(line));
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

void toMarkedText::openFilename(const QString &file)
{
  QCString data=toReadFile(file);
  setText(QString::fromLocal8Bit(data));
  setFilename(file);
  setEdited(false);
  toMainWidget()->addRecentFile(file);
  toStatusMessage(tr("File opened successfully"),false,false);
}

bool toMarkedText::editOpen(QString suggestedFile)
{
  if (edited()) {
    int ret=TOMessageBox::information(this,
				      tr("Save changes?"),
				      tr("The editor has been changed, do you want to save them\n"
					 "before opening a new file?"),
				      tr("&Yes"),tr("&No"),tr("Cancel"),0,2);
    if (ret==2)
      return false;
    else if (ret==0)
      if (!editSave(false))
	return false;
  }

  QString fname;
  if (suggestedFile!=QString::null)
    fname=suggestedFile;
  else {
    QFileInfo file(filename());
    fname=toOpenFilename(file.dirPath(),QString::null,this);
  }
  if (!fname.isEmpty()) {
    try {
      openFilename(fname);
      return true;
    } TOCATCH
  }
  return false;
}

bool toMarkedText::editSave(bool askfile)
{
  QFileInfo file(filename());
  QString fn=filename();
  if (askfile||fn.isEmpty())
    fn=toSaveFilename(file.dirPath(),QString::null,this);
  if (!fn.isEmpty()) {
    if (!toWriteFile(fn,text()))
      return false;
    toMainWidget()->addRecentFile(fn);
    setFilename(fn);
    setEdited(false);
    return true;
  }
  return false;
}

void toMarkedText::newLine(void)
{
  toMultiLineEdit::newLine();
  if (!toTool::globalConfig(CONF_AUTO_INDENT,"Yes").isEmpty()) {
    int curline,curcol;
    getCursorPosition (&curline,&curcol);
    if (curline>0) {
      QString str=textLine(curline-1);
      QString ind;
      for(unsigned int i=0;i<str.length()&&str.at(i).isSpace();i++)
	ind+=str.at(i);
      if (ind.length())
	insert(ind,false);
    }
  }
}

void toMarkedText::searchFound(int line,int col)
{
  setCursorPosition(line,col+SearchString.length(),false);
  setCursorPosition(line,col,true);
  toStatusMessage(tr("Incremental search")+QString::fromLatin1(":")+SearchString,false,false);
}

void toMarkedText::incrementalSearch(bool forward,bool next)
{
  int curline,curcol;
  getCursorPosition (&curline,&curcol);
  QString line;
  if(SearchFailed&&next) {
    if (forward) {
      curline=0;
      curcol=0;
      next=false;
      line=textLine(curline);
    } else {
      curline=numLines()-1;
      line=textLine(curline);
      curcol=line.length();
      next=false;
    }
    SearchFailed=false;
  } else
    line=textLine(curline);
  if (forward) {
    if (next)
      curcol++;
    if (curcol+SearchString.length()<=line.length()) {
      int pos=line.find(SearchString,curcol,false);
      if (pos>=0) {
	searchFound(curline,pos);
	return;
      }
    }
    for(curline++;curline<numLines();curline++) {
      int pos=textLine(curline).find(SearchString,0,false);
      if (pos>=0) {
	searchFound(curline,pos);
	return;
      }
    }
  } else {
    if (next)
      curcol--;
    if (curcol>=0) {
      int pos=line.findRev(SearchString,curcol,false);
      if (pos>=0) {
	searchFound(curline,pos);
	return;
      }
    }
    for(curline--;curline>=0;curline--) {
      int pos=textLine(curline).findRev(SearchString,-1,false);
      if (pos>=0) {
	searchFound(curline,pos);
	return;
      }
    }
  }
  toStatusMessage(tr("Incremental search")+QString::fromLatin1(":")+SearchString+QString::fromLatin1(" (failed)"),false,false);
  SearchFailed=true;
}

void toMarkedText::mousePressEvent(QMouseEvent *e)
{
  if (Search) {
    Search=false;
    LastSearch=SearchString;
    toStatusMessage(QString::null);
  }
  toMultiLineEdit::mousePressEvent(e);
}

void toMarkedText::incrementalSearch(bool forward)
{
  SearchForward=forward;
  if (!Search) {
    Search=true;
    SearchFailed=false;
    SearchString=QString::null;
    toStatusMessage(tr("Incremental search")+QString::fromLatin1(":"),false,false);
  } else if (Search) {
    if (!SearchString.length())
      SearchString=LastSearch;
    if (SearchString.length())
      incrementalSearch(SearchForward,true);
  }
}

void toMarkedText::keyPressEvent(QKeyEvent *e)
{
  if(e->state()==0&&e->key()==Key_Insert) {
    setOverwriteMode(!isOverwriteMode());
    e->accept();
    return;
  } else if(e->state()==ControlButton&&e->key()==Key_A) {
    selectAll();
    e->accept();
    return;
  } else if (Search) {
    bool ok=false;
    if (e->state()==NoButton&&e->key()==Key_Backspace) {
      int len=SearchString.length();
      if (len>0)
	SearchString.truncate(len-1);
      ok=true;
    } else if (e->key()!=Key_Escape) {
      QString t=e->text();
      if (t.length()) {
	SearchString+=t;
	ok=true;
      } else if (e->key()==Key_Shift||
		 e->key()==Key_Control||
		 e->key()==Key_Meta||
		 e->key()==Key_Alt) {
	ok=true;
      }
    }
    
    if (ok) {
      incrementalSearch(SearchForward,false);
      e->accept();
      return;
    } else {
      Search=false;
      LastSearch=SearchString;
      toStatusMessage(QString::null);
    }
  }
  toMultiLineEdit::keyPressEvent(e);
}

void toMarkedText::exportData(std::map<QCString,QString> &data,const QCString &prefix)
{
  data[prefix+":Filename"]=Filename;
  data[prefix+":Text"]=text();
  int curline,curcol;
  getCursorPosition (&curline,&curcol);
  data[prefix+":Column"]=QString::number(curcol);
  data[prefix+":Line"]=QString::number(curline);
  if (edited())
    data[prefix+":Edited"]="Yes";
}

void toMarkedText::importData(std::map<QCString,QString> &data,const QCString &prefix)
{
  QString txt=data[prefix+":Text"];
  if (txt!=text())
    setText(txt);
  Filename=data[prefix+":Filename"];
  setCursorPosition(data[prefix+":Line"].toInt(),data[prefix+":Column"].toInt());
  if (data[prefix+":Edited"].isEmpty())
    setEdited(false);
}

static int FindIndex(const QString &str,int line,int col)
{
  int pos=0;
  for (int i=0;i<line;i++) {
    pos=str.find(QString::fromLatin1("\n"),pos);
    if (pos<0)
      return pos;
    pos++;
  }
  return pos+col;
}

void toMarkedText::findPosition(int index,int &line,int &col)
{
  int pos=0;
  for (int i=0;i<numLines();i++) {
    QString str=textLine(i);
    if (str.length()+pos>=(unsigned int)index) {
      line=i;
      col=index-pos;
      return;
    }
    pos+=str.length()+1;
  }
  col=-1;
  line=-1;
  return ;
}

bool toMarkedText::searchNext(toSearchReplace *search)
{
  QString text=toMarkedText::text();

  int col;
  int line;
  cursorPosition(&line,&col);
  int pos=FindIndex(text,line,col);

  int endPos;
  if (search->findString(text,pos,endPos)) {
    int endCol;
    int endLine;
    findPosition(pos,line,col);
    findPosition(endPos,endLine,endCol);
    setCursorPosition(line,col,false);
    setCursorPosition(endLine,endCol,true);
    
    return true;
  }

  return false;
}

void toMarkedText::searchReplace(const QString &newData)
{
  if (!isReadOnly())
    insert(newData);
}

bool toMarkedText::searchCanReplace(bool all)
{
  if (isReadOnly())
    return false;
  if (all||hasMarkedText())
    return true;
  return false;
}
