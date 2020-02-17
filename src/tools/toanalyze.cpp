
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

#include "tools/toanalyze.h"
#include "core/utils.h"
#include "core/totool.h"
#include "editor/tomemoeditor.h"
#include "widgets/toresultschema.h"
#include "tools/toworksheetstatistic.h"
//#include "core/toconfiguration.h"
#include <QComboBox>
#include <QSpinBox>
#include <QMenu>
#include <QToolBar>
#include <QVBoxLayout>
#include <QPlainTextEdit>

#include "icons/execute.xpm"
#include "icons/refresh.xpm"
#include "icons/sql.xpm"
#include "icons/stop.xpm"
#include "icons/toanalyze.xpm"

#include "connection/tooracleconfiguration.h"
#include "toresultplan.h"
#include "toresulttableview.h"

class toAnalyzeTool : public toTool
{
        const char **pictureXPM(void) override
        {
            return const_cast<const char**>(toanalyze_xpm);
        }
    public:
        toAnalyzeTool()
            : toTool(320, "Statistics Manager") { }

        void closeWindow(toConnection &connection) override {};

        const char *menuItem() override
        {
            return "Statistics Manager";
        }
        toToolWidget* toolWindow(QWidget *parent, toConnection &connection) override
        {
            return new toAnalyze(parent, connection);
        }
        bool canHandle(const toConnection &conn) override
        {
            return conn.providerIs("Oracle") || conn.providerIs("QMYSQL") || conn.providerIs("QPSQL");
        }
};


static toAnalyzeTool AnalyzeTool;

static toSQL SQLListTablesMySQL("toAnalyze:ListTables",
                                "show table status",
                                "Get table statistics, first three columns and binds must be same",
                                "0401",
                                "QMYSQL");
static toSQL SQLListTables("toAnalyze:ListTables",
                           "select 'TABLE' \"Type\",\n"
                           "       owner,\n"
                           "       table_name,\n"
                           "       num_rows,\n"
                           "       blocks,\n"
                           "       empty_blocks,\n"
                           "       avg_space \"Free space/block\",\n"
                           "       chain_cnt \"Chained rows\",\n"
                           "       avg_row_len \"Average row length\",\n"
                           "       sample_size,\n"
                           "       last_analyzed\n"
                           "  from sys.all_all_tables\n"
                           " where iot_name is null\n"
                           "   and temporary != 'Y' and secondary = 'N'",
                           "",
                           "0800");
static toSQL SQLListTables7("toAnalyze:ListTables",
                            "select 'TABLE' \"Type\",\n"
                            "       owner,\n"
                            "       table_name,\n"
                            "       num_rows,\n"
                            "       blocks,\n"
                            "       empty_blocks,\n"
                            "       avg_space \"Free space/block\",\n"
                            "       chain_cnt \"Chained rows\",\n"
                            "       avg_row_len \"Average row length\",\n"
                            "       sample_size,\n"
                            "       last_analyzed\n"
                            "  from sys.all_tables\n"
                            " where temporary != 'Y' and secondary = 'N'",
                            "",
                            "0703");

static toSQL SQLListTablesPg(
    "toAnalyze:ListTables",
    "SELECT 'TABLE' AS \"Type\",\n"
    "       st.schemaname,\n"
    "       st.relname AS \"table\",\n"
    "       c.relfrozenxid AS \"Frozen XID\",\n"
    "       c.relpages AS \"8KB Pages\",\n"
    "       c.reltuples AS \"Tuples\",\n"
    "       st.last_analyze AS \"Last Analyze\",\n"
    "       st.last_autoanalyze AS \"Last Auto Analyze\",\n"
    "       st.last_vacuum AS \"Last Vacuum\",\n"
    "       st.last_autovacuum AS \"Last Auto Vacuum\"\n"
    "  FROM pg_stat_all_tables st,\n"
    "       pg_class c\n"
    " WHERE st.relid = c.OID\n"
    " ORDER BY 1,\n"
    "          2",
    "",
    "0702",
    "QPSQL");

