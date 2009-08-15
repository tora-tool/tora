
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

#ifndef TODEBUG_H
#define TODEBUG_H

#include "config.h"
#include "tobackground.h"
#include "toconnection.h"
#include "ui_todebugwatch.h"
#include "tohighlightedtext.h"
#include "tosqlparse.h"
#include "tothread.h"
#include "totool.h"

#include <QString>
#include <QMenu>
#include <QAction>

#define TO_SUCCESS  0
#define TO_NO_SUCH_BREAKPOINT 13
#define TO_ERROR_NO_DEBUG_INFO  2
#define TO_ERROR_ILLEGAL_LINE 12
#define TO_ERROR_BAD_HANDLE 16
#define TO_ERROR_UNIMPLEMENTED 17
#define TO_ERROR_TIMEOUT 31
#define TO_ERROR_NULLVALUE 32
#define TO_ERROR_NULLCOLLECTION 40
#define TO_ERROR_INDEX_TABLE 18

#define TO_REASON_KNL_EXIT 25
#define TO_REASON_NO_SESSION -1
#define TO_REASON_TIMEOUT 17
#define TO_REASON_EXIT  15
#define TO_REASON_WHATEVER 0
#define TO_REASON_STARTING 2

#define TO_NAME_CURSOR  0
#define TO_NAME_TOPLEVEL 1
#define TO_NAME_BODY  2
#define TO_NAME_TRIGGER  3
#define TO_NAME_NONE  127

#define TO_BREAK_CONTINUE       0
#define TO_BREAK_EXCEPTION 2
#define TO_BREAK_ANY_CALL 12
#define TO_BREAK_RETURN  16
#define TO_BREAK_NEXT_LINE 32
#define TO_BREAK_ANY_RETURN 512
#define TO_BREAK_HANDLER 2048
#define TO_ABORT_EXECUTION 8192

class QButtonGroup;
class QComboBox;
class QTabWidget;
class QTreeView;
class QToolButton;
class toCodeModel;
class toConnection;
class toDebugText;
class toMarkedText;
class toOutput;
class toTreeWidget;
class toTreeWidgetItem;

class toDebug : public toToolWidget
{
    Q_OBJECT;

    struct debugParam
    {
        debugParam()
        {
            In = false;
            Out = false;
        }
        QString Name;
        QString Type;
        bool In;
        bool Out;
    };

    std::list<debugParam> CurrentParams;

    // Toolbar
    void createActions(void);

    QComboBox *Schema;
    QAction   *refreshAct;
    QAction   *newSheetAct;
    QAction   *scanSourceAct;
    QAction   *compileAct;
    QAction   *closeAct;
    QAction   *closeAllAct;
    QAction   *executeAct;
    QAction   *stopAct;
    QAction   *stepAct;
    QAction   *nextAct;
    QAction   *returnAct;
    QAction   *debugPaneAct;
    QAction   *nextErrorAct;
    QAction   *previousErrorAct;
    QAction   *toggleBreakAct;
    QAction   *disableBreakAct;
    QAction   *addWatchAct;
    QAction   *deleteWatchAct;
    QAction   *changeWatchAct;
    QAction   *eraseLogAct;

    // Extra menu
    QMenu *ToolMenu;

    // Content pane
    QTreeView    *Objects;
    toCodeModel  *CodeModel;
    toTreeWidget *Contents;

    // Debug pane
    QTabWidget   *DebugTabs;
    toTreeWidget *Breakpoints;
    toTreeWidget *StackTrace;
    toTreeWidget *Watch;
    toTreeWidget *Parameters;
    toOutput     *Output;
    toMarkedText *RuntimeLog;

    // Editors
    QTabWidget *Editors;

    // Must hold lock before reading or writing to these
    toLock       Lock;
    toSemaphore  TargetSemaphore;
    toSemaphore  ChildSemaphore;
    toSemaphore  StartedSemaphore;
    toThread    *TargetThread;
    QString      TargetSQL;
    QString      TargetLog;
    QString      TargetException;
    toQList      InputData;
    toQList      OutputData;
    int          ColumnSize;
    bool         RunningTarget;
    bool         DebuggerStarted;
    // Can be read after thread startup
    QString      TargetID;
    // End of lock stuff
    toTimer      StartTimer;

