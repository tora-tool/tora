
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

#include "parsing/tolexeroracle.h"
#include "core/utils.h"
#include "core/toconfiguration.h"

#include <QtCore/QDebug>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscistyle.h>

#include <iostream>

#define declareStyle(style,color, paper, font) styleNames[style] = tr(#style); \
setColor(color, style); \
setPaper(paper, style); \
setFont(font, style);

toLexerOracle::toLexerOracle(QObject *parent)
        : QsciLexerCustom(parent)
        , lexer(LexerFactTwoParmSing::Instance().create("OracleGuiLexer", "", "toLexerOracle - OracleGuiLexer"))
		, lineLength(32)
		, bufferLength(1024)
		, lineText(NULL)
		, bufferText(NULL)
{
	QFont mono;
#if defined(Q_OS_WIN)
	mono = QFont("Courier New", 10);
#elif defined(Q_OS_MAC)
	mono = QFont("Courier", 12);
#else
	mono = QFont(Utils::toStringToFont(toConfigurationSingle::Instance().codeFontName()));
#endif

	declareStyle(Default,
			QColor(0x0, 0x0, 0x0),
			QColor("white"),
			mono);
	declareStyle(Comment,
			QColor(0x0, 0x80, 0x0),
			QColor("white"),
			mono);
	declareStyle(CommentMultiline,
			QColor(0x0, 0x80, 0x0),
			QColor("white"),
			mono);
	declareStyle(Reserved,
			QColor(0x80, 0x0, 0x0),
			QColor("white"),
			mono);
	declareStyle(Builtin,
			QColor(Qt::green),
			QColor("white"),
			mono);
	declareStyle(Identifier,
			QColor(Qt::darkBlue),
			QColor("white"),
			mono);
	declareStyle(Failure,
			QColor(Qt::black),
			QColor(Qt::red),
			mono);

	lineText = (char*) malloc(lineLength);
	if(lineText == NULL)
		throw QString("Buffer allocation error");

	bufferText = (char*) malloc(bufferLength);
	if(bufferText == NULL)
		throw QString("Buffer allocation error");
}

toLexerOracle::~toLexerOracle()
{
	free(lineText);
	free(bufferText);
	qDebug() << __FUNCTION__;
}

const char* toLexerOracle::language() const
{
        return "Oracle";
}

QString toLexerOracle::description(int style) const
{
	if (styleNames.contains(style)) {
		return styleNames[style];
	} else {
		return QString("");
	}
}

void toLexerOracle::styleText(int start, int end)
{
	QString source;
	int len = end - start;

	if (!editor())
		return;

	if( lineLength < (end - start + 1)) // +1 for 0x00
	{
		lineLength = Utils::toNextPowerOfTwo(end - start + 1);
		lineText = (char*) realloc(lineText, lineLength);
	}
	editor()->SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, start, end, lineText);
	source = QString::fromUtf8(lineText, len);
	lexer->setStatement(lineText, len);

	int line = editor()->SendScintilla(QsciScintilla::SCI_LINEFROMPOSITION, start);
	int state = -1;
	if ( line > 0)
	{
		// the previous state may be needed for multi-line styling
		int pos = editor()->SendScintilla(QsciScintilla::SCI_GETLINEENDPOSITION, line -1);
		state = editor()->SendScintilla(QsciScintilla::SCI_GETSTYLEAT, pos);
	}

    	//qDebug() << '[' << line << ',' << start << "," << end << ' ' << lexer->firstWord() << "] " << "source =" << source << '"';

	startStyling(start, 0x1f);

	SQLLexer::Lexer::token_const_iterator i = lexer->begin();
	unsigned offset = 0;
	if( state == CommentMultiline)
	{
		for(; i != lexer->end(); ++i)
		{
			//Q_ASSERT_X( , qPrintable(__QHERE__), "Pos1");
			SQLLexer::Token const &node = *i;
			unsigned len2 = node.getLength();
			setStyling(len2, CommentMultiline); offset += len2;
			if( node.getTokenType() == SQLLexer::Token::X_COMMENT_ML_END)
			{
				++i;
				break;
			}
		}
	} else if ( (*i).getTokenType() == SQLLexer::Token::X_ONE_LINE) {
		for(; i != lexer->end(); ++i)
		{
			SQLLexer::Token const &node = *i;
			if( node.getTokenType() == SQLLexer::Token::X_EOL || node.getTokenType() == SQLLexer::Token::X_EOF)
			{
				++i;
				break;
			}
			unsigned len2 = node.getLength();
			setStyling(len2, OneLine); offset += len2;
		}
	}

	for(; i != lexer->end(); )
	{
		SQLLexer::Token const &node = *i;
		unsigned len2 = node.getLength();
		//qDebug() << '\t' << len2 << ' ' << node.getTokenType() << ' ' << state;

		switch( node.getTokenType())
		{
		case SQLLexer::Token::X_WHITE:
			setStyling(len2, Default);
			break;
		case SQLLexer::Token::X_UNASSIGNED:
		case SQLLexer::Token::L_IDENTIFIER:
			setStyling(len2, Identifier);
			break;
		case SQLLexer::Token::X_COMMENT:
			setStyling(len2, Comment);
			break;
		case SQLLexer::Token::X_COMMENT_ML:
			setStyling(len2, CommentMultiline);
			break;
		case SQLLexer::Token::L_SELECT_INTRODUCER:
		case SQLLexer::Token::L_DML_INTRODUCER:
		case SQLLexer::Token::L_PL_INTRODUCER:
		case SQLLexer::Token::L_OTHER_INTRODUCER:
		case SQLLexer::Token::L_RESERVED:
			setStyling(len2, Reserved);
			break;
		case SQLLexer::Token::L_BUILDIN:
			setStyling(len2, Builtin);
			break;
		case SQLLexer::Token::L_STRING:
			setStyling(len2, Identifier);
			break;
		case SQLLexer::Token::X_FAILURE:
			setStyling(len2, Failure);
			break;
		case SQLLexer::Token::X_EOF:
			break;
		default:
			setStyling(len2, Default);
		}
		offset += len2;
        SQLLexer::Token pnode(*i);
		i++;
        SQLLexer::Token const &nnode = *i;
        if(nnode.getTokenType() == SQLLexer::Token::X_EOF)
            break;
        Q_ASSERT_X( nnode.getPosition() > pnode.getPosition(), qPrintable(__QHERE__), "Token position");
	}

//	startStyling(start, 0x1f);
//	setStyling(len, getStyle());
}
