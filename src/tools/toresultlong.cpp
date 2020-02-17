
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

#include "tools/toresultlong.h"

#include "core/utils.h"
#include "core/toeventquery.h"
#include "core/toconfiguration.h"
#include "core/todatabaseconfig.h"

toResultLong::toResultLong(bool readable, bool dispCol,
                           QWidget *parent, const char *name, toWFlags f)
    : toResultView(readable, dispCol, parent, name, f)
    , First(false)
    , MaxNumber(-1)
{
    Query      = NULL;
    Statistics = NULL;
    HasHeaders = false;
}

toResultLong::toResultLong(QWidget *parent, const char *name, toWFlags f)
    : toResultView(parent, name, f)
    , First(false)
    , MaxNumber(-1)
{
    Query      = NULL;
    Statistics = NULL;
    HasHeaders = false;
}

void toResultLong::query(const QString &sql, const toQueryParams &param)
{
    setSqlAndParams(sql, param);
    slotStop();
    Query = NULL;
    LastItem = NULL;
    RowNumber = 0;
    First = true;

    clear();

    setSorting( -1);

    if (NumberColumn)
    {
        addColumn(QString::fromLatin1("#"));
        setColumnAlignment(0, Qt::AlignRight);
    }

    if (Filter)
        Filter->startingQuery();

    try
    {
        Query = new toEventQuery(this
                                 , connection()
                                 , sql
                                 , param
                                 //, Statistics
                                 , toEventQuery::READ_ALL
                                );
        connect(Query, &toEventQuery::dataAvailable, this, &toResultLong::receiveData);
        connect(Query, SIGNAL(done(toEventQuery*, unsigned long)), this, SLOT(slotQueryDone()));
        if (ReadAll)
        {
            MaxNumber = -1;
            //// Query->slotReadAll(); // indicate that all records should be fetched
        }
        else
            MaxNumber = toConfigurationNewSingle::Instance().option(ToConfiguration::Database::InitialFetchInt).toInt();
        Query->start();
    }
    catch (const toConnection::exception &str)
    {
        First = false;
        emit firstResult(toResult::sql(), str, true);
        emit done();
        //// if (Mode != toQuery::Long)
        Utils::toStatusMessage(str);
    }
    catch (const QString &str)
    {
        First = false;
        emit firstResult(toResult::sql(), str, true);
        emit done();
        //// if (Mode != toQuery::Long)
        Utils::toStatusMessage(str);
    }
    updateContents();
}

#define TO_POLL_CHECK 100

void toResultLong::editReadAll(void)
{
    if (Query && Query->hasMore())
    {
        MaxNumber = -1;
        Query->setFetchMode(toEventQuery::READ_ALL);
    }
}

void toResultLong::receiveData(toEventQuery*)
{
    if (!Utils::toCheckModal(this))
        return;

    try
    {
        while (Query->hasMore())
        {
            bool em = false; // should we emit signal about first row fetched?
            QString buffer;
            if (First)
            {
                QString tmp = sql().simplified().mid(0, 10).toLower();
                if (tmp.startsWith(QString::fromLatin1("update")) ||
                        tmp.startsWith(QString::fromLatin1("delete")) ||
                        tmp.startsWith(QString::fromLatin1("insert")))
                    buffer = tr("%1 rows processed").arg(Query->rowsProcessed());
                else if (tmp.startsWith(QString::fromLatin1("select")))
                    buffer = tr("Query executed");
                else
                    buffer = tr("Statement executed");
                em = true;
            }
            if (!HasHeaders)
            {
                Description = Query->describe();
                bool hidden = false;

                for (toQColumnDescriptionList::iterator i = Description.begin(); i != Description.end(); i++)
                {
                    QString name = (*i).Name;
                    if (ReadableColumns)
                        Utils::toReadableColumn(name);
                    if (name.length() > 0 && name[0].toLatin1() != ' ')
                    {
                        if (hidden)
                            throw tr("Can only hide last column in query");
                        if (name[0].toLatin1() == '-')
                        {
                            addColumn(Utils::toTranslateMayby(sqlName(), name.right(name.length() - 1)));
                            setColumnAlignment(columns() - 1, Qt::AlignRight);
                        }
                        else
                        {
                            addColumn(Utils::toTranslateMayby(sqlName(), name));
                            if ((*i).AlignRight)
                                setColumnAlignment(columns() - 1, Qt::AlignRight);
                        }
                    }
                    else
                        hidden = true;
                }
                HasHeaders = true;

                setResizeMode(toTreeWidget::AllColumns);

                if (sortColumn() < 0)
                {
                    if (NumberColumn)
                        setSorting(0);
                    else
                        setSorting(Description.size());
                }
            }

            if (Query->hasMore())
            {
                int disp = 0;
                unsigned int cols = Description.size();
                if (NumberColumn)
                {
                    disp = 1;
                }
                do
                {
                    toTreeWidgetItem *last = LastItem;
                    LastItem = createItem(LastItem, QString::null);
                    if (NumberColumn)
                        LastItem->setText(0, QString::number(RowNumber + 1));
                    else
                        LastItem->setText(cols, QString::number(RowNumber + 1));
                    toResultViewItem *ri = dynamic_cast<toResultViewItem *>(LastItem);
                    toResultViewCheck *ci = dynamic_cast<toResultViewCheck *>(LastItem);
                    for (unsigned int j = 0; (j < cols || j == 0) && !Query->eof(); j++)
                    {
                        if (ri)
                            ri->setText(j + disp, (QString)Query->readValue());
                        else if (ci)
                            ci->setText(j + disp, (QString)Query->readValue());
                        else
                            LastItem->setText(j + disp, (QString)Query->readValue());
                    }
                    if (Filter && !Filter->check(LastItem))
                    {
                        delete LastItem;
                        LastItem = last;
                    }
                    else
                        RowNumber++;
                }
                while (Query->hasMore() && (MaxNumber < 0 || MaxNumber > RowNumber));
            }
            if (em)
            {
                First = false;
                emit firstResult(sql(), toConnection::exception(buffer), false);
            }
            if (!(MaxNumber < 0 || MaxNumber > RowNumber))
                Query->stop();
        }
    }
    catch (const toConnection::exception &str)
    {
        if (First)
        {
            First = false;
            emit firstResult(sql(), str, true);
            //// if (Mode != toQuery::Long)
            Utils::toStatusMessage(str);
        }
        else
            Utils::toStatusMessage(str);
        cleanup();
    }
    catch (const QString &str)
    {
        if (First)
        {
            First = false;
            emit firstResult(sql(), str, true);
            //// if (Mode != toQuery::Long)
            Utils::toStatusMessage(str);
        }
        else
            Utils::toStatusMessage(str);
        cleanup();
    }
}

void toResultLong::cleanup(void)
{
    if (Query)
    {
        // delete causes crash on short running queries.
        delete Query;
        Query = NULL;
        emit done();
    }
}

void toResultLong::slotQueryDone(void)
{
    cleanup();
}

bool toResultLong::eof(void)
{
    return !Query || !Query->hasMore();
}

void toResultLong::slotStop(void)
{
    if (Query)
    {
        delete Query;
        Query = NULL;
        emit done();
    }
}

toResultLong::~toResultLong()
{
    if (Query)
    {
        delete Query;
        Query = NULL;
    }
}

