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
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include <ctype.h>
#include <stdio.h>

#include <qpainter.h>
#include <qpalette.h>
#include <qsimplerichtext.h>
#include <qstylesheet.h>
#include <qapplication.h>

#include "tohighlightedtext.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"

#include "todefaultkeywords.h"

#include "tohighlightedtext.moc"

toSyntaxAnalyzer::toSyntaxAnalyzer(const char **keywords)
{
  for (int i=0;keywords[i];i++) {
    list<const char *> &curKey=Keywords[(unsigned char)char(toupper(*keywords[i]))];
    curKey.insert(curKey.end(),keywords[i]);
  }
}

toSyntaxAnalyzer::posibleHit::posibleHit(const char *text)
{
  Pos=1;
  Text=text;
}

list<toSyntaxAnalyzer::highlightInfo> toSyntaxAnalyzer::analyzeLine(const QString &str)
{
  list<highlightInfo> highs;
  list<posibleHit> search;

  bool inWord;
  bool wasWord=false;
  int inString=-1;
  QChar endString;

  for (int i=0;i<int(str.length());i++) {
    list<posibleHit>::iterator j=search.begin();

    bool nextSymbol=((int(str.length())!=i+1)&&isSymbol(str[i+1]));
    if (inString>=0) {
      if (str[i]==endString) {
	highs.insert(highs.end(),highlightInfo(inString,String));
	highs.insert(highs.end(),highlightInfo(i+1));
	inString=-1;
      }
    } else if (str[i]=='\''||str[i]=='\"') {
      inString=i;
      endString=str[i];
      search.clear();
      wasWord=false;
    } else if (str[i]=='-'&&str[i+1]=='-') {
      highs.insert(highs.end(),highlightInfo(i,Comment));
      highs.insert(highs.end(),highlightInfo(str.length()+1));
      return highs;
    } else {
      while (j!=search.end()) {
	posibleHit &cur=(*j);
	if (cur.Text[cur.Pos]==str[i].upper()) {
	  cur.Pos++;
	  if (!cur.Text[cur.Pos]&&!nextSymbol) {
	    search.clear();
	    highs.insert(highs.end(),highlightInfo(i-cur.Pos,Keyword));
	    highs.insert(highs.end(),highlightInfo(i+1));
	    break;
	  }
	  j++;
	} else {
	  list<posibleHit>::iterator k=j;
	  j++;
	  search.erase(k);
	}
      }
      if (isSymbol(str[i]))
	inWord=true;
      else
	inWord=false;

      if (!wasWord&&inWord) {
	list<const char *> &curKey=Keywords[(unsigned char)char(str[i].upper())];
	for (list<const char *>::iterator j=curKey.begin();
	     j!=curKey.end();j++) {
	  if (strlen(*j)==1) {
	    if (!nextSymbol) {
	      highs.insert(highs.end(),highlightInfo(i,Keyword));
	      highs.insert(highs.end(),highlightInfo(i));
	    }
	  } else
	    search.insert(search.end(),posibleHit(*j));
	}
      }
      wasWord=inWord;
    }
  }
  if (inString>=0) {
    highs.insert(highs.end(),highlightInfo(inString,Error));
    highs.insert(highs.end(),highlightInfo(str.length()+1));
  }

  return highs;
}

static toSyntaxAnalyzer DefaultAnalyzer((const char **)DefaultKeywords);

toSyntaxAnalyzer &toDefaultAnalyzer(void)
{
  return DefaultAnalyzer;
}

toHighlightedText::toHighlightedText(QWidget *parent,const char *name)
  : toMarkedText(parent,name),Analyzer(&DefaultAnalyzer)
{
  Current=LastCol=LastRow=-1;
  Highlight=!toTool::globalConfig(CONF_HIGHLIGHT,"Yes").isEmpty();
  KeywordUpper=!toTool::globalConfig(CONF_KEYWORD_UPPER,"").isEmpty();
  LeftIgnore=0;
  connect(this,SIGNAL(textChanged(void)),this,SLOT(textChanged(void)));
  LastLength=0;
}

void toHighlightedText::setText(const QString &str)
{
  Errors.clear();
  Current=-1;
  toMarkedText::setText(str);
}

void toHighlightedText::setCurrent(int line)
{
  int lCur=Current;
  Current=line;
  if (lCur>=0)
    updateCell(lCur,0,false);
  if (line>=0) {
    setCursorPosition(line,0);
    updateCell(line,0,false);
  }
}

