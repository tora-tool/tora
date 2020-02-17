
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

#include "tools/toresultstats.h"

#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "core/tosql.h"

static toSQL SQLStatisticName("toResultStats:StatisticName",
                              "SELECT b.Name,a.Statistic#,a.Value\n"
                              "  FROM V$SesStat a,V$StatName b\n"
                              " WHERE a.SID = :f1<int> AND a.statistic# = b.statistic#\n",
                              "Get statistics and their names for session, must have same number of columns");
static toSQL SQLSystemStatisticName("toResultStats:SystemStatisticName",
                                    "SELECT Name,Statistic#,Value\n"
                                    "  FROM v$sysstat\n",
                                    "Get statistics and their names for system statistics, must have same number of columns");

static toSQL SQLSession("toResultStats:Session",
                        "SELECT MIN(SID) FROM V$MYSTAT",
                        "Get session id of current session");

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

toResultStats::toResultStats(bool onlyChanged
                             , int ses
                             , QWidget *parent
                             , const char *name)
    : toResultView(false, false, parent, name)
    , OnlyChanged(onlyChanged)
{
    if (!handled())
        return ;

    m_sessionID = ses;
    setSQLName(QString::fromLatin1("toResultStats"));
    System = false;
    setup();
}

toResultStats::toResultStats(bool onlyChanged
                             , QWidget *parent
                             , const char *name)
    : toResultView(false, false, parent, name)
    , m_sessionID(-2)
    , OnlyChanged(onlyChanged)
{
    if (!handled())
        return ;

    System = false;

    setup();
}

toResultStats::toResultStats(QWidget *parent
                             , const char *name)
    : toResultView(false, false, parent, name)
    , OnlyChanged(false)
{
    if (!handled())
        return ;

    System = true;
    setup();
}

toResultStats::~toResultStats()
{
    close();
}

bool toResultStats::canHandle(const toConnection &conn)
{
    return conn.providerIs("Oracle");
}

bool toResultStats::close()
{
    delete Query;
    delete SessionIO;
    Query = SessionIO = NULL;

    return true;
}

void toResultStats::resetStats(void)
{
    if (!handled())
        return ;

    Utils::toBusy busy;
    try
    {
        toConnectionSubLoan conn(connection());
        toQueryParams args;
        if (!System)
            args << sid();
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

void toResultStats::slotChangeSession(int ses)
{
    if (!handled())
        return ;

    if (System)
        throw tr("Can't change session on system statistics");
    if (m_sessionID != ses)
    {
        m_sessionID = ses;
        emit sessionChanged(sid());
        emit sessionChanged(QString::number(sid()));
        resetStats();
    }
}

void toResultStats::slotRefreshStats(bool reset)
{
    if (!handled() || Query || SessionIO)
        return ;

    try
    {
        clear();
        Row = 0;
        toConnection &conn = connection();
        toQueryParams args;
        if (!System)
            args << sid();
        Query = new toEventQuery(this
                                 , conn
                                 , toSQL::string(System ? SQLSystemStatisticName : SQLStatisticName, connection())
                                 , args
                                 , toEventQuery::READ_ALL
                                );
        auto c1 = connect(Query, &toEventQuery::dataAvailable, this, &toResultStats::receiveStatisticNamesData);
        auto c2 = connect(Query, &toEventQuery::done, this, &toResultStats::queryStatisticNamesDone);
        Query->start();

        if (!System)
        {
            SessionIO = new toEventQuery(this
                                         , conn
                                         , toSQL::string(SQLSessionIO, connection())
                                         , args
                                         , toEventQuery::READ_ALL
                                        );
            auto c1 = connect(Query, &toEventQuery::dataAvailable, this, &toResultStats::receiveStatisticValuesData);
            auto c2 = connect(Query, &toEventQuery::done, this, &toResultStats::queryStatisticsValuesDone);
            SessionIO->start();
        }

        Reset = reset;

    }
    TOCATCH
}

void toResultStats::receiveStatisticNamesData(toEventQuery*)
{
    if (!Utils::toCheckModal(this))
        return;

    try
    {

        while (Query->hasMore())
        {
            QString name = (QString)Query->readValue();
            int id = Query->readValue().toInt();
            double value = Query->readValue().toDouble();
            addValue(Reset, id, name, value);
        }
    }
    catch (const QString &exc)
    {
        delete Query;
        delete SessionIO;
        Query = SessionIO = NULL;
        Utils::toStatusMessage(exc);
    }
} // pollQuery

void toResultStats::queryStatisticNamesDone(toEventQuery*, unsigned long)
{
    delete Query;
    Query = NULL;
    resizeColumnsToContents();
} // queryDone

void toResultStats::receiveStatisticValuesData(toEventQuery*)
{
    if (!Utils::toCheckModal(this))
        return;

    try
    {
        int id = 0;
        toQColumnDescriptionList description = SessionIO->describe();
        toQColumnDescriptionList::iterator i = description.begin();
        while (SessionIO->hasMore())
        {
            addValue(Reset, id, (*i).Name, SessionIO->readValue().toDouble());
            id++;
            if (i == description.end())
                i = description.begin();
            else
                i++;
        }
    }
    catch (const QString &exc)
    {
        delete SessionIO;
        SessionIO = NULL;
        Utils::toStatusMessage(exc);
    }
} // pollSystem

void toResultStats::queryStatisticsValuesDone(toEventQuery*, unsigned long)
{
    delete SessionIO;
    SessionIO = NULL;
    resizeColumnsToContents();
} // systemDone

void toResultStats::setup(void)
{
    for (int i = 0; i < TO_STAT_MAX; i++)
        LastValues[i] = 0;

    addColumn(tr("Name"));
    if (!OnlyChanged)
        addColumn(tr("Value"));
    addColumn(tr("Delta"));
    setSorting(0);

    setColumnAlignment(1, Qt::AlignRight);
    setColumnAlignment(2, Qt::AlignRight);

    Query = SessionIO = NULL;
    connect(this,
            SIGNAL(sessionChanged(int)),
            this,
            SLOT(slotRefreshStats()),
            Qt::QueuedConnection);
}

int toResultStats::sid()
{
    // TODO: this is non-sense here we store SID of randomly borrowed session
    if (m_sessionID == -2)
        try
        {
            toConnectionSubLoan conn(connection());
            toQuery query(conn, SQLSession, toQueryParams());
            m_sessionID = query.readValue().toInt();
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
            m_sessionID = -1;
        }

    return m_sessionID;
}

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
