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

#ifndef __TOHIGHLIGHTEDTEXT_H
#define __TOHIGHLIGHTEDTEXT_H

#include <list>
#include <map>

#include "tomarkedtext.h"

class QPainter;
class toSyntaxSetup;
class QListBox;

/** This class implements a syntax parser to provide information to
 * a syntax highlighted editor.
 */

class toSyntaxAnalyzer {
public:
  /** Information about how highlighting has changed.
   */
  enum infoType {
    /** Normal text
     */
    Normal=0,
    /** This is a keyword
     */
    Keyword=1,
    /** This is a string
     */
    String=2,
    /** Malformed syntax
     */
    Error=3,
    /** Comment
     */
    Comment=4,
    /** Error in SQL background. Not used by highlighter, but configured in the same way.
     */
    ErrorBkg=5,
    /** Normal background. Not used by highlighter, but configured in the same way.
     */
    NormalBkg=6,
    /** Current background. Not used by highlighter, but configured in the same way.
     */
    CurrentBkg=7
  };
  /** Information about a change in highlighting.
   */
  struct highlightInfo {
    /** New type of highlighting.
     */
    infoType Type;
    /** Start in line for this highlighting.
     */
    int Start;
    highlightInfo(int start,infoType typ=Normal)
    { Start=start; Type=typ; }
  };
private:
  /** Indicate if colors are updated, can't do this in constructor since QApplication
   * isn't initialized yet.
   */
  bool ColorsUpdated;
  /** Colors allocated for the different @ref infoType values.
   */
  QColor Colors[8];
  /** Keeps track of possible hits found so far.
   */
  struct posibleHit {
    posibleHit(const char *);
    /** Where you are in this word to find a hit.
     */
    int Pos;
    /** The text to hit, points into keywords array.
     */
    const char *Text;
  };
  /** An array of lists of keywords, indexed on the first character.
   */
  std::list<const char *> Keywords[256];
protected:
  /** Check if this is part of a symbol or not.
   */
  bool isSymbol(QChar c)
  { return (c.isLetterOrNumber()||c=='_'||c=='#'||c=='$'||c=='.'); }
private:
  /** Get a colordefinition from a @ref infoType value.
   * @param def Color to fill out.
   * @param pos @ref infoType to get color for.
   */
  void readColor(const QColor &def,infoType pos);
  /** Get a string representation of an @ref infoType.
   * @param typ @ref infoType to get string for.
   * @return Description of infotype.
   */
  static QString typeString(infoType typ);
  /** Get an @ref infoType from a string representation of it.
   * @param str Description of @ref infoType.
   * @return @ref infoType described by string.
   */
  static infoType typeString(const QString &str);
  /** Update configuration settings from this class color values.
   */
  void updateSettings(void);
public:
  /** Create a syntax analysed
   * @param keywords A list of keywords.
   */
  toSyntaxAnalyzer(const char **keywords);
  virtual ~toSyntaxAnalyzer()
  { }
  /** Analyze a line. There is no guarantee that lines will be called in any kind of order.
   * @param str Line to analyze.
   * @return A list of where highlighting should change. Start as normal.
   */
  virtual std::list<highlightInfo> analyzeLine(const QString &str);
  /** Get a colordefinition for a @ref infoType value.
   * @param typ @ref infoType to get color for.
   * @return Color of that type.
   */
  QColor getColor(infoType typ)
  { return Colors[typ]; }

  friend class toSyntaxSetup;
};

/** Get the default syntax analyzer.
 * @return Reference to the default analyzer.
 */
toSyntaxAnalyzer &toDefaultAnalyzer(void);

/** A simple editor which supports syntax highlighting.
 */

class toHighlightedText : public toMarkedText {
private:
  Q_OBJECT

  /** Used internally for drawing.
   */
  int LastCol;
  /** Used internally for drawing.
   */
  int LastRow;
  /** Used internally for drawing.
   */
  int LastLength;
  /** Current line has different background than others.
   */
  int Current;
  /** Indicate how many pixels on the left to ignore painting.
   */
  int LeftIgnore;
  /** Indicate if text should be highlighted or not.
   */
  bool Highlight;
  /** Indicate if keywords should be converted to uppercase when displayed.
   */
  bool KeywordUpper;
  /** Used internally for drawing.
   */
  int Cursor;
  /** Map of rows with errors and their error message.
   */
  std::map<int,QString> Errors;
  /** The syntax analyzer to use.
   */
  toSyntaxAnalyzer *Analyzer;

  bool NoCompletion;
  bool KeepCompletion;
  QListBox *Completion;
  int CompleteItem;
  std::list<QString> AllComplete;

  bool invalidToken(int line,int col);
protected:
  /** Set how much of the left margin to ignore painting.
   */
  void setLeftIgnore(int ignore)
  { LeftIgnore=ignore; }
  /** Reimplemented for internal reasons.
   */
  virtual void keyPressEvent(QKeyEvent *e);
  /** Reimplemented for internal reasons.
   */
  virtual void focusOutEvent(QFocusEvent *e);
  /** Check if to view completion.
   */
  virtual void checkComplete(void);
public:
  /** Convert a linenumber after a change of the buffer to another linenumber. Can be
   * used to convert a specific linenumber after receiving a @ref insertedLines call.
   * @param line Line number.
   * @param start Start of change.
   * @param diff Lines added or removed.
   * @return New linenumber or -1 if line doesn't exist anymore.
   */
  static int convertLine(int line,int start,int diff);
  /** Create a new editor.
   * @param parent Parent of widget.
   * @param name Name of widget.
   */
  toHighlightedText(QWidget *parent,const char *name=NULL);
  /** Reimplemented for internal reasons
   */
  virtual ~toHighlightedText();

  /** Clear the editor.
   */
  void clear(void)
  { Errors.clear(); Current=-1; toMarkedText::clear(); }
  /** Set the text of this editor.
   */
  void setText(const QString &str);

  /** Set the error list map.
   * @param errors A map of linenumbers to errorstrings. These will be displayed in the
   *               statusbar if the cursor is placed on the line.
   */
  void setErrors(const std::map<int,QString> &errors);
  /** Set current line. Will be indicated with a different background.
   * @param current Current line.
   */
  void setCurrent(int current);
  /** Get current line.
   * @return Current line.
   */
  int current(void)
  { return Current; update(); }
  /** Set keyword upper flag. If this is set keywords will be converted to uppercase when painted.
   * @param val New value of keyword to upper flag.
   */
  void setKeywordUpper(bool val)
  { KeywordUpper=val; update(); }
  /** The the highlighting flag. If this isn't set no highlighting is done.
   */
  void setHighlight(bool val)
  { Highlight=val; update(); }
  /** Set the syntax highlighter to use.
   * @param analyzer Analyzer to use.
   */
  void setAnalyzer(toSyntaxAnalyzer &analyzer)
  { Analyzer=&analyzer; update(); }
  /** Get the current syntaxhighlighter.
   * @return Analyzer used.
   */
  toSyntaxAnalyzer &analyzer(void)
  { return *Analyzer; }

  /** Reimplemented for internal reasons.
   */
  virtual void paintCell (QPainter *painter,int row,int col);

signals:
  /** Emitted when lines are inserted or removed.
   * @param start Start of line inserted.
   * @param diff Number of lines inserted or removed.
   */
  void insertedLines(int start,int diff);
protected slots:
  void textChanged(void);

public slots:
  /** Go to next error.
   */
  void nextError(void);
  /** Go to previous error.
   */
  void previousError(void);
private slots:
  void selectComplete(void);
};

#endif
