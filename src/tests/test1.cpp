
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

//#include "core/utils.h"
//#include "parsing/tsqlparse.h"
#include "parsing/tsqllexer.h"
#include "parsing/persistenttrie.h"

#include <QtCore/QDateTime>
#include <QtGui/QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QString>
#include <QtCore/QTranslator>
#include <QtCore/QLibrary>
#include <QtCore/QDebug>

#include <memory>

using namespace QmlJS::PersistentTrie;

int main(int argc, char **argv)
{
	//toConfiguration::setQSettingsEnv();

	/*! \warning: Keep the code before QApplication init as small
	  as possible. There could be serious display issues when
	  you construct some Qt classes before QApplication.
	  It's the same for global static stuff - some instances can
	  break it (e.g. qscintilla lexers etc.).
	*/
	QApplication app(argc, argv);

	// Set the default codec to use for QString
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	try
	{
		const char input[] = "a b c aaa ab abc";
		qDebug() << "Lexer input:" << input;

		std::auto_ptr <SQLLexer::Lexer> lexer = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", input, "");
		QmlJS::PersistentTrie::Trie trie;

		for(int i = 0; i <= 10; i++)
		{
			std::cout << qPrintable(lexer->wordAt(SQLLexer::Position(0, i))) << std::endl;
		}
		
		SQLLexer::Lexer::token_const_iterator i = lexer->begin();
		while( i != lexer->end())
		{
			std::cout << '\'' << qPrintable(i->getText()) << '\'' << std::endl;
			trie.insert(i->getText());
			i++;
		}

		qDebug() << "Empty string completions";
		QStringList completions1 = trie.complete("", "", LookupFlags(CaseInsensitive));
		Q_FOREACH(QString s, completions1)
		{
			qDebug() << s;
		}

		qDebug() << "Completions for 'a'";
		QStringList completions2 = trie.complete("a", ".", LookupFlags(CaseInsensitive));
		Q_FOREACH(QString s, completions2)
		{
			qDebug() << s;
		}

		qDebug() << "Completions for 'ab'";
		QStringList completions3 = trie.complete("ab", ".", LookupFlags(CaseInsensitive));
		Q_FOREACH(QString s, completions3)
		{
			qDebug() << s;
		}
	}
	catch (const QString &str)
	{
		std::cerr << "Unhandled exception:"<< std::endl << std::endl << qPrintable(str) << std::endl;
	}
	return 1;
}
