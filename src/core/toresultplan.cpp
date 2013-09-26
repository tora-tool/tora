
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

#include "core/toresultplan.h"
#include "core/utils.h"
#include "core/toeventquery.h"
#include "core/toresultcombo.h"
#include "core/toconfiguration.h"
//#include "core/tosqlparse.h"

toResultPlan::toResultPlan(QWidget *parent, const char *name)
    : toResultView(false, false, parent, name)
    , Explaining(false)
{
    setSQLName(QString::fromLatin1("toResultPlan"));
    setAlternatingRowColors(true);
    oracleSetup();
}

static toSQL SQLVSQLChildSel("toResultPlan:VSQLChildSel",
                             "SELECT distinct to_char(child_number)||' ('||to_char(plan_hash_value)||')' cn_disp, child_number, sql_id, plan_hash_value\n"
                             "FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = :addr<char[40],in> \n"
                             "ORDER BY child_number",
                             "Get list of child plans for cursor",
                             "1000");

static toSQL SQLVSQLChildSel9("toResultPlan:VSQLChildSel",
                              "SELECT distinct to_char(child_number) cn_disp, child_number, null sql_id, null plan_hash_value\n"
                              "FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = :addr<char[40],in> \n"
                              "ORDER BY child_number",
                              "",
                              "0900");

static toSQL SQLViewVSQLPlan("toResultPlan:ViewVSQLPlan",
                             "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                             "  io_cost,Bytes,Cardinality,partition_start,partition_stop,\n"
                             "  temp_space,time,access_predicates,filter_predicates\n"
                             "FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = :addr<char[40],in> and child_number = :chld<char[10],in> \n"
                             "ORDER BY NVL(Parent_ID,0),ID",
                             "Get the contents of SQL plan from V$SQL_PLAN.",
                             "1000");

static toSQL SQLViewVSQLPlan92("toResultPlan:ViewVSQLPlan",
                               "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                               "  io_cost,Bytes,Cardinality,partition_start,partition_stop,\n"
                               "  temp_space,null time,access_predicates,filter_predicates\n"
                               "FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = :addr<char[40],in> and child_number = :chld<char[10],in>\n"
                               "ORDER BY NVL(Parent_ID,0),ID",
                               "",
                               "0902");

static toSQL SQLViewVSQLPlan9("toResultPlan:ViewVSQLPlan",
                              "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                              "  io_cost,Bytes,Cardinality,partition_start,partition_stop,\n"
                              "  null temp_space,null time,access_predicates,filter_predicates\n"
                              "FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = :addr<char[40],in> and child_number = :chld<char[10],in>\n"
                              "ORDER BY NVL(Parent_ID,0),ID",
                              "",
                              "0900");

static toSQL SQLViewPlan("toResultPlan:ViewPlan",
                         "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                         "  io_cost,Bytes,Cardinality,partition_start,partition_stop,\n"
                         "  temp_space,time,access_predicates,filter_predicates\n"
                         "  FROM %1 WHERE Statement_ID = '%2' ORDER BY NVL(Parent_ID,0),ID",
                         "Get the contents of a plan table. Observe the %1 and %2 which must be present. Must return same columns",
                         "1000");

static toSQL SQLViewPlan8("toResultPlan:ViewPlan",
                          "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                          "  io_cost,Bytes,Cardinality,partition_start,partition_stop,\n"
                          "  ' ',' ',' 'access_predicates,' 'filter_predicates\n"
                          "  FROM %1 WHERE Statement_ID = '%2' ORDER BY NVL(Parent_ID,0),ID",
                          "",
                          "0800");

bool toResultPlan::canHandle(const toConnection &conn)
{
    return
        conn.providerIs("Oracle") ||
        conn.providerIs("QMYSQL")  ||
        conn.providerIs("PostgreSQL");
}

void toResultPlan::oracleSetup(void)
{
    clear();

    setAllColumnsShowFocus(true);
    setSorting( -1);
    setRootIsDecorated(true);

    QTreeWidgetItem *header = new QTreeWidgetItem();
    header->setText(0, QString::fromLatin1("#"));
    header->setText(1, tr("Operation"));
    header->setText(2, tr("Options"));
    header->setText(3, tr("Object name"));
    header->setText(4, tr("Mode"));
    header->setText(5, tr("Cost"));
    header->setText(6, tr("%CPU"));
    header->setText(7, tr("Bytes"));
    header->setText(8, tr("Rows"));
    header->setText(9, tr("Time"));
    header->setText(10, tr("Access pred."));
    header->setText(11, tr("Filter pred."));
    header->setText(12, tr("TEMP Space"));
    header->setText(13, tr("Startpartition"));
    header->setText(14, tr("Endpartition"));
    setHeaderItem(header);
}

