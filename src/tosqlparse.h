
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSQLPARSE_H
#define TOSQLPARSE_H

#include "config.h"

#include <list>
#include <algorithm>
#include <qstring.h>

class toConnection;
class toMarkedText;
class toSyntaxAnalyzer;

/** A bunch of functions to parse and indent SQL text.
 */

class toSQLParse
{
public:
    struct settings
    {
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
    class statement
    {
        /** Subtokens to this token.
         */
        std::list<statement> *SubTokens;
    public:
        /** Type of this token.
         */
        enum type
        {
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
        statement(type ntype = Token, const QString &token = QString::null, int cline = -1);
        /** Allocate subtokens if not available and return reference to subtokens
         */
        std::list<statement> &subTokens();
        /** Copy constructor
         */
        statement(const statement &);
        /** Copy operator
         */
        const statement &operator = (const statement &);
        /** Equality operator. Does not include the line for comparison.
         */
        bool operator == (const statement &) const;
        /** Non rquality operator. Does not include the line for comparison.
         */
        bool operator != (const statement &stat) const
        {
            return !((*this) == stat);
        }
        /** Destroy statement
         */
        ~statement();
    };

    /** Abstract class to define a source of tokens for the parser.
     */
    class tokenizer
    {
    private:
        toSyntaxAnalyzer *Analyzer;
    protected:
        int Offset;
        int Line;
    public:
        /** Create a tokenizer. Optionally specify which line and offset to start at.
         */
        tokenizer(int offset = 0, int line = 0)
        {
            Line = line;
            Offset = offset;
            Analyzer = NULL;
        }
        /** Create a tokenizer. Optionally specify which line and offset to start at.
         */
        tokenizer(toSyntaxAnalyzer &analyzer, int offset = 0, int line = 0)
        {
            Line = line;
            Offset = offset;
            Analyzer = &analyzer;
        }
        virtual ~tokenizer()
        { }
        /** Get a token from the string.
         * @param forward Go forward or backwards to get next token.
         * @param comment Include comments as tokens.
         */
        virtual QString getToken(bool forward = true, bool comment = false) = 0;
        /** Specify the syntax analyzer to use for the tokanizer
         * @param analyzer
         */
        virtual void setAnalyzer(toSyntaxAnalyzer &analyzer)
        {
            Analyzer = &analyzer;
        }
        virtual toSyntaxAnalyzer &analyzer();
        /** Get the current line of the tokenizer. A line is defined by a \n character
         */
        virtual int line(void)
        {
            return Line;
        }
        /** Current offset of the tokenizer (Should point to the character after the last token in the specified direction)
         */
        virtual int offset(void)
        {
            return Offset;
        }
        /** Set a new offset of tokenzer.
         */
        virtual void setOffset(int offset)
        {
            Offset = offset;
        }
        /** Set new current line of tokenizer.
         * This will not affect the current position of the tokenizer.
         */
        virtual void setLine(int line)
        {
            Line = line;
        }
        /** Get the data remaining after the current position.
         * @param eol If true end of line, otherwise end of tokenizer.
         */
        virtual QString remaining(bool eol) = 0;
    };

    /** Tokenizer class which gets tokens from a string.
     */
class stringTokenizer : public tokenizer
    {
        QString String;
    public:
        stringTokenizer(const QString &str, int offset = 0, int line = 0)
                : tokenizer(offset, line)
        {
            String = str;
        }
        stringTokenizer(const QString &str, toSyntaxAnalyzer &analyzer, int offset = 0, int line = 0)
                : tokenizer(analyzer, offset, line)
        {
            String = str;
        }
        /** Get a token from the string.
         * @param forward Go forward or backwards to get next token.
         * @param comment Include comments as tokens.
         */
        virtual QString getToken(bool forward = true, bool comment = false);
        /** Get the data remaining after the current position.
         * @param eol If true end of line, otherwise end of tokenizer.
         */
        virtual QString remaining(bool eol);
    };

    /** Tokenizer class which gets tokens from an editor.
     */
class editorTokenizer : public tokenizer
    {
        toMarkedText *Editor;
    public:
        /** Create a tokenizer which takes its input from an editor.
         * @param editor The editor to read from. Observe that if this
         * is a syntax highlighted editor the syntax analyzer is used
         * that is used for the editor.
         */
        editorTokenizer(toMarkedText *editor, int offset = 0, int line = 0);
        /** Get a token from the string.
         * @param forward Go forward or backwards to get next token.
         * @param comment Include comments as tokens.
         */
        virtual QString getToken(bool forward = true, bool comment = false);
        /** Get the data remaining after the current position.
         * @param eol If true end of line, otherwise end of tokenizer.
         */
        virtual QString remaining(bool eol);
    };

