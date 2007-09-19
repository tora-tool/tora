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

#include "tochangeconnection.h"
#include "toconf.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "toparamget.h"
#include "toresultbar.h"
#include "toresultcols.h"
#include "toresultcombo.h"
#include "toresultlong.h"
#include "toresultplan.h"
#include "toresultresources.h"
#include "toresultstats.h"
#include "toresultview.h"
#include "tosession.h"
#include "tosgatrace.h"
#include "totabwidget.h"
#include "totool.h"
#include "totabwidget.h"
#include "tovisualize.h"
#include "toworksheet.h"
#include "toworksheetsetupui.h"
#include "toworksheetstatistic.h"

#include <qaccel.h>
#include <qcheckbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qgrid.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qmultilineedit.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qprogressdialog.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qworkspace.h>

#include "toworksheet.moc"
#include "toworksheetsetupui.moc"

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

#define TO_ID_STATISTICS  (toMain::TO_TOOL_MENU_ID+ 0)
#define TO_ID_STOP   (toMain::TO_TOOL_MENU_ID+ 1)
#define TO_ID_PLAN   (toMain::TO_TOOL_MENU_ID+ 2)

#define CONF_AUTO_SAVE    "AutoSave"
#define CONF_CHECK_SAVE   "CheckSave"
#define CONF_AUTO_LOAD    "AutoLoad"
#define CONF_LOG_AT_END   "LogAtEnd"
#define CONF_LOG_MULTI    "LogMulti"
#define CONF_STATISTICS   "Statistics"
#define CONF_TIMED_STATS  "TimedStats"
#define CONF_NUMBER   "Number"
#define CONF_MOVE_TO_ERR  "MoveToError"
#define CONF_HISTORY   "History"
#define CONF_EXEC_LOG     "ExecLog"
#define CONF_TOPLEVEL_DESCRIBE "ToplevelDescribe"

class toWorksheetSetup : public toWorksheetSetupUI, public toSettingTab
{
    toTool *Tool;

public:
    toWorksheetSetup(toTool *tool, QWidget* parent = 0, const char* name = 0)
            : toWorksheetSetupUI(parent, name), toSettingTab("worksheet.html#preferences"), Tool(tool)
    {
        if (!tool->config(CONF_AUTO_SAVE, "").isEmpty())
            AutoSave->setChecked(true);
        if (!tool->config(CONF_CHECK_SAVE, "Yes").isEmpty())
            CheckSave->setChecked(true);
        if (!tool->config(CONF_LOG_AT_END, "Yes").isEmpty())
            LogAtEnd->setChecked(true);
        if (!tool->config(CONF_LOG_MULTI, "Yes").isEmpty())
            LogMulti->setChecked(true);
        MoveToError->setChecked(!tool->config(CONF_MOVE_TO_ERR, "Yes").isEmpty());
        if (!tool->config(CONF_STATISTICS, "").isEmpty())
            Statistics->setChecked(true);
        TimedStatistics->setChecked(!tool->config(CONF_TIMED_STATS, "Yes").isEmpty());
        History->setChecked(!tool->config(CONF_HISTORY, "").isEmpty());
        if (!tool->config(CONF_NUMBER, "Yes").isEmpty())
            DisplayNumber->setChecked(true);
        if (!tool->config(CONF_TOPLEVEL_DESCRIBE, "Yes").isEmpty())
            ToplevelDescribe->setChecked(true);
        DefaultFile->setText(tool->config(CONF_AUTO_LOAD, ""));
        ExecLog->setChecked(!tool->config(CONF_EXEC_LOG, "").isEmpty());
#ifdef TO_NO_ORACLE

        TimedStatistics->hide();
        MoveToError->hide();
        Statistics->hide();
#endif

    }
    virtual void saveSetting(void)
    {
        if (AutoSave->isChecked())
            Tool->setConfig(CONF_AUTO_SAVE, "Yes");
        else
            Tool->setConfig(CONF_AUTO_SAVE, "");
        if (CheckSave->isChecked())
            Tool->setConfig(CONF_CHECK_SAVE, "Yes");
        else
            Tool->setConfig(CONF_CHECK_SAVE, "");
        if (LogAtEnd->isChecked())
            Tool->setConfig(CONF_LOG_AT_END, "Yes");
        else
            Tool->setConfig(CONF_LOG_AT_END, "");
        if (LogMulti->isChecked())
            Tool->setConfig(CONF_LOG_MULTI, "Yes");
        else
            Tool->setConfig(CONF_LOG_MULTI, "");
        Tool->setConfig(CONF_TOPLEVEL_DESCRIBE, ToplevelDescribe->isChecked() ? "Yes" : "");
        Tool->setConfig(CONF_MOVE_TO_ERR, MoveToError->isChecked() ? "Yes" : "");
        Tool->setConfig(CONF_STATISTICS, Statistics->isChecked() ? "Yes" : "");
        Tool->setConfig(CONF_HISTORY, History->isChecked() ? "Yes" : "");
        Tool->setConfig(CONF_TIMED_STATS, TimedStatistics->isChecked() ? "Yes" : "");
        Tool->setConfig(CONF_NUMBER, DisplayNumber->isChecked() ? "Yes" : "");
        Tool->setConfig(CONF_EXEC_LOG, ExecLog->isChecked() ? "Yes" : "");
        Tool->setConfig(CONF_AUTO_LOAD, DefaultFile->text());
    }
public slots:
    void chooseFile(void)
    {
        QString str = toOpenFilename(DefaultFile->text(), QString::null, this);
        if (!str.isEmpty())
            DefaultFile->setText(str);
    }
};

class toWorksheetTool : public toTool
{
protected:
    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(toworksheet_xpm);
    }
public:
    toWorksheetTool()
            : toTool(10, "SQL Editor")
    { }
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
    virtual void closeWindow(toConnection &connection){};
};

static toWorksheetTool WorksheetTool;

