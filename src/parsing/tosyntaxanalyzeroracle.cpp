
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "parsing/tosyntaxanalyzer.h"
#include "parsing/tolexeroracle.h"
#include "editor/tohighlightedtext.h"

#include <QtCore/QDebug>

#include <Qsci/qscilexer.h>
#include <Qsci/qscilexersql.h>

#include <iostream>

toSyntaxAnalyzerOracle::toSyntaxAnalyzerOracle(toHighlightedText* parent)
	: toSyntaxAnalyzer(parent)
{
}

toSyntaxAnalyzerOracle::~toSyntaxAnalyzerOracle()
{
}

toSyntaxAnalyzer::statementList toSyntaxAnalyzerOracle::getStatements(const QString& text)
{
	toSyntaxAnalyzer::statementList retval;
	std::string str(text.toStdString());
	try {
		std::auto_ptr <SQLLexer::Lexer> lexer = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", "", "toCustomLexer");
		lexer->setStatement(str.c_str(), str.length());

		SQLLexer::Lexer::token_const_iterator start = lexer->begin();
		start = lexer->findStartToken(start);
		while(start->getTokenType() != SQLLexer::Token::X_EOF)
		{
			SQLLexer::Lexer::token_const_iterator end = lexer->findEndToken(start);
			retval << statement(
					start->getPosition().getLine(),
					end->getPosition().getLine());
			start = lexer->findStartToken(end);
		}
	} catch(std::exception const &e) {
		std::string s(e.what());
		std::cout << s << std::endl;
	} catch(QString const& e) {
		qDebug() << e;
	} catch(...) {
		qDebug() << __FUNCTION__ ;
	}
	return retval;
}

toSyntaxAnalyzer::statement toSyntaxAnalyzerOracle::getStatementAt(unsigned line, unsigned linePos)
{
	toSyntaxAnalyzer::statement retval;

	toHighlightedText *editor = qobject_cast<toHighlightedText *>(parent());
	std::string str(editor->text().toStdString());
	try {
		std::auto_ptr <SQLLexer::Lexer> lexer = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", "", "toCustomLexer");
		lexer->setStatement(str.c_str(), str.length());

		SQLLexer::Lexer::token_const_iterator start = lexer->begin();
		start = lexer->findStartToken(start);
		while(start->getTokenType() != SQLLexer::Token::X_EOF)
		{
			SQLLexer::Lexer::token_const_iterator end = lexer->findEndToken(start);
			SQLLexer::Lexer::token_const_iterator nextStart = lexer->findStartToken(end);
			if(end->getPosition().getLine() < line)
			{
				start = nextStart;
				continue;
			}

			// The statement was found - setup retval
			toHighlightedText *editor = qobject_cast<toHighlightedText *>(parent());
			retval = statement(
					start->getPosition().getLine(),
					end->getPosition().getLine());

			retval.firstWord = start->getText();
			retval.posFrom = editor->positionFromLineIndex(start->getPosition().getLine()
					, start->getPosition().getLinePos());
			retval.posTo   = editor->positionFromLineIndex(end->getPosition().getLine()
					, end->getPosition().getLinePos() + ( end->getTokenType() == SQLLexer::Token::X_EOL ? 0 : end->getLength()));
			switch(start->getTokenType())
			{
			case SQLLexer::Token::L_LPAREN:
			case SQLLexer::Token::L_DML_INTRODUCER:		// INSERT/UPDATE/DELETE/MERGE
				retval.statementType = DML;
				break;
			case SQLLexer::Token::L_SELECT_INTRODUCER:
				retval.statementType = SELECT;
				break;
			case SQLLexer::Token::L_PL_INTRODUCER:
				retval.statementType = PLSQL;
				break;
			case SQLLexer::Token::L_OTHER_INTRODUCER:
				retval.statementType = OTHER;
				break;
			case SQLLexer::Token::X_ONE_LINE:
				retval.statementType = SQLPLUS;
				break;
			default:
				//	        DDL,        // CREATE
				//	        OTHER,      // ALTER SESSION ..., ANALYZE, SET ROLE, EXPLAIN
				//	        SQLPLUS		// sqlplus command
				retval.statementType = UNKNOWN;
				break;
			}
			return retval;
		}
	} catch(std::exception const &e) {
		std::string s(e.what());
		std::cout << s << std::endl;
	} catch(QString const& e) {
		qDebug() << e;
	} catch(...) {
		qDebug() << __FUNCTION__ ;
	}
	return retval;
}

QsciLexer* toSyntaxAnalyzerOracle::createLexer(QObject* parent)
{
	QsciLexer *retval = new toLexerOracle(parent);
	return retval;
}

void toSyntaxAnalyzerOracle::sanitizeStatement(statement& stat)
{
	toHighlightedText *editor = qobject_cast<toHighlightedText *>(parent());
	char *buf = new char[stat.posTo - stat.posFrom + 1];
    editor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, stat.posFrom, stat.posTo, buf);
    stat.sql = editor->convertTextS2Q(buf);
    delete []buf;
}
