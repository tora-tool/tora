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

#include "parsing/toscilexersql.h"
#include "core/toeditorconfiguration.h"
#include "core/tostyle.h"
#include "core/toconfiguration.h"

#include <Qsci/qsciscintillabase.h>

toSciLexerSQL::toSciLexerSQL(QObject *parent)
	: QsciLexerSQL(parent)
{

    toStylesMap stylesMap = toConfigurationNewSingle::Instance().option(ToConfiguration::Editor::EditStyleMap).value<toStylesMap>();
    QFont font(stylesMap.value(QsciLexerSQL::Default).Font);
    Q_FOREACH(int key, stylesMap.keys())
    {
    	QsciLexerSQL::setColor(stylesMap.value(key).FGColor, key);
    	QsciLexerSQL::setPaper(stylesMap.value(key).BGColor, key);
    	QsciLexerSQL::setFont(font, key);
    }

    // Treat all comment style as one
    QsciLexerSQL::setColor(stylesMap.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentLine);
    QsciLexerSQL::setColor(stylesMap.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentLineHash);
    QsciLexerSQL::setColor(stylesMap.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::PlusComment);
    QsciLexerSQL::setColor(stylesMap.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentDocKeyword);
    QsciLexerSQL::setColor(stylesMap.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::CommentDocKeywordError);
    QsciLexerSQL::setColor(stylesMap.value(QsciLexerSQL::Comment).FGColor, QsciLexerSQL::QuotedIdentifier+1); // See tolexeroracle.h:58

    QsciLexerSQL::setPaper(stylesMap.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentLine);
    QsciLexerSQL::setPaper(stylesMap.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentLineHash);
    QsciLexerSQL::setPaper(stylesMap.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::PlusComment);
    QsciLexerSQL::setPaper(stylesMap.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentDocKeyword);
    QsciLexerSQL::setPaper(stylesMap.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::CommentDocKeywordError);
    QsciLexerSQL::setPaper(stylesMap.value(QsciLexerSQL::Comment).BGColor, QsciLexerSQL::QuotedIdentifier+1);

    QsciLexerSQL::setFont(font, QsciLexerSQL::CommentLine);
    QsciLexerSQL::setFont(font, QsciLexerSQL::CommentLineHash);
    QsciLexerSQL::setFont(font, QsciLexerSQL::PlusComment);
    QsciLexerSQL::setFont(font, QsciLexerSQL::CommentDocKeyword);
    QsciLexerSQL::setFont(font, QsciLexerSQL::CommentDocKeywordError);
    QsciLexerSQL::setFont(font, QsciLexerSQL::QuotedIdentifier+1);

    // see QsciScintilla::setPaper
    // There are two "default" styles
    // 0  - used for white spaces only
    // 32 - used otherwise(used or empty paper's color), but this one can not be set when NOT using custom lexer
    QsciLexerSQL::setPaper(stylesMap.value(QsciLexerSQL::Default).BGColor, QsciScintillaBase::STYLE_DEFAULT);

    // be sure the same font is used
    QsciLexerSQL::setFont(font);
    QsciLexerSQL::setDefaultFont(font);

    QsciLexerSQL::setFoldComments(true);
    QsciLexerSQL::setFoldCompact(false);
}
