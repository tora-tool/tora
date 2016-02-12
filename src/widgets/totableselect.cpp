
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

#include "widgets/totableselect.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconnection.h"
#include "core/toconnectiontraits.h"
#include "widgets/toresultschema.h"

#include <QLabel>
#include <QtCore/QTimer>
#include <QVBoxLayout>

void toTableSelect::setup()
{
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    setTitle(tr("Table selection"));
    bool mysql = false;
    try
    {
        mysql = toConnection::currentConnection(this).providerIs("QMYSQL");
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
    QLabel *label = new QLabel(mysql ? tr("Database") : tr("Schema"), this);
    label->show();
    vbox->addWidget(label);

    Schema = new toResultSchema(this);
    Schema->show();
    Schema->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    vbox->addWidget(Schema);

    label = new QLabel(tr("Table"), this);
    label->show();
    vbox->addWidget(label);

    Table = new toResultCombo(this);
    Table->show();
    Table->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    vbox->addWidget(Table);

    Schema->additionalItem(mysql ? tr("Select database") : tr("Select schema"));
    Schema->refresh();
    Table->additionalItem(tr("Select table"));
    // petr vanek 03/01/07 bug #1180847 Error when creating referential constraint
    Table->setSQL(toSQL::sql("toBrowser:ListTableNames"));

    Schema->refresh();
    if (!SelectedTable.isNull())
        setTable(SelectedTable);
    connect(Schema, SIGNAL(activated(int)), this, SLOT(changeSchema()));
    connect(Table, SIGNAL(activated(int)), this, SLOT(changeTable()));
}

toTableSelect::toTableSelect(QWidget *parent, const char *name)
    : QGroupBox(parent)
    , Schema(NULL)
    , Table(NULL)
{
    setObjectName(name);
    QTimer::singleShot(0, this, SLOT(setup()));
}

void toTableSelect::setTable(const QString &table)
{
    if (!Table || !Schema)
        SelectedTable = table;
    else
    {
        QStringList parts = table.split(".");
        toConnection &conn = toConnection::currentConnection(this);
        if (parts.size() > 1)
        {
            Schema->setSelected(conn.getTraits().unQuote(parts[0]));
            Table->setSelected(conn.getTraits().unQuote(parts[1]));
        }
        else
            Schema->setSelected(conn.getTraits().unQuote(table));
        Table->refreshWithParams(toQueryParams() << Schema->selected());
    }
}

void toTableSelect::changeSchema(void)
{
    if (Schema->currentIndex() != 0)
        Table->refreshWithParams(toQueryParams() << Schema->selected());
}

void toTableSelect::changeTable(void)
{
    if (Table->currentIndex() != 0)
    {
        toConnection &conn = toConnection::currentConnection(this);
        QString table = conn.getTraits().quote(Schema->selected());
        table += ".";
        table += conn.getTraits().quote(Table->selected());
        emit selectTable(table);
    }
}
