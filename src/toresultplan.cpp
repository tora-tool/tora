
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

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "toeventquery.h"
#include "toresultplan.h"
#include "tosql.h"
#include "totool.h"

#include <time.h>

#include <map>

#include <qmessagebox.h>


toResultPlan::toResultPlan(QWidget *parent, const char *name)
        : toResultView(false, false, parent, name)
{
    setSQLName(QString::fromLatin1("toResultPlan"));
    setAlternatingRowColors(true);
    Query = NULL;
    oracleSetup();
}

static toSQL SQLVSQLChildSel("toResultPlan:VSQLChildSel",
			 "SELECT distinct to_char(child_number)||' ('||to_char(plan_hash_value)||')' cn_disp, child_number, sql_id, plan_hash_value\n"
			 "FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = '%1'\n"
			 "ORDER BY child_number",
			 "Get list of child plans for cursor",
			 "1000");

static toSQL SQLVSQLChildSel9("toResultPlan:VSQLChildSel",
			 "SELECT distinct to_char(child_number) cn_disp, child_number, null sql_id, null plan_hash_value\n"
			 "FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = '%1'\n"
			 "ORDER BY child_number",
			 "",
			 "0900");

static toSQL SQLViewVSQLPlan("toResultPlan:ViewVSQLPlan",
                         "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                         "  io_cost,Bytes,Cardinality,partition_start,partition_stop,\n"
                         "  temp_space,time,access_predicates,filter_predicates\n"
                         "FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = '%1' and child_number = %2\n"
			 "ORDER BY NVL(Parent_ID,0),ID",
                         "Get the contents of SQL plan from V$SQL_PLAN.",
                         "1000");

static toSQL SQLViewVSQLPlan92("toResultPlan:ViewVSQLPlan",
                         "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                         "  io_cost,Bytes,Cardinality,partition_start,partition_stop,\n"
                         "  temp_space,null time,access_predicates,filter_predicates\n"
                         "FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = '%1' and child_number = %2\n"
			 "ORDER BY NVL(Parent_ID,0),ID",
                         "",
                         "0902");

