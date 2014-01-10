
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

#include "core/toconfiguration.h"
#include "parsing/tosyntaxanalyzer.h"
#include "editor/tohighlightedtext.h"

#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QtCore/QtDebug>

#include <Qsci/qscilexer.h>
#include <Qsci/qscilexersql.h>

toSyntaxAnalyzerNL::toSyntaxAnalyzerNL(toHighlightedText *parent)
	: toSyntaxAnalyzer(parent)
{}

toSyntaxAnalyzerNL::~toSyntaxAnalyzerNL()
{}

toSyntaxAnalyzer::statementList toSyntaxAnalyzerNL::getStatements(QString const& text)
{
    QRegExp NL("\\r?\\n"); // TODO mac?, static variable can be bused in both threads(can not be static)
    QRegExp WS("^\\s*$");

    toSyntaxAnalyzer::statementList retval;

	QStringList lines = text.split(NL);
	unsigned lineStart = 0, lineEnd = 0;
	unsigned lineNumber = 1;
	foreach(QString const &line, lines)
	{
		if( WS.exactMatch(line))
		{
			// Empty line found
			if( lineStart && lineEnd )
			{
				retval << statement(lineStart-1, lineEnd-1); // QScintilla lines start from 0th although reported as 1st
				lineStart = 0; // reset marker to initial value
			}
		} else {
			// Non-Empty line found
			if( lineStart == 0)
				lineStart = lineNumber;
			lineEnd = lineNumber;
		}

		lineNumber++;
	}

	// Handle the last line (if non-empty)
	if( lineStart && lineEnd )
	{
		retval << statement(lineStart-1, lineEnd-1);
	}

	return retval;
}

toSyntaxAnalyzer::statement toSyntaxAnalyzerNL::getStatementAt(unsigned line, unsigned linePos)
{
	// This code assumes that text starts with 1st line
	// Also QScintilla markers start from 1st line
	// But internally QScintilla enumerates from zero
	line++;

    static QRegExp NL("\\r?\\n"); // TODO mac?
    static QRegExp WS("^\\s*$");

    toSyntaxAnalyzer::statement retval;

	toHighlightedText *editor = qobject_cast<toHighlightedText *>(parent());
	QString text(editor->text());

	QStringList lines = text.split(NL);
	unsigned lineStart = 0, lineEnd = 0;
	unsigned lineNumber = 1;
	foreach(QString const &lineStr, lines)
	{
		if( WS.exactMatch(lineStr))
		{
			// Empty line found
			if( lineStart && lineEnd )
			{
				retval = statement(lineStart-1, lineEnd-1);
				lineStart = 0; // reset marker to initial value
			}
			if(lineNumber > line && retval.lineTo+1 >= line) // off-by-one offset retval.lineTo+1 >= line
				return retval;
		} else {
			// Non-Empty line found
			if( lineStart == 0)
				lineStart = lineNumber;
			lineEnd = lineNumber;
		}

		lineNumber++;
	}

	// Handle the last line (if non-empty)
	if( lineStart && lineEnd )
	{
		retval = statement(lineStart-1, lineEnd-1);
	}

	return retval;
}

