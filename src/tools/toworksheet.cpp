
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

#include "tools/toworksheet.h"
#include "ui_toworksheetsetupui.h"

//#include "tools/tosgatrace.h"
//
#include "core/toeditablemenu.h"
#include "core/utils.h"
#include "core/toconfenum.h"
#include "core/tochangeconnection.h"
#include "core/toconnectionsub.h"
#include "core/toconnectiontraits.h"
#include "tools/toparamget.h"
#include "tools/toresultbar.h"
#include "tools/toresultcols.h"
#include "tools/toresultplan.h"
#include "tools/toresultstats.h"
#include "tools/toresulttableview.h"
#include "tools/toresultview.h"
#include "widgets/toresultschema.h"
#include "widgets/toresultitem.h"
#include "widgets/toresultresources.h"
#include "widgets/totabwidget.h"
#include "widgets/totreewidget.h"
#include "widgets/torefreshcombo.h"
#include "widgets/tooutput.h"

#include "editor/toworksheettext.h"
//obsolete #include "core/tovisualize.h"
#ifdef TORA3_STAT
#include "tools/toworksheetstatistic.h"
#endif
#include "todescribe.h"
#include "core/toeditmenu.h"

#ifdef TORA_EXPERIMENTAL
#include "parsing/tsqlparse.h"
#endif
#include "parsing/tsqllexer.h"
#include "core/tosyntaxanalyzer.h"
#include "editor/tosyntaxanalyzernl.h"
#include "editor/tosyntaxanalyzeroracle.h"

#include "editor/tosqltext.h"
#include "core/toglobalevent.h"
#include "core/toconfiguration.h"
#include "core/todatabaseconfig.h"
#include "connection/toqmysqlsetting.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QInputDialog>
#include <QProgressDialog>

#include "icons/clock.xpm"
#include "icons/recall.xpm"
#include "icons/describe.xpm"
#include "icons/eraselog.xpm"
#include "icons/lockconnection.xpm"
#include "icons/execute.xpm"
#include "icons/executeall.xpm"
#include "icons/executestep.xpm"
#include "icons/explainplan.xpm"
#include "icons/filesave.xpm"
#include "icons/insertsaved.xpm"
#include "icons/previous.xpm"
#include "icons/refresh.xpm"
#include "icons/stop.xpm"
#include "icons/toworksheet.xpm"
#include "icons/up.xpm"
#include "icons/down.xpm"
#include "icons/tooutput.xpm"

namespace ToConfiguration
{
    QVariant Worksheet::defaultValue(int option) const
    {
        switch (option)
        {
            case AutoSaveBool:
                return QVariant((bool)false);
            case CheckSaveBool:
                return QVariant((bool)true);
            case LogAtEndBool:
                return QVariant((bool)true);
            case LogMultiBool:
                return QVariant((bool)true);
            case StatisticsBool:
                return QVariant((bool)false);
            case TimedStatsBool:
                return QVariant((bool)true);
            case DisplayNumberColumnBool:
                return QVariant((bool)true);
            case MoveToErrorBool:
                return QVariant((bool)true);
            case HistoryErrorBool:
                return QVariant((bool)false);
            case ExecLogBool:
                return QVariant((bool)false);
            case ToplevelDescribeBool:
                return QVariant((bool)true);
            case AutoLoad:
                return QVariant(QString(""));
            default:
                Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Worksheet un-registered enum value: %1").arg(option)));
                return QVariant();
        }
    }
}

class toWorksheetTool : public toTool
{
    protected:
        virtual const char **pictureXPM(void)
        {
            return const_cast<const char**>(toworksheet_xpm);
        }
    public:
        toWorksheetTool()
            : toTool(10, "SQL Editor") { }
        virtual const char *menuItem()
        {
            return "SQL Editor";
        }
        virtual toToolWidget* toolWindow(QWidget *parent, toConnection &connection)
        {
            return new toWorksheet(parent, connection);
        }
        virtual QWidget *configurationTab(QWidget *parent)
        {
            return new toWorksheetSetting(this, parent);
        }
        virtual bool canHandle(const toConnection &)
        {
            return true;
        }
        virtual void closeWindow(toConnection &connection) {};
    private:
        static ToConfiguration::Worksheet s_worksheetConf;
};

static toWorksheetTool WorksheetTool;

ToConfiguration::Worksheet toWorksheetTool::s_worksheetConf;

#define CHUNK_SIZE 31
/** Get an address to a SQL statement in the SGA. The address has the form
 * 'address:hash_value' which are resolved from the v$sqltext_with_newlines
 * view in Oracle.
 * @param conn Connection to get address from
 * @param sql Statement to get address for.
 * @return String with address in.
 * @exception QString if address not found.
 */
static toSQL SQLAddress("Global:Address",
                        "SELECT Address||':'||Hash_Value\n"
                        "  FROM V$SQLText_With_Newlines\n"
                        " WHERE SQL_Text LIKE :f1<char[150]>||'%'",
                        "Get address of an SQL statement");

static toSQL SQLParseSql("toWorksheet:ParseSql",
                         "declare                                                                      "
                         "	 l_cursor number := dbms_sql.open_cursor;                                 "
                         "	 l_offset number := -1 ;                                                  "
                         "begin                                                                        "
                         "    begin                                                                    "
                         "        dbms_sql.parse(  l_cursor, :st<char[32000],in> , dbms_sql.native );  "
                         "    exception when others then                                               "
                         "        l_offset := dbms_sql.last_error_position;                            "
                         "    end;                                                                     "
                         "    dbms_sql.close_cursor( l_cursor );                                       "
                         "    :off<int,out> := l_offset;                                               "
                         "end;                                                                         "
                         , "Parse Oracle SQL query"
                         , "0800"
                         , "Oracle");

#if 0
QString toSQLToSql_Id(const QString &sql);

static QString toSQLToAddress(toConnection &conn, const QString &sql)
{
	QString sql_id = toSQLToSql_Id(sql);

    QString search = Utils::toSQLStripSpecifier(sql);
	QString s = search.left(CHUNK_SIZE);
    toQList vals = toQuery::readQuery(conn, SQLAddress, toQueryParams() << search.left(CHUNK_SIZE));

    for (toQList::iterator i = vals.begin(); i != vals.end(); i++)
    {
		int index = std::distance(vals.begin(), i);
        if (search == Utils::toSQLString(conn, (QString)*i))
            return (QString)*i;
    }
    throw qApp->translate("toSQLToAddress", "SQL Query not found in SGA");
}
#endif

void toWorksheet::viewResources(void)
{
    try
    {
		QString sql_id = Utils::toSQLToSql_Id(m_lastQuery.sql);
		Resources->refreshWithParams(toQueryParams() /*<< sid*/ << sql_id);

		if (LockedConnection)
		{
			toQueryParams sidserial = (*LockedConnection)->sessionId();
			QString sql = toSQL::string("toSGATrace:LongOps", connection());
			sql += " AND b.SID = :sid<int> && b.SERIAL# = :serial<int> \n";
			sql += " AND b.SQL_ID :sqlid<char[100]>                    \n";
			LongOps->setSQL(sql);
			LongOps->clearParams();
			LongOps->refreshWithParams(toQueryParams() << sidserial << sql_id);
		}
    }
    TOCATCH
}

