
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

#include "tochangeconnection.h"
#include "toconf.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "toparamget.h"
#include "toresultbar.h"
#include "toresultcols.h"
#include "toresultschema.h"
#include "toresultplan.h"
#include "toresultresources.h"
#include "toresultstats.h"
#include "toresultview.h"
#include "toresulttableview.h"
#include "tosession.h"
#include "tosgatrace.h"
#include "totabwidget.h"
#include "totool.h"
#include "totabwidget.h"
#include "tovisualize.h"
#include "toworksheet.h"
#include "ui_toworksheetsetupui.h"
#include "toworksheetstatistic.h"
#include "toworksheettext.h"
#include "toeditablemenu.h"
#include "todescribe.h"

#include <qcheckbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfileinfo.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include "totreewidget.h"
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <QMdiArea>

#include <QList>
#include <QKeyEvent>
#include <QString>
#include <QVBoxLayout>
#include <QProgressDialog>
#include <QDateTime>
#include <QSettings>
#include <QDebug>

#include "icons/clock.xpm"
#include "icons/recall.xpm"
#include "icons/describe.xpm"
#include "icons/eraselog.xpm"
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

toWorksheetSetup::toWorksheetSetup(toTool *tool, QWidget* parent, const char* name)
        : QWidget(parent), toSettingTab("worksheet.html#preferences"), Tool(tool)
{

    setupUi(this);
    AutoSave->setChecked(toConfigurationSingle::Instance().wsAutoSave());
    CheckSave->setChecked(toConfigurationSingle::Instance().wsCheckSave());
    LogAtEnd->setChecked(toConfigurationSingle::Instance().wsLogAtEnd());
    LogMulti->setChecked(toConfigurationSingle::Instance().wsLogMulti());
    MoveToError->setChecked(toConfigurationSingle::Instance().wsMoveToErr());
    Statistics->setChecked(toConfigurationSingle::Instance().wsStatistics());
    TimedStatistics->setChecked(toConfigurationSingle::Instance().wsTimedStats());
    History->setChecked(toConfigurationSingle::Instance().wsHistory());
    DisplayNumber->setChecked(toConfigurationSingle::Instance().wsNumber());
    ToplevelDescribe->setChecked(toConfigurationSingle::Instance().wsToplevelDescribe());
    DefaultFile->setText(toConfigurationSingle::Instance().wsAutoLoad());
    ExecLog->setChecked(toConfigurationSingle::Instance().wsExecLog());

#ifdef TO_NO_ORACLE
    TimedStatistics->hide();
    MoveToError->hide();
    Statistics->hide();
#endif
}


void toWorksheetSetup::saveSetting(void)
{
    toConfigurationSingle::Instance().setWsAutoSave(AutoSave->isChecked());
    toConfigurationSingle::Instance().setWsCheckSave(CheckSave->isChecked());
    toConfigurationSingle::Instance().setWsLogAtEnd(LogAtEnd->isChecked());
    toConfigurationSingle::Instance().setWsLogMulti(LogMulti->isChecked());
    toConfigurationSingle::Instance().setWsToplevelDescribe(ToplevelDescribe->isChecked());
    toConfigurationSingle::Instance().setWsMoveToErr(MoveToError->isChecked());
    toConfigurationSingle::Instance().setWsStatistics(Statistics->isChecked());
    toConfigurationSingle::Instance().setWsHistory(History->isChecked());
    toConfigurationSingle::Instance().setWsTimedStats(TimedStatistics->isChecked());
    toConfigurationSingle::Instance().setWsNumber(DisplayNumber->isChecked());
    toConfigurationSingle::Instance().setWsExecLog(ExecLog->isChecked());
    toConfigurationSingle::Instance().setWsAutoLoad(DefaultFile->text());
}


void toWorksheetSetup::chooseFile(void)
{
    QString str = toOpenFilename(DefaultFile->text(), QString::null, this);
    if (!str.isEmpty())
        DefaultFile->setText(str);
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
    virtual QWidget *toolWindow(QWidget *main, toConnection &connection)
    {
        return new toWorksheet(main, connection);
    }
    virtual QWidget *configurationTab(QWidget *parent)
    {
        return new toWorksheetSetup(this, parent);
    }
    virtual bool canHandle(toConnection &)
    {
        return true;
    }
    virtual void closeWindow(toConnection &connection) {};
};

static toWorksheetTool WorksheetTool;

void toWorksheet::viewResources(void)
{
    try
    {
        QString address = toSQLToAddress(connection(), QueryString);

        Resources->changeParams(address);

        QString sql = toSQL::string(TOSQL_LONGOPS, connection());
        sql += "   AND b.SQL_Address||':'||b.SQL_Hash_Value = :addr<char[100]>";
        LongOps->setSQL(sql);
        LongOps->clearParams();
        LongOps->changeParams(address);
    }
    TOCATCH
}

#define TOWORKSHEET "toWorksheet:"

