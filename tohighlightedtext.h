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
class toSyntaxSetup;

class toSyntaxAnalyzer {
public:
  enum infoType {
    Normal=0,
    Keyword=1,
    String=2,
    Error=3,
    Comment=4,
    ErrorBkg=5,
    NormalBkg=6,
    CurrentBkg=7
  };
  struct highlightInfo {
    infoType Type;
    int Start;
    highlightInfo(int start,infoType typ=Normal)
    { Start=start; Type=typ; }
  };
private:
  QColor Colors[8];
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
  void readColor(const QColor &def,infoType pos);
  static QString typeString(infoType typ);
  static infoType typeString(const QString &str);
public:
  toSyntaxAnalyzer(const char **keywords);
  virtual ~toSyntaxAnalyzer()
  { }
  virtual list<highlightInfo> analyzeLine(const QString &str);
  QColor getColor(infoType typ)
  { return Colors[typ]; }
  void updateSettings(void);

  friend toSyntaxSetup;
};

toSyntaxAnalyzer &toDefaultAnalyzer(void);

class toHighlightedText : public toMarkedText {
private:
  Q_OBJECT

  int LastCol;
  int LastRow;
  int LastLength;
  int Current;
  int LeftIgnore;
  bool Highlight;
  bool KeywordUpper;
  bool Cursor;
  map<int,QString> Errors;
  toSyntaxAnalyzer *Analyzer;
protected:
  void setLeftIgnore(int ignore)
  { LeftIgnore=ignore; }
public:

  static int convertLine(int line,int start,int diff);

  toHighlightedText(QWidget *parent,const char *name=NULL);

  void clear(void)
  { Errors.clear(); Current=-1; toMarkedText::clear(); }
  void setText(const QString &str);

  void setErrors(const map<int,QString> &errors)
  { Errors=errors; update(); }
  void setCurrent(int current);
  int current(void)
  { return Current; update(); }
  void setKeywordUpper(bool val)
  { KeywordUpper=val; update(); }
  void setHighlight(bool val)
  { Highlight=val; update(); }
  void setAnalyzer(toSyntaxAnalyzer &analyzer)
  { Analyzer=&analyzer; update(); }
  toSyntaxAnalyzer &analyzer(void)
  { return *Analyzer; }

  virtual void paintCell (QPainter *painter,int row,int col);

signals:
  void insertedLines(int,int);
protected slots:
  void textChanged(void);

public slots:
  void nextError(void);
  void previousError(void);
};

#endif
