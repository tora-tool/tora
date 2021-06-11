
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

#include "editor/tosyntaxanalyzernl.h"
#include "core/toconfiguration.h"
#include "core/toeditorconfiguration.h"
#include "core/tostyle.h"
#include "editor/tosqltext.h"
#include "parsing/toscilexersql.h"

#include <QtCore/QString>
#include <QtCore/QtDebug>
#include <QRegExp>
#include <QRegularExpression>

toSyntaxAnalyzerNL::toSyntaxAnalyzerNL(toSqlText *parent)
    : toSyntaxAnalyzer(parent)
{}

toSyntaxAnalyzerNL::~toSyntaxAnalyzerNL()
{}


toSyntaxAnalyzer::statementList toSyntaxAnalyzerNL::getStatements(QString const& text)
{
    return getStatements(text, 0); // 0 => no line limit, return all statements
}

toSyntaxAnalyzer::statement toSyntaxAnalyzerNL::getStatementAt(unsigned line, unsigned linePos)
{
    // This code assumes that text starts with 1st line
    // Also QScintilla markers start from 1st line
    // But internally QScintilla enumerates from zero

    toSqlText *editor = qobject_cast<toSqlText *>(parent());
    QString text(editor->text());

    toSyntaxAnalyzer::statementList statements = getStatements(text, line);

    if (!statements.isEmpty() && statements.last().lineFrom <= line && statements.last().lineTo >= line)
        return statements.last();

    return toSyntaxAnalyzer::statement();
}

toSyntaxAnalyzer::statementList toSyntaxAnalyzerNL::getStatements(QString const& text, int lineLimit)
{
    QRegularExpression NL("\\r?\\n"); // TODO mac?, static variable can be used in both threads(can not be static)
    QRegExp WS("^\\s*$");
    QRegExp SEMI("^.*;\\s*(--\\s*)?$");

    toSyntaxAnalyzer::statementList retval;

    QStringList lines = text.split(NL);
    unsigned lineStart = 0, lineEnd = 0;
    unsigned lineNumber = 1;
    bool splitOnSemi = false;
    foreach(QString const &line, lines)
    {
        if ( WS.exactMatch(line))
        {
            // Empty line found
            if ( lineStart && lineEnd )
            {
                retval << statement(lineStart-1, lineEnd-1); // QScintilla lines start from 0th although reported as 1st
                lineStart = 0; // reset marker to initial value
                splitOnSemi = false; // reset this flag
            }
        }
        else
        {
            // Non-Empty line found
            if ( lineStart == 0)
            {
                // try to deduce statement type
                QStringList words = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                foreach(QString const &word, words)
                {
                    switch (statementClass(word))
                    {
                        case SELECT:
                        case DML:
                            splitOnSemi = true;
                            goto LINE_CLASIFIED;
                        case DDL:   // CREATE
                        case PLSQL: // DECLATE/BEGIN/CALL
                        case OTHER: // ALTER SESSION ..., ANALYZE, SET ROLE, EXPLAIN (never returned from statementClass)
                            splitOnSemi = false;
                            goto LINE_CLASIFIED;
                        case SQLPLUS:
                            splitOnSemi = false;
                            // TODO this is commented out as wo do not support sqlplus commands yet
                            // retval << statement(lineNumber, lineNumber);
                            // lineStart = 0;
                            goto LINE_IGNORED;
                        case UNKNOWN:
                        case COMMENT:
                            goto LINE_IGNORED;
                        default:
                            continue;
                    }
                    break; // iterate only once unless "continue" is called
                }
LINE_CLASIFIED:
                lineStart = lineNumber;
            }
LINE_IGNORED:
            lineEnd = lineNumber;
        }

        if(splitOnSemi && SEMI.exactMatch(line))
        {
            if ( lineStart && lineEnd )
            {
                retval << statement(lineStart-1, lineEnd-1);
                lineStart = 0;
            }
        }

        lineNumber++;

        // if lineLimit is defined and last detected statement if beyond this limit, return what we have
        if (lineLimit && !retval.isEmpty() && retval.last().lineTo >= lineLimit)
            return retval;
    }

    // Handle the last line (if non-empty)
    if ( lineStart && lineEnd )
    {
        retval << statement(lineStart-1, lineEnd-1);
    }

    return retval;
}

QsciLexer * toSyntaxAnalyzerNL::createLexer(QObject *parent)
{
    return new toSciLexerSQL(parent);
}