class toWorksheetText : public toHighlightedText
{
    toWorksheet *Worksheet;
public:
    toWorksheetText(toWorksheet *worksheet, QWidget *parent, const char *name = NULL)
            : toHighlightedText(parent, name), Worksheet(worksheet)
    { }
    /** Reimplemented for internal reasons.
     */
    virtual void keyPressEvent(QKeyEvent *e)
    {
        if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toWorksheet", "Ctrl+Return", "Worksheet|Execute current"))))
        {
            Worksheet->execute();
            e->accept();
        }
        else if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toWorksheet", "F8", "Worksheet|Execute all"))))
        {
            Worksheet->executeAll();
            e->accept();
        }
        else if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toWorksheet", "F9", "Worksheet|Execute next"))))
        {
            Worksheet->executeStep();
            e->accept();
        }
        else if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toWorksheet", "Shift+F9", "Worksheet|Execute newline separated"))))
        {
            Worksheet->executeNewline();
            e->accept();
        }
        else if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toWorksheet", "F7", "Worksheet|Execute saved SQL"))))
        {
            Worksheet->executeSaved();
            e->accept();
        }
        else if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toWorksheet", "Shift+F7", "Worksheet|Insert saved SQL"))))
        {
            Worksheet->insertSaved();
            e->accept();
        }
        else if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toWorksheet", "F4", "Worksheet|Describe under cursor"))))
        {
            Worksheet->describe();
            e->accept();
        }
        else if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toWorksheet", "Alt+Up", "Worksheet|Previous log entry"))))
        {
            Worksheet->executePreviousLog();
            e->accept();
        }
        else if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toWorksheet", "Alt+Down", "Worksheet|Next log entry"))))
        {
            Worksheet->executeNextLog();
            e->accept();
        }
        else
        {
            toHighlightedText::keyPressEvent(e);
        }
    }
    virtual bool editSave(bool askfile)
    {
        bool ret = toHighlightedText::editSave(askfile);
        Worksheet->setCaption();
        return ret;
    }
    virtual bool editOpen(QString suggestedFile)
    {
        int ret = 1;
        if (isModified())
        {
            ret = TOMessageBox::information(this,
                                            qApp->translate("toWorksheetText", "Save changes?"),
                                            qApp->translate("toWorksheetText", "The editor has been changed. Do you want to save them,\n"
                                                            "discard changes or open file in new worksheet?"),
                                            qApp->translate("toWorksheetText", "&Save"),
                                            qApp->translate("toWorksheetText", "&Discard"),
                                            qApp->translate("toWorksheetText", "&New worksheet"), 0);
            if (ret < 0)
                return false;
            else if (ret == 0)
            {
                if (!editSave(false))
                    return false;
            }
        }

        QString fname;
        if (suggestedFile != QString::null)
            fname = suggestedFile;
        else
        {
            QFileInfo file(filename());
            fname = toOpenFilename(file.dirPath(), QString::null, this);
        }
        if (fname.isEmpty())
            return false;
        try
        {
            if (ret == 2)
                toWorksheet::fileWorksheet(fname);
            else
            {
                openFilename(fname);
                Worksheet->setCaption();
            }
            return true;
        }
        TOCATCH

        return false;
    }
};

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

