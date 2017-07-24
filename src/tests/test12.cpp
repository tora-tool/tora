
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

#include <QtCore/QCoreApplication>

//#include "core/utils.h"
#include "core/tora_export.h"
#include "core/tologger.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"
#include "parsing/tsqlparse.h"

#include <iostream>
#include <ostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <fstream>

using namespace std;
using namespace SQLParser;

string slurp(string const& fileName);
void unslurp(string const& fileName, string const& what);

static void usage();
void toASTWalker(SQLParser::Statement &source, const std::function<void(Statement &source, Token const& none)>& visitor);

unsigned emptyAliasCnt = 0;
QMap<QString, int> tableNameEnumerator;
std::function<void(Statement &source, Token const& n)> token_print = [](Statement &source, Token const&node)
{
	using namespace std;
	cout << setw(4) << node.depth() << '|'
	<< setw(node.depth()*4) << node.toString()
	<< endl;
};

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	if (argc != 2)
		usage();

	string sql = slurp(string(argv[1]));
	QString qsql = QString::fromStdString(sql);

	try
	{
		unique_ptr<SQLParser::Statement> parser = StatementFactTwoParmSing::Instance().create("OracleDML", qsql, "");
		parser->dumpTree();
		
		cout << "================================================================================" << endl;
		string deserialized(qPrintable(parser->root()->toStringRecursive(true)));
		toASTWalker(*parser, token_print);

		cout << "================================================================================" << endl;
		cout << parser->dot;
	}
	catch (const ParseException &pe)
	{
		return -2;
	}	
	catch (const QString &str)
	{
		cerr << "Unhandled exception:"<< endl << endl << qPrintable(str) << endl;
		return -1;
	}
	return 1;
}

static void usage()
{
	printf("Usage:\n\n  test12 filename\n\n");
	exit(2);
}

string slurp(string const& fileName)
{
	ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);
	ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, ios::beg);

	stringstream sstr;
	sstr << ifs.rdbuf();
	return sstr.str();
}

void toASTWalker(Statement &source, const std::function<void(Statement &source,Token const& n)>& visitor)
{
	SQLParser::Statement::token_const_iterator node;
	for (node = source.begin(); node != source.end(); ++node)
	{
		visitor(source, *node);
	}
}
