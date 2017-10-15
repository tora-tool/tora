
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
void toASTWalk(SQLParser::Statement &source, bool (*filter)(Token const&node));
	
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
		toASTWalk(*parser, [](Token const&node)
			  {
				  return node.getTokenType() == Token::S_SUBQUERY_FACTORED;
			  }
			);
		cout << "================================================================================" << endl;
		string deserialized(qPrintable(parser->root()->toStringRecursive(true)));
		cout << deserialized;
		cout << "================================================================================" << endl;

		SQLParser::ObjectCache *o = new SQLParser::ObjectCache();
		QList<QString> cols;

		QMap<QString, int> tableNameEnumerator;

		// INCIDENTS // T1
		cols.append(QString("INC_ACTUALFINISH"));
		cols.append(QString("INC_ASSIGN_PERSON_TO"));
		cols.append(QString("INC_ASSIGN_WORKGROUP"));
		cols.append(QString("INC_CAT_OID"));
		cols.append(QString("INC_CIT_OID"));
		cols.append(QString("INC_CLA_OID"));
		cols.append(QString("INC_CLO_OID"));
		cols.append(QString("INC_DEADLINE"));
		cols.append(QString("INC_DESCRIPTION"));
		cols.append(QString("INC_ID"));
		cols.append(QString("INC_IMP_OID"));
		cols.append(QString("INC_OID"));
		cols.append(QString("INC_POO_OID"));
		cols.append(QString("INC_PRI_OID"));
		cols.append(QString("INC_SEV_OID"));
		cols.append(QString("INC_SOLUTION"));
		cols.append(QString("INC_STA_OID"));
		cols.append(QString("REG_CREATED"));
		o->addTable(SQLParser::ObjectCache::TableInfo(QString("SERVICEDESK"), QString("ITSM_INCIDENTS"), cols));
		cols.clear();

		// REP_CODES_TEXT // T2 T12 T99
		cols.append(QString("RCT_NAME"));
		cols.append(QString("RCT_RCD_OID"));
		o->addTable(SQLParser::ObjectCache::TableInfo(QString("SERVICEDESKREPO"), QString("REP_CODES_TEXT"), cols));
		cols.clear();

		// ITSM_INC_CUSTOM_FIELDS // T3
		cols.append(QString("ICF_INCSHORTTEXT2"));
		cols.append(QString("ICF_INCSHORTTEXT3"));
		cols.append(QString("ICF_INCSHORTTEXT4"));
		cols.append(QString("ICF_INC_OID"));
		o->addTable(SQLParser::ObjectCache::TableInfo(QString("SERVICEDESK"), QString("ITSM_INC_CUSTOM_FIELDS"), cols));
		cols.clear();

		// ITSM_WORKGROUPS // T4
		cols.append(QString("WOG_OID"));
		cols.append(QString("WOG_SEARCHCODE"));
		o->addTable(SQLParser::ObjectCache::TableInfo(QString("SERVICEDESK"), QString("ITSM_WORKGROUPS"), cols));
		cols.clear();

		// ITSM_PERSONS // T5
		cols.append(QString("PER_SEARCHCODE"));
		cols.append(QString("PER_OID"));
		o->addTable(SQLParser::ObjectCache::TableInfo(QString("SERVICEDESK"), QString("ITSM_PERSONS"), cols));
		cols.clear();

		// ITSM_CODES_LOCALE // T6 T7 T8 T9 T11 T13 T16
		cols.append(QString("CDL_NAME"));
		cols.append(QString("CDL_COD_OID"));
		o->addTable(SQLParser::ObjectCache::TableInfo(QString("SERVICEDESK"), QString("ITSM_CODES_LOCALE"), cols));
		cols.clear();

		// ITSM_CONFIGURATION_ITEMS // T10
		cols.append(QString("CIT_SEARCHCODE"));
		cols.append(QString("CIT_OID"));
		o->addTable(SQLParser::ObjectCache::TableInfo(QString("SERVICEDESK"), QString("ITSM_CONFIGURATION_ITEMS"), cols));
		cols.clear();

		// ITSM_HISTORYLINES_INCIDENT // a d
		cols.append("HIN_SUBJECT");
		cols.append("HIN_INC_OID");
		cols.append("REG_CREATED");
		o->addTable(SQLParser::ObjectCache::TableInfo(QString("SERVICEDESK"), QString("ITSM_HISTORYLINES_INCIDENT"), cols));
		cols.clear();

		// ITSM_CODES // T14 T15
		cols.append(QString("COD_OID"));
		cols.append(QString("COD_COD_OID"));
		o->addTable(SQLParser::ObjectCache::TableInfo(QString("SERVICEDESK"), QString("ITSM_CODES"), cols));
		cols.clear();

		parser->scanTree(/*o, QString("SERVICEDESK")*/);

		unslurp(string(argv[1]) + ".out", deserialized);
	}
	catch (const QString &str)
	{
		cerr << "Unhandled exception:"<< endl << endl << qPrintable(str) << endl;
	}
	return 1;
}

static void usage()
{
    printf("Usage:\n\n  test7 filename\n\n");
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

void unslurp(string const& fileName, string const& what)
{
    ofstream ofs(fileName.c_str(), ios::out | ios::trunc);
	bool o = ofs.is_open();
    ofs << what;
    ofs.close();
}

void toASTWalk(Statement &source, bool (*filter)(Token const& n))
{
	SQLParser::Statement::token_const_iterator node;
	for (node = source.begin(); node != source.end(); ++node)
	{
		if(filter(*node))
		{
			cout << "Filtered: " << qPrintable(node->toStringRecursive(true)) << endl;
		}
	}
}