void toWorksheet::setup(bool autoLoad)
{
    QToolBar *toolbar = toAllocBar(this, tr("SQL worksheet"));

    new QToolButton(QPixmap(const_cast<const char**>(execute_xpm)),
                    tr("Execute current statement"),
                    tr("Execute current statement"),
                    this, SLOT(execute(void)),
                    toolbar);
    new QToolButton(QPixmap(const_cast<const char**>(executestep_xpm)),
                    tr("Step through statements"),
                    tr("Step through statements"),
                    this, SLOT(executeStep(void)),
                    toolbar);
    new QToolButton(QPixmap(const_cast<const char**>(executeall_xpm)),
                    tr("Execute all statements"),
                    tr("Execute all statements"),
                    this, SLOT(executeAll(void)),
                    toolbar);
    toolbar->addSeparator();
    new QToolButton(QPixmap(const_cast<const char**>(refresh_xpm)),
                    tr("Reexecute Last Statement"),
                    tr("Reexecute Last Statement"),
                    this, SLOT(refreshSetup(void)),
                    toolbar);
    RefreshSeconds = 60;
    connect(&RefreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));

    LastLine = LastOffset = -1;
    LastID = 0;

    if (Light)
    {
        Editor = new toWorksheetText(this, this);
        Current = Result = new toResultLong(this);
        Result->hide();
        connect(Result, SIGNAL(done(void)), this, SLOT(queryDone(void)));
        connect(Result, SIGNAL(firstResult(const QString &, const toConnection::exception &, bool)),
                this, SLOT(addLog(const QString &, const toConnection::exception &, bool)));
        ResultTab = NULL;
        Plan = NULL;
        CurrentTab = NULL;
        Resources = NULL;
        Statistics = NULL;
        Logging = NULL;
        LastLogItem = NULL;
        StatisticButton = NULL;
        StatTab = NULL;
        Columns = NULL;
        Refresh = NULL;
        ToolMenu = NULL;
        Visualize = NULL;
        WaitChart = IOChart = NULL;
        toolbar->addSeparator();
        StopButton = new QToolButton(QPixmap(const_cast<const char**>(stop_xpm)),
                                     tr("Stop execution"),
                                     tr("Stop execution"),
                                     Result, SLOT(stop(void)),
                                     toolbar);
        StopButton->setEnabled(false);
        toolbar->setStretchableWidget(Started = new QLabel(toolbar, TO_TOOLBAR_WIDGET_NAME));
        Started->setAlignment(AlignRight | AlignVCenter | ExpandTabs);
    }
    else
    {
        QSplitter *splitter = new QSplitter(Vertical, this);

        Editor = new toWorksheetText(this, splitter);

	QValueList<int> list;
	list.append(75);
	splitter->setSizes(list);

        ResultTab = new toTabWidget(splitter);
        QVBox *box = new QVBox(ResultTab);
        ResultTab->addTab(box, tr("&Result"));

        Current = Result = new toResultLong(box);
        connect(Result, SIGNAL(done(void)), this, SLOT(queryDone(void)));
        connect(Result, SIGNAL(firstResult(const QString &, const toConnection::exception &, bool)),
                this, SLOT(addLog(const QString &, const toConnection::exception &, bool)));

        bool toplevel = !WorksheetTool.config(CONF_TOPLEVEL_DESCRIBE, "Yes").isEmpty();
        Columns = new toResultCols(box, "description", toplevel ? WType_TopLevel : 0);
        Columns->hide();

        ResultTab->setTabEnabled(Columns, false);
        Plan = new toResultPlan(ResultTab);
        ResultTab->addTab(Plan, tr("E&xecution plan"));

        ResourceSplitter = new QSplitter(Vertical, ResultTab);
        Resources = new toResultResources(ResourceSplitter);

        LongOps = new toResultLong(ResourceSplitter);

        Visualize = new toVisualize(Result, ResultTab);
        ResultTab->addTab(Visualize, tr("&Visualize"));
        ResultTab->addTab(ResourceSplitter, tr("&Information"));
        ResultTab->setTabShown(ResourceSplitter, Resources->handled());

        StatTab = new QVBox(ResultTab);
        {
            QToolBar *stattool = toAllocBar(StatTab, tr("Worksheet Statistics"));
            new QToolButton(QPixmap(const_cast<const char**>(filesave_xpm)),
                            tr("Save statistics for later analysis"),
                            tr("Save statistics for later analysis"),
                            this, SLOT(saveStatistics(void)),
                            stattool);
            stattool->setStretchableWidget(new QLabel(stattool));
        }
        splitter = new QSplitter(Horizontal, StatTab);
        Statistics = new toResultStats(true, splitter);
        Statistics->setTabWidget(ResultTab);
        WaitChart = new toResultBar(splitter);
        try
        {
            WaitChart->setSQL(toSQL::sql(TO_SESSION_WAIT));
        }
        catch (...)
        {}
        WaitChart->setTitle(tr("Wait states"));
        WaitChart->setYPostfix(QString::fromLatin1("ms/s"));
        WaitChart->setSamples( -1);
        WaitChart->start();
        connect(Statistics, SIGNAL(sessionChanged(const QString &)),
                WaitChart, SLOT(changeParams(const QString &)));
        IOChart = new toResultBar(splitter);
        try
        {
            IOChart->setSQL(toSQL::sql(TO_SESSION_IO));
        }
        catch (...)
        {}
        IOChart->setTitle(tr("I/O"));
        IOChart->setYPostfix(tr("blocks/s"));
        IOChart->setSamples( -1);
        IOChart->start();
        connect(Statistics, SIGNAL(sessionChanged(const QString &)),
                IOChart, SLOT(changeParams(const QString &)));
        ResultTab->addTab(StatTab, tr("&Statistics"));
        ResultTab->setTabEnabled(StatTab, false);

        Logging = new toListView(ResultTab);
        ResultTab->addTab(Logging, tr("&Logging"));
        Logging->addColumn(tr("SQL"));
        Logging->addColumn(tr("Result"));
        Logging->addColumn(tr("Timestamp"));
        Logging->addColumn(tr("Duration"));
        Logging->setColumnAlignment(3, AlignRight);
        Logging->setSelectionMode(QListView::Single);
        connect(Logging, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(executeLog()));
        LastLogItem = NULL;

        toolbar->addSeparator();
        new QToolButton(QPixmap(const_cast<const char**>(describe_xpm)),
                        tr("Describe under cursor"),
                        tr("Describe under cursor"),
                        this, SLOT(describe(void)),
                        toolbar);
        PlanButton = new QToolButton(QPixmap(const_cast<const char**>(explainplan_xpm)),
                                     tr("Explain plan of current statement"),
                                     tr("Explain plan of current statement"),
                                     this, SLOT(explainPlan(void)),
                                     toolbar);
        PlanButton->setEnabled(Plan->handled());
        StopButton = new QToolButton(QPixmap(const_cast<const char**>(stop_xpm)),
                                     tr("Stop execution"),
                                     tr("Stop execution"),
                                     this, SLOT(stop(void)),
                                     toolbar);
        StopButton->setEnabled(false);
        toolbar->addSeparator();
        new QToolButton(QPixmap(const_cast<const char**>(eraselog_xpm)),
                        tr("Clear execution log"),
                        tr("Clear execution log"),
                        this, SLOT(eraseLogButton(void)),
                        toolbar);

        toolbar->addSeparator();
        StatisticButton = new QToolButton(toolbar);
        StatisticButton->setToggleButton(true);
        StatisticButton->setIconSet(QIconSet(QPixmap(const_cast<const char**>(clock_xpm))));
        connect(StatisticButton, SIGNAL(toggled(bool)), this, SLOT(enableStatistic(bool)));
        QToolTip::add
            (StatisticButton, tr("Gather session statistic of execution"));
#ifdef TO_NO_ORACLE

        QLabel *label =
#endif
            new QLabel(tr("Refresh") + " ", toolbar, TO_TOOLBAR_WIDGET_NAME);
        Refresh = toRefreshCreate(toolbar, TO_TOOLBAR_WIDGET_NAME);
#ifdef TO_NO_ORACLE

        label->hide();
        Refresh->hide();
#else

        toolbar->addSeparator();
#endif

        connect(Refresh, SIGNAL(activated(const QString &)), this, SLOT(changeRefresh(const QString &)));
        connect(StatisticButton, SIGNAL(toggled(bool)), Refresh, SLOT(setEnabled(bool)));
        Refresh->setEnabled(false);
        Refresh->setFocusPolicy(NoFocus);

        new QToolButton(QPixmap(const_cast<const char**>(up_xpm)),
                        tr("Previous log entry"),
                        tr("Previous log entry"),
                        this, SLOT(executePreviousLog()),
                        toolbar);
        new QToolButton(QPixmap(const_cast<const char**>(down_xpm)),
                        tr("Next log entry"),
                        tr("Next log entry"),
                        this, SLOT(executeNextLog()),
                        toolbar);
        toolbar->addSeparator();

        InsertSavedButton = new toPopupButton(QPixmap(const_cast<const char**>(insertsaved_xpm)),
                                              tr("Insert current saved SQL"),
                                              tr("Insert current saved SQL"),
                                              toolbar);
        InsertSavedMenu = new QPopupMenu(InsertSavedButton);
        InsertSavedButton->setPopup(InsertSavedMenu);
        connect(InsertSavedMenu, SIGNAL(aboutToShow()), this, SLOT(showInsertSaved()));
        connect(InsertSavedMenu, SIGNAL(activated(int)), this, SLOT(insertSaved(int)));

        SavedButton = new toPopupButton(QPixmap(const_cast<const char**>(recall_xpm)),
                                        tr("Run current saved SQL"),
                                        tr("Run current saved SQL"),
                                        toolbar);
        SavedMenu = new QPopupMenu(SavedButton);
        SavedButton->setPopup(SavedMenu);
        connect(SavedMenu, SIGNAL(aboutToShow()), this, SLOT(showSaved()));
        connect(SavedMenu, SIGNAL(activated(int)), this, SLOT(executeSaved(int)));

        new QToolButton(QPixmap(const_cast<const char**>(previous_xpm)),
                        tr("Save last SQL"),
                        tr("Save last SQL"),
                        this, SLOT(saveLast(void)),
                        toolbar);

        toolbar->setStretchableWidget(Started = new QLabel(toolbar, TO_TOOLBAR_WIDGET_NAME));
        Started->setAlignment(AlignRight | AlignVCenter | ExpandTabs);

        Schema = new toResultCombo(toolbar);
        Schema->setSQL(toSQL::sql(toSQL::TOSQL_USERLIST));
        if (toIsMySQL(connection()))
            Schema->setSelected(connection().database());
        else if (toIsOracle(connection()) || toIsSapDB(connection()))
            Schema->setSelected(connection().user().upper());
        else
            Schema->setSelected(connection().user());
        connect(Schema, SIGNAL(activated(int)), this, SLOT(changeSchema()));
        try
        {
            Schema->refresh();
        }
        catch (...)
        {}

        new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

        connect(ResultTab, SIGNAL(currentChanged(QWidget *)),
                this, SLOT(changeResult(QWidget *)));

        if (autoLoad)
        {
            Editor->setFilename(WorksheetTool.config(CONF_AUTO_LOAD, ""));
            if (!Editor->filename().isEmpty())
            {
                try
                {
                    QCString data = toReadFile(Editor->filename());
                    Editor->setText(QString::fromLocal8Bit(data));
                    Editor->setModified(false);
                }
                TOCATCH
            }
        }

        ToolMenu = NULL;
        connect(toMainWidget()->workspace(), SIGNAL(windowActivated(QWidget *)),
                this, SLOT(windowActivated(QWidget *)));

        try
        {
            if (connection().provider() == "Oracle")
            {
                if (!WorksheetTool.config(CONF_STATISTICS, "").isEmpty())
                {
                    show();
                    StatisticButton->setOn(true);
                }
            }
            else
            {
                StatisticButton->setShown(false);
            }
        }
        TOCATCH

        connect(this, SIGNAL(connectionChange()), this, SLOT(connectionChanged()));
    }
    Editor->setAnalyzer(connection().analyzer());
    connect(Editor, SIGNAL(displayMenu(QPopupMenu *)), this, SLOT(displayMenu(QPopupMenu *)));

    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
    setFocusProxy(Editor);
}