void toWorksheet::createActions()
{
    parseAct = new QAction(tr("Check syntax of buffer"), this);
    parseAct->setShortcut(Qt::CTRL + Qt::Key_F9);
    connect(parseAct, SIGNAL(triggered()), this, SLOT(slotParse(void)));

    executeAct = new QAction(QPixmap(const_cast<const char**>(execute_xpm)),
                             tr("Execute current statement"),
                             this);
    executeAct->setShortcut(Qt::CTRL + Qt::Key_Return);
    connect(executeAct, SIGNAL(triggered()), this, SLOT(slotExecute(void)));

    executeStepAct = new QAction(QPixmap(const_cast<const char**>(executestep_xpm)),
                                 tr("Step through statements"),
                                 this);
    executeStepAct->setShortcut(Qt::Key_F9);
    connect(executeStepAct, SIGNAL(triggered()), this, SLOT(slotExecuteStep(void)));

    executeAllAct = new QAction(QPixmap(const_cast<const char**>(executeall_xpm)),
                                tr("Execute all statements"),
                                this);
    executeAllAct->setShortcut(Qt::Key_F8);
    connect(executeAllAct, SIGNAL(triggered()), this, SLOT(slotExecuteAll(void)));

    refreshAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                             tr("Reexecute Last Statement"),
                             this);
    refreshAct->setShortcut(QKeySequence::Refresh); //NOTE: on Win and MacOSX Refresh is bound to "F5"
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(slotRefreshSetup(void)));

    describeAct = new QAction(QPixmap(const_cast<const char**>(describe_xpm)),
                              tr("Describe under cursor"),
                              this);
    describeAct->setShortcut(Qt::Key_F4);
    connect(describeAct, SIGNAL(triggered()), this, SLOT(slotDescribe(void)));

    describeActNew = new QAction(QPixmap(const_cast<const char**>(describe_xpm)),
                                 tr("Describe under cursor(New parser)"),
                                 this);
    describeActNew->setShortcut(Qt::SHIFT + Qt::Key_F4);
    connect(describeActNew, SIGNAL(triggered()), this, SLOT(slotDescribeNew(void)));

    explainAct = new QAction(QPixmap(const_cast<const char**>(explainplan_xpm)),
                             tr("Explain plan of current statement"),
                             this);
    explainAct->setShortcut(Qt::Key_F6);
    connect(explainAct, SIGNAL(triggered()), this, SLOT(slotExplainPlan(void)));

    stopAct = new QAction(QPixmap(const_cast<const char**>(stop_xpm)),
                          tr("Stop execution"),
                          this);
    connect(stopAct, SIGNAL(triggered()), this, SLOT(slotStop(void)));
    stopAct->setEnabled(false);

    lockConnectionAct = new QAction(QPixmap(const_cast<const char**>(lockconnection_xpm)),
                                    tr("Lock connection to this worksheet"),
                                    this);
    lockConnectionAct->setCheckable(true);
    connect(lockConnectionAct, SIGNAL(toggled(bool)), this, SLOT(slotLockConnection(bool)));

    outputAct = new QAction(QPixmap(const_cast<const char**>(tooutput_xpm)),
                                    tr("Enable output"),
                                    this);
    outputAct->setCheckable(true);

    connect(outputAct, SIGNAL(toggled(bool)), this, SLOT(slotEnableOutput(bool)));

    eraseAct = new QAction(QPixmap(const_cast<const char**>(eraselog_xpm)),
                           tr("Clear execution log"),
                           this);
    connect(eraseAct, SIGNAL(triggered()), this, SLOT(slotEraseLogButton(void)));

    statisticAct = new QAction(QPixmap(const_cast<const char**>(clock_xpm)),
                               tr("Gather session statistic of execution"),
                               this);
    statisticAct->setCheckable(true);
    connect(statisticAct, SIGNAL(toggled(bool)), this, SLOT(slotEnableStatistic(bool)));

    previousAct = new QAction(QPixmap(const_cast<const char**>(up_xpm)),
                              tr("Previous log entry"),
                              this);
    connect(previousAct, SIGNAL(triggered()), this, SLOT(slotExecutePreviousLog(void)));

    nextAct = new QAction(QPixmap(const_cast<const char**>(down_xpm)),
                          tr("Next log entry"),
                          this);
    connect(nextAct, SIGNAL(triggered()), this, SLOT(slotExecuteNextLog(void)));

    saveLastAct = new QAction(QPixmap(const_cast<const char**>(previous_xpm)),
                              tr("Save last SQL"),
                              this);
    connect(saveLastAct, SIGNAL(triggered()), this, SLOT(slotSaveLast(void)));
}


void toWorksheet::setup(bool autoLoad)
{
    QToolBar *workToolbar = Utils::toAllocBar(this, tr("SQL worksheet"));
    layout()->addWidget(workToolbar);

    workToolbar->addAction(executeAct);
    workToolbar->addAction(executeStepAct);
    workToolbar->addAction(executeAllAct);
    workToolbar->addAction(stopAct);
    workToolbar->addAction(lockConnectionAct);

    workToolbar->addSeparator();
    workToolbar->addAction(outputAct);

    workToolbar->addSeparator();
    workToolbar->addAction(refreshAct);

    workToolbar->addSeparator();
    workToolbar->addAction(describeAct);
    workToolbar->addAction(describeActNew);
    workToolbar->addAction(explainAct);

    workToolbar->addSeparator();
    workToolbar->addAction(eraseAct);

    workToolbar->addSeparator();
    workToolbar->addAction(statisticAct);
    workToolbar->addWidget(new QLabel(tr("Refresh") + " ",
                                      workToolbar));
    Refresh = new toRefreshCombo(workToolbar);
    workToolbar->addWidget(Refresh);
    Refresh->setEnabled(false);
    Refresh->setFocusPolicy(Qt::NoFocus);

    // enable refresh button from statisticAct
    connect(statisticAct, SIGNAL(toggled(bool)), Refresh, SLOT(setEnabled(bool)));

    workToolbar->addSeparator();
    workToolbar->addAction(previousAct);
    workToolbar->addAction(nextAct);

    workToolbar->addSeparator();
    InsertSavedMenu = new toEditableMenu(workToolbar);
    InsertSavedMenu->setIcon(
        QPixmap(const_cast<const char**>(insertsaved_xpm)));
    InsertSavedMenu->setTitle(tr("Insert current saved SQL"));
    workToolbar->addAction(InsertSavedMenu->menuAction());
    connect(InsertSavedMenu,
            SIGNAL(aboutToShow()),
            this,
            SLOT(slotShowInsertSaved()));
    connect(InsertSavedMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(slotInsertSaved(QAction *)));
    connect(InsertSavedMenu,
            SIGNAL(actionRemoved(QAction *)),
            this,
            SLOT(slotRemoveSaved(QAction *)));

    SavedMenu = new toEditableMenu(workToolbar);
    SavedMenu->setIcon(QPixmap(const_cast<const char**>(recall_xpm)));
    SavedMenu->setTitle(tr("Run current saved SQL"));
    workToolbar->addAction(SavedMenu->menuAction());
    connect(SavedMenu, SIGNAL(aboutToShow()), this, SLOT(slotShowSaved()));
    connect(SavedMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(slotExecuteSaved(QAction *)));
    connect(SavedMenu,
            SIGNAL(actionRemoved(QAction *)),
            this,
            SLOT(slotRemoveSaved(QAction *)));

    workToolbar->addAction(saveLastAct);

    workToolbar->addWidget(Started = new QLabel(workToolbar));
    Started->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    Started->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                       QSizePolicy::Minimum));

    Schema = new toResultSchema(workToolbar);
    workToolbar->addWidget(Schema);
    connect(Schema, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(slotSchemaChanged(const QString &)));

    Schema->refresh();

    new toChangeConnection(workToolbar);

    RefreshSeconds = 60;
    connect(&RefreshTimer, SIGNAL(timeout()), this, SLOT(slotRefresh()));

    LastID = 0;

    EditSplitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(EditSplitter);

    //Editor = new toWorksheetEditor(this, EditSplitter);
	Editor = new toWorksheetText(this, EditSplitter);
    // stop any running query when a file is loaded
    connect(Editor, SIGNAL(fileOpened()), this, SLOT(slotStop()));
    connect(Editor, SIGNAL(modificationChanged(bool)), this, SLOT(slotSetCaption()));

    ResultTab = new toTabWidget(EditSplitter);

    Current = Result = new toResultTableView(false, true, ResultTab, "ResultTab");
    ResultTab->addTab(Result, tr("&Result"));
    connect(Result, SIGNAL(done(void)), this, SLOT(slotQueryDone(void)));
    connect(Result,
            SIGNAL(firstResult(const QString &,
                               const toConnection::exception &,
                               bool)),
            this,
            SLOT(slotFirstResult(const QString &,
                                 const toConnection::exception &,
                                 bool)));
    connect(Result, SIGNAL(modelChanged(toResultModel*)), this, SLOT(slotRefreshModel(toResultModel*)));
    connect(Result,
            SIGNAL(firstResult(const QString &,
                               const toConnection::exception &,
                               bool)),
            this,
            SLOT(slotUnhideResults(const QString &,
                                   const toConnection::exception &,
                                   bool)));

    Columns = new toResultCols(ResultTab, "description");
    ResultTab->addTab(Columns, tr("&Columns"));
    Columns->hide();
    ResultTab->setTabEnabled(ResultTab->indexOf(Columns), false);

    Plan = new toResultPlanExplain(ResultTab);
    Plan->setRelatedAction(explainAct);
    ResultTab->addTab(Plan, tr("E&xecution plan"));

    ResourceSplitter = new QSplitter(Qt::Vertical, ResultTab);
    Resources = new toResultResources(ResourceSplitter);

    LongOps = new toResultTableView(true, true, ResourceSplitter);

    //obsolete Visualize = new toVisualize(Result, ResultTab);
    // ResultTab->addTab(Visualize, tr("&Visualize"));
    ResultTab->addTab(ResourceSplitter, tr("&Information"));

    StatTab = new QWidget(ResultTab);
    QVBoxLayout *box = new QVBoxLayout;

    QToolBar *stattool = Utils::toAllocBar(StatTab, tr("Worksheet Statistics"));

    stattool->addAction(QIcon(QPixmap(const_cast<const char**>(filesave_xpm))),
                        tr("Save statistics for later analysis"),
                        this,
                        SLOT(slotSaveStatistics(void)));

    QLabel *statlabel = new QLabel(stattool);
    stattool->addWidget(statlabel);

    box->addWidget(stattool);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, StatTab);
    Statistics = new toResultStats(true, splitter);
    Statistics->setRelatedAction(statisticAct);

