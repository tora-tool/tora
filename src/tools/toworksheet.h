
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

#ifndef TOWORKSHEET_H
#define TOWORKSHEET_H

#include "core/toconnection.h"
#include "core/toconfenum.h"
#include "core/tosettingtab.h"
#include "widgets/totoolwidget.h"
#include "ui_toworksheetsetupui.h"

#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QLabel>
#include <QAction>
#include <QToolBar>

#include <map>

#include "core/tosyntaxanalyzer.h"

class QComboBox;
class QLabel;
class QSplitter;
class QMdiSubWindow;

class toWorksheetEditor;
class toHighlightedEditor;
class toListView;
class toResultBar;
class toResultCols;
class toResultCombo;
class toResultSchema;
class toResultItem;
class toResultPlanExplain;
class toResultTableView;
class toResultStats;
class toTabWidget;
class toTreeWidgetItem;
class toEditableMenu;
class toRefreshCombo;

namespace ToConfiguration
{
    class Worksheet : public ConfigContext
    {
            Q_OBJECT;
            Q_ENUMS(OptionTypeEnum);
        public:
            Worksheet() : ConfigContext("Worksheet", ENUM_REF(Worksheet,OptionTypeEnum)) {};
            enum OptionTypeEnum
            {
                AutoSaveBool  = 7000      // #define CONF_AUTO_SAVE
                , CheckSaveBool           // #define CONF_CHECK_SAVE
                , LogAtEndBool            // #define CONF_LOG_AT_END
                , LogMultiBool            // #define CONF_LOG_MULTI
                , StatisticsBool          // #define CONF_STATISTICS
                , TimedStatsBool          // #define CONF_TIMED_STATS
                , DisplayNumberColumnBool // #define CONF_NUMBER (Display row number)
                , MoveToErrorBool         // #define CONF_MOVE_TO_ERR
                , HistoryErrorBool        // #define CONF_HISTORY (Save previous results)
                , ExecLogBool             // #define CONF_EXEC_LOG
                , ToplevelDescribeBool    // #define CONF_TOPLEVEL_DESCRIBE
                , AutoLoad                // #define CONF_AUTO_LOAD (Default file)
            };
            QVariant defaultValue(int option) const;
    };
};

class toWorksheet : public toToolWidget
{
        Q_OBJECT;
    public:
        typedef toToolWidget super;

        toWorksheet(QWidget *parent, toConnection &connection, bool autoLoad = true);
        virtual ~toWorksheet();

        virtual bool hasTransaction() const; // override fro toToolWidget

        toWorksheetEditor *editor(void);

        virtual bool canHandle(const toConnection &);

#ifdef TORA3_SESSION
        virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
        virtual void importData(std::map<QString, QString> &data, const QString &prefix);
#endif

        static toToolWidget* fileWorksheet(const QString &file);

        virtual void commitChanges();
        virtual void rollbackChanges();

        toSyntaxAnalyzer::statement currentStatement() const;
        QString schema() const override;

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

    public slots:
        /**
         * Updates tool caption.
         */
        void slotSetCaption(void);
        bool slotClose();

    protected slots:

        void slotSchemaChanged(const QString &);
        void slotConnectionChanged(void);
        void slotRefresh();
        void slotExecute();
        void slotParse();
        void slotExecuteAll();
        void slotExecuteStep();
        void slotDescribe();
        void slotDescribeNew();
        void slotEraseLogButton();
        void slotChangeResult(int);
        void slotWindowActivated(toToolWidget*) override;
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
        void slotFirstResult(const QString &sql, const toConnection::exception &result, bool error);
        void slotLastResult(const QString &message, bool error);
        void slotSaveLast(void);
        void slotSaveStatistics(void);
        void slotRefreshSetup(void);
        void slotStop(void);
        void slotLockConnection(bool);
        void slotRefreshModel(toResultModel*);

        /**
         * create context menus
         */
        void slotCreatePopupMenu(const QPoint &pos);

    protected:
        void closeEvent(QCloseEvent *event);

        virtual void focusInEvent(QFocusEvent *e);
        virtual void focusOutEvent(QFocusEvent *e);

    private slots:
        void slotPoll(void);
        void slotChangeConnection(void);

        void slotUnhideResults(const QString &, const toConnection::exception &, bool);
        void slotUnhideResults(void);

    private:

        class BatchExecException : public std::exception
        {
        };

        bool describe(toSyntaxAnalyzer::statement const&);

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

        void queryStarted(const toSyntaxAnalyzer::statement &stat);
        void lockConnection();
        void unlockConnection();
        bool checkUnlockConnection();

        toWorksheetEditor *Editor;
        toTabWidget       *ResultTab;
        toResultTableView *Result;
        toResultPlanExplain *Plan;
        QWidget           *CurrentTab;
        toSyntaxAnalyzer::statement m_lastQuery; // query is saved in order to reexecute it periodically ("refresh")
        toResultItem      *Resources;
        toResultStats     *Statistics;
#ifdef TORA_EXPERIMENTAL
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
        toRefreshCombo    *Refresh;
        QLabel            *Started;
        toResultSchema    *Schema;
        toResultModel     *ResultModel;
        int RefreshSeconds;
        QTimer RefreshTimer;

        toEditableMenu *SavedMenu;
        toEditableMenu *InsertSavedMenu;

        bool m_FirstDataReceived;
        QTime Time;     // Timer used for query run duration (See QLabel *Started, slotPoll())
        QTimer Poll;	// Periodically refresh duration timer "Started"

        QWidget *Current;
        std::map<int, QWidget *> History;
        int LastID;

        QMenu *ToolMenu, *context;

        QAction *parseAct, *lockConnectionAct, *executeAct, *executeStepAct,
                *executeAllAct,
                *refreshAct, *describeAct, *describeActNew, *explainAct, *stopAct, *eraseAct,
                *statisticAct, *previousAct, *nextAct, *saveLastAct;

        QSharedPointer<toConnectionSubLoan> LockedConnection;
        bool lockConnectionActClicked;
};


class toWorksheetSetting
    : public QWidget
    , public Ui::toWorksheetSetupUI
    , public toSettingTab
{
        Q_OBJECT;
        toTool *Tool;

    public:
        toWorksheetSetting(toTool *tool, QWidget* parent = 0, const char* name = 0);

        virtual void saveSetting(void);

    public slots:
        void slotChooseFile(void);
};

#endif
