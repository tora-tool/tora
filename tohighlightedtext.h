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



#ifndef __TOHIGHLIGHTEDTEXT_H
#define __TOHIGHLIGHTEDTEXT_H

#include <list>

#include "tomarkedtext.h"

class QPainter;

class toSyntaxAnalyzer {
public:
  struct highlightInfo {
    highlightInfo(int start,bool keyword=false,bool str=false,bool error=false,bool comment=false)
    { Start=start; Keyword=keyword; String=str; Error=error; Comment=comment; }
    int Start;
    bool Keyword;
    bool String;
    bool Error;
    bool Comment;
  };
private:
  struct posibleHit {
    posibleHit(const char *);
    int Pos;
    const char *Text;
  };
  list<const char *> Keywords[256];
protected:
  bool isSymbol(QChar c)
  { return (c.isLetterOrNumber()||c=='_'||c=='#'||c=='$'||c=='.'); }
public:
  toSyntaxAnalyzer(const char **keywords);
  virtual ~toSyntaxAnalyzer()
  { }
  virtual list<highlightInfo> analyzeLine(const QString &str);
};

class toHighlightedText : public toMarkedText {
private:
  int LastCol;
  int LastRow;
  bool Highlight;
  bool KeywordUpper;
public:

  toHighlightedText(QWidget *parent,const char *name=NULL);
  virtual void paintCell (QPainter *painter,int row,int col);
};

#endif
