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
#include <map>

#include "tomarkedtext.h"

class QPainter;

class toSyntaxAnalyzer {
public:
  enum infoType {
    Normal=0,
    Keyword=1,
    String=2,
    Error=3,
    Comment=4,
    ErrorBkg=5,
    NormalBkg=6
  };
  struct highlightInfo {
    infoType Type;
    int Start;
    highlightInfo(int start,infoType typ=Normal)
    { Start=start; Type=typ; }
  };
private:
  QColor Colors[7];
  struct posibleHit {
    posibleHit(const char *);
    int Pos;
    const char *Text;
  };
  list<const char *> Keywords[256];
protected:
  bool isSymbol(QChar c)
  { return (c.isLetterOrNumber()||c=='_'||c=='#'||c=='$'||c=='.'); }
private:
  void readColor(const QString &str,const QColor &def,int pos);
public:
  toSyntaxAnalyzer(const char **keywords);
  virtual ~toSyntaxAnalyzer()
  { }
  virtual list<highlightInfo> analyzeLine(const QString &str);
  QColor getColor(infoType typ)
  { return Colors[typ]; }
  void updateSettings(void);
};

toSyntaxAnalyzer &toDefaultAnalyzer(void);

class toHighlightedText : public toMarkedText {
private:
  int LastCol;
  int LastRow;
  bool Highlight;
  bool KeywordUpper;
  map<int,QString> Errors;
public:

  toHighlightedText(QWidget *parent,const char *name=NULL);

  void setErrors(const map<int,QString> &errors)
  { Errors=errors; repaint(); }

  virtual void paintCell (QPainter *painter,int row,int col);
};

#endif
