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

#include <ctype.h>

#include <qpainter.h>
#include <qpalette.h>
#include <qsimplerichtext.h>
#include <qstylesheet.h>
#include <qapplication.h>
#include <qlistbox.h>

#include "tohighlightedtext.h"
#include "toconnection.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"

#include "todefaultkeywords.h"

#include "tohighlightedtext.moc"

toSyntaxAnalyzer::toSyntaxAnalyzer(const char **keywords)
{
  for (int i=0;keywords[i];i++) {
    std::list<const char *> &curKey=Keywords[(unsigned char)char(toupper(*keywords[i]))];
    curKey.insert(curKey.end(),keywords[i]);
  }
  ColorsUpdated=false;
}

toSyntaxAnalyzer::posibleHit::posibleHit(const char *text)
{
  Pos=1;
  Text=text;
}

std::list<toSyntaxAnalyzer::highlightInfo> toSyntaxAnalyzer::analyzeLine(const QString &str)
{
  if (!ColorsUpdated) {
    updateSettings();
    ColorsUpdated=true;
  }
  std::list<highlightInfo> highs;
  std::list<posibleHit> search;

  bool inWord;
  bool wasWord=false;
  int inString=-1;
  QChar endString;

  for (int i=0;i<int(str.length());i++) {
    std::list<posibleHit>::iterator j=search.begin();

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
	  std::list<posibleHit>::iterator k=j;
	  j++;
	  search.erase(k);
	}
      }
      if (isSymbol(str[i]))
	inWord=true;
      else
	inWord=false;

      if (!wasWord&&inWord) {
	std::list<const char *> &curKey=Keywords[(unsigned char)char(str[i].upper())];
	for (std::list<const char *>::iterator j=curKey.begin();
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
  Completion=NULL;
  NoCompletion=KeepCompletion=false;
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

  std::list<toSyntaxAnalyzer::highlightInfo> highs=Analyzer->analyzeLine(str);
  std::list<toSyntaxAnalyzer::highlightInfo>::iterator highPos=highs.begin();

  int posx=hMargin()-1;
  QRect rect;

#if 0
  if (viewWidth()>cellWidth()) {
    setCellWidth(viewWidth());
    return;
  }
#endif

  int height=cellHeight();
  int width=cellWidth();
  QPalette cp=qApp->palette();

  painter->setBrush(cp.active().highlight());

  QColor bkg=Analyzer->getColor(toSyntaxAnalyzer::NormalBkg);

  std::map<int,QString>::iterator err=Errors.find(row);
  if (err!=Errors.end())
    bkg=Analyzer->getColor(toSyntaxAnalyzer::ErrorBkg);
  if (Current==row)
    bkg=Analyzer->getColor(toSyntaxAnalyzer::CurrentBkg);
  int cursorx=posx;
  int curline,curcol;
  getCursorPosition (&curline,&curcol);

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

      if (wasMarked!=marked||col!=wasCol||str[i]=='\t'||(curline==row&&curcol==i)) {
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
	c=nc;
      }
      if (curline==row&&curcol==i)
	cursorx=posx;

      if (c=="\t") {
	int tab=painter->fontMetrics().width("xxxxxxxx");;
	int nx=((posx-hMargin()+1)/tab+1)*tab+hMargin()-1;
	int left=(posx==hMargin()-1?LeftIgnore:posx);
	painter->fillRect(left,0,nx-left,height,marked?painter->brush():bkg);
	posx=nx;
	c="";
      }
    }
    if (posx<width)
      painter->fillRect(posx,0,width-posx,height,bkg);
  } else
    painter->fillRect(LeftIgnore,0,width-LeftIgnore,height,bkg);
  painter->setPen(cp.active().text());

  if (hasFocus()) {
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
      bool check;
      if (LastRow!=curline||LastCol!=curcol) {
	Cursor=0;
	if (Completion&&!KeepCompletion) {
	  delete Completion;
	  Completion=NULL;
	}
	check=!Completion;
      }
      if (Cursor<2) {
	if (!isReadOnly()) {
	  painter->drawLine(cursorx-1,0,cursorx-1,
			    painter->fontMetrics().ascent()+painter->fontMetrics().descent());
	}
	if (Completion) {
	  int x=cursorPoint().x()-xOffset();
	  QString mrk=markedText();
	  if (!mrk.isEmpty()&&(line1!=curline||col1!=curcol))
	    x-=painter->fontMetrics().width(mrk);
	  QPoint p=mapToGlobal(QPoint(x,cursorPoint().y()+cellHeight()-yOffset()));
	  Completion->move(topLevelWidget()->mapFromGlobal(p));
	}
	Cursor++;
      } else
	Cursor=0;
      LastRow=curline;
      LastCol=curcol;
      if (check)
	checkComplete();
    }
  }
}