// Connects query signals to appropriate slots. Created just in order not to repeat code...
void toResultPlan::connectSlotsAndStart()
{
    connect(Query, SIGNAL(dataAvailable(toEventQuery*)), this, SLOT(slotPoll()));
    connect(Query, SIGNAL(done(toEventQuery*)), this, SLOT(slotQueryDone()));
    connect(Query, SIGNAL(error(toEventQuery*,toConnection::exception const &)), this, SLOT(slotErrorHanler(toEventQuery*, toConnection::exception  const &)));
    Query->start();
}

//void toResultPlan::oracleNext()
//{
//    LastTop = NULL;
//    Parents.clear();
//    Last.clear();
//
//    toConnection &conn = connection();
//    //toConnectionSubLoan connSub(conn);
//
//    //connSub->execute(QString::fromLatin1("SAVEPOINT %1").arg(chkPoint));
//
//    Ident = QString::fromLatin1("TOra ") + QString::number((int)time(NULL) + rand());
//
//    QString planTable(toConfigurationSingle::Instance().planTable(conn.user()));
//
//    QString sql = Utils::toShift(Statements);
//    if (sql.isNull())
//    {
//        return ;
//    }
//    if (sql.length() > 0 && sql.at(sql.length() - 1).toLatin1() == ';')
//        sql = sql.mid(0, sql.length() - 1);
//
//    QString explain = QString::fromLatin1("EXPLAIN PLAN SET STATEMENT_ID = '%1' INTO %2 FOR %3").
//                      arg(Ident).arg(planTable).arg(Utils::toSQLStripSpecifier(sql));
//
//    if (!Schema.isNull() && Schema != conn.user().toUpper())
//    {
//        //try
//        //{
//        //    connSub->execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = \"%1\"").arg(User));
//        //    connSub->execute(explain);
//        //}
//        //catch (...)
//        //{
//        //    try
//        //    {
//        //        // conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = %1").arg(connection().user()));
//        //        // when we start connection it is for user but in schema context
//        //        connSub->execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = \"%1\"").arg(connection().schema()));
//        //    }
//        //    catch (...)
//        //    {
//        //        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
//        //    }
//        //    throw;
//        //}
//        ////conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = %1").arg(connection().user()));
//        //when we start connection it is for user but in schema context
//        //connSub->execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = \"%1\"").arg(connection().schema()));
//        Query = new toEventQuery(this
//        		                 , conn
//                                 , toSQL::string(SQLViewPlan, conn).
//                                   // arg(toConfigurationSingle::Instance().planTable()).
//                                   // Since EXPLAIN PLAN is always to conn.user() plan_table
//                                   arg(explain).arg(Ident)
//								 , toQueryParams()
//								 , toEventQuery::READ_ALL
//								 );
//        connectSlotsAndStart();
//        Explaining = false;
//    }
//    else
//    {
//        Explaining = true;
//		Query = new toEventQuery(this, conn, explain, toQueryParams(), toEventQuery::READ_ALL);
//        connectSlotsAndStart();
//    }
//    TopItem = new toResultViewItem(this, TopItem, QString::fromLatin1("EXPLAIN PLAN:"));
//    TopItem->setText(1, sql.left(50).trimmed());
//}

//static void StripInto(std::list<toSQLParse::statement> &stats)
//{
//    std::list<toSQLParse::statement> res;
//    bool into = false;
//    bool add
//    = true;
//    for (std::list<toSQLParse::statement>::iterator i = stats.begin(); i != stats.end(); i++)
//    {
//        if (into)
//        {
//            if (!add
//                    && (*i).String.toUpper() == QString::fromLatin1("FROM"))
//                add
//                = true;
//        }
//        else if ((*i).String.toUpper() == QString::fromLatin1("INTO"))
//        {
//            add
//            = false;
//            into = true;
//        }
//        if (add
//           )
//            res.insert(res.end(), *i);
//    }
//    stats = res;
//}

