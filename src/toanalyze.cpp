
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

#include "toanalyze.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "toresultcombo.h"
#include "toresulttableview.h"
#include "toresultplan.h"
#include "tosql.h"
#include "totool.h"
#include "toworksheetstatistic.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbutton.h>

#include <QMenu>
#include <QPixmap>
#include <QVBoxLayout>

#include "icons/execute.xpm"
#include "icons/refresh.xpm"
#include "icons/sql.xpm"
#include "icons/stop.xpm"
#include "icons/toanalyze.xpm"

class toAnalyzeTool : public toTool
{
    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(toanalyze_xpm);
    }
public:
    toAnalyzeTool()
            : toTool(320, "Statistics Manager") { }

    virtual void closeWindow(toConnection &connection) {};

    virtual const char *menuItem()
    {
        return "Statistics Manager";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toAnalyze(parent, connection);
    }
    virtual bool canHandle(toConnection &conn)
    {
        return toIsOracle(conn) || toIsMySQL(conn) || toIsPostgreSQL(conn);
    }
};


static toAnalyzeTool AnalyzeTool;

static toSQL SQLListTablesMySQL("toAnalyze:ListTables",
                                "show table status",
                                "Get table statistics, first three columns and binds must be same",
                                "4.1",
                                "MySQL");
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
    "7.2",
    "PostgreSQL");

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
    "7.2",
    "PostgreSQL");

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

    QToolBar *toolbar = toAllocBar(container, tr("Statistics Manager"));
    box->addWidget(toolbar);

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                       tr("Refresh"),
                       this,
                       SLOT(refresh()));

    toolbar->addSeparator();

    Analyzed = NULL;
    if (toIsOracle(connection))
    {
        Analyzed = new QComboBox(toolbar);
        Analyzed->addItem(tr("All"));
        Analyzed->addItem(tr("Not analyzed"));
        Analyzed->addItem(tr("Analyzed"));
        toolbar->addWidget(Analyzed);
    }

    Schema = new toResultCombo(toolbar, TO_TOOLBAR_WIDGET_NAME);
    Schema->setSelected(tr("All"));
    Schema->additionalItem(tr("All"));
    toolbar->addWidget(Schema);
    try
    {
        Schema->query(toSQL::sql(toSQL::TOSQL_USERLIST));
    }
    TOCATCH;

    if (toIsOracle(connection))
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
                SLOT(changeOperation(int)));

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
    else if (toIsPostgreSQL(connection))
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
                SLOT(changeOperation(int)));

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
                SLOT(changeOperation(int)));

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
                       SLOT(execute()));

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(sql_xpm))),
                       tr("Display SQL"),
                       this,
                       SLOT(displaySQL()));

    Current = new QLabel(toolbar);
    Current->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    toolbar->addWidget(Current);
    Current->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                       QSizePolicy::Minimum));

    Stop = new QToolButton(this);
    Stop->setIcon(QIcon(stop_xpm));
    Stop->setText(tr("Stop current run"));
    Stop->setEnabled(false);
    connect(Stop, SIGNAL(triggered(QAction *)), this, SLOT(stop()));
    toolbar->addWidget(Stop);

    Statistics = new toResultTableView(true, false, container);
    Statistics->setSelectionMode(QAbstractItemView::ExtendedSelection);
    Statistics->setReadAll(true);
    box->addWidget(Statistics);
    connect(Statistics, SIGNAL(done()), this, SLOT(fillOwner()));
    connect(Statistics,
            SIGNAL(displayMenu(QMenu *)),
            this,
            SLOT(displayMenu(QMenu *)));

    if (Analyzed)
        connect(Analyzed, SIGNAL(activated(int)), this, SLOT(refresh()));
    connect(Schema, SIGNAL(activated(int)), this, SLOT(refresh()));
    if (Type)
        connect(Type, SIGNAL(activated(int)), this, SLOT(refresh()));

    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));

    box->setSpacing(0);
    box->setContentsMargins(0, 0, 0, 0);
    container->setLayout(box);

    if (toIsOracle(connection))
    {
        container = new QWidget(Tabs);
        box = new QVBoxLayout;
        toolbar = toAllocBar(container, tr("Explain plans"));
        box->addWidget(toolbar);

        Tabs->addTab(container, tr("Explain plans"));
        QSplitter *splitter = new QSplitter(Qt::Horizontal, container);
        box->addWidget(splitter);
        Plans = new toResultTableView(false, false, splitter);
        try
        {
            Plans->query(toSQL::string(SQLListPlans, connection).arg(
                             toConfigurationSingle::Instance().planTable()));
        }
        TOCATCH;

        connect(Plans,
                SIGNAL(selectionChanged()),
                this,
                SLOT(selectPlan()));

        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                           tr("Refresh"),
                           Plans,
                           SLOT(refresh()));

        QLabel *s = new QLabel(toolbar);
        s->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        s->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                     QSizePolicy::Minimum));
        toolbar->addWidget(s);

        CurrentPlan = new toResultPlan(splitter);

        Worksheet = new toWorksheetStatistic(Tabs);
        Tabs->addTab(Worksheet, tr("Worksheet statistics"));

        box->setSpacing(0);
        box->setContentsMargins(0, 0, 0, 0);
        container->setLayout(box);
    }
    else
    {
        Plans       = NULL;
        CurrentPlan = NULL;
        Worksheet   = NULL;
    }

    refresh();
    setFocusProxy(Tabs);
}

void toAnalyze::fillOwner(void)
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

void toAnalyze::selectPlan(void)
{
    QModelIndex index = Plans->selectedIndex();
    if (index.isValid())
        CurrentPlan->query("SAVED:" + index.data(Qt::EditRole).toString());
}