static toSQL SQLViewVSQLPlan9("toResultPlan:ViewVSQLPlan",
                         "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                         "  io_cost,Bytes,Cardinality,partition_start,partition_stop,\n"
                         "  null temp_space,null time,access_predicates,filter_predicates\n"
                         "FROM V$SQL_PLAN WHERE Address||':'||Hash_Value = '%1' and child_number = %2\n"
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

bool toResultPlan::canHandle(toConnection &conn)
{
    return toIsOracle(conn) || toIsMySQL(conn) || toIsPostgreSQL(conn);
}

toResultPlan::~toResultPlan()
{
    delete Query;
}

void toResultPlan::oracleSetup(void)
{
    clear();

    setAllColumnsShowFocus(true);
    setSorting( -1);
    setRootIsDecorated(true);
    
    QTreeWidgetItem *header = new QTreeWidgetItem();
    header->setText(0, QString::fromLatin1("#"));
    header->setText(1,tr("Operation"));
    header->setText(2,tr("Options"));
    header->setText(3,tr("Object name"));
    header->setText(4,tr("Mode"));
    header->setText(5,tr("Cost"));
    header->setText(6,tr("%CPU"));
    header->setText(7,tr("Bytes"));
    header->setText(8,tr("Rows"));
    header->setText(9,tr("Time"));
    header->setText(10,tr("Access pred."));
    header->setText(11,tr("Filter pred."));
    header->setText(12,tr("TEMP Space"));
    header->setText(13,tr("Startpartition"));
    header->setText(14,tr("Endpartition"));
    setHeaderItem(header);
}

// Connects query signals to appropriate slots. Created just in order not to repeat code...
void toResultPlan::connectSlotsAndStart()
{
    connect(Query, SIGNAL(dataAvailable()), this, SLOT(poll()));
    connect(Query, SIGNAL(done()), this, SLOT(queryDone()));
    Query->readAll(); // indicate that all records should be fetched
    Query->start();
} // connectSlotsAndStart

void toResultPlan::oracleNext(void)
{
    LastTop = NULL;
    Parents.clear();
    Last.clear();

    QString chkPoint(toConfigurationSingle::Instance().planCheckpoint());

    toConnection &conn = connection();

    conn.execute(QString::fromLatin1("SAVEPOINT %1").arg(chkPoint));

    Ident = QString::fromLatin1("TOra ") + QString::number((int)time(NULL) + rand());

    QString planTable(toConfigurationSingle::Instance().planTable(conn.user()));

    QString sql = toShift(Statements);
    if (sql.isNull())
    {
       return ;
    }
    if (sql.length() > 0 && sql.at(sql.length() - 1).toLatin1() == ';')
        sql = sql.mid(0, sql.length() - 1);

    QString explain = QString::fromLatin1("EXPLAIN PLAN SET STATEMENT_ID = '%1' INTO %2 FOR %3").
                      arg(Ident).arg(planTable).arg(toSQLStripSpecifier(sql));
	
    if (!User.isNull() && User != conn.user().toUpper())
    {
        try
        {
            conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = \"%1\"").arg(User));
            conn.execute(explain);
        }
        catch (...)
        {
            try
            {
                // conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = %1").arg(connection().user()));
                // when we start connection it is for user but in schema context
                conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = \"%1\"").arg(connection().schema()));
            }
            catch (...)
                {}
            throw;
        }
        //conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = %1").arg(connection().user()));
        //when we start connection it is for user but in schema context
        conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = \"%1\"").arg(connection().schema()));
        toQList par;
        Query = new toEventQuery(connection(), toQuery::Normal,
                                 toSQL::string(SQLViewPlan, conn).
                                 // arg(toConfigurationSingle::Instance().planTable()).
                                 // Since EXPLAIN PLAN is always to conn.user() plan_table
                                 arg(explain).
                                 arg(Ident), par);
        connectSlotsAndStart();
        Reading = true;
    }
    else
    {
        Reading = false;
        toQList par;
        Query = new toEventQuery(conn, toQuery::Normal, explain, par);
        connectSlotsAndStart();
    }
    TopItem = new toResultViewItem(this, TopItem, QString::fromLatin1("EXPLAIN PLAN:"));
    TopItem->setText(1, sql.left(50).trimmed());
}

static void StripInto(std::list<toSQLParse::statement> &stats)
{
    std::list<toSQLParse::statement> res;
    bool into = false;
    bool add
    = true;
    for (std::list<toSQLParse::statement>::iterator i = stats.begin();i != stats.end();i++)
    {
        if (into)
        {
            if (!add
                    && (*i).String.toUpper() == QString::fromLatin1("FROM"))
                add
                = true;
        }
        else if ((*i).String.toUpper() == QString::fromLatin1("INTO"))
        {
            add
            = false;
            into = true;
        }
        if (add
           )
            res.insert(res.end(), *i);
    }
    stats = res;
}

void toResultPlan::addStatements(std::list<toSQLParse::statement> &stats)
{
    for (std::list<toSQLParse::statement>::iterator i = stats.begin();i != stats.end();i++)
    {
        if ((*i).Type == toSQLParse::statement::Block)
            addStatements((*i).subTokens());
        else if ((*i).Type == toSQLParse::statement::Statement)
        {
            if ((*i).subTokens().begin() != (*i).subTokens().end())
            {
                QString t = (*((*i).subTokens().begin())).String.toUpper();
                if (t == QString::fromLatin1("SELECT"))
                    StripInto((*i).subTokens());

                if (t == QString::fromLatin1("SELECT") ||
                        t == QString::fromLatin1("INSERT") ||
                        t == QString::fromLatin1("UPDATE") ||
                        t == QString::fromLatin1("DELETE"))
                    Statements.insert(Statements.end(),
                                      toSQLParse::indentStatement(*i).trimmed());
            }
        }
    }
}

