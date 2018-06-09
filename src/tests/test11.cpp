
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
void toASTWalkFilter(SQLParser::Statement &source, const std::function<bool(Statement &source, Token & node)>& filter);

DotGraph target;
GraphExporter exporter;

#define GLUE '_'
unsigned emptyAliasCnt = 0;
QMap<QString, int> tableNameEnumerator;

std::function<bool(Statement &source, Token & n)> root = [&](Statement &source, Token & node)
{
    if (node.getTokenType() != Token::X_ROOT)
        return false;

    SQLParser::TokenSubquery * snode = static_cast<SQLParser::TokenSubquery*>(&node);
    QList<SQLParser::Token*> tables = snode->nodeTables().values();
    foreach(SQLParser::Token * table, tables)
    {
        if ( table->getTokenType() != SQLParser::Token::S_TABLE_REF)
            continue;
        // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
        if ( source.translateAlias(table->child(0)->toStringRecursive(false).toUpper(), &node))
            continue;
        SQLParser::TokenTable *tt = static_cast<SQLParser::TokenTable*>(table);
        QMap<QString, QString> ta; // table atributes
        QString tableName = tt->toStringRecursive(false); // DOT does not allow ant "dots" in identifier names
        tableName.replace('.', '_');
        tableName.replace(':', '_');

        if ( tableNameEnumerator.keys().contains(tableName))
        {
            unsigned cnt = tableNameEnumerator.value(tableName);
            cnt++;
            tableNameEnumerator.insert(tableName, cnt);
            tableName = tableName + '_' + QString::number(cnt);
        }
        else
        {
            tableNameEnumerator.insert(tableName, 1);
            tableName = tableName + '_' + '1';
        }

        ta["name"] = tt->toStringRecursive(false);
        ta["label"] = tt->toStringRecursive(false);
        ta["fontsize"] = "10";
        ta["comment"] = tt->toStringRecursive(false) + '/' + tt->getValidPosition().toString();
        ta["id"] = QString("ROOT") + GLUE + tableName;
        if (tt->nodeAlias() != nullptr)
            ta["tooltip"] = tt->nodeAlias()->toString();
        tt->setNodeID(ta["id"]);
        target.addNewNode(ta);
        TLOG(8, toNoDecorator, __HERE__) << "new node:" << ta["id"] << " under X_ROOT" << std::endl;
    }
    return true;
};

std::function<bool(Statement &source, Token & n)> subquery_factored = [&](Statement &source, Token & node)
{
    if (node.getTokenType() != Token::S_SUBQUERY_FACTORED)
        return false;

    SQLParser::TokenSubquery const* snode = static_cast<SQLParser::TokenSubquery const*>(&node);
    QList<SQLParser::Token*> tables = snode->nodeTables().values();
    QString clusterName;

    QMap<QString, QString> sg;
    if ( snode->nodeAlias() == nullptr)
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
    sg["comment"] = snode->getValidPosition().toString();
    snode->setNodeID(sg["id"]);
    target.addNewSubgraph(sg);

    return true;
};

std::function<bool(Statement &source, Token & n)> subquery_nested = [&](Statement &source, Token & node)
{
    if (node.getTokenType() != Token::S_SUBQUERY_NESTED)
        return false;

    SQLParser::TokenSubquery const* snode = static_cast<SQLParser::TokenSubquery const*>(&node);
    QList<SQLParser::Token*> tables = snode->nodeTables().values();
    QString clusterName;

    QMap<QString, QString> sg;
    if ( snode->nodeAlias() == nullptr)
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
    sg["comment"] = snode->getValidPosition().toString();
    snode->setNodeID(sg["id"]);
    target.addNewSubgraph(sg);

    return true;
};

// add tables, but only into subquery clusters
std::function<bool(Statement &source, Token & n)> table_ref = [&](Statement &source, Token & node)
{
    if ( node.getTokenType() != SQLParser::Token::S_TABLE_REF)
        return false;

    Token const* context(nullptr);
    for(SQLParser::Statement::token_const_iterator_to_root k(&node); &*k; ++k)
    {
        if (k->getTokenType() == SQLParser::Token::S_SUBQUERY_FACTORED
                || k->getTokenType() == SQLParser::Token::S_SUBQUERY_NESTED
                || k->getTokenType() == SQLParser::Token::X_ROOT)
        {
            context = &(*k);
            break;
        }
    }
    if (context == nullptr)
        return false;
    if (context->getTokenType() == SQLParser::Token::X_ROOT) // Do not add tables under X_ROOT, it is done in root lambda
        return false;

    // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
    if ( source.translateAlias(node.child(0)->toStringRecursive(false).toUpper(), context))
        return false;

    SQLParser::TokenTable *tt = static_cast<SQLParser::TokenTable*>(&node);
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

    auto tokenSubQuery = dynamic_cast<SQLParser::TokenSubquery const*>(context);
	if (!tokenSubQuery)
	{
		TLOG(8, toNoDecorator, __HERE__) << "failure: failed to resolve subquery for: " << tt->toStringRecursive(false) << '/' + tt->getValidPosition().toString() << std::endl;
		return false;
	}

    QString clusterName = tokenSubQuery->nodeID();

    QMap<QString, QString> ta; // table attributes
    ta["name"]     = tt->toStringRecursive(false);
    ta["label"]    = tt->toStringRecursive(false);
    ta["fontsize"] = "10";
    ta["comment"]  = tt->toStringRecursive(false) + '/' + tt->getValidPosition().toString();
    ta["id"]       = clusterName + GLUE + tableName;
    if (tt->nodeAlias() != nullptr)
        ta["tooltip"] = tt->nodeAlias()->toString();
    tt->setNodeID(ta["id"]);
    target.addNewNodeToSubgraph(ta, clusterName);
    TLOG(8, toNoDecorator, __HERE__) << "new node:" << ta["id"] << " in subgraph: " << clusterName << std::endl;

    return true;
};