toWorksheet::toWorksheet(QWidget *main, toConnection &connection, bool autoLoad)
        : toToolWidget(WorksheetTool, "worksheet.html", main, connection), Light(false)
{
    setup(autoLoad);
}

toWorksheet::toWorksheet(QWidget *main, const char *name, toConnection &connection)
        : toToolWidget(WorksheetTool, "worksheetlight.html", main, connection, name), Light(true)
{
    setup(false);
}

void toWorksheet::changeRefresh(const QString &str)
{
    try
    {
        if (!Light && StopButton->isEnabled() && StatisticButton->isOn())
            toRefreshParse(timer(), str);
    }
    TOCATCH
}

void toWorksheet::windowActivated(QWidget *widget)
{
    if (Light)
        return ;

    QWidget *w = this;
    while (w && w != widget)
    {
        w = w->parentWidget();
    }

    if (widget == w)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QPopupMenu(this);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(execute_xpm)),
                                 tr("&Execute Current"), this, SLOT(execute(void)),
                                 toKeySequence(tr("Ctrl+Return", "Worksheet|Execute current")));
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(executestep_xpm)),
                                 tr("Execute &Next"), this, SLOT(executeStep(void)),
                                 toKeySequence(tr("F9", "Worksheet|Execute next")));
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(executeall_xpm)),
                                 tr("Execute &All"), this, SLOT(executeAll(void)),
                                 toKeySequence(tr("F8", "Worksheet|Execute all")));
            ToolMenu->insertItem(tr("Execute &Newline Separated"), this,
                                 SLOT(executeNewline(void)),
                                 toKeySequence(tr("Shift+F9", "Worksheet|Execute newline separated")));
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(refresh_xpm)),
                                 tr("&Reexecute Last Statement"), this, SLOT(refresh(void)),
                                 toKeySequence(tr("F5", "Worksheet|Reexecute last statement")));
            if (connection().provider() == "Oracle")
                ToolMenu->insertItem(tr("Check syntax of buffer"),
                                     this, SLOT(parseAll()),
                                     toKeySequence(tr("Ctrl+F9", "Worksheet|Check syntax of buffer")));
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(describe_xpm)),
                                 tr("&Describe Under Cursor"), this, SLOT(describe(void)),
                                 toKeySequence(tr("F4", "Worksheet|Describe under cursor")));
            ToolMenu->insertItem(tr("&Explain current statement"), this, SLOT(explainPlan(void)),
                                 toKeySequence(tr("F3", "Worksheet|Explain plan")), TO_ID_PLAN);
            if (connection().provider() == "Oracle")
                ToolMenu->insertItem(tr("&Enable Statistics"), this, SLOT(toggleStatistic(void)),
                                     0, TO_ID_STATISTICS);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(stop_xpm)),
                                 tr("&Stop Execution"), Result, SLOT(stop(void)),
                                 0, TO_ID_STOP);
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(tr("Execute Saved SQL"),
                                 this, SLOT(executeSaved()),
                                 toKeySequence(tr("F7", "Worksheet|Execute saved SQL")));
            ToolMenu->insertItem(tr("Select Saved SQL"),
                                 this, SLOT(selectSaved()),
                                 toKeySequence(tr("Ctrl+Shift+S", "Worksheet|Select saved SQL")));
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(previous_xpm)),
                                 tr("Save last SQL"),
                                 this, SLOT(saveLast()));
            ToolMenu->insertItem(tr("Edit Saved SQL..."),
                                 this, SLOT(editSaved()));
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(tr("Previous Log Entry"), this, SLOT(executePreviousLog()),
                                 toKeySequence(tr("Alt+Up", "Worksheet|Previous log entry")));
            ToolMenu->insertItem(tr("Next Log Entry"), this, SLOT(executeNextLog()),
                                 toKeySequence(tr("Alt+Down", "Worksheet|Next log entry")));
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(eraselog_xpm)),
                                 tr("Erase &Log"), this, SLOT(eraseLogButton(void)));


            toMainWidget()->menuBar()->insertItem(tr("Edit&or"), ToolMenu, -1, toToolMenuIndex());
            ToolMenu->setItemEnabled(TO_ID_STOP, StopButton->isEnabled());
            ToolMenu->setItemChecked(TO_ID_STATISTICS, StatisticButton->isOn());
            ToolMenu->setItemEnabled(TO_ID_PLAN, Plan->handled());
        }
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
        StatisticButton->setShown(connection().provider() == "Oracle");
        ResultTab->setTabShown(ResourceSplitter, Resources->handled());
        Editor->setAnalyzer(connection().analyzer());
        PlanButton->setEnabled(Plan->handled());
        delete ToolMenu;
        ToolMenu = NULL;
        windowActivated(this);
    }
    TOCATCH
}

bool toWorksheet::checkSave(bool input)
{
    if (Light)
        return true;
    if (Editor->isModified())
    {
        if (WorksheetTool.config(CONF_AUTO_SAVE, "").isEmpty() ||
                Editor->filename().isEmpty())
        {
            if (!WorksheetTool.config(CONF_CHECK_SAVE, "Yes").isEmpty())
            {
                if (input)
                {
                    QString conn;
                    try
                    {
                        conn = connection().description();
                    }
                    catch (...)
                    {
                        conn += QString::fromLatin1("unknown connection");
                    }
                    QString str = tr("Save changes to editor for %1").arg(conn);
                    if (!Editor->filename().isEmpty())
                        str += QString::fromLatin1("\n(") + Editor->filename() + QString::fromLatin1(")");
                    int ret = TOMessageBox::information(this,
                                                        tr("Save file"),
                                                        str,
                                                        tr("&Yes"), tr("&No"), tr("Cancel"), 0, 2);
                    if (ret == 1)
                        return true;
                    else if (ret == 2)
                        return false;
                }
                else
                    return true;
            }
            else
                return true;
            if (Editor->filename().isEmpty() && input)
                Editor->setFilename(toSaveFilename(Editor->filename(), QString::null, this));
            if (Editor->filename().isEmpty())
                return false;
        }
        if (!toWriteFile(Editor->filename(), Editor->text()))
            return false;
        Editor->setModified(false);
    }
    return true;
}

bool toWorksheet::close(bool del)
{
    if (checkSave(true))
    {
        Result->stop();
        return QVBox::close(del);
    }
    return false;
}

toWorksheet::~toWorksheet()
{
    checkSave(false);
    eraseLogButton();
}

#define LARGE_BUFFER 4096

void toWorksheet::changeResult(QWidget *widget)
{
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
        query(QueryString, Normal);
    if (RefreshSeconds > 0)
        RefreshTimer.start(RefreshSeconds*1000, true);
}

static QString unQuote(const QString &str)
{
    if (str.at(0).latin1() == '\"' && str.at(str.length() - 1).latin1() == '\"')
        return str.left(str.length() - 1).right(str.length() - 2);
    return str.upper();
}

