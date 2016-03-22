
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

#include "core/tosyntaxanalyzer.h"
#include "core/tostyle.h"
#include "core/tologger.h"

#include <QtCore/QString>
#include <QtCore/QRegExp>

#include <Qsci/qscilexer.h>
#include <Qsci/qscilexersql.h>

toSyntaxAnalyzer::statement::statement()
    : lineFrom(0)
    , lineTo(0)
    , posFrom(0)
    , posTo(0)
    , sql()
    , firstWord()
    , statementType(UNKNOWN) {};
toSyntaxAnalyzer::statement::statement(unsigned lf, unsigned lt)
    : lineFrom(lf)
    , lineTo(lt)
    , posFrom(0)
    , posTo(0)
    , sql()
    , firstWord()
    , statementType(UNKNOWN) {};
toSyntaxAnalyzer::statement& toSyntaxAnalyzer::statement::operator=(toSyntaxAnalyzer::statement const& other)
{
    lineFrom = other.lineFrom;
    lineTo = other.lineTo;
    posFrom = other.posFrom;
    posTo = other.posTo;
    firstWord = other.firstWord;
    sql = other.sql;
    statementType = other.statementType;
    return *this;
}

//toSyntaxAnalyzer::toSyntaxAnalyzer(toSqlText *parent)
toSyntaxAnalyzer::toSyntaxAnalyzer(QObject *parent)
    : QObject(parent)
{
}

QColor toSyntaxAnalyzer::getColor(toSyntaxAnalyzer::WordClassEnum type) const
{
    if (Colors.contains(type))
        return Colors[type];
    throw QString("Unknown color reference: %1").arg(type);
}

toSyntaxAnalyzer::~toSyntaxAnalyzer()
{
}

#if 0
void toSyntaxAnalyzer::updateLexerStyles(QsciLexer *lexer, toStylesMap const& map)
{
	TLOG(8, toDecorator, __HERE__) << "  Lexer:" << lexer->language() << std::endl;
	QFont font = map.value(QsciLexerSQL::Default).Font;
	QString fontName = font.toString();
    Q_FOREACH(int key, map.keys())
    {
        lexer->setColor(map.value(key).FGColor, key);
        lexer->setPaper(map.value(key).BGColor, key);
        lexer->setFont(font, key);

#ifdef QT_DEBUG
        QColor c(map.value(key).FGColor);
        QColor p(map.value(key).BGColor);
        QFont  f(map.value(key).Font);
        TLOG(8, toNoDecorator, __HERE__) << "  Style:" << key << '\t'
                                         //<< std::endl
                                         //<< "   Fore:" << c.name() << '(' << c.red() << ' ' << c.green() << ' ' << c.blue() << ' ' << c.alpha() << ')' << std::endl
                                         //<< "   Back:" << p.name() << '(' << p.red() << ' ' << p.green() << ' ' << p.blue() << ' ' << p.alpha() << ')' << std::endl
                                         << "   Font:" << f.toString() << std::endl;
#endif
    }
#if 0
    // Treat all comment style as one
    lexer->setColor(map.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentLine);
    lexer->setColor(map.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentLineHash);
    lexer->setColor(map.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::PlusComment);
    lexer->setColor(map.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentDocKeyword);
    lexer->setColor(map.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentDocKeywordError);
    lexer->setColor(map.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::QuotedIdentifier+1); // See tolexeroracle.h:58

    lexer->setPaper(map.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentLine);
    lexer->setPaper(map.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentLineHash);
    lexer->setPaper(map.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::PlusComment);
    lexer->setPaper(map.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentDocKeyword);
    lexer->setPaper(map.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentDocKeywordError);
    lexer->setPaper(map.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::QuotedIdentifier+1);
#endif

    lexer->setFont(font, QsciLexerSQL::CommentLine);
    lexer->setFont(font, QsciLexerSQL::CommentLineHash);
    lexer->setFont(font, QsciLexerSQL::PlusComment);
    lexer->setFont(font, QsciLexerSQL::CommentDocKeyword);
    lexer->setFont(font, QsciLexerSQL::CommentDocKeywordError);
    lexer->setFont(font, QsciLexerSQL::QuotedIdentifier+1);

    // see QsciScintilla::setPaper
    // There are two "default" styles
    // 0  - used for white spaces only
    // 32 - used otherwise(used or empty paper's color), but this one can not be set when NOT using custom lexer
    lexer->setPaper(map.value(QsciLexerSQL::Default).BGColor, QsciScintillaBase::STYLE_DEFAULT);

    // be sure the same font is used
    lexer->setFont(font);
    lexer->setDefaultFont(font);
}
#endif
