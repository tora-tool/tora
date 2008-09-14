/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconnection.h"
#include "toconf.h"
#include "toresultfield.h"
#include "toresultplan.h"
#include "toresultplan.h"
#include "toresultresources.h"
#include "toresultview.h"
#include "tosgastatement.h"
#include "tosgatrace.h"
#include "tosqlparse.h"
#include "totool.h"


void toSGAStatement::viewResources(void)
{
    try
    {
        Resources->changeParams(Address);
    }
    catch (...)
    {
        toStatusMessage(tr("Couldn't find SQL statement in SGA"), false, false);
    }
}

static toSQL SQLParsingSchema(
    "toSGAStatement:ParsingSchema",
    "SELECT username\n"
    "  FROM v$sql a,\n"
    "       all_users b\n"
    " WHERE b.user_id = a.parsing_schema_id\n"
    "   AND a.address || ':' || a.hash_value = :f1<char[101]> AND a.child_number = 0",
    "Get the schema that parsed a statement");

static toSQL SQLBackendSql(
    "toSGAStatement:BackendSql",
    "SELECT pg_stat_get_backend_activity ( :backend<int> )",
    "Get the backend's SQL statement.",
    "",
    "PostgreSQL");

toSGAStatement::toSGAStatement(QWidget *parent)
        : QTabWidget(parent)
{

    SQLText = new toResultField(this);
    addTab(SQLText, tr("SQL"));

    if (toIsOracle(toCurrentConnection(this)))
    {
        Plan = new toResultPlan(this);
        addTab(Plan, tr("Execution plan"));
        Resources = new toResultResources(this);
        addTab(Resources, tr("Information"));
    }
    else
        QTabWidget::tabBar()->hide();

    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(changeTab(int)));
    CurrentTab = SQLText;
}

void toSGAStatement::changeTab(int index)
{
    QWidget *widget = QTabWidget::widget(index);
    try
    {
        CurrentTab = widget;
        if (!Address.isEmpty())
        {
            if (CurrentTab == SQLText)
            {
                QString sql;
                toConnection &conn = toCurrentConnection(this);

                if (toIsOracle(conn))
                    sql = toSQLString(conn, Address);
                else if (toIsPostgreSQL(conn))
                {
                    toQList vals = toQuery::readQuery(conn, SQLBackendSql, Address);

                    for (toQList::iterator i = vals.begin(); i != vals.end(); i++)
                        sql.append(*i);
                }

                if (toConfigurationSingle::Instance().autoIndent())
                    sql = toSQLParse::indent(sql);
                SQLText->setText(sql);
            }
            else if (CurrentTab == Plan)
            {
                Plan->query(toSQLString(toCurrentConnection(this), Address),
                            toQuery::readQuery(toCurrentConnection(this),
                                               SQLParsingSchema, Address));
            }
            else if (CurrentTab == Resources)
                viewResources();
        }
    }
    TOCATCH;
}

void toSGAStatement::changeAddress(const QString &str)
{
    Address = str;
    changeTab(QTabWidget::indexOf(CurrentTab));
}
