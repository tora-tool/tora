/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toanalyze.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "toresultcombo.h"
#include "toresultlong.h"
#include "toresultplan.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"
#include "toworksheetstatistic.h"

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <qcombobox.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include "toanalyze.moc"

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
            : toTool(320, "Statistics Manager")
    { }
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
        return toIsOracle(conn) || toIsMySQL(conn);
    }
};

static toAnalyzeTool AnalyzeTool;

static toSQL SQLListTablesMySQL("toAnalyze:ListTables",
                                "toad 0,* show table status",
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

static toSQL SQLListPlans("toAnalyze:ListPlans",
                          "SELECT DISTINCT\n"
                          "       statement_id \"Statement\",\n"
                          "       MAX(timestamp) \"Timestamp\",\n"
                          "       MAX(remarks) \"Remarks\" FROM %1\n"
                          " GROUP BY statement_id",
                          "Display available saved statements. Must have same first "
                          "column and %1");

toAnalyze::toAnalyze(QWidget *main, toConnection &connection)
        : toToolWidget(AnalyzeTool, "analyze.html", main, connection)
{
    Tabs = new QTabWidget(this);
    QVBox *box = new QVBox(Tabs);
    Tabs->addTab(box, tr("Analyze"));

    QToolBar *toolbar = toAllocBar(box, tr("Statistics Manager"));

    new QToolButton(QPixmap(const_cast<const char**>(refresh_xpm)),
                    tr("Refresh"),
                    tr("Refresh"),
                    this, SLOT(refresh()),
                    toolbar);

    toolbar->addSeparator();
    if (toIsOracle(connection))
    {
        Analyzed = new QComboBox(toolbar, TO_KDE_TOOLBAR_WIDGET);
        Analyzed->insertItem(tr("All"));
        Analyzed->insertItem(tr("Not analyzed"));
        Analyzed->insertItem(tr("Analyzed"));
    }
    else
        Analyzed = NULL;

    Schema = new toResultCombo(toolbar, TO_KDE_TOOLBAR_WIDGET);
    Schema->setSelected(tr("All"));
    Schema->additionalItem(tr("All"));
    try
    {
        Schema->query(toSQL::sql(toSQL::TOSQL_USERLIST));
    }
    TOCATCH


    if (toIsOracle(connection))
    {
        Type = new QComboBox(toolbar, TO_KDE_TOOLBAR_WIDGET);
        Type->insertItem(tr("Tables"));
        Type->insertItem(tr("Indexes"));

        toolbar->addSeparator();

        Operation = new QComboBox(toolbar, TO_KDE_TOOLBAR_WIDGET);
        Operation->insertItem(tr("Compute statistics"));
        Operation->insertItem(tr("Estimate statistics"));
        Operation->insertItem(tr("Delete statistics"));
        Operation->insertItem(tr("Validate references"));
        connect(Operation, SIGNAL(activated(int)),
                this, SLOT(changeOperation(int)));

        new QLabel(" " + tr("for") + " ", toolbar, TO_KDE_TOOLBAR_WIDGET);
        For = new QComboBox(toolbar, TO_KDE_TOOLBAR_WIDGET);
        For->insertItem(tr("All"));
        For->insertItem(tr("Table"));
        For->insertItem(tr("Indexed columns"));
        For->insertItem(tr("Local indexes"));
        toolbar->addSeparator();
        new QLabel(tr("Sample") + " ", toolbar, TO_KDE_TOOLBAR_WIDGET);
        Sample = new QSpinBox(1, 100, 1, toolbar, TO_KDE_TOOLBAR_WIDGET);
        Sample->setValue(100);
        Sample->setSuffix(" " + tr("%"));
        Sample->setEnabled(false);
    }
    else
    {
        Operation = new QComboBox(toolbar, TO_KDE_TOOLBAR_WIDGET);
        Operation->insertItem(tr("Analyze table"));
        Operation->insertItem(tr("Optimize table"));
        connect(Operation, SIGNAL(activated(int)),
                this, SLOT(changeOperation(int)));

        Type = NULL;
        Sample = NULL;
        For = NULL;
    }

    toolbar->addSeparator();
    new QLabel(tr("Parallel") + " ", toolbar, TO_KDE_TOOLBAR_WIDGET);
    Parallel = new QSpinBox(1, 100, 1, toolbar, TO_KDE_TOOLBAR_WIDGET);
    toolbar->addSeparator();

    new QToolButton(QPixmap(const_cast<const char**>(execute_xpm)),
                    tr("Start analyzing"),
                    tr("Start analyzing"),
                    this, SLOT(execute()),
                    toolbar);
    new QToolButton(QPixmap(const_cast<const char**>(sql_xpm)),
                    tr("Display SQL"),
                    tr("Display SQL"),
                    this, SLOT(displaySQL()),
                    toolbar);

    Current = new QLabel(toolbar, TO_KDE_TOOLBAR_WIDGET);
    Current->setAlignment(AlignRight | AlignVCenter | ExpandTabs);
    toolbar->setStretchableWidget(Current);

    Stop = new QToolButton(QPixmap(const_cast<const char**>(stop_xpm)),
                           tr("Stop current run"),
                           tr("Stop current run"),
                           this, SLOT(stop()),
                           toolbar);
    Stop->setEnabled(false);

    Statistics = new toResultLong(true, false, toQuery::Background, box);
    Statistics->setSelectionMode(QListView::Extended);
    Statistics->setReadAll(true);
    connect(Statistics, SIGNAL(done()), this, SLOT(fillOwner()));
    connect(Statistics, SIGNAL(displayMenu(QPopupMenu *)), this, SLOT(displayMenu(QPopupMenu *)));

    if (Analyzed)
        connect(Analyzed, SIGNAL(activated(int)), this, SLOT(refresh()));
    connect(Schema, SIGNAL(activated(int)), this, SLOT(refresh()));
    if (Type)
        connect(Type, SIGNAL(activated(int)), this, SLOT(refresh()));

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(windowActivated(QWidget *)),
            this, SLOT(windowActivated(QWidget *)));

    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));

    if (toIsOracle(connection))
    {
        box = new QVBox(Tabs);
        toolbar = toAllocBar(box, tr("Explain plans"));

        Tabs->addTab(box, tr("Explain plans"));
        QSplitter *splitter = new QSplitter(Horizontal, box);
        Plans = new toResultLong(false, false, toQuery::Background, splitter);
        Plans->setSelectionMode(QListView::Single);
        try
        {
            Plans->query(toSQL::string(SQLListPlans,
                                       connection).arg(toConfigurationSingle::Instance().globalConfig(CONF_PLAN_TABLE,
                                                                            DEFAULT_PLAN_TABLE)));
        }
        TOCATCH

        connect(Plans, SIGNAL(selectionChanged()),
                this, SLOT(selectPlan()));
        new QToolButton(QPixmap(const_cast<const char**>(refresh_xpm)),
                        tr("Refresh"),
                        tr("Refresh"),
                        Plans, SLOT(refresh()),
                        toolbar);
        toolbar->setStretchableWidget(new QLabel(toolbar, TO_KDE_TOOLBAR_WIDGET));

        CurrentPlan = new toResultPlan(splitter);

        Worksheet = new toWorksheetStatistic(Tabs);
        Tabs->addTab(Worksheet, tr("Worksheet statistics"));
    }
    else
    {
        Plans = NULL;
        CurrentPlan = NULL;
        Worksheet = NULL;
    }

    refresh();
    setFocusProxy(Tabs);
}

