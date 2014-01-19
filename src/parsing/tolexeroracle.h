
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

#ifndef TOLEXERORACLE_H_
#define TOLEXERORACLE_H_

#include "parsing/tsqllexer.h"
#include "parsing/tosyntaxanalyzer.h"

#include <QtCore/QObject>
#include <Qsci/qscilexercustom.h>
#include <Qsci/qscilexersql.h>

class QsciStyle;
class QsciAbstractAPIs;

class toLexerOracle : public QsciLexerCustom {
        Q_OBJECT;
public:
        enum Style {
                Default    = toSyntaxAnalyzer::Default,     // white spaces only
                Comment    = toSyntaxAnalyzer::Comment,
                Reserved   = toSyntaxAnalyzer::Keyword,
                Builtin    = toSyntaxAnalyzer::KeywordSet5,
                Identifier = toSyntaxAnalyzer::Identifier,
                OneLine    = toSyntaxAnalyzer::PlusPrompt,
                CommentMultiline = toSyntaxAnalyzer::QuotedIdentifier + 1,
                Failure,
                MaxStyle
        };

        toLexerOracle(QObject *parent = 0);
        virtual ~toLexerOracle();

        /* override - reimplemented from QsciLexerCustom */
        virtual const char *language() const;
        virtual QString description(int) const;
        virtual void styleText(int start, int end);

        /* override - reimplemented from QsciLexer */
        QsciAbstractAPIs *apis() const;

        virtual bool caseSensitive() const
        {
            return false;
        }

        virtual const char *wordCharacters() const
        {
        	return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$_#0123456789:.";
        }

protected:
        char *lineText, *bufferText;
        unsigned lineLength, bufferLength;

        QMap<int,QString> styleNames;
    	QList<int> styleStack;
    	std::auto_ptr <SQLLexer::Lexer> lexer;
};

#endif /* TOLEXERORACLE_H_ */