//void toResultPlan::addStatements(std::list<toSQLParse::statement> &stats)
//{
//    for (std::list<toSQLParse::statement>::iterator i = stats.begin(); i != stats.end(); i++)
//    {
//        if ((*i).Type == toSQLParse::statement::Block)
//            addStatements((*i).subTokens());
//        else if ((*i).Type == toSQLParse::statement::Statement)
//        {
//            if ((*i).subTokens().begin() != (*i).subTokens().end())
//            {
//                QString t = (*((*i).subTokens().begin())).String.toUpper();
//                if (t == QString::fromLatin1("SELECT"))
//                    StripInto((*i).subTokens());
//
//                if (t == QString::fromLatin1("SELECT") ||
//                        t == QString::fromLatin1("INSERT") ||
//                        t == QString::fromLatin1("UPDATE") ||
//                        t == QString::fromLatin1("DELETE"))
//                    Statements.insert(Statements.end(),
//                                      toSQLParse::indentStatement(*i).trimmed());
//            }
//        }
//    }
//}

void toResultPlan::query(const QString &sql, toQueryParams const& param)
{
    if (!handled())
        return ;

    try
    {
    	setSqlAndParams(sql, param);

        toConnection &conn = connection();
        if (conn.providerIs("QMYSQL") || conn.providerIs("QPSQL"))
        {
            setRootIsDecorated(false);
            setSorting(0);
            toResultView::query(QString::fromLatin1("EXPLAIN ") + Utils::toSQLStripBind(sql), param);
            return ;
        }

        clear();

        Statements.clear();
        LastTop = NULL;
        Parents.clear();
        Parents.clear();
        Last.clear();

        // Display already saved plan
        if (sql.startsWith(QString::fromLatin1("SAVED:")))
        {
            Explaining = false;
            Ident = sql.mid(6);
            TopItem = new toResultViewItem(this, NULL, QString::fromLatin1("DML"));
            TopItem->setText(1, QString::fromLatin1("Saved plan"));
        	QString planTable(toConfigurationSingle::Instance().planTable(connection().user()));
            Query = new toEventQuery(this
				                     , connection()
				                     // NOTE: this is special case - plan table is defined as %1. table name can not use bind variable place holder
                                     , toSQL::string(SQLViewPlan, connection()).arg(planTable).arg(Ident)
                                     , toQueryParams()
				                     , toEventQuery::READ_ALL);
            connectSlotsAndStart();

        }
        // Display plan held in SGA - the plan will be diplayed when slotChildComboReady is called
        else if (sql.startsWith(QString::fromLatin1("SGA:")))
        {
            Explaining = false;
            Ident = sql.mid(4);
            QString queryText = Utils::toSQLString(conn, Ident);
            TopItem = new toResultViewItem(this, NULL, QString::fromLatin1("V$SQL_PLAN:"));
            TopItem->setText(1, queryText.left(50).trimmed());
            TopItem->setToolTip(1, queryText);

            CursorChildSel = new toResultCombo(this, "toResultPlan");
            CursorChildSel->setSQL(SQLVSQLChildSel);            
            CursorChildSel->setSelectionPolicy(toResultCombo::First);
            try
            {
				CursorChildSel->refreshWithParams(toQueryParams() << Ident);
            }
            TOCATCH;
            setItemWidget(TopItem, 3, CursorChildSel);
            connect(CursorChildSel, SIGNAL(done()), this, SLOT(slotChildComboReady())); //Wait for cursor children combo to fill
        }
        // Execute EXPLAIN PLAN FOR ...
        else
        {
        	//throw QString("toResultPlan::query EXPLAIN PLAN FOR ... not implemented yet.");
        	{
        		QSharedPointer<toConnectionSubLoan> conn(new toConnectionSubLoan(connection()));
        		this->LockedConnection = conn;
        	}
            Explaining = true;
            Ident = QString::fromLatin1("TOra ") + QString::number(QDateTime::currentMSecsSinceEpoch()/1000 + rand());
            TopItem = new toResultViewItem(this, NULL, QString::fromLatin1("EXPLAIN PLAN:"));
            TopItem->setText(1, sql.left(50).trimmed());

            QString planTable(toConfigurationSingle::Instance().planTable(conn.user()));

            QString explain = QString::fromLatin1("EXPLAIN PLAN SET STATEMENT_ID = '%1' INTO %2 FOR %3").
                              arg(Ident).
                              arg(planTable).
                              arg(Utils::toSQLStripSpecifier(sql));

    		Query = new toEventQuery(this, LockedConnection, explain, toQueryParams(), toEventQuery::READ_ALL);
            connectSlotsAndStart();
        }
    }
    catch (const QString &str)
    {
        checkException(str);
    }
}