QsciLexer * toSyntaxAnalyzerNL::createLexer(QObject *parent)
{
	QsciLexerSQL *retval = new QsciLexerSQL(parent);
    retval->setFoldComments(true);
    retval->setFoldCompact(false);

	QMetaEnum StyleNameEnum(ENUM_REF(toSyntaxAnalyzer,WordClassEnum));
	QsciLexerSQL *l = new QsciLexerSQL(NULL);
	for (int idx = 0; idx < StyleNameEnum.keyCount(); idx++)
	{
		int styleNameEnum = StyleNameEnum.value(idx);
		retval->setColor(toConfigurationSingle::Instance().styleFgColor(styleNameEnum), styleNameEnum);
		retval->setPaper(toConfigurationSingle::Instance().styleBgColor(styleNameEnum), styleNameEnum);
		retval->setFont(toConfigurationSingle::Instance().codeFontName(), styleNameEnum);
	}

	retval->setColor(toConfigurationSingle::Instance().styleFgColor(toSyntaxAnalyzer::Comment), QsciLexerSQL::CommentLine);
	retval->setColor(toConfigurationSingle::Instance().styleFgColor(toSyntaxAnalyzer::Comment), QsciLexerSQL::CommentLineHash);
	retval->setColor(toConfigurationSingle::Instance().styleFgColor(toSyntaxAnalyzer::Comment), QsciLexerSQL::PlusComment);
	retval->setColor(toConfigurationSingle::Instance().styleFgColor(toSyntaxAnalyzer::Comment), QsciLexerSQL::CommentDocKeyword);
	retval->setColor(toConfigurationSingle::Instance().styleFgColor(toSyntaxAnalyzer::Comment), QsciLexerSQL::CommentDocKeywordError);
	retval->setPaper(toConfigurationSingle::Instance().styleBgColor(toSyntaxAnalyzer::Comment), QsciLexerSQL::CommentLine);
	retval->setPaper(toConfigurationSingle::Instance().styleBgColor(toSyntaxAnalyzer::Comment), QsciLexerSQL::CommentLineHash);
	retval->setPaper(toConfigurationSingle::Instance().styleBgColor(toSyntaxAnalyzer::Comment), QsciLexerSQL::PlusComment);
	retval->setPaper(toConfigurationSingle::Instance().styleBgColor(toSyntaxAnalyzer::Comment), QsciLexerSQL::CommentDocKeyword);
	retval->setPaper(toConfigurationSingle::Instance().styleBgColor(toSyntaxAnalyzer::Comment), QsciLexerSQL::CommentDocKeywordError);

	retval->setFont(toConfigurationSingle::Instance().codeFontName());

    return retval;
}

void toSyntaxAnalyzerNL::sanitizeStatement(statement &stat)
{
	toHighlightedText *editor = qobject_cast<toHighlightedText *>(parent());
	stat.posFrom = editor->SendScintilla(QsciScintilla::SCI_POSITIONFROMLINE, stat.lineFrom);
	stat.posTo = editor->SendScintilla(QsciScintilla::SCI_GETLINEENDPOSITION, stat.lineTo);

    char *buf = new char[stat.posTo - stat.posFrom + 1];

	QString lastWord;
	int wordStyle, lastPos;

	qDebug() << "- 0 -----------";
	editor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, stat.posFrom, stat.posTo, buf);
	qDebug() << buf;

	// iterate QScintilla word-by-word and query it's style (forward)
	// stop at the 1st interesting word, like SELECT, CREATE, ... to detect statement type
	for(int pos = stat.posFrom; pos < stat.posTo; )
	{
	    long end_pos = editor->SendScintilla(QsciScintilla::SCI_WORDENDPOSITION, pos, (long)false);

	    int style = editor->SendScintilla(QsciScintilla::SCI_GETSTYLEAT, pos) & 0x1f;
	    editor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, pos, end_pos, buf);

	    qDebug() << buf << ':' << style;

	    switch(style)
	    {
	    case QsciLexerSQL::Default: // assuming Default=0 is used for white space only
	    case QsciLexerSQL::Comment:
	    case QsciLexerSQL::CommentLine:
	    case QsciLexerSQL::CommentDoc:
	    case QsciLexerSQL::PlusComment:
	    //case QsciLexerSQL::CommentLineHash: illegal for Oracle
	    case  QsciLexerSQL::CommentDocKeyword:
	    case  QsciLexerSQL::CommentDocKeywordError:
	    	pos = end_pos;
	    	continue;
	    }

	    editor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, pos, end_pos, buf);
	    stat.firstWord = editor->convertTextS2Q(buf);
	    wordStyle = style;
	    break;
	}

	qDebug() << "- 1 -------------";

	// iterate QScintilla word-by-word and query it's style (backwards)
	// stop at the last interesting word, to exclude ';' or '/'
	for(int pos = stat.posTo; pos > stat.posFrom; )
	{
	    long start_pos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, pos, (long)false);

		editor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, start_pos, pos, buf);

		int styleX = editor->SendScintilla(QsciScintilla::SCI_GETSTYLEAT, pos) & 0x1f;
	    int style = editor->SendScintilla(QsciScintilla::SCI_GETSTYLEAT, start_pos) & 0x1f;

	    qDebug() << buf << ':' << style;

	    switch(style)
	    {
	    case QsciLexerSQL::Default: // assuming Default=0 is used for white space only
	    case QsciLexerSQL::Comment:
	    case QsciLexerSQL::CommentLine:
	    case QsciLexerSQL::CommentDoc:
	    case QsciLexerSQL::PlusComment:
	    //case QsciLexerSQL::CommentLineHash: illegal for Oracle
	    case  QsciLexerSQL::CommentDocKeyword:
	    case  QsciLexerSQL::CommentDocKeywordError:
	    	pos = start_pos;
	    	continue;
	    }

	    editor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, start_pos, pos, buf);
	    lastWord = editor->convertTextS2Q(buf);
	    lastPos = pos;
	    break;
	}

	qDebug() << "- 2 -------------";
	qDebug() << stat.firstWord << "..." << lastWord;

	if (SELECT_INTRODUCERS.contains(stat.firstWord.toUpper()))
		stat.statementType = SELECT;
	else if (DML_INTRODUCERS.contains(stat.firstWord.toUpper()))
		stat.statementType = DML;
	else if( DDL_INTRODUCERS.contains(stat.firstWord.toUpper()))
		stat.statementType = DDL;
	else if( PLSQL_INTRODUCERS.contains(stat.firstWord.toUpper()))
		stat.statementType = PLSQL;
	else if( SQLPLUS_INTRODUCERS.contains(stat.firstWord.toUpper()))
		stat.statementType = SQLPLUS;

	// omit the trailing semicolon (PLSQL block)
	// TODO: this fails for CREATE TRIGGER or CREATE TYPE BODY (these statements end with: "END object_name? ;"
	if ( stat.statementType != PLSQL && lastWord.endsWith(";")) // "select * from (select * from dual);" the last word is ");"
		stat.posTo = editor->SendScintilla(QsciScintilla::SCI_POSITIONBEFORE, lastPos);
	else if (lastWord == "/")
		stat.posTo = editor->SendScintilla(QsciScintilla::SCI_POSITIONBEFORE, lastPos);

    editor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, stat.posFrom, stat.posTo, buf);
    stat.sql = editor->convertTextS2Q(buf);
    delete []buf;
}

