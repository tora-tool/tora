
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

#include "dotgraph.h"
#include "graphexporter.h"

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
void toASTWalkFilter(SQLParser::Statement &source, const std::function<bool(Statement &source, Token const& none)>& filter);

DotGraph target;
GraphExporter exporter;

#define GLUE '_'
unsigned emptyAliasCnt = 0;
QMap<QString, int> tableNameEnumerator;
std::function<bool(Statement &source, Token const& n)> subquery_factored = [&](Statement &source, Token const&node)
{
    if (node.getTokenType() != Token::S_SUBQUERY_FACTORED)
        return false;

    SQLParser::TokenSubquery const* snode = static_cast<SQLParser::TokenSubquery const*>(&node);
    QList<SQLParser::Token*> tables = snode->nodeTables().values();
    QString clusterName;

    QMap<QString, QString> sg;
    if ( snode->nodeAlias() == NULL)
    {
        emptyAliasCnt++;
        clusterName = QString("cluster_") + QString::number(emptyAliasCnt);
    }
    else
    {
        clusterName = QString("cluster_") + snode->nodeAlias()->toString();
        sg["label"] = snode->nodeAlias()->toString();
    }
    clusterName.replace('.', GLUE);
    clusterName.replace(':', GLUE);
    sg["id"] = clusterName;
    snode->setNodeID(sg["id"]);
    target.addNewSubgraph(sg);

    TLOG(8, toNoDecorator, __HERE__) << "Subgraph: " << clusterName << ":" << node.toStringRecursive(false) << std::endl;
    foreach(SQLParser::Token * table, tables)
    {
        if ( table->getTokenType() != SQLParser::Token::S_TABLE_REF)
            continue;
        // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
        if ( source.translateAlias(table->child(0)->toStringRecursive(false).toUpper(), &node))
            continue;

        SQLParser::TokenTable *tt = static_cast<SQLParser::TokenTable*>(table);
        QMap<QString, QString> ta; // table attributes
        QString tableName = tt->toStringRecursive(false); // DOT does not allow ant "dots" in identifier names
        tableName.replace('.', GLUE);
        tableName.replace(':', GLUE);


        if ( tableNameEnumerator.keys().contains(tableName))
        {
            int cnt = tableNameEnumerator.value(tableName);
            cnt++;
            tableNameEnumerator.insert(tableName, cnt);
            tableName = tableName + '_' + QString::number(cnt);
        }
        else
        {
            tableNameEnumerator.insert(tableName, 1);
            tableName = tableName + QString("_1");
        }

        ta["name"] = tt->toStringRecursive(false);
        ta["label"] = tt->toStringRecursive(false);
        ta["fontsize"] = "10";
        ta["comment"] = tt->toStringRecursive(false);
        ta["id"] = clusterName + GLUE + tableName;
        if (tt->nodeAlias() != NULL)
            ta["tooltip"] = tt->nodeAlias()->toString();
        tt->setNodeID(ta["id"]);
        target.addNewNodeToSubgraph(ta, sg["id"]);
        TLOG(8, toNoDecorator, __HERE__) << "new node:" << ta["id"] << " in subgraph: " << sg["id"] << std::endl;
    }
    return true;
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
        cout << deserialized;
        cout << "================================================================================" << endl;

        SQLParser::ObjectCache *o = new SQLParser::ObjectCache();
        parser->scanTree(o, QString("SERVICEDESK"));

        DotGraph::setLayoutCommandPath(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::GraphvizHomeDirectory).toString());
        toASTWalkFilter(*parser, subquery_factored);

        QString str = exporter.writeDot(&target, QString(argv[1]) + ".dot");
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
    printf("Usage:\n\n  test11 filename\n\n");
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

void toASTWalkFilter(Statement &source, const std::function<bool(Statement &source,Token const& n)>& filter)
{
	SQLParser::Statement::token_const_iterator node;
	for (node = source.begin(); node != source.end(); ++node)
	{
		if(filter(source, *node))
		{
			cout << "Filtered: " << qPrintable(node->toStringRecursive(true)) << endl;
		}
	}
}
