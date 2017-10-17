#include "docklets/toastwalk.h"
#include "core/tologger.h"
#include "dotgraph.h"
#include "parsing/tsqlparse.h"

#define GLUE '_'

#include <QtCore/QMap>

using namespace SQLParser;
using namespace std;

unsigned emptyAliasCnt = 0;
QMap<QString, int> tableNameEnumerator;

void toASTWalkFilter(Statement &source, DotGraph &target, const std::function<bool(Statement &source, DotGraph &target, Token & n)>& filter);

std::function<bool(Statement &source, DotGraph &target, Token & n)> root = [&](Statement &source, DotGraph &target, Token &node)
{
    if (node.getTokenType() != Token::X_ROOT)
        return false;

    SQLParser::TokenSubquery *snode = static_cast<SQLParser::TokenSubquery*>(&node);
    snode->setNodeID("ROOT");
#if 0
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
#endif
    return true;
};

std::function<bool(Statement &source, DotGraph &target, Token & n)> subquery_factored = [&](Statement &source, DotGraph &target, Token & node)
{
    if (node.getTokenType() != Token::S_SUBQUERY_FACTORED
            && node.getTokenType() != Token::S_SUBQUERY_NESTED)
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
    snode->setNodeID(sg["id"]);
    target.addNewSubgraph(sg);

    return true;
};

std::function<bool(Statement &source, DotGraph &target, Token &n)> table_ref = [&](Statement &source, DotGraph &target, Token &node)
{
    if ( node.getTokenType() != SQLParser::Token::S_TABLE_REF)
        return false;

    Token const* context(nullptr);
	QString clusterName;
    for(SQLParser::Statement::token_const_iterator_to_root k(&node); &*k; ++k)
    {
        if (k->getTokenType() == SQLParser::Token::S_SUBQUERY_FACTORED)
        {
            context = &(*k);
            clusterName = static_cast<SQLParser::TokenSubquery const*>(context)->nodeID();
            break;
        }

        if (k->getTokenType() == SQLParser::Token::S_SUBQUERY_NESTED)
        {
            context = &(*k);
            clusterName = static_cast<SQLParser::TokenSubquery const*>(context)->nodeID();
            break;
        }

        if (k->getTokenType() == SQLParser::Token::X_ROOT)
        {
            context = &(*k);
			clusterName = "ROOT";
            break;
        }
    }
    if (context == nullptr)
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

    QMap<QString, QString> ta; // table attributes
    ta["name"]     = tt->tableName();
    ta["label"]    = tt->tableName();
    ta["fontsize"] = "10";
    ta["comment"]  = tt->getValidPosition().toString();
    ta["id"]       = clusterName + GLUE + tableName;
    if (tt->nodeAlias() != nullptr)
        ta["tooltip"] = tt->nodeAlias()->toString();
    tt->setNodeID(ta["id"]);
    if (context->getTokenType() == SQLParser::Token::X_ROOT)
        target.addNewNode(ta);
    else
        target.addNewNodeToSubgraph(ta, clusterName);
    TLOG(8, toNoDecorator, __HERE__) << "new node:" << ta["id"] << " in subgraph: " << clusterName << std::endl;

    return true;
};

std::function<bool(Statement &source, DotGraph &target, Token &n)> binary_operator = [&](Statement &source, DotGraph &target, Token &node)
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

void toASTWalk(SQLParser::Statement *source, DotGraph *target)
{
    emptyAliasCnt = 0;
    tableNameEnumerator.clear();

    source->scanTree();

    // If true, allow edges between clusters. (See lhead and ltail)
    target->attributes().insert("compound", "true");

    toASTWalkFilter(*source, *target, root);
    toASTWalkFilter(*source, *target, subquery_factored);
    toASTWalkFilter(*source, *target, table_ref);
    toASTWalkFilter(*source, *target, binary_operator);

    target->update();
}

void toASTWalkFilter(Statement &source, DotGraph &target, const std::function<bool(Statement &source, DotGraph &target, Token & n)>& filter)
{
        SQLParser::Statement::token_const_iterator node;
        for (node = source.begin(); node != source.end(); ++node)
        {
                if(filter(source, target, const_cast<SQLParser::Token&>(*node)))
                {
                        cout << "Filtered: " << qPrintable(node->toStringRecursive(true)) << endl;
                }
        }
}