#ifdef TORA_EXPERIMENTAL
    WaitChart = new toResultBar(splitter);
    try
    {
        WaitChart->setSQL(toSQL::sql("toSession:SessionWait"));
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
    WaitChart->setTitle(tr("Wait states"));
    WaitChart->setYPostfix(QString::fromLatin1("ms/s"));
    WaitChart->setSamples(-1);
#ifdef TORA3_GRAPH
    WaitChart->start();
#endif
    // TODO: there is no method toResult::changeParams(QString const &, ...)
    // it was renamed to refreshWithParams(toQueryParams const&)
    connect(Statistics, SIGNAL(sessionChanged(const QString &)),
            WaitChart, SLOT(changeParams(const QString &)));

    IOChart = new toResultBar(splitter);
    try
    {
        IOChart->setSQL(toSQL::sql("toSession:SessionIO"));
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
    IOChart->setTitle(tr("I/O"));
    IOChart->setYPostfix(tr("blocks/s"));
    IOChart->setSamples(-1);
#ifdef TORA3_GRAPH
    IOChart->start();
#endif
    // TODO: there is no method toResult::changeParams(QString const &, ...)
    // it was renamed to refreshWithParams(toQueryParams const&)
    connect(Statistics, SIGNAL(sessionChanged(const QString &)),
            IOChart, SLOT(changeParams(const QString &)));
#endif
    box->setSpacing(0);
    box->setContentsMargins(0, 0, 0, 0);
    box->addWidget(splitter);
    StatTab->setLayout(box);

    ResultTab->addTab(StatTab, tr("&Statistics"));
    ResultTab->setTabEnabled(ResultTab->indexOf(StatTab), false);

    Logging = new toListView(ResultTab);
    ResultTab->addTab(Logging, tr("&Logging"));
    Logging->addColumn(tr("SQL"));
    Logging->addColumn(tr("Result"));
    Logging->addColumn(tr("Timestamp"));
    Logging->addColumn(tr("Duration"));
    Logging->setColumnAlignment(3, Qt::AlignRight);
    Logging->setSelectionMode(toTreeWidget::Single);
    connect(Logging, SIGNAL(selectionChanged(toTreeWidgetItem *)), this, SLOT(slotExecuteLog()));
    LastLogItem = NULL;

    Output = new toOutputWidget(this);
    ResultTab->addTab(Output, tr("Output"));

    connect(ResultTab, SIGNAL(currentChanged(int)),
            this, SLOT(slotChangeResult(int)));

    if (autoLoad && !toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::AutoLoad).toString().isEmpty())
    {
        Editor->editOpen(toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::AutoLoad).toString());
    }

    ToolMenu = NULL;

    try
    {
        if (connection().providerIs("Oracle"))
        {
            if (toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::StatisticsBool).toBool())
            {
                statisticAct->setChecked(true);
            }
        }
        else
            statisticAct->setEnabled(false);
    }
    TOCATCH;

    connect(this, SIGNAL(connectionChange()), this, SLOT(slotConnectionChanged()));

    connect(&Poll, SIGNAL(timeout()), this, SLOT(slotPoll()));
    connect(this, SIGNAL(connectionChange()), this, SLOT(slotChangeConnection()));

    ///setFocusProxy(Editor);

    // don't show results yet

    QList<int> list;
    list.append(1);
    list.append(0);
    EditSplitter->setSizes(list);

    QSettings s;
    int v;
    s.beginGroup("toWorksheet");
    v = s.value("EditSplitterSizes0", 10).toInt();
    if (v == 0) v = 10;
    EditSplitterSizes << v;
    v = s.value("EditSplitterSizes1", 10).toInt();
    if (v == 0) v = 10;
    EditSplitterSizes << v;
    s.endGroup();

    slotSetCaption();
}

toWorksheet::toWorksheet(QWidget *main, toConnection &connection, bool autoLoad)
    : toToolWidget(WorksheetTool, "worksheet.html", main, connection, "toWorksheet")
    , CurrentTab(NULL)
    , ResultModel(NULL)
    , lockConnectionActClicked(false)
{
    createActions();
    setup(autoLoad);
}