void toHighlightedText::setErrors(const std::map<int,QString> &errors)
{
  Errors=errors;	

  toStatusMessage(QString::null);
  update();
}

void toHighlightedText::textChanged(void)
{
  int curline,curcol,lines;
  lines=numLines();
  if (lines!=LastLength) {
    if (LastLength>0) {
      getCursorPosition (&curline,&curcol);
      int diff=lines-LastLength;
      std::map<int,QString> newErr;
      for (std::map<int,QString>::iterator i=Errors.begin();i!=Errors.end();i++) {
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
  for (std::map<int,QString>::iterator i=Errors.begin();i!=Errors.end();i++) {
    if ((*i).first>curline) {
      setCursorPosition((*i).first,0);
      break;
    }
  }
}

static QString UpperIdent(const QString &str)
{
  if (str.length()>0&&str[0]=='\"')
    return str;
  else
    return str.upper();
}

bool toHighlightedText::invalidToken(int line,int col)
{
  bool ident=true;
  if (line<0) {
    line=0;
    col=0;
  }
  while(line<numLines()) {
    QString cl=textLine(line);
    while(col<int(cl.length())) {
      QChar c=cl[col];
      if (!toIsIdent(c))
	ident=false;
      if (!ident&&!c.isSpace())
	return c=='.';
      col++;
    }
    line++;
    col=0;
  }
  return false;
}

void toHighlightedText::checkComplete(void)
{
  if (Completion||NoCompletion)
    return;

  int curline,curcol;
  getCursorPosition (&curline,&curcol);

  QString line=textLine(curline);

  if (!isReadOnly()&&curcol>0&&line[curcol-1]=='.') {
    if (toTool::globalConfig(CONF_CODE_COMPLETION,"Yes").isEmpty())
      return;
    int mrklen=0;
    while(curcol+mrklen<int(line.length())&&toIsIdent(line[curcol+mrklen]))
      mrklen++;

    QString name=UpperIdent(toGetToken(this,curline,curcol,false));
    QString owner;
    if (name==".")
      name=UpperIdent(toGetToken(this,curline,curcol,false));

    QString token=UpperIdent(toGetToken(this,curline,curcol,false));
    if (token==".")
      owner=UpperIdent(toGetToken(this,curline,curcol,false));
    else {
      while ((invalidToken(curline,curcol+token.length())||token!=name)&&
	     token!=";"&&!token.isEmpty()) {
	token=UpperIdent(toGetToken(this,curline,curcol,false));
      }

      if (token==";"||token.isEmpty()) {
	getCursorPosition (&curline,&curcol);
	token=UpperIdent(toGetToken(this,curline,curcol));
	while ((invalidToken(curline,curcol)||token!=name)&&token!=";"&&!token.isEmpty())
	  token=UpperIdent(toGetToken(this,curline,curcol));
	UpperIdent(toGetToken(this,curline,curcol,false));
      }
      if (token!=";"&&!token.isEmpty()) {
	token=UpperIdent(toGetToken(this,curline,curcol,false));
	if (token!="TABLE"&&
	    token!="UPDATE"&&
	    token!="FROM"&&
	    token!="INTO"&&
	    toIsIdent(token[0])) {
	  name=token;
	  token=toGetToken(this,curline,curcol,false);
	  if (token==".")
	    owner=UpperIdent(toGetToken(this,curline,curcol,false));
	} else if (token==")") {
	  return;
	}
      }
    }
    if (!owner.isEmpty()) {
      name=owner+"."+name;
    }
    if (!name.isEmpty()) {
      try {
	toConnection &conn=toCurrentConnection(this);
	AllComplete=conn.columns(conn.realName(name));
	if (!toTool::globalConfig(CONF_COMPLETION_SORT,"Yes").isEmpty())
	  AllComplete.sort();
	Completion=new QListBox(topLevelWidget());
	Completion->setFocusPolicy(NoFocus);
	Completion->setFont(font());
	CompleteItem=-1;
	for (std::list<QString>::iterator i=AllComplete.begin();i!=AllComplete.end();i++) {
	  Completion->insertItem(*i);
	}
	QPoint p=mapToGlobal(QPoint(cursorPoint().x()-xOffset(),
				    cursorPoint().y()+cellHeight()-yOffset()));
	Completion->move(topLevelWidget()->mapFromGlobal(p));
	QSize size=Completion->sizeHint();
	size.setWidth(size.width()+20);
	Completion->setFixedSize(size);
	Completion->show();
	connect(Completion,SIGNAL(clicked(QListBoxItem *)),this,SLOT(selectComplete()));

	getCursorPosition(&curline,&curcol);
	KeepCompletion=true;
	if (mrklen>0) {
	  setCursorPosition(curline,curcol+mrklen,false);
	  setCursorPosition(curline,curcol,true);
	}
	KeepCompletion=false;
      } catch(...) {
      }
    }
  }
}

void toHighlightedText::keyPressEvent(QKeyEvent *e)
{
  if (Completion&&Completion->isHidden()) {
    delete Completion;
    Completion=NULL;
  }

  NoCompletion=true;

  if (Completion) {
    if (e->key()==Key_Down||e->key()==Key_Tab||e->key()==Key_Up) {
      if (e->key()==Key_Up) {
	CompleteItem--;
	if (CompleteItem<0)
	  CompleteItem=0;
      } else {
	CompleteItem++;
	if (CompleteItem>=int(Completion->count()))
	  CompleteItem=Completion->count()-1;
      }
      Completion->setSelected(CompleteItem,true);
      if (CompleteItem<Completion->topItem())
	Completion->setTopItem(CompleteItem);
      else if (CompleteItem-Completion->numItemsVisible()+1>=Completion->topItem())
	Completion->setBottomItem(CompleteItem);
      e->accept();
      return;
    } else if (e->key()==Key_Escape) {
      delete Completion;
      Completion=NULL;
      e->accept();
      return;
    } else if (e->key()==Key_Backspace) {
      QString mrk=markedText();
      if (mrk.length()) {
	mrk=mrk.left(mrk.length()-1);

	Completion->clear();
	CompleteItem=-1;
	for (std::list<QString>::iterator i=AllComplete.begin();i!=AllComplete.end();i++) {
	  if ((*i)==mrk.upper()) {
	    insert(mrk,false);
	    return;
	  } else if ((*i).startsWith(mrk.upper()))
	    Completion->insertItem(mrk+(*i).mid(mrk.length()));
	}

	KeepCompletion=true;
	insert(mrk,true);
	KeepCompletion=false;
	e->accept();
	return;
      }
    } else if (e->key()==Key_Return&&CompleteItem>=0) {
      if (CompleteItem>=0)
	insert(Completion->text(CompleteItem),false);
      else {
	QString mrk=markedText();
	insert(mrk,false);
      }
      e->accept();
      return;
    } else {
      QString txt=e->text();
      if (txt.length()&&toIsIdent(txt.at(0))) {
	QString mrk=markedText();
	mrk+=txt;
	Completion->clear();
	CompleteItem=-1;
	for (std::list<QString>::iterator i=AllComplete.begin();i!=AllComplete.end();i++) {
	  if ((*i)==mrk.upper()) {
	    insert(mrk,false);
	    return;
	  } else if ((*i).startsWith(mrk.upper()))
	    Completion->insertItem(mrk+(*i).mid(mrk.length()));
	}

	KeepCompletion=true;
	insert(mrk,true);
	KeepCompletion=false;
	e->accept();
	return;
      }
    }
  }
  toMarkedText::keyPressEvent(e);
  NoCompletion=false;
  checkComplete();
}

void toHighlightedText::selectComplete(void)
{
  if (Completion) {
    QString tmp=Completion->currentText();
    if (!tmp.isEmpty()) {
      KeepCompletion=true;
      insert(tmp,false);
      KeepCompletion=false;
    }
  }
  Completion->hide();
  setFocus();
}

void toHighlightedText::previousError(void)
{
  int curline,curcol;
  getCursorPosition (&curline,&curcol);
  curcol=-1;
  for (std::map<int,QString>::iterator i=Errors.begin();i!=Errors.end();i++) {
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

void toHighlightedText::focusOutEvent(QFocusEvent *e)
{
  delete Completion;
  Completion=NULL;
}

toHighlightedText::~toHighlightedText()
{
  delete Completion;
  Completion=NULL;
}