    /** Parse a string.
     * @param tokens Tokenizer provider to generate parsed tree from.
     * @return Parsed statement tree.
     */
    static std::list<statement> parse(tokenizer &tokens);
    /** Parse a string.
     * @param str String to parse.
     * @return Parsed statement tree.
     */
    static std::list<statement> parse(const QString &str)
    {
        stringTokenizer tokens(str);
        return parse(tokens);
    }
    /** Parse a string.
     * @param str String to parse.
     * @param conn Connection to determine SQL dialect. (For future use)
     * @return Parsed statement tree.
     */
    static std::list<statement> parse(const QString &str, toConnection &conn);

    /** Get one statement (or block) from the root of an editor or string.
     * @param tokens Tokenizer to get tokens from.
     * @param conn Connection to determine SQL dialog. (For future use)
     */
    static statement parseStatement(tokenizer &tokens);
    /** Get one statement (or block) from the root of an editor or string.
     * @param str Tokenizer to get tokens from.
     * @param conn Connection to determine SQL dialog. (For future use)
     */
    static statement parseStatement(const QString &str)
    {
        stringTokenizer tokens(str);
        return parseStatement(tokens);
    }
    /** Get one statement (or block) from the root of an editor or string.
     * @param tokens Tokenizer to get tokens from.
     * @param conn Connection to determine SQL dialog. (For future use)
     */
    static statement parseStatement(tokenizer &tokens, toConnection &conn);
    /** Get one statement (or block) from the root of an editor or string.
     * @param str Tokenizer to get tokens from.
     * @param conn Connection to determine SQL dialog. (For future use)
     */
    static statement parseStatement(const QString &str, toConnection &conn)
    {
        stringTokenizer tokens(str);
        return parseStatement(tokens, conn);
    }

    /** Indent a string.
     * @param str String to indent.
     * @return An indented string.
     */
    static QString indent(const QString &str);
    /** Indent a parse statement structure into a string.
     * @param stat Statement to indent.
     * @param level Initial indentation level to use.
     * @param syntax The syntax analyzer to use.
     * @return A string with the indented statement.
     */
    static QString indentStatement(statement &stat, toConnection &conn, int level = 0);
    /** Indent a parse statement structure into a string.
     * @param stat Statement to indent.
     * @param level Initial indentation level to use.
     * @return A string with the indented statement.
     */
    static QString indentStatement(statement &stat, int level = 0);
    /** Indent a string.
     * @param str String to indent.
     * @param conn Connection to determine SQL dialect. (For future use)
     * @return An indented string.
     */
    static QString indent(const QString &str, toConnection &conn);

    /** Create an indentation string.
     * @param level Number of characters to indent.
     */
    static QString indentString(int level);

    /** Indent a string.
     * @param str List of statements
     * @param conn Connection to determine SQL dialect. (For future use)
     * @return An indented string.
     */
    static QString indent(std::list<statement> &stat, toConnection &conn);
    /** Indent a string.
     * @param stat List of statements
     * @return An indented string.
     */
    static QString indent(std::list<statement> &stat);

    /** Count indentation level of a string.
     * @param str String to check.
     * @param chars Position in string.
     */
    static int countIndent(const QString &str, int &chars);
    /** Indent a string.
     * @param str String to indent.
     * @param syntax Syntax analyzer to determine SQL dialect. (For future use)
     * @return An indented string.
     */
    static QString indent(const QString &str, toSyntaxAnalyzer &syntax);
    /** Indent a string.
     * @param stat List of statements
     * @param syntax Syntax analyzer to determine SQL dialect. (For future use)
     * @return An indented string.
     */
    static QString indent(std::list<statement> &stat, toSyntaxAnalyzer &syntax);
private:
    static settings Settings;
    static statement parseStatement(tokenizer &tokens,
                                    bool declare, bool lst);
    /** Indent a parse statement structure into a string.
     * @param stat Statement to indent.
     * @param level Initial indentation level to use.
     * @param syntax The syntax analyzer to use.
     * @return A string with the indented statement.
     */
    static QString indentStatement(statement &stat, int level, toSyntaxAnalyzer &conn);

    /**
     * Test if string is a known SQL operator.
     */
    static bool isOperator(QString);

public:
    /** Get current settings.
     */
    static settings getSetting(void)
    {
        return Settings;
    }
    /** Get current settings.
     */
    static void setSetting(const settings &setting)
    {
        Settings = setting;
    }
};

#endif