void toWorksheet::slotWindowActivated(toToolWidget *widget)
{
    if (!widget)
        return;

    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("Edit&or"), this);

            ToolMenu->addAction(executeAct);
            ToolMenu->addAction(executeStepAct);
            ToolMenu->addAction(executeAllAct);
            ToolMenu->addAction(stopAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(refreshAct);
            ToolMenu->addAction(parseAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(describeAct);
            ToolMenu->addAction(describeActNew);
            ToolMenu->addAction(explainAct);
            ToolMenu->addAction(statisticAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(SavedMenu->menuAction());

            ToolMenu->addAction(saveLastAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(previousAct);
            ToolMenu->addAction(nextAct);
            ToolMenu->addAction(eraseAct);

            toGlobalEventSingle::Instance().addCustomMenu(ToolMenu);
        }

        // disabled. can cause infinite loop on some window systems
        // depending on the timing.
        // Editor->setFocus();
        //        if (Editor)
        Editor->setFocus(Qt::ActiveWindowFocusReason);

        // must set correct schema every time so having two worksheets
        // on different schemas works properly.
#ifndef QT_DEBUG
        // This method is called whenever Tora gets focus,
        // this makes debugging pretty anyoning
        if (Schema)
            Schema->update();
#endif
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toWorksheet::slotConnectionChanged(void)
{
    try
    {
        delete ToolMenu;
        ToolMenu = NULL;
        slotWindowActivated(this);
    }
    TOCATCH;
}

bool toWorksheet::checkSave()
{
    using namespace ToConfiguration;

    if (!Editor->isModified())
        return true;

    if (!toConfigurationNewSingle::Instance().option(Worksheet::CheckSaveBool).toBool())
        return true;

    if (toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::AutoSaveBool).toBool() && !Editor->filename().isEmpty())
    {
        if (Utils::toWriteFile(Editor->filename(), Editor->text()))
        {
            Editor->setModified(false);
            return true;
        }
        else
        {
            return false;
        }
    }

    // CheckSave is true
    // AutoSave is false or Editor->filemame is empty
    // Display file save dialog
    QString description;
    try
    {
        description = connection().description();
    }
    catch (...)
    {
        description = QString::fromLatin1("unknown connection");
    }

    // grab focus so user can see file and decide to save
    setFocus(Qt::OtherFocusReason);

    QString str = tr("Save changes to editor for %1?").arg(description);
    if (!Editor->filename().isEmpty())
        str += QString::fromLatin1("\n(%1)").arg(Editor->filename());

    int ret = TOMessageBox::information(
                  this,
                  "Save File",
                  str,
                  QMessageBox::Save |
                  QMessageBox::Discard |
                  QMessageBox::Cancel);

    switch (ret)
    {
        case QMessageBox::Save:
            {
                // Editor->filename is empty => show filesave dialog
                if (Editor->filename().isEmpty())
                {
                    // Ask user for desired filename
                    Editor->setFilename(Utils::toSaveFilename(Editor->filename(),
                                        QString(),
                                        this));
                    // Try to save the workseet text into file
                    if (Utils::toWriteFile(Editor->filename(), Editor->text()))
                    {
                        Editor->setModified(false);
                        toGlobalEventSingle::Instance().addRecentFile(Editor->filename());
                        return true;
                    }

                    // if Editor's filename is still empty => file save dialog failed
                    if (Editor->filename().isEmpty())
                    {
                        return false;
                    }

                    // Try to save the workseet text into file
                    if (Utils::toWriteFile(Editor->filename(), Editor->text()))
                    {
                        Editor->setModified(false);
                        return true;
                    }
                    return false;
                }
                else
                {
                    if (Utils::toWriteFile(Editor->filename(), Editor->text()))
                    {
                        Editor->setModified(false);
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            break;
        case QMessageBox::Discard:
            {
                // only ever called if closing widget, make sure this
                // is not called again.
                Editor->setModified(false);
                return true;
            }
        default: // QMessageBox::Cancel
            return false;
    }
}

bool toWorksheet::slotClose()
{
    if (Statistics)
    {
        Statistics->close();
        delete Statistics;
        Statistics = NULL;
    }

    if (!checkSave())
        return false;

    Result->slotStop();
    unlockConnection();
    if (!LockedConnection)
    {
        return toToolWidget::close();
    }
    else
        return false;
}

void toWorksheet::slotSchemaChanged(const QString &)
{
    if (LockedConnection)
    {
        (*LockedConnection).SchemaInitialized = false;
        (*LockedConnection).Schema = schema();
    }
}

void toWorksheet::closeEvent(QCloseEvent *event)
{
    QSettings s;
    s.beginGroup("toWorksheet");
    s.setValue("EditSplitterSizes0", EditSplitter->sizes()[0]);
    s.setValue("EditSplitterSizes1", EditSplitter->sizes()[1]);
    s.endGroup();

    if (slotClose())
        event->accept();
    else
        event->ignore();
}

void toWorksheet::focusInEvent(QFocusEvent *e)
{
    qDebug() << ">>> toWorksheet::focusInEvent" << this;
    super::focusInEvent(e);
}

void toWorksheet::focusOutEvent(QFocusEvent *e)
{
    qDebug() << ">>> toWorksheet::focusOutEvent" << this;
    super::focusOutEvent(e);
}

void toWorksheet::handle(QObject *obj, QMenu *menu)
{
    QString name = obj->objectName();

    if (toWorksheetText* t = dynamic_cast<toWorksheetText*>(obj))
    {
        Q_UNUSED(t);
        menu->addSeparator();
        menu->addAction(executeAct);
        menu->addAction(executeStepAct);
        menu->addAction(executeAllAct);
        menu->addAction(refreshAct);

        menu->addSeparator();

        menu->addAction(describeAct);
        menu->addAction(describeActNew);
        menu->addAction(explainAct);

        menu->addSeparator();

        menu->addAction(stopAct);

        menu->addSeparator();

        menu->addAction(SavedMenu->menuAction());
        menu->addAction(InsertSavedMenu->menuAction());
        menu->addAction(saveLastAct);
	}
	else if (obj == Result)
	{
		//TODO CopyAsSQLValue
	}
}

toWorksheet::~toWorksheet()
{
}

bool toWorksheet::hasTransaction() const
{
    if (LockedConnection)
        return (*LockedConnection)->hasTransaction();
    return false;
}

void toWorksheet::commitChanges()
{
    Q_ASSERT_X(LockedConnection, qPrintable(__QHERE__), "No connection for commit");
    (*LockedConnection)->commit();
    if (!lockConnectionActClicked)
        unlockConnection();
    else
        toGlobalEventSingle::Instance().setNeedCommit(this, this->hasTransaction());
}

void toWorksheet::rollbackChanges()
{
    Q_ASSERT_X(LockedConnection, qPrintable(__QHERE__), "No connection for rollback");
    (*LockedConnection)->rollback();
    if (!lockConnectionActClicked)
        unlockConnection();
    else
        toGlobalEventSingle::Instance().setNeedCommit(this, this->hasTransaction());
}

toWorksheetText* toWorksheet::editor(void)
{
    return Editor;
}

bool toWorksheet::canHandle(const toConnection &)
{
    return true;
}

void toWorksheet::slotChangeResult(int index)
{
    QWidget *widget = ResultTab->widget(index);
    if (!widget)
        return;

    CurrentTab = widget;
    if (!m_lastQuery.sql.isEmpty())
    {
        if (CurrentTab == Plan)
            Plan->queryPlanTable(toQueryParams() << m_lastQuery.sql);
        else if (CurrentTab == ResourceSplitter)
            viewResources();
        else if (CurrentTab == Statistics && Result->running())
            Statistics->slotRefreshStats(false);
    }
}

void toWorksheet::slotRefresh(void)
{
    if (!m_lastQuery.sql.isEmpty())
        query(m_lastQuery, Normal, DontSelectQueryEnum);
    if (RefreshSeconds > 0)
    {
        RefreshTimer.setSingleShot(true);
        RefreshTimer.start(RefreshSeconds * 1000);
    }
}

bool toWorksheet::describe(toSyntaxAnalyzer::statement const& query)
{
	static QRegExp desc("\\s*DESC(R(I(B(E)?)?)?)?\\s+",  Qt::CaseInsensitive);

	if (!query.firstWord.startsWith("DESC", Qt::CaseInsensitive))
		return false;

	int pos = desc.indexIn(query.sql, 0);
	if (pos == -1)
		return false;

	Editor->gotoPosition(query.posFrom + desc.matchedLength());
	slotDescribe();
	return true;
}

QString toWorksheet::schema() const
{
    return Schema->currentText();
}

static toSQL SQLCheckMySQLRoutine("toWorksheet:CheckRoutine",
                                  "select count(1)\n"
                                  "from information_schema.routines\n"
                                  "where routine_name = :f1<char[101]>\n"
                                  "  and lower(routine_type) = :f2<char[101]>\n"
                                  "  and routine_schema = :f3<char[101]>",
                                  "Check if routine exists in MySQL",
                                  "0500",
                                  "QMYSQL");

static toSQL SQLDropMySQLRoutine("toWorksheet:DropRoutine",
                                 "drop :f1<noquote> if exists :f2<noquote>;",
                                 "Drop MySQL routine if it exists",
                                 "0500",
                                 "QMYSQL");

// MySQL does not support replacing currently existing routines. Trying to create existing
// routine raises exception. Therefore when creating a routine a previous one should be
// dropped first.
// There is a configuration option in MySQL settings section on how exactly this feature
// should behave:
//   0 - do nothing
//   1 - drop before creating
//   2 - drop before creating (if exists)
//   3 - ask
//   4 - ask (if exists)
void toWorksheet::mySQLBeforeCreate(QString &chk)
{
    // whether routine exists must checked if config is set to 2 (drop if exists) and 4 (ask if exists)
    bool check = (toConfigurationNewSingle::Instance().option(ToConfiguration::MySQL::BeforeCreateActionInt).toInt() % 2 == 0);
    // whether to ask or drop automatically if config is set to 3 (ask) and 4 (ask if exists)
    bool ask = (toConfigurationNewSingle::Instance().option(ToConfiguration::MySQL::BeforeCreateActionInt).toInt() > 2);
    bool answerYes;

    // do a "poor mans" parsing as we do not actually need to parse everything
    chk.replace("(", " ");
    chk.replace("\n", " ");
    QStringList tok = chk.split(" ", QString::SkipEmptyParts);

    // only for "create" statements
    if (tok[0] == "create")
    {
        int i;
        if (tok[1].startsWith("definer="))
            i = 1;
        else
            i = 0;

        // only for create function|procedure statements (not for create table|index etc...)
        if (tok[1 + i] == "function" || tok[1 + i] == "procedure")
        {
            int c = 1; // count of existing routines (would logically be 0 or 1)

            try
            {
                if (check)
                {
                    // Check if this routine actually exists in database
                    toQueryParams param;
                    toConnectionSubLoan conn(connection());
                    param << toQValue(tok[2 + i].remove('`')); // routine name
                    param << toQValue(tok[1 + i]); // routine type (procedure or function)
                    param << toQValue(Schema->currentText());
                    toQuery query(conn, /*toQuery::Long,*/ SQLCheckMySQLRoutine, param);
                    c = query.readValue().toInt(); // 1 - exists, 0 - does not exist
                }

                if (c == 1)
                {
                    if (ask)
                        answerYes = (TOMessageBox::information(this, tr("Drop routine?"),
                                                               tr("Do you want to drop %1 %2?").arg(tok[1 + i]).arg(tok[2 + i]),
                                                               tr("&Drop"), tr("Leave it and continue")) == 0);
                    else
                        answerYes = true;

                    if (answerYes)
                    {
                        toQueryParams param;
                        toConnectionSubLoan conn(connection());
                        param << toQValue(tok[1 + i]); // routine type (procedure or function)
                        param << toQValue(tok[2 + i]); // routine name
                        // Note that if routine creation is not successfull you will be left without any routine!
                        toQuery query(conn, /*toQuery::Long,*/ SQLDropMySQLRoutine, param);
                    }
                }
            }
            TOCATCH;
        }
    }
} // mySQLBeforeCreate

void toWorksheet::query(QString const& text, execTypeEnum execType)
{
    toSyntaxAnalyzer *analyzer = Editor->analyzer();
    toSyntaxAnalyzer::statementList stats = analyzer->getStatements(text);

    if (stats.isEmpty())
        return;

    toSyntaxAnalyzer::statement stat(stats.first());
    analyzer->sanitizeStatement(stat);
    query(stat, execType, DontSelectQueryEnum);
}

void toWorksheet::query(toSyntaxAnalyzer::statement const& statement, execTypeEnum execType, selectionModeEnum selectMode)
{
    Result->slotStop();
    RefreshTimer.stop();

    if (!Editor->hasSelectedText() || selectMode)
        Editor->setSelection(statement.posFrom, statement.posTo);

    TLOG(0, toDecorator, __HERE__)
            << "Current statement: " << std::endl
            << statement.sql.toStdString() << std::endl;

    if (statement.firstWord.trimmed().isEmpty())
        return;

    // Imitate something like "create or replace" syntax for MySQL
    //if (connection().providerIs("QMYSQL") && code && toConfigurationNewSingle::Instance().createAction() > 0)
    //mySQLBeforeCreate(chk);

    if (describe(statement))
        return;

    QWidget *curr = ResultTab->currentWidget();
    Current->hide();
    Result->show();
    Current = Result;
    if (curr == Columns)
        ResultTab->setCurrentIndex(ResultTab->indexOf(Result));

    if (connection().providerIs("Oracle") && statement.firstWord.startsWith("EXEC", Qt::CaseInsensitive))
    {
        Utils::toStatusMessage("Ignoring SQL*Plus command", true);
        // todo handle leading spaces any comments here
        // put exec in anonymous plsql block or they won't work
        //m_lastQuery = m_lastQuery.sql.trimmed().right(m_lastQuery.sql.length() - m_lastQuery.firstWord.length())
        //m_lastQuery = QString("BEGIN\n%1;\nEND;").arg(m_lastQuery);
        return;
    }

    if (statement.statementType == toSyntaxAnalyzer::SQLPLUS)
    {
        QString t = tr("Ignoring SQL*Plus command");
        slotFirstResult(statement.sql, toConnection::exception(t), false);
        Utils::toStatusMessage(t, true);
        return ;
    }

    Time.start(); // Setup query duration timer
    m_FirstDataReceived = false;

    switch (execType)
    {
        case OnlyPlan:
            {
                if (ResultTab->currentIndex() != ResultTab->indexOf(Plan))
                    ResultTab->setCurrentIndex(ResultTab->indexOf(Plan));
                Plan->query(statement.sql, toQueryParams() << statement.sql);
                slotUnhideResults();
            }
            break;
        case Parse:
            {
                try
                {
                    //parser connection().parse(QueryString);
                }
                catch (const QString &exc)
                {
                    addLog(exc);
                }
            }
            break;
            // This is in fact Batch Execution
        case Direct:
            {
                try
                {
                    ////lockConnection();
                    QString buffer;
                    if (!toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::HistoryErrorBool).toBool())
                    {
                        toConnectionSubLoan conn(connection(), schema());
                        toQuery query(conn, statement.sql, toQueryParams());
                        if (query.rowsProcessed() > 0)
                            buffer = tr("%1 rows processed").arg((int)query.rowsProcessed());
                        else
                            buffer = tr("Query executed");
                    }
                    else
                    {
                        throw QString("TODO: rewrite unreadable code: %1").arg(__QHERE__);
                        // This code has relation to WorksheetSetup UI: checkbox Save Previous Results
                        // This should open a new Result tab for each query exec.
#ifdef TORA3_SOMETHING_UGLY
                        toResultView *statement = new toResultView(Current->parentWidget());

                        try
                        {
                            statement->statement((QString) m_lastQuery, param);
                            if (statement->statement() && statement->statement()->rowsProcessed() > 0)
                                buffer = tr("%1 rows processed").arg((int)statement->statement()->rowsProcessed());
                            else
                                buffer = tr("Query executed");
                            Current->hide();
                            Current = statement;
                            Current->show();
                        }
                        catch (...)
                        {
                            delete statement;
                            throw;
                        }
#endif
                    }

                    slotFirstResult(statement.sql, toConnection::exception(buffer), false);
                }
                catch (const QString &exc)
                {
                    addLog(exc);
                    if (QMessageBox::question(this, tr("Direct Execute Error"),
                                              exc + "\n\n" + tr("Stop execution ('No' to continue)?"),
                                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                        throw BatchExecException();
                    if (!lockConnectionActClicked)
                        unlockConnection();
                }
            }
            break;
        case Normal:
            {
                //if (statement.statementType != toSyntaxAnalyzer::SELECT) TODO: check for "FOR UPDATE HERE
                {
                    lockConnection();
                    lockConnectionAct->setDisabled(true);
                }
                this->m_lastQuery = statement;

                // unhide the results pane if there's something to show
                if (m_lastQuery.statementType == toSyntaxAnalyzer::SELECT)
                {
                    ResultTab->setCurrentWidget(Result);
                    slotUnhideResults();
                }

                if (m_lastQuery.statementType == toSyntaxAnalyzer::PLSQL && outputAct->isChecked())
                {
                    ResultTab->setCurrentWidget(Output);
                    slotUnhideResults();
                }

                toQueryParams param;
                if (m_lastQuery.statementType == toSyntaxAnalyzer::SELECT || m_lastQuery.statementType == toSyntaxAnalyzer::DML)
                {
                    try
                    {
                        param = toParamGet::getParam(connection(), this, m_lastQuery.sql);
                    }
                    catch (...)
                    {
                        return ;
                    }
                }
                Utils::toStatusMessage(tr("Processing query"), true, false);

                stopAct->setEnabled(true);
                Poll.start(1000);
                Started->setToolTip(tr("Duration while query has been running\n\n") + statement.sql);
                stopAct->setEnabled(true);
                Result->setNumberColumn(toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::DisplayNumberColumnBool).toBool());

#if 0
                // Do not understand context if this issue
                // it fixes crash running statements from Schema Browser - PV
                if (ResultTab)
                    ResultTab->setCurrentIndex(0);
#endif

                try
                {
                    saveHistory();
                    Result->removeSQL();
                    if (LockedConnection)
                        Result->querySub(LockedConnection, m_lastQuery.sql, param);
                    else
                        Result->query(m_lastQuery.sql, param);

                    if (CurrentTab)
                    {
                        // todo
                        // PV - let's open really required tab for called action
                        // e.g. Plan for explainplan, Result for run statement action etc.
                        // It stops to really run a statement when I expect explain plan
                        // if (CurrentTab == Plan)
                        // Plan->query(QueryString);
                        // else
                        if (CurrentTab == ResourceSplitter)
                            viewResources();
                    }
                }
                catch (const toConnection::exception &exc)
                {
                    addLog(exc);
                    if (!lockConnectionActClicked)
                        unlockConnection();
                }
                catch (const QString &exc)
                {
                    addLog(exc);
                    if (!lockConnectionActClicked)
                        unlockConnection();
                }
                Result->setSQLName(statement.sql.simplified().left(40));
            }
            break;
    }
}

void toWorksheet::querySelection(execTypeEnum execType)
{
    toSyntaxAnalyzer *analyzer = Editor->analyzer();

    int lineFrom, indexFrom, lineTo, indexTo;
    Editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

    if (indexTo == 0)
        lineTo = (std::max)(lineFrom, lineTo-1);
    toSyntaxAnalyzer::statement stat(lineFrom, lineTo);
    analyzer->sanitizeStatement(stat);
    query(stat, execType, DontSelectQueryEnum);
}


void toWorksheet::saveHistory(void)
{
#if 0                           // todo
    if (WorksheetTool.config(CONF_HISTORY, "").isEmpty())
        return ;
    if (Result->firstChild() && Current == Result)
    {
        History[LastID] = Result;
        Result->hide();
        Result->slotStop();
        disconnect(Result, SIGNAL(done(void)), this, SLOT(slotQueryDone(void)));
        disconnect(Result, SIGNAL(firstResult(const QString &, const toConnection::exception &, bool)),
                   this, SLOT(slotFirstResult(const QString &, const toConnection::exception &, bool)));
        disconnect(stopAct, SIGNAL(clicked(void)), Result, SLOT(slotStop(void)));

        Result = new toResultTableView(Result->parentWidget());
        if (statisticAct->isChecked())
            slotEnableStatistic(true);
        Result->show();
        Current = Result;
        connect(stopAct, SIGNAL(clicked(void)), Result, SLOT(slotStop(void)));
        connect(Result, SIGNAL(done(void)), this, SLOT(slotQueryDone(void)));
        connect(Result, SIGNAL(firstResult(const QString &, const toConnection::exception &, bool)),
                this, SLOT(slotFirstResult(const QString &, const toConnection::exception &, bool)));
    }
#endif
}

QString toWorksheet::duration(int dur, bool hundreds)
{
    char buf[100];
    if (dur >= 3600000)
    {
        if (hundreds)
            sprintf(buf, "%d:%02d:%02d.%02d", dur / 3600000, (dur / 60000) % 60, (dur / 1000) % 60, (dur / 10) % 100);
        else
            sprintf(buf, "%d:%02d:%02d", dur / 3600000, (dur / 60000) % 60, (dur / 1000) % 60);
    }
    else
    {
        if (hundreds)
            sprintf(buf, "%d:%02d.%02d", dur / 60000, (dur / 1000) % 60, (dur / 10) % 100);
        else
            sprintf(buf, "%d:%02d", dur / 60000, (dur / 1000) % 60);
    }
    return QString::fromLatin1(buf);
}

void toWorksheet::slotFirstResult(const QString &sql,
                                  const toConnection::exception &result,
                                  bool error)
{
    // Stop ticking clock
    // TODO: resume (un-pause) the clock when read-all is executed in Model
    Poll.stop();

    m_FirstDataReceived = true;

    if (error && result.offset() >= 0 && toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::MoveToErrorBool).toBool())
        Editor->setCursorPosition(m_lastQuery.lineFrom + result.line() - 1, result.column() - 1);

    if (error && LockedConnection && (*LockedConnection)->isBroken())
        unlockConnection();

    if (!error)
    {
        if (ResultTab)
            slotChangeResult(ResultTab->indexOf(CurrentTab));

        try
        {
            // TODO: DDL commit/rollback on non-Oracle DBs
            // TODO: move this into queryDone
            if (m_lastQuery.statementType == toSyntaxAnalyzer::DML)
            {
                if (toConfigurationNewSingle::Instance().option(ToConfiguration::Database::AutoCommitBool).toBool())
                {
                    Q_ASSERT_X(LockedConnection, qPrintable(__QHERE__), "Connection was not lent for DML");
                    (*LockedConnection)->commit();
                }
                else
                    toGlobalEventSingle::Instance().setNeedCommit(this, this->hasTransaction());
            }
        }
        TOCATCH;

        QString str = result;
        str += "\n" + tr("(Duration ") + duration(Time.elapsed()) + ")";
        Utils::toStatusMessage(str, false, false);
    }

    addLog(result);

    saveDefaults();
}

void toWorksheet::slotLastResult(const QString &message, bool error)
{
    Utils::toStatusMessage(message, false, false);
}

void toWorksheet::slotUnhideResults(const QString &,
                                    const toConnection::exception &,
                                    bool error)
{
    if (!error && Result->model()->rowCount() > 0)
        slotUnhideResults();
}

void toWorksheet::slotUnhideResults()
{
    // move splitter if currently hidden
    if (EditSplitter->sizes()[1] == 0)
        EditSplitter->setSizes(EditSplitterSizes);
}

void toWorksheet::slotExecute()
{
    if (Editor->hasSelectedText())
    {
        querySelection(Normal);
        return;
    }
    toSyntaxAnalyzer::statement stat = Editor->currentStatement();
    query(stat, Normal);
}

void toWorksheet::slotExplainPlan()
{
    if (Editor->hasSelectedText())
    {
        querySelection(OnlyPlan);
        return ;
    }
    toSyntaxAnalyzer::statement stat = Editor->currentStatement();
    query(stat, OnlyPlan);
}

void toWorksheet::slotExecuteStep()
{
    toSyntaxAnalyzer::statement stat = Editor->currentStatement();
    query(stat, Normal);
}

void toWorksheet::slotToggleStatistic(void)
{
    statisticAct->toggle();
}


void toWorksheet::slotExecuteAll()
{
    /* TODO get analyzer from Editor->editor() */
    toSyntaxAnalyzerNL analyzer(Editor);
    toSyntaxAnalyzer::statementList stats = analyzer.getStatements(Editor->text());

    int cpos, cline, lastLinePos;
    Editor->getCursorPosition(&cline, &cpos);

    QProgressDialog dialog(tr("Executing all statements"),
                           tr("Cancel"),
                           0,
                           Editor->lines(),
                           this);
    Q_FOREACH(toSyntaxAnalyzer::statement stat, stats)
    {
        dialog.setValue(stat.lineFrom);
        qApp->processEvents();
        if (dialog.wasCanceled())
            break;

        if ( stat.lineTo < cline)
            continue;

        analyzer.sanitizeStatement(stat);

        try
        {
            query(stat, Direct, DontSelectQueryEnum);
        }
        catch ( BatchExecException const& e)
        {
            Q_UNUSED(e);
            break;
        }

        if (Current)
            if (toResultView *last = dynamic_cast<toResultView *>(Current))
                if (toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::ExecLogBool).toBool())
                    if (last->firstChild())
                        History[LastID] = last;
        lastLinePos = stat.lineTo;
    }

    Editor->setSelection(cline, 0, lastLinePos+1, 0);
}

void toWorksheet::slotParse()
{
    Utils::toBusy busy;
    toSyntaxAnalyzer::statement stat = Editor->currentStatement();
    toSyntaxAnalyzer *analyzer = Editor->analyzer();
    analyzer->sanitizeStatement(stat);

    if (!connection().providerIs("Oracle")) // so far no support for other DBs
        return;
    if (stat.statementType == toSyntaxAnalyzer::DML || stat.statementType == toSyntaxAnalyzer::SELECT)
    {
        toQList vals = toQuery::readQuery(connection(), SQLParseSql, toQueryParams() << stat.sql);
        int parseOffset = vals.front().toInt();
        if (parseOffset < 0)
            return;

        int pos = stat.posFrom;
        pos = Editor->positionAfter(pos, parseOffset);
        Editor->gotoPosition(pos);
        TLOG(1, toDecorator, __HERE__) << "Parse offset:" << parseOffset << std::endl;
    }
}

void toWorksheet::slotEraseLogButton()
{
    Logging->clear();
    LastLogItem = NULL;
    for (std::map<int, QWidget *>::iterator i = History.begin(); i != History.end(); i++)
        delete(*i).second;
    History.clear();
}

void toWorksheet::slotQueryDone(void)
{
    stopAct->setDisabled(true);

    // Possibly the toConnectionSub.Schema got changed after ~toQuery
    // could be possible if something like:
    //   BEGIN
    //     EXECUTE IMMEDIATE 'ALTER SESSION SET CURRENT_SCHEMA=ABC';
    //   END;
    // was executed
    if (LockedConnection && !(*LockedConnection)->schema().isEmpty() && (*LockedConnection)->schema() != schema())
    {
        Schema->setSelected((*LockedConnection)->schema());
        Schema->refresh();
    }
    // TODO: LockedConnection.isNull is mandatory here. For some unknown reason slotQueryDone can be called twice for the same query
    if (!lockConnectionActClicked && LockedConnection)
    {
        try
        {
            Utils::toBusy busy;
            if ( !(*LockedConnection)->hasTransaction())
                unlockConnection();
        }
        TOCATCH
    }
    lockConnectionAct->setEnabled(true);
}

void toWorksheet::saveDefaults(void)
{
#if 0                           // todo
    toTreeWidgetItem *item = Result->firstChild();
    if (item)
    {
        QTreeWidgetItem *head = Result->headerItem();
        for (int i = 0; i < Result->columns(); i++)
        {
            toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(item);
            QString str;
            if (resItem)
                str = resItem->allText(i);
            else if (item)
                str = item->text(i);

            try
            {
                toParamGet::setDefault(connection(), head->text(i).lower(), toUnnull(toQValue(str)));
            }
            TOCATCH
        }
    }
#endif
}

#define ENABLETIMED "ALTER SESSION SET TIMED_STATISTICS = TRUE"
#define DISABLETIMED "ALTER SESSION SET TIMED_STATISTICS = FALSE"

void toWorksheet::slotEnableStatistic(bool ena)
{
    if (ena)
    {
        ResultTab->setTabEnabled(ResultTab->indexOf(StatTab), true);
        statisticAct->setChecked(true);
        Statistics->clear();
        if (toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::TimedStatsBool).toBool())
        {
            ///connection().setInit("STATISTICS", QString::fromLatin1(ENABLETIMED));
        }
    }
    else
    {
        ///connection().setInit("STATISTICS", QString::fromLatin1(DISABLETIMED));
        ResultTab->setTabEnabled(ResultTab->indexOf(StatTab), false);
        statisticAct->setChecked(false);
    }
}

void toWorksheet::slotDescribe(void)
{
    toCache::ObjectRef table;
    Editor->tableAtCursor(table);
    table.context = schema();
    table.first = connection().getTraits().unQuote(table.first);
    table.second = connection().getTraits().unQuote(table.second);

    if (toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::ToplevelDescribeBool).toBool())
    {
        toDescribe * d = new toDescribe(this);
        d->changeParams(table); // this also calls QWidget::show()
    }
    else
    {
        slotUnhideResults();
        Columns->changeObject(table);
        Columns->show();
        Current = Columns;
        ResultTab->setTabEnabled(ResultTab->indexOf(Columns), true);
        ResultTab->setCurrentIndex(ResultTab->indexOf(Columns));
    }
}

void toWorksheet::slotDescribeNew(void)
{
    toSyntaxAnalyzer::statement stat = Editor->currentStatement();
    TOMessageBox::information(this, Utils::toSQLToSql_Id(stat.sql), stat.sql);

    TLOG(1, toDecorator, __HERE__) << "sql_id lf: " << Utils::toSQLToSql_Id(stat.sql) << std::endl;

#ifdef TORA_EXPERIMENTAL
    int line, col;
    QString buffer;
    QString firstWord, currentWord;
    QString txt = Editor->text();
    toSyntaxAnalyzer::statement currentStat = Editor->currentStatement();
    Editor->getCursorPosition(&line, &col);
    TLOG(1, toDecorator, __HERE__) << "describe: "
                                   << '[' << line << ',' << col << ']'
                                   << "--------------------------------------------------------------------------------" << std::endl
                                   << txt.toStdString()
                                   << std::endl;

    try
    {
        static QSet<QString> PLSQL_INTRODUCERS
        {
            "ANALYZE",            "BEGIN",            "CALL",
            "COMMIT",            "CREATE",            "DECLARE",
            "DROP",            "EXPLAIN",            "FUNCTION",
            "GRANT",            "LOCK",            "PACKAGE",
            "PACKAGE",            "PROCEDURE",            "ROLLBACK",
            "SAVEPOINT",            "SET",            "TRUNCATE"
        };
        static QSet<QString> DML_INTRODUCERS
        {
            "WITH", "SELECT", "INSERT", "UPDATE", "DELETE", "MERGE"
        };

        std::unique_ptr <SQLParser::Statement> stmt;
        std::unique_ptr <SQLLexer::Lexer> lexer = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", currentStat.sql, "");
        firstWord = lexer->firstWord();
        currentWord = lexer->wordAt(SQLLexer::Position(line, col));

        if (PLSQL_INTRODUCERS.contains(firstWord.toUpper()))
        {
            std::cout << "PLSQL:" << std::endl;
            stmt = StatementFactTwoParmSing::Instance().create("OraclePLSQL", txt, "");
            std::cout << stmt->root()->toStringRecursive().toStdString() << std::endl;
        }
        else if (DML_INTRODUCERS.contains(firstWord.toUpper()))
        {
            std::cout << "SQL:" << std::endl;
            stmt = StatementFactTwoParmSing::Instance().create("OracleDML", txt, "");
            std::cout << stmt->root()->toStringRecursive().toStdString() << std::endl;
        }
        else
        {
            std::cout << "Unknown:" << firstWord.QString::toStdString() << std::endl;
            throw QString("Unknown word %1").arg(firstWord);
        }

        SQLParser::Position cursor(line + 1, col);
        SQLParser::Statement::token_const_iterator currentToken = stmt->begin();
        for (SQLParser::Statement::token_const_iterator j = stmt->begin(); j != stmt->end(); ++j)
        {
            if ( j->getPosition() > cursor)
                break;
            if ( j->getPosition() == cursor)
            {
                currentToken = j;
                break;
            }
            if (j->isLeaf())
                currentToken = j;
        }

        SQLParser::Statement::token_const_iterator_to_root stackPath(&*currentToken);
        QTextStream buf(&buffer);

        buf << "Cursor position: " << cursor.toString() << "\n"
            << "Detail for the token: " << currentToken->toString() << "\n"
            << "Positon: " << currentToken->getPosition().toString() << "\n"
            << "Token AType: " << currentToken->getTokenATypeName() << '\n'
//            << "Token Type: " << currentToken->getTokenTypeString() << '\n'
            ;

        buf << "AST path:\n";

        QList<QString> stack;
        while (stackPath->parent())
        {
            QString t = stackPath->toString() + '(' + stackPath->getTokenATypeName() + ')';
            stack.push_back(t);
            stackPath++;
        }

        buf << "Token depth: " << stack.size() << '\n';
        QString padding;
        while (!stack.empty())
        {
            buffer += padding + stack.takeLast() + '\n';
            padding += "  ";
        }
        buf << '\n';

        switch (currentToken->getTokenType())
        {
            case SQLParser::Token::L_TABLEALIAS:
            case SQLParser::Token::S_SUBQUERY_NESTED:
            case SQLParser::Token::L_SUBQUERY_ALIAS:
                QList<const SQLParser::Token*> d = stmt->declarations(currentToken->toString());
                while (!d.empty())
                {
                    const SQLParser::Token *alias = d.takeFirst();
                    buf << "Table alias: " << currentToken->toString() << " => " << '\n'
                        << alias->toStringRecursive() << '\n';
                }

                SQLParser::Token const* e = stmt->translateAlias (currentToken->toString(), &*currentToken);
                if (e)
                {
                	buf << currentToken->toString() << " is alias for: " << e->toStringRecursive();
                }
                break;
        }

    }
    catch ( SQLParser::ParseException const &)
    {
        buffer = "Parser error\n";
    }
    catch ( QString const& e)
    {
        buffer = "Parser error: " + e;
    }
    TOMessageBox::information (this, currentWord, buffer );
    //currentToken->parent()->toStringRecursive() );

    return;
#endif
}

void toWorksheet::slotExecuteSaved(QAction *act)
{
    QString sql = act->data().toString();
    if (!sql.isEmpty())
    {
        try
        {
            query(sql, Normal);
        }
        TOCATCH;
    }
}

void toWorksheet::slotInsertSaved(QAction *act)
{
    QString sql = act->data().toString();
    if (!sql.isEmpty())
    {
        Editor->setFocus();
        Editor->insertAndSelect(sql, false);
    }
}

void toWorksheet::slotShowSaved()
{
    SavedMenu->clear();

    QSettings settings;
    settings.beginGroup("toWorksheet");

    QList<QVariant> statements = settings.value("sql").toList();
    QAction *last = 0;
    foreach(QVariant v, statements)
    {
        QAction *a = new QAction(v.toString().left(50), SavedMenu);
        a->setData(v.toString());
        SavedMenu->insertAction(last, a);
        last = a;
    }
}

void toWorksheet::slotShowInsertSaved()
{
    InsertSavedMenu->clear();

    QSettings settings;
    settings.beginGroup("toWorksheet");

    QList<QVariant> statements = settings.value("sql").toList();
    QAction *last = 0;
    foreach(QVariant v, statements)
    {
        QAction *a = new QAction(v.toString().left(50), InsertSavedMenu);
        a->setData(v.toString());
        InsertSavedMenu->insertAction(last, a);
        last = a;
    }
}


void toWorksheet::slotRemoveSaved(QAction *action)
{
    QSettings settings;
    settings.beginGroup("toWorksheet");

    QList<QVariant> statements = settings.value("sql").toList();
    statements.removeAll(action->data().toString());
    settings.setValue("sql", statements);
}


void toWorksheet::slotSaveLast()
{
    if (m_lastQuery.firstWord.isEmpty())
    {
        TOMessageBox::warning(this, tr("No SQL to save"),
                              tr("You haven't executed any SQL yet"),
                              tr("&Ok"));
        return;
    }

    QSettings settings;
    settings.beginGroup("toWorksheet");

    QString sql = m_lastQuery.sql.trimmed();
    if (!sql.endsWith(";"))
        sql += ";";

    QList<QVariant> statements = settings.value("sql").toList();
    if (statements.indexOf(sql) > -1)
        return;                 // already in list

    statements.append(sql);
    settings.setValue("sql", statements);
}

void toWorksheet::insertStatement(const QString &str)
{
    QString txt = Editor->text();

    int i = txt.indexOf(str);

    if (i >= 0)
    {
        int startCol, endCol;
        int startRow, endRow;

        Editor->findPosition(i, startRow, startCol);
        Editor->findPosition(i + str.length(), endRow, endCol);

        if (endCol < Editor->text(endRow).size())
        {
            if (Editor->text(endRow).at(endCol) == ';')
                endCol++;
        }
        Editor->setSelection(startRow, startCol, endRow, endCol);
    }
    else
    {
        QString t = str;
        if (str.right(1) != ";")
        {
            t += ";";
        }

        Editor->insertAndSelect(t, true);
    }
}

void toWorksheet::slotExecutePreviousLog(void)
{
    Result->slotStop();

    saveHistory();

    toTreeWidgetItem *item = Logging->currentItem();
    if (item)
    {
        toTreeWidgetItem *pt = Logging->firstChild();
        while (pt && pt->nextSibling() != item)
            pt = pt->nextSibling();

        if (pt)
            Logging->setSelected(pt, true);
    }
}

void toWorksheet::slotExecuteLog(void)
{
    Result->slotStop();

    saveHistory();

    toTreeWidgetItem *ci = Logging->currentItem();
    toResultViewItem *item = dynamic_cast<toResultViewItem *>(ci);
    if (item)
    {
        insertStatement(item->allText(0));

        if (item->text(4).isEmpty())
        {
            if (toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::ExecLogBool).toBool())
            {
                query(item->allText(0), Normal);
            }
        }
        else
        {
            std::map<int, QWidget *>::iterator i = History.find(item->text(4).toInt());
            slotChangeResult(ResultTab->currentIndex());
            if (i != History.end() && (*i).second)
            {
                Current->hide();
                Current = (*i).second;
                Current->show();
            }
        }
    }
}

