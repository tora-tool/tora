
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

#include "editor/tocustomlexer.h"
#include "parsing/tsqllexer.h"
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

toCustomLexer::toCustomLexer(QObject *parent)
        : QsciLexerCustom(parent)
        , lexer(LexerFactTwoParmSing::Instance().create("OracleGuiLexer", "", "toCustomLexer - OracleGuiLexer"))
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

	styleStack = QList<int>();
	resetStyle ();

	declareStyle(OneLine,
			QColor(Qt::black),
			QColor(Qt::cyan),
			mono);
	declareStyle(OneLineAlt,
			QColor(Qt::black),
			QColor(Qt::magenta),
			mono);
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
	declareStyle(String,
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

	thread = new QThread(this);
	worker = new toCustomLexerWorker(NULL);
	worker->moveToThread(thread);
	//connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
	//connect(thread, SIGNAL(started()),  worker, SLOT(process()));
	connect(this, SIGNAL(parsingRequested(const char*, unsigned)),  worker, SLOT(process(const char*, unsigned)));
	connect(worker, SIGNAL(processed()), this, SLOT(processed()));
	connect(worker, SIGNAL(finished()),  thread, SLOT(quit()));
	connect(worker, SIGNAL(finished()),  worker, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()),  thread, SLOT(deleteLater()));
	thread->start();

	schedule();
}

toCustomLexer::~toCustomLexer()
{
	thread->quit();
	qDebug() << __FUNCTION__;
}

const char* toCustomLexer::language() const
{
        return "MyLexer";
}

QString toCustomLexer::description(int style) const
{
	if (styleNames.contains(style)) {
		return styleNames[style];
	} else {
		return QString("");
	}
}

void toCustomLexer::setOracle()
{
    lexer = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", "", "toCustomLexer");
    styleText(0, editor()->text().length()-1);
}

void toCustomLexer::setMySQL()
{
    lexer = LexerFactTwoParmSing::Instance().create("mySQLGuiLexer", "", "toCustomLexer");
    styleText(0, editor()->text().length()-1);
}

void toCustomLexer::styleText(int start, int end)
{
	QString source;
	int len = end - start;

	if (!editor())
		return;

	resetStyle();

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
			setStyling(len2, Default);
			break;
		case SQLLexer::Token::X_COMMENT:
			setStyling(len2, Comment);
			break;
		case SQLLexer::Token::X_COMMENT_ML:
			setStyling(len2, CommentMultiline);
			break;
		case SQLLexer::Token::L_SELECT_INTRODUCER:
		case SQLLexer::Token::L_DML_INTRODUCER:
		case SQLLexer::Token::L_DDL_INTRODUCER:
		case SQLLexer::Token::L_PL_INTRODUCER:
		case SQLLexer::Token::L_OTHER_INTRODUCER:
		case SQLLexer::Token::L_RESERVED:
			setStyling(len2, Reserved);
			break;
		case SQLLexer::Token::L_BUILDIN:
			setStyling(len2, Builtin);
			break;
		case SQLLexer::Token::L_STRING:
			setStyling(len2, String);
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

bool toCustomLexer::pushStyle(int style)
{
	bool pushed = false;

	if (style >= getStyle()) {
		styleStack << style;
		pushed = true;
		qDebug() << __FUNCTION__ << description(style);
	}
	return pushed;
}

bool toCustomLexer::popStyle()
{
	int style;
	bool popped = false;
	if (styleStack.size() > 1) {
		style = styleStack.takeLast() ;
		popped = true;
		qDebug() << __FUNCTION__ << description(style);
	}
	return popped;
}

int toCustomLexer::getStyle()
{
	return styleStack.last();
}

bool toCustomLexer::hasStyle(int style)
{
	return styleStack.contains(style);
}

void toCustomLexer::resetStyle()
{
	styleStack.clear();
	styleStack << Default;
}

void toCustomLexer::process()
{
	emit parsingStarted();
	int len = editor()->length();
	if(bufferLength < len)
	{
		bufferLength = Utils::toNextPowerOfTwo(len);
		bufferText = (char*) realloc(bufferText, bufferLength);
	}
	editor()->SendScintilla(QsciScintillaBase::SCI_GETTEXT, bufferLength, bufferText);
	bufferText[len] = '\0';
	emit parsingRequested(bufferText, len);
}

void toCustomLexer::processed()
{
	Style style = OneLine;
	unsigned lastLine = 0;
	Q_FOREACH(const StmtLineRange &r, worker->statements)
	{
		// "clear" line numbers before the statement
		while(lastLine < r.first)
		{
			editor()->setMarginText(lastLine++, QString(), Default);
		}

		// "draw" line numbers for the sql statement
		for(unsigned int i=r.first, j=1; i <= r.second; ++i, ++j)
		{
			editor()->setMarginText(i, QString::number(j), style);
		}
		lastLine = r.second + 1;
		style = style == OneLine ? OneLineAlt : OneLine;
	}

	// "clear" line numbers after the last statement
	while(lastLine < editor()->lines())
	{
		editor()->setMarginText(lastLine++, QString(), Default);
	}

	emit parsingFinished();
	schedule();
};

void toCustomLexer::schedule()
{
	QTimer::singleShot(5000, this, SLOT(process()));
}

toCustomLexerWorker::toCustomLexerWorker(QObject *parent)
	: QObject(parent)
{

}

toCustomLexerWorker::~toCustomLexerWorker()
{

}

void toCustomLexerWorker::process(const char*text, unsigned len)
{

	try {
		std::auto_ptr <SQLLexer::Lexer> lexer = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", "", "toCustomLexer");
		lexer->setStatement(text, len);
		
		statements.clear();
		
		SQLLexer::Lexer::token_const_iterator start = lexer->begin();
		start = lexer->findStartToken(start);
		while(start->getTokenType() != SQLLexer::Token::X_EOF)
		{
			SQLLexer::Lexer::token_const_iterator end = lexer->findEndToken(start);
			statements << StmtLineRange(
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
		qDebug() << "aaa";
	}
	emit processed();
//	for(unsigned i = 1; i < 150; ++i)
//	{
//		if( i % 7 == 1)
//		{
//			start = i;
//			for(; i < 150; ++i)
//			{
//				if( i % 13 == 1)
//				{
//					end = i;
//					statements << StmtLineRange(start,end);
//					break;
//				}
//			}
//		}
//	}

}
