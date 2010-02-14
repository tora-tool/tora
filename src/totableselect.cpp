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

#include "toconnection.h"
#include "toresultcombo.h"
#include "totableselect.h"

#include <qlabel.h>
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
        mysql = toIsMySQL(toCurrentConnection(this));
    }
    catch (...)
        {}
    QLabel *label = new QLabel(mysql ? tr("Database") : tr("Schema"), this);
    label->show();
    vbox->addWidget(label);

    Schema = new toResultCombo(this);
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
    Schema->query(toSQL::sql(toSQL::TOSQL_USERLIST));
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
{
    setObjectName(name);
    Schema = Table = NULL;
    QTimer::singleShot(1, this, SLOT(setup()));
}

void toTableSelect::setTable(const QString &table)
{
    if (!Table || !Schema)
        SelectedTable = table;
    else
    {
        QStringList parts = table.split(".");
        toConnection &conn = toCurrentConnection(this);
        if (parts.size() > 1)
        {
            Schema->setSelected(conn.unQuote(parts[0]));
            Table->setSelected(conn.unQuote(parts[1]));
        }
        else
            Schema->setSelected(conn.unQuote(table));
        Table->changeParams(Schema->selected());
    }
}

void toTableSelect::changeSchema(void)
{
    if (Schema->currentIndex() != 0)
        Table->changeParams(Schema->selected());
}

void toTableSelect::changeTable(void)
{
    if (Table->currentIndex() != 0)
    {
        toConnection &conn = toCurrentConnection(this);
        QString table = conn.quote(Schema->selected());
        table += ".";
        table += conn.quote(Table->selected());
        emit selectTable(table);
    }
}