void toResultPlan::cleanup()
{
    if (Query)
    {
        disconnect(Query, 0, this, 0);

        Query->stop();
        delete Query;
        Query = NULL;
    }
}

void toResultPlan::slotPoll(void)
{
    if (!Query)
    {
        cleanup();
        return;
    }

    if (!Utils::toCheckModal(this))
        return;

    try
    {
        while (Query->hasMore())
        {
            QString id = Query->readValue();
            QString parentid = Query->readValue();
            QString operation = Query->readValue();
            QString options = Query->readValue();
            QString object = Query->readValue();
            QString optimizer = Query->readValue();
            QString cost = Query->readValue();
            QString iocost = Query->readValue();
            QString bytes = Query->readValue().toSIsize();
            QString cardinality = Query->readValue();
            QString startpartition = Query->readValue();
            QString endpartition = Query->readValue();
            QString tempspace = Query->readValue().toSIsize();
            QString time = Query->readValue();
            QString accesspred = Query->readValue();
            QString filterpred = Query->readValue();

            toResultViewItem *item;
            if (!parentid.isNull() && Parents[parentid])
            {
                item = new toResultViewItem(Parents[parentid], Last[parentid]);
//            setOpen(Parents[parentid], true);
                Last[parentid] = item;
            }
            else
            {
                item = new toResultViewItem(TopItem, LastTop);
                LastTop = item;
            }

            QString cpupct = NULL;

            if (!cost.isEmpty())
            {
                double pct = 100;

                if (cost.toDouble() > 0)
                {
                    pct = 100 - (iocost.toDouble() / cost.toDouble() * 100);
                }

                cpupct.setNum(pct, 'f', 2);
            }

            if (!time.isEmpty())
            {
                double seconds = time.toDouble();

                int hours = (int) (seconds / 3600);
                int mins = (int) (( seconds - hours * 3600) / 60);
                int secs = (int) seconds - (hours * 3600 + mins * 60);

                time.sprintf("%d:%02d:%02d", hours, mins, secs);
            }

            item->setText(0, id);
            item->setText(1, operation);
            item->setText(2, options);
            item->setText(3, object);
            item->setText(4, optimizer);
            item->setText(5, cost);
            item->setTextAlignment (5, Qt::AlignRight);
            item->setText(6, cpupct);
            item->setTextAlignment (6, Qt::AlignRight);
            item->setText(7, bytes);
            item->setTextAlignment (7, Qt::AlignRight);
            item->setText(8, cardinality);
            item->setTextAlignment (8, Qt::AlignRight);
            item->setText(9, time);
            item->setText(10, accesspred);
            item->setToolTip(10, accesspred);
            item->setSizeHint(10, QSize(120, 0));
            item->setText(11, filterpred);
            item->setToolTip(11, filterpred);
            item->setSizeHint(11, QSize(120, 0));
            item->setText(12, tempspace);
            item->setTextAlignment (12, Qt::AlignRight);
            item->setText(13, startpartition);
            item->setText(14, endpartition);
            Parents[id] = item;
        }
        expandAll();
        resizeColumnsToContents();
    }
    catch (const QString &str)
    {
        Query = NULL;
        checkException(str);
    }
}

void toResultPlan::slotChildComboReady()
{
    Explaining = false;
	if(CursorChildSel->currentIndex () == -1)
		return;
    toConnection &conn = connection();
    QStringList cur_sel = CursorChildSel->itemData(0).toStringList();
    QString ChildNumber = cur_sel.at(0);
    QString SInfo = QString::fromLatin1("V$SQL_PLAN: %1\nChild: %2 SQL_ID: %3").arg(cur_sel.at(2)).arg(ChildNumber).arg(cur_sel.at(1));
    TopItem->setText(0, SInfo);
    connect(CursorChildSel, SIGNAL(currentIndexChanged (int) ), this, SLOT(slotChildComboChanged(int)));

    Query = new toEventQuery(this
    		                 , conn
    		                 , toSQL::string(SQLViewVSQLPlan, connection())
                             , toQueryParams() << Ident << ChildNumber
							 , toEventQuery::READ_ALL);
    connectSlotsAndStart();
}