void toHighlightedText::paintCell(QPainter *painter,int row,int col)
{
  if (!Highlight) {
    toMarkedText::paintCell(painter,row,col);
    return;
  }

  QString str=textLine(row);

  int line1,col1,line2,col2;
  if (!getMarkedRegion(&line1,&col1,&line2,&col2)) {
    line2=line1=-1;
    col2=col1=-1;
  }

  list<toSyntaxAnalyzer::highlightInfo> highs=Analyzer->analyzeLine(str);
  list<toSyntaxAnalyzer::highlightInfo>::iterator highPos=highs.begin();

  int posx=hMargin()-1;
  QRect rect;

  if (viewWidth()>cellWidth()) {
    setCellWidth(viewWidth());
    return;
  }

  int height=cellHeight();
  int width=cellWidth();
  QPalette cp=qApp->palette();

  painter->setBrush(cp.active().highlight());

  QColor bkg=Analyzer->getColor(toSyntaxAnalyzer::NormalBkg);

  map<int,QString>::iterator err=Errors.find(row);
  if (err!=Errors.end())
    bkg=Analyzer->getColor(toSyntaxAnalyzer::ErrorBkg);
  if (Current==row)
    bkg=Analyzer->getColor(toSyntaxAnalyzer::CurrentBkg);

  if (!str.isEmpty()) {
    bool marked;
    QColor col=Analyzer->getColor(toSyntaxAnalyzer::Normal);
    bool upper=false;

    bool wasMarked=false;
    QColor wasCol;

    QString c;

    for (int i=0;i<=int(str.length())&&posx<width;i++) {

      if (i==int(str.length())) {
	marked=!wasMarked;
      } else {
	marked=false;
	if (row==line1&&i>=col1)
	  marked=true;
	else if (row>line1&&row<=line2)
	  marked=true;
	if (row==line2&&i>=col2)
	  marked=false;
      }

      while (highPos!=highs.end()&&(*highPos).Start<=i) {
	col=Analyzer->getColor((*highPos).Type);
	if ((*highPos).Type==toSyntaxAnalyzer::Keyword&&KeywordUpper)
	  upper=true;
	else
	  upper=false;
	highPos++;
      }

      if (i<int(str.length())) {
	if (upper)
	  c.append(str[i].upper());
	else
	  c.append(str[i]);
      }
      if (i==0) {
	wasCol=col;
	wasMarked=marked;
      }

      if (wasMarked!=marked||col!=wasCol||str[i]=='\t') {
	QChar nc;
	if (c.length()>0&&i<int(str.length())) {
	  nc=c.at(c.length()-1);
	  c.truncate(c.length()-1);
	}

	if (c.length()>0) {
	  rect=painter->boundingRect(0,0,width,height,AlignLeft|AlignTop|ExpandTabs,c);
	  int left=posx;
	  int cw=rect.right()+1;
	  if (i==int(str.length()))
	    cw+=hMargin()-1;
	  if (i==int(c.length())) {
	    cw+=left;
	    left=LeftIgnore;
	  }
	  if (wasMarked) {
	    painter->fillRect(left,0,cw,height,painter->brush());
	    painter->setPen(cp.active().highlightedText());
	  } else {
	    painter->setPen(wasCol);
	    painter->fillRect(left,0,cw,height,bkg);
	  }

	  painter->drawText(posx,0,width-posx,height,AlignLeft|AlignTop|ExpandTabs,c,c.length(),&rect);
	  posx=rect.right()+1;
	}
	wasCol=col;
	wasMarked=marked;
	if (nc=='\t') {
	  int tab=painter->fontMetrics().width("xxxxxxxx");;
	  int nx=((posx-hMargin()+1)/tab+1)*tab+hMargin()-1;
	  int left=(posx==hMargin()-1?LeftIgnore:posx);
	  painter->fillRect(left,0,nx-left,height,marked?painter->brush():bkg);
	  posx=nx;
	  c="";
	} else
	  c=nc;
      }
    }
    if (posx<width)
      painter->fillRect(posx,0,width-posx,height,bkg);
  } else
      painter->fillRect(LeftIgnore,0,width-LeftIgnore,height,bkg);
  painter->setPen(cp.active().text());

  if (hasFocus()) {
    int curline,curcol;
    getCursorPosition (&curline,&curcol);
    if (row==curline) {
      if (err!=Errors.end())
	toStatusMessage((*err).second,true);
      else {
	err=Errors.find(LastRow);
	if (err!=Errors.end())
	  toStatusMessage("");
      }
    }

    if (row==curline) {
      if (LastRow!=curline||LastCol!=curcol)
	Cursor=true;
      if (Cursor) {
	LastRow=curline;
	LastCol=curcol;
	if (!isReadOnly()) {
	  QPoint pos=cursorPoint();
	  painter->drawLine(pos.x(),0,pos.x(),
			    painter->fontMetrics().ascent()+painter->fontMetrics().descent());
	}
	Cursor=false;
      } else
	Cursor=true;
    }
  }
}

void toHighlightedText::textChanged(void)
{
  int curline,curcol,lines;
  lines=numLines();
  if (lines!=LastLength) {
    if (LastLength>0) {
      getCursorPosition (&curline,&curcol);
      int diff=lines-LastLength;
      map<int,QString> newErr;
      for (map<int,QString>::iterator i=Errors.begin();i!=Errors.end();i++) {
	int newLine=convertLine((*i).first,curline-diff,diff);
	if (newLine>=0)
	  newErr[newLine]=(*i).second;
      }
      if (Current>=0)
	Current=convertLine(Current,curline-diff,diff);
      Errors=newErr;
      emit insertedLines(curline-diff,diff);
      update();
    }
    LastLength=lines;
  }
}

int toHighlightedText::convertLine(int line,int start,int diff)
{
  if (diff>0) {
    if (line>start)
      return line+diff;
    else
      return line;
  } else if (line<start+diff)
    return line;
  else if (line>=start)
    return line+diff;
  return -1;
}

void toHighlightedText::nextError(void)
{
  int curline,curcol;
  getCursorPosition (&curline,&curcol);
  for (map<int,QString>::iterator i=Errors.begin();i!=Errors.end();i++) {
    if ((*i).first>curline) {
      setCursorPosition((*i).first,0);
      break;
    }
  }
}

void toHighlightedText::previousError(void)
{
  int curline,curcol;
  getCursorPosition (&curline,&curcol);
  curcol=-1;
  for (map<int,QString>::iterator i=Errors.begin();i!=Errors.end();i++) {
    if ((*i).first>=curline) {
      if (curcol<0)
	curcol=(*i).first;
      break;
    }
    curcol=(*i).first;
  }
  if (curcol>=0)
    setCursorPosition(curcol,0);
}