void toWorksheet::createActions()
{
    executeNewlineAct = new QAction(tr("Execute &Newline Separated"), this);
    executeNewlineAct->setShortcut(Qt::SHIFT + Qt::Key_F9);
    connect(executeNewlineAct, SIGNAL(triggered()), this, SLOT(executeNewline(void)));

    parseAct = new QAction(tr("Check syntax of buffer"), this);
    parseAct->setShortcut(Qt::CTRL + Qt::Key_F9);
    connect(parseAct, SIGNAL(triggered()), this, SLOT(parseAll(void)));

    executeAct = new QAction(QPixmap(const_cast<const char**>(execute_xpm)),
                             tr("Execute current statement"),
                             this);
    executeAct->setShortcut(Qt::CTRL + Qt::Key_Return);
    connect(executeAct, SIGNAL(triggered()), this, SLOT(execute(void)));

    executeStepAct = new QAction(QPixmap(const_cast<const char**>(executestep_xpm)),
                                 tr("Step through statements"),
                                 this);
    executeStepAct->setShortcut(Qt::Key_F9);
    connect(executeStepAct, SIGNAL(triggered()), this, SLOT(executeStep(void)));

    executeAllAct = new QAction(QPixmap(const_cast<const char**>(executeall_xpm)),
                                tr("Execute all statements"),
                                this);
    executeAllAct->setShortcut(Qt::Key_F8);
    connect(executeAllAct, SIGNAL(triggered()), this, SLOT(executeAll(void)));

    refreshAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                             tr("Reexecute Last Statement"),
                             this);
    refreshAct->setShortcut(QKeySequence::Refresh);
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refreshSetup(void)));

    describeAct = new QAction(QPixmap(const_cast<const char**>(describe_xpm)),
                              tr("Describe under cursor"),
                              this);
    describeAct->setShortcut(Qt::Key_F4);
    connect(describeAct, SIGNAL(triggered()), this, SLOT(describe(void)));

    explainAct = new QAction(QPixmap(const_cast<const char**>(explainplan_xpm)),
                             tr("Explain plan of current statement"),
                             this);
    explainAct->setShortcut(Qt::Key_F6);
    connect(explainAct, SIGNAL(triggered()), this, SLOT(explainPlan(void)));

    stopAct = new QAction(QPixmap(const_cast<const char**>(stop_xpm)),
                          tr("Stop execution"),
                          this);
    connect(stopAct, SIGNAL(triggered()), this, SLOT(stop(void)));
    stopAct->setEnabled(false);

    eraseAct = new QAction(QPixmap(const_cast<const char**>(eraselog_xpm)),
                           tr("Clear execution log"),
                           this);
    connect(eraseAct, SIGNAL(triggered()), this, SLOT(eraseLogButton(void)));

    statisticAct = new QAction(QPixmap(const_cast<const char**>(clock_xpm)),
                               tr("Gather session statistic of execution"),
                               this);
    statisticAct->setCheckable(true);
    connect(statisticAct, SIGNAL(toggled(bool)), this, SLOT(enableStatistic(bool)));

    previousAct = new QAction(QPixmap(const_cast<const char**>(up_xpm)),
                              tr("Previous log entry"),
                              this);
    connect(previousAct, SIGNAL(triggered()), this, SLOT(executePreviousLog(void)));

    nextAct = new QAction(QPixmap(const_cast<const char**>(down_xpm)),
                          tr("Next log entry"),
                          this);
    connect(nextAct, SIGNAL(triggered()), this, SLOT(executeNextLog(void)));

    saveLastAct = new QAction(QPixmap(const_cast<const char**>(previous_xpm)),
                              tr("Save last SQL"),
                              this);
    connect(saveLastAct, SIGNAL(triggered()), this, SLOT(saveLast(void)));
}


void toWorksheet::setup(bool autoLoad)
{
    QToolBar *workToolbar = toAllocBar(this, tr("SQL worksheet"));
    layout()->addWidget(workToolbar);

    workToolbar->addAction(executeAct);
    workToolbar->addAction(executeStepAct);
    workToolbar->addAction(executeAllAct);

    workToolbar->addSeparator();
    workToolbar->addAction(refreshAct);

    workToolbar->addSeparator();
    workToolbar->addAction(describeAct);
    workToolbar->addAction(explainAct);

    workToolbar->addAction(stopAct);

    workToolbar->addSeparator();

    workToolbar->addAction(eraseAct);

    workToolbar->addSeparator();

    workToolbar->addAction(statisticAct);

    workToolbar->addWidget(new QLabel(tr("Refresh") + " ",
                                      workToolbar));
    Refresh = toRefreshCreate(workToolbar, TO_TOOLBAR_WIDGET_NAME);
    workToolbar->addWidget(Refresh);
    connect(Refresh,
            SIGNAL(activated(const QString &)),
            this,
            SLOT(changeRefresh(const QString &)));
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
            SLOT(showInsertSaved()));
    connect(InsertSavedMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(insertSaved(QAction *)));
    connect(InsertSavedMenu,
            SIGNAL(actionRemoved(QAction *)),
            this,
            SLOT(removeSaved(QAction *)));

    SavedMenu = new toEditableMenu(workToolbar);
    SavedMenu->setIcon(QPixmap(const_cast<const char**>(recall_xpm)));
    SavedMenu->setTitle(tr("Run current saved SQL"));
    workToolbar->addAction(SavedMenu->menuAction());
    connect(SavedMenu, SIGNAL(aboutToShow()), this, SLOT(showSaved()));
    connect(SavedMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(executeSaved(QAction *)));
    connect(SavedMenu,
            SIGNAL(actionRemoved(QAction *)),
            this,
            SLOT(removeSaved(QAction *)));

    workToolbar->addAction(saveLastAct);

    workToolbar->addWidget(Started = new QLabel(workToolbar));
    Started->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    Started->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                       QSizePolicy::Minimum));

    Schema = new toResultSchema(connection(), workToolbar);
    workToolbar->addWidget(Schema);
    try
    {
        Schema->refresh();
    }
    catch (...) {}
    connect(Schema,
            SIGNAL(currentIndexChanged(const QString &)),
            Schema,
            SLOT(update(const QString &)));

    new toChangeConnection(workToolbar, TO_TOOLBAR_WIDGET_NAME);

    RefreshSeconds = 60;
    connect(&RefreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));

    LastLine = LastOffset = -1;
    LastID = 0;

    EditSplitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(EditSplitter);

    Editor = new toWorksheetText(this, EditSplitter);
    // stop any running query when a file is loaded
    connect(Editor, SIGNAL(fileOpened()), this, SLOT(stop()));
    connect(Editor, SIGNAL(modificationChanged(bool)), this, SLOT(setCaption()));

    ResultTab = new toTabWidget(EditSplitter);
    QWidget *container = new QWidget(ResultTab);
    QVBoxLayout *box = new QVBoxLayout;
    ResultTab->addTab(container, tr("&Result"));

    Current = Result = new toResultTableView(false, true, container);
    box->addWidget(Result);
    connect(Result, SIGNAL(done(void)), this, SLOT(queryDone(void)));
    connect(Result,
            SIGNAL(firstResult(const QString &,
                               const toConnection::exception &,
                               bool)),
            this,
            SLOT(addLog(const QString &,
                        const toConnection::exception &,
                        bool)));
    connect(Result,
            SIGNAL(firstResult(const QString &,
                               const toConnection::exception &,
                               bool)),
            this,
            SLOT(unhideResults(const QString &,
                               const toConnection::exception &,
                               bool)));

    Columns = new toResultCols(container, "description");
    box->addWidget(Columns);
    Columns->hide();

    box->setContentsMargins(0, 0, 0, 0);
    container->setLayout(box);

    ResultTab->setTabEnabled(ResultTab->indexOf(Columns), false);
    Plan = new toResultPlan(ResultTab);
    ResultTab->addTab(Plan, tr("E&xecution plan"));
    explainAct->setEnabled(Plan->handled());

    ResourceSplitter = new QSplitter(Qt::Vertical, ResultTab);
    Resources = new toResultResources(ResourceSplitter);

    LongOps = new toResultTableView(true, true, ResourceSplitter);

    Visualize = new toVisualize(Result, ResultTab);
    ResultTab->addTab(Visualize, tr("&Visualize"));
    ResultTab->addTab(ResourceSplitter, tr("&Information"));
    ResultTab->setTabShown(ResourceSplitter, Resources->handled());

    StatTab = new QWidget(ResultTab);
    box = new QVBoxLayout;      // reassigned

    QToolBar *stattool = toAllocBar(StatTab, tr("Worksheet Statistics"));

    stattool->addAction(QIcon(QPixmap(const_cast<const char**>(filesave_xpm))),
                        tr("Save statistics for later analysis"),
                        this,
                        SLOT(saveStatistics(void)));

    QLabel *statlabel = new QLabel(stattool);
    stattool->addWidget(statlabel);

    box->addWidget(stattool);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, StatTab);
    Statistics = new toResultStats(true, splitter);
    Statistics->setTabWidget(ResultTab);
    WaitChart = new toResultBar(splitter);
    try
    {
        WaitChart->setSQL(toSQL::sql(TO_SESSION_WAIT));
    }
    catch (...) {}
    WaitChart->setTitle(tr("Wait states"));
    WaitChart->setYPostfix(QString::fromLatin1("ms/s"));
    WaitChart->setSamples(-1);
    WaitChart->start();
    connect(Statistics, SIGNAL(sessionChanged(const QString &)),
            WaitChart, SLOT(changeParams(const QString &)));
    IOChart = new toResultBar(splitter);
    try
    {
        IOChart->setSQL(toSQL::sql(TO_SESSION_IO));
    }
    catch (...) {}
    IOChart->setTitle(tr("I/O"));
    IOChart->setYPostfix(tr("blocks/s"));
    IOChart->setSamples(-1);
    IOChart->start();
    connect(Statistics, SIGNAL(sessionChanged(const QString &)),
            IOChart, SLOT(changeParams(const QString &)));

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
    connect(Logging, SIGNAL(selectionChanged(toTreeWidgetItem *)), this, SLOT(executeLog()));
    LastLogItem = NULL;

    connect(ResultTab, SIGNAL(currentChanged(int)),
            this, SLOT(changeResult(int)));

    if (autoLoad)
    {
        Editor->editOpen(toConfigurationSingle::Instance().wsAutoLoad());
    }

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(windowActivated(QMdiSubWindow *)));

    try
    {
        if (connection().provider() == "Oracle")
        {
            if (toConfigurationSingle::Instance().wsStatistics())
            {
                show();
                statisticAct->setChecked(true);
            }
        }
        else
            statisticAct->setEnabled(false);
    }
    TOCATCH;

    connect(this, SIGNAL(connectionChange()), this, SLOT(connectionChanged()));

    Editor->setAnalyzer(connection().analyzer());

    context = NULL;
    Editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(Editor,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(createPopupMenu(const QPoint &)));

    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
    connect(this, SIGNAL(connectionChange()), this, SLOT(changeConnection()));
    setFocusProxy(Editor);

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

    setCaption();
}

