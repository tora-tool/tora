#include "docklets/toastwalk.h"
#include "core/tologger.h"
#include "dotgraph.h"
#include "parsing/tsqlparse.h"

#define GLUE '_'

#include <QtCore/QMap>

void toASTWalk(SQLParser::Statement *source, DotGraph *target)
{
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

    source->scanTree(o, QString("SERVICEDESK"));

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
    for(node = source->begin(); node != source->end(); ++node)
    {
        switch(node->getTokenType())
        {
        case SQLParser::Token::S_SUBQUERY_FACTORED:
        case SQLParser::Token::S_SUBQUERY_NESTED:
        {
            SQLParser::TokenSubquery const* snode = static_cast<SQLParser::TokenSubquery const*>(&*node);
            QList<SQLParser::Token*> tables = snode->nodeTables().values();
            QString clusterName;

            QMap<QString, QString> sg;
            if( snode->nodeAlias() == NULL)
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

            TLOG(0, toNoDecorator, __HERE__) << "Subgraph: " << clusterName << ":" << node->toStringRecursive(false) << std::endl;
            foreach(SQLParser::Token * table, tables)
            {
                if( table->getTokenType() != SQLParser::Token::S_TABLE_REF)
                    continue;
                // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
                if( source->translateAlias(table->child(0)->toStringRecursive(false).toUpper(), &*node))
                    continue;

                SQLParser::TokenTable *tt = static_cast<SQLParser::TokenTable*>(table);
                QMap<QString, QString> ta; // table attributes
                QString tableName = tt->toStringRecursive(false); // DOT does not allow ant "dots" in identifier names
                tableName.replace('.', GLUE);
                tableName.replace(':', GLUE);


                if( tableNameEnumerator.keys().contains(tableName))
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
                if(tt->nodeAlias() != NULL)
                    ta["tooltip"] = tt->nodeAlias()->toString();
                tt->setNodeID(ta["id"]);
                target->addNewNodeToSubgraph(ta, sg["id"]);
                TLOG(0, toNoDecorator, __HERE__) << "new node:" << ta["id"] << " in subgraph: " << sg["id"] << std::endl;
            }
            break;
        }
        case SQLParser::Token::X_ROOT:
        {
            SQLParser::TokenSubquery const* snode = static_cast<SQLParser::TokenSubquery const*>(&*node);
            QList<SQLParser::Token*> tables = snode->nodeTables().values();
            foreach(SQLParser::Token * table, tables)
            {
                if( table->getTokenType() != SQLParser::Token::S_TABLE_REF)
                    continue;
                // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
                if( source->translateAlias(table->child(0)->toStringRecursive(false).toUpper(), &*node))
                    continue;
                SQLParser::TokenTable *tt = static_cast<SQLParser::TokenTable*>(table);
                QMap<QString, QString> ta; // table atributes
                QString tableName = tt->toStringRecursive(false); // DOT does not allow ant "dots" in identifier names
                tableName.replace('.', '_');
                tableName.replace(':', '_');

                if( tableNameEnumerator.keys().contains(tableName))
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
                ta["comment"] = tt->toStringRecursive(false);
                ta["id"] = QString("ROOT") + GLUE + tableName;
                if(tt->nodeAlias() != NULL)
                    ta["tooltip"] = tt->nodeAlias()->toString();
                tt->setNodeID(ta["id"]);
                target->addNewNode(ta);
                TLOG(0, toNoDecorator, __HERE__) << "new node:" << ta["id"] << " under X_ROOT" << std::endl;
            }
            break;
        }
        case SQLParser::Token::L_JOINING_CLAUSE:
        {
            SQLParser::Statement::token_const_iterator subnode(node);
            subnode++;

            SQLParser::Token const* firstTable = NULL;
            SQLParser::Token const* secondTable = NULL;

            bool isJoinON = false;
            bool isJoinUSING = false;

            TLOG(0, toNoDecorator, __HERE__) << "JC:(" << node.depth() << ')' << std::endl
                                             << node->toStringRecursive().toStdString() << std::endl;
            while(subnode.depth() > node.depth())
            {
                ///TLOG(0,toNoDecorator,__HERE__) << "JC:(" << subnode.depth() << ')' << subnode->toString() << std::endl;
                switch(subnode->getTokenType())
                {
                case SQLParser::Token::L_RESERVED:
                    if( QString::compare("ON", subnode->toString(), Qt::CaseInsensitive) == 0)
                        isJoinON = true;
                    if( QString::compare("USING", subnode->toString(), Qt::CaseInsensitive) == 0)
                        isJoinUSING = true;
                    break;
                case SQLParser::Token::S_OPERATOR_BINARY:
                    if(subnode->childCount() != 3) // assuming condition "T1.C1" "=" "T2.C2";
                        break;
                    if(subnode->child(0)->getTokenType() == SQLParser::Token::S_IDENTIFIER &&
                            subnode->child(2)->getTokenType() == SQLParser::Token::S_IDENTIFIER)
                    {
                        firstTable = subnode->child(0);
                        secondTable = subnode->child(2);
                        TLOG(0, toNoDecorator, __HERE__) << "!?" << firstTable->toStringRecursive(false) << "->" << secondTable->toStringRecursive(false) << std::endl;
                    }
                    break;
                }
                subnode++;
            } // while

            if(isJoinON &&
                    firstTable && firstTable->childCount() == 3 &&
                    secondTable && secondTable->childCount() == 3
              )
            {
                SQLParser::Token const *t1 = source->translateAlias(firstTable->child(0)->toStringRecursive(false).toUpper(), &*node);
                if( t1)
                    firstTable = t1;
                else
                    firstTable = source->getTableRef(firstTable->child(0)->toStringRecursive(false).toUpper(), &*node);

                SQLParser::Token const *t2 = source->translateAlias(secondTable->child(0)->toStringRecursive(false).toUpper(), &*node);
                if( t2)
                    secondTable = t2;
                else
                    secondTable = source->getTableRef(secondTable->child(0)->toStringRecursive(false).toUpper(), &*node);

                QString e1, e2;

                if(firstTable && (
                            firstTable->getTokenType() == SQLParser::Token::S_SUBQUERY_NESTED ||
                            firstTable->getTokenType() == SQLParser::Token::S_SUBQUERY_FACTORED))
                {
                    if(!static_cast<SQLParser::TokenSubquery const*>(firstTable)->nodeID().isEmpty())
                        e1 = static_cast<SQLParser::TokenSubquery const*>(firstTable)->nodeID();
                }

                if(firstTable && firstTable->getTokenType() == SQLParser::Token::S_TABLE_REF)
                {
                    e1 = static_cast<SQLParser::TokenTable const*>(firstTable)->nodeID();
                }

                if(secondTable && (
                            secondTable->getTokenType() == SQLParser::Token::S_SUBQUERY_NESTED ||
                            secondTable->getTokenType() == SQLParser::Token::S_SUBQUERY_FACTORED))
                {
                    if(!static_cast<SQLParser::TokenSubquery const*>(secondTable)->nodeID().isEmpty())
                        e2 = static_cast<SQLParser::TokenSubquery const*>(secondTable)->nodeID();
                }

                if(secondTable && secondTable->getTokenType() == SQLParser::Token::S_TABLE_REF)
                {
                    e2 = static_cast<SQLParser::TokenTable const*>(secondTable)->nodeID();
                }

                QMap<QString, QString> ea; // edge attributes
                if(!e1.isEmpty() && !e2.isEmpty()) // TODO this is wrong
                {
                    target->addNewEdge(
                        e1,
                        e2,
                        ea);
                    TLOG(0, toNoDecorator, __HERE__) << "!!" << e1.toStdString() << "->" << e2.toStdString() << std::endl;
                }
                else
                {
                    ////TLOG(0,toNoDecorator,__HERE__) << "??" << firstTable->toStringRecursive(false) << "->" << secondTable->toStringRecursive(false) << std::endl;
                }
            }
            break;
        }
        case SQLParser::Token::S_WHERE:
        {
            SQLParser::Statement::token_const_iterator subnode(node);
            subnode++;

            while(subnode.depth() > node.depth())
            {
                switch(subnode->getTokenType())
                {
                case SQLParser::Token::S_OPERATOR_BINARY:
                {
                    TLOG(0, toNoDecorator, __HERE__) << "S_OPERATOR_BINARY: " << std::endl
                                                     << subnode->toStringRecursive() << std::endl;

                    SQLParser::Token const* firstTable = NULL;
                    SQLParser::Token const* secondTable = NULL;

                    if(subnode->childCount() != 3) // assuming condition "T1.C1" "=" "T2.C2";
                        break;
                    if(subnode->child(0)->getTokenType() == SQLParser::Token::S_IDENTIFIER &&
                            subnode->child(2)->getTokenType() == SQLParser::Token::S_IDENTIFIER)
                    {
                        firstTable = subnode->child(0);
                        secondTable = subnode->child(2);
                        TLOG(0, toNoDecorator, __HERE__) << "!?" << firstTable->toStringRecursive(false) << "->" << secondTable->toStringRecursive(false) << std::endl;
                    }

                    SQLParser::Token const *t1 = firstTable ? source->translateAlias(firstTable->child(0)->toStringRecursive(false).toUpper(), &*node) : NULL;
                    SQLParser::Token const *t2 = secondTable ? source->translateAlias(secondTable->child(0)->toStringRecursive(false).toUpper(), &*node) : NULL;
                    QString e1, e2;

                    if( t1 && (
                                t1->getTokenType() == SQLParser::Token::S_SUBQUERY_NESTED ||
                                t1->getTokenType() == SQLParser::Token::S_SUBQUERY_FACTORED))
                    {
                        if(!static_cast<SQLParser::TokenSubquery const*>(t1)->nodeID().isEmpty())
                            e1 = static_cast<SQLParser::TokenSubquery const*>(t1)->nodeID();
                    }

                    if(t1 && t1->getTokenType() == SQLParser::Token::S_TABLE_REF)
                    {
                        // e1 = QString("ROOT") + GLUE + t1->child(0)->toString();
                        // if(t1->childCount() >= 3)
                        //  e1 += GLUE + t1->child(2)->toString();
                        e1 = static_cast<SQLParser::TokenTable const*>(t1)->nodeID();
                    }

                    if(t2 && (
                                t2->getTokenType() == SQLParser::Token::S_SUBQUERY_NESTED ||
                                t2->getTokenType() == SQLParser::Token::S_SUBQUERY_FACTORED))
                    {
                        if(!static_cast<SQLParser::TokenSubquery const*>(t2)->nodeID().isEmpty())
                            e2 = static_cast<SQLParser::TokenSubquery const*>(t2)->nodeID();
                    }

                    if(t2 && t2->getTokenType() == SQLParser::Token::S_TABLE_REF)
                    {
                        // e2 = QString("ROOT") + GLUE + t2->child(0)->toString();
                        // if(t2->childCount() >= 3)
                        //  e2 += GLUE + t2->child(2)->toString();
                        e2 = static_cast<SQLParser::TokenTable const*>(t2)->nodeID();
                    }

                    QMap<QString, QString> ea; // edge attreibutes
                    if(!e1.isEmpty() && !e2.isEmpty()) // TODO this is wrong
                    {
                        target->addNewEdge(
                            e1,
                            e2,
                            ea);
                        TLOG(0, toNoDecorator, __HERE__) << "!!" << e1.toStdString() << "->" << e2.toStdString() << std::endl;
                    }
                    else
                    {
                        ////TLOG(0,toNoDecorator,__HERE__) << "??" << firstTable->toStringRecursive(false) << "->" << secondTable->toStringRecursive(false) << std::endl;
                    }

                    break;
                } // case S_OPERATOR_BINARY
                } // switch getTokenType
                subnode++;
            } // while(subnode.depth() > node.depth())
        } // case S_WHERE
        } // switch getTokenType
    } // for each node
    target->update();
};
