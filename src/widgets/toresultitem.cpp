
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

#include "widgets/toresultitem.h"
#include "core/utils.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "widgets/toresultresources.h"
#include "core/tosql.h"
#include "core/totool.h"

#include <QLabel>
#include <QLayout>
#include <QToolTip>
#include <QApplication>
#include <QtGui/QResizeEvent>

static toSQL SQLResource(
    "toResultResources:Information",
    "SELECT 'Total' AS \"-\", "
    "       'per Execution' AS \"-\", "
    "       'per Row processed' AS \"-\", "
    "       Sorts, "
    "       DECODE ( Executions, "
    "                0, "
    "                'N/A', "
    "                ROUND ( Sorts / Executions, "
    "                        3 ) ) AS \" \", "
    "       DECODE ( Rows_Processed, "
    "                0, "
    "                'N/A', "
    "                ROUND ( Sorts / Rows_Processed, "
    "                        3 ) ) AS \" \", "
    "       Parse_Calls AS \"Parse\", "
    "       DECODE ( Executions, "
    "                0, "
    "                'N/A', "
    "                ROUND ( Parse_Calls / Executions, "
    "                        3 ) ) AS \" \", "
    "       DECODE ( Rows_Processed, "
    "                0, "
    "                'N/A', "
    "                ROUND ( Parse_Calls / Rows_Processed, "
    "                        3 ) ) AS \" \", "
    "       Disk_Reads, "
    "       DECODE ( Executions, "
    "                0, "
    "                'N/A', "
    "                ROUND ( Disk_Reads / Executions, "
    "                        3 ) ) AS \" \", "
    "       DECODE ( Rows_Processed, "
    "                0, "
    "                'N/A', "
    "                ROUND ( Disk_Reads / Rows_Processed, "
    "                        3 ) ) AS \" \", "
    "       Buffer_Gets, "
    "       DECODE ( Executions, "
    "                0, "
    "                'N/A', "
    "                ROUND ( Buffer_Gets / Executions, "
    "                        3 ) ) AS \" \", "
    "       DECODE ( Rows_Processed, "
    "                0, "
    "                'N/A', "
    "                ROUND ( Buffer_Gets / Rows_Processed, "
    "                        3 ) ) AS \" \", "
    "       Rows_Processed, "
    "       DECODE ( Executions, "
    "                0, "
    "                'N/A', "
    "                ROUND ( Rows_Processed / Executions, "
    "                        3 ) ) AS \" \", "
    "       ' ' AS \"-\", "
    "       Executions, "
    "       ' ' AS \"-\", "
    "       ' ' AS \"-\", "
    "       ' ' AS \"-\", "
    "       ' ' AS \"-\", "
    "       ' ' AS \"-\", "
    "       Loads, "
    "       First_Load_Time, "
    "       Parsing_User_Id, "
    "       Parsing_Schema_Id, "
    "       Users_Executing, "
    "       Users_Opening, "
    "       Open_Versions, "
    "       Sharable_Mem, "
    "       Kept_Versions, "
    "       Persistent_Mem, "
    "       Optimizer_Mode, "
    "       Loaded_Versions, "
    "       Runtime_Mem, "
    "       Serializable_Aborts, "
    "       Invalidations "
    "  FROM v$sqlarea "
    " WHERE Address || ':' || Hash_Value = :f1<char[100]> ",
    "Display information about an SQL statement");

toResultResources::toResultResources(QWidget *parent, const char *name)
    : toResultItem(3, true, parent, name)
{
    setSQL(SQLResource);
}

toResultItem::toResultItem(int num,
                           bool readable,
                           QWidget *parent,
                           const char *name)
    : QScrollArea(parent), DataFont(QFont())
{
    setObjectName(name);
    setup(num, readable);
}

toResultItem::toResultItem(int num, QWidget *parent, const char *name)
    : QScrollArea(parent)
    , DataFont(QFont())
{
    setObjectName(name);
    setup(num, false);
}

toResultItem::toResultItem(QWidget *parent, const char *name, int num)
    : QScrollArea(parent)
    , DataFont(QFont())
{
    setObjectName(name);
    setup(num, false);
}

toResultItem::~toResultItem()
{
    delete Query;
}

void toResultItem::showTitle(bool val)
{
    ShowTitle = val;
}

void toResultItem::alignRight(bool val)
{
    Right = val;
}

void toResultItem::dataFont(const QFont &val)
{
    DataFont = val;
}

// Must be alloced in multiples of 2
#define ALLOC_SIZE 1000

QWidget *toResultItem::createTitle(QWidget *parent)
{
    QLabel *widget = new QLabel(parent);
    widget->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    widget->setWordWrap(true);
    return widget;
}

QWidget *toResultItem::createValue(QWidget *parent)
{
    QLabel *widget = new QLabel(parent);
    return widget;
}

