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

#include "toconf.h"


#include "tomain.h"
#include "tohtml.h"

#include <ctype.h>
#include <string.h>

#include <qapplication.h>
#include <qregexp.h>

toHtml::toHtml(const QCString &data)
{
  Length=strlen(data);
  Data=new char[Length+1];
  strcpy(Data,data);
  Position=0;
  LastChar=0;
}

toHtml::~toHtml()
{
  delete[] Data;
}

void toHtml::skipSpace(void)
{
  if (Position>=Length)
    return;
  char c=LastChar;
  if (!c)
    c=Data[Position];
  if (isspace(c)) {
    Position++;
    LastChar=0;
    while(Position<Length&&isspace(Data[Position]))
      Position++;
  }
}

bool toHtml::eof(void)
{
  if (Position>Length)
    throw qApp->translate("toHtml","Invalidly went beyond end of file");
  return Position==Length;
}

void toHtml::nextToken(void)
{
  if (eof())
    throw qApp->translate("toHtml","Reading HTML after eof");
  QualifierNum=0;
  char c=LastChar;
  if (!c)
    c=Data[Position];
  if (c=='<') {
    IsTag=true;
    Position++;
    LastChar=0;
    skipSpace();
    if (Position>=Length)
      throw qApp->translate("toHtml","Lone < at end");
    if (Data[Position]!='/') {
      Open=true;
    } else {
      Open=false;
      Position++;
    }
    skipSpace();
    {
      size_t start=Position;
      while(Position<Length&&!isspace(Data[Position])&&Data[Position]!='>') {
	Data[Position]=tolower(Data[Position]);
	Position++;
      }
      Tag=mid(start,Position-start);
    }
    for(;;) {
      skipSpace();
      if (Position>=Length)
	throw qApp->translate("toHtml","Unended tag at end");

      c=LastChar;
      if (!c)
	c=Data[Position];
      if (c=='>') {
	LastChar=0;
	Position++;
	break;
      }

      // Must always be an empty char here, so LastChar not needed to be checked.

      {
	size_t start=Position;
	
	while(Position<Length&&
	      !isspace(Data[Position])&&
	      Data[Position]!='='&&
	      Data[Position]!='>') {
	  Data[Position]=tolower(Data[Position]);
	  Position++;
	}
	Qualifiers[QualifierNum].Name=mid(start,Position-start);
      }
      skipSpace();
      if (Position>=Length)
	throw qApp->translate("toHtml","Unended tag qualifier at end");
      c=LastChar;
      if (!c)
	c=Data[Position];
      if (c=='=') {
	LastChar=0;
	Position++;
	skipSpace();
	if (Position>=Length)
	  throw qApp->translate("toHtml","Unended tag qualifier data at end");
	c=Data[Position];
	if (c=='\''||c=='\"') {
	  Position++;
	  size_t start=Position;
	  while(Data[Position]!=c) {
	    Position++;
	    if (Position>=Length)
	      throw qApp->translate("toHtml","Unended quoted string at end");
	  }
	  Qualifiers[QualifierNum].Value=mid(start,Position-start);
	  Position++;
	  LastChar=0;
	} else {
	  size_t start=Position;
	  while(!isspace(Data[Position])&&Data[Position]!='>') {
	    Position++;
	    if (Position>=Length)
	      throw qApp->translate("toHtml","Unended qualifier data at end");
	  }
	  Qualifiers[QualifierNum].Value=mid(start,Position-start);
	}
      }
      QualifierNum++;
      if (QualifierNum>=TO_HTML_MAX_QUAL)
	throw qApp->translate("toHtml","Exceded qualifier max in toHtml");
    }
  } else {
    IsTag=false;
    size_t start=Position;
    Position++;
    LastChar=0;
    while(Position<Length) {
      if (Data[Position]=='<')
	break;
      Position++;
    }
    Text=mid(start,Position-start);
  }
}

const char *toHtml::value(const QCString &q)
{
  for (int i=0;i<QualifierNum;i++) {
    if (q==Qualifiers[i].Name)
      return Qualifiers[i].Value;
  }
  return NULL;
}

QCString toHtml::text()
{
  QCString ret;
  for (const char *cur=Text;*cur;cur++) {
    char c=*cur;
    if (c=='&') {
      const char *start=cur+1;
      while(*cur&&*cur!=';')
	cur++;
      QCString tmp(start,cur-start);
      if (tmp[0]=='#') {
	tmp=tmp.right(tmp.length()-1);
	ret+=char(tmp.toInt());
      } else if (tmp=="auml")
	ret+="å";
      // The rest of the & codes...
    } else
      ret+=c;
  }
  return ret;
}

const char *toHtml::mid(size_t start,size_t size)
{
  if (size==0)
    return "";
  if (start>=Length)
    throw qApp->translate("toHtml","Tried to access string out of bounds in mid (start=%1)").arg(start);
  if (size>Length)
    throw qApp->translate("toHtml","Tried to access string out of bounds in mid (size=%1)").arg(size);
  if (start+size>Length)
    throw qApp->translate("toHtml","Tried to access string out of bounds in mid (total=%1+%2>%3)").
      arg(start).
      arg(size).
      arg(Length);

  LastChar=Data[start+size];
  Data[start+size]=0;
  return Data+start;
}

bool toHtml::search(const QCString &all,const QString &str)
{
  QCString data(str.lower().latin1());
  enum {
    beginning,
    inTag,
    inString,
    inWord
  } lastState=beginning,state=beginning;
  unsigned int pos=0;
  char endString;
  for (size_t i=0;i<all.length();i++) {
    char c=tolower(all.at(i));
    if (c=='\''||c=='\"') {
      endString=c;
      state=inString;
    } else if (c=='<') {
      state=inTag;
    } else {
      switch (state) {
      case inString:
	if (c==endString)
	  state=lastState;
	break;
      case beginning:
	if (data.at(pos)!=c) {
	  pos=0;
	  state=inWord;
	} else {
	  pos++;
	  if (pos>=data.length()) {
	    if (i+1>=all.length()||!isalnum(all.at(i+1)))
	      return true;
	    pos=0;
	  }
	  break;
	}
	// Intentionally no break here
      case inWord:
	if (!isalnum(c))
	  state=beginning;
	break;
      case inTag:
	if (c=='>')
	  state=beginning;
	break;
      }
    }
  }
  return false;
}

QString toHtml::escape(const QString &html)
{
  QString ret=html;

  static QRegExp amp(QString::fromLatin1("\\&"));
  static QRegExp lt(QString::fromLatin1("\\<"));
  static QRegExp gt(QString::fromLatin1("\\>"));

#if 0
  ret.replace(amp,"&amp;");
  ret.replace(lt,"&lt;");
  ret.replace(gt,"&gt;");
#endif
  return ret;
}