std::function<bool(Statement &source, Token & n)> binary_operator = [&](Statement &source, Token const& node)
{
    if (node.getTokenType() != Token::S_OPERATOR_BINARY)
        return false;

    if (node.childCount() != 2)
        // assuming condition "T1.C1" "=" "T2.C2";
        // ((= [OPERATOR_BINARY]
        // ((ANY_ELEMENT[IDENTIFIER]((task[UNASSIGNED]))((id[UNASSIGNED]))))
        //  ((ANY_ELEMENT[IDENTIFIER]((filter[UNASSIGNED]))((task_id[UNASSIGNED])))))))))))
        return false;

    SQLParser::Token const* firstTable = nullptr;
    SQLParser::Token const* secondTable = nullptr;

    SQLParser::Statement::token_const_iterator ch0(node.child(0));
    SQLParser::Statement::token_const_iterator ch0_end(source.subtree_end(node.child(0)));
    if (ch0 != ch0_end)
        do {
            TLOG(8, toNoDecorator, __HERE__) << "CH0:(" << ch0.depth() << '/' << node.depth() << ')' << ch0->toString() << ' ' << node.toString() << std::endl;

            if (ch0->getTokenType() == SQLParser::Token::S_IDENTIFIER)
            {
                firstTable = &(*ch0);
                break;
            }
            ch0++;
        } while (ch0 != source.subtree_end(node.child(0)));
    if (firstTable == NULL)
        return false;

    SQLParser::Statement::token_const_iterator ch1(node.child(1));
    SQLParser::Statement::token_const_iterator ch1_end(source.subtree_end(node.child(1)));
    if (ch1 != ch1_end)
        do {
            TLOG(8, toNoDecorator, __HERE__) << "CH1:(" << ch1.depth() << '/' << node.depth() << ')' << ch1->toString() << ' ' << node.toString() << std::endl;

            if (ch1->getTokenType() == SQLParser::Token::S_IDENTIFIER)
            {
                secondTable = &(*ch1);
                break;
            }
            ch1++;
        } while (ch1 != source.subtree_end(node.child(1)));
    if (secondTable == nullptr)
        return false;

    SQLParser::Token const *t1 = source.translateAlias(firstTable->child(0)->toStringRecursive(false).toUpper(), &node);
    if ( t1)
        firstTable = t1;
    else
        firstTable = source.getTableRef(firstTable->child(0)->toStringRecursive(false).toUpper(), &node);

    SQLParser::Token const *t2 = source.translateAlias(secondTable->child(0)->toStringRecursive(false).toUpper(), &node);
    if ( t2)
        secondTable = t2;
    else
        secondTable = source.getTableRef(secondTable->child(0)->toStringRecursive(false).toUpper(), &node);

    QString e1, e2;

    if (firstTable && (
            firstTable->getTokenType() == SQLParser::Token::S_SUBQUERY_NESTED ||
            firstTable->getTokenType() == SQLParser::Token::S_SUBQUERY_FACTORED))
    {
        if (!static_cast<SQLParser::TokenSubquery const*>(firstTable)->nodeID().isEmpty())
            e1 = static_cast<SQLParser::TokenSubquery const*>(firstTable)->nodeID();
    }
    if (firstTable && firstTable->getTokenType() == SQLParser::Token::S_TABLE_REF)
    {
        e1 = static_cast<SQLParser::TokenTable const*>(firstTable)->nodeID();
    }

    if (secondTable && (
            secondTable->getTokenType() == SQLParser::Token::S_SUBQUERY_NESTED ||
            secondTable->getTokenType() == SQLParser::Token::S_SUBQUERY_FACTORED))
    {
        if (!static_cast<SQLParser::TokenSubquery const*>(secondTable)->nodeID().isEmpty())
            e2 = static_cast<SQLParser::TokenSubquery const*>(secondTable)->nodeID();
    }
    if (secondTable && secondTable->getTokenType() == SQLParser::Token::S_TABLE_REF)
    {
        e2 = static_cast<SQLParser::TokenTable const*>(secondTable)->nodeID();
    }

    QMap<QString, QString> ea; // edge attributes
    if (!e1.isEmpty() && !e2.isEmpty()) // TODO this is wrong
    {
        target.addNewEdge(
                e1,
                e2,
                ea);
        TLOG(8, toNoDecorator, __HERE__) << "!!" << e1.toStdString() << "->" << e2.toStdString() << std::endl;
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

        parser->scanTree();

        DotGraph::setLayoutCommandPath(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::GraphvizHomeDirectory).toString());
        toASTWalkFilter(*parser, root);
        toASTWalkFilter(*parser, subquery_factored);
        toASTWalkFilter(*parser, subquery_nested);
        toASTWalkFilter(*parser, table_ref);
        toASTWalkFilter(*parser, binary_operator);

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

void toASTWalkFilter(Statement &source, const std::function<bool(Statement &source,Token & n)>& filter)
{
	SQLParser::Statement::token_const_iterator node;
	for (node = source.begin(); node != source.end(); ++node)
	{
		if(filter(source, const_cast<SQLParser::Token&>(*node)))
		{
			cout << "Filtered: " << qPrintable(node->toStringRecursive(true)) << endl;
		}
	}
}
