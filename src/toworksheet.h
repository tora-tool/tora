
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

#include "config.h"

#include <map>

#include <qdatetime.h>
#include <qtimer.h>
#include <QString>
#include <QLabel>
#include <QAction>
#include <QToolBar>

#include "toconnection.h"
#include "tosqlparse.h"
#include "totool.h"
#include "totreewidget.h"
#include "toresulttableview.h"

#include "ui_toworksheetsetupui.h"

class QComboBox;
class QLabel;
class QSplitter;
class QMdiSubWindow;

class toHighlightedText;
class toListView;
class toResultBar;
class toResultCols;
class toResultCombo;
class toResultSchema;
class toResultItem;
class toResultPlan;
class toResultStats;
class toVisualize;
class toTabWidget;
class toWorksheetWidget;
class toEditableMenu;


class toWorksheet : public toToolWidget
{
    Q_OBJECT;

    friend class toWorksheetWidget;

    toHighlightedText *Editor;
    toTabWidget       *ResultTab;
    toResultTableView *Result;
    toResultPlan      *Plan;
    QWidget           *CurrentTab;
    QString            QueryString; // query is saved in order to reexecute it periodically ("refresh")
    toSQLParse::statement::statementClass QueryStatementClass; // statement class has to be saved as well
    toResultItem      *Resources;
    toResultStats     *Statistics;
    toResultBar       *WaitChart;
    toResultBar       *IOChart;
    toListView        *Logging;
    //! Re-set EditSplitter sizes with settings.
    QList<int>        EditSplitterSizes;
    QSplitter         *EditSplitter;
    QSplitter         *ResourceSplitter;
    toResultTableView *LongOps;
    QWidget           *Visualize;
    toTreeWidgetItem  *LastLogItem;
    QWidget           *StatTab;
    bool               TryStrip;
    //! Used only for DESC/DESCRIBE sql statements
    toResultCols      *Columns;
    QComboBox         *Refresh;
    QLabel            *Started;
    toResultSchema    *Schema;

    int RefreshSeconds;
    QTimer RefreshTimer;
    
    //! Flag to handle continue-on-error for executeAllAct
    bool m_batchContinue;

    toEditableMenu *SavedMenu;
    toEditableMenu *InsertSavedMenu;

    bool First;
    QTime Timer;
    QTimer Poll;

    QWidget *Current;
    std::map<int, QWidget *> History;
    int LastID;

    int LastLine;
    int LastOffset;

    enum execType
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

    QMenu *ToolMenu;
    QMenu *context;

    QAction *executeNewlineAct;
    QAction *parseAct;

    QAction *executeAct;
    QAction *executeStepAct;
    QAction *executeAllAct;
    QAction *refreshAct;
    QAction *describeAct;
    QAction *explainAct;
    QAction *stopAct;
    QAction *eraseAct;
    QAction *statisticAct;
    QAction *previousAct;
    QAction *nextAct;
    QAction *saveLastAct;

    QString duration(int, bool hundreds = true);
    void saveHistory(void);
    void viewResources(void);
    void query(const QString &str, execType type, toSQLParse::statement::statementClass sc);
    virtual bool checkSave(bool input);
    void saveDefaults(void);
    void setup(bool autoLoad);

    void execute(toSQLParse::tokenizer &tokens, int line, int pos, execType typeto,
                 toSQLParse::statement::statementClass sc);

    void insertStatement(const QString &);

    void createActions();

    // private for toworksheetwidget
    toWorksheet(QWidget *parent, const char *name, toConnection &connection);

    // Imitate something like "create or replace" syntax for MySQL
    void mySQLBeforeCreate(QString &chk);

protected:
    void closeEvent(QCloseEvent *event);

public:
    toWorksheet(QWidget *parent, toConnection &connection, bool autoLoad = true);
    virtual ~toWorksheet();

    toHighlightedText *editor(void)
    {
        return Editor;
    }
    virtual bool describe(const QString &query);

    virtual bool canHandle(toConnection &)
    {
        return true;
    }

    virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
    virtual void importData(std::map<QString, QString> &data, const QString &prefix);

    static toWorksheet *fileWorksheet(const QString &file);

signals:
    void executed(void);
public slots:
    bool close();
    void connectionChanged(void);
    void refresh();
    void execute();
    void parseAll();
    void executeAll();
    void executeStep();
    void executeNewline();
    void describe();
    void eraseLogButton();
    void changeResult(int);
    void windowActivated(QMdiSubWindow *w);
    void queryDone(void);
    void enableStatistic(bool);
    void explainPlan(void);
    void toggleStatistic(void)
    {
        statisticAct->toggle();
    }
    void showInsertSaved(void);
    void showSaved(void);
    void insertSaved(QAction *);
    void executeSaved(QAction *);
    void removeSaved(QAction *);
    void executePreviousLog(void);
    void executeNextLog(void);
    void executeLog(void);
    void addLog(const QString &sql, const toConnection::exception &result, bool error);
    void changeRefresh(const QString &);
    void saveLast(void);
    void saveStatistics(void);
    void refreshSetup(void);
    void stop(void);

    /**
     * create context menus
     *
     */
    void createPopupMenu(const QPoint &pos);

    /**
     * Updates tool caption.
     *
     */
    void setCaption(void);

private slots:
    void poll(void);
    void changeConnection(void);

    void unhideResults(const QString &, const toConnection::exception &, bool);
    void unhideResults(void);
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
    void chooseFile(void);
};

#endif