void toWorksheet::slotExecuteNextLog(void)
{
    Result->slotStop();

    saveHistory();

    toTreeWidgetItem *item = Logging->currentItem();
    if (item && item->nextSibling())
    {
        toResultViewItem *next = dynamic_cast<toResultViewItem *>(item->nextSibling());
        if (next)
            Logging->setSelected(next, true);
    }
}

void toWorksheet::slotPoll(void)
{
    Started->setText(duration(Time.elapsed(), false));
}

void toWorksheet::slotSaveStatistics(void)
{
    std::map<QString, QString> stmt;
#ifdef TORA3_SESSION
    Statistics->exportData(stmt, "Stat");
    IOChart->exportData(stmt, "IO");
    WaitChart->exportData(stmt, "Wait");

    if (Plan->firstChild())
        Plan->exportData(stmt, "Plan");
    else
        Utils::toStatusMessage(tr("No plan available to save"), false, false);
    stmt["Description"] = m_lastQuery;

    toWorksheetStatistic::saveStatistics(stmt);
#endif
}

#ifdef TORA3_SESSION
void toWorksheet::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    Editor->exportData(data, prefix + ":Edit");
    if (statisticAct->isChecked())
        data[prefix + ":Stats"] = Refresh->currentText();
    toToolWidget::exportData(data, prefix);
}

