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

#include "tosqlparse.h"
#include "tohighlightedtext.h"

#ifdef TOPARSE_DEBUG

#include <stdio.h>

bool toMonolithic(void)
{
  return false;
}

void printStatement(toSQLParse::statement &stat,int level)
{
  for (int i=0;i<level;i++)
    printf(" ");

  switch(stat.Type) {
  case toSQLParse::statement::Block:
    printf("Block:");
    break;
  case toSQLParse::statement::Statement:
    printf("Statement:");
    break;
  case toSQLParse::statement::List:
    printf("List:");
    break;
  case toSQLParse::statement::Parameter:
    printf("Parameter:");
    break;
  case toSQLParse::statement::Token:
    printf("Token:");
    break;
  case toSQLParse::statement::Raw:
    printf("Raw:");
    break;
  }
  printf("%s\n",(const char *)stat.String);
  if (!stat.Comment.isNull()) {
    for (int i=0;i<level;i++)
      printf(" ");
    printf("Comment:%s\n",(const char *)stat.Comment);
  }
  for(std::list<toSQLParse::statement>::iterator i=stat.SubTokens.begin();
      i!=stat.SubTokens.end();
      i++)
    printStatement(*i,level+1);
}

int main(int argc,char **argv) {
  QString res="
CREATE OR REPLACE procedure spTuxGetAccData (oRet                        OUT  NUMBER,
					     oNumSwt                     OUT  NUMBER)
AS
  vYear  CHAR(4);
BEGIN

    DECLARE
      oTrdStt NUMBER;
    BEGIN
      oTrdStt := 0;
    END;

    EXCEPTION
        WHEN VALUE_ERROR THEN
	    oRet := 3;
	WHEN NO_DATA_FOUND THEN
	    oRet := 2;
	WHEN OTHERS THEN
	    oRet := 1;
END;
";

#if 0
  QApplication test(argc,argv);
  toMarkedText text(NULL);
  text.setText(res);

  int pos;
  int line;

  printf("\nForward string without comments\n");
  pos=0;
  for (QString ret=toSQLParse::getToken(res,pos,true,false);
       !ret.isNull();
       ret=toSQLParse::getToken(res,pos,true,false)) {
    printf("Token:%s\n",(const char *)ret);
  }

  printf("\nForward editor without comments\n");
  line=0;
  pos=0;
  for (QString ret=toSQLParse::getToken(&text,line,pos,true,false);
       !ret.isNull();
       ret=toSQLParse::getToken(&text,line,pos,true,false)) {
    printf("Token:%s\n",(const char *)ret);
  }

  printf("\nForward string with comments\n");
  pos=0;
  for (QString ret=toSQLParse::getToken(res,pos,true,true);
       !ret.isNull();
       ret=toSQLParse::getToken(res,pos,true,true)) {
    printf("Token:%s\n",(const char *)ret);
  }

  printf("\nForward editor with comments\n");
  line=0;
  pos=0;
  for (QString ret=toSQLParse::getToken(&text,line,pos,true,true);
       !ret.isNull();
       ret=toSQLParse::getToken(&text,line,pos,true,true)) {
    printf("Token:%s\n",(const char *)ret);
  }

  printf("\nBackward string without comments (Shouldn't work with --)\n");
  pos=res.length();
  for (QString ret=toSQLParse::getToken(res,pos,false,false);
       !ret.isNull();
       ret=toSQLParse::getToken(res,pos,false,false)) {
    printf("Token:%s\n",(const char *)ret);
  }

  printf("\nBackward editor without comments\n");
  line=text.numLines()-1;
  pos=text.textLine(line).length();
  for (QString ret=toSQLParse::getToken(&text,line,pos,false,false);
       !ret.isNull();
       ret=toSQLParse::getToken(&text,line,pos,false,false)) {
    printf("Token:%s\n",(const char *)ret);
  }

  printf("\nBackward string with comments (Shouldn't work with --)\n");
  pos=res.length();
  for (QString ret=toSQLParse::getToken(res,pos,false,true);
       !ret.isNull();
       ret=toSQLParse::getToken(res,pos,false,true)) {
    printf("Token:%s\n",(const char *)ret);
  }

  printf("\nBackward editor with comments\n");
  line=text.numLines()-1;
  pos=text.textLine(line).length();
  for (QString ret=toSQLParse::getToken(&text,line,pos,false,true);
       !ret.isNull();
       ret=toSQLParse::getToken(&text,line,pos,false,true)) {
    printf("Token:%s\n",(const char *)ret);
  }

#endif
  std::list<toSQLParse::statement> stat=toSQLParse::parse(res);

  for(std::list<toSQLParse::statement>::iterator i=stat.begin();i!=stat.end();i++) {
    printStatement(*i,1);
  }

  return 0;
}