toWorksheet::toWorksheet(QWidget *main, toConnection &connection, bool autoLoad)
        : toToolWidget(WorksheetTool, "worksheet.html", main, connection, "toWorksheet")
{
    createActions();
    setup(autoLoad);
}


// private for toworksheetwidget
toWorksheet::toWorksheet(QWidget *main, const char *name, toConnection &connection)
        : toToolWidget(WorksheetTool, "worksheetlight.html", main, connection, name)
{

    createActions();
}


void toWorksheet::changeRefresh(const QString &str)
{
    try
    {
        if (stopAct->isEnabled() && statisticAct->isChecked())
            toRefreshParse(timer(), str);
    }
    TOCATCH;
}

void toWorksheet::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;

    QWidget *w = this;
    while (w && w != widget->widget())
        w = w->parentWidget();

    if (widget->widget() == w)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("Edit&or"), this);

            ToolMenu->addAction(executeAct);
            ToolMenu->addAction(executeStepAct);
            ToolMenu->addAction(executeAllAct);
            ToolMenu->addAction(executeNewlineAct);
            ToolMenu->addAction(refreshAct);
            ToolMenu->addAction(parseAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(describeAct);
            ToolMenu->addAction(explainAct);
            ToolMenu->addAction(statisticAct);
            ToolMenu->addAction(stopAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(SavedMenu->menuAction());

            ToolMenu->addAction(saveLastAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(previousAct);
            ToolMenu->addAction(nextAct);
            ToolMenu->addAction(eraseAct);

            toMainWidget()->addCustomMenu(ToolMenu);
        }

        // disabled. can cause infinite loop on some window systems
        // depending on the timing.
        // Editor->setFocus();
		if (Editor)
			Editor->setFocus(Qt::ActiveWindowFocusReason);

        // must set correct schema every time so having two worksheets
        // on different schemas works properly.
        if(Schema)
            Schema->update();
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toWorksheet::connectionChanged(void)
{
    try
    {
        statisticAct->setEnabled(connection().provider() == "Oracle");
        ResultTab->setTabShown(ResourceSplitter, Resources->handled());
        Editor->setAnalyzer(connection().analyzer());
        explainAct->setEnabled(Plan->handled());
        delete ToolMenu;
        ToolMenu = NULL;
        windowActivated(qobject_cast<QMdiSubWindow*>(this->parent()));
    }
    TOCATCH;
}

bool toWorksheet::checkSave(bool input)
{
    if (!Editor->isModified())
        return true;

    if (!toConfigurationSingle::Instance().wsAutoSave() ||
            Editor->filename().isEmpty())
    {

//         if (!WorksheetTool.config(CONF_CHECK_SAVE, "Yes").isEmpty()) {
        if (toConfigurationSingle::Instance().wsCheckSave())
        {
            if (!input)
                return true;

            QString conn;
            try
            {
                conn = connection().description();
            }
            catch (...)
            {
                conn += QString::fromLatin1("unknown connection");
            }

            // grab focus so user can see file and decide to save
            setFocus(Qt::OtherFocusReason);

            QString str = tr("Save changes to editor for %1?").arg(conn);
            if (!Editor->filename().isEmpty())
                str += QString::fromLatin1("\n(") +
                       Editor->filename() +
                       QString::fromLatin1(")");

            int ret = TOMessageBox::information(
                          this,
                          "Save File",
                          str,
                          QMessageBox::Save |
                          QMessageBox::Discard |
                          QMessageBox::Cancel);

            if (ret == QMessageBox::Save)
            {
                if (Editor->filename().isEmpty() && input)
                    Editor->setFilename(toSaveFilename(Editor->filename(),
                                                       QString::null,
                                                       this));
                if (Editor->filename().isEmpty())
                    return false;
            }
            else if (ret == QMessageBox::Discard)
            {
                // only ever called if closing widget, make sure this
                // is not called again.
                Editor->setModified(false);
                return true;
            }
            else
                return false;
        }
        else
            return true;
    }

    if (!toWriteFile(Editor->filename(), Editor->text()))
        return false;
    Editor->setModified(false);
    return true;
}

bool toWorksheet::close()
{
    if (Statistics)
    {
        Statistics->close();
        delete Statistics;
        Statistics = NULL;
    }

    if (checkSave(true))
    {
        Result->stop();
        return toToolWidget::close();
    }

    return false;
}

void toWorksheet::closeEvent(QCloseEvent *event)
{
    QSettings s;
    s.beginGroup("toWorksheet");
    s.setValue("EditSplitterSizes0", EditSplitter->sizes()[0]);
    s.setValue("EditSplitterSizes1", EditSplitter->sizes()[1]);
    s.endGroup();

    if (close())
        event->accept();
    else
        event->ignore();
}


toWorksheet::~toWorksheet()
{
}

#define LARGE_BUFFER 4096

void toWorksheet::changeResult(int index)
{
    QWidget *widget = ResultTab->widget(index);
    if (!widget)
        return;

    CurrentTab = widget;
    if (QueryString.length())
    {
        if (CurrentTab == Plan)
            Plan->query(QueryString);
        else if (CurrentTab == ResourceSplitter)
            viewResources();
        else if (CurrentTab == Statistics && Result->running())
            Statistics->refreshStats(false);
    }
}

void toWorksheet::refresh(void)
{
    if (!QueryString.isEmpty())
        query(QueryString, Normal, QueryStatementClass);
    if (RefreshSeconds > 0)
    {
        RefreshTimer.setSingleShot(true);
        RefreshTimer.start(RefreshSeconds * 1000);
    }
}

static QString unQuote(const QString &str)
{
    if (str.at(0).toLatin1() == '\"' && str.at(str.length() - 1).toLatin1() == '\"')
        return str.left(str.length() - 1).right(str.length() - 2);
    return str.toUpper();
}

bool toWorksheet::describe(const QString &query)
{
    try
    {
        QRegExp white(QString::fromLatin1("[ \r\n\t.]+"));
        QStringList part = query.split(white);
        if (part[0].toUpper() == QString("DESC") ||
                part[0].toUpper() == QString("DESCRIBE"))
        {
            unhideResults();

            if (toIsOracle(connection()))
            {
                if (part.count() == 2)
                {
                    Columns->changeParams(unQuote(part[1]));
                }
                else if (part.count() == 3)
                {
                    Columns->changeParams(unQuote(part[1]), unQuote(part[2]));
                }
                else
                    throw tr("Wrong number of parameters for describe");
            }
            else if (connection().provider() == "MySQL")
            {
                if (part.count() == 2)
                {
                    Columns->changeParams(part[1]);
                }
                else
                    throw tr("Wrong number of parameters for describe");
            }
            Current->hide();
            Columns->show();
            Current = Columns;
            return true;
        }
        else
        {
            QWidget *curr = ResultTab->currentWidget();
            Current->hide();
            Result->show();
            Current = Result;
            if (curr == Columns)
                ResultTab->setCurrentIndex(ResultTab->indexOf(Result));
            return false;
        }
    }
    TOCATCH
    return false;
}

static toSQL SQLCheckMySQLRoutine("toWorksheet:CheckRoutine",
                        "select count(1)\n"
                        "from information_schema.routines\n"
                        "where routine_name = :f1<char[101]>\n"
                        "  and lower(routine_type) = :f2<char[101]>\n"
                        "  and routine_schema = :f3<char[101]>",
                        "Check if routine exists in MySQL",
                        "5.0",
                        "MySQL");

static toSQL SQLDropMySQLRoutine("toWorksheet:DropRoutine",
                        "drop :f1<noquote> if exists :f2<noquote>;",
                        "Drop MySQL routine if it exists",
                        "5.0",
                        "MySQL");

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
    // wether routine exists must checked if config is set to 2 (drop if exists) and 4 (ask if exists)
    bool check = (toConfigurationSingle::Instance().createAction() % 2 == 0);
    // wether to ask or drop automatically if config is set to 3 (ask) and 4 (ask if exists)
    bool ask = (toConfigurationSingle::Instance().createAction() > 2);
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

            try {
                if (check)
                {
                    // Check if this routine actually exists in database
                    toQList param;
                    toPush(param, toQValue(tok[2 + i].remove('`'))); // routine name
                    toPush(param, toQValue(tok[1 + i])); // routine type (procedure or function)
                    toPush(param, toQValue(Schema->currentText()));
                    toQuery query(connection(), toQuery::Long, SQLCheckMySQLRoutine, param);
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
                        toQList param;
                        toPush(param, toQValue(tok[1 + i])); // routine type (procedure or function)
                        toPush(param, toQValue(tok[2 + i])); // routine name
                        // Note that if routine creation is not successfull you will be left without any routine!
                        toQuery query(connection(), toQuery::Long, SQLDropMySQLRoutine, param);
                    }
                }
            }
            TOCATCH;
        }
    }
} // mySQLBeforeCreate

