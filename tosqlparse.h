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

#ifndef TOSQLPARSE_H
#define TOSQLPARSE_H

#include <list>

#include <qstring.h>

class toMarkedText;
class toConnection;

/** A bunch of functions to parse and indent SQL text.
 */

class toSQLParse {
public:
  struct settings {
    bool ExpandSpaces;
    bool CommaBefore;
    bool BlockOpenLine;
    bool OperatorSpace;
    bool KeywordUpper;
    bool RightSeparator;
    bool EndBlockNewline;
    int IndentLevel;
    int CommentColumn;
  };

  /** Structure the statement is parsed into.
   */
  class statement {
    /** Subtokens to this token.
     */
    std::list<statement> *SubTokens;
  public:
    /** Type of this token.
     */
    enum type {
      /** Start of block.
       */
      Block,
      /** Start of statement.
       */
      Statement,
      /** Sub list
       */
      List,
      /** Parameter of statement.
       */
      Keyword,
      /** Another token of whatever it is detected in.
       */
      Token,
      /** Unparsed data
       */
      Raw
      /** Type of token.
       */
    } Type;
    /** Was there a comment attached to this token
     */
    QString Comment;
    /** The actual token.
     */
    QString String;
    /** The original line of the token (0 is first line)
     */
    int Line;
    /** Create statement
     */
    statement(type ntype=Token,const QString &token=QString::null,int cline=-1);
    /** Allocate subtokens if not available and return reference to subtokens
     */
    std::list<statement> &subTokens();
    /** Copy constructor
     */
    statement(const statement &);
    /** Copy operator
     */
    const statement &operator = (const statement &);
    /** Destroy statement
     */
    ~statement();
  };

  /** Get next SQL token from an editor.
   * @param text Editor to get token from
   * @param curLine Current line when parsing, modified when calling.
   * @param pos Current position when parsing, modified when calling.
   * @param forward If true go forward from current position, otherwise backward.
   * @param comment Include comments as tokens.
   * @return String with next token in editor
   */
  static QString getToken(toMarkedText *text,int &curLine,int &pos,bool forward=true,
			  bool comment=false);

  /** Get next SQL token from a string.
   * @param str String to get token from
   * @param line The line the current position is on. This is not needed, but only valid
   *             if a correct value is passed into the function.
   * @param pos Current position when parsing, modified when calling.
   * @param forward If true go forward from current position, otherwise backward.
   * @param comment Include comments as tokens.
   * @return String with next token in editor
   */
  static QString getToken(const QString &sql,int &line,int &pos,bool forward=true,
			  bool comment=false);
  /** Get next SQL token from a string.
   * @param str String to get token from
   * @param pos Current position when parsing, modified when calling.
   * @param forward If true go forward from current position, otherwise backward.
   * @param comment Include comments as tokens.
   * @return String with next token in editor
   */
  static QString getToken(const QString &sql,int &pos,bool forward=true,
			  bool comment=false)
  { int t; return getToken(sql,t,pos,forward,comment); }

  /** Parse a string.
   * @param str String to parse.
   * @return Parsed statement tree.
   */
  static std::list<statement> parse(const QString &str);
  /** Parse a string.
   * @param str String to parse.
   * @param conn Connection to determine SQL dialect. (For future use)
   * @return Parsed statement tree.
   */
  static std::list<statement> parse(const QString &str,toConnection &conn)
  { return parse(str); }

  /** Indent a string.
   * @param str String to indent.
   * @return An indented string.
   */
  static QString indent(const QString &str);
  /** Indent a parse statement structure into a string.
   * @param stat Statement to indent.
   * @param level Initial indentation level to use.
   * @return A string with the indented statement.
   */
  static QString indentStatement(statement &stat,int level=0);
  /** Indent a string.
   * @param str String to indent.
   * @param conn Connection to determine SQL dialect. (For future use)
   * @return An indented string.
   */
  static QString indent(const QString &str,toConnection &conn)
  { return indent(str); }

  /** Create an indentation string.
   * @param level Number of characters to indent.
   */
  static QString indentString(int level);
  /** Count indentation level of a string.
   * @param str String to check.
   * @param chars Position in string.
   */
  static int countIndent(const QString &str,int &chars);
private:
  static settings Settings;
  static statement parseStatement(const QString &str,
				  int &cline,
				  int &pos,
				  bool declare);
public:
  /** Get current settings.
   */
  static settings getSetting(void)
  { return Settings; }
  /** Get current settings.
   */
  static void setSetting(const settings &setting)
  { Settings=setting; }
};

#endif
