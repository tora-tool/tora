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
#include "toconnection.h"
#include "tohighlightedtext.h"
#include "tosqlparse.h"
#include "totool.h"

#include <ctype.h>

#include <qapplication.h>
#include <qlistbox.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qsimplerichtext.h>
#include <qstylesheet.h>
#include <qtimer.h>

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

QColor toSyntaxAnalyzer::getColor(toSyntaxAnalyzer::infoType typ)
{
  if (!ColorsUpdated) {
    updateSettings();
    ColorsUpdated=true;
  }
  return Colors[typ];
}

#define ISIDENT(c) (isalnum(c)||(c)=='_'||(c)=='%'||(c)=='$'||(c)=='#')

std::list<toSyntaxAnalyzer::highlightInfo> toSyntaxAnalyzer::analyzeLine(const QString &str,
									 toSyntaxAnalyzer::infoType in,
									 toSyntaxAnalyzer::infoType &out)
{
  std::list<highlightInfo> highs;
  std::list<posibleHit> search;

  bool inWord;
  bool wasWord=false;
  int multiComment=-1;
  int inString=-1;
  QChar endString;

  if (in==String) {
    inString=0;
    endString='\'';
  } else if (in==Comment) {
    multiComment=0;
  }

  char c;
  char nc=str[0];
  for (int i=0;i<int(str.length());i++) {
    std::list<posibleHit>::iterator j=search.begin();

    c=nc;
    if (int(str.length())>i)
      nc=str[i+1];
    else
      nc=' ';

    bool nextSymbol=ISIDENT(nc);
    if (multiComment>=0) {
      if (c=='*'&&nc=='/') {
	highs.insert(highs.end(),highlightInfo(multiComment,Comment));
	highs.insert(highs.end(),highlightInfo(i+2));
	multiComment=-1;
      }
    } else if (inString>=0) {
      if (c==endString) {
	highs.insert(highs.end(),highlightInfo(inString,String));
	highs.insert(highs.end(),highlightInfo(i+1));
	inString=-1;
      }
    } else if (c=='\''||c=='\"') {
      inString=i;
      endString=str[i];
      search.clear();
      wasWord=false;
    } else if (c=='-'&&nc=='-') {
      highs.insert(highs.end(),highlightInfo(i,Comment));
      highs.insert(highs.end(),highlightInfo(str.length()+1));
      out=Normal;
      return highs;
    } else if (c=='/'&&nc=='*') {
      multiComment=i;
      search.clear();
      wasWord=false;
    } else {
      std::list<posibleHit> newHits;
      while (j!=search.end()) {
	posibleHit &cur=(*j);
	if (cur.Text[cur.Pos]==toupper(c)) {
	  cur.Pos++;
	  if (!cur.Text[cur.Pos]&&!nextSymbol) {
	    newHits.clear();
	    highs.insert(highs.end(),highlightInfo(i-cur.Pos,Keyword));
	    highs.insert(highs.end(),highlightInfo(i+1));
	    break;
	  }
	  newHits.insert(newHits.end(),cur);
	}
	j++;
      }
      search=newHits;
      if (ISIDENT(c))
	inWord=true;
      else
	inWord=false;

      if (!wasWord&&inWord) {
	std::list<const char *> &curKey=Keywords[(unsigned char)char(toupper(c))];
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
    if (endString=='\'') {
      out=String;
      highs.insert(highs.end(),highlightInfo(inString,String));
    } else {
      out=Normal;
      highs.insert(highs.end(),highlightInfo(inString,Error));
    }
    highs.insert(highs.end(),highlightInfo(str.length()+1));
  } else if (multiComment>=0) {
    highs.insert(highs.end(),highlightInfo(multiComment,Comment));
    highs.insert(highs.end(),highlightInfo(str.length()+1));
    out=Comment;
  } else
    out=Normal;

  return highs;
}

static toSyntaxAnalyzer DefaultAnalyzer((const char **)DefaultKeywords);

toSyntaxAnalyzer &toSyntaxAnalyzer::defaultAnalyzer(void)
{
  return DefaultAnalyzer;
}

bool toSyntaxAnalyzer::reservedWord(const QString &str)
{
  if(str.length()==0)
    return false;
  QString t=str.upper();
  std::list<const char *> &curKey=Keywords[(unsigned char)char(str[0])];
  for(std::list<const char *>::iterator i=curKey.begin();i!=curKey.end();i++)
    if (t==(*i))
      return true;
  return false;
}

toHighlightedText::toHighlightedText(QWidget *parent,const char *name)
  : toMarkedText(parent,name),Analyzer(&DefaultAnalyzer)
{
  setFont(toStringToFont(toTool::globalConfig(CONF_CODE,"")));
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
  LineInput.clear();
  toMarkedText::setText(str);

  toSyntaxAnalyzer::infoType typ=toSyntaxAnalyzer::Normal;
  for(int i=0;i<numLines();i++) {
    Analyzer->analyzeLine(textLine(i),typ,typ);
    if (typ!=toSyntaxAnalyzer::Normal)
      LineInput[i]=typ;
  }
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

toSyntaxAnalyzer::infoType toHighlightedText::lineIn(int line)
{
  std::map<int,toSyntaxAnalyzer::infoType>::iterator i=LineInput.find(line);
  if (i==LineInput.end())
    return toSyntaxAnalyzer::Normal;
  else
    return (*i).second;
}

void toHighlightedText::setStatusMessage(void)
{
  int curline,curcol;
  getCursorPosition (&curline,&curcol);
  std::map<int,QString>::iterator err=Errors.find(curline);
  if (err==Errors.end())
    toStatusMessage(QString::null);
  else
    toStatusMessage((*err).second,true);
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

  toSyntaxAnalyzer::infoType out;
  std::list<toSyntaxAnalyzer::highlightInfo> highs=Analyzer->analyzeLine(str,lineIn(row),out);
  std::list<toSyntaxAnalyzer::highlightInfo>::iterator highPos=highs.begin();
  if (lineIn(row+1)!=out) {
    int i=row+1;
    painter->save();
    while(i<numLines()&&out!=lineIn(i)) {
      if (out==toSyntaxAnalyzer::Normal)
	LineInput.erase(LineInput.find(i));
      else
	LineInput[i]=out;
      if (i<=lastRowVisible()) {
	painter->translate(0,cellHeight(i-1));
	painter->setClipping(false);
        paintCell(painter,i,0);
	break;
      }
      Analyzer->analyzeLine(textLine(i),out,out);
      i++;
    }
    painter->restore();
  }

  int posx=hMargin()-1;
  QRect rect;

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
	  rect=painter->boundingRect(0,0,width,height,AlignLeft|AlignTop,c);
	  int left=posx;
	  int cw=rect.right()+1;
	  if (i==int(str.length()))
	    cw+=hMargin()-1;
	  if (i==int(c.length())) {
	    cw+=left;
	    left=LeftIgnore;
	  }
	  if (wasMarked) {
	    if (Completion&&Completion->count()==0) {
	      painter->setBrush(Analyzer->getColor(toSyntaxAnalyzer::ErrorBkg));
	      painter->fillRect(left,0,cw,height,painter->brush());
	      painter->setBrush(cp.active().highlight());
	    } else
	      painter->fillRect(left,0,cw,height,painter->brush());
	    painter->setPen(cp.active().highlightedText());
	  } else {
	    painter->setPen(wasCol);
	    painter->fillRect(left,0,cw,height,bkg);
	  }

	  painter->drawText(posx,0,width-posx,height,AlignLeft|AlignTop,c,c.length(),&rect);
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
	QTimer::singleShot(1,this,SLOT(setStatusMessage(void)));
      else {
	err=Errors.find(LastRow);
	if (err!=Errors.end())
	  QTimer::singleShot(1,this,SLOT(setStatusMessage(void)));
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
	if (Completion&&!KeepCompletion) {
	  int x=cursorPoint().x()-xOffset();
	  QString mrk=markedText();
	  if (!mrk.isEmpty()&&(line1!=curline||col1!=curcol))
	    x-=painter->fontMetrics().width(mrk);
	  QPoint p=mapToGlobal(QPoint(x,cursorPoint().y()+cellHeight()-yOffset()));
	  p=topLevelWidget()->mapFromGlobal(p);
	  if (Completion->pos()!=p)
	    Completion->move(p);
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

void toHighlightedText::startComplete(std::list<QString> &completes)
{
  delete Completion;

  AllComplete=completes;
  if (!toTool::globalConfig(CONF_COMPLETION_SORT,"Yes").isEmpty())
    AllComplete.sort();
  Completion=new QListBox(topLevelWidget());
  Completion->setFocusPolicy(NoFocus);
  Completion->setFont(font());
  CompleteItem=-1;

  QPoint p=mapToGlobal(QPoint(cursorPoint().x()-xOffset(),
			      cursorPoint().y()+cellHeight()-yOffset()));
  p=topLevelWidget()->mapFromGlobal(p);
  connect(Completion,SIGNAL(clicked(QListBoxItem *)),this,SLOT(selectComplete()));

  int curline,curcol;
  getCursorPosition(&curline,&curcol);

  QString beg=markedText();

  if (beg.isEmpty()) {
    QString line=textLine(curline);
    int mrklen=0;
    while(curcol+mrklen<int(line.length())&&toIsIdent(line[curcol+mrklen]))
      mrklen++;

    KeepCompletion=true;
    if (mrklen>0) {
      setCursorPosition(curline,curcol+mrklen,false);
      setCursorPosition(curline,curcol,true);
    }
    KeepCompletion=false;
  }

  for (std::list<QString>::iterator i=AllComplete.begin();i!=AllComplete.end();i++) {
    if (beg.length()) {
      if ((*i).upper().startsWith(beg.upper()))
	Completion->insertItem(beg+(*i).mid(beg.length()));
    } else
      Completion->insertItem(*i);
  }
  QSize size=Completion->sizeHint();
  size.setWidth(size.width()+20);
  Completion->setGeometry(p.x(),p.y(),size.width(),size.height());
  Completion->show();
}

void toHighlightedText::checkComplete(void)
{
  if (Completion||NoCompletion)
    return;

  int curline,curcol;
  getCursorPosition (&curline,&curcol);

  QString line=textLine(curline);

  if (!isReadOnly()&&curcol>0&&line[curcol-1]=='.') {
    QString mrk=markedText();
    if (!mrk.isEmpty())
      return;
    if (toTool::globalConfig(CONF_CODE_COMPLETION,"Yes").isEmpty())
      return;

    QString name=toSQLParse::getToken(this,curline,curcol,false);
    QString owner;
    if (name==".")
      name=toSQLParse::getToken(this,curline,curcol,false);

    QString token=toSQLParse::getToken(this,curline,curcol,false);
    if (token==".")
      owner=toSQLParse::getToken(this,curline,curcol,false);
    else {
      QString cmp=UpperIdent(name);
      while ((invalidToken(curline,curcol+token.length())||UpperIdent(token)!=cmp)&&
	     token!=";"&&!token.isEmpty()) {
	token=toSQLParse::getToken(this,curline,curcol,false);
      }

      if (token==";"||token.isEmpty()) {
	getCursorPosition (&curline,&curcol);
	token=toSQLParse::getToken(this,curline,curcol);
	while ((invalidToken(curline,curcol)||UpperIdent(token)!=cmp)&&
	       token!=";"&&!token.isEmpty())
	  token=toSQLParse::getToken(this,curline,curcol);
	toSQLParse::getToken(this,curline,curcol,false);
      }
      if (token!=";"&&!token.isEmpty()) {
	token=toSQLParse::getToken(this,curline,curcol,false);
	if (token!="TABLE"&&
	    token!="UPDATE"&&
	    token!="FROM"&&
	    token!="INTO"&&
	    (toIsIdent(token[0])||token[0]=='\"')) {
	  name=token;
	  token=toSQLParse::getToken(this,curline,curcol,false);
	  if (token==".")
	    owner=toSQLParse::getToken(this,curline,curcol,false);
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
	toQDescList &desc=conn.columns(conn.realName(name,false));
	std::list<QString> complete;
	for (toQDescList::iterator i=desc.begin();
	     i!=desc.end();i++) {
	  QString t=conn.quote((*i).Name);
	  if (!(*i).Comment.isEmpty()) {
	    t+=" - ";
	    t+=(*i).Comment;
	  }
	  complete.insert(complete.end(),t);
	}

	startComplete(complete);
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
    if ((e->key()==Key_Down||e->key()==Key_Tab||e->key()==Key_Up)&&
	e->state()==0) {
      bool passon=false;
      if (e->key()==Key_Up) {
	CompleteItem--;
	if (CompleteItem<0) {
	  passon=true;
	  CompleteItem=0;
	}
      } else {
	CompleteItem++;
	if (CompleteItem>=int(Completion->count())) {
	  CompleteItem=Completion->count()-1;
	  passon=true;
	}
      }
      if (!passon) {
	Completion->setSelected(CompleteItem,true);
	if (CompleteItem<Completion->topItem())
	  Completion->setTopItem(CompleteItem);
	else if (CompleteItem-Completion->numItemsVisible()+1>=Completion->topItem())
	  Completion->setBottomItem(CompleteItem);
	e->accept();
	return;
      }
    } else if (e->key()==Key_Right&&e->state()==0) {
      int curline,curcol;
      getCursorPosition(&curline,&curcol);
      int line1,col1,line2,col2;
      if (getMarkedRegion(&line1,&col1,&line2,&col2)&&
	  line1==curline&&col1==curcol) {
	e->accept();
	KeepCompletion=true;
	setCursorPosition(line1,col1);
	setCursorPosition(line2,col2,true);
	KeepCompletion=false;
	return;
      }
    } else if (e->key()==Key_Escape&&e->state()==0) {
      delete Completion;
      Completion=NULL;
      e->accept();
      return;
    } else if (e->key()==Key_Backspace&&e->state()==0) {
      QString mrk=markedText();
      if (mrk.length()) {
	mrk=mrk.left(mrk.length()-1);

	Completion->clear();
	CompleteItem=-1;
	for (std::list<QString>::iterator i=AllComplete.begin();i!=AllComplete.end();i++) {
	  if ((*i).upper().startsWith(mrk.upper()))
	    Completion->insertItem(mrk+(*i).mid(mrk.length()));
	}
	QSize size=Completion->sizeHint();
	size.setWidth(size.width()+20);
	Completion->resize(size);

	KeepCompletion=true;
	insert(mrk,true);
	KeepCompletion=false;
	e->accept();
	return;
      }
    } else if (e->key()==Key_Return&&e->state()==0&&CompleteItem>=0) {
      if (CompleteItem>=0) {
	QString tmp=Completion->text(CompleteItem);
	unsigned int pos=0;
	while(pos<tmp.length()&&toIsIdent(tmp.at(pos))||tmp.at(pos)=='.')
	  pos++;
	tmp=tmp.left(pos);
	insert(tmp,false);
      } else {
	int curline,curcol;
	getCursorPosition(&curline,&curcol);
	setCursorPosition(curline,curcol);
      }
      e->accept();
      return;
    } else {
      QString txt=e->text();
      if (txt.length()) {
	if (toIsIdent(txt.at(0))) {
	  QString mrk;
	  bool passon=false;
	  {
	    int curline,curcol;
	    getCursorPosition(&curline,&curcol);
	    int line1,col1,line2,col2;
	    if (getMarkedRegion(&line1,&col1,&line2,&col2)) {
	      if (line2==curline&&col2==curcol)
		mrk=markedText();
	      else {
		setCursorPosition(curline,curcol);
		passon=true;
	      }
	    }
	  }
	  
	  if (!passon) {
	    mrk+=txt;
	    Completion->clear();
	    CompleteItem=-1;
	    for (std::list<QString>::iterator i=AllComplete.begin();i!=AllComplete.end();i++) {
	      if ((*i).upper().startsWith(mrk.upper()))
		Completion->insertItem(mrk+(*i).mid(mrk.length()));
	    }
	    QSize size=Completion->sizeHint();
	    size.setWidth(size.width()+20);
	    Completion->resize(size);

	    KeepCompletion=true;
	    insert(mrk,true);
	    KeepCompletion=false;
	    e->accept();
	    return;
	  }
	} else {
	  int curline,curcol;
	  getCursorPosition(&curline,&curcol);
	  setCursorPosition(curline,curcol);
	}
      }
    }
  }
  if (e->state()==ControlButton&&e->key()==Key_T) {
    try {
      toConnection &conn=toCurrentConnection(this);
      std::list<toConnection::objectName> &objects=conn.objects(false);
      std::map<QString,toConnection::objectName> &synonyms=conn.synonyms(false);

      QString table;
      QString owner;
      tableAtCursor(owner,table,true);

      std::list<QString> complete;

      if (owner.isEmpty()) {
	for(std::map<QString,toConnection::objectName>::iterator i=synonyms.begin();
	    i!=synonyms.end();i++) {
	  if ((*i).second.Type=="TABLE"||(*i).second.Type=="VIEW") {
	    QString add=conn.quote((*i).first);
	    if(!(*i).second.Comment.isEmpty()) {
	      add+=" - ";
	      add+=(*i).second.Comment;
	    }
	    complete.insert(complete.end(),add);
	  }
	}
      }

      for(std::list<toConnection::objectName>::iterator i=objects.begin();i!=objects.end();i++) {
	if ((*i).Type=="TABLE"||(*i).Type=="VIEW") {
	  if (owner.isEmpty()) {
	    if ((*i).Owner.upper()==conn.user().upper()) {
	      if (synonyms.find((*i).Owner)==synonyms.end()) {
		QString add=conn.quote((*i).Name);
		if(!(*i).Comment.isEmpty()) {
		  add+=" - ";
		  add+=(*i).Comment;
		}
		complete.insert(complete.end(),add);
	      }
	    }
	  } else {
	    QString add=conn.quote((*i).Owner);
	    if (!add.isEmpty())
	      add+=".";
	    add+=conn.quote((*i).Name);
	    if(!(*i).Comment.isEmpty()) {
	      add+=" - ";
	      add+=(*i).Comment;
	    }
	    complete.insert(complete.end(),add);
	  }
	}
      }
      startComplete(complete);
    } catch (...) {
    }
    
    e->accept();
    return;
  }

  toMarkedText::keyPressEvent(e);
  NoCompletion=false;
  checkComplete();
}

void toHighlightedText::selectComplete(void)
{
  if (Completion) {
    QString tmp=Completion->currentText();
    unsigned int pos=0;
    while(pos<tmp.length()&&toIsIdent(tmp.at(pos)))
      pos++;
    tmp=tmp.left(pos);
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
  int curline,curcol;
  getCursorPosition (&curline,&curcol);
  std::map<int,QString>::iterator err=Errors.find(curline);
  if (err==Errors.end())
    toStatusMessage(QString::null);

  delete Completion;
  Completion=NULL;
}

void toHighlightedText::tableAtCursor(QString &owner,QString &table,bool mark)
{
  try {
    toConnection &conn=toCurrentConnection(this);
    int curline,curcol;
    getCursorPosition (&curline,&curcol);

    QString token=textLine(curline);
    if (curcol>0&&toIsIdent(token[curcol-1]))
      token=toSQLParse::getToken(this,curline,curcol,false);
    else
      token=QString::null;

    int lastline=curline;
    int lastcol=curcol;

    token=toSQLParse::getToken(this,curline,curcol,false);
    if (token==".") {
      lastline=curline;
      lastcol=curcol;
      owner=conn.unQuote(toSQLParse::getToken(this,curline,curcol,false));
      toSQLParse::getToken(this,lastline,lastcol,true);
      table+=conn.unQuote(toSQLParse::getToken(this,lastline,lastcol,true));
    } else {
      curline=lastline;
      curcol=lastcol;
      owner=conn.unQuote(toSQLParse::getToken(this,lastline,lastcol,true));
      int tmplastline=lastline;
      int tmplastcol=lastcol;
      token=toSQLParse::getToken(this,lastline,lastcol,true);
      if (token==".")
	table=conn.unQuote(toSQLParse::getToken(this,lastline,lastcol,true));
      else {
	lastline=tmplastline;
	lastcol=tmplastcol;
	table=owner;
	owner=QString::null;
      }
    }
    if (mark) {
      setCursorPosition(curline,curcol,false);
      if (lastline>=numLines()) {
	lastline=numLines()-1;
	lastcol=textLine(lastline).length();
      }
      setCursorPosition(lastline,lastcol,true);
    }
  } catch(...) {
  }
}

void toHighlightedText::paintEvent(QPaintEvent *pe)
{
  toMarkedText::paintEvent(pe);

  if (Highlight) {
    if (cellWidth()<viewWidth()) {
      QPainter painter(this);
      QColor bkg=Analyzer->getColor(toSyntaxAnalyzer::ErrorBkg);
      int lasty=minViewY();
      int ypos=0;
      int x=cellWidth()+minViewX();
      int width=viewWidth()+minViewX()-x;
      QRect re;
      QRegion region=pe->region();
      for(std::map<int,QString>::iterator i=Errors.begin();i!=Errors.end();i++) {
	if (rowYPos((*i).first,&ypos)) {
	  re=QRect(x,ypos,width,cellHeight());
	  if (region.contains(re))
	    painter.fillRect(re,bkg);
	  lasty=ypos+cellHeight();
	}
      }
      if (rowYPos(Current,&ypos)) {
	re=QRect(x,ypos,width,cellHeight());
	if (region.contains(re))
	  painter.fillRect(re,Analyzer->getColor(toSyntaxAnalyzer::CurrentBkg));
      }
    }
  }
}