    class targetTask : public toTask
    {
        toDebug &Parent;

    public:
        targetTask(toDebug &parent) : Parent(parent)
        { }

        virtual void run(void);
    };

    virtual bool close(void);

    int continueExecution(int stopon);

    bool checkCompile(toDebugText *edit);
    bool checkCompile(void);
    void updateCurrent(void);
    QString editorName(const QString &schema,
                       const QString &object,
                       const QString &type);
    QString editorName(toDebugText *text);
    int sync(void);
    bool hasMembers(const QString &str);
    void readLog(void);
    void updateState(int reason);
    void updateArguments(toSQLParse::statement &statements,
                         toTreeWidgetItem *parent);
    void updateContent(toSQLParse::statement &statements,
                       toTreeWidgetItem *parent,
                       const QString &id = QString::null);
    void updateContent(toDebugText *editor);
    void updateContent(void)
    {
        updateContent(currentEditor());
    }
    void reorderContent(toTreeWidgetItem *item, int, int);
    bool viewSource(const QString &schema,
                    const QString &name,
                    const QString &type,
                    int line,
                    bool current = false);

    void setDeferedBreakpoints(void);
    bool checkStop(void);
    void closeEditor(toDebugText* &editor);

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void startTarget(void);

    /*! \brief Enable/disable debugging related actions (steps etc.).
    It's set to false when DBMS_DEBUG related calls fail.
    */
    void enableDebugger(bool);

public:
    toDebug(QWidget *parent, toConnection &connection);

    virtual ~toDebug();

    friend class targetTask;

    bool isRunning(void);

    toTreeWidgetItem *contents(void);
    toDebugText *currentEditor(void);
    QString currentSchema(void);

    void executeInTarget(const QString &, toQList &params);

    QString checkWatch(const QString &name);

    virtual void exportData(std::map < QString,
                            QString > &data,
                            const QString &prefix);
    virtual void importData(std::map < QString,
                            QString > &data,
                            const QString &prefix);
public slots:
    void stop(void);
    void compile(void);
    void refresh(void);
    void changeSchema(int);
    void changePackage(const QModelIndex &, const QModelIndex &);
    void showDebug(bool);
    void prevError(void);
    void nextError(void);
    void changeContent(toTreeWidgetItem *);
    void showSource(toTreeWidgetItem *);
    void scanSource(void);
    void reorderContent(int, int);
    void newSheet(void);
    void execute(void);
    void toggleBreak(void);
    void toggleEnable(void);
    void addWatch(void);
    void stepInto(void)
    {
        continueExecution(TO_BREAK_ANY_CALL);
    }
    void stepOver(void)
    {
        continueExecution(TO_BREAK_NEXT_LINE);
    }
    void returnFrom(void)
    {
        continueExecution(TO_BREAK_ANY_RETURN);
    }
    void windowActivated(QMdiSubWindow *w);
    void selectedWatch(void);
    void deleteWatch(void);
    void clearLog(void);
    void changeWatch(void);
    void changeWatch(toTreeWidgetItem *item);
    void closeEditor(void);
    void closeAllEditor(void);
};

class toDebugWatch : public QDialog, public Ui::toDebugWatchUI
{
    Q_OBJECT;

    toTreeWidgetItem *HeadItems;
    toTreeWidgetItem *BodyItems;
    toDebug          *Debugger;
    QString           Object;
    QString           Default;
    QButtonGroup     *ScopeGroup;

    toTreeWidgetItem *findMisc(const QString &str, toTreeWidgetItem *);

public:
    toDebugWatch(toDebug *parent);

    toTreeWidgetItem *createWatch(toTreeWidget *watches);

public slots:
    void changeScope(int num);
};

#endif