QSet<QString> toSyntaxAnalyzerNL:: SELECT_INTRODUCERS = QSet<QString>()
		<< "SELECT"
		<< "WITH";

QSet<QString> toSyntaxAnalyzerNL:: DML_INTRODUCERS = QSet<QString>()
		//<< "SELECT"
		//<< "WITH"
		<< "INSERT"
		<< "UPDATE"
		<< "DELETE"
		<< "MERGE"
		<< "(";   // "(select * from dual)"

QSet<QString> toSyntaxAnalyzerNL::DDL_INTRODUCERS = QSet<QString>()  // see Oracle SQL reference (CREATE ... statement)
		<< "CREATE"
		<< "DROP"
		<< "ANALYZE"
		<< "ALTER"
		<< "COMMENT"
		<< "CLUSTER"
		<< "CONTEXT"
		<< "DATABASE"
		<< "DIMENSION"
		<< "DIRECTORY"
		<< "DISKGROUP"
		<< "EDITION"
		<< "FLASHBACK"
		<< "FUNCTION"
		<< "INDEX"
		<< "INDEXTYPE"
		<< "JAVA"
		<< "LIBRARY"
		<< "MATERIALIZED"
		<< "OPERATOR"
		<< "OUTLINE"
		<< "PACKAGE"
		<< "PROCEDURE"
		<< "PROFILE"
		<< "RESTORE"
		<< "ROLE"
		<< "ROLLBACK"
		<< "SEQUENCE"
		<< "SYNONYM"
		<< "TABLE"
		<< "TABLESPACE"
		<< "TRIGGER"
		<< "TYPE"
		<< "VIEW"
		<< "USER"
		<< "VIEW";

QSet<QString> toSyntaxAnalyzerNL::PLSQL_INTRODUCERS = QSet<QString>()
		<< "DECLARE"
		<< "BEGIN"
		<< "CALL";

QSet<QString> toSyntaxAnalyzerNL::SQLPLUS_INTRODUCERS = QSet<QString>()
		<< "REM"
		<< "ASSIGN"
		<< "PROMPT"
		<< "COLUMN"
		<< "SPOOL"
		<< "STORE"
		<< "SET"
		<< "DEFINE"
		<< "EXEC"
		<< "UNDEFINE";
