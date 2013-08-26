
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

#ifndef TOWORKSHEET_H
#define TOWORKSHEET_H

#include "core/toconnection.h"
#include "core/totool.h"
#include "parsing/tosyntaxanalyzer.h"

#include "ui_toworksheetsetupui.h"

#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtGui/QLabel>
#include <QtGui/QAction>
#include <QtGui/QToolBar>

#include <map>

class QComboBox;
class QLabel;
class QSplitter;
class QMdiSubWindow;

class toWorksheetEditor;
class toHighlightedEditor;
class toListView;
//obsolete class toResultBar;
class toResultCols;
class toResultCombo;
class toResultSchema;
class toResultItem;
class toResultPlan;
class toResultTableView;
class toResultStats;
class toTabWidget;
class toTreeWidgetItem;
class toEditableMenu;

class toWorksheet : public toToolWidget
{
    Q_OBJECT;
public:
    typedef toToolWidget super;

    toWorksheet(QWidget *parent, toConnection &connection, bool autoLoad = true);
    virtual ~toWorksheet();

    toHighlightedEditor *editor(void);

    bool describe(toSyntaxAnalyzer::statement const&);

    virtual bool canHandle(toConnection &);

#ifdef TORA3_GRAPH
    virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
    virtual void importData(std::map<QString, QString> &data, const QString &prefix);
#endif

    static toToolWidget* fileWorksheet(const QString &file);


    enum execTypeEnum
    {
        //! Display results in result tab
        Normal,
        //! Don't display results, just send statements to the DB
        Direct,
        //! Don't run query, just parse it. (Check syntax etc.)
        Parse,
        //! Call explain plan
        OnlyPlan
    };

    enum selectionModeEnum
    {
    	DontSelectQueryEnum = false,
    	SelectQueryEnum = true
    };

protected:
    void closeEvent(QCloseEvent *event);

    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

public slots:
    bool slotClose();
    void slotConnectionChanged(void);
    void slotRefresh();
    void slotExecute();
    void slotParseAll();
    void slotExecuteAll();
    void slotExecuteStep();
    void slotDescribe();
    void slotDescribeNew();
    void slotEraseLogButton();
    void slotChangeResult(int);
    virtual void slotWindowActivated(toToolWidget*);
    void slotQueryDone(void);
    void slotEnableStatistic(bool);
    void slotExplainPlan(void);
    void slotToggleStatistic(void);
    void slotShowInsertSaved(void);
    void slotShowSaved(void);
    void slotInsertSaved(QAction *);
    void slotExecuteSaved(QAction *);
    void slotRemoveSaved(QAction *);
    void slotExecutePreviousLog(void);
    void slotExecuteNextLog(void);
    void slotExecuteLog(void);
    void slotAddLog(const QString &sql, const toConnection::exception &result, bool error);
    void slotChangeRefresh(const QString &);
    void slotSaveLast(void);
    void slotSaveStatistics(void);
    void slotRefreshSetup(void);
    void slotStop(void);
    void slotLockConnection(bool);

    /**
     * create context menus
     */
    void slotCreatePopupMenu(const QPoint &pos);

    /**
     * Updates tool caption.
     */
    void slotSetCaption(void);

private slots:
    void slotPoll(void);
    void slotChangeConnection(void);

    void slotUnhideResults(const QString &, const toConnection::exception &, bool);
    void slotUnhideResults(void);

    void slotCommitChanges(toConnection &);
    void slotRollbackChanges(toConnection &);

private:

    class BatchExecException : public std::exception
    {
    };

    toSyntaxAnalyzer::statement currentStatement() const;

    QString duration(int, bool hundreds = true);
    void saveHistory(void);
    void viewResources(void);

    void query(QString const& text, execTypeEnum type);
    void query(toSyntaxAnalyzer::statement const&, execTypeEnum type, selectionModeEnum = SelectQueryEnum);
    void querySelection(execTypeEnum type);

    bool checkSave();
    void saveDefaults(void);
    void setup(bool autoLoad);

    void insertStatement(const QString &);

    void createActions();

    // Imitate something like "create or replace" syntax for MySQL
    void mySQLBeforeCreate(QString &chk);

    void addLog(const QString &result);

    toWorksheetEditor *Editor;
    toTabWidget       *ResultTab;
    toResultTableView *Result;
    toResultPlan      *Plan;
    QWidget           *CurrentTab;
    toSyntaxAnalyzer::statement m_lastQuery; // query is saved in order to reexecute it periodically ("refresh")
    toResultItem      *Resources;
    toResultStats     *Statistics;
#ifdef TORA3_GRAPH
    toResultBar       *WaitChart;
    toResultBar       *IOChart;
#endif
    toListView        *Logging;
    //! Re-set EditSplitter sizes with settings.
    QList<int>        EditSplitterSizes;
    QSplitter         *EditSplitter;
    QSplitter         *ResourceSplitter;
    toResultTableView *LongOps;
    toTreeWidgetItem  *LastLogItem;
    QWidget           *StatTab;
    //! Used only for DESC/DESCRIBE sql statements
    toResultCols      *Columns;
    QComboBox         *Refresh;
    QLabel            *Started;
    toResultSchema    *Schema;

    int RefreshSeconds;
    QTimer RefreshTimer;

    toEditableMenu *SavedMenu;
    toEditableMenu *InsertSavedMenu;

    bool m_FirstDataReceived;
    QTime Time;
    QTimer Poll;

    QWidget *Current;
    std::map<int, QWidget *> History;
    int LastID;

    QMenu *ToolMenu, *context;

    QAction *parseAct, *lockConnectionAct, *executeAct, *executeStepAct,
     *executeAllAct,
     *refreshAct, *describeAct, *describeActNew, *explainAct, *stopAct, *eraseAct,
     *statisticAct, *previousAct, *nextAct, *saveLastAct;

    QSharedPointer<toConnectionSubLoan> LockedConnection;
};


class toWorksheetSetup : public QWidget,
    public Ui::toWorksheetSetupUI,
    public toSettingTab
{
    Q_OBJECT;
    toTool *Tool;

public:
    toWorksheetSetup(toTool *tool, QWidget* parent = 0, const char* name = 0);

    virtual void saveSetting(void);

public slots:
    void slotChooseFile(void);
};

#endif