static toSQL SQLListIndex("toAnalyze:ListIndex",
                          "SELECT 'INDEX' \"Type\",\n"
                          "       Owner,\n"
                          "       Index_Name,\n"
                          "       Num_rows,\n"
                          "       Distinct_Keys,\n"
                          "       Leaf_Blocks,\n"
                          "       Avg_Leaf_Blocks_Per_Key,\n"
                          "       Avg_Data_Blocks_Per_Key,\n"
                          "       Clustering_Factor,\n"
                          "       Sample_Size,\n"
                          "       Last_Analyzed\n"
                          "  FROM SYS.ALL_INDEXES\n"
                          " WHERE 1 = 1",
                          "List the available indexes, first three column and binds must be same");

static toSQL SQLListIndexPg(
    "toAnalyze:ListIndex",
    "SELECT 'INDEX' AS \"Type\",\n"
    "       st.schemaname,\n"
    "       st.relname AS \"table\",\n"
    "       st.indexrelname,\n"
    "       c.relpages AS \"8KB Pages\",\n"
    "       c.reltuples AS \"Tuples\",\n"
    "       st.idx_scan,\n"
    "       st.idx_tup_read,\n"
    "       st.idx_tup_fetch\n"
    "  FROM pg_stat_all_indexes st,\n"
    "       pg_class c\n"
    " WHERE st.indexrelid = c.OID\n"
    " ORDER BY 1,\n"
    "          2",
    "",
    "0702",
    "QPSQL");

static toSQL SQLListPlans("toAnalyze:ListPlans",
                          "SELECT DISTINCT\n"
                          "       statement_id \"Statement\",\n"
                          "       MAX(timestamp) \"Timestamp\",\n"
                          "       MAX(remarks) \"Remarks\" FROM %1\n"
                          " GROUP BY statement_id",
                          "Display available saved statements. Must have same first "
                          "column and %1");


toAnalyze::toAnalyze(QWidget *main, toConnection &connection)
    : toToolWidget(AnalyzeTool, "analyze.html", main, connection, "toAnalyze")
{

    Tabs = new QTabWidget(this);
    layout()->addWidget(Tabs);

    QWidget *container = new QWidget(Tabs);
    QVBoxLayout *box = new QVBoxLayout;
    Tabs->addTab(container, tr("Analyze"));

    QToolBar *toolbar = Utils::toAllocBar(container, tr("Statistics Manager"));
    box->addWidget(toolbar);

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                       tr("Refresh"),
                       this,
                       SLOT(slotRefresh()));

    toolbar->addSeparator();

    Analyzed = NULL;
    if (connection.providerIs("Oracle"))
    {
        Analyzed = new QComboBox(toolbar);
        Analyzed->addItem(tr("All"));
        Analyzed->addItem(tr("Not analyzed"));
        Analyzed->addItem(tr("Analyzed"));
        toolbar->addWidget(Analyzed);
    }

    Schema = new toResultSchema(toolbar);
    Schema->additionalItem(tr("All"));
    Schema->setSelected(tr("All"));
    toolbar->addWidget(Schema);
    Schema->refresh();

    if (connection.providerIs("Oracle"))
    {
        Type = new QComboBox(toolbar);
        Type->addItem(tr("Tables"));
        Type->addItem(tr("Indexes"));
        toolbar->addWidget(Type);

        toolbar->addSeparator();

        Operation = new QComboBox(toolbar);
        Operation->addItem(tr("Compute statistics"));
        Operation->addItem(tr("Estimate statistics"));
        Operation->addItem(tr("Delete statistics"));
        Operation->addItem(tr("Validate references"));
        toolbar->addWidget(Operation);
        connect(Operation,
                SIGNAL(activated(int)),
                this,
                SLOT(slotChangeOperation(int)));

        toolbar->addWidget(
            new QLabel(" " + tr("for") + " ", toolbar));

        For = new QComboBox(toolbar);
        For->addItem(tr("All"));
        For->addItem(tr("Table"));
        For->addItem(tr("Indexed columns"));
        For->addItem(tr("Local indexes"));
        toolbar->addWidget(For);

        toolbar->addSeparator();

        toolbar->addWidget(new QLabel(tr("Sample") + " ",
                                      toolbar));

        Sample = new QSpinBox(toolbar);
        Sample->setMinimum(1);
        Sample->setMaximum(100);
        Sample->setValue(100);
        Sample->setSuffix(" " + tr("%"));
        Sample->setEnabled(false);
        toolbar->addWidget(Sample);
    }
    else if (connection.providerIs("QPSQL"))
    {
        Type = new QComboBox(toolbar);
        Type->addItem(tr("Tables"));
        Type->addItem(tr("Indexes"));
        toolbar->addWidget(Type);

        toolbar->addSeparator();

        Operation = new QComboBox(toolbar);
        Operation->addItem(tr("Vacuum table"));
        Operation->addItem(tr("Analyze table"));
        toolbar->addWidget(Operation);
        connect(Operation,
                SIGNAL(activated(int)),
                this,
                SLOT(slotChangeOperation(int)));

        Sample = NULL;
        For    = NULL;
    }
    else
    {
        Operation = new QComboBox(toolbar);
        Operation->addItem(tr("Analyze table"));
        Operation->addItem(tr("Optimize table"));
        toolbar->addWidget(Operation);
        connect(Operation,
                SIGNAL(activated(int)),
                this,
                SLOT(slotChangeOperation(int)));

        Type   = NULL;
        Sample = NULL;
        For    = NULL;
    }

    toolbar->addSeparator();

    toolbar->addWidget(new QLabel(tr("Parallel") + " ",
                                  toolbar));
    Parallel = new QSpinBox(toolbar);
    Parallel->setMinimum(1);
    Parallel->setMaximum(100);
    toolbar->addWidget(Parallel);

    toolbar->addSeparator();

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(execute_xpm))),
                       tr("Start analyzing"),
                       this,
                       SLOT(slotExecute()));

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(sql_xpm))),
                       tr("Display SQL"),
                       this,
                       SLOT(slotDisplaySQL()));

    Current = new QLabel(toolbar);
    Current->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    toolbar->addWidget(Current);
    Current->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                       QSizePolicy::Minimum));

    Stop = new QToolButton(this);
    Stop->setIcon(QIcon(QPixmap(stop_xpm)));
    Stop->setText(tr("Stop current run"));
    Stop->setEnabled(false);
    connect(Stop, SIGNAL(triggered(QAction *)), this, SLOT(slotStop()));
    toolbar->addWidget(Stop);

    Statistics = new toResultTableView(true, false, container);
    Statistics->setSelectionMode(QAbstractItemView::ExtendedSelection);
    Statistics->setReadAll(true);
    box->addWidget(Statistics);
    connect(Statistics, SIGNAL(done()), this, SLOT(slotFillOwner()));
    connect(Statistics,
            SIGNAL(slotDisplayMenu(QMenu *)),
            this,
            SLOT(slotDisplayMenu(QMenu *)));

    if (Analyzed)
        connect(Analyzed, SIGNAL(activated(int)), this, SLOT(slotRefresh()));
    connect(Schema, SIGNAL(activated(int)), this, SLOT(slotRefresh()));
    if (Type)
        connect(Type, SIGNAL(activated(int)), this, SLOT(slotRefresh()));

    box->setSpacing(0);
    box->setContentsMargins(0, 0, 0, 0);
    container->setLayout(box);