#endif

static char *Operators[]={":=",
			  "=>",
			  "||",
			  "**",
			  "<<",
			  ">>",
			  "..",
			  "<>",
			  "!=",
			  "~=",
			  "^=",
			  "<=",
			  ">=",
			  NULL};

QString toSQLParse::getToken(const QString &line,int &pos,bool forward,bool comments)
{
  QChar c;
  QChar nc;
  QChar endString;

  enum {
    space,
    any,
    identifier,
    string,
    comment
  } state=space;

  QString token;

  int inc=forward?1:-1;

  while((forward&&pos<int(line.length()))||(!forward&&pos>=1)) {
    if (!forward)
      pos--;
    c=line[pos];
    if ((forward&&pos<int(line.length()-1))||(!forward&&pos>0))
      nc=line[pos+inc];
    else
      nc='\n';
    if (state==space) {
      if(forward&&c=='-'&&nc=='-') {
	int spos=pos;
	if (forward)
	  for (pos++;pos<int(line.length())&&line[pos]!='\n';pos++)
	    ;
	if (comments)
	  return line.mid(spos,pos-spos);
	continue;
      }
      if(c=='/'&&nc=='*')
	state=comment;
      else if(!c.isSpace())
	state=any;
    }
    
    if (forward)
      pos++;
    
    if (state!=space) {
      if(forward)
	token+=c;
      else
	token.prepend(c);
      switch(state) {
      case comment:
	if(c=='*'&&nc=='/') {
	  if(forward)
	    token+=nc;
	  else
	    token.prepend(nc);
	  pos+=inc;
	  if (comments)
	    return token;
	  else {
	    state=space;
	    token=QString::null;
	  }
	}
	break;
      case space:
	break;
      case any:
	if (toIsIdent(c)) {
	  if (!toIsIdent(nc))
	    return token;
	  state=identifier;
	} else if (c=='\''||c=='\"') {
	  endString=c;
	  state=string;
	} else {
	  for (int i=0;Operators[i];i++) {
	    if ((forward&&c==Operators[i][0]&&nc==Operators[i][1])||
		(!forward&&nc==Operators[i][0]&&c==Operators[i][1])) {
	      if(forward)
		token+=nc;
	      else
		token.prepend(nc);
	      pos+=inc;
	      break;
	    }
	  }
	  return token;
	}
	break;
      case identifier:
	if (!toIsIdent(nc))
	  return token;
	break;
      case string:
	if (c==endString)
	  return token;
	break;
      }
    }
  }
  return token;
}

QString toSQLParse::getToken(toMarkedText *text,int &curLine,int &pos,bool forward,
			     bool comments)
{
  bool first=true;
  while(curLine<int(text->numLines())&&curLine>=0) {
    QString line=text->textLine(curLine);
    if (!first) {
      if (forward)
	pos=0;
      else
	pos=line.length();
    }
    QString ret=getToken(line,pos,forward,true);
    if (!ret.isNull()) {
      if (forward) {
	QString end;
	if (ret.startsWith("/*")&&
	    (ret[ret.length()-2]!='*'||
	     ret[ret.length()-1]!='/')) {
	  end="*/";
	} else if (ret.startsWith("'")&&(ret.length()<2||ret[ret.length()-1]!='\'')) {
	  end="'";
	} else if (ret.startsWith("\"")&&(ret.length()<2||ret[ret.length()-1]!='\"')) {
	  end="\"";
	}
	if (!end.isNull()) {
	  for (curLine++;
	       curLine<int(text->numLines())&&(pos=text->textLine(curLine).find(end))<0;
	       curLine++)
	    ret+="\n"+text->textLine(curLine);
	  if (curLine<int(text->numLines())) {
	    ret+="\n"+text->textLine(curLine).mid(0,pos+2);
	    pos+=2;
	  }
	}
      } else {
	QString end;
	if (ret.length()>=2&&
	    ret[ret.length()-2]=='*'&&
	    ret[ret.length()-1]=='/'&&
	    !ret.startsWith("/*")) {
	  end="/*";
	} else if ((ret.length()>=1&&ret[ret.length()-1]=='\'')&&
		   (ret.length()<2||ret[0]!='\'')) {
	  end="\'";
	} else if ((ret.length()>=1&&ret[ret.length()-1]=='\"')&&
		   (ret.length()<2||ret[0]!='\"')) {
	  end="\"";
	}
	if (!end.isNull()) {
	  for (curLine--;
	       curLine>=0&&(pos=text->textLine(curLine).findRev(end))<0;
	       curLine--)
	    ret.prepend(text->textLine(curLine)+"\n");
	  if (curLine>=0) {
	    QString str=text->textLine(curLine);
	    ret.prepend(str.mid(pos,str.length()-pos)+"\n");
	  }
	}
      }
      if (comments||(!ret.startsWith("/*")&&!ret.startsWith("--")))
	return ret;
      else {
	first=true;
	continue;
      }
    }
    curLine+=(forward?1:-1);
    first=false;
  }
  return QString::null;
}