void toSyntaxAnalyzerNL::sanitizeStatement(statement &stat)
{
    toSqlText *editor = qobject_cast<toSqlText *>(parent());
    stat.posFrom = editor->SendScintilla(QsciScintilla::SCI_POSITIONFROMLINE, stat.lineFrom);
    stat.posTo = editor->SendScintilla(QsciScintilla::SCI_GETLINEENDPOSITION, stat.lineTo);

    char *buf = new char[stat.posTo - stat.posFrom + 1];

    QString lastWord;
    int wordStyle, lastPos;

    qDebug() << "- 0 -----------";
    editor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, stat.posFrom, stat.posTo, buf);
    qDebug() << buf;

    stat.firstWord = firstEditorWord(stat.posFrom, stat.posTo);
    qDebug() << stat.firstWord << "..." << lastWord;

    stat.statementType = statementClass(stat.firstWord);

    qDebug() << "- 1 -------------";

    lastWord = lastEditorWord(stat.posFrom, stat.posTo, lastPos); // lastPos is "out" variable

    qDebug() << "- 2 -------------";

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

QString toSyntaxAnalyzerNL::firstEditorWord(int posFrom, int posTo)
{
    toSqlText *editor = qobject_cast<toSqlText *>(parent());
    char *buf = new char[posTo - posFrom + 1];
    QString retval;

    // iterate QScintilla word-by-word and query it's style (forward)
    // stop at the 1st interesting word, like SELECT, CREATE, ... to detect statement type
    for (int pos = posFrom; pos < posTo; )
    {
        long end_pos = editor->SendScintilla(QsciScintilla::SCI_WORDENDPOSITION, pos, (long)false);

        int style = editor->SendScintilla(QsciScintilla::SCI_GETSTYLEAT, pos) & 0x1f;
        editor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, pos, end_pos, buf);

        qDebug() << buf << ':' << style;

        switch (style)
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
        retval = editor->convertTextS2Q(buf);
        break;
    }
    delete []buf;
    return retval;
}

QString toSyntaxAnalyzerNL::lastEditorWord(int posFrom, int posTo, int &lastPos)
{
    toSqlText *editor = qobject_cast<toSqlText *>(parent());
    char *buf = new char[posTo - posFrom + 1];
    QString retval;

    // iterate QScintilla word-by-word and query it's style (backwards)
    // stop at the last interesting word, to exclude ';' or '/'
    for (int pos = posTo; pos > posFrom; )
    {
        long start_pos = editor->SendScintilla(QsciScintilla::SCI_WORDSTARTPOSITION, pos, (long)false);

        editor->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, start_pos, pos, buf);

        int styleX = editor->SendScintilla(QsciScintilla::SCI_GETSTYLEAT, pos) & 0x1f;
        int style = editor->SendScintilla(QsciScintilla::SCI_GETSTYLEAT, start_pos) & 0x1f;

        qDebug() << buf << ':' << style;

        switch (style)
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
        retval = editor->convertTextS2Q(buf);
        lastPos = pos;
        break;
    }

    delete []buf;
    return retval;
}

toSyntaxAnalyzerNL::statementClassEnum toSyntaxAnalyzerNL::statementClass(QString const& firstWord)
{
    if (SELECT_INTRODUCERS.contains(firstWord.toUpper()))
        return SELECT;
    else if (DML_INTRODUCERS.contains(firstWord.toUpper()))
        return DML;
    else if ( DDL_INTRODUCERS.contains(firstWord.toUpper()))
        return DDL;
    else if ( PLSQL_INTRODUCERS.contains(firstWord.toUpper()))
        return PLSQL;
    else if ( SQLPLUS_INTRODUCERS.contains(firstWord.toUpper()))
        return SQLPLUS;
    else  if (firstWord.startsWith("--"))
        return COMMENT;
    else  if (firstWord.startsWith("/*"))
        return COMMENT;
    return UNKNOWN;
}

QSet<QString> toSyntaxAnalyzerNL:: SELECT_INTRODUCERS = QSet<QString>()
        << "SELECT"
        << "WITH"
        << "("     // "(select * from dual)"
        << "SHOW"; // MySQL

QSet<QString> toSyntaxAnalyzerNL:: DML_INTRODUCERS = QSet<QString>()
        //<< "SELECT"
        //<< "WITH"
        << "INSERT"
        << "UPDATE"
        << "DELETE"
        << "MERGE";

QSet<QString> toSyntaxAnalyzerNL::DDL_INTRODUCERS = QSet<QString>()  // see Oracle SQL reference (CREATE ... statement)
        << "CREATE"
        << "DROP"
        << "ANALYZE"
        << "ADMINISTER"	   
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