#ifndef TO_NO_ORACLE
    if (connection.providerIs("Oracle"))
    {
        container = new QWidget(Tabs);
        box = new QVBoxLayout;
        toolbar = Utils::toAllocBar(container, tr("Explain plans"));
        box->addWidget(toolbar);

        Tabs->addTab(container, tr("Explain plans"));
        QSplitter *splitter = new QSplitter(Qt::Horizontal, container);
        box->addWidget(splitter);
        Plans = new toResultTableView(false, false, splitter);
        try
        {
            Plans->query(toSQL::string(SQLListPlans, connection).arg(ToConfiguration::Oracle::planTable(connection.user())), toQueryParams());
        }
        TOCATCH;

        connect(Plans,
                SIGNAL(selectionChanged()),
                this,
                SLOT(slotSelectPlan()));

        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                           tr("Refresh"),
                           Plans,
                           SLOT(slotRefresh()));

        QLabel *s = new QLabel(toolbar);
        s->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        s->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                     QSizePolicy::Minimum));
        toolbar->addWidget(s);

        CurrentPlan = new toResultPlanSaved(splitter);

        Worksheet = new toWorksheetStatistic(Tabs);
        Tabs->addTab(Worksheet, tr("Worksheet statistics"));

        box->setSpacing(0);
        box->setContentsMargins(0, 0, 0, 0);
        container->setLayout(box);
    }
    else
#endif
    {
        Plans       = NULL;
        CurrentPlan = NULL;
        Worksheet   = NULL;
    }

    slotRefresh();
    setFocusProxy(Tabs);
}

void toAnalyze::createTool(void)
{
    AnalyzeTool.createWindow();
}

