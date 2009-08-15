
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

#include "toconnection.h"
#include "tonoblockquery.h"
#include "toresultstats.h"
#include "tosql.h"


toResultStats::toResultStats(bool onlyChanged, int ses, QWidget *parent,
                             const char *name)
        : toResultView(false, false, parent, name), OnlyChanged(onlyChanged)
{
    if (!handled())
        return ;

    SessionID = ses;
    setSQLName(QString::fromLatin1("toResultStats"));
    System = false;
    setup();
}

bool toResultStats::canHandle(toConnection &conn)
{
    return toIsOracle(conn);
}

bool toResultStats::close()
{
    delete Query;
    delete SessionIO;

    return true;
}

toResultStats::~toResultStats()
{
    close();
}

static toSQL SQLSession("toResultStats:Session",
                        "SELECT MIN(SID) FROM V$MYSTAT",
                        "Get session id of current session");

toResultStats::toResultStats(bool onlyChanged, QWidget *parent,
                             const char *name)
        : toResultView(false, false, parent, name), OnlyChanged(onlyChanged)
{
    if (!handled())
        return ;

    try
    {
        toQList params;
        toQuery query(connection(), toQuery::Background, SQLSession, params);
        SessionID = query.readValue().toInt();
    }
    catch (...)
    {
        SessionID = -1;
    }
    System = false;

    setup();
}

toResultStats::toResultStats(QWidget *parent, const char *name)
        : toResultView(false, false, parent, name), OnlyChanged(false)
{
    if (!handled())
        return ;

    System = true;
    setup();
}

void toResultStats::setup(void)
{
    for (int i = 0;i < TO_STAT_MAX;i++)
        LastValues[i] = 0;

    addColumn(tr("Name"));
    if (!OnlyChanged)
        addColumn(tr("Value"));
    addColumn(tr("Delta"));
    setSorting(0);

    setColumnAlignment(1, Qt::AlignRight);
    setColumnAlignment(2, Qt::AlignRight);

    Query = SessionIO = NULL;
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));

    connect(this,
            SIGNAL(sessionChanged(int)),
            this,
            SLOT(refreshStats()),
            Qt::QueuedConnection);
}

static toSQL SQLStatistics("toResultStats:Statistics",
                           "SELECT Statistic#,Value FROM V$SesStat WHERE SID = :f1<int>",
                           "Get statistics for session, must have same number of columns");
static toSQL SQLSessionIO("toResultStats:SessionIO",
                          "SELECT Block_Gets \"block gets\",\n"
                          "       Block_Changes \"block changes\",\n"
                          "       Consistent_Changes \"consistent changes\"\n"
                          "  FROM v$sess_io\n"
                          " WHERE SID = :f1<int>",
                          "Get session IO, must have same binds");
static toSQL SQLSystemStatistics("toResultStats:SystemStatistics",
                                 "SELECT Statistic#,Value FROM v$sysstat",
                                 "Get system statistics, must have same number of columns");

void toResultStats::resetStats(void)
{
    if (!handled())
        return ;

    toBusy busy;
    try
    {
        toConnection &conn = connection();
        toQList args;
        if (!System)
            args.insert(args.end(), SessionID);
        toQuery query(conn, System ? SQLSystemStatistics : SQLStatistics, args);
        while (!query.eof())
        {
            int id;
            double value;
            id = query.readValue().toInt();
            value = query.readValue().toDouble();
            id += TO_STAT_BLOCKS;
            if (id < TO_STAT_MAX + TO_STAT_BLOCKS)
                LastValues[id] = value;
        }
        if (!System)
        {
            toQuery queryio(conn, SQLSessionIO, args);
            int id = 0;
            while (!queryio.eof())
            {
                double value;
                value = queryio.readValue().toDouble();
                LastValues[id] = value;
                id++;
            }
        }
    }
    TOCATCH
}

