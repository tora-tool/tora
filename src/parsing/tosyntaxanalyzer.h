
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

#ifndef TOSYNTAX_ANALYZER_H
#define TOSYNTAX_ANALYZER_H

#include <QtCore/QObject>
#include <QtGui/QColor>

#include <Qsci/qscilexersql.h>

class toHighlightedText;
class QsciLexer;

/** provides two "independent" functionalities:
 * - lexer (used for syntax highlighting)
 * - pseudo-parser (used to get beginning and the and of the SQL statement)
 */
class toSyntaxAnalyzer : public QObject
{
	Q_OBJECT;
	Q_ENUMS(statementClass);
	Q_ENUMS(wordClass);
	friend class toHighlightedText;
public:
    /** Indicates type of statement - DDL/DML or PLSQL block
        This is later used when deciding if trailing semicolon should
        be removed or not before executing.
     */
    enum statementClass
    {
        UNKNOWN,
        SELECT,		// SELECT/WITH - this one shows data grid with result
        DML,		// INSERT/UPDATE/DELETE/MERGE
        DDL,        // CREATE
        PLSQL,      // DECLARE/BEGIN ...
        OTHER,      // ALTER SESSION ..., ANALYZE, SET ROLE, EXPLAIN
        SQLPLUS		// sqlplus command
    };

    //! This enum defines the meanings of the different styles used by the
    //! SQL lexer. This enum is copied from QsciLexerSQL
    enum wordClass {
    	//! The default.  - not used at all - or used for whitespace only
    	Default = QsciLexerSQL::Default,

    	//! A comment.
    	Comment = QsciLexerSQL::Comment,

    	//! A line comment.
    	////CommentLine = 2,                    - not used, see Comment

    	//! A JavaDoc/Doxygen style comment.
    	////CommentDoc = 3,                     - not used, see Comment

    	//! A number.
    	Number = QsciLexerSQL::Number,

    	//! A keyword.
    	Keyword = QsciLexerSQL::Keyword,

    	//! A double-quoted string.
    	DoubleQuotedString = 6,

    	//! A single-quoted string.
    	SingleQuotedString = QsciLexerSQL::SingleQuotedString,

    	//! An SQL*Plus keyword.
    	PlusKeyword = 8,

    	//! An SQL*Plus prompt.
    	PlusPrompt = 9,

    	//! An operator.
    	Operator = QsciLexerSQL::Operator,

    	//! An identifier
    	Identifier = QsciLexerSQL::Identifier,

    	//! An SQL*Plus comment.
    	////PlusComment = 13,                 - not used, see Comment

    	//! A '#' line comment.
    	////CommentLineHash = 15,             - not used, see Comment

    	//! A JavaDoc/Doxygen keyword.        - not used, see Comment
    	////CommentDocKeyword = 17,

    	//! A JavaDoc/Doxygen keyword error.  - not used, see Comment
    	////CommentDocKeywordError = 18,

    	//! A keyword defined in keyword set number 5.  The class must be
    	//! sub-classed and re-implement keywords() to make use of this style.
    	KeywordSet5 = 19,

    	//! A keyword defined in keyword set number 6.  The class must be
    	//! sub-classed and re-implement keywords() to make use of this style.
    	////KeywordSet6 = 20,

    	//! A keyword defined in keyword set number 7.  The class must be
    	//! sub-classed and re-implement keywords() to make use of this style.
    	////KeywordSet7 = 21,

    	//! A keyword defined in keyword set number 8.  The class must be
    	//! sub-classed and re-implement keywords() to make use of this style.
    	////KeywordSet8 = 22,

    	//! A quoted identifier.
    	QuotedIdentifier = 23
    };

    /* statements is represented only by lines range */
    class statement
    {
    public:
    	int lineFrom, lineTo;
    	int posFrom, posTo;
    	QString sql, firstWord;
    	statementClass statementType;
    	statement();
    	statement(unsigned lf, unsigned lt);
    	statement& operator=(statement const& other);
    };

    typedef QList<statement> statementList;

    toSyntaxAnalyzer(toHighlightedText *parent);
    virtual ~toSyntaxAnalyzer();

    QColor getColor(wordClass type) const;

    /*
     * this method should be "stateless" can be called from "both" threads
     */
    virtual statementList getStatements(QString const& text) = 0;

    virtual statement getStatementAt(unsigned line, unsigned linePos) = 0;

    virtual QsciLexer* createLexer(QObject *parent = 0) = 0;

    virtual void sanitizeStatement(statement&) = 0;
private:
    /** Colors allocated for the different @ref wordClass values.
     */
    QMap<wordClass, QColor> Colors;

};

/* This "analyzer" uses an empty line as statement delimiter */
class toSyntaxAnalyzerNL : public toSyntaxAnalyzer
{
	Q_OBJECT;
public:
	toSyntaxAnalyzerNL(toHighlightedText *parent);
	virtual ~toSyntaxAnalyzerNL();

	virtual statementList getStatements(QString const& text);
	virtual statement getStatementAt(unsigned line, unsigned linePos);
	virtual QsciLexer* createLexer(QObject *parent);
	virtual void sanitizeStatement(statement&);

	static QSet<QString> SELECT_INTRODUCERS;
	static QSet<QString> DML_INTRODUCERS;
	static QSet<QString> DDL_INTRODUCERS;
	static QSet<QString> PLSQL_INTRODUCERS;
	static QSet<QString> SQLPLUS_INTRODUCERS;
};

/* This "analyzer" uses ANTLR lexer and implements QsciLexerCustom */
class toSyntaxAnalyzerOracle : public toSyntaxAnalyzer
{
	Q_OBJECT;
public:
	toSyntaxAnalyzerOracle(toHighlightedText *parent);
	virtual ~toSyntaxAnalyzerOracle();

	virtual statementList getStatements(QString const& text);
	virtual statement getStatementAt(unsigned line, unsigned linePos);
	virtual QsciLexer* createLexer(QObject *parent);
	virtual void sanitizeStatement(statement&);
};

#endif