toWorksheetStatistic *toAnalyze::worksheet(void)
{
    Tabs->setCurrentIndex(Tabs->indexOf(Worksheet));
    return Worksheet;
}


void toAnalyze::startQuery(toEventQuery * q)
{
    connect(q, &toEventQuery::dataAvailable, this, &toAnalyze::receiveData);
    connect(q, &toEventQuery::done, this, &toAnalyze::queryDone);
    q->start();
} // connectSlots


QStringList toAnalyze::getSQL(void)
{
    QStringList ret;
    for (toResultTableView::iterator it(Statistics); (*it).isValid(); it++)
    {
        if (Statistics->isRowSelected((*it)))
        {
            if (connection().providerIs("Oracle"))
            {
                QString sql = QString::fromLatin1("ANALYZE %3 %1.%2 ");
                QString forc;
                if ((*it).data(Qt::EditRole) == QString::fromLatin1("TABLE"))
                {
                    switch (For->currentIndex())
                    {
                        case 0:
                            forc = QString::null;
                            break;
                        case 1:
                            forc = QString::fromLatin1(" FOR TABLE");
                            break;
                        case 2:
                            forc = QString::fromLatin1(" FOR ALL INDEXED COLUMNS");
                            break;
                        case 3:
                            forc = QString::fromLatin1(" FOR ALL LOCAL INDEXES");
                            break;
                    }
                }

                switch (Operation->currentIndex())
                {
                    case 0:
                        sql += QString::fromLatin1("COMPUTE STATISTICS");
                        sql += forc;
                        break;
                    case 1:
                        sql += QString::fromLatin1("ESTIMATE STATISTICS");
                        sql += forc;
                        sql += QString::fromLatin1(" SAMPLE %1 PERCENT").arg(Sample->value());
                        break;
                    case 2:
                        sql += QString::fromLatin1("DELETE STATISTICS");
                        break;
                    case 3:
                        sql += QString::fromLatin1("VALIDATE REF UPDATE");
                        break;
                }
                ret.append(
                    sql.arg(Statistics->model()->data((*it).row(), 2).toString())
                    .arg(Statistics->model()->data((*it).row(), 3).toString())
                    .arg(Statistics->model()->data((*it).row(), 1).toString()));

            }
            else if (connection().providerIs("QPSQL"))
            {
                QString sql;

                switch (Operation->currentIndex())
                {
                    case 0:
                        sql += QString("VACUUM FULL VERBOSE %1.%2");
                        break;
                    case 1:
                        sql += QString("ANALYZE VERBOSE %1.%2");
                        break;
                    default:
                        Utils::toStatusMessage(tr("Internal Error"));
                        continue;
                }

                QString table = Statistics->model()->data((*it).row(), 3).toString();
                QString schema = Statistics->model()->data((*it).row(), 2).toString();
                ret.append(sql.arg(schema).arg(table));
            }
            else
            {
                QString sql;
                switch (Operation->currentIndex())
                {
                    case 0:
                        sql = QString::fromLatin1("ANALYZE TABLE %1.%2 ");
                        break;
                    case 1:
                        sql = QString::fromLatin1("OPTIMIZE TABLE %1.%2 ");
                        break;
                }
                QString owner = Statistics->model()->data((*it).row(), 2).toString();
                if (owner.isNull())
                    owner = Schema->selected();
                ret.append(sql.arg(owner).arg(
                               Statistics->model()->data((*it).row(), 1).toString()));
            }
        }
    }
    return ret;
}

void toAnalyze::slotDisplaySQL(void)
{
    QStringList sql = getSQL();
    new toMemoEditor(this, sql.join(";\n"), -1, -1, true);
}

void toAnalyze::slotChangeOperation(int op)
{
    if (Sample)
        Sample->setEnabled(op == 1);
    if (For)
        For->setEnabled(op == 0 || op == 1);
}

void toAnalyze::slotExecute(void)
{
    slotStop();

    Pending = getSQL();

    if (Pending.isEmpty())
        return;

    try
    {
        for (int i = 0; i < Parallel->value(); i++)
        {
            if (!Pending.isEmpty())
            {
                QString sql = Pending.takeFirst();
                toEventQuery * q = new toEventQuery(this, connection(), sql, toQueryParams(), toEventQuery::READ_ALL);
                Running.append(q);
                startQuery(q);
            }
        }
        Stop->setEnabled(true);
    }
    TOCATCH;
}

