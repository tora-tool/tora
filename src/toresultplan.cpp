
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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
#include "tonoblockquery.h"
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
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
    setAlternatingRowColors(true);
    Query = NULL;
    oracleSetup();
}

static toSQL SQLViewPlan("toResultPlan:ViewPlan",
                         "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                         "  io_cost,Bytes,Cardinality,\n"
                         "  partition_start,partition_stop,temp_space,time\n"
                         "  FROM %1 WHERE Statement_ID = '%2' ORDER BY NVL(Parent_ID,0),ID",
                         "Get the contents of a plan table. Observe the %1 and %2 which must be present. Must return same columns",
                         "1000");

static toSQL SQLViewPlan8("toResultPlan:ViewPlan",
                          "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                          "  io_cost,Bytes,Cardinality,\n"
                          "  partition_start,partition_stop,' ',' '\n"
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
    addColumn(QString::fromLatin1("#"));
    addColumn(tr("Operation"));
    addColumn(tr("Options"));
    addColumn(tr("Object name"));
    addColumn(tr("Mode"));
    addColumn(tr("Cost"));
    addColumn(tr("%CPU"));
    addColumn(tr("Bytes"));
    addColumn(tr("Rows"));
    addColumn(tr("TEMP Space"));
    addColumn(tr("Time"));
    addColumn(tr("Startpartition"));
    addColumn(tr("Endpartition"));
    setColumnAlignment(5, Qt::AlignRight);
    setColumnAlignment(6, Qt::AlignRight);
    setColumnAlignment(7, Qt::AlignRight);
    setColumnAlignment(8, Qt::AlignRight);
    setColumnAlignment(9, Qt::AlignRight);
    setColumnAlignment(10, Qt::AlignRight);
    setColumnAlignment(11, Qt::AlignRight);
    setColumnAlignment(12, Qt::AlignRight);
}

void toResultPlan::oracleNext(void)
{
    LastTop = NULL;
    Parents.clear();
    Last.clear();

    QString chkPoint(toConfigurationSingle::Instance().planCheckpoint());

    toConnection &conn = connection();

    conn.execute(QString::fromLatin1("SAVEPOINT %1").arg(chkPoint));

    Ident = QString::fromLatin1("TOra ") + QString::number((int)time(NULL) + rand());

    QString planTable(toConfigurationSingle::Instance().planTable());

    QString sql = toShift(Statements);
    if (sql.isNull())
    {
        Poll.stop();
        return ;
    }
    if (sql.length() > 0 && sql.at(sql.length() - 1).toLatin1() == ';')
        sql = sql.mid(0, sql.length() - 1);

    QString explain = QString::fromLatin1("EXPLAIN PLAN SET STATEMENT_ID = '%1' INTO %2.%3 FOR %4").
                      arg(Ident).arg(connection().user()).arg(planTable).arg(toSQLStripSpecifier(sql));
    if (!User.isNull() && User != conn.user().toUpper())
    {
        try
        {
            conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = %1").arg(User));
            conn.execute(explain);
        }
        catch (...)
        {
            try
            {
                conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = %1").arg(connection().user()));
            }
            catch (...)
                {}
            throw;
        }
        conn.execute(QString::fromLatin1("ALTER SESSION SET CURRENT_SCHEMA = %1").arg(connection().user()));
        toQList par;
        Query = new toNoBlockQuery(connection(), toQuery::Normal,
                                   toSQL::string(SQLViewPlan, connection()).
                                   arg(toConfigurationSingle::Instance().planTable()).
                                   arg(Ident), par);
        Reading = true;
    }
    else
    {
        Reading = false;
        toQList par;
        Query = new toNoBlockQuery(conn, toQuery::Normal, explain, par);
    }
    TopItem = new toResultViewItem(this, TopItem, QString::fromLatin1("DML"));
    TopItem->setText(1, sql.left(50).trimmed());
    Poll.start(100);
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

        QString planTable(toConfigurationSingle::Instance().planTable());

        Statements.clear();
        if (sql.startsWith(QString::fromLatin1("SAVED:")))
        {
            Ident = sql.mid(6);
            toQList par;
            Query = new toNoBlockQuery(connection(), toQuery::Background,
                                       toSQL::string(SQLViewPlan, connection()).
                                       arg(planTable).arg(Ident),
                                       par);
            Reading = true;
            LastTop = NULL;
            Parents.clear();
            Last.clear();
            TopItem = new toResultViewItem(this, NULL, QString::fromLatin1("DML"));
            TopItem->setText(1, QString::fromLatin1("Saved plan"));
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
    try
    {
        if (!toCheckModal(this))
            return ;
        if (Query && Query->poll())
        {
            if (!Reading)
            {
                toQList par;
                delete Query;
                Query = NULL;
                Query = new toNoBlockQuery(connection(), toQuery::Normal,
                                           toSQL::string(SQLViewPlan, connection()).
                                           arg(toConfigurationSingle::Instance().planTable()).
                                           arg(Ident), par);
                Reading = true;
            }
            else
            {
                while (Query->poll() && !Query->eof())
                {
                    QString id = Query->readValueNull();
                    QString parentid = Query->readValueNull();
                    QString operation = Query->readValueNull();
                    QString options = Query->readValueNull();
                    QString object = Query->readValueNull();
                    QString optimizer = Query->readValueNull();
                    QString cost = Query->readValueNull();
                    QString iocost = Query->readValueNull();
                    QString bytes = Query->readValueNull().toSIsize();
                    QString cardinality = Query->readValueNull();
                    QString startpartition = Query->readValueNull();
                    QString endpartition = Query->readValueNull();
                    QString tempspace = Query->readValueNull().toSIsize();
                    QString time = Query->readValueNull();

                    toResultViewItem *item;
                    if (!parentid.isNull() && Parents[parentid])
                    {
                        item = new toResultViewItem(Parents[parentid], Last[parentid]);
//                         setOpen(Parents[parentid], true);
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
                    item->setText(6, cpupct);
                    item->setText(7, bytes);
                    item->setText(8, cardinality);
                    item->setText(9, tempspace);
                    item->setText(10, time);
                    item->setText(11, startpartition);
                    item->setText(12, endpartition);
                    Parents[id] = item;
                }
                if (Query->eof())
                {
                    delete Query;
                    Query = NULL;
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
            }
        }
        expandAll();
        resizeColumnsToContents();
    }
    catch (const QString &str)
    {
        delete Query;
        Query = NULL;
        Poll.stop();
        checkException(str);
    }
}

void toResultPlan::checkException(const QString &str)
{
    try
    {
        if (str.startsWith(QString::fromLatin1("ORA-02404")))
        {
            QString planTable(toConfigurationSingle::Instance().planTable());
            int ret = TOMessageBox::warning(this,
                                            tr("Plan table doesn't exist"),
                                            tr("Specified plan table %1 didn't exist.\n"
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
        else
            toStatusMessage(str);
    }
    TOCATCH
}
