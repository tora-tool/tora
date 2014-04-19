
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

#include "dotgrammar.h"
#include "dotgraph.h"
#include "DotGraphParsingHelper.h"

#include <boost/spirit/include/classic_confix.hpp>
#include <boost/throw_exception.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <QtCore/QString>

#include <stdlib.h>
#include <stdio.h>

using namespace std;

static void usage();
string slurp(string const& fileName);

extern DotGraphParsingHelper* phelper;

int main(int argc, char **argv)
{
	using namespace boost::spirit::classic;
	if (argc != 2)
		usage();

	string txt = slurp(string(argv[1]));

	cout << txt << endl;

	DotGraph newGraph(QString("xdot"), QString(argv[1]));
	phelper = new DotGraphParsingHelper;
	phelper->graph = &newGraph;
	phelper->z = 1;
	phelper->maxZ = 1;
	phelper->uniq = 0;

	DotGrammar g;
	//parse_info<> p_info= boost::spirit::classic::parse(txt.c_str(), g >> end_p, (+boost::spirit::classic::space_p|boost::spirit::classic::comment_p("/*", "*/")));
	parse_info<> p_info= boost::spirit::classic::parse(txt.c_str()
//			"[_draw_=\"c 9 -#fffffe00 C 7 -#ffffff P 4 0 0 0 36 54 36 54 0 \","
//            "bb=\"0,0,54,36\","
//            "fontsize=11, xdotversion=1.6,"
//            "id=unnamed]"
//            "xdotversion=1.6"
			, g >> end_p, (+boost::spirit::classic::space_p|boost::spirit::classic::comment_p("/*", "*/")));

	if (p_info.full)
		cout << "Parse OK" << endl;
	else
		cout << "Parse Fail at:" << p_info.length << " " << p_info.stop << endl;
}

static void usage()
{
	printf("Usage:\n\n  test6 filename\n\n");
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