void toAnalyze::fillOwner(void)
{
    for (QListViewItem *item = Statistics->firstChild();item;item = item->nextSibling())
    {
        if (toUnnull(item->text(0)).isNull())
            item->setText(0, Schema->selected());
    }
}

void toAnalyze::selectPlan(void)
{
    QListViewItem *item = Plans->selectedItem();
    if (item)
        CurrentPlan->query("SAVED:" + item->text(0));
}

void toAnalyze::windowActivated(QWidget *widget)
{
    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QPopupMenu(this);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(refresh_xpm)), tr("&Refresh"),
                                 this, SLOT(refresh(void)), toKeySequence(tr("F5", "Statistics|Refresh")));

            toMainWidget()->menuBar()->insertItem(tr("&Statistics"), ToolMenu, -1, toToolMenuIndex());
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

toWorksheetStatistic *toAnalyze::worksheet(void)
{
    Tabs->showPage(Worksheet);
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
        if (!Type || Type->currentItem() == 0)
            sql = toSQL::string(SQLListTables, connection());
        else
            sql = toSQL::string(SQLListIndex, connection());
        if (Schema->selected() != tr("All"))
        {
            par.insert(par.end(), Schema->selected());
            if (toIsOracle(connection()))
                sql += "\n   AND owner = :own<char[100]>";
            else
                sql += " FROM :f1<noquote>";
        }
        else if (toIsMySQL(connection()))
            sql += " FROM :f1<alldatabases>";
        if (Analyzed)
        {
            switch (Analyzed->currentItem())
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
    TOCATCH
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
                    delete (*i);
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
    TOCATCH
}

std::list<QString> toAnalyze::getSQL(void)
{
    std::list<QString> ret;
    for (QListViewItem *item = Statistics->firstChild();item;item = item->nextSibling())
    {
        if (item->isSelected())
        {
            if (toIsOracle(connection()))
            {
                QString sql = QString::fromLatin1("ANALYZE %3 %1.%2 ");
                QString forc;
                if (item->text(0) == QString::fromLatin1("TABLE"))
                {
                    switch (For->currentItem())
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

                switch (Operation->currentItem())
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
                toPush(ret, sql.arg(item->text(1)).arg(item->text(2)).arg(item->text(0)));

            }
            else
            {
                QString sql;
                switch (Operation->currentItem())
                {
                case 0:
                    sql = QString::fromLatin1("ANALYZE TABLE %1.%2 ");
                    break;
                case 1:
                    sql = QString::fromLatin1("OPTIMIZE TABLE %1.%2 ");
                    break;
                }
                QString owner = item->text(0);
                if (toUnnull(owner).isNull())
                    owner = Schema->selected();
                toPush(ret, sql.arg(owner).arg(item->text(1)));
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
        for (int i = 0;i < Parallel->value();i++)
        {
            QString sql = toShift(Pending);
            if (!sql.isEmpty())
                toPush(Running, new toNoBlockQuery(connection(), sql, par));
        }
        Poll.start(100);
        Stop->setEnabled(true);
        poll();
    }
    TOCATCH
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
            catch (...)
            { }
        }
    }
    TOCATCH
}

void toAnalyze::createTool(void)
{
    AnalyzeTool.createWindow();
}

void toAnalyze::displayMenu(QPopupMenu *menu)
{
    menu->insertSeparator(0);
    menu->insertItem(QPixmap(const_cast<const char**>(sql_xpm)), tr("Display SQL"), this, SLOT(displaySQL()), 0, 0, 0);
    menu->insertItem(QPixmap(const_cast<const char**>(execute_xpm)), tr("Execute"), this, SLOT(execute()), 0, 0, 0);
    menu->insertSeparator(0);
    menu->insertItem(QPixmap(const_cast<const char**>(refresh_xpm)), tr("Refresh"), this, SLOT(refresh()), 0, 0, 0);
}
