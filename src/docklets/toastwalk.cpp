#include "docklets/toastwalk.h"
#include "core/tologger.h"
#include "dotgraph.h"
#include "parsing/tsqlparse.h"

#define GLUE '_'

#include <QtCore/QMap>

void toASTWalk(SQLParser::Statement *source, DotGraph *target)
{
	QMap<QString, int> tableNameEnumerator;
    //target->initEmpty();
    //QMap<QString,QString> ga;
    //ga["id"] = "Query";
    //ga["compound"] = "true";
    //ga["shape"] = "circle";
    //target->setGraphAttributes(ga);

    //foreach(SQLParser::Token const*t, source->tableTokens())
    //{
    //  QMap<QString,QString> node;
    //  node["name"] = t->toString();
    //  node["label"] = t->toString();
    //  node["fontsize"] = "10";
    //  node["comment"]= t->toString();
    //  node["id"]= t->toString();
    //
    //  target->addNewNode(node);
    //}

    // TODO: add UNION count here: "SELECT * FROM A UNION ALL SELECT * FROM A" => id="A", id="UNION_1:A"
    unsigned emptyAliasCnt = 0;

    SQLParser::Statement::token_const_iterator node;
    for (node = source->begin(); node != source->end(); ++node)
    {
        QString nn = node->toLispStringRecursive();
        switch (node->getTokenType())
        {
            case SQLParser::Token::S_SUBQUERY_FACTORED:
            case SQLParser::Token::S_SUBQUERY_NESTED:
                {
                    SQLParser::TokenSubquery const* snode = static_cast<SQLParser::TokenSubquery const*>(&*node);
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
                    target->addNewSubgraph(sg);

                    TLOG(8, toNoDecorator, __HERE__) << "Subgraph: " << clusterName << ":" << node->toStringRecursive(false) << std::endl;
                    foreach(SQLParser::Token * table, tables)
                    {
                        if ( table->getTokenType() != SQLParser::Token::S_TABLE_REF)
                            continue;
                        // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
                        if ( source->translateAlias(table->child(0)->toStringRecursive(false).toUpper(), &*node))
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
                        ta["comment"] = tt->toStringRecursive(false) + '/' + tt->getValidPosition().toString();
                        ta["id"] = clusterName + GLUE + tableName;
                        if (tt->nodeAlias() != NULL)
                            ta["tooltip"] = tt->nodeAlias()->toString();
                        tt->setNodeID(ta["id"]);
                        target->addNewNodeToSubgraph(ta, sg["id"]);
                        TLOG(8, toNoDecorator, __HERE__) << "new node:" << ta["id"] << " in subgraph: " << sg["id"] << std::endl;
                    }
                    break;
                }
            case SQLParser::Token::X_ROOT:
                {
                    SQLParser::TokenSubquery const* snode = static_cast<SQLParser::TokenSubquery const*>(&*node);
                    QList<SQLParser::Token*> tables = snode->nodeTables().values();
                    foreach(SQLParser::Token * table, tables)
                    {
                        if ( table->getTokenType() != SQLParser::Token::S_TABLE_REF)
                            continue;
                        // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
                        if ( source->translateAlias(table->child(0)->toStringRecursive(false).toUpper(), &*node))
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
                        if (tt->nodeAlias() != NULL)
                            ta["tooltip"] = tt->nodeAlias()->toString();
                        tt->setNodeID(ta["id"]);
                        target->addNewNode(ta);
                        TLOG(8, toNoDecorator, __HERE__) << "new node:" << ta["id"] << " under X_ROOT" << std::endl;
                    }
                    break;
                }
            case SQLParser::Token::L_JOINING_CLAUSE:
                {
                    SQLParser::Statement::token_const_iterator subnode(node);
                    SQLParser::Statement::token_const_iterator node_end(source->subtree_end(&(*node)));
                    subnode++;
                    if (subnode == node_end)
                        break;
                    SQLParser::Token const* firstTable = NULL;
                    SQLParser::Token const* secondTable = NULL;

                    bool isJoinON = false;
                    bool isJoinUSING = false;

                    TLOG(8, toNoDecorator, __HERE__) << "JC:(" << node.depth() << ')' << std::endl
                            << node->toStringRecursive().toStdString() << std::endl;
                    TLOG(8, toNoDecorator, __HERE__) << "JC:(" << node.depth() << ')' << std::endl
                            << node->toLispStringRecursive().toStdString() << std::endl;
                    std::string ns = node->toLispStringRecursive().toStdString();
                    do
                    {
                        TLOG(8,toNoDecorator,__HERE__) << "JC:(" << subnode.depth() << ')' << subnode->toString() << std::endl;
                        QString sn = subnode->toLispStringRecursive();
                        switch (subnode->getTokenType())
                        {
                            case SQLParser::Token::L_RESERVED:
                                if ( QString::compare("ON", subnode->toString(), Qt::CaseInsensitive) == 0)
                                    isJoinON = true;
                                if ( QString::compare("USING", subnode->toString(), Qt::CaseInsensitive) == 0)
                                    isJoinUSING = true;
                                break;
                            case SQLParser::Token::S_OPERATOR_BINARY:
                                if (subnode->childCount() != 2) 
                                    // assuming condition "T1.C1" "=" "T2.C2";
                                    // ((= [OPERATOR_BINARY]
                                    // ((ANY_ELEMENT[IDENTIFIER]((task[UNASSIGNED]))((id[UNASSIGNED]))))
                                    //	((ANY_ELEMENT[IDENTIFIER]((filter[UNASSIGNED]))((task_id[UNASSIGNED])))))))))))
                                    break;
                                SQLParser::Statement::token_const_iterator ch0(subnode->child(0));
                                SQLParser::Statement::token_const_iterator ch0_end(source->subtree_end(subnode->child(0)));
                                if (ch0 != ch0_end)
                                    do {
                                        TLOG(8, toNoDecorator, __HERE__) << "CH0:(" << ch0.depth() << '/' << subnode->depth() << ')' << ch0->toString() << ' ' << subnode->toString() << std::endl;

                                        if (ch0->getTokenType() == SQLParser::Token::S_IDENTIFIER)
                                        {
                                            firstTable = &(*ch0);
                                            break;
                                        }
                                        ch0++;
                                    } while (ch0 != source->subtree_end(subnode->child(0)));

                                SQLParser::Statement::token_const_iterator ch1(subnode->child(1));
                                SQLParser::Statement::token_const_iterator ch1_end(source->subtree_end(subnode->child(1)));
                                if (ch1 != ch1_end)
                                    do {
                                        TLOG(8, toNoDecorator, __HERE__) << "CH1:(" << ch1.depth() << '/' << subnode->depth() << ')' << ch1->toString() << ' ' << subnode->toString() << std::endl;

                                        if (ch1->getTokenType() == SQLParser::Token::S_IDENTIFIER)
                                        {
                                            secondTable = &(*ch1);
                                            break;
                                        }
                                        ch1++;
                                    } while (ch1 != source->subtree_end(subnode->child(1)));

                                break;
                        }
                        subnode++;
                    } while (subnode != node_end);

                    if (isJoinON == false ||
                        firstTable == NULL ||  firstTable->childCount() != 2 ||
                        secondTable == NULL || secondTable->childCount() != 2 )
                        break;

                    SQLParser::Token const *t1 = source->translateAlias(firstTable->child(0)->toStringRecursive(false).toUpper(), &*node);
                    if ( t1)
                        firstTable = t1;
                    else
                        firstTable = source->getTableRef(firstTable->child(0)->toStringRecursive(false).toUpper(), &*node);

                    SQLParser::Token const *t2 = source->translateAlias(secondTable->child(0)->toStringRecursive(false).toUpper(), &*node);
                    if ( t2)
                        secondTable = t2;
                    else
                        secondTable = source->getTableRef(secondTable->child(0)->toStringRecursive(false).toUpper(), &*node);

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
                            target->addNewEdge(
                                               e1,
                                               e2,
                                               ea);
                            TLOG(8, toNoDecorator, __HERE__) << "!!" << e1.toStdString() << "->" << e2.toStdString() << std::endl;
                        }
                    else
                        {
                            ////TLOG(8,toNoDecorator,__HERE__) << "??" << firstTable->toStringRecursive(false) << "->" << secondTable->toStringRecursive(false) << std::endl;
                        }
                }
                break;
            case SQLParser::Token::S_WHERE:
                {
                    SQLParser::Statement::token_const_iterator subnode(node);
                    SQLParser::Statement::token_const_iterator node_end(source->subtree_end(&(*node)));
                    subnode++;
                    if (subnode == node_end)
                        break;

                    do
                    {
                        switch (subnode->getTokenType())
                        {
                            case SQLParser::Token::S_OPERATOR_BINARY:
                                {
                                    TLOG(8, toNoDecorator, __HERE__) << "S_OPERATOR_BINARY: " << std::endl
                                                                     << subnode->toStringRecursive() << std::endl;

                                    SQLParser::Token const* firstTable = NULL;
                                    SQLParser::Token const* secondTable = NULL;

                                    if (subnode->childCount() != 2) // assuming condition "T1.C1" "=" "T2.C2";
                                        break;

                                    SQLParser::Statement::token_const_iterator ch0(subnode->child(0));
                                    SQLParser::Statement::token_const_iterator ch0_end(source->subtree_end(subnode->child(0)));
                                    if (ch0 != ch0_end)
                                        do {
                                            TLOG(8, toNoDecorator, __HERE__) << "CH0:(" << ch0.depth() << '/' << subnode->depth() << ')' << ch0->toString() << ' ' << subnode->toString() << std::endl;

                                            if (ch0->getTokenType() == SQLParser::Token::S_IDENTIFIER)
                                            {
                                                firstTable = &(*ch0);
                                                break;
                                            }
                                            ch0++;
                                        } while (ch0 != source->subtree_end(subnode->child(0)));

                                    SQLParser::Statement::token_const_iterator ch1(subnode->child(1));
                                    SQLParser::Statement::token_const_iterator ch1_end(source->subtree_end(subnode->child(1)));
                                    if (ch1 != ch1_end)
                                        do {
                                            TLOG(8, toNoDecorator, __HERE__) << "CH1:(" << ch1.depth() << '/' << subnode->depth() << ')' << ch1->toString() << ' ' << subnode->toString() << std::endl;

                                            if (ch1->getTokenType() == SQLParser::Token::S_IDENTIFIER)
                                            {
                                                secondTable = &(*ch1);
                                                break;
                                            }
                                            ch1++;
                                        } while (ch1 != source->subtree_end(subnode->child(1)));

                                    if (firstTable == NULL ||  firstTable->childCount() != 2 ||
                                        secondTable == NULL || secondTable->childCount() != 2 )
                                        break;

                                    SQLParser::Token const *t1 = source->translateAlias(firstTable->child(0)->toStringRecursive(false).toUpper(), &*node);
                                    if ( t1)
                                        firstTable = t1;
                                    else
                                        firstTable = source->getTableRef(firstTable->child(0)->toStringRecursive(false).toUpper(), &*node);

                                    SQLParser::Token const *t2 = source->translateAlias(secondTable->child(0)->toStringRecursive(false).toUpper(), &*node);
                                    if ( t2)
                                        secondTable = t2;
                                    else
                                        secondTable = source->getTableRef(secondTable->child(0)->toStringRecursive(false).toUpper(), &*node);
                                    
                                    QString e1, e2;

                                    if ( firstTable && (
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

                                    QMap<QString, QString> ea; // edge attreibutes
                                    if (!e1.isEmpty() && !e2.isEmpty()) // TODO this is wrong
                                    {
                                        target->addNewEdge(
                                            e1,
                                            e2,
                                            ea);
                                        TLOG(8, toNoDecorator, __HERE__) << "!!" << e1.toStdString() << "->" << e2.toStdString() << std::endl;
                                    }
                                    else
                                    {
                                        ////TLOG(8,toNoDecorator,__HERE__) << "??" << firstTable->toStringRecursive(false) << "->" << secondTable->toStringRecursive(false) << std::endl;
                                    }

                                    break;
                                } // case S_OPERATOR_BINARY
                        } // switch getTokenType
                        subnode++;
                    } while (subnode != node_end);
                } // case S_WHERE
        } // switch getTokenType
    } // for each node
    target->update();
};