toWorksheetStatistic *toAnalyze::worksheet(void)
{
    Tabs->setCurrentIndex(Tabs->indexOf(Worksheet));
    return Worksheet;
}

void toAnalyze::changeOperation(int op)
{
    if (Sample)
        Sample->setEnabled(op == 1);
    if (For)
        For->setEnabled(op == 0 || op == 1);
}

void toAnalyze::refresh(void)
{
    try
    {
        Statistics->setSQL(QString::null);
        toQList par;
        QString sql;
        if (!Type || Type->currentIndex() == 0)
            sql = toSQL::string(SQLListTables, connection());
        else
            sql = toSQL::string(SQLListIndex, connection());
        if (Schema->selected() != tr("All"))
        {
            par.insert(par.end(), Schema->selected());
            if (toIsOracle(connection()))
                sql += "\n   AND owner = :own<char[100]>";
            else if (toIsPostgreSQL(connection()))
            {
                sql =
                    QString(" SELECT * FROM ( %1 ) sub\n"
                            "  WHERE schemaname = :own<char[100]>").arg(sql);
            }
            else
                sql += " FROM :f1<noquote>";
        }
        else if (toIsMySQL(connection()))
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

        Statistics->query(sql, (const toQList &)par);
    }
    TOCATCH;
}

void toAnalyze::poll(void)
{
    try
    {
        int running = 0;
        for (std::list<toNoBlockQuery *>::iterator i = Running.begin();i != Running.end();i++)
        {
            bool eof = false;

            try
            {
                if ((*i)->poll())
                {
                    int cols = (*i)->describe().size();
                    for (int j = 0;j < cols;j++)
                        (*i)->readValueNull();  // Eat the output if any.
                }

                try
                {
                    eof = (*i)->eof();
                }
                catch (const QString &)
                {
                    eof = true;
                }
            }
            catch (const QString &err)
            {
                toStatusMessage(err);
                eof = true;
            }
            if (eof)
            {
                QString sql = toShift(Pending);
                if (!sql.isEmpty())
                {
                    delete(*i);
                    toQList par;
                    (*i) = new toNoBlockQuery(connection(), sql, par);
                    running++;
                }
            }
            else
                running++;
        }
        if (!running)
        {
            Poll.stop();
            refresh();
            stop();
        }
        else
            Current->setText(tr("Running %1 Pending %2").arg(running).arg(Pending.size()));
    }
    TOCATCH;
}

std::list<QString> toAnalyze::getSQL(void)
{
    std::list<QString> ret;
    for (toResultTableView::iterator it(Statistics); (*it).isValid(); it++)
    {
        if (Statistics->isRowSelected((*it)))
        {
            if (toIsOracle(connection()))
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
                toPush(ret,
                       sql.arg(Statistics->model()->data((*it).row(), 2).toString())
                       .arg(Statistics->model()->data((*it).row(), 3).toString())
                       .arg(Statistics->model()->data((*it).row(), 1).toString()));

            }
            else if (toIsPostgreSQL(connection()))
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
                    toStatusMessage(tr("Internal Error"));
                    continue;
                }

                QString table = Statistics->model()->data((*it).row(), 3).toString();
                QString schema = Statistics->model()->data((*it).row(), 2).toString();
                toPush(ret, sql.arg(schema).arg(table));
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
                if (toUnnull(owner).isNull())
                    owner = Schema->selected();
                toPush(ret,
                       sql.arg(owner).arg(
                           Statistics->model()->data((*it).row(), 1).toString()));
            }
        }
    }
    return ret;
}

void toAnalyze::displaySQL(void)
{
    QString txt;
    std::list<QString> sql = getSQL();
    for (std::list<QString>::iterator i = sql.begin();i != sql.end();i++)
        txt += (*i) + ";\n";
    new toMemoEditor(this, txt, -1, -1, true);
}

void toAnalyze::execute(void)
{
    stop();

    std::list<QString> sql = getSQL();
    for (std::list<QString>::iterator i = sql.begin();i != sql.end();i++)
        toPush(Pending, *i);

    try
    {
        toQList par;
        for (int i = 0; i < Parallel->value(); i++)
        {
            QString sql = toShift(Pending);
            if (!sql.isEmpty())
                toPush(Running, new toNoBlockQuery(connection(), sql, par));
        }
        Poll.start(100);
        Stop->setEnabled(true);
        poll();
    }
    TOCATCH;
}

void toAnalyze::stop(void)
{
    try
    {
        for_each(Running.begin(), Running.end(), DeleteObject());
        Running.clear();
        Pending.clear();
        Stop->setEnabled(false);
        Current->setText(QString::null);
        if (!connection().needCommit())
        {
            try
            {
                connection().rollback();
            }
            catch (...) { }
        }
    }
    TOCATCH;
}

void toAnalyze::createTool(void)
{
    AnalyzeTool.createWindow();
}

void toAnalyze::displayMenu(QMenu *menu)
{
    QAction *before = menu->actions()[0];

    menu->insertSeparator(before);

    QAction *action;

    action = new QAction(QIcon(QPixmap(const_cast<const char**>(sql_xpm))),
                         tr("Display SQL"),
                         menu);
    connect(action, SIGNAL(triggered()), this, SLOT(displaySQL()));
    menu->insertAction(before, action);

    action = new QAction(QIcon(QPixmap(const_cast<const char**>(execute_xpm))),
                         tr("Execute"),
                         menu);
    connect(action, SIGNAL(triggered()), this, SLOT(execute()));
    menu->insertAction(before, action);

    menu->insertSeparator(before);
}
