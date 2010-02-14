
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
#include "tonoblockquery.h"
#include "toresultstats.h"
#include "totool.h"

#include <qapplication.h>

#include <signal.h>
#include <sys/types.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#endif
#include <stdio.h>

#define PREFETCH_SIZE 5000

void toNoBlockQuery::queryTask::run(void)
{
    int Length = 0;
    try
    {
        if (Parent.Query)
            Parent.Query->execute(Parent.SQL, Parent.Param);

        bool eof = true;
        {
            toLocker lock (Parent.Lock)
            ;
            if (Parent.Query)
            {
                Parent.Description = Parent.Query->describe();
                Length = Parent.Query->columns();
                eof = Parent.Query->eof();
            }
        }

        if (!eof)
        {
            bool signaled = false;
            for (;;)
            {
                for (int i = 0;i < Length && Parent.Query;i++)
                {
                    toQValue value(Parent.Query->readValueNull());
                    {
                        toLocker lock (Parent.Lock)
                        ;
                        Parent.ReadingValues.insert(Parent.ReadingValues.end(), value);
                        if (Parent.ReadingValues.size() == 1)
                        {
                            if (signaled)
                            {
                                Parent.Lock.unlock();
                                Parent.Continue.down();
                                Parent.Lock.lock();
                            }
                            Parent.Running.up();
                            signaled = true;
                        }
                    }
                }
                toLocker lock (Parent.Lock)
                ;
                if (!Parent.Query || Parent.Query->eof())
                    break;
                else
                {
                    if (Parent.ReadingValues.size() > PREFETCH_SIZE)
                    {
                        Parent.Lock.unlock();
                        Parent.Continue.down();
                        signaled = false;
                        Parent.Lock.lock();
                    }
                }
                if (Parent.Quit)
                    break;
            }
        }
        if (Parent.Query)
            Parent.Processed = Parent.Query->rowsProcessed();
        else
            Parent.Processed = 0;
    }
    catch (const toConnection::exception &str)
    {
        toLocker lock (Parent.Lock)
        ;
        Parent.Error = str;
    }
    catch (const QString &str)
    {
        toLocker lock (Parent.Lock)
        ;
        Parent.Error = str;
    }
    catch (...)
    {
        toLocker lock (Parent.Lock)
        ;
        Parent.Error = qApp->translate("toNoBlockQuery", "Unknown exception");
    }

    toLocker lock (Parent.Lock)
    ;
    if (!Parent.Error.isNull() && !Parent.Query->eof())
        try
        {
            Parent.Query->cancel();
        }
        catch (...)
            {}
    delete Parent.Query;
    Parent.Query = NULL;
    Parent.Running.up();
    Parent.EOQ = true;
}

toQValue toNoBlockQuery::readValueNull()
{
    if (CurrentValue == Values.end())
    {
        Lock.lock();
        bool eoq = EOQ;
        Lock.unlock();
        if (!eoq)
            Running.down();
        toLocker lock (Lock)
        ;
        Values = ReadingValues;
        CurrentValue = Values.begin();
        ReadingValues.clear();
        Continue.up();
        if (CurrentValue == Values.end())
            throw qApp->translate("toNoBlockQuery", "Reading past end of query");
    }
    checkError();
    toQValue ret = *CurrentValue;
    CurrentValue++;
    return ret;
}

toQValue toNoBlockQuery::readValue()
{
    return toNull(readValueNull());
}

toNoBlockQuery::toNoBlockQuery(toConnection &conn, const QString &sql,
                               const toQList &param, toResultStats *stats)
        : SQL(sql),
        Error(QString::null),
        Param(param),
        Statistics(stats)
{
    CurrentValue = Values.end();
    Quit = EOQ = false;
    Processed = 0;

    try
    {
        Query = NULL;
        Query = new toQuery(conn, toQuery::Long);
    }
    catch (...)
    {
        delete Query;
        Query = NULL;
        Error = qApp->translate("toNoBlockQuery", "Couldn't open query");
        throw;
    }

    try
    {
        if (Statistics)
            Statistics->changeSession(*Query);
    }
    catch (...)
    {
        Statistics = NULL;
    }

    int val = toConfigurationSingle::Instance().autoLong();
    if (val != 0)
        Started = time(NULL) + val;
    else
        Started = 0;

    toLocker lock (Lock)
    ;
    try
    {
        toThread *thread = new toThread(new queryTask(*this));
        thread->start();
    }
    catch (...)
    {
        Error = qApp->translate("toNoBlockQuery", "Failed to start background query thread");
        while (Running.getValue())
            Running.down();
        Quit = EOQ = true;
        delete Query;
        Query = NULL;
    }
}