void toResultPlan::query(const QString &sql,
                         const toQList &param)
{
    if (!handled())
        return ;

    try
    {
        if (!setSQLParams(sql, param))
            return ;

        if (Query)
        {
            delete Query;
            Query = NULL;
        }
        if (connection().provider() == "MySQL" || connection().provider() == "PostgreSQL")
        {
            setRootIsDecorated(false);
            setSorting(0);
            toResultView::query(QString::fromLatin1("EXPLAIN ") + toSQLStripBind(sql), param);
            return ;
        }

        toQList::iterator cp = ((toQList &)param).begin();
        if (cp != ((toQList &)param).end())
            User = *cp;
        else
            User = QString::null;

        clear();

        QString planTable(toConfigurationSingle::Instance().planTable(connection().user()));

        Statements.clear();
        if (sql.startsWith(QString::fromLatin1("SAVED:")))
        {
            Ident = sql.mid(6);
            toQList par;
            Query = new toEventQuery(connection(), toQuery::Background,
                                     toSQL::string(SQLViewPlan, connection()).
                                     arg(planTable).arg(Ident),
                                     par);
            connectSlotsAndStart();
            Reading = true;
            LastTop = NULL;
            Parents.clear();
            Last.clear();
            TopItem = new toResultViewItem(this, NULL, QString::fromLatin1("DML"));
            TopItem->setText(1, QString::fromLatin1("Saved plan"));
        }
        else if (sql.startsWith(QString::fromLatin1("SGA:")))
        {
            Ident = sql.mid(4);
	    toConnection &conn = connection();
	    
	    LastTop = NULL;
            Parents.clear();
            Last.clear();
            TopItem = new toResultViewItem(this, NULL, QString::fromLatin1("V$SQL_PLAN:"));
	    QString queryText = toSQLString(conn, Ident);
            TopItem->setText(1, queryText.left(50).trimmed());
            TopItem->setToolTip(1, queryText);

	    CursorChildSel = new toResultCombo(this, "toResultPlan");
	    CursorChildSel->setSQL(toSQL::string(SQLVSQLChildSel, conn).arg(Ident));
	    CursorChildSel->setSelectionPolicy(toResultCombo::First);
	    try
	    {
			CursorChildSel->refresh();
	    }
	    TOCATCH;
	    setItemWidget(TopItem, 3, CursorChildSel);
	    connect(CursorChildSel, SIGNAL(done()), this, SLOT(ChildComboReady())); //Wait for cursor children combo to fill
       }
	else
        {
            TopItem = NULL;
            std::list<toSQLParse::statement> ret = toSQLParse::parse(sql);
            addStatements(ret);
            oracleNext();
        }
    }
    catch (const QString &str)
    {
        checkException(str);
    }
}

void toResultPlan::poll(void)
{
    if (!toCheckModal(this))
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
	    item->setTextAlignment (5,Qt::AlignRight);
            item->setText(6, cpupct);
	    item->setTextAlignment (6,Qt::AlignRight);
            item->setText(7, bytes);
	    item->setTextAlignment (7,Qt::AlignRight);
            item->setText(8, cardinality);
	    item->setTextAlignment (8,Qt::AlignRight);
            item->setText(9, time);
            item->setText(10, accesspred);
            item->setToolTip(10, accesspred);
            item->setSizeHint(10, QSize(120,0));
            item->setText(11, filterpred);
            item->setToolTip(11, filterpred);
            item->setSizeHint(11, QSize(120,0));
            item->setText(12, tempspace);
	    item->setTextAlignment (12,Qt::AlignRight);
            item->setText(13, startpartition);
            item->setText(14, endpartition);
            Parents[id] = item;
        }
        expandAll();
        resizeColumnsToContents();
    }
    catch (const QString &str)
    {
        delete Query;
        Query = NULL;
        checkException(str);
    }
}