void toResultItem::setTitle(QWidget *widget,
                            const QString &title,
                            const QString &)
{
    QLabel *label = dynamic_cast<QLabel *>(widget);
    if (label)
        label->setText(title);

    Result->addWidget(widget,
                      (int) WidgetPos / (Columns * 2), // row
                      WidgetPos % (Columns * 2));      // column
}

void toResultItem::setValue(QWidget *widget,
                            const QString &title,
                            const QString &value)
{
    QLabel *label = dynamic_cast<QLabel *>(widget);
    if (label)
    {
        if (title != "-")
        {
            label->setFrameStyle(StyledPanel | Sunken);
            label->setFont(DataFont);
        }
        else
        {
            label->setFrameStyle(NoFrame);
            label->setFont(qApp->font());
        }

        label->setText(value);

        if (Right)
        {
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            label->setWordWrap(true);
        }
        else
        {
            label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            label->setWordWrap(true);
        }
    }

    Result->addWidget(widget,
                      (int) WidgetPos / (Columns * 2), // row
                      WidgetPos % (Columns * 2));      // column
}


void toResultItem::query(const QString &sql, toQueryParams const& param)
{
    if (!setSqlAndParams(sql, param))
        return ;

    start();
    if (!handled() || Query)
    {
        if (!Query)
            done();
        return ;
    }

    try
    {
        if (Query)
        {
            delete Query;
            Query = NULL;
        }
        Query = new toEventQuery(this
                                 , connection()
                                 , sql
                                 , param
                                 , toEventQuery::READ_ALL);
        connect(Query, SIGNAL(dataAvailable(toEventQuery*)), this, SLOT(slotPoll()));
        connect(Query, SIGNAL(done(toEventQuery*)), this, SLOT(slotQueryDone()));
        Query->start();
    }
    catch (const QString &str)
    {
        done();
        Utils::toStatusMessage(str);
    }
}

void toResultItem::clearData()
{
    for (int i = 0; i < NumWidgets; i++)
    {
        if (Widgets[i])
            Widgets[i]->hide();
    }
}

bool toResultItem::canHandle(const toConnection &)
{
    return true;
}

void toResultItem::start(void)
{
    WidgetPos = 0;
}

void toResultItem::addItem(const QString &title, const QString &value)
{
    if (WidgetPos >= NumWidgets)
    {
        NumWidgets += ALLOC_SIZE;
        Widgets.resize(NumWidgets, 0);
    }
    QString t;
    if (title != "-")
        t = Utils::toTranslateMayby(sqlName(), title);
    QWidget *widget;
    if (!Widgets[WidgetPos])
    {
        widget = createTitle(this);
        Widgets[WidgetPos] = widget;
    }
    else
        widget = ((QLabel *)Widgets[WidgetPos]);

    setTitle(widget, t, value);
    if (ShowTitle)
        widget->show();
    else
        widget->hide();

    WidgetPos++;
    if (!Widgets[WidgetPos])
    {
        widget = createValue(this);
        Widgets[WidgetPos] = widget;
    }
    else
        widget = Widgets[WidgetPos];
    setValue(widget, title, value);

    widget->show();
    WidgetPos++;
}

void toResultItem::done(void)
{
    for (int i = WidgetPos; i < NumWidgets; i++)
    {
        if (Widgets[i])
            Widgets[i]->hide();
    }

    int rows = WidgetPos / (Columns * 2);

    // by default, no rows my stretch except when none can. then they
    // will all resize regardless of what flags you pass to the
    // layout.

    // add a label past the data to stretch.
    Result->addWidget(new QLabel(this), rows + 1, 0);
    Result->setRowStretch(rows + 1, 1);

    Result->layout();
}

void toResultItem::slotPoll(void)
{
    if (!Utils::toCheckModal(this))
        return;

    try
    {
        toQColumnDescriptionList desc = Query->describe();

        if (Query->hasMore())
        {
            for (toQColumnDescriptionList::iterator i = desc.begin();
                    i != desc.end();
                    i++)
            {
                QString name = (*i).Name;
                if (ReadableColumns)
                    Utils::toReadableColumn(name);

                addItem(name, (QString)Query->readValue());
            }
        }
        // Should we try to fetch any remaining values here,
        // because we've only red one row...
    }
    catch (const QString &str)
    {
        delete Query;
        Query = NULL;
        done();
        Utils::toStatusMessage(str);
    }
}

void toResultItem::slotQueryDone(void )
{
    done();
    delete Query;
    Query = NULL;
}

void toResultItem::setup(int num, bool readable)
{
    ReadableColumns = readable;
    NumWidgets      = 0;
    WidgetPos       = 0;
    Columns         = num;
    ShowTitle       = true;
    Right           = true;
    Query           = NULL;
    DataFont.setBold(true);

    // qscrollarea needs a central widget for it's resizing policies.

    QWidget *w = new QWidget(this);
    setWidget(w);
    setWidgetResizable(true);

    Result = new QGridLayout;
    Result->setSpacing(3);
    w->setLayout(Result);
}