void toResultPlan::slotChildComboChanged(int NewIndex)
{
    Explaining = false;
    if (NewIndex > -1 )
    {
        toConnection &conn = connection();
        QStringList cur_sel = CursorChildSel->itemData(NewIndex).toStringList();
        QString ChildNumber = cur_sel.at(0);
        QString SInfo = QString::fromLatin1("V$SQL_PLAN: %1\nChild: %2 SQL_ID: %3").arg(cur_sel.at(2)).arg(ChildNumber).arg(cur_sel.at(1));
        TopItem->setText(0, SInfo);

        /*
             std::map <QString, toTreeWidgetItem *>::reverse_iterator it;
             for (it=Parents.rbegin(); it!=Parents.rend(); it++) {printf("Aqq %s\n",it->first.toAscii().constData()); delete it->second;}
        */
        TopItem->deleteChildren();
        LastTop = NULL;
        Parents.clear();
        Last.clear();

        Query = new toEventQuery(this
        		                 , conn
                                 , toSQL::string(SQLViewVSQLPlan, connection())
                                 , toQueryParams() << Ident << ChildNumber
								 , toEventQuery::READ_ALL);
        connectSlotsAndStart();
    }
}

/** this ugly function handles various query ends:
 * - v$sql_plan child combo query
 * - v$sql_plan query
 * - EXLPLAIN PLAN FOR ... query
 * - SELECT ... FROM PLAN_TABLE QUERY
 */
void toResultPlan::slotQueryDone()
{
    Query = NULL;
    if (Explaining)
    {
    	Explaining = false;
        toQueryParams par;
        toConnection &conn(connection());

        Query = new toEventQuery(this
        		                 , LockedConnection
        		                 , toSQL::string(SQLViewPlan, conn).
                                 // arg(toConfigurationSingle::Instance().planTable()).
                                 // Since EXPLAIN PLAN is always to conn.user() plan_table
                                 // and current_schema can be different
                                 arg(toConfigurationSingle::Instance().planTable(conn.user())).
                                 arg(Ident)
                                 , toQueryParams()
                                 , toEventQuery::READ_ALL);
        connectSlotsAndStart();
    }
    else if (sql().startsWith(QString::fromLatin1("SGA:")))
    {
        return;
    }
    else
    {
        //if (!sql().startsWith(QString::fromLatin1("SAVED:")))
        //{
        //	Utils::toStatusMessage("Not implemented yet toResultPlan::queryDone");
        //    //if (!toConfigurationSingle::Instance().keepPlans())
        //    //    connection().execute(QString::fromLatin1("ROLLBACK TO SAVEPOINT %1").arg(chkPoint));
        //    //else
        //    //    toMainWidget()->setNeedCommit(connection());
        //}
        //oracleNext();
		LockedConnection.clear();
    }
} // queryDone

void toResultPlan::slotErrorHanler(toEventQuery*, toConnection::exception  const &e)
{
	checkException(e);
}

void toResultPlan::checkException(const QString &str)
{
    try
    {
        if (str.contains(QString::fromLatin1("ORA-02404")))
        {
            QString planTable(toConfigurationSingle::Instance().planTable(connection().user()));

            // if shared plan table does not exist, do not try to create it
            if(toConfigurationSingle::Instance().sharedPlan())
            {
                TOMessageBox::warning(this,
                                      tr("Plan table doesn't exist"),
                                      tr("Specified plan table %1 doesn't exist.").arg(planTable),
                                      tr("&OK"));
            }
            else
            {
                int ret = TOMessageBox::warning(this,
                                                tr("Plan table doesn't exist"),
                                                tr("Specified plan table %1 doesn't exist.\n"
                                                   "Should TOra try to create it?").arg(planTable),
                                                tr("&Yes"), tr("&No"), QString::null, 0, 1);
                if (ret == 0)
                {
                	Utils::toBusy busy;
                	toConnectionSubLoan conn(connection());
                	toQuery createPlanTable(conn, toSQL::string(toSQL::TOSQL_CREATEPLAN, connection()).arg(planTable), toQueryParams());
                	createPlanTable.eof();
                }
            }
        }
        else
            Utils::toStatusMessage(str);
    }
    TOCATCH
}
