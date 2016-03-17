
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

#include "core/tostyle.h"

#include <Qsci/qsciapis.h>
#include <Qsci/qscilexersql.h>

void toStylesMap::updateLexer(QsciLexer *lexer)
{
    Q_FOREACH(int key, keys())
    {
        lexer->setColor(value(key).FGColor, key);
        lexer->setPaper(value(key).BGColor, key);
        lexer->setFont(value(key).Font, key);
    }

    // Treat all comment style as one
    lexer->setColor(value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentLine);
    lexer->setColor(value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentLineHash);
    lexer->setColor(value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::PlusComment);
    lexer->setColor(value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentDocKeyword);
    lexer->setColor(value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentDocKeywordError);
    lexer->setColor(value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::QuotedIdentifier+1); // See tolexeroracle.h:58

    lexer->setPaper(value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentLine);
    lexer->setPaper(value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentLineHash);
    lexer->setPaper(value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::PlusComment);
    lexer->setPaper(value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentDocKeyword);
    lexer->setPaper(value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentDocKeywordError);
    lexer->setPaper(value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::QuotedIdentifier+1);

    lexer->setFont(value(QsciLexerSQL::Comment).Font, QsciLexerSQL::CommentLine);
    lexer->setFont(value(QsciLexerSQL::Comment).Font, QsciLexerSQL::CommentLineHash);
    lexer->setFont(value(QsciLexerSQL::Comment).Font, QsciLexerSQL::PlusComment);
    lexer->setFont(value(QsciLexerSQL::Comment).Font, QsciLexerSQL::CommentDocKeyword);
    lexer->setFont(value(QsciLexerSQL::Comment).Font, QsciLexerSQL::CommentDocKeywordError);
    lexer->setFont(value(QsciLexerSQL::Comment).Font, QsciLexerSQL::QuotedIdentifier+1);

    // see QsciScintilla::setPaper
    // There are two "default" styles
    // 0  - used for white spaces only
    // 32 - used otherwise(used or empty paper's color), but this one can not be set when NOT using custom lexer
    lexer->setPaper(value(QsciLexerSQL::Default).BGColor, QsciScintillaBase::STYLE_DEFAULT);

    // be sure the same font is used
    lexer->setFont(value(QsciLexerSQL::Comment).Font);
    lexer->setDefaultFont(value(QsciLexerSQL::Comment).Font);
}