void toWorksheet::importData(std::map<QString, QString> &data, const QString &prefix)
{
    Editor->importData(data, prefix + ":Edit");
    QString stmt = data[prefix + ":Stats"];
    if (!stmt.isNull())
    {
        for (int i = 0; i < Refresh->count(); i++)
        {
            if (Refresh->itemText(i) == stmt)
            {
                Refresh->setCurrentIndex(i);
                break;
            }
        }
        statisticAct->setChecked(true);
    }
    else
        statisticAct->setChecked(false);

    toToolWidget::importData(data, prefix);
    slotSetCaption();
}
#endif

void toWorksheet::slotSetCaption(void)
{
    QString name = WorksheetTool.name();

    QString filename;
    if (!Editor->filename().isNull() && !Editor->filename().isEmpty())
    {
        QFileInfo file(Editor->filename());
        filename = file.fileName();
    }
    else
        filename = "Untitled";

    name += (Editor->isModified() ?
             QString(" - *") :
             QString(" - ")) + filename;
    toToolWidget::setCaption(name);
}

toToolWidget* toWorksheet::fileWorksheet(const QString &file)
{
    toWorksheet *worksheet = static_cast<toWorksheet*>(WorksheetTool.createWindow());
    worksheet->editor()->openFilename(file);
    worksheet->slotSetCaption();

    return worksheet;
}