void toResultPlan::ChildComboReady()
{
    toConnection &conn = connection();
    QStringList cur_sel = CursorChildSel->itemData(0).toStringList();
    QString ChildNumber = cur_sel.at(0);
    QString SInfo = QString::fromLatin1("V$SQL_PLAN: %1\nChild: %2 SQL_ID: %3").arg(cur_sel.at(2)).arg(ChildNumber).arg(cur_sel.at(1));
    TopItem->setText(0,SInfo);
    connect(CursorChildSel, SIGNAL(currentIndexChanged (int) ), this, SLOT(ChildComboChanged(int)));

    toQList par;
    Query = new toEventQuery(conn, toQuery::Background,
                             toSQL::string(SQLViewVSQLPlan, conn).arg(Ident).arg(ChildNumber),
                             par);
    connectSlotsAndStart();
    Reading = true;
}

void toResultPlan::ChildComboChanged(int NewIndex)
{
    if (NewIndex > -1 )
    {
      toConnection &conn = connection();
      QStringList cur_sel = CursorChildSel->itemData(NewIndex).toStringList();
      QString ChildNumber = cur_sel.at(0);
      QString SInfo = QString::fromLatin1("V$SQL_PLAN: %1\nChild: %2 SQL_ID: %3").arg(cur_sel.at(2)).arg(ChildNumber).arg(cur_sel.at(1));
      TopItem->setText(0,SInfo);
      
 /*
      std::map <QString, toTreeWidgetItem *>::reverse_iterator it;
      for (it=Parents.rbegin(); it!=Parents.rend(); it++) {printf("Aqq %s\n",it->first.toAscii().constData()); delete it->second;}
 */
      TopItem->deleteChildren();
      LastTop = NULL;
      Parents.clear();
      Last.clear();
      toQList par;
      Query = new toEventQuery(conn, toQuery::Background,
			      toSQL::string(SQLViewVSQLPlan, conn).arg(Ident).arg(ChildNumber),
			      par);
      connectSlotsAndStart();
      Reading = true;
    }
}


void toResultPlan::queryDone()
{
    delete Query;
    Query = NULL;
    if (!Reading)
    {
        toQList par;
        toConnection &conn = connection();
        Query = new toEventQuery(connection(), toQuery::Normal,
                                 toSQL::string(SQLViewPlan, conn).
                                 // arg(toConfigurationSingle::Instance().planTable()).
                                 // Since EXPLAIN PLAN is always to conn.user() plan_table
                                 // and current_schema can be different
                                 arg(toConfigurationSingle::Instance().planTable(conn.user())).
                                 arg(Ident), par);
        connectSlotsAndStart();
        Reading = true;
    }
    else if (sql().startsWith(QString::fromLatin1("SGA:")))
    {
	return;
    }
    else
    {
        QString chkPoint(toConfigurationSingle::Instance().planCheckpoint());
        if (!sql().startsWith(QString::fromLatin1("SAVED:")))
        {
            if (toConfigurationSingle::Instance().keepPlans())
                connection().execute(QString::fromLatin1("ROLLBACK TO SAVEPOINT %1").arg(chkPoint));
            else
                toMainWidget()->setNeedCommit(connection());
        }
        oracleNext();
    }
} // queryDone

void toResultPlan::checkException(const QString &str)
{
    try
    {
        if (str.startsWith(QString::fromLatin1("ORA-02404")))
        {
            QString planTable(toConfigurationSingle::Instance().planTable(connection().user()));

            // if shared plan table does not exist, do not try to create it
            if(toConfigurationSingle::Instance().sharedPlan()) {
                TOMessageBox::warning(this,
                                      tr("Plan table doesn't exist"),
                                      tr("Specified plan table %1 doesn't exist.").arg(planTable),
                                      tr("&OK"));
            } else {
                int ret = TOMessageBox::warning(this,
                                                tr("Plan table doesn't exist"),
                                                tr("Specified plan table %1 doesn't exist.\n"
                                                   "Should TOra try to create it?").arg(planTable),
                                                tr("&Yes"), tr("&No"), QString::null, 0, 1);
                if (ret == 0)
                {
                    connection().execute(toSQL::string(toSQL::TOSQL_CREATEPLAN,
                                                       connection()).arg(planTable));
                    QString t = sql();
                    setSQL(QString::null);
                    query(t, params());
                }
            }
        }
        else
            toStatusMessage(str);
    }
    TOCATCH
}