toNoBlockQuery::toNoBlockQuery(toConnection &conn, toQuery::queryMode mode,
                               const QString &sql, const toQList &param, toResultStats *stats)
        : SQL(sql),
        Error(QString::null),
        Param(param),
        Statistics(stats)
{
    CurrentValue = Values.end();
    Quit = EOQ = false;
    Processed = 0;

    try
    {
        Query = NULL;
        Query = new toQuery(conn, mode);
    }
    catch (...)
    {
        delete Query;
        Query = NULL;
        Error = qApp->translate("toNoBlockQuery", "Couldn't open query");
        throw;
    }

    try
    {
        if (Statistics)
            Statistics->changeSession(*Query);
    }
    catch (...)
    {
        Statistics = NULL;
    }

    int val = toConfigurationSingle::Instance().autoLong();
    if (val != 0)
        Started = time(NULL) + val;
    else
        Started = 0;

    toLocker lock (Lock)
    ;
    try
    {
        toThread *thread = new toThread(new queryTask(*this));
        thread->start();
    }
    catch (...)
    {
        Error = qApp->translate("toNoBlockQuery", "Failed to start background query thread");
        while (Running.getValue())
            Running.down();
        Quit = EOQ = true;
        delete Query;
        Query = NULL;
    }
}

toQDescList const &toNoBlockQuery::describe(void) const
{
    toLocker lock (Lock)
    ;
    checkError();
    return Description;
}

bool toNoBlockQuery::eof(void)
{
    toLocker lock (Lock)
    ;
    checkError();
    return EOQ && CurrentValue == Values.end() && !ReadingValues.size();
}

int toNoBlockQuery::rowsProcessed(void)
{
    toLocker lock (Lock)
    ;
    checkError();
    return Processed;
}

void toNoBlockQuery::stop(void)
{
    Lock.lock();
    int sleep = 100;
    while (!EOQ)
    {
        Quit = true;
        ReadingValues.clear();
        Continue.up();
        Lock.unlock();

        toThread::msleep(sleep);

        Lock.lock();
        if (Running.getValue() == 0)
        {
            if (Query)
            {
                Query->cancel();
                sleep += 100;
            }
        }
    }
    Lock.unlock();

    if (Statistics)
    {
        Statistics->refreshStats(false);
    }
    while (Query)
    {
        fprintf(stderr, "Internal error, query not deleted after stopping it.\n");
        toThread::msleep(100);
    }
}

toNoBlockQuery::~toNoBlockQuery()
{
    try
    {
        stop();
    }
    TOCATCH
}

bool toNoBlockQuery::poll(void)
{
    unsigned int count = 0;
    for (toQList::iterator i = CurrentValue;i != Values.end();i++)
        count++;
    if (Running.getValue() > 0)
    {
        toLocker lock (Lock)
        ;
        count += ReadingValues.size();
    }

    if ((count >= Description.size() && Description.size() > 0) || eof())
        return true;

    Lock.lock();
    if (Started > 0 && Started < time(NULL) && !Description.size())
    {
        if (Query && (Query->mode() == toQuery::Normal || Query->mode() == toQuery::Background))
        {
            try
            {
                toStatusMessage(qApp->translate("toNoBlockQuery", "Restarting query in own connection"), false, false);
                toConnection &conn = Query->connection();
                Lock.unlock();
                stop();
                toLocker lock (Lock)
                ;
                while (Running.getValue() > 0)
                    Running.down();
                while (Continue.getValue() > 0)
                    Continue.down();
                Error = QString();
                ReadingValues.clear();
                Values.clear();
                CurrentValue = Values.end();
                Quit = EOQ = false;
                Processed = 0;

                Query = new toQuery(conn, toQuery::Long);

                try
                {
                    if (Statistics)
                        Statistics->changeSession(*Query);
                }
                catch (...)
                {
                    Statistics = NULL;
                }

                toThread *thread = new toThread(new queryTask(*this));
                thread->start();
            }
            catch (...)
            {
                Error = qApp->translate("toNoBlockQuery", "Failed to start background query thread");
                while (Running.getValue())
                    Running.down();
                Quit = EOQ = true;
                delete Query;
                Query = NULL;
            }
        }
        else
            Lock.unlock();
    }
    else
        Lock.unlock();

    return false;
}

void toNoBlockQuery::checkError() const
{
    if (!Error.isNull())
    {
        throw Error;
    }
}