void toResultStats::changeSession(toQuery &query)
{
    if (!handled())
        return ;

    if (System)
        throw tr("Can't change session on system statistics");
    try
    {
        toQList args;
        query.execute(SQLSession, args);
        SessionID = query.readValue().toInt();
        emit sessionChanged(SessionID);
        emit sessionChanged(QString::number(SessionID));
        resetStats();
    }
    TOCATCH
}

void toResultStats::changeSession(int ses)
{
    if (!handled())
        return ;

    if (System)
        throw tr("Can't change session on system statistics");
    if (SessionID != ses)
    {
        SessionID = ses;
        emit sessionChanged(SessionID);
        emit sessionChanged(QString::number(SessionID));
        resetStats();
    }
}

static toSQL SQLStatisticName("toResultStats:StatisticName",
                              "SELECT b.Name,a.Statistic#,a.Value\n"
                              "  FROM V$SesStat a,V$StatName b\n"
                              " WHERE a.SID = :f1<int> AND a.statistic# = b.statistic#\n",
                              "Get statistics and their names for session, must have same number of columns");
static toSQL SQLSystemStatisticName("toResultStats:SystemStatisticName",
                                    "SELECT Name,Statistic#,Value\n"
                                    "  FROM v$sysstat\n",
                                    "Get statistics and their names for system statistics, must have same number of columns");

void toResultStats::addValue(bool reset, int id, const QString &name, double value)
{
    QString delta;
    QString absVal;

    if (value != 0)
    {
        absVal.sprintf("%.15g", value);
        if (id < TO_STAT_MAX + TO_STAT_BLOCKS)
        {
            delta.sprintf("%.15g", value - LastValues[id]);
            if (value != LastValues[id] || !OnlyChanged)
            {
                toResultViewItem *item = new toResultViewItem(this, NULL);
                if (reset)
                    LastValues[id] = value;
                item->setText(0, name);
                if (OnlyChanged)
                    item->setText(1, delta);
                else
                {
                    item->setText(1, absVal);
                    item->setText(2, delta);
                }
                item->setText(3, QString::number(++Row));
            }
        }
    }
}

void toResultStats::refreshStats(bool reset)
{
    if (!handled() || Query || SessionIO)
        return ;

    try
    {
        clear();
        Row = 0;
        toConnection &conn = connection();
        toQList args;
        if (!System)
            args.insert(args.end(), SessionID);
        Query = new toNoBlockQuery(conn,
                                   toQuery::Background,
                                   toSQL::string(System ? SQLSystemStatisticName : SQLStatisticName,
                                                 connection()),
                                   args);
        if (!System)
            SessionIO = new toNoBlockQuery(conn, toQuery::Background,
                                           toSQL::string(SQLSessionIO, connection()),
                                           args);
        Poll.start(100);
        Reset = reset;

    }
    TOCATCH
}

void toResultStats::poll(void)
{
    try
    {
        if (!toCheckModal(this))
            return ;
        bool done = true;

        if (Query && Query->poll())
        {
            while (Query->poll() && !Query->eof())
            {
                QString name = Query->readValue();
                int id = Query->readValue().toInt();
                double value = Query->readValue().toDouble();
                addValue(Reset, id, name, value);
            }

            if (Query->eof())
            {
                delete Query;
                Query = NULL;
            }
            else
                done = false;
        }
        else if (Query)
            done = false;

        if (SessionIO && SessionIO->poll())
        {
            int id = 0;
            toQDescList description = SessionIO->describe();
            toQDescList::iterator i = description.begin();
            while (!SessionIO->eof())
            {
                addValue(Reset, id, (*i).Name, SessionIO->readValue().toDouble());
                id++;
                if (i == description.end())
                    i = description.begin();
                else
                    i++;
            }
            delete SessionIO;
            SessionIO = NULL;
        }
        else if (SessionIO)
            done = false;

        if (done)
            Poll.stop();
		resizeColumnsToContents();
    }
    catch (const QString &exc)
    {
        delete Query;
        Query = NULL;
        delete SessionIO;
        SessionIO = NULL;
        Poll.stop();
        toStatusMessage(exc);
    }
}
