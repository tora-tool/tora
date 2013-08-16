
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

#include "core/utils.h"
#include "core/tologger.h"

#include "core/toconf.h"
#include "core/tomain.h"
#include "core/tosql.h"
#include "core/toconfiguration.h"
#include "core/toqvalue.h"
#include "core/toconnection.h"
#include "core/toraversion.h"

//#include "parsing/tsqlparse.h"
#include "parsing/tsqllexer.h"

#include <QtCore/QDateTime>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtCore/QTextCodec>
#include <QtCore/QString>
#include <QtCore/QTranslator>
#include <QtGui/QStyleFactory>
#include <QtCore/QLibrary>

#include <memory>

int main(int argc, char **argv)
{
	toConfiguration::setQSettingsEnv();

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
		QLibrary parsing("parsing");
		parsing.load();
		
		std::auto_ptr <SQLLexer::Lexer> lexer = LexerFactTwoParmSing::Instance().create("OracleSQL", "a b c", "");

		for(int i = 0; i <= 10; i++)
		{
			std::cout << lexer->wordAt(SQLLexer::Position(0, i)) << std::endl;
		}
		
		for(int i = 10; i >= 0; i--)
		{
			std::cout << lexer->wordAt(SQLLexer::Position(0, i)) << std::endl;
		}

	}
	catch (const QString &str)
	{
		std::cerr << "Unhandled exception:"<< std::endl << std::endl << qPrintable(str) << std::endl;
		TOMessageBox::critical(NULL,
				       qApp->translate("main", "Unhandled exception"),
				       str,
				       qApp->translate("main", "Exit"));
	}
	return 1;
}