void toWorksheet::query(const QString &str, execType type, toSQLParse::statement::statementClass sc)
{
    Result->stop();
    RefreshTimer.stop();

    QRegExp strq(QString::fromLatin1("'[^']*'"));
    QString chk = str.toLower();
    chk.replace(strq, QString::fromLatin1(" "));
    bool code;

    // TS 2010-03-11 Parser identifies if it is SQL or PLSQL block
    if (sc == toSQLParse::statement::unknown)
    {
        // If query is executed from some text buffer it should be parsed to identify statementClass
        toSQLParse::stringTokenizer tokens(str);
        toSQLParse::statement st = toSQLParse::parseStatement(tokens);
        sc = st.StatementClass;
    }
    code = (sc == toSQLParse::statement::plsqlblock);
    /*
    static QRegExp codere(QString::fromLatin1("[^a-z0-9]end\\s+[a-z0-9_-]*;$"), Qt::CaseSensitive);
    static QRegExp codere2(QString::fromLatin1("[^a-z0-9]end;"), Qt::CaseSensitive);

    if (codere.indexIn(chk) >= 0 || codere2.indexIn(chk) >= 0)
        code = true;
    */

    QueryString = str.trimmed();
    QueryStatementClass = sc;
    if (!code && QueryString.length() > 0 && QueryString.at(QueryString.length() - 1) == ';')
        QueryString.truncate(QueryString.length() - 1);

    bool nobinds = false;
    chk = str.toLower();
    chk.replace(strq, QString::fromLatin1(" "));
    chk = chk.simplified();
    chk.replace(QRegExp(QString::fromLatin1(" or replace ")), QString::fromLatin1(" "));
    if (chk.startsWith(QString::fromLatin1("create trigger ")))
        nobinds = true;

    // Imitate something like "create or replace" syntax for MySQL
    if (toIsMySQL(connection()) && code && toConfigurationSingle::Instance().createAction() > 0)
        mySQLBeforeCreate(chk);

    if (type == OnlyPlan)
    {
        ResultTab->setCurrentIndex(ResultTab->indexOf(Plan));
        Plan->query(str);
        unhideResults();
    }
    else if (!describe(QueryString))
    {

        toSQLParse::stringTokenizer tokens(str);
        QString first = tokens.getToken(true).toUpper();

        if (toIsOracle(connection()) && first == "EXEC") {
            // put exec in anonymous plsql block or they won't work
            QueryString = QueryString.trimmed().right(QueryString.length() - 4);
            QueryString = QString("BEGIN\n%1;\nEND;").arg(QueryString);
        }

        if (first == QString::fromLatin1("REM") ||
                first == QString::fromLatin1("ASSIGN") ||
                first == QString::fromLatin1("PROMPT") ||
                first == QString::fromLatin1("COLUMN") ||
                first == QString::fromLatin1("SPOOL") ||
                first == QString::fromLatin1("STORE"))
        {

            QString t = tr("Ignoring SQL*Plus command");
            Timer.start();
            addLog(QueryString, toConnection::exception(t), false);
            toStatusMessage(t, true);
            return ;
        }

        // unhide the results pane if there's something to show
        if (first == "SELECT" || (ResultTab && ResultTab->currentIndex() != 0))
            unhideResults();

        toQList param;
        if (!nobinds)
            try
            {
                param = toParamGet::getParam(connection(), this, QueryString);
            }
            catch (...)
            {
                return ;
            }
        toStatusMessage(tr("Processing query"), true);

        if (type == Parse)
        {
            try
            {
                First = false;
                Timer.start();
                connection().parse(QueryString);
            }
            catch (const QString &exc)
            {
                addLog(QueryString, exc, true);
            }
        }
        else if (type == Direct)
        {
            try
            {
                First = false;
                Timer.start();
                QString buffer;
                if (!toConfigurationSingle::Instance().wsHistory())
                {
                    toQuery query(connection(), toQuery::Long, QueryString, param);
                    if (query.rowsProcessed() > 0)
                        buffer = tr("%1 rows processed").arg((int)query.rowsProcessed());
                    else
                        buffer = tr("Query executed");
                }
                else
                {
                    toResultView *query = new toResultView(Current->parentWidget());

                    try
                    {
                        query->query((QString) QueryString, (toQList) param);
                        if (query->query() && query->query()->rowsProcessed() > 0)
                            buffer = tr("%1 rows processed").arg((int)query->query()->rowsProcessed());
                        else
                            buffer = tr("Query executed");
                        Current->hide();
                        Current = query;
                        Current->show();
                    }
                    catch (...)
                    {
                        delete query;
                        throw;
                    }
                }

                addLog(QueryString, toConnection::exception(buffer), false);
            }
            catch (const QString &exc)
            {
                addLog(QueryString, exc, true);
                if (QMessageBox::question(this, tr("Direct Execute Error"),
                                      exc + "\n\n" + tr("Stop execution ('No' to continue)?"),
                                      QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                    m_batchContinue = false;
            }
        }
        else
        {
            First = false;
            Timer.start();
            stopAct->setEnabled(true);
            Poll.start(1000);
            Started->setToolTip(tr("Duration while query has been running\n\n") + QueryString);
            stopAct->setEnabled(true);
            Result->setNumberColumn(toConfigurationSingle::Instance().wsNumber());
            // it fixes crash running statements from Schema Browser - PV
            if (ResultTab)
                ResultTab->setCurrentIndex(0);

            try
            {
                saveHistory();
                Result->setSQL(QString::null);
                Result->query(QueryString, param);
//                 if (CurrentTab)
//                 {
                    // todo
                    // if(CurrentTab == Visualize)
//                         Visualize->display();
// PV - let's open really required tab for called action
// e.g. Plan for explainplan, Result for run statement action etc.
// It stops to really run a statement when I expect explain plan
//                     else
//                     if (CurrentTab == Plan)
//                         Plan->query(QueryString);
//                     else if (CurrentTab == ResourceSplitter)
//                         viewResources();
//                 }
            }
            catch (const toConnection::exception &exc)
            {
                addLog(QueryString, exc, true);
            }
            catch (const QString &exc)
            {
                addLog(QueryString, exc, true);
            }
            try
            {
                if (statisticAct->isChecked())
                    toRefreshParse(timer(), Refresh->currentText());
            }
            TOCATCH
            Result->setSQLName(QueryString.simplified().left(40));
        }
    }
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
        Result->stop();
        disconnect(Result, SIGNAL(done(void)), this, SLOT(queryDone(void)));
        disconnect(Result, SIGNAL(firstResult(const QString &, const toConnection::exception &, bool)),
                   this, SLOT(addLog(const QString &, const toConnection::exception &, bool)));
        disconnect(stopAct, SIGNAL(clicked(void)), Result, SLOT(stop(void)));

        Result = new toResultTableView(Result->parentWidget());
        if (statisticAct->isChecked())
            enableStatistic(true);
        Result->show();
        Current = Result;
        connect(stopAct, SIGNAL(clicked(void)), Result, SLOT(stop(void)));
        connect(Result, SIGNAL(done(void)), this, SLOT(queryDone(void)));
        connect(Result, SIGNAL(firstResult(const QString &, const toConnection::exception &, bool)),
                this, SLOT(addLog(const QString &, const toConnection::exception &, bool)));
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

void toWorksheet::addLog(const QString &sql,
                         const toConnection::exception &result,
                         bool error)
{
    QString now = QDateTime::currentDateTime().toString(Qt::SystemLocaleDate);
    toResultViewItem *item = NULL;

    LastID++;

    int dur = 0;
    if (!Timer.isNull())
        dur = Timer.elapsed();
    First = true;

    if (Logging)
    {
        if (!toConfigurationSingle::Instance().wsLogMulti())
        {
            if (!toConfigurationSingle::Instance().wsLogAtEnd())
                item = new toResultViewItem(Logging, NULL);
            else
                item = new toResultViewItem(Logging, LastLogItem);
        }
        else if (!toConfigurationSingle::Instance().wsLogAtEnd())
            item = new toResultViewMLine(Logging, NULL);
        else
            item = new toResultViewMLine(Logging, LastLogItem);
        item->setText(0, sql);

        LastLogItem = item;
        item->setText(1, result);
        item->setText(2, now);
        if (toConfigurationSingle::Instance().wsHistory())
            item->setText(4, QString::number(LastID));
        item->setText(5, QString::number(result.offset()));
    }

    if (result.offset() >= 0 && LastLine >= 0 && LastOffset >= 0 &&
            toConfigurationSingle::Instance().wsMoveToErr())
    {
        QChar cmp = '\n';
        int lastnl = 0;
        int lines = 0;
        for (int i = 0;i < result.offset() && i < sql.length();i++)
        {
            if (sql.at(i) == cmp)
            {
                LastOffset = 0;
                lastnl = i + 1;
                lines++;
            }
        }
        Editor->setCursorPosition(LastLine + lines, LastOffset + result.offset() - lastnl);
        LastLine = LastOffset = -1;
    }

    QString buf = duration(dur);

    if (Logging)
    {
        item->setText(3, buf);

        toTreeWidgetItem *last = Logging->currentItem();
        toResultViewItem *citem = NULL;
        if (last)
            citem = dynamic_cast<toResultViewItem *>(last);
        if (!citem || citem->allText(0) != sql)
        {
            disconnect(Logging,
                       SIGNAL(selectionChanged(toTreeWidgetItem *)),
                       this,
                       SLOT(executeLog()));
            Logging->setSelected(item, true);
            connect(Logging,
                    SIGNAL(selectionChanged(toTreeWidgetItem *)),
                    this,
                    SLOT(executeLog()));
            Logging->ensureItemVisible(item);
        }
    }

    {
        QString str = result;
        str += "\n" + tr("(Duration %1)").arg(buf);
        if (!error)
            toStatusMessage(str, false, false);
    }

    if (!error) {
        if(ResultTab)
            changeResult(ResultTab->indexOf(CurrentTab));

        // the sql string will be trimmed but case will be same as the
        // original.  the code originally compared the result return, but
        // that class doesn't know if a commit is needed either.

        static QRegExp re(QString::fromLatin1("^SELECT"));
        re.setCaseSensitivity(Qt::CaseInsensitive);
        try
        {
            if (!sql.contains(re))
            {
                if (toConfigurationSingle::Instance().autoCommit())
                    connection().commit();
                else
                    toMainWidget()->setNeedCommit(connection());
            }
        }
        TOCATCH;
    }

    saveDefaults();
}

void toWorksheet::execute(toSQLParse::tokenizer &tokens, int line, int pos, execType type,
                          toSQLParse::statement::statementClass sc)
{
    LastLine = line;
    LastOffset = pos;
    int endLine, endCol;
    if (Editor->lines() <= tokens.line())
    {
        endLine = Editor->lines() - 1;
        endCol = Editor->lineLength(endLine);
    }
    else
    {
        endLine = tokens.line();
        if (Editor->lineLength(tokens.line()) <= tokens.offset())
            endCol = Editor->lineLength(tokens.line());
        else
        {
            endCol = tokens.offset();
        }
    }
    Editor->setSelection(line, pos, endLine, endCol);
    QString t = Editor->selectedText();

    bool comment = false;
    bool multiComment = false;
    int oline = line;
    int opos = pos;
    int i;

    // Remove empty space and the slash at the end of statement.
    // Note: slash is valid in oracle sqlplus only
    i = t.length() - 1;
    while ((i > 0) &&
           ((t.at(i) == ' ') ||
            (t.at(i) == '\r') ||
            (t.at(i) == '\n') ||
            (t.at(i) == '\t') ||
            (t.at(i) == '/')))
      i--;
    t.truncate(i + 1);

    for (i = 0;i < t.length() - 1;i++)
    {
        if (comment)
        {
            if (t.at(i).toLatin1() == '\n')
                comment = false;
        }
        else if (multiComment)
        {
            if (t.at(i).toLatin1() == '*' &&
                    t.at(i + 1).toLatin1() == '/')
            {
                multiComment = false;
                i++;
            }
        }
        else if (t.at(i).toLatin1() == '-' &&
                 t.at(i + 1).toLatin1() == '-')
            comment = true;
        else if (t.at(i).toLatin1() == '/' &&
                 t.at(i + 1).toLatin1() == '/')
            comment = true;
        else if (t.at(i).toLatin1() == '/' &&
                 t.at(i + 1).toLatin1() == '*')
            multiComment = true;
        else if (!t.at(i).isSpace() && t.at(i) != '/')
            break;

        if (t.at(i).toLatin1() == '\n')
        {
            line++;
            pos = 0;
        }
        else
            pos++;
    }

    if (line != oline ||
            pos != opos)
    {
        LastLine = line;
        LastOffset = pos;
        Editor->setSelection(line, pos, endLine, endCol);
        t = t.mid(i);
    }

    if (t.trimmed().length())
        query(t, type, sc);
}

void toWorksheet::unhideResults(const QString &,
                                const toConnection::exception &,
                                bool error)
{
    if (!error && Result->model()->rowCount() > 0)
        unhideResults();
}

void toWorksheet::unhideResults()
{
    // move splitter if currently hidden
    if (EditSplitter->sizes()[1] == 0)
        EditSplitter->setSizes(EditSplitterSizes);
}

void toWorksheet::execute()
{
    if (Editor->hasSelectedText())
    {
        query(Editor->selectedText(), Normal, toSQLParse::statement::unknown);
        return;
    }

    toSQLParse::editorTokenizer tokens(Editor);

    int cpos, cline;
    Editor->getCursorPosition(&cline, &cpos);

    int line;
    int pos;
    toSQLParse::statement st;
    do
    {
        line = tokens.line();
        pos = tokens.offset();
        st = toSQLParse::parseStatement(tokens);
    }
    while (tokens.line() < cline ||
            (tokens.line() == cline && tokens.offset() < cpos));

    execute(tokens, line, pos, Normal, st.StatementClass);
}

// TODO: TS 2010-03-11 This function is the same as execute, just Normal->OnlyPlan. Should be optimised.
void toWorksheet::explainPlan()
{
    if (Editor->hasSelectedText())
    {
        query(Editor->selectedText(), OnlyPlan, toSQLParse::statement::unknown);
        return ;
    }

    toSQLParse::editorTokenizer tokens(Editor);

    int cpos, cline;
    Editor->getCursorPosition(&cline, &cpos);

    int line;
    int pos;
    toSQLParse::statement st;
    do
    {
        line = tokens.line();
        pos = tokens.offset();
        st = toSQLParse::parseStatement(tokens);
    }
    while (tokens.line() < cline ||
            (tokens.line() == cline && tokens.offset() < cpos));

    execute(tokens, line, pos, OnlyPlan, st.StatementClass);
}

void toWorksheet::executeStep()
{
    toSQLParse::editorTokenizer tokens(Editor);

    int cpos, cline;
    Editor->getCursorPosition(&cline, &cpos);

    int line;
    int pos;
    toSQLParse::statement st;
    do
    {
        line = tokens.line();
        pos = tokens.offset();
        st = toSQLParse::parseStatement(tokens);
    }
    while (tokens.line() < cline ||
            (tokens.line() == cline && tokens.offset() <= cpos));

    execute(tokens, line, pos, Normal, st.StatementClass);
}

void toWorksheet::executeAll()
{
    toSQLParse::editorTokenizer tokens(Editor);

    int cpos, cline;
    Editor->getCursorPosition(&cline, &cpos);

    QProgressDialog dialog(tr("Executing all statements"),
                           tr("Cancel"),
                           0,
                           Editor->lines(),
                           this);
    int line;
    int pos;
    bool ignore = true;
    m_batchContinue = true;
    do
    {
        line = tokens.line();
        pos = tokens.offset();
        dialog.setValue(line);
        qApp->processEvents();
        if (dialog.wasCanceled())
            break;
        toSQLParse::statement st = toSQLParse::parseStatement(tokens);

        if (ignore && (tokens.line() > cline ||
                       (tokens.line() == cline &&
                        tokens.offset() >= cpos)))
        {
            ignore = false;
            cline = line;
            cpos = pos;
        }

        if (tokens.line() < Editor->lines() && !ignore)
        {
            execute(tokens, line, pos, Direct, st.StatementClass);
            if (Current)
            {
                toResultView *last = dynamic_cast<toResultView *>(Current);
                if (toConfigurationSingle::Instance().wsHistory() &&
                        last && last->firstChild())
                    History[LastID] = last;
            }
        }
    }
    while (m_batchContinue && (tokens.line() < Editor->lines()));

    Editor->setSelection(cline, cpos, tokens.line(), tokens.offset());
}

void toWorksheet::parseAll()
{
    toSQLParse::editorTokenizer tokens(Editor);

    int cpos, cline;
    Editor->getCursorPosition(&cline, &cpos);

    QProgressDialog dialog(tr("Parsing all statements"),
                           tr("Cancel"),
                           0,
                           Editor->lines(),
                           this);
    int line;
    int pos;
    bool ignore = true;
    do
    {
        line = tokens.line();
        pos = tokens.offset();
        dialog.setValue(line);
        qApp->processEvents();
        if (dialog.wasCanceled())
            break;
        toSQLParse::statement st = toSQLParse::parseStatement(tokens);

        if (ignore && (tokens.line() > cline ||
                       (tokens.line() == cline &&
                        tokens.offset() >= cpos)))
        {
            ignore = false;
            cline = line;
            cpos = pos;
        }

        if (tokens.line() < Editor->lines() && !ignore)
        {
            execute(tokens, line, pos, Parse, st.StatementClass);
            if (Current)
            {
                toResultView *last = dynamic_cast<toResultView *>(Current);
                if (toConfigurationSingle::Instance().wsHistory() &&
                        last && last->firstChild())
                    History[LastID] = last;
            }
        }
    }
    while (tokens.line() < Editor->lines());

    Editor->setSelection(cline, cpos, tokens.line(), tokens.offset());
}

void toWorksheet::eraseLogButton()
{
    if (!Logging)
        return;

    Logging->clear();
    LastLogItem = NULL;
    for (std::map<int, QWidget *>::iterator i = History.begin();i != History.end();i++)
        delete(*i).second;
    History.clear();
}

void toWorksheet::queryDone(void)
{
    if (!First && !QueryString.isEmpty())
        addLog(QueryString, toConnection::exception(tr("Aborted")), false);
    else
        emit executed();
    try
    {
        timer()->stop();
    }
    TOCATCH
    stopAct->setEnabled(false);
    Poll.stop();
    stopAct->setEnabled(false);
    saveDefaults();
}

void toWorksheet::saveDefaults(void)
{
#if 0                           // todo
    toTreeWidgetItem *item = Result->firstChild();
    if (item)
    {
        QTreeWidgetItem *head = Result->headerItem();
        for (int i = 0;i < Result->columns();i++)
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

void toWorksheet::enableStatistic(bool ena)
{
    if (ena)
    {
        Result->setStatistics(Statistics);
        ResultTab->setTabEnabled(ResultTab->indexOf(StatTab), true);
        statisticAct->setChecked(true);
        Statistics->clear();
        if (toConfigurationSingle::Instance().wsTimedStats())
        {
            try
            {
                connection().allExecute(QString::fromLatin1(ENABLETIMED));
                connection().addInit(QString::fromLatin1(ENABLETIMED));
            }
            TOCATCH;
        }
    }
    else
    {
        try
        {
            connection().delInit(QString::fromLatin1(ENABLETIMED));
        }
        catch (...) {}
        Result->setStatistics(NULL);
        ResultTab->setTabEnabled(ResultTab->indexOf(StatTab), false);
        statisticAct->setChecked(false);
    }
}

void toWorksheet::executeNewline(void)
{
    int cline, epos;

    Editor->getCursorPosition(&cline, &epos);

    if (cline > 0)
        cline--;
    while (cline > 0)
    {
        QString data = Editor->text(cline).simplified();
        if (data.length() == 0 || data == QString::fromLatin1(" "))
        {
            cline++;
            break;
        }
        cline--;
    }

    while (cline < Editor->lines())
    {
        QString data = Editor->text(cline).simplified();
        if (data.length() != 0 && data != QString::fromLatin1(" "))
            break;
        cline++;
    }

    int eline = cline;

    while (eline < Editor->lines())
    {
        QString data = Editor->text(eline).simplified();
        if (data.length() == 0 || data == QString::fromLatin1(" "))
        {
            eline--;
            break;
        }
        epos = Editor->text(eline).length();
        eline++;
    }
    Editor->setSelection(cline, 0, eline, epos);
    LastLine = cline;
    LastOffset = 0;
    if (Editor->hasSelectedText())
        query(Editor->selectedText(), Normal, toSQLParse::statement::unknown);
}

void toWorksheet::describe(void)
{
    QString owner, table;
    Editor->tableAtCursor(owner, table);
    if (owner.isNull())
        owner = Schema->currentText();

    if (toConfigurationSingle::Instance().wsToplevelDescribe())
    {
        toDescribe * d = new toDescribe(this);
        d->changeParams(owner, table);
//         d->show();
    }
    else
    {
        unhideResults();
        Columns->changeParams(owner, table);
        Columns->show();
        Current = Columns;
    }
}

void toWorksheet::executeSaved(QAction *act)
{
    QString sql = act->data().toString();
    if(!sql.isEmpty())
    {
        try
        {
            query(sql, Normal, toSQLParse::statement::unknown);
        }
        TOCATCH;
    }
}

void toWorksheet::insertSaved(QAction *act)
{
    QString sql = act->data().toString();
    if(!sql.isEmpty())
    {
        Editor->setFocus();
        Editor->insert(sql);
    }
}

void toWorksheet::showSaved()
{
    SavedMenu->clear();

    QSettings settings;
    settings.beginGroup("toWorksheet");

    QList<QVariant> statements = settings.value("sql").toList();
    QAction *last = 0;
    foreach(QVariant v, statements) {
        QAction *a = new QAction(v.toString().left(50), SavedMenu);
        a->setData(v.toString());
        SavedMenu->insertAction(last, a);
        last = a;
    }
}

void toWorksheet::showInsertSaved()
{
    InsertSavedMenu->clear();

    QSettings settings;
    settings.beginGroup("toWorksheet");

    QList<QVariant> statements = settings.value("sql").toList();
    QAction *last = 0;
    foreach(QVariant v, statements) {
        QAction *a = new QAction(v.toString().left(50), InsertSavedMenu);
        a->setData(v.toString());
        InsertSavedMenu->insertAction(last, a);
        last = a;
    }
}


void toWorksheet::removeSaved(QAction *action)
{
    QSettings settings;
    settings.beginGroup("toWorksheet");

    QList<QVariant> statements = settings.value("sql").toList();
    statements.removeAll(action->data().toString());
    settings.setValue("sql", statements);
}


void toWorksheet::saveLast()
{
    if (QueryString.isEmpty())
    {
        TOMessageBox::warning(this, tr("No SQL to save"),
                              tr("You haven't executed any SQL yet"),
                              tr("&Ok"));
        return;
    }

    QSettings settings;
    settings.beginGroup("toWorksheet");

    QString sql = QueryString.trimmed();
    if(!sql.endsWith(";"))
        sql += ";";

    QList<QVariant> statements = settings.value("sql").toList();
    if(statements.indexOf(sql) > -1)
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

        Editor->insert(t, true);
    }
}

void toWorksheet::executePreviousLog(void)
{
    Result->stop();

    LastLine = LastOffset = -1;
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

void toWorksheet::executeLog(void)
{
    Result->stop();

    LastLine = LastOffset = -1;
    saveHistory();

    toTreeWidgetItem *ci = Logging->currentItem();
    toResultViewItem *item = dynamic_cast<toResultViewItem *>(ci);
    if (item)
    {
        insertStatement(item->allText(0));

        if (item->text(4).isEmpty())
        {
            if (toConfigurationSingle::Instance().wsExecLog())
                query(item->allText(0), Normal, toSQLParse::statement::unknown);
        }
        else
        {
            std::map<int, QWidget *>::iterator i = History.find(item->text(4).toInt());
            QueryString = item->allText(0);
            changeResult(ResultTab->currentIndex());
            if (i != History.end() && (*i).second)
            {
                Current->hide();
                Current = (*i).second;
                Current->show();
            }
        }
    }
}

void toWorksheet::executeNextLog(void)
{
    Result->stop();

    LastLine = LastOffset = -1;
    saveHistory();

    toTreeWidgetItem *item = Logging->currentItem();
    if (item && item->nextSibling())
    {
        toResultViewItem *next = dynamic_cast<toResultViewItem *>(item->nextSibling());
        if (next)
            Logging->setSelected(next, true);
    }
}

void toWorksheet::poll(void)
{
    Started->setText(duration(Timer.elapsed(), false));
}

void toWorksheet::saveStatistics(void)
{
    std::map<QString, QString> stat;

    Statistics->exportData(stat, "Stat");
    IOChart->exportData(stat, "IO");
    WaitChart->exportData(stat, "Wait");
    if (Plan->firstChild())
        Plan->exportData(stat, "Plan");
    else
        toStatusMessage(tr("No plan available to save"), false, false);
    stat["Description"] = QueryString;

    toWorksheetStatistic::saveStatistics(stat);
}

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
    QString stat = data[prefix + ":Stats"];
    if (!stat.isNull())
    {
        for (int i = 0;i < Refresh->count();i++)
        {
            if (Refresh->itemText(i) == stat)
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
    setCaption();
}

void toWorksheet::setCaption(void)
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
    toToolCaption(this, name);
}

toWorksheet *toWorksheet::fileWorksheet(const QString &file)
{
    QMdiSubWindow * w = new QMdiSubWindow(toMainWidget()->workspace());

    toWorksheet *worksheet = new toWorksheet(w,
            toMainWidget()->currentConnection(),
            false);
    worksheet->editor()->openFilename(file);
    worksheet->setCaption();

    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setWidget(worksheet);
    w->setWindowIcon(QIcon(*WorksheetTool.toolbarImage()));
    toMainWidget()->workspace()->addSubWindow(w);
    w->show();
    toMainWidget()->updateWindowsMenu();
    return worksheet;
}

void toWorksheet::refreshSetup(void)
{
    bool ok = false;
    int num = QInputDialog::getInteger(this,
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
        RefreshTimer.start(num*1000);
    }
    else
        RefreshTimer.stop();
}

void toWorksheet::stop(void)
{
    RefreshTimer.stop();
    Result->stop();
}


void toWorksheet::createPopupMenu(const QPoint &pos)
{
    if (!context)
    {
        context = new QMenu;

        context->addAction(executeAct);
        context->addAction(executeStepAct);
        context->addAction(executeAllAct);
        context->addAction(executeNewlineAct);
        context->addAction(refreshAct);

        context->addSeparator();

        context->addAction(describeAct);
        context->addAction(explainAct);

        context->addSeparator();

        context->addAction(toMainWidget()->getUndoAction());
        context->addAction(toMainWidget()->getRedoAction());

        context->addSeparator();

        context->addAction(toMainWidget()->getCutAction());
        context->addAction(toMainWidget()->getCopyAction());
        context->addAction(toMainWidget()->getPasteAction());

        context->addSeparator();

        context->addAction(toMainWidget()->getSelectAllAction());

        context->addSeparator();

        context->addAction(stopAct);

        context->addSeparator();

        context->addAction(SavedMenu->menuAction());
        context->addAction(InsertSavedMenu->menuAction());
        context->addAction(saveLastAct);
    }

    context->exec(QCursor::pos());
}


void toWorksheet::changeConnection(void)
{
    Schema->query(toSQL::sql(toSQL::TOSQL_USERLIST));
    if (toIsMySQL(connection()))
        Schema->setSelected(connection().database());
    else if (toIsOracle(connection()) || toIsSapDB(connection()))
        Schema->setSelected(connection().user().toUpper());
    else
        Schema->setSelected(connection().user());
}