bool toWorksheet::describe(const QString &query)
{
    try
    {
        QRegExp white(QString::fromLatin1("[ \r\n\t.]+"));
        QStringList part = QStringList::split(white, query);
        if (part[0].upper() == QString::fromLatin1("DESC") ||
                part[0].upper() == QString::fromLatin1("DESCRIBE"))
        {
            if (Light)
                return true;
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
            if (Light)
                return false;
            QWidget *curr = ResultTab->currentPage();
            Current->hide();
            Result->show();
            Current = Result;
            if (curr == Columns)
                ResultTab->showPage(Result);
            return false;
        }
    }
    TOCATCH
    return false;
}

void toWorksheet::query(const QString &str, execType type)
{
    Result->stop();
    RefreshTimer.stop();

    QRegExp strq(QString::fromLatin1("'[^']*'"));
    QString chk = str.lower();
    chk.replace(strq, QString::fromLatin1(" "));
    bool code = false;
    static QRegExp codere(QString::fromLatin1("[^a-z0-9]end\\s+[a-z0-9_-]*;$"), true);
    static QRegExp codere2(QString::fromLatin1("[^a-z0-9]end;"), true);

    if (codere.match(chk) >= 0 || codere2.match(chk) >= 0)
        code = true;

    QueryString = str;
    if (!code && QueryString.length() > 0 && QueryString.at(QueryString.length() - 1) == ';')
        QueryString.truncate(QueryString.length() - 1);

    bool nobinds = false;
    chk = str.lower();
    chk.replace(strq, QString::fromLatin1(" "));
    chk = chk.simplifyWhiteSpace();
    chk.replace(QRegExp(QString::fromLatin1(" or replace ")), QString::fromLatin1(" "));
    if (chk.startsWith(QString::fromLatin1("create trigger ")))
        nobinds = true;

    if (type == OnlyPlan)
    {
        ResultTab->showPage(Plan);
        Plan->query(str);
    }
    else if (!describe(QueryString))
    {

        toSQLParse::stringTokenizer tokens(str);
        QString first = tokens.getToken(true).upper();
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
                if (WorksheetTool.config(CONF_HISTORY, "").isEmpty() && !Light)
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
                        query->query(QueryString, param);
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
            }
        }
        else
        {
            First = false;
            Timer.start();
            StopButton->setEnabled(true);
            Poll.start(1000);
            QToolTip::add
                (Started, tr("Duration while query has been running\n\n") + QueryString);
            if (ToolMenu)
                ToolMenu->setItemEnabled(TO_ID_STOP, true);
            Result->setNumberColumn(!WorksheetTool.config(CONF_NUMBER, "Yes").isEmpty());
            try
            {
                saveHistory();
                Result->setSQL(QString::null);
                Result->query(QueryString, param);
                if (Light)
                    return ;
                if (CurrentTab == Visualize)
                    Visualize->display();
                else if (CurrentTab == Plan)
                    Plan->query(QueryString);
                else if (CurrentTab == ResourceSplitter)
                    viewResources();
            }
            catch (const toConnection::exception &exc)
            {
                addLog(QueryString, exc, true);
            }
            catch (const QString &exc)
            {
                addLog(QueryString, exc, true);
            }
            if (!Light)
            {
                try
                {
                    if (StatisticButton->isOn())
                        toRefreshParse(timer(), Refresh->currentText());
                }
                TOCATCH
            }
            Result->setSQLName(QueryString.simplifyWhiteSpace().left(40));
        }
    }
}