void toWorksheet::slotRefreshSetup(void)
{
    bool ok = false;
    int num = QInputDialog::getInt(this,
                                   tr("Enter refreshrate"),
                                   tr("Refresh rate of query in seconds"),
                                   RefreshSeconds,
                                   0,
                                   1000000,
                                   1,
                                   &ok);
    if (ok)
    {
        RefreshSeconds = num;
        RefreshTimer.start(num * 1000);
    }
    else
        RefreshTimer.stop();
}

void toWorksheet::slotStop(void)
{
    RefreshTimer.stop();
    Result->slotStop();
}

void toWorksheet::slotChangeConnection(void)
{
    if (!lockConnectionActClicked)
        unlockConnection();
    lockConnectionAct->setEnabled(true);
}

void toWorksheet::slotLockConnection(bool enabled)
{
    if (enabled)
    {
        lockConnection();
    } else {
        unlockConnection();
    }
    lockConnectionActClicked = enabled;
}

void toWorksheet::slotEnableOutput(bool enabled)
{
    if(enabled)
    {   // DDBMS_OUTPUT works with locked connection only
        lockConnectionAct->setChecked(true); // this emits signal to slotLockConnection see @ref unlockConnection for opposite action

        Output->setConnection(LockedConnection);
    }
    Output->setEnabled(enabled);
}