void toAnalyze::receiveData(toEventQuery* q)
{
    // This function will probably never be called as table statistics
    // calculation is a statement and it does not return any values.
    try
    {
        int cols = q->describe().size();
        for (int j = 0; j < cols; j++)
            q->readValue();  // Eat the output if any.

        Current->setText(tr("Running %1 Pending %2").arg(Running.size()).arg(Pending.size()));
    }
    TOCATCH;
}

void toAnalyze::queryDone(toEventQuery* q)
{
    Running.removeOne(q);
    delete q;

    if (!Pending.isEmpty())
    {
        QString sql = Pending.takeFirst();
        toEventQuery * q = new toEventQuery(this, connection(), sql, toQueryParams(), toEventQuery::READ_ALL);
        startQuery(q);
        Running.append(q);
        Current->setText(tr("Running %1 Pending %2").arg(Running.size()).arg(Pending.size()));
    }
    else
    {
        if (Running.isEmpty())
        {
            slotRefresh();
            slotStop();
        }
    }
} // queryDone

void toAnalyze::slotStop(void)
{
    QList<toEventQuery*>::iterator i;
    for (i = Running.begin(); i != Running.end(); i++)
        delete *i;
    Running.clear();
    Pending.clear();
    Stop->setEnabled(false);
    Current->setText(QString::null);
    //    if (!connection().needCommit())
    //    {
    //        try
    //        {
    //            try
    //            {
    //                connection().rollback();
    //            }
    //            catch (...)
    //            {
    //                TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    //            }
    //        }
    //        catch (...) { }
    //    }
} // stop

void toAnalyze::slotRefresh(void)
{
    try
    {
        Statistics->removeSQL();
        toQueryParams par;
        QString sql;
        if (!Type || Type->currentIndex() == 0)
            sql = toSQL::string(SQLListTables, connection());
        else
            sql = toSQL::string(SQLListIndex, connection());
        if (Schema->selected() != tr("All"))
        {
            par.insert(par.end(), Schema->selected());
            if (connection().providerIs("Oracle"))
                sql += "\n   AND owner = :own<char[100]>";
            else if (connection().providerIs("QPSQL"))
            {
                sql =
                    QString(" SELECT * FROM ( %1 ) sub\n"
                            "  WHERE schemaname = :own<char[100]>").arg(sql);
            }
            else
                sql += " FROM :f1<noquote>";
        }
        else if (connection().providerIs("QMYSQL"))
            sql += " FROM :f1<alldatabases>";
        if (Analyzed)
        {
            switch (Analyzed->currentIndex())
            {
                default:
                    break;
                case 1:
                    sql += QString::fromLatin1("\n  AND Last_Analyzed IS NULL");
                    break;
                case 2:
                    sql += QString::fromLatin1("\n  AND Last_Analyzed IS NOT NULL");
                    break;
            }
        }

        Statistics->query(sql, par);
    }
    TOCATCH;
}

void toAnalyze::slotSelectPlan(void)
{
    QModelIndex index = Plans->selectedIndex();
    if (index.isValid())
        CurrentPlan->query("SAVED:" + index.data(Qt::EditRole).toString(), toQueryParams() << QString("SAVED") << index.data(Qt::EditRole).toString());
}

void toAnalyze::slotFillOwner(void)
{
    for (toResultTableView::iterator it(Statistics); (*it).isValid(); it++)
    {
        if ((*it).data(Qt::EditRole).isNull())
        {
            Statistics->model()->setData(
                (*it),
                Schema->selected(),
                Qt::EditRole);
        }
    }
}

void toAnalyze::slotDisplayMenu(QMenu *menu)
{
    QAction *before = menu->actions()[0];

    menu->insertSeparator(before);

    QAction *action;

    action = new QAction(QIcon(QPixmap(const_cast<const char**>(sql_xpm))),
                         tr("Display SQL"),
                         menu);
    connect(action, SIGNAL(triggered()), this, SLOT(slotDisplaySQL()));
    menu->insertAction(before, action);

    action = new QAction(QIcon(QPixmap(const_cast<const char**>(execute_xpm))),
                         tr("Execute"),
                         menu);
    connect(action, SIGNAL(triggered()), this, SLOT(slotExecute()));
    menu->insertAction(before, action);

    menu->insertSeparator(before);
}