void toWorksheet::saveHistory(void)
{
    if (WorksheetTool.config(CONF_HISTORY, "").isEmpty())
        return ;
    if (Result->firstChild() && Current == Result && !Light)
    {
        History[LastID] = Result;
        Result->hide();
        Result->stop();
        disconnect(Result, SIGNAL(done(void)), this, SLOT(queryDone(void)));
        disconnect(Result, SIGNAL(firstResult(const QString &, const toConnection::exception &, bool)),
                   this, SLOT(addLog(const QString &, const toConnection::exception &, bool)));
        disconnect(StopButton, SIGNAL(clicked(void)), Result, SLOT(stop(void)));

        Result = new toResultLong(Result->parentWidget());
        if (StatisticButton->isOn())
            enableStatistic(true);
        Result->show();
        Current = Result;
        connect(StopButton, SIGNAL(clicked(void)), Result, SLOT(stop(void)));
        connect(Result, SIGNAL(done(void)), this, SLOT(queryDone(void)));
        connect(Result, SIGNAL(firstResult(const QString &, const toConnection::exception &, bool)),
                this, SLOT(addLog(const QString &, const toConnection::exception &, bool)));
    }
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

void toWorksheet::addLog(const QString &sql, const toConnection::exception &result, bool error)
{
    QString now;
    try
    {
        now = toNow(connection());
    }
    catch (...)
    {
        now = QString::fromLatin1("Unknown");
    }
    toResultViewItem *item = NULL;

    LastID++;

    int dur = 0;
    if (!Timer.isNull())
        dur = Timer.elapsed();
    First = true;

    if (!Light)
    {
        if (WorksheetTool.config(CONF_LOG_MULTI, "Yes").isEmpty())
        {
            if (WorksheetTool.config(CONF_LOG_AT_END, "Yes").isEmpty())
                item = new toResultViewItem(Logging, NULL);
            else
                item = new toResultViewItem(Logging, LastLogItem);
        }
        else if (WorksheetTool.config(CONF_LOG_AT_END, "Yes").isEmpty())
            item = new toResultViewMLine(Logging, NULL);
        else
            item = new toResultViewMLine(Logging, LastLogItem);
        item->setText(0, sql);

        LastLogItem = item;
        item->setText(1, result);
        item->setText(2, now);
        if (!WorksheetTool.config(CONF_HISTORY, "").isEmpty())
            item->setText(4, QString::number(LastID));
        item->setText(5, QString::number(result.offset()));
    }

    if (result.offset() >= 0 && LastLine >= 0 && LastOffset >= 0 &&
            !WorksheetTool.config(CONF_MOVE_TO_ERR, "Yes").isEmpty())
    {
        QChar cmp = '\n';
        int lastnl = 0;
        int lines = 0;
        for (int i = 0;i < result.offset();i++)
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

    if (!Light)
    {
        item->setText(3, buf);

        QListViewItem *last = Logging->currentItem();
        toResultViewItem *citem = NULL;
        if (last)
            citem = dynamic_cast<toResultViewItem *>(last);
        if (!citem || citem->allText(0) != sql)
        {
            disconnect(Logging, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(executeLog()));
            Logging->setSelected(item, true);
            connect(Logging, SIGNAL(selectionChanged(QListViewItem *)), this, SLOT(executeLog()));
            Logging->ensureItemVisible(item);
        }
    }

    {
        QString str = result;
        str += "\n" + tr("(Duration %1)").arg(buf);
        if (error)

            toStatusMessage(str);
        else
            toStatusMessage(str, false, false);
    }
    if (!Light && !error)
        changeResult(CurrentTab);

    static QRegExp re(QString::fromLatin1("^[1-9]\\d* rows processed$"));
    try
    {
        if (result.contains(re))
        {
            if (!toConfigurationSingle::Instance().globalConfig(CONF_AUTO_COMMIT, "").isEmpty())
                connection().commit();
            else
                toMainWidget()->setNeedCommit(connection());
        }
    }
    TOCATCH
    saveDefaults();
}

void toWorksheet::execute(toSQLParse::tokenizer &tokens, int line, int pos, execType type)
{
    LastLine = line;
    LastOffset = pos;
    int endLine,endCol;
    if(Editor->lines()<=tokens.line()){
     endLine=Editor->lines()-1;
     endCol=Editor->lineLength(Editor->lines()-1);
    }else{
      endLine=tokens.line();
      if(Editor->lineLength(tokens.line())<=tokens.offset())
        endCol=Editor->lineLength(tokens.line());
      else{
        endCol=tokens.offset();
      }
    }
    Editor->setSelection(line, pos, endLine,endCol);
    QString t = Editor->selectedText();

    bool comment = false;
    bool multiComment = false;
    int oline = line;
    int opos = pos;
    unsigned int i;

    for (i = 0;i < t.length() - 1;i++)
    {
        if (comment)
        {
            if (t.at(i).latin1() == '\n')
                comment = false;
        }
        else if (multiComment)
        {
            if (t.at(i).latin1() == '*' &&
                    t.at(i + 1).latin1() == '/')
            {
                multiComment = false;
                i++;
            }
        }
        else if (t.at(i).latin1() == '-' &&
                 t.at(i + 1).latin1() == '-')
            comment = true;
        else if (t.at(i).latin1() == '/' &&
                 t.at(i + 1).latin1() == '/')
            comment = true;
        else if (t.at(i).latin1() == '/' &&
                 t.at(i + 1).latin1() == '*')
            multiComment = true;
        else if (!t.at(i).isSpace() && t.at(i) != '/')
            break;

        if (t.at(i).latin1() == '\n')
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
    if (t.length())
        query(t, type);
}

void toWorksheet::execute()
{
    if (Editor->hasSelectedText())
    {
        query(Editor->selectedText(), Normal);
        return ;
    }

    toSQLParse::editorTokenizer tokens(Editor);

    int cpos, cline;
    Editor->getCursorPosition(&cline, &cpos);

    int line;
    int pos;
    do
    {
        line = tokens.line();
        pos = tokens.offset();
        toSQLParse::parseStatement(tokens);
    }
    while (tokens.line() < cline ||
            (tokens.line() == cline && tokens.offset() < cpos));

    execute(tokens, line, pos, Normal);
}

void toWorksheet::explainPlan()
{
    if (Editor->hasSelectedText())
    {
        query(Editor->selectedText(), OnlyPlan);
        return ;
    }

    toSQLParse::editorTokenizer tokens(Editor);

    int cpos, cline;
    Editor->getCursorPosition(&cline, &cpos);

    int line;
    int pos;
    do
    {
        line = tokens.line();
        pos = tokens.offset();
        toSQLParse::parseStatement(tokens);
    }
    while (tokens.line() < cline ||
            (tokens.line() == cline && tokens.offset() < cpos));

    execute(tokens, line, pos, OnlyPlan);
}

void toWorksheet::executeStep()
{
    toSQLParse::editorTokenizer tokens(Editor);

    int cpos, cline;
    Editor->getCursorPosition(&cline, &cpos);

    int line;
    int pos;
    do
    {
        line = tokens.line();
        pos = tokens.offset();
        toSQLParse::parseStatement(tokens);
    }
    while (tokens.line() < cline ||
            (tokens.line() == cline && tokens.offset() <= cpos));

    execute(tokens, line, pos, Normal);
}

void toWorksheet::executeAll()
{
    toSQLParse::editorTokenizer tokens(Editor);

    int cpos, cline;
    Editor->getCursorPosition(&cline, &cpos);

    QProgressDialog dialog(tr("Executing all statements"),
                           tr("Cancel"),
                           Editor->lines(),
                           this,
                           "Progress",
                           true);
    int line;
    int pos;
    bool ignore = true;
    do
    {
        line = tokens.line();
        pos = tokens.offset();
        dialog.setProgress(line);
        qApp->processEvents();
        if (dialog.wasCancelled())
            break;
        toSQLParse::parseStatement(tokens);

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
            execute(tokens, line, pos, Direct);
            if (Current)
            {
                toResultView *last = dynamic_cast<toResultView *>(Current);
                if (!WorksheetTool.config(CONF_HISTORY, "").isEmpty() &&
                        last && last->firstChild())
                    History[LastID] = last;
            }
        }
    }
    while (tokens.line() < Editor->lines());

    Editor->setSelection(cline, cpos, tokens.line(), tokens.offset());
}

void toWorksheet::parseAll()
{
    toSQLParse::editorTokenizer tokens(Editor);

    int cpos, cline;
    Editor->getCursorPosition(&cline, &cpos);

    QProgressDialog dialog(tr("Parsing all statements"),
                           tr("Cancel"),
                           Editor->lines(),
                           this,
                           "Progress",
                           true);
    int line;
    int pos;
    bool ignore = true;
    do
    {
        line = tokens.line();
        pos = tokens.offset();
        dialog.setProgress(line);
        qApp->processEvents();
        if (dialog.wasCancelled())
            break;
        toSQLParse::parseStatement(tokens);

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
            execute(tokens, line, pos, Parse);
            if (Current)
            {
                toResultView *last = dynamic_cast<toResultView *>(Current);
                if (!WorksheetTool.config(CONF_HISTORY, "").isEmpty() &&
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
    if (Light)
        return ;
    Logging->clear();
    LastLogItem = NULL;
    for (std::map<int, QWidget *>::iterator i = History.begin();i != History.end();i++)
        delete (*i).second;
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
    StopButton->setEnabled(false);
    Poll.stop();
    if (ToolMenu)
        ToolMenu->setItemEnabled(TO_ID_STOP, false);
    saveDefaults();
}

void toWorksheet::saveDefaults(void)
{
    QListViewItem *item = Result->firstChild();
    if (item)
    {
        QHeader *head = Result->header();
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
                toParamGet::setDefault(connection(), head->label(i).lower(), toUnnull(toQValue(str)));
            }
            TOCATCH
        }
    }
}

#define ENABLETIMED "ALTER SESSION SET TIMED_STATISTICS = TRUE"

void toWorksheet::enableStatistic(bool ena)
{
    if (ena)
    {
        Result->setStatistics(Statistics);
        ResultTab->setTabEnabled(StatTab, true);
        if (ToolMenu)
            ToolMenu->setItemChecked(TO_ID_STATISTICS, true);
        Statistics->clear();
        if (!WorksheetTool.config(CONF_TIMED_STATS, "Yes").isEmpty())
        {
            try
            {
                connection().allExecute(QString::fromLatin1(ENABLETIMED));
                connection().addInit(QString::fromLatin1(ENABLETIMED));
            }
            TOCATCH
        }
    }
    else
    {
        try
        {
            connection().delInit(QString::fromLatin1(ENABLETIMED));
        }
        catch (...)
        {}
        Result->setStatistics(NULL);
        ResultTab->setTabEnabled(StatTab, false);
        if (ToolMenu)
            ToolMenu->setItemChecked(TO_ID_STATISTICS, false);
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
        QString data = Editor->text(cline).simplifyWhiteSpace();
        if (data.length() == 0 || data == QString::fromLatin1(" "))
        {
            cline++;
            break;
        }
        cline--;
    }

    while (cline < Editor->lines())
    {
        QString data = Editor->text(cline).simplifyWhiteSpace();
        if (data.length() != 0 && data != QString::fromLatin1(" "))
            break;
        cline++;
    }

    int eline = cline;

    while (eline < Editor->lines())
    {
        QString data = Editor->text(eline).simplifyWhiteSpace();
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
        query(Editor->selectedText(), Normal);
}

void toWorksheet::describe(void)
{
    if (Light)
        return ;

    QString owner, table;
    Editor->tableAtCursor(owner, table);

    if (owner.isNull())
        Columns->changeParams(table);
    else
        Columns->changeParams(owner, table);
    if (!Columns->isTopLevel())
        Current->hide();
    Columns->show();
    Current = Columns;
}

void toWorksheet::executeSaved(void)
{
    if (Light)
        return ;

    LastLine = LastOffset = -1;

    if (SavedLast.length() > 0)
    {
        try
        {
            query(toSQL::string(SavedLast, connection()), Normal);
        }
        TOCATCH
    }
}

void toWorksheet::insertSaved(void)
{
    if (Light)
        return ;

    LastLine = LastOffset = -1;

    if (InsertSavedLast.length() > 0)
    {
        try
        {
            Editor->setText(toSQL::string(InsertSavedLast, connection()));
        }
        TOCATCH
    }
}

void toWorksheet::executeSaved(int id)
{
    std::list<QCString> def = toSQL::range(TOWORKSHEET);
    for (std::list<QCString>::iterator i = def.begin();i != def.end();i++)
    {
        id--;
        if (id == 0)
        {
            SavedLast = *i;
            executeSaved();
            break;
        }
    }
}

void toWorksheet::insertSaved(int id)
{
    std::list<QCString> def = toSQL::range(TOWORKSHEET);
    for (std::list<QCString>::iterator i = def.begin();i != def.end();i++)
    {
        id--;
        if (id == 0)
        {
            InsertSavedLast = *i;
            insertSaved();
            break;
        }
    }
}

void toWorksheet::showSaved(void)
{
    static QRegExp colon(QString::fromLatin1(":"));
    std::list<QCString> def = toSQL::range(TOWORKSHEET);
    SavedMenu->clear();
    std::map<QString, QPopupMenu *> menues;
    int id = 0;
    for (std::list<QCString>::iterator sql = def.begin();sql != def.end();sql++)
    {

        id++;

        QStringList spl = QStringList::split(colon, QString::fromLatin1(*sql));
        spl.remove(spl.begin());

        if (spl.count() > 0)
        {
            QString name = spl.last();
            spl.remove(spl.fromLast());

            QPopupMenu *menu;
            if (spl.count() == 0)
                menu = SavedMenu;
            else
            {
                QStringList exs = spl;
                while (exs.count() > 0 && menues.find(exs.join(QString::fromLatin1(":"))) == menues.end())
                    exs.remove(exs.fromLast());
                if (exs.count() == 0)
                    menu = SavedMenu;
                else
                    menu = menues[exs.join(QString::fromLatin1(":"))];
                QString subname = exs.join(QString::fromLatin1(":"));
                for (unsigned int i = exs.count();i < spl.count();i++)
                {
                    QPopupMenu *next = new QPopupMenu(this);
                    connect(next, SIGNAL(activated(int)), this, SLOT(executeSaved(int)));
                    if (i != 0)
                        subname += QString::fromLatin1(":");
                    subname += spl[i];
                    menu->insertItem(spl[i], next);
                    menu = next;
                    menues[subname] = menu;
                }
            }
            menu->insertItem(name, id);
        }
    }
}

void toWorksheet::showInsertSaved(void)
{
    static QRegExp colon(QString::fromLatin1(":"));
    std::list<QCString> def = toSQL::range(TOWORKSHEET);
    InsertSavedMenu->clear();
    std::map<QString, QPopupMenu *> menues;
    int id = 0;
    for (std::list<QCString>::iterator sql = def.begin();sql != def.end();sql++)
    {

        id++;

        QStringList spl = QStringList::split(colon, QString::fromLatin1(*sql));
        spl.remove(spl.begin());

        if (spl.count() > 0)
        {
            QString name = spl.last();
            spl.remove(spl.fromLast());

            QPopupMenu *menu;
            if (spl.count() == 0)
                menu = InsertSavedMenu;
            else
            {
                QStringList exs = spl;
                while (exs.count() > 0 && menues.find(exs.join(QString::fromLatin1(":"))) == menues.end())
                    exs.remove(exs.fromLast());
                if (exs.count() == 0)
                    menu = InsertSavedMenu;
                else
                    menu = menues[exs.join(QString::fromLatin1(":"))];
                QString subname = exs.join(QString::fromLatin1(":"));
                for (unsigned int i = exs.count();i < spl.count();i++)
                {
                    QPopupMenu *next = new QPopupMenu(this);
                    connect(next, SIGNAL(activated(int)), this, SLOT(insertSaved(int)));
                    if (i != 0)
                        subname += QString::fromLatin1(":");
                    subname += spl[i];
                    menu->insertItem(spl[i], next);
                    menu = next;
                    menues[subname] = menu;
                }
            }
            menu->insertItem(name, id);
        }
    }
}


void toWorksheet::editSaved(void)
{
    QCString sql = TOWORKSHEET;
    sql += "Untitled";
    toMainWidget()->editSQL(QString::fromLatin1(sql));
}

void toWorksheet::selectSaved()
{
    SavedMenu->popup(SavedButton->mapToGlobal(QPoint(0, SavedButton->height())));
}

void toWorksheet::insertStatement(const QString &str)
{
    QString txt = Editor->text();

    int i = txt.find(str);

    if (i >= 0)
    {
        int startCol, endCol;
        int startRow, endRow;
        
        Editor->findPosition(i, startRow, startCol);
        Editor->findPosition(i + str.length(), endRow, endCol);
        
        if (Editor->text(endRow).at(endCol) == ';')
            endCol++;
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
    if (Light)
        return ;

    Result->stop();

    LastLine = LastOffset = -1;
    saveHistory();

    QListViewItem *item = Logging->currentItem();
    if (item)
    {
        QListViewItem *pt = Logging->firstChild();
        while (pt && pt->nextSibling() != item)
            pt = pt->nextSibling();

        if (pt)
            Logging->setSelected(pt, true);
    }
}

void toWorksheet::executeLog(void)
{
    if (Light)
        return ;

    Result->stop();

    LastLine = LastOffset = -1;
    saveHistory();

    QListViewItem *ci = Logging->currentItem();
    toResultViewItem *item = dynamic_cast<toResultViewItem *>(ci);
    if (item)
    {
        insertStatement(item->allText(0));

        if (item->text(4).isEmpty())
        {
            if (!WorksheetTool.config(CONF_EXEC_LOG, "").isEmpty())
                query(item->allText(0), Normal);
        }
        else
        {
            std::map<int, QWidget *>::iterator i = History.find(item->text(4).toInt());
            QueryString = item->allText(0);
            changeResult(ResultTab->currentPage());
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
    if (Light)
        return ;

    Result->stop();

    LastLine = LastOffset = -1;
    saveHistory();

    QListViewItem *item = Logging->currentItem();
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

void toWorksheet::saveLast(void)
{
    if (QueryString.isEmpty())
    {
        TOMessageBox::warning(this, tr("No SQL to save"),
                              tr("You haven't executed any SQL yet"),
                              tr("&Ok"));
        return ;
    }
    bool ok = false;
    QCString name = QInputDialog::getText(tr("Enter title"),
                                          tr("Enter the title in the menu of the saved SQL,\n"
                                             "submenues are separated by a ':' character."),
                                          QLineEdit::Normal, QString::null, &ok, this).latin1();
    if (ok && !name.isEmpty())
    {
        try
        {
            toSQL::updateSQL(TOWORKSHEET + name,
                             QueryString,
                             tr("Undescribed"),
                             "Any",
                             connection().provider());
            toSQL::saveSQL(toConfigurationSingle::Instance().globalConfig(CONF_SQL_FILE, DEFAULT_SQL_FILE));
        }
        TOCATCH
    }
}

void toWorksheet::saveStatistics(void)
{
    std::map<QCString, QString> stat;

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

void toWorksheet::exportData(std::map<QCString, QString> &data, const QCString &prefix)
{
    Editor->exportData(data, prefix + ":Edit");
    if (StatisticButton->isOn())
        data[prefix + ":Stats"] = Refresh->currentText();
    toToolWidget::exportData(data, prefix);
}

void toWorksheet::importData(std::map<QCString, QString> &data, const QCString &prefix)
{
    Editor->importData(data, prefix + ":Edit");
    QString stat = data[prefix + ":Stats"];
    if (stat)
    {
        for (int i = 0;i < Refresh->count();i++)
        {
            if (Refresh->text(i) == stat)
            {
                Refresh->setCurrentItem(i);
                break;
            }
        }
        StatisticButton->setOn(true);
    }
    else
        StatisticButton->setOn(false);

    toToolWidget::importData(data, prefix);
    setCaption();
}

void toWorksheet::setCaption(void)
{
    QString name = WorksheetTool.name();
    if (! Editor->filename().isEmpty())
    {
        QFileInfo file(Editor->filename());
        name += QString::fromLatin1(" ") + file.fileName();
    }
    toToolCaption(this, name);
}

toWorksheet *toWorksheet::fileWorksheet(const QString &file)
{
    toWorksheet *worksheet = new toWorksheet(toMainWidget()->workspace(),
                             toMainWidget()->currentConnection(),
                             false);
    worksheet->editor()->openFilename(file);
    worksheet->setCaption();
    worksheet->show();
    toMainWidget()->windowsMenu();
    return worksheet;
}

void toWorksheet::refreshSetup(void)
{
    bool ok = false;
    int num = QInputDialog::getInteger(tr("Enter refreshrate"),
                                       tr("Refresh rate of query in seconds"),
                                       RefreshSeconds, 0, 1000000, 1, &ok, this);
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

void toWorksheet::displayMenu(QPopupMenu *menu)
{
    menu->insertSeparator(0);
    if (!Light)
    {
        menu->insertItem(tr("&Explain current statement"), this, SLOT(explainPlan(void)),
                         toKeySequence(tr("F3", "Worksheet|Explain plan")), TO_ID_PLAN, 0);
        menu->insertItem(QPixmap(const_cast<const char**>(describe_xpm)),
                         tr("&Describe Under Cursor"), this, SLOT(describe(void)),
                         toKeySequence(tr("F4", "Worksheet|Describe under cursor")), 0, 0);
        menu->insertSeparator(0);
    }
    if (connection().provider() == "Oracle")
        menu->insertItem(tr("Check syntax of buffer"),
                         this, SLOT(parseAll()),
                         toKeySequence(tr("Ctrl+F9", "Worksheet|Check syntax of buffer")), 0, 0);
    menu->insertItem(QPixmap(const_cast<const char**>(refresh_xpm)),
                     tr("&Reexecute Last Statement"), this, SLOT(refresh(void)),
                     toKeySequence(tr("F5", "Worksheet|Reexecute last statement")), 0, 0);
    menu->insertItem(tr("Execute &Newline Separated"), this,
                     SLOT(executeNewline(void)),
                     toKeySequence(tr("Shift+F9", "Worksheet|Execute newline separated")), 0, 0);
    menu->insertItem(QPixmap(const_cast<const char**>(executeall_xpm)),
                     tr("Execute &All"), this, SLOT(executeAll(void)),
                     toKeySequence(tr("F8", "Worksheet|Execute all")), 0, 0);
    menu->insertItem(QPixmap(const_cast<const char**>(executestep_xpm)),
                     tr("Execute &Next"), this, SLOT(executeStep(void)),
                     toKeySequence(tr("F9", "Worksheet|Execute next")), 0, 0);
    menu->insertItem(QPixmap(const_cast<const char**>(execute_xpm)),
                     tr("&Execute Current"), this, SLOT(execute(void)),
                     toKeySequence(tr("Ctrl+Return", "Worksheet|Execute current")), 0, 0);

    menu->insertSeparator();
    if (!Light)
    {
        if (connection().provider() == "Oracle")
            menu->insertItem(tr("&Enable Statistics"), this, SLOT(toggleStatistic(void)),
                             0, TO_ID_STATISTICS);
    }
    menu->insertItem(QPixmap(const_cast<const char**>(stop_xpm)),
                     tr("&Stop Execution"), Result, SLOT(stop(void)),
                     0, TO_ID_STOP);
    if (!Light)
    {
        menu->insertSeparator();
        menu->insertItem(tr("Execute Saved SQL"),
                         this, SLOT(executeSaved()),
                         toKeySequence(tr("F7", "Worksheet|Execute saved SQL")));
        menu->insertItem(tr("Insert Saved SQL"),
                         this, SLOT(insertSaved()),
                         toKeySequence(tr("Shift+F7", "Worksheet|Insert saved SQL")));
        menu->insertItem(tr("Select Saved SQL"),
                         this, SLOT(selectSaved()),
                         toKeySequence(tr("Ctrl+Shift+S", "Worksheet|Select saved SQL")));
        menu->insertItem(QPixmap(const_cast<const char**>(previous_xpm)),
                         tr("Save last SQL"),
                         this, SLOT(saveLast()));
    }
}

#define CHANGE_CURRENT_SCHEMA QString("ALTER SESSION SET CURRENT_SCHEMA = %1")

void toWorksheet::changeSchema(void)
{
    try
    {
        QString schema = Schema->selected();
        toConnection &conn = connection();
        if (toIsOracle(conn))
        {
            /* remove current schema initstring */
            conn.delInit(QString::fromLatin1(CHANGE_CURRENT_SCHEMA).arg(conn.user()));

            /* set the new one with selected schema */
            QString sql = QString::fromLatin1(CHANGE_CURRENT_SCHEMA).arg(schema);
            conn.allExecute(sql);

            conn.addInit(sql);
        }
        else if (toIsMySQL(conn))
        {
            conn.allExecute(QString("USE %1").arg(schema));
            conn.setDatabase(schema);
        }
        else
            throw QString("No support for changing schema for this database");
    }
    TOCATCH
}