void toWorksheet::slotRefreshModel(class toResultModel *model)
{
	ResultModel = model;
	if (ResultModel)
		connect(ResultModel, SIGNAL(lastResult(const QString &, bool)), this, SLOT(slotLastResult(const QString &, bool)));
}

void toWorksheet::lockConnection()
{
    if (LockedConnection) // Do not lock connection twice
        return;

    try
    {
        QSharedPointer<toConnectionSubLoan> conn(new toConnectionSubLoan(connection(), schema()));
        this->LockedConnection = conn;

        //Utils::toBusy busy;
        //toQuery schema(*LockedConnection, toSQL::string("Global:CurrentSchema", connection()), toQueryParams());
        //QString value = schema.readValue();
        //Schema->setSelected(value);
        //Schema->refresh();
        //connection().setDefaultSchema(value);

        bool oldVal = lockConnectionAct->blockSignals(true);
        lockConnectionAct->setChecked(true);
        lockConnectionAct->blockSignals(oldVal);
    }
    catch (const QString &str)
    {
        bool oldVal = lockConnectionAct->blockSignals(true);
        lockConnectionAct->setChecked(false);
        lockConnectionAct->blockSignals(oldVal);
        Utils::toStatusMessage(str);
    }

}

void toWorksheet::unlockConnection()
{
    // If User refused to unlock
    if (!checkUnlockConnection())
    {
        bool oldVal = lockConnectionAct->blockSignals(true);
        lockConnectionAct->setChecked(true);
        lockConnectionAct->blockSignals(oldVal);


        return;
    }

    // Release connection from child toOutputWidget 1st
    outputAct->setChecked(false);

    this->LockedConnection.clear();
    lockConnectionActClicked = false;
    bool oldVal = lockConnectionAct->blockSignals(true);
    lockConnectionAct->setChecked(false);
    lockConnectionAct->setEnabled(true);
    lockConnectionAct->blockSignals(oldVal);

    toGlobalEventSingle::Instance().setNeedCommit(this, false);
}

bool toWorksheet::checkUnlockConnection()
{
    try
    {
        if (!this->hasTransaction())
            return true;

        QString str = tr("Commit work in session?");
        switch (TOMessageBox::warning(this,
                                      tr("Commit work?"),
                                      str,
                                      tr("&Commit"),
                                      tr("&Rollback"),
                                      tr("Cancel")))
        {
            case 0:
                (*LockedConnection)->commit();
                return true;
            case 1:
                (*LockedConnection)->rollback();
                return true;
            case 2:
                return false;
        }
    }
    catch (const QString &str)
    {
        Utils::toStatusMessage(str);
        if ((*LockedConnection)->isBroken() || !(*LockedConnection)->hasTransaction())
            return true;
    }
    return false;
}

void toWorksheet::addLog(const QString &result)
{
    using namespace ToConfiguration;
    QString dur = duration(Time.elapsed());
    QString now = QDateTime::currentDateTime().toString(Qt::SystemLocaleDate);
    toResultViewItem *item = NULL;

    if (!toConfigurationNewSingle::Instance().option(Worksheet::LogMultiBool).toBool())
    {
        if (!toConfigurationNewSingle::Instance().option(Worksheet::LogAtEndBool).toBool())
            item = new toResultViewItem(Logging, NULL);
        else
            item = new toResultViewItem(Logging, LastLogItem);
    }
    else if (!toConfigurationNewSingle::Instance().option(Worksheet::LogAtEndBool).toBool())
        item = new toResultViewMLine(Logging, NULL);
    else
        item = new toResultViewMLine(Logging, LastLogItem);

    LastLogItem = item;
    item->setText(0, m_lastQuery.sql);
    item->setText(1, result);
    item->setText(2, now);
    item->setText(3, dur);
    if (toConfigurationNewSingle::Instance().option(ToConfiguration::Worksheet::HistoryErrorBool).toBool())
        item->setText(4, QString::number(LastID));

    toResultViewItem *citem= dynamic_cast<toResultViewItem *>(Logging->currentItem());
    if (!citem || citem->allText(0) != m_lastQuery.sql)
    {
        bool oldState = Logging->blockSignals(true);
        Logging->setSelected(item, true);
        Logging->ensureItemVisible(item);
        Logging->blockSignals(oldState);
    }
}

void toWorksheet::queryStarted(const toSyntaxAnalyzer::statement &stat)
{

}

toWorksheetSetting::toWorksheetSetting(toTool *tool, QWidget* parent, const char* name)
    : QWidget(parent)
    , toSettingTab("worksheet.html#preferences")
    , Tool(tool)
{

    setupUi(this);
    toSettingTab::loadSettings(this);
}


void toWorksheetSetting::saveSetting(void)
{
    toSettingTab::saveSettings(this);
}


void toWorksheetSetting::slotChooseFile(void)
{
    QString str = Utils::toOpenFilename(AutoLoad->text(), QString(), this);
    if (!str.isEmpty())
        AutoLoad->setText(str);
}
