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

#ifndef __TOSQLPARSE_H
#define __TOSQLPARSE_H

#include <list>

#include <qstring.h>

class toMarkedText;
class toConnection;

/** Parse an SQL statement to a structured list of blocks, statements and tokens.
 */

class toSQLParse {
public:
  /** Structure the statement is parsed into.
   */
  struct statement {
    /** Type of this token.
     */
    enum type {
      /** Start of block.
       */
      Block,
      /** Start of statement.
       */
      Statement,
      /** Parameter of statement.
       */
      Parameter,
      /** Another token of whatever it is detected in.
       */
      Token
      /** Type of token.
       */
    } Type;
    /** The actual token.
     */
    QString String;
    /** Was there a comment attached to this token
     */
    QString Comment;
    /** Subtokens to this token.
     */
    std::list<statement> SubTokens;
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
   * @param pos Current position when parsing, modified when calling.
   * @param forward If true go forward from current position, otherwise backward.
   * @param comment Include comments as tokens.
   * @return String with next token in editor
   */
  static QString getToken(const QString &sql,int &pos,bool forward=true,
			  bool comment=false);

  /** Parse a string.
   * @param str String to parse.
   * @param conn Connection to determine SQL dialect. (For future use)
   * @return Parsed statement tree.
   */
  std::list<statement> parse(const QString &str,toConnection &conn);
};

#endif