toSQLParse::statement toSQLParse::parseStatement(const QString &str,int &pos,bool declare)
{
  statement ret(statement::Statement);

  toSyntaxAnalyzer &syntax=toSyntaxAnalyzer::defaultAnalyzer();

  QString first=getToken(str,pos,true,true);

  for (QString token=first;
       !token.isNull();
       token=getToken(str,pos,true,true)) {
    QString upp=token.upper();
    printf("Token:%s\n",(const char *)token);
    if ((first=="IF"&&upp=="THEN")||
	upp=="LOOP"||
	upp=="DECLARE"||
	upp=="AS"||
	upp=="IS"||
	(!declare&&upp=="BEGIN")) {
      statement blk(statement::Block);
      ret.SubTokens.insert(ret.SubTokens.end(),statement(statement::Token,token));
      blk.SubTokens.insert(blk.SubTokens.end(),ret);
      statement cur(statement::Statement);
      bool dcl=(upp=="DECLARE"||upp=="IS"||upp=="AS");
      do {
	cur=parseStatement(str,pos,dcl);
	if (cur.Type==statement::List)
	  toStatusMessage("Unbalanced parenthesis (Too many ')')");
	blk.SubTokens.insert(blk.SubTokens.end(),cur);
      } while(cur.SubTokens.begin()!=cur.SubTokens.end()&&
	      (*cur.SubTokens.begin()).String.upper()!="END");
      return blk;
    } else if (upp=="THEN"||upp=="BEGIN"||upp=="EXCEPTION") {
      ret.SubTokens.insert(ret.SubTokens.end(),statement(statement::Parameter,token));
      return ret;
    } else if (upp==","||syntax.reservedWord(upp)) {
      ret.SubTokens.insert(ret.SubTokens.end(),statement(statement::Parameter,token));
    } else if (upp=="(") {
      ret.SubTokens.insert(ret.SubTokens.end(),statement(statement::Token,token));
      statement lst=parseStatement(str,pos,false);
      statement t=toPop(lst.SubTokens);
      ret.SubTokens.insert(ret.SubTokens.end(),lst);
      ret.SubTokens.insert(ret.SubTokens.end(),t);
      if (lst.Type!=statement::List) {
	toStatusMessage("Unbalanced parenthesis (Too many '(')");
	return ret;
      }
    } else if (upp==")") {
      ret.Type=statement::List;
      ret.SubTokens.insert(ret.SubTokens.end(),statement(statement::Token,token));
      return ret;
    } else if (upp==";") {
      ret.SubTokens.insert(ret.SubTokens.end(),statement(statement::Token,token));
      return ret;
    } else if (upp.startsWith("/*")||upp.startsWith("--")) {
      if (ret.SubTokens.begin()==ret.SubTokens.end()) {
	if (ret.Comment.isNull())
	  ret.Comment=token;
	else
	  ret.Comment+="\n"+token;
      } else {
	QString &com=(*ret.SubTokens.rbegin()).Comment;
	if (com.isEmpty())
	  com=token;
	else
	  com+="\n"+token;
      }
    } else {
      ret.SubTokens.insert(ret.SubTokens.end(),statement(statement::Token,token));
    }
  }
  return ret;
}

std::list<toSQLParse::statement> toSQLParse::parse(const QString &str)
{
  int pos=0;
  std::list<toSQLParse::statement> ret;
  statement cur(statement::Statement);
  for(cur=parseStatement(str,pos,false);
      cur.SubTokens.begin()!=cur.SubTokens.end();
      cur=parseStatement(str,pos,false)) {
    ret.insert(ret.end(),cur);
  }
  if (pos<int(str.length()))
    ret.insert(ret.end(),statement(statement::Raw,
				   str.mid(pos,str.length()-pos)));
  return ret;
}
