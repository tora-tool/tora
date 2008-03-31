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
