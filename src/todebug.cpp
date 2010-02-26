
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

#include <QtDebug>
#include "utils.h"

#include "toconf.h"
#include "todebug.h"
#include "ui_todebugchangeui.h"
#include "todebugtext.h"
#include "tohelp.h"
#include "tohighlightedtext.h"
#include "tomarkedtext.h"
#include "tomain.h"
#include "tooutput.h"
#include "toparamget.h"
#include "toresultview.h"
#include "tosql.h"
#include "tosqlparse.h"
#include "totool.h"
#include "tocodemodel.h"

#include <cstdio>
#include <stack>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qregexp.h>
#include <qsizepolicy.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include <QPixmap>
#include <QList>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QMdiArea>
#include <QMessageBox>
#include <QButtonGroup>
#include <QTreeView>

#include "icons/addwatch.xpm"
#include "icons/changewatch.xpm"
#include "icons/close.xpm"
#include "icons/compile.xpm"
#include "icons/delwatch.xpm"
#include "icons/enablebreak.xpm"
#include "icons/execute.xpm"
#include "icons/nextbug.xpm"
#include "icons/prevbug.xpm"
#include "icons/refresh.xpm"
#include "icons/returnfrom.xpm"
#include "icons/scansource.xpm"
#include "icons/stepinto.xpm"
#include "icons/stepover.xpm"
#include "icons/stop.xpm"
#include "icons/todebug.xpm"
#include "icons/togglebreak.xpm"
#include "icons/toworksheet.xpm"

#ifdef DEBUG
static toSQL SQLDebugSelfCheck("toDebug:SelfCheck",
                               "DECLARE\n"
                               "  error_message VARCHAR2(1000);\n"
                               "BEGIN\n"
                               "  SYS.DBMS_DEBUG.SELF_CHECK;\n"
                               "  SELECT 'OK' INTO :line<char[101],out> FROM sys.DUAL;\n"
                               "EXCEPTION WHEN OTHERS THEN\n"
                               "  error_message := sqlerrm;\n"
                               "  SELECT error_message INTO :line<char[101],out> FROM sys.DUAL;\n"
                               "END;",
                               "Perform Self-check of debugging system");
#endif
class toDebugTool : public toTool
{
    std::map<toConnection *, QWidget *> Windows;

    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(todebug_xpm);
    }
public:
    toDebugTool()
            : toTool(110, "PL/SQL Debugger")
    { }
    virtual const char *menuItem()
    {
        return "PL/SQL Debugger";
    }
    // This one is called then Debug Tool is being launched by TOra user
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
#ifdef DEBUG
        // perform self-check (should probably display scary message and close if this test fails)
        toQuery selfCheck(connection, SQLDebugSelfCheck);
        QString str = selfCheck.readValueNull();
        qDebug() << "DBMS_DEBUG.SELF_CHECK result: " << str;
#endif

        // Only one Debug Tool window is allowed per connection
        // Therefor is Debug Tool is being launched for a connection with an already
        // opened Debug window - that window is activated instead of opening a new one
        std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
        if (i != Windows.end())
        {
            (*i).second->raise();
            (*i).second->setFocus();
            return NULL;
        }
        else
        {
            QWidget *window = new toDebug(parent, connection);
            Windows[&connection] = window;
            return window;
        }
    } // toolWindow
    void closeWindow(toConnection &connection)
    {
        std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
        if (i != Windows.end())
            Windows.erase(i);
    }
    virtual bool canHandle(toConnection &conn)
    {
        // Debug Tool is only available for Oracle DB
        if (!toIsOracle(conn))
        {
            return false;
        }
#if 1
        // Only Oracle DB version 8 and above can handle debugging of plsql code
        QString version = conn.version();
        if (version.left(version.indexOf('.')).toInt() < 8)
        {
            return false;
        }
#endif
        return true;
    }
}; // class toDebugTool

static toDebugTool DebugTool;

// A class for a Watch dialog (adding/modifying a watch)
toDebugWatch::toDebugWatch(toDebug *parent)
        : QDialog(parent), Debugger(parent)
{
    setupUi(this);
    setModal(true);

    toHelp::connectDialog(this);
    {
        int curline, curcol;
        Debugger->currentEditor()->getCursorPosition (&curline, &curcol);
        Default = Debugger->currentEditor()->text(curline);
        while (curcol > 0 && toIsIdent(Default[curcol - 1]))
            curcol--;
        while (curcol < int(Default.length()) && !toIsIdent(Default[curcol]))
            curcol++;
        Default.replace(0, curcol, QString::null);
        curcol = 1;
        while (curcol < int(Default.length()) && toIsIdent(Default[curcol]))
            curcol++;
        Default = Default.left(curcol);
    }

    Object = Debugger->currentEditor()->object();

    ScopeGroup = new QButtonGroup(Scope);
    ScopeGroup->addButton(LocalScope_2, 1);
    ScopeGroup->addButton(LocalScope, 2);
    ScopeGroup->addButton(Package, 3);
    ScopeGroup->addButton(GlobalScope, 4);

    connect(ScopeGroup,
            SIGNAL(buttonClicked(int)),
            this,
            SLOT(changeScope(int)));
    changeScope(1);
} // toDebugWatch::toDebugWatch

void toDebugWatch::changeScope(int num)
{
    switch (num)
    {
    default:
        Name->clear();
        Name->addItem(Default);
        break;
    case 4:
    {
        Name->clear();
        QString str = Debugger->currentEditor()->schema();
        str += QString::fromLatin1(".");
        if (!Object.isEmpty())
        {
            str += Object;
            str += QString::fromLatin1(".");
        }
        str += Default;
        Name->addItem(str);
    }
    break;
    }
} // toDebugWatch::changeScope

toTreeWidgetItem *toDebugWatch::createWatch(toTreeWidget *watches)
{
    QString str;
    switch (ScopeGroup->checkedId())
    {
    case 1:
    case 5:
    {
        toResultViewItem *item = new toResultViewItem(watches, NULL);
        item->setText(0, QString::null);
        item->setText(1, QString::null);
        item->setText(2, Name->currentText());
        item->setText(3, QString::null);
        item->setText(4, QString::fromLatin1("NOCHANGE"));
        item->setText(6, ScopeGroup->checkedId() == 5 ? "AUTO" : "");
        return item;
    }
    case 3:
        str = Debugger->currentEditor()->schema();
        str += QString::fromLatin1(".");
        str += Debugger->currentEditor()->object();
        str += QString::fromLatin1(".");
        str += Name->currentText();
        break;
    case 4:
        str = Name->currentText();
        break;
    }
    QString schema;
    QString object;
    QString name;
    int pos = str.indexOf(QString::fromLatin1("."));
    if (pos > 0)
    {
        schema = str.left(pos);
        str.remove(0, pos + 1);
    }
    else
    {
        toStatusMessage(tr("Can't parse location"));
        return NULL;
    }
    pos = str.indexOf(QString::fromLatin1("."));
    if (pos > 0)
    {
        object = str.left(pos);
        str.remove(0, pos + 1);
    }
    toResultViewItem *item = new toResultViewItem(watches, NULL);
    item->setText(0, schema);
    item->setText(1, object);
    item->setText(2, str);
    item->setText(3, QString::null);
    item->setText(4, QString::fromLatin1("NOCHANGE"));
    return item;
} // toDebugWatch::createWatch

static toSQL SQLDebugOutputPoll("toDebugOutput:Poll",
                                "DECLARE\n"
                                "   coll sys.dbms_debug_vc2coll;\n"
                                "   ret INTEGER;\n"
                                "   i INTEGER;\n"
                                "   line VARCHAR2(100);\n"
                                "   errm VARCHAR2(100);\n"
                                "BEGIN\n"
                                "  SYS.DBMS_DEBUG.EXECUTE('DECLARE '||\n"
                                "                         '  pp SYS.dbms_debug_vc2coll := SYS.dbms_debug_vc2coll(); '||\n"
                                "                         '  line VARCHAR2(100); '||\n"
                                "                         '  ret INTEGER; '||\n"
                                "                         'BEGIN '||\n"
                                "                         '  SYS.DBMS_OUTPUT.GET_LINE(line,ret); '||\n"
                                "                         '  pp.EXTEND(2); '||\n"
                                "                         '  pp(1):=ret; '||\n"
                                "                         '  pp(2):=line; '||\n"
                                "                         '  :1 := pp; '||\n"
                                "                         'END;',\n"
                                "                         -1,\n"
                                "                         1,\n"
                                "                         coll,\n"
                                "                         line);\n"
                                "  i:=coll.first;\n"
                                "  IF i IS NOT NULL THEN\n"
                                "    ret:=coll(i);\n"
                                "    IF ret = 0 THEN\n"
                                "      i:=coll.next(i);\n"
                                "      line:=coll(i);\n"
                                "    ELSE\n"
                                "      line:='';\n"
                                "    END IF;\n"
                                "  ELSE\n"
                                "    ret:=1;\n"
                                "  END IF;\n"
                                "  SELECT ret,line INTO :ret<int,out>,:line<char[101],out> FROM sys.DUAL;\n"
                                "END;",
                                "Poll for output in the debug session, must have same bindings");
static toSQL SQLDebugOutputDisable("toDebugOutput:Disable",
                                   "DECLARE\n"
                                   "   coll sys.dbms_debug_vc2coll;\n"
                                   "   errm VARCHAR2(100);\n"
                                   "BEGIN\n"
                                   "  SYS.DBMS_DEBUG.EXECUTE('BEGIN SYS.DBMS_OUTPUT.DISABLE; END;',\n"
                                   "                         -1,\n"
                                   "                         0,\n"
                                   "                         coll,\n"
                                   "                         errm);\n"
                                   "END;",
                                   "Disable output in the debug session");
static toSQL SQLDebugOutputEnable("toDebugOutput:Enable",
                                  "DECLARE\n"
                                  "   coll sys.dbms_debug_vc2coll;\n"
                                  "   errm VARCHAR2(100);\n"
                                  "BEGIN\n"
                                  "  SYS.DBMS_DEBUG.EXECUTE('BEGIN SYS.DBMS_OUTPUT.ENABLE; END;',\n"
                                  "                         -1,\n"
                                  "                         0,\n"
                                  "                         coll,\n"
                                  "                         errm);\n"
                                  "END;",
                                  "Enable output in the debug session");

// Class handling output from target session
class toDebugOutput : public toOutput
{
    toDebug *Debugger;
//    toConnection * outputConnection;
    toQuery * outputSession; // A specific debug session, the only one having target session attached.
    // Attempts to call output functions from any other sessions will fail.
public:
    toDebugOutput(toDebug *debug, QWidget *parent, toConnection &conn, toQuery *query)
            : toOutput(parent, conn), Debugger(debug)
    {
        outputSession = query;      // One specific oracle session must be used.
    }
    virtual void refresh(void)
    {
        if (Debugger->isRunning() && enabled())
        {
            try
            {
                int ret = -1;
                do
                {
                    outputSession->execute(SQLDebugOutputPoll);
                    ret = outputSession->readValue().toInt();
                    QString str = outputSession->readValueNull();
                    if (ret == 0 || str.length())
                        insertLine(str);
                }
                while (ret == 0);
            }
            TOCATCH
        }
    }
    virtual void disable(bool dis)
    {
#ifdef DEBUG
        if (dis) qDebug() << "toDebugOutput::disable DISABLE!";
        else qDebug() << "toDebugOutput::disable ENABLE!";
#endif
        if (Debugger->isRunning())
        {
            try
            {
                if (dis)
                    outputSession->execute(SQLDebugOutputDisable);
                else
                    outputSession->execute(SQLDebugOutputEnable);
            }
//            catch (...)
            catch (const QString &exc)
            {
                toStatusMessage(qApp->translate(
                                    "toDebugOutput",
                                    "Couldn't enable/disable output for session"));
            }
        }
    } // disable

    virtual void closeEvent(QCloseEvent *event)
    {
        toOutput::closeEvent(event);
    }
}; // class toDebugOutput

#ifdef DEBUG
// Get text version from error/return/reason code returned by DBMS_DEBUG routines
// Used for debug purposes
// type: 1 - error, 2 - reason, 3 - continue
QString toDebug::getErrorText(int code, int type)
{
    QString ret;

    if (type == 1) // error codes
    {
        switch (code)
        {
        case TO_SUCCESS:
            ret = "SUCCESS";
            break;
        case TO_NO_SUCH_BREAKPOINT:
            ret = "NO_SUCH_BREAKPOINT";
            break;
        case TO_ERROR_NO_DEBUG_INFO:
            ret = "ERROR_NO_DEBUG_INFO";
            break;
        case TO_ERROR_ILLEGAL_LINE:
            ret = "ERROR_ILLEGAL_LINE";
            break;
        case TO_ERROR_BAD_HANDLE:
            ret = "ERROR_BAD_HANDLE";
            break;
        case TO_ERROR_UNIMPLEMENTED:
            ret = "ERROR_UNIMPLEMENTED";
            break;
        case TO_ERROR_TIMEOUT:
            ret = "ERROR_TIMEOUT";
            break;
        case TO_ERROR_NULLVALUE:
            ret = "ERROR_NULLVALUE";
            break;
        case TO_ERROR_NULLCOLLECTION:
            ret = "ERROR_NULLCOLLECTION";
            break;
        case TO_ERROR_INDEX_TABLE:
            ret = "ERROR_INDEX_TABLE";
            break;
        case TO_ERROR_DEFERRED:
            ret = "ERROR_DEFERRED";
            break;
        case TO_ERROR_EXCEPTION:
            ret = "ERROR_EXCEPTION";
            break;
        case TO_ERROR_COMMUNICATION:
            ret = "ERROR_COMMUNICATION";
            break;
        default:
            ret = "UNKNOWN ERROR CODE " + code;
        } // switch
    }
    else if (type == 2) // reason codes
    {
        switch (code)
        {
        case TO_REASON_WHATEVER:
            ret = "TO_REASON_WHATEVER (0)";
            break;
        case TO_REASON_STARTING:
            ret = "TO_REASON_STARTING (2)";
            break;
        case TO_REASON_BREAKPOINT:
            ret = "TO_REASON_BREAKPOINT (3)";
            break;
        case TO_REASON_ENTER:
            ret = "TO_REASON_ENTER (6)";
            break;
        case TO_REASON_RETURN:
            ret = "TO_REASON_RETURN (7)";
            break;
        case TO_REASON_FINISH:
            ret = "TO_REASON_FINISH (8)";
            break;
        case TO_REASON_LINE:
            ret = "TO_REASON_LINE (9)";
            break;
        case TO_REASON_EXIT:
            ret = "TO_REASON_EXIT (15)";
            break;
        case TO_REASON_TIMEOUT:
            ret = "TO_REASON_TIMEOUT (17)";
            break;
        case TO_REASON_KNL_EXIT:
            ret = "TO_REASON_KNL_EXIT (25)";
            break;
        case TO_REASON_NO_SESSION:
            ret = "TO_REASON_NO_SESSION (-1)";
            break;
        default:
            ret = "UNKNOWN REASON CODE " + QString(code);
        }
    }
    else if (type == 3) // continue code
    {
        switch (code)
        {
        case TO_BREAK_ANY_CALL:
            ret = "TO_BREAK_ANY_CALL (12)";
            break;
        case TO_BREAK_NEXT_LINE:
            ret = "TO_BREAK_NEXT_LINE (32)";
            break;
        default:
            ret = "UNKOWN CONTINUE CODE " + code;
        }
    }
    else ret = "ERROR! Unknown type " + type;

    return ret;
} // getTextVersion
#endif

bool toDebug::isRunning(void)
{
    toLocker lock (Lock);
    return RunningTarget;
}

// TODO: PLSQL_DEBUG is deprecated, PLSQL_OPTIMIZE_LEVEL=1 should be used instead
//       Should be analysed which OracleDB versions support this new way.
static toSQL SQLDebugEnable("toDebug:EnableDebug",
//                            "ALTER SESSION SET PLSQL_DEBUG = TRUE",
                            "ALTER SESSION SET PLSQL_OPTIMIZE_LEVEL = 1",
                            "Enable PL/SQL debugging");

static toSQL SQLDebugInit("toDebug:Initialize",
                          "DECLARE\n"
                          "  ret VARCHAR2(200);\n"
                          "BEGIN\n"
                          "  ret:=SYS.DBMS_DEBUG.INITIALIZE;\n"
                          "  SELECT ret INTO :f2<char[201],out> FROM sys.DUAL;\n"
                          "  SYS.DBMS_DEBUG.DEBUG_ON;\n"
                          "END;",
                          "Initialize the debug session, must have same bindings");

// Target session running in a separate thread
void toDebug::targetTask::run(void)
{
//qDebug() << "toDebug::targetTask::run";
    try
    {
        // Create a target session using parameters of a current "debug" connection
        toQuery targetSession(Parent.connection());
        try
        {
#ifdef DEBUG
            qDebug() << "toDebug::targetTask::run ALTER SESSION SET PLSQL_OPTIMIZE_LEVEL = 1";
#endif
            targetSession.execute(SQLDebugEnable);
        }
        catch (...)
        {
            toLocker lock (Parent.Lock);
            Parent.TargetLog += QString::fromLatin1("Couldn't enable debugging for target session\n");
        }
        try
        {
#ifdef DEBUG
            qDebug() << "toDebug::targetTask::run DBMS_DEBUG.INITIALIZE";
            qDebug() << "toDebug::targetTask::run DBMS_DEBUG.DEBUG_ON";
#endif
            targetSession.execute(SQLDebugInit);

            // can't use moc from nested class
            QMetaObject::invokeMethod(&Parent,
                                      "enableDebugger",
                                      Qt::QueuedConnection,
                                      Q_ARG(bool, true));

            toLocker lock (Parent.Lock);
            Parent.DebuggerStarted = true;
            Parent.TargetID = targetSession.readValue();
//qDebug() << "toDebug::targetTask::run target id = " << Parent.TargetID;
            Parent.ChildSemaphore.up(); // resume main TOra thread
            Parent.TargetLog += QString::fromLatin1("Debug session connected\n");
        }
        catch (const QString &exc)
        {
            QMetaObject::invokeMethod(&Parent,
                                      "enableDebugger",
                                      Qt::QueuedConnection,
                                      Q_ARG(bool, false));

            toLocker lock (Parent.Lock);
            Parent.TargetLog += QString::fromLatin1("Couldn't start debugging:");
            Parent.TargetLog += exc;
            Parent.DebuggerStarted = false;
            Parent.ChildSemaphore.up(); // resume main TOra thread
            return ;
        }
        while (1) // this will loop until target thread is required (while debugger tool is running)
        {
#ifdef DEBUG
            qDebug() << "toDebug::targetTask::run .........starting iteration....";
#endif
            {
                toLocker lock (Parent.Lock);
                Parent.RunningTarget = false;
            }
#ifdef DEBUG
            qDebug() << "toDebug::targetTask::run sleep until TargetSemaphore goes up";
#endif
            Parent.TargetSemaphore.down();
#ifdef DEBUG
            qDebug() << "toDebug::targetTask::run TargetSemaphore up! Continue!";
#endif

            QString sql;
            toQList inParams;
            toQList outParams;
            {
                toLocker lock (Parent.Lock);
                Parent.RunningTarget = true;
                sql = Parent.TargetSQL;
                Parent.TargetSQL = "";
#ifdef DEBUG
                qDebug() << "toDebug::targetTask::run Target now RUNNING! Got sql to execute/debug: " << sql;
#endif
                inParams = Parent.InputData;
                Parent.InputData.clear(); // To make sure data is not shared
                Parent.OutputData.clear();
            }
//             Parent.StartedSemaphore.up();
            if (sql.isEmpty())
                break;

            try
            {
                Parent.Lock.lock();
                Parent.TargetLog += QString::fromLatin1("Executing SQL\n");
                Parent.Lock.unlock();

                Parent.ChildSemaphore.up(); // resume main TOra thread
#ifdef DEBUG
                qDebug() << "toDebug::targetTask::run before execution of sql";
#endif
                outParams = targetSession.readQuery(sql, inParams);
#ifdef DEBUG
                qDebug() << "toDebug::targetTask::run after execution of sql.";
#endif
            }
            catch (const QString &str)
            {
#ifdef DEBUG
                qDebug() << "toDebug::targetTask::run ERROR! catch1. str = " << str;
#endif
                Parent.Lock.lock();
                Parent.TargetLog += QString::fromLatin1("Encountered error: ");
                Parent.TargetLog += str;
                if (!str.startsWith("ORA-06543:")) // PL/SQL: execution error - execution aborted
                    Parent.TargetException += str;
                Parent.TargetLog += QString::fromLatin1("\n");
                Parent.Lock.unlock();
            }
            catch (...)
            {
#ifdef DEBUG
                qDebug() << "toDebug::targetTask::run ERROR! catch2";
#endif
                Parent.Lock.lock();
                Parent.TargetLog += QString::fromLatin1("Encountered unknown exception\n");
                Parent.TargetException += QString::fromLatin1("Encountered unknown exception\n");
                Parent.Lock.unlock();
            }

            {
                toLocker lock (Parent.Lock);
                Parent.OutputData = outParams;
                Parent.TargetLog += QString::fromLatin1("Execution ended\n");
            }
            Parent.ChildSemaphore.up();
        }

    }
    TOCATCH
#ifdef DEBUG
    qDebug() << "toDebug::targetTask::run DONE. Finishing target session.";
#endif
    toLocker lock (Parent.Lock);
    Parent.DebuggerStarted = false;
    QMetaObject::invokeMethod(&Parent,
                              "enableDebugger",
                              Qt::QueuedConnection,
                              Q_ARG(bool, false));
    Parent.TargetLog += QString::fromLatin1("Closing debug session\n");
    Parent.TargetThread = NULL;
    Parent.ChildSemaphore.up(); // Resume main TOra thread
}

static toTreeWidgetItem *toLastItem(toTreeWidgetItem *parent)
{
    toTreeWidgetItem *lastItem = NULL;
    for (toTreeWidgetItem *item = parent->firstChild(); item; item = item->nextSibling())
        lastItem = item;
    return lastItem;
}

class toContentsItem : public toTreeWidgetItem
{
public:
    int Line;
    toContentsItem(toTreeWidgetItem *parent, const QString &name, int line)
            : toTreeWidgetItem(parent, toLastItem(parent), name)
    {
        Line = line;
    }
    toContentsItem(toTreeWidget *parent, const QString &name, const QString &id, int line)
            : toTreeWidgetItem(parent, name, id)
    {
        Line = line;
    }
};

void toDebug::reorderContent(toTreeWidgetItem *parent, int start, int diff)
{
    toTreeWidgetItem *next;
    for (toTreeWidgetItem *item = parent->firstChild(); item; item = next)
    {
        toContentsItem * cont = dynamic_cast<toContentsItem *>(item);
        next = item->nextSibling();
        bool del = false;
        if (cont)
        {
            int line = toHighlightedText::convertLine(cont->Line, start, diff);
            if (line >= 0)
                cont->Line = line;
            else
            {
                delete cont;
                del = true;
            }
        }
        if (!del && item->firstChild())
            reorderContent(item, start, diff);
    }
}

QString toDebug::editorName(const QString &schema, const QString &object, const QString &type)
{
    QString ret = connection().quote(schema) + "." + connection().quote(object);
    if (type.contains("BODY"))
        ret += tr(" body");
    return ret;

}

QString toDebug::editorName(toDebugText *text)
{
    return editorName(text->schema(), text->object(), text->type());
}

QString toDebug::currentSchema(void)
{
    return Schema->currentText();
}

toTreeWidgetItem *toDebug::contents(void)
{
    return Contents->firstChild();
}

void toDebug::reorderContent(int start, int diff)
{
    QString name = currentEditor()->objectName();
    for (toTreeWidgetItem *item = Contents->firstChild(); item; item = item->nextSibling())
    {
        if (item->text(1) == name)
            reorderContent(item, start, diff);
    }
}

bool toDebug::hasMembers(const QString &str)
{
    if (str == QString::fromLatin1("PACKAGE") || str == QString::fromLatin1("PACKAGE BODY") ||
            str == QString::fromLatin1("TYPE") || str == QString::fromLatin1("TYPE BODY"))
        return true;
    else
        return false;
}

QString toDebug::constructAnonymousBlock(toTreeWidgetItem * head, toTreeWidgetItem * last)
{
    // Anonymous plsql block has to be constructed calling function/procedure
    // requested for debugging and handling return value (if any).
    QString curName = currentEditor()->objectName();
    toHighlightedText *current = currentEditor();
    int curline, curcol;
    current->getCursorPosition (&curline, &curcol);

    bool valid = false;
    int line = -1;
    // Check if it is clear which procedure/function to run. This is only a question if
    // package is being tested, in which case cursor must be placed on procedure/function.
    if (hasMembers(currentEditor()->type()))
    {
        for (toTreeWidgetItem *parent = Contents->firstChild(); parent; parent = parent->nextSibling())
        {
#ifdef DEBUG
            qDebug() << parent->text(1);
#endif
            if (parent->text(1) == curName)
            {
                for (parent = parent->firstChild(); parent; parent = parent->nextSibling())
                {
                    toContentsItem * cont = dynamic_cast<toContentsItem *>(parent);
                    if (cont)
                    {
                        QString type = cont->text(0);
                        if (cont->Line > curline)
                            break;
                        if (cont->Line > line)
                        {
                            line = cont->Line;
                            if (type.startsWith("Procedure ") || type.startsWith("Function "))
                                valid = true;
                            else
                                valid = false;
                        }
                    }
                }
                break;
            }
        }
    }
    else if (currentEditor()->type() == QString::fromLatin1("PROCEDURE") ||
             currentEditor()->type() == QString::fromLatin1("FUNCTION"))
    {
        valid = true;
        line = 0;
    }

    // Only procede if it is clear which procedure/function should be tested
    if (valid)
    {
        try
        {
            QString token;
            int level = 0;

            enum
            {
                beginning,
                waitingEnd,
                returnType,
                parType,
                inOut,
                name,
                done
            } state = beginning;

            CurrentParams.clear();
            std::list<debugParam>::iterator cp = CurrentParams.begin();
            QString callName;
            QString retType;

            toSQLParse::editorTokenizer tokens(current, 0, line);

            do
            {
                token = tokens.getToken();
            }
            while (token.toUpper() == QString::fromLatin1("CREATE") || token.toUpper() == QString::fromLatin1("OR") ||
                    token.toUpper() == QString::fromLatin1("REPLACE"));

            if (token.toUpper() != QString::fromLatin1("FUNCTION") && token.toUpper() != QString::fromLatin1("PROCEDURE"))
            {
                toStatusMessage(tr("Expected function or procedure, internal error"));
                return QString("");
            }
            do
            {
                token = tokens.getToken();
                if (token.isEmpty())
                {
                    toStatusMessage(tr("Unexpected end of declaration."));
                    return QString("");
                }
                if (state == returnType)
                {
                    if (retType.isEmpty() || retType.at(retType.length() - 1) == '.' || token == QString::fromLatin1("."))
                        retType += token;
                    else
                        state = done;
                }
                else if (token.toUpper() == QString::fromLatin1("RETURN") && level == 0)
                {
                    state = returnType;
                }
                if (token == QString::fromLatin1("("))
                {
                    level++;
                }
                else if (token == QString::fromLatin1(")"))
                    level--;
                else if (level == 1)
                {
                    switch (state)
                    {
                    case name:
                    {
                        debugParam newPar;
                        CurrentParams.insert(CurrentParams.end(), newPar);
                        cp++;
                        (*cp).Name = token;
                        state = inOut;
                        break;
                    }
                    case inOut:
                        if (token.toUpper() == QString::fromLatin1("IN"))
                        {
                            (*cp).In = true;
                            break;
                        }
                        else if (token.toUpper() == QString::fromLatin1("OUT"))
                        {
                            (*cp).Out = true;
                            break;
                        }
                        else if (token.toUpper() == QString::fromLatin1("NOCOPY"))
                            break;
                        if (!(*cp).In && !(*cp).Out)
                            (*cp).In = true;
                        state = parType;
                    case parType:
                        if (token == QString::fromLatin1(","))
                        {
                            state = name;
                            break;
                        }
                        else if (token.toUpper() == QString::fromLatin1("DEFAULT") || token == QString::fromLatin1(":="))
                        {
                            state = waitingEnd;
                            break;
                        }
                        else
                        {
                            (*cp).Type += token;
                        }
                        break;
                    case waitingEnd:
                        if (token == QString::fromLatin1(","))
                            state = name;
                        else
                        {
                            if (token[0] == '\'' && token.length() >= 2)
                                token = token.mid(1, token.length() - 2);
                            if (token.toUpper() == QString::fromLatin1("NULL"))
                                toParamGet::setDefault(connection(), (*cp).Name, QString::null);
                            else
                                toParamGet::setDefault(connection(), (*cp).Name, token);
                        }
                        break;
                    case done:
                    case returnType:
                    case beginning:
                        break;
                    }
                }
                else if (state == beginning)
                {
                    callName = token;
                    state = name;
                }
            }
            while (state != done && token.toUpper() != "IS" && token.toUpper() != "AS" && token != ";");

            // construct an anonymous block calling chosen procedure/function
            QChar sep = '(';
            QString sql;
            if (!retType.isEmpty())
                sql += QString::fromLatin1("DECLARE\n  ret VARCHAR2(4000);\n");
            sql += QString::fromLatin1("BEGIN\n  ");
            if (!retType.isEmpty())
                sql += QString::fromLatin1("ret:=");
            sql += currentEditor()->schema();
            sql += QString::fromLatin1(".");
            if (hasMembers(currentEditor()->type()))
            {
                sql += currentEditor()->object();
                sql += QString::fromLatin1(".");
            }
            sql += callName;

            for (std::list<debugParam>::iterator i = CurrentParams.begin(); i != CurrentParams.end(); i++)
            {
                if ((*i).In)
                    last = new toResultViewItem(head, last, (*i).Name);
                sql += sep;
                sql += QString::fromLatin1(":");
                QString nam = (*i).Name;
                nam.replace(QRegExp(QString::fromLatin1("[^a-zA-Z0-9]+")), QString::fromLatin1("_"));
                sql += nam;
                sql += QString::fromLatin1("<char[");
                sql += QString::number(toConfigurationSingle::Instance().maxColSize());
                sql += QString::fromLatin1("],");
                if ((*i).In)
                    sql += QString::fromLatin1("in");
                if ((*i).Out)
                    sql += QString::fromLatin1("out");
                sql += QString::fromLatin1(">");
                sep = ',';
            }
            if (sep == ',')
                sql += QString::fromLatin1(")");
            if (!retType.isEmpty())
            {
                sql += QString::fromLatin1(";\n  SELECT ret INTO :tora_int_return<char[");
                sql += QString::number(toConfigurationSingle::Instance().maxColSize());
                sql += QString::fromLatin1("],out> FROM sys.DUAL");
            }
            sql += QString::fromLatin1(";\nEND;");

            return sql;
        }
        TOCATCH
    }
    else
        toStatusMessage(tr("Couldn't find any function or procedure under cursor."));

    return QString("");
} // constructAnonymousBlock

// Action when button "Execute/Run" is pressed
void toDebug::execute(void)
{
//qDebug() << "toDebug::execute";
    if (RunningTarget)
    {
        // If target is already running, continue to next breakpoint
        continueExecution(TO_BREAK_CONTINUE);
        return ;
    }

    // Check if current code has been modified in TOra and not commited to database.
    // If so - force user to compile code.
    if (!checkCompile())
        return ;

    // Anonymous plsql block has to be constructed calling function/procedure
    // requested for debugging and handling return value (if any).
    Parameters->clear();
    toTreeWidgetItem *head = new toResultViewItem(Parameters, NULL, tr("Input"));
    toTreeWidgetItem *last = NULL;
    head->setOpen(true);

    QString sql = constructAnonymousBlock(head, last);
    if (!sql.isEmpty())
    {
        // Can't hold lock since refresh of output will try to lock
        toQList input;
        try
        {
            // Parse generated anonymous block and check if there are any input
            // parameters to be entered by user. If there are any - display a
            // screen asking user to do so.
            input = toParamGet::getParam(connection(), this, sql);
        }
        catch (...)
        {
            return ;
        }
        {
            toLocker lock (Lock);
            InputData = input;
            last = head->firstChild();
            if (InputData.begin() != InputData.end())
                for (toQList::iterator i = InputData.begin();
                        last && i != InputData.end();
                        i++, last = last->nextSibling())
                {
                    // Is there a smarter way to make a deep copy
                    last->setText(1, toDeepCopy(*i));
                }
            else
                delete head;
            TargetSQL = toDeepCopy(sql); // Deep copy of SQL
        }
#ifdef DEBUG
        qDebug() << "toDebug::execute sql has been prepared. upping TargetSemaphore!";
#endif
        TargetSemaphore.up(); // Go go power rangers!
        ChildSemaphore.down(); // Wait until target actually runs the statement
//        StartedSemaphore.down();
#ifdef DEBUG
        qDebug() << "toDebug::execute before sync()";
#endif
        if (sync() >= 0 && RunningTarget)
            continueExecution(TO_BREAK_ANY_CALL);
#ifdef DEBUG
        qDebug() << "toDebug::execute after sync() & continueExecution";
#endif
    }
#ifdef DEBUG
    qDebug() << "toDebug::execute DONE!";
#endif
} // execute

static toSQL SQLSync("toDebug:Sync",
                     "DECLARE\n"
                     "  ret binary_integer;\n"
                     "  runinf SYS.DBMS_DEBUG.Runtime_Info;\n"
                     "BEGIN\n"
                     "  ret:=SYS.DBMS_DEBUG.SYNCHRONIZE(runinf,:info<int,in>);\n"
                     "  SELECT ret,\n"
                     "         runinf.Reason\n"
                     "    INTO :ret<int,out>,\n"
                     "         :reason<int,out>\n"
                     "    FROM sys.DUAL;\n"
                     "END;",
                     "Sync with the debugging session, must have same binds");

int toDebug::sync(void)
{
//qDebug() << "toDebug::sync";
    try
    {
        toQList args;
        toPush(args, toQValue(0));
        int ret;
        int reason;
        do
        {
#ifdef DEBUG
            qDebug() << "toDebug::sync DBMS_DEBUG.SYNCHRONIZE";
#endif
            debugSession->execute(SQLSync, args);
            ret = debugSession->readValue().toInt();
            reason = debugSession->readValue().toInt();
#ifdef DEBUG
            if (ret == TO_ERROR_COMMUNICATION)
            {
                qDebug() << "Error in communication with target session.";
            }
            qDebug() << "toDebug::sync ret = " << ret << ", reason = " << getErrorText(reason, 2);
#endif
            {
                toLocker lock (Lock);
                TargetLog += QString::fromLatin1("Syncing debug session\n");
                if (!RunningTarget)
                {
#ifdef DEBUG
                    qDebug() << "toDebug::sync no running target. exit";
#endif
                    return TO_REASON_KNL_EXIT;
                }
            }
#if 0
            qApp->processEvents();
#endif
        }
        while (reason == TO_REASON_TIMEOUT || ret == TO_ERROR_TIMEOUT);
#ifdef DEBUG
        qDebug() << "toDebug::sync got reason = " << getErrorText(reason, 2);
#endif
        setDeferedBreakpoints();
        if (Output->enabled())
            Output->disable(false);
        else
            Output->disable(true);
#ifdef DEBUG
        qDebug() << "toDebug::sync returning";
#endif
        return reason;
    }
    TOCATCH
    return -1;
}

static struct TypeMapType
{
    const char *Type;
    const char *Description;
    bool WantName;
    bool Declaration;
}
TypeMap[] = { { "FUNCTION", "Function", true , true },    // Must be first in list
    { "PROCEDURE", "Procedure", true , true },
    { "PACKAGE", "Package", true , true },
    { "DECLARE", "Anonymous", false, true },
    { "TYPE", "Type", true , false},
    { "CURSOR", "Cursor", true , false},
    { "IF", "Conditional", false, false},
    { "LOOP", "Loop", false, false},
    { "WHILE", "Loop", false, false},
    { "FOR", "Loop", false, false},
    { NULL, NULL, false, false}
};

static bool FindKeyword(toSQLParse::statement &statements, bool onlyNames, bool &declaration, int &line, QString &name)
{
    if (statements.Type == toSQLParse::statement::Keyword ||
            statements.Type == toSQLParse::statement::Token)
    {
        line = statements.Line;
        if (name.isEmpty())
        {
            name = statements.String.toUpper();

            int j;
            for (j = 0; TypeMap[j].Type && TypeMap[j].Type != name; j++)
                ;
            if (TypeMap[j].Type)
                name = TypeMap[j].Description;
            else
                name = "Anonymous";

            declaration = TypeMap[j].Declaration;

            if (onlyNames && !TypeMap[j].WantName)
            {
                name = QString::null;
                return true;
            }

            return !TypeMap[j].WantName;
        }
        else if (statements.String.toUpper() != "BODY")
        {
            name += " " + statements.String;
            return true;
        }
        else
            name += " body";
    }
    for (std::list<toSQLParse::statement>::iterator i = statements.subTokens().begin(); i != statements.subTokens().end(); i++)
    {
        bool ret = FindKeyword(*i, onlyNames, declaration, line, name);
        if (ret)
            return ret;
    }
    return false;
}

void toDebug::updateArguments(toSQLParse::statement &statements, toTreeWidgetItem *parent)
{
    for (std::list<toSQLParse::statement>::iterator i = statements.subTokens().begin(); i != statements.subTokens().end(); i++)
    {
        if ((*i).Type == toSQLParse::statement::List)
        {
            bool first = true;
            for (std::list<toSQLParse::statement>::iterator j = (*i).subTokens().begin(); j != (*i).subTokens().end(); j++)
            {
                if ((*j).String == ",")
                    first = true;
                else if (first)
                {
                    new toContentsItem(parent, "Parameter " + (*j).String, (*j).Line);
                    first = false;
                }
            }
        }
    }
}

void toDebug::updateContent(toSQLParse::statement &statements, toTreeWidgetItem *parent, const QString &id)
{
    toTreeWidgetItem *item = NULL;
    int line;
    QString name;
    bool declaration;
    if (!FindKeyword(statements, statements.Type == toSQLParse::statement::Statement, declaration, line, name) || name.isNull())
        return ;

    if (parent)
        item = new toContentsItem(parent, name, line);
    else
    {
        for (item = Contents->firstChild(); item; item = item->nextSibling())
            if (item->text(0) == name && item->text(1) == id)
            {
                item->setText(2, QString::null);
                break;
            }
        if (!item)
            item = new toContentsItem(Contents, name, id, line);
        else
        {
            while (item->firstChild())
                delete item->firstChild();
            toContentsItem *ci = dynamic_cast<toContentsItem *>(item);
            if (ci)
                ci->Line = line;
        }
        item->setOpen(true);
    }

    std::list<toSQLParse::statement>::iterator i = statements.subTokens().begin();
    if (statements.Type == toSQLParse::statement::Block)
    {
        if (i != statements.subTokens().end())
        {
            updateArguments(*i, item);
            i++;
        }
    }
    else
    {
        updateArguments(statements, item);
        if (i != statements.subTokens().end())
            i++;
    }
    while (i != statements.subTokens().end())
    {
        if ((*i).Type == toSQLParse::statement::Block || (*i).Type == toSQLParse::statement::Statement)
        {
            if (declaration)
            {
                std::list<toSQLParse::statement>::iterator j = (*i).subTokens().begin();
                if (j != (*i).subTokens().end())
                {
                    if ((*j).String.toUpper() == "BEGIN")
                        declaration = false;
                    else if ((*j).Type == toSQLParse::statement::Token && (*j).String.toUpper() != "END")
                        new toContentsItem(item, "Variable " + (*j).String, (*j).Line);
                }
            }
            updateContent(*i, item);
        }
        i++;
    }
    if (!parent && !item->firstChild())
        delete item;
}

void toDebug::updateContent(toDebugText *current)
{
    toSQLParse::editorTokenizer tokenizer(current);
    std::list<toSQLParse::statement> statements = toSQLParse::parse(tokenizer);

    toTreeWidgetItem *item;

    for (item = Contents->firstChild(); item; item = item->nextSibling())
        if (item->text(1) == current->objectName())
            item->setText(2, "DELETE");

    for (std::list<toSQLParse::statement>::iterator i = statements.begin(); i != statements.end(); i++)
        updateContent(*i, NULL, current->objectName());

    toTreeWidgetItem *ni;
    for (item = Contents->firstChild(); item; item = ni)
    {
        ni = item->nextSibling();
        if (item->text(2) == "DELETE")
            delete item;
    }
}

void toDebug::readLog(void)
{
#ifdef DEBUG
    qDebug() << "toDebug::readLog";
#endif
    toLocker lock (Lock);
    if (!TargetLog.isEmpty())
    {
        TargetLog.replace(TargetLog.length() - 1, 1, QString::null);
        RuntimeLog->insert(TargetLog);
        RuntimeLog->setCursorPosition(RuntimeLog->lines() - 1, 0);
        TargetLog = QString::null;
    }
    if (!TargetException.isEmpty())
    {
        toStatusMessage(TargetException);
        TargetException = QString::null;
    }
}

static toSQL SQLRuntimeInfo("toDebug:RuntimeInfo",
                            "DECLARE\n"
                            "  info SYS.DBMS_DEBUG.RUNTIME_INFO;\n"
                            "  ret BINARY_INTEGER;\n"
                            "BEGIN\n"
                            "  ret:=SYS.DBMS_DEBUG.GET_RUNTIME_INFO(SYS.DBMS_DEBUG.info_getStackDepth,info);\n"
                            "  SELECT ret,info.stackDepth INTO :ret<int,out>,:depth<int,out> FROM sys.DUAL;\n"
                            "EXCEPTION WHEN OTHERS THEN\n"
                            "  SELECT -1,-1 INTO :ret<int,out>,:depth<int,out> FROM sys.DUAL;\n"
                            "END;",
                            "Get runtime info from debug session, must have same bindings");
static toSQL SQLStackTrace("toDebug:StackTrace",
                           "DECLARE\n"
                           "  info SYS.DBMS_DEBUG.PROGRAM_INFO;\n"
                           "  stack SYS.DBMS_DEBUG.BACKTRACE_TABLE;\n"
                           "  i BINARY_INTEGER;\n"
                           "BEGIN\n"
                           "  SYS.DBMS_DEBUG.PRINT_BACKTRACE(stack);\n"
                           "  i:=:num<int,in>;\n"
                           "  info:=stack(i);\n"
                           "  SELECT info.Name,info.Owner,info.Line#,DECODE(info.LibunitType,SYS.DBMS_DEBUG.LibunitType_cursor,'CURSOR',\n"
                           "                                                                 SYS.DBMS_DEBUG.LibunitType_function,'FUNCTION',\n"
                           "                                                                 SYS.DBMS_DEBUG.LibunitType_procedure,'PROCEDURE',\n"
                           "                                                                 SYS.DBMS_DEBUG.LibunitType_package,'PACKAGE',\n"
                           "                                                                 SYS.DBMS_DEBUG.LibunitType_package_body,'PACKAGE BODY',\n"
                           "                                                                 SYS.DBMS_DEBUG.LibunitType_trigger,'TRIGGER',\n"
                           "                                                                 'UNKNOWN')\n"
                           "    INTO :name<char[101],out>,:owner<char[101],out>,:line<int,out>,:type<char[101],out> FROM sys.DUAL;\n"
                           "END;",
                           "Get stacktrace from debug session, must have same bindings");
static toSQL SQLLocalWatch("toDebug:LocalWatch",
                           "DECLARE\n"
                           "  ret BINARY_INTEGER;\n"
                           "  data VARCHAR2(4000);\n"
                           "BEGIN\n"
                           "  ret:=SYS.DBMS_DEBUG.GET_VALUE(:name<char[101],in>,0,data,NULL);\n"
                           "  SELECT ret,data INTO :ret<int,out>,:val<char[4001],out> FROM sys.DUAL;\n"
                           "END;",
                           "Get data from local watch, must have same bindings");
static toSQL SQLGlobalWatch("toDebug:GlobalWatch",
                            "DECLARE\n"
                            "  data VARCHAR2(4000);\n"
                            "  proginf SYS.DBMS_DEBUG.program_info;\n"
                            "  ret BINARY_INTEGER;\n"
                            "BEGIN\n"
                            "  proginf.Namespace:=SYS.DBMS_DEBUG.Namespace_pkg_body;\n"
                            "  proginf.Name:=:object<char[101],in>;\n"
                            "  proginf.Owner:=:owner<char[101],in>;\n"
                            "  proginf.DBLink:=NULL;\n"
                            "  ret:=SYS.DBMS_DEBUG.GET_VALUE(:name<char[101],in>,proginf,data,NULL);\n"
                            "  IF ret =SYS.DBMS_DEBUG.error_no_such_object THEN\n"
                            "    proginf.Namespace:=SYS.DBMS_DEBUG.namespace_pkgspec_or_toplevel;\n"
                            "    ret:=SYS.DBMS_DEBUG.GET_VALUE(:name<char[101],in>,proginf,data,NULL);\n"
                            "  END IF;\n"
                            "  SELECT ret          ,data                ,proginf.Namespace\n"
                            "    INTO :ret<int,out>,:val<char[4001],out>,:namespace<int,out>\n"
                            "    FROM sys.DUAL;\n"
                            "END;",
                            "Get data from global watch, must have same bindings");
static toSQL SQLLocalIndex("toDebug:LocalIndex",
                           "DECLARE\n"
                           "  ret BINARY_INTEGER;\n"
                           "  proginf SYS.DBMS_DEBUG.program_info;\n"
                           "  i BINARY_INTEGER;\n"
                           "  indata SYS.DBMS_DEBUG.index_table;\n"
                           "  outdata VARCHAR2(4000);\n"
                           "BEGIN\n"
                           "  ret:=SYS.DBMS_DEBUG.GET_INDEXES(:name<char[101],in>,0,proginf,indata);\n"
                           "  IF ret = SYS.DBMS_DEBUG.success THEN\n"
                           "    i:=indata.first;\n"
                           "    WHILE i IS NOT NULL AND (LENGTH(outdata)<3900 OR outdata IS NULL) LOOP\n"
                           "      outdata:=outdata||indata(i)||',';\n"
                           "      i:=indata.next(i);\n"
                           "    END LOOP;\n"
                           "  END IF;\n"
                           "  SELECT outdata INTO :data<char[4001],out> FROM sys.DUAL;\n"
                           "END;",
                           "Get indexes of local watch, must have same bindings");
static toSQL SQLGlobalIndex("toDebug:GlobalIndex",
                            "DECLARE\n"
                            "  ret BINARY_INTEGER;\n"
                            "  proginf SYS.DBMS_DEBUG.program_info;\n"
                            "  i BINARY_INTEGER;\n"
                            "  indata SYS.DBMS_DEBUG.index_table;\n"
                            "  outdata VARCHAR2(4000);\n"
                            "BEGIN\n"
                            "  proginf.Namespace:=:namespace<int,in>;\n"
                            "  proginf.Name:=:object<char[101],in>;\n"
                            "  proginf.Owner:=:owner<char[101],in>;\n"
                            "  proginf.DBLink:=NULL;\n"
                            "  ret:=SYS.DBMS_DEBUG.GET_INDEXES(:name<char[101],in>,NULL,proginf,indata);\n"
                            "  IF ret = SYS.DBMS_DEBUG.success THEN\n"
                            "    i:=indata.first;\n"
                            "    WHILE i IS NOT NULL AND (LENGTH(outdata)<3900 OR outdata IS NULL) LOOP\n"
                            "      outdata:=outdata||indata(i)||',';\n"
                            "      i:=indata.next(i);\n"
                            "    END LOOP;\n"
                            "  END IF;\n"
                            "  SELECT outdata INTO :data<char[4001],out> FROM sys.DUAL;\n"
                            "END;",
                            "Get indexes of global watch, must have same bindings");

// Called from updateState. Processes watches.
void toDebug::processWatches(void)
{
    toTreeWidgetItem *next = NULL;
    for (toTreeWidgetItem *item = Watch->firstChild(); item; item = next)
    {
#ifdef DEBUG
        qDebug() << "toDebug::processWatches watch = " << item->text(2);
#endif
        int ret = -1;
        int space = 0;
        QString value;
        bool local = false;
        QString object;
        QString schema;
        if (!item->text(6).isEmpty())
        {
            local = true;
            /* Get value of given variable. Note, that this call can semi-fail raising
               exception telling that given object/variable is:
               * The object is a table, but no index was provided
               * Value of current variable is NULL
            */
            debugSession->execute(SQLLocalWatch, item->text(2));
            ret = debugSession->readValue().toInt();
            value = debugSession->readValue();
            if (ret != TO_SUCCESS &&
                    ret != TO_ERROR_NULLVALUE &&
                    ret != TO_ERROR_INDEX_TABLE &&
                    ret != TO_ERROR_NULLCOLLECTION)
            {
                object = currentEditor()->object();
                schema = currentEditor()->schema();
                local = false;
                debugSession->execute(SQLGlobalWatch,
                                      object,
                                      schema,
                                      item->text(2));
                ret = debugSession->readValue().toInt();
                value = debugSession->readValue();
                space = debugSession->readValue().toInt();
                item->setText(0, schema);
                item->setText(1, object);
            }
        }
        else if (item->text(0).isEmpty())
        {
            debugSession->execute(SQLLocalWatch, item->text(2));
            ret = debugSession->readValue().toInt();
            value = debugSession->readValue();
            local = true;
        }
        else
        {
            object = item->text(1);
            schema = item->text(0);
            debugSession->execute(SQLGlobalWatch,
                                  object, schema, item->text(2));
            ret = debugSession->readValue().toInt();
            value = debugSession->readValue();
            space = debugSession->readValue().toInt();
            local = false;
        }
        item->setText(4, QString::null);
        if (ret == TO_SUCCESS)
            item->setText(3, value);
        else if (ret == TO_ERROR_NULLVALUE)
        {
            if (toConfigurationSingle::Instance().indicateEmpty())
                item->setText(3, QString::fromLatin1("{null}"));
            else
                item->setText(3, QString::null);
            item->setText(5, QString::fromLatin1("NULL"));
        }
        else if (ret == TO_ERROR_NULLCOLLECTION)
        {
            item->setText(3, tr("[Count %1]").arg(0));
            item->setText(5, QString::fromLatin1("LIST"));
        }
        else if (ret == TO_ERROR_INDEX_TABLE)
        {
            if (local)
            {
                // this call get's a list of indexes separated with comma (with trailing comma)
                // example:  -1,0,1,2,5,8,
                debugSession->execute(SQLLocalIndex, item->text(2));
                value = debugSession->readValue();
            }
            else
            {
                toQList args;
                toPush(args, toQValue(space));
                toPush(args, toQValue(object));
                toPush(args, toQValue(schema));
                toPush(args, toQValue(item->text(2)));
                debugSession->execute(SQLGlobalIndex, args);
                value = debugSession->readValue();
            }
            int start = 0;
            int end;
            toTreeWidgetItem *last = NULL;
            int num = 0;
            for (end = start; end < value.length(); end++)
            {
                if (value.at(end) == ',')
                {
                    if (start < end)
                    {
                        QString name = item->text(2);
                        name += QString::fromLatin1("(");
                        name += QString::number(value.mid(start, end - start).toInt());
                        name += QString::fromLatin1(")");
                        last = new toResultViewItem(item, last);
                        last->setText(0, schema);
                        last->setText(1, object);
                        last->setText(2, name);
                        last->setText(3, value.mid(start, end - start));
                        last->setText(4, QString::fromLatin1("NOCHANGE"));
                        num++;
                    }
                    start = end + 1;
                }
            }
            QString str = tr("[Count %1]").arg(num);
            item->setText(3, str);
            item->setText(5, QString::fromLatin1("LIST"));
        }
        else
        {
            item->setText(3, ret == TO_ERROR_NO_DEBUG_INFO ? tr("{No debug info}") : tr("{Unavailable}"));
            item->setText(4, QString::fromLatin1("NOCHANGE"));
        }
        if (item->firstChild())
            next = item->firstChild();
        else if (item->nextSibling())
            next = item->nextSibling();
        else
        {
            next = item;
            do
            {
                next = next->parent();
            }
            while (next && !next->nextSibling());
            if (next)
                next = next->nextSibling();
        }
    }
} // processWatches

void toDebug::updateState(int reason)
{
#ifdef DEBUG
    qDebug() << "toDebug::updateState, reason = " << getErrorText(reason, 2);
#endif
    switch (reason)
    {
    case TO_REASON_EXIT:
    case TO_REASON_KNL_EXIT:
        ChildSemaphore.down();
        // Intentionally no break here
    case TO_REASON_NO_SESSION:
        stopAct->setEnabled(false);
        stepAct->setEnabled(false);
        nextAct->setEnabled(false);
        returnAct->setEnabled(false);
        {
            for (int i = 0; i < Editors->count(); i++)
            {
                toDebugText *editor = dynamic_cast<toDebugText *>(Editors->widget(i));
                editor->setCurrent( -1);
            }
            StackTrace->clear();
            toLocker lock (Lock);
            if (OutputData.begin() != OutputData.end())
            {
                toTreeWidgetItem *head = Parameters->firstChild();
                while (head && head->nextSibling())
                    head = head->nextSibling();
                head = new toResultViewItem(Parameters, head, tr("Output"));
                head->setOpen(true);
                std::list<debugParam>::iterator cp;
                for (cp = CurrentParams.begin(); cp != CurrentParams.end() && !(*cp).Out; cp++)
                    ;

                toTreeWidgetItem *last = NULL;
                {
                    for (toQList::iterator i = OutputData.begin(); i != OutputData.end(); i++)
                    {
                        QString name;
                        if (cp != CurrentParams.end())
                        {
                            name = (*cp).Name;
                            for (cp++; cp != CurrentParams.end() && !(*cp).Out; cp++)
                                ;
                        }
                        if (name.isEmpty())
                            name = tr("Returning");
                        last = new toResultViewItem(head, last, name);
                        last->setText(1, toDeepCopy(*i)); // Deep copy just to be sure
                    }
                }
            }
            toTreeWidgetItem *next = NULL;
            for (toTreeWidgetItem *item = Watch->firstChild(); item; item = next)
            {
                item->setText(4, QString::fromLatin1("NOCHANGE"));
                if (item->firstChild())
                    next = item->firstChild();
                else if (item->nextSibling())
                    next = item->nextSibling();
                else
                {
                    next = item;
                    do
                    {
                        next = next->parent();
                    }
                    while (next && !next->nextSibling());
                    if (next)
                        next = next->nextSibling();
                }
            }
        }
        break;
    default:
        debugPaneAct->setChecked(true);
        stopAct->setEnabled(true);
        stepAct->setEnabled(true);
        nextAct->setEnabled(true);
        returnAct->setEnabled(true);
        try
        {
#ifdef DEBUG
            qDebug() << "toDebug::updateState DBMS_DEBUG.GET_RUNTIME_INFO";
#endif
            debugSession->execute(SQLRuntimeInfo);
            int ret, depth;
            ret = debugSession->readValue().toInt();
            depth = debugSession->readValue().toInt();
#ifdef DEBUG
            qDebug() << "toDebug::updateState ret = " << ret << ", depth = " << depth;
#endif
            if (ret != TO_SUCCESS)
            {
                toStatusMessage(tr("Failed to get runtime info (Reason %1)").arg(ret));
#ifdef DEBUG
                qDebug() << "toDebug::updateState FAILED to get runtime info. Reason " << getErrorText(ret, 1/*error*/);
#endif
                return ;
            }

            toTreeWidgetItem *item = NULL;
            StackTrace->clear();
            QString name;
            QString schema;
            QString line;
            QString type;

            for (int num = 2; num <= depth; num++)
            {
                toQList args;
                toPush(args, toQValue(num));
#ifdef DEBUG
                qDebug() << "toDebug::updateState DBMS_DEBUG.PRINT_BACKTRACE depth=" << num;
#endif
                debugSession->execute(SQLStackTrace, args);

                name = debugSession->readValue();
                schema = debugSession->readValue();
                line = debugSession->readValue();
                type = debugSession->readValue();

                if (!item)
                    item = new toTreeWidgetItem(StackTrace, name, line, schema, type);
                else
                    item = new toTreeWidgetItem(item, name, line, schema, type);
                item->setOpen(true);
            }
            Output->refresh();
            try
            {
                for (toTreeWidgetItem *item = Watch->firstChild(); item; item = item->nextSibling())
                {
                    while (item->firstChild())
                        delete item->firstChild();
                }
                processWatches();
            }
            TOCATCH
            // Stack depth of 1 means that control is in TOra generated anonymous
            // block. Therefore we should continue until control is in at least
            // depth 2 - in function/procedure being debugged.
            if (depth >= 2)
            {
                viewSource(schema, name, type, line.toInt(), true);
            }
            else
            {
                if (RunningTarget)
                    continueExecution(TO_BREAK_ANY_CALL); // break any call will go INTO code being debugged
                return ;
            }
        }
        TOCATCH
        break;
    }
    selectedWatch();
    readLog();
} // updateState

#if 0 // Not used yet
QString toDebug::checkWatch(const QString &name)
{
    int ret = -1;
    int space = 0;

    bool local;
    QString object;
    QString schema;
    QString value;
    local = true;
    toQuery query(connection(), SQLLocalWatch, name);
    ret = query.readValue().toInt();
    value = query.readValue();
    if (ret != TO_SUCCESS &&
            ret != TO_ERROR_NULLVALUE &&
            ret != TO_ERROR_INDEX_TABLE &&
            ret != TO_ERROR_NULLCOLLECTION)
    {
        object = currentEditor()->object();
        schema = currentEditor()->schema();
        local = false;
        toQuery q2(connection(), SQLGlobalWatch,
                   object,
                   schema,
                   name);
        ret = q2.readValue().toInt();
        value = q2.readValue();
        space = q2.readValue().toInt();
    }
    if (ret == TO_SUCCESS)
        return value;
    else if (ret == TO_ERROR_NULLVALUE)
        return "{null}";
    else if (ret == TO_ERROR_NULLCOLLECTION)
        return tr("[Count %1]").arg(0);
    else if (ret == TO_ERROR_INDEX_TABLE)
    {
        if (local)
        {
            toQuery query(connection(), SQLLocalIndex, name);
            value = query.readValue();
        }
        else
        {
            toQList args;
            toPush(args, toQValue(space));
            toPush(args, toQValue(object));
            toPush(args, toQValue(schema));
            toPush(args, toQValue(name));
            toQuery query(connection(), SQLGlobalIndex, args);
            value = query.readValue();
        }
        unsigned int start = 0;
        unsigned int end;
        int num = 0;
        QString ret;
        for (end = start; end < value.length(); end++)
        {
            if (value.at(end) == ',')
            {
                if (start < end)
                {
                    ret += "\n" + value.mid(start, end - start);
                    num++;
                }
                start = end + 1;
            }
        }
        return tr("[Count %1]").arg(num) + ret;
    }
    return QString::null;
}
#endif

bool toDebug::viewSource(const QString &schema, const QString &name, const QString &type,
                         int line, bool setCurrent)
{
    try
    {
        toDebugText *editor = NULL;
        int row = line - 1;
        int col = 0;
        for (int i = 0; i < Editors->count(); i++)
        {
            QString tabname = editorName(schema, name, type);
            toDebugText *te = dynamic_cast<toDebugText *>(Editors->widget(i));
            if (Editors->tabText(Editors->indexOf(te)) == tabname)
            {
                editor = te;
                break;
            }
            if (Editors->tabText(Editors->indexOf(te)) == tr("Unknown") && !te->isModified())
                editor = te;
        }
        if (!editor)
        {
            editor = new toDebugText(Breakpoints, Editors, this);
            // signal removed with qscintilla port.
//             connect(editor, SIGNAL(insertedLines(int, int)),
//                     this, SLOT(reorderContent(int, int)));
            Editors->addTab(editor, editorName(editor));
        }
        else
        {
            editor->getCursorPosition(&row, &col);
        }
        if (editor->lines() <= 1)
        {
            editor->setData(schema, type, name);
            editor->readData(connection(), StackTrace);
            updateContent(editor);
            Editors->setTabText(Editors->indexOf(editor), editorName(editor));
            if (editor->hasErrors())
                Editors->setTabIcon(Editors->indexOf(editor),
                                    QIcon(QPixmap(const_cast<const char**>(nextbug_xpm))));
            else
                Editors->setTabIcon(Editors->indexOf(editor), QIcon());
        }
        Editors->setCurrentIndex(Editors->indexOf(editor));
        editor->setCursorPosition(row, col);
        if (setCurrent)
            editor->setCurrent(line - 1);
        editor->setFocus();
        return true;
    }
    catch (const QString &str)
    {
        toStatusMessage(str);
        return false;
    }
}

void toDebug::setDeferedBreakpoints(void)
{
#ifdef DEBUG
    qDebug() << "toDebug::setDeferedBreakpoints";
#endif
    for (toTreeWidgetItem *item = Breakpoints->firstChild(); item; item = item->nextSibling())
    {
        toBreakpointItem * point = dynamic_cast<toBreakpointItem *>(item);
        if (point)
        {
            if (point->text(4) == tr("DEFERED"))
                point->setBreakpoint();
        }
    }
}

static toSQL SQLContinue("toDebug:Continue",
                         "DECLARE\n"
                         "  runinf SYS.DBMS_DEBUG.runtime_info;\n"
                         "  ret BINARY_INTEGER;\n"
                         "BEGIN\n"
                         "  ret:=SYS.DBMS_DEBUG.CONTINUE(runinf,:break<int,in>,NULL);\n"
                         "  SELECT ret,\n"
                         "         runinf.Reason\n"
                         "    INTO :ret<int,out>,\n"
                         "         :reason<int,out>\n"
                         "    FROM sys.DUAL;\n"
                         "END;",
                         "Continue execution, must have same bindings");

int toDebug::continueExecution(int stopon)
{
#ifdef DEBUG
    qDebug() << "toDebug::continueExecution(" << getErrorText(stopon, 3) << ")";
#endif
    Lock.lock();
    if (RunningTarget)
    {
        Lock.unlock();
        try
        {
            int ret, reason;
            setDeferedBreakpoints();
            while (1)
            {
                toQList args;
                toPush(args, toQValue(stopon));
                debugSession->execute(SQLContinue, args);
                ret = debugSession->readValue().toInt();
                reason = debugSession->readValue().toInt();
#ifdef DEBUG
                qDebug() << "toDebug::continueExecution ret=" << ret << ", reason=" << reason;
#endif
                if (reason == TO_REASON_TIMEOUT || ret == TO_ERROR_TIMEOUT)
                {
                    reason = sync();
                    if (reason < 0)
                        ret = -1;
                    else
                        ret = TO_SUCCESS;
                }

                if (ret != TO_SUCCESS)
                    return -1;
                if (reason != TO_REASON_STARTING)
                    break;
            }
            updateState(reason);
            return reason;
        }
        TOCATCH
    }
    else
    {
        toStatusMessage(tr("No running target"));
        Lock.unlock();
        readLog();
    }
    return -1;
}

void toDebug::executeInTarget(const QString &str, toQList &params)
{
#ifdef DEBUG
    qDebug() << "toDebug::executeInTarget 1" << str;
#endif
    toBusy busy;
    {
        toLocker lock (Lock);
        TargetSQL = toDeepCopy(str);
        InputData = params;
        TargetSemaphore.up();
    }
//     StartedSemaphore.down();

    int ret = sync();
    while (ret >= 0 && ret != TO_REASON_EXIT && ret != TO_REASON_KNL_EXIT && RunningTarget)
    {
        ret = continueExecution(TO_BREAK_ANY_RETURN);
#ifdef DEBUG
//        qDebug() << "ret = " << ret;
#endif
    }
#ifdef DEBUG
    qDebug() << "out of loop";
#endif
    readLog();
//     qDebug() << "toDebug::executeInTarget 2";
}

void toDebug::stop(void)
{
    if (RunningTarget)
        continueExecution(TO_ABORT_EXECUTION);
}

toDebug::toDebug(QWidget *main, toConnection &connection)
        : toToolWidget(DebugTool, "debugger.html", main, connection, "toDebug"),
        TargetThread()
{
    debugSession = new toQuery(connection);

    createActions();

    // these buttons should only be enabled after starting debug process
    stopAct->setEnabled(false);
    stepAct->setEnabled(false);
    nextAct->setEnabled(false);
    returnAct->setEnabled(false);

    QToolBar *toolbar = toAllocBar(this, tr("Debugger"));
    layout()->addWidget(toolbar);

    toolbar->addAction(refreshAct);

    toolbar->addSeparator();

    Schema = new QComboBox(toolbar);
    Schema->setObjectName(TO_TOOLBAR_WIDGET_NAME);
    toolbar->addWidget(Schema);
    connect(Schema,
            SIGNAL(activated(int)),
            this,
            SLOT(changeSchema(int)));

    toolbar->addSeparator();

    toolbar->addAction(newSheetAct);
    toolbar->addAction(scanSourceAct);
    toolbar->addAction(compileAct);

    toolbar->addSeparator();

    toolbar->addAction(executeAct);
    toolbar->addAction(stopAct);

    toolbar->addSeparator();

    toolbar->addAction(stepAct);
    toolbar->addAction(nextAct);
    toolbar->addAction(returnAct);

    toolbar->addSeparator();

    toolbar->addAction(debugPaneAct);

    toolbar->addSeparator();

    toolbar->addAction(nextErrorAct);
    toolbar->addAction(previousErrorAct);

    toolbar->addSeparator();

    toolbar->addAction(toggleBreakAct);
    toolbar->addAction(disableBreakAct);

    toolbar->addSeparator();

    toolbar->addAction(addWatchAct);
    toolbar->addAction(deleteWatchAct);

    toolbar->addAction(changeWatchAct);

    toolbar->addWidget(new toSpacer());

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(splitter);

    QSplitter *hsplitter = new QSplitter(Qt::Horizontal, splitter);
    DebugTabs = new QTabWidget(splitter);
    DebugTabs->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum));
    DebugTabs->setMinimumHeight(1);
    QList<int> sizes = splitter->sizes();
    sizes[1] = 200;
    splitter->setSizes(sizes);
    DebugTabs->hide();

    //splitter->setResizeMode(DebugTabs, QSplitter::KeepSize);

    QSplitter *objSplitter = new QSplitter(Qt::Vertical, hsplitter);

    Objects = new QTreeView(objSplitter);
    CodeModel = new toCodeModel(Objects);
    Objects->setModel(CodeModel);
    QString selected = Schema->currentText();
    if (!selected.isEmpty())
        CodeModel->refresh(connection, selected);
//     Objects->setSelectionMode(toTreeWidget::Single);
//     Objects->setResizeMode(toTreeWidget::AllColumns);
    connect(Objects->selectionModel(),
            SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this,
            SLOT(changePackage(const QModelIndex &, const QModelIndex &)));

    Contents = new toListView(objSplitter);
    Contents->addColumn(tr("Contents"));
    Contents->setRootIsDecorated(true);
    Contents->setSorting( -1);
    Contents->setTreeStepSize(10);
    Contents->setSelectionMode(toTreeWidget::Single);
    Contents->setResizeMode(toTreeWidget::AllColumns);
    connect(Contents, SIGNAL(selectionChanged(toTreeWidgetItem *)),
            this, SLOT(changeContent(toTreeWidgetItem *)));

    StackTrace = new toListView(DebugTabs);
    StackTrace->addColumn(tr("Object"));
    StackTrace->addColumn(tr("Line"));
    StackTrace->addColumn(tr("Schema"));
    StackTrace->addColumn(tr("Type"));
    StackTrace->setColumnAlignment(1, Qt::AlignRight);
    StackTrace->setSorting( -1);
    StackTrace->setRootIsDecorated(true);
    StackTrace->setTreeStepSize(10);
    StackTrace->setAllColumnsShowFocus(true);
    StackTrace->setResizeMode(toTreeWidget::AllColumns);
    DebugTabs->addTab(StackTrace, tr("&Stack Trace"));
    connect(StackTrace, SIGNAL(clicked(toTreeWidgetItem *)),
            this, SLOT(showSource(toTreeWidgetItem *)));

    Watch = new toListView(DebugTabs);
    Watch->addColumn(tr("Schema"));
    Watch->addColumn(tr("Object"));
    Watch->addColumn(tr("Variable"));
    Watch->addColumn(tr("Data"));
    Watch->setRootIsDecorated(true);
    Watch->setTreeStepSize(10);
    Watch->setAllColumnsShowFocus(true);
    Watch->setResizeMode(toTreeWidget::AllColumns);
    DebugTabs->addTab(Watch, tr("W&atch"));
    Watch->setSelectionMode(toTreeWidget::Single);
    connect(Watch, SIGNAL(selectionChanged(void)),
            this, SLOT(selectedWatch(void)));
    connect(Watch, SIGNAL(doubleClicked(toTreeWidgetItem *)),
            this, SLOT(changeWatch(toTreeWidgetItem *)));

    Breakpoints = new toListView(DebugTabs);
    Breakpoints->addColumn(tr("Object"));
    Breakpoints->addColumn(tr("Line"));
    Breakpoints->addColumn(tr("Schema"));
    Breakpoints->addColumn(tr("Object Type"));
    Breakpoints->addColumn(tr("Enabled"));
    Breakpoints->setColumnAlignment(1, Qt::AlignRight);
    Breakpoints->setSorting( -1);
    Breakpoints->setAllColumnsShowFocus(true);
    Breakpoints->setResizeMode(toTreeWidget::AllColumns);
    DebugTabs->addTab(Breakpoints, tr("&Breakpoints"));
    connect(Breakpoints, SIGNAL(clicked(toTreeWidgetItem *)),
            this, SLOT(showSource(toTreeWidgetItem *)));

    Parameters = new toListView(DebugTabs);
    Parameters->addColumn(tr("Name"));
    Parameters->addColumn(tr("Content"));
    Parameters->setSorting( -1);
    Parameters->setTreeStepSize(10);
    Parameters->setRootIsDecorated(true);
    Parameters->setAllColumnsShowFocus(true);
    Parameters->setResizeMode(toTreeWidget::AllColumns);
    DebugTabs->addTab(Parameters, tr("&Parameters"));

    Output = new toDebugOutput(this, DebugTabs, connection, debugSession);
    DebugTabs->addTab(Output, tr("Debug &Output"));

    RuntimeLog = new toMarkedText(DebugTabs);
    DebugTabs->addTab(RuntimeLog, tr("&Runtime Log"));

    Editors = new QTabWidget(hsplitter);
    Editors->setTabPosition(QTabWidget::North);

    QToolButton *closeButton = new toPopupButton(Editors);
    closeButton->setIcon(QPixmap(const_cast<const char**>(close_xpm)));
    closeButton->setFixedSize(20, 18);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeEditor()));
    Editors->setCornerWidget(closeButton);

    setFocusProxy(Editors);
    newSheet();

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(windowActivated(QMdiSubWindow *)));

    refresh();
    connect(&StartTimer, SIGNAL(timeout(void)), this, SLOT(startTarget(void)));

    StartTimer.start(1, true);
}


void toDebug::enableDebugger(bool enable)
{
    executeAct->setEnabled(enable);
    stopAct->setEnabled(enable);
    stepAct->setEnabled(enable);
    nextAct->setEnabled(enable);
    returnAct->setEnabled(enable);
    debugPaneAct->setEnabled(enable);
    nextErrorAct->setEnabled(enable);
    previousErrorAct->setEnabled(enable);
    toggleBreakAct->setEnabled(enable);
    disableBreakAct->setEnabled(enable);
    addWatchAct->setEnabled(enable);
    deleteWatchAct->setEnabled(enable);
    changeWatchAct->setEnabled(enable);
}

void toDebug::createActions(void)
{
    refreshAct = new QAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                             tr("Refresh"),
                             this);
    connect(refreshAct,
            SIGNAL(triggered()),
            this,
            SLOT(refresh()),
            Qt::QueuedConnection);
    refreshAct->setShortcut(QKeySequence::Refresh);

    newSheetAct = new QAction(QIcon(QPixmap(const_cast<const char**>(toworksheet_xpm))),
                              tr("&New Sheet"),
                              this);
    connect(newSheetAct,
            SIGNAL(triggered()),
            this,
            SLOT(newSheet()),
            Qt::QueuedConnection);

    scanSourceAct = new QAction(QIcon(QPixmap(const_cast<const char**>(scansource_xpm))),
                                tr("S&can Source"),
                                this);
    connect(scanSourceAct,
            SIGNAL(triggered()),
            this,
            SLOT(scanSource()),
            Qt::QueuedConnection);
    scanSourceAct->setShortcut(Qt::CTRL + Qt::Key_F9);

    compileAct = new QAction(QIcon(QPixmap(const_cast<const char**>(compile_xpm))),
                             tr("&Compile"),
                             this);
    connect(compileAct,
            SIGNAL(triggered()),
            this,
            SLOT(compile()),
            Qt::QueuedConnection);
    compileAct->setShortcut(Qt::Key_F9);

    closeAct = new QAction(QIcon(QPixmap(const_cast<const char**>(close_xpm))),
                           tr("Close"),
                           this);
    connect(closeAct,
            SIGNAL(triggered()),
            this,
            SLOT(closeEditor()),
            Qt::QueuedConnection);

    closeAllAct = new QAction(tr("Close All"),
                              this);
    connect(closeAllAct,
            SIGNAL(triggered()),
            this,
            SLOT(closeAllEditor()),
            Qt::QueuedConnection);

    executeAct = new QAction(QIcon(QPixmap(const_cast<const char**>(execute_xpm))),
                             tr("&Execute or continue"),
                             this);
    connect(executeAct,
            SIGNAL(triggered()),
            this,
            SLOT(execute()),
            Qt::QueuedConnection);
    executeAct->setShortcut(Qt::CTRL + Qt::Key_Return);

    stopAct = new QAction(QIcon(QPixmap(const_cast<const char**>(stop_xpm))),
                          tr("&Stop"),
                          this);
    connect(stopAct,
            SIGNAL(triggered()),
            this,
            SLOT(stop()),
            Qt::QueuedConnection);
    stopAct->setShortcut(Qt::Key_F12);

    stepAct = new QAction(QIcon(QPixmap(const_cast<const char**>(stepinto_xpm))),
                          tr("Step &Into"),
                          this);
    connect(stepAct,
            SIGNAL(triggered()),
            this,
            SLOT(stepInto()),
            Qt::QueuedConnection);
    stepAct->setShortcut(Qt::Key_F7);

    nextAct = new QAction(QIcon(QPixmap(const_cast<const char**>(stepover_xpm))),
                          tr("&Next Line"),
                          this);
    connect(nextAct,
            SIGNAL(triggered()),
            this,
            SLOT(stepOver()),
            Qt::QueuedConnection);
    nextAct->setShortcut(Qt::Key_F8);

    returnAct = new QAction(QIcon(QPixmap(const_cast<const char**>(returnfrom_xpm))),
                            tr("&Return From"),
                            this);
    connect(returnAct,
            SIGNAL(triggered()),
            this,
            SLOT(returnFrom()),
            Qt::QueuedConnection);
    returnAct->setShortcut(Qt::Key_F6);

    debugPaneAct = new QAction(tr("&Debug Pane"), this);
    debugPaneAct->setIcon(QIcon(QPixmap(const_cast<const char**>(todebug_xpm))));
    debugPaneAct->setCheckable(true);
    connect(debugPaneAct,
            SIGNAL(toggled(bool)),
            this,
            SLOT(showDebug(bool)),
            Qt::QueuedConnection);
    debugPaneAct->setShortcut(Qt::Key_F11);

    nextErrorAct = new QAction(QIcon(QPixmap(const_cast<const char**>(nextbug_xpm))),
                               tr("Next &Error"),
                               this);
    connect(nextErrorAct,
            SIGNAL(triggered()),
            this,
            SLOT(nextError()),
            Qt::QueuedConnection);
    nextErrorAct->setShortcut(Qt::CTRL + Qt::Key_N);

    previousErrorAct = new QAction(QIcon(QPixmap(const_cast<const char**>(prevbug_xpm))),
                                   tr("Pre&vious Error"),
                                   this);
    connect(previousErrorAct,
            SIGNAL(triggered()),
            this,
            SLOT(prevError()),
            Qt::QueuedConnection);
    previousErrorAct->setShortcut(Qt::CTRL + Qt::Key_P);

    toggleBreakAct = new QAction(QIcon(QPixmap(const_cast<const char**>(togglebreak_xpm))),
                                 tr("&Toggle Breakpoint"),
                                 this);
    connect(toggleBreakAct,
            SIGNAL(triggered()),
            this,
            SLOT(toggleBreak()),
            Qt::QueuedConnection);
    toggleBreakAct->setShortcut(Qt::CTRL + Qt::Key_F5);

    disableBreakAct = new QAction(QIcon(QPixmap(const_cast<const char**>(enablebreak_xpm))),
                                  tr("D&isable Breakpoint"),
                                  this);
    connect(disableBreakAct,
            SIGNAL(triggered()),
            this,
            SLOT(toggleEnable()),
            Qt::QueuedConnection);
    disableBreakAct->setShortcut(Qt::CTRL + Qt::Key_F6);

    addWatchAct = new QAction(QIcon(QPixmap(const_cast<const char**>(addwatch_xpm))),
                              tr("&Add Watch..."),
                              this);
    connect(addWatchAct,
            SIGNAL(triggered()),
            this,
            SLOT(addWatch()),
            Qt::QueuedConnection);
    addWatchAct->setShortcut(Qt::Key_F4);

    deleteWatchAct = new QAction(QIcon(QPixmap(const_cast<const char**>(delwatch_xpm))),
                                 tr("Delete &Watch"),
                                 this);
    deleteWatchAct->setEnabled(false);
    connect(deleteWatchAct,
            SIGNAL(triggered()),
            this,
            SLOT(deleteWatch()),
            Qt::QueuedConnection);
    deleteWatchAct->setShortcut(Qt::CTRL + Qt::Key_Delete);

    changeWatchAct = new QAction(QIcon(QPixmap(const_cast<const char**>(changewatch_xpm))),
                                 tr("Chan&ge Watch..."),
                                 this);
    changeWatchAct->setEnabled(false);
    connect(changeWatchAct,
            SIGNAL(triggered()),
            this,
            SLOT(changeWatch()),
            Qt::QueuedConnection);
    changeWatchAct->setShortcut(Qt::CTRL + Qt::Key_F4);

    eraseLogAct = new QAction(tr("Erase Runtime &Log"), this);
    connect(eraseLogAct,
            SIGNAL(triggered()),
            this,
            SLOT(clearLog()),
            Qt::QueuedConnection);
}


static toSQL SQLAttach("toDebug:Attach",
                       "DECLARE\n"
                       "    timeout BINARY_INTEGER;\n"
                       "BEGIN\n"
                       "    SYS.DBMS_DEBUG.ATTACH_SESSION(:sess<char[201],in>);\n"
                       "END;",
                       "Connect to the debugging session");

void toDebug::startTarget(void)
{
#ifdef DEBUG
    qDebug() << "toDebug::startTarget";
#endif
    try
    {
        toLocker lock (Lock);
        TargetThread = new toThread(new targetTask(*this));
        TargetThread->start();
    }
    catch (...)
    {
        toStatusMessage(tr("Failed to start target task thread, close some other tools and try again"));
        return ;
    }

    ChildSemaphore.down(); // sleep until target session is initialised (or fails doing so) in other
    // thread (toDebug::targetTask::run must raise this semapthore)
    if (!DebuggerStarted)
    {
        {
//             toLocker lock (Lock);
            TOMessageBox::critical(this, tr("Couldn't start debugging"),
                                   tr("Couldn't connect to target session:\n") +
                                   TargetLog,
                                   tr("&Ok"));
        }
//         close();
        return ;
    }
    try
    {
#ifdef DEBUG
        qDebug() << "toDebug::startTarget DBMS_DEBUG.ATTACH_SESSION " << TargetID;
#endif
        if (DebuggerStarted)
            debugSession->execute(SQLAttach, TargetID);
    }
    TOCATCH  // Trying to run somthing after this won't work (And will hang tora I think)
    readLog();
}

toDebugText *toDebug::currentEditor(void)
{
    return dynamic_cast<toDebugText *>(Editors->currentWidget());
}

void toDebug::changeSchema(int)
{
    refresh();
}

static toSQL SQLListObjects("toDebug:ListObjects",
                            "SELECT Object_Type,Object_Name Type FROM SYS.ALL_OBJECTS\n"
                            " WHERE OWNER = :owner<char[101]>\n"
                            "   AND Object_Type IN ('FUNCTION','PACKAGE',\n"
                            "                       'PROCEDURE','TYPE')\n"
                            " ORDER BY Object_Type,Object_Name",
                            "List objects available in a schema, must have same result columns");

void toDebug::refresh(void)
{
//     qDebug() << "toDebug::refresh 1";
    try
    {
        QString selected = Schema->currentText();
        QString currentSchema;
        if (selected.isEmpty())
        {
            selected = connection().user().toUpper();
            Schema->clear();
            toQList users = toQuery::readQuery(connection(),
                                               toSQL::string(toSQL::TOSQL_USERLIST, connection()));
            for (toQList::iterator i = users.begin(); i != users.end(); i++)
                Schema->addItem(*i);
        }
        if (!selected.isEmpty())
        {
            for (int i = 0; i < Schema->count(); i++)
            {
                if (Schema->itemText(i) == selected)
                {
                    Schema->setCurrentIndex(i);
                    break;
                }
            }

            CodeModel->refresh(connection(), selected);
        }
    }
    TOCATCH
//     qDebug() << "toDebug::refresh 2";
}

bool toDebug::checkStop(void)
{
//     qDebug() << "toDebug::checkStop 1";
    Lock.lock();
    if (RunningTarget)
    {
        Lock.unlock();
        if (TOMessageBox::information(this, tr("Stop execution?"),
                                      tr("Do you want to abort the current execution?"),
                                      tr("&Ok"), tr("Cancel")) != 0)
        {
//             return false;
            stop();
            return false;
        }
    }
    else
        Lock.unlock();
//     qDebug() << "toDebug::checkStop 2";
    return true;
}

bool toDebug::checkCompile(toDebugText *editor)
{
//     qDebug() << "toDebug::checkCompile 1";
    if (editor->isModified())
    {
        switch (TOMessageBox::warning(this,
                                      tr("%1 changed").arg(editorName(editor)),
                                      tr("%1 changed. Continuing will discard uncompiled or saved changes").arg(editorName(editor)),
                                      tr("&Compile"),
                                      tr("&Discard changes"),
                                      tr("Cancel")))
        {
        case 0:
            if (!checkStop())
                return false;
            if (!editor->compile())
                return false;
            break;
        case 1:
            editor->setModified(false);
            break;
        case 2:
            return false;
        }
    }
//     qDebug() << "toDebug::checkCompile 2";
    return true;
}

bool toDebug::checkCompile(void)
{
//     qDebug() << "toDebug::checkCompile void 1";
    for (int i = 0; i < Editors->count(); i++)
    {
        toDebugText *editor = dynamic_cast<toDebugText *>(Editors->widget(i));
        if (!checkCompile(editor))
            return false;
    }
//     qDebug() << "toDebug::checkCompile void 2";
    return true;
}

bool toDebug::close()
{
//     qDebug() << "toDebug::close 1";
    if (checkCompile())
    {
        bool ret = toToolWidget::close();
        if (ret && Output)
            Output->close();
        return ret;
    }
//     qDebug() << "toDebug::close 2";
    return false;
}

void toDebug::closeEvent(QCloseEvent *e)
{
//     qDebug() << "toDebug::closeEvent 1";
    if (close())
    {
        try
        {
            Lock.lock();
            if (DebuggerStarted)
            {
                Lock.unlock();
                stop();
                {
                    toLocker lock (Lock);
                    TargetSQL = "";
                    TargetSemaphore.up();
                }
                ChildSemaphore.down();
            }
            else
                Lock.unlock();
        }
        TOCATCH;

        try
        {
            DebugTool.closeWindow(connection());
        }
        TOCATCH;
// qDebug() << "toDebug::closeEvent 2";
        e->accept();
    }
    else
        e->ignore();
// qDebug() << "toDebug::closeEvent 3";
}

void toDebug::updateCurrent()
{
//     qDebug() << "toDebug::updateCurrent 1";
    try
    {
        toDebugText *editor = currentEditor();

        editor->readData(connection(), StackTrace);
        editor->setFocus();
        updateContent();
    }
    TOCATCH
//     qDebug() << "toDebug::updateCurrent 2";
}

void toDebug::changePackage(const QModelIndex &current, const QModelIndex &previous)
{
//     qDebug() << "toDebug::changePackage 1";
    toCodeModelItem *item = static_cast<toCodeModelItem*>(current.internalPointer());
    if (item && item->parent())
    {
        QString ctype = item->parent()->display();
        if (ctype.isEmpty() || ctype == "Code")
            return;
        ctype = ctype.toUpper();

        viewSource(Schema->currentText(), item->display(), ctype, 0);
        if (ctype == "PACKAGE" || ctype == "TYPE")
            viewSource(Schema->currentText(), item->display(), ctype + " BODY", 0);
    }
#ifdef AUTOEXPAND
    else if (item && !item->parent())
        item->setOpen(true);
#endif
// qDebug() << "toDebug::changePackage 2";
}

void toDebug::showDebug(bool show)
{
//     qDebug() << "toDebug::showDebug 1";
    if (show)
        DebugTabs->show();
    else
        DebugTabs->hide();

    debugPaneAct->setChecked(show);
//     qDebug() << "toDebug::showDebug 2";
}

bool toDebugText::compile(void)
{
#ifdef DEBUG
    qDebug() << "toDebugText::compile start 1";
#endif
    QString str = text();
    bool ret = true;
    if (!str.isEmpty())
    {
        bool body = false;

        toSQLParse::stringTokenizer tokens(str);

        QString token = tokens.getToken();
        if (token.toUpper() == "CREATE")
        {
            token = tokens.getToken();
            if (token.toUpper() == "OR")
            {
                token = tokens.getToken();
                if (token.toUpper() == "REPLACE")
                    token = tokens.getToken();
            }
        }

        QString type = token.toUpper();
        if (type != QString::fromLatin1("PROCEDURE") &&
                type != QString::fromLatin1("TYPE") &&
                type != QString::fromLatin1("FUNCTION") &&
                type != QString::fromLatin1("PACKAGE"))
        {
            toStatusMessage(tr("Invalid start of code"));
            return false;
        }

        token = tokens.getToken();
        if (token.toUpper() == "BODY")
        {
            body = true;
            token = tokens.getToken();
        }
        QString object = token;
        QString schema = Schema;

        int offset = tokens.offset();
        token = tokens.getToken();
        if (token == ".")
        {
            schema = object;
            object = tokens.getToken();
            offset = tokens.offset();
        }

        QString sql = QString::fromLatin1("CREATE OR REPLACE ");
        sql.append(type);
        if (body)
            sql.append(QString::fromLatin1(" BODY "));
        else
            sql.append(QString::fromLatin1(" "));
        sql.append(schema);
        sql.append(QString::fromLatin1("."));
        sql.append(object);
        sql.append(QString::fromLatin1(" "));
        sql.append(str.mid(offset));

        try
        {
            toQList nopar;
            Debugger->executeInTarget(sql, nopar);
            Schema = schema.toUpper();
            Object = Debugger->connection().unQuote(object.toUpper());
            Type = type.toUpper();
            if (body)
                Type += QString::fromLatin1(" BODY");
            readErrors(Debugger->connection());
            setModified(false);
            toConnection::objectName no;
            no.Name = Object;
            no.Owner = Schema;
            no.Type = type;
            Debugger->connection().addIfNotExists(no);
        }
        catch (const QString &exc)
        {
            toStatusMessage(exc);
            ret = false;
        }
    }
#ifdef DEBUG
    qDebug() << "toDebugText::compile end 1";
#endif
    return ret;
}

void toDebug::compile(void)
{
#ifdef DEBUG
    qDebug() << "toDebug::compile start 2";
#endif
    if (!checkStop())
        return ;

    QString lastSchema = currentEditor()->schema();
    for (int i = 0; i < Editors->count(); i++)
    {
        toDebugText *editor = dynamic_cast<toDebugText *>(Editors->widget(i));
        int row, col;
        editor->getCursorPosition(&row, &col);
        if (editor->compile())
        {
            if (editor == currentEditor() &&
                    lastSchema != currentEditor()->schema())
            {
                for (int i = 0; i < Schema->count(); i++)
                    if (Schema->itemText(i) == lastSchema)
                    {
                        Schema->setCurrentIndex(i);
                        break;
                    }
            }
            if (editor->hasErrors())
                Editors->setTabIcon(Editors->indexOf(editor),
                                    QIcon(QPixmap(const_cast<const char**>(nextbug_xpm))));
            else
                Editors->setTabIcon(Editors->indexOf(editor), QIcon());
            Editors->setTabText(Editors->indexOf(editor), editorName(editor));
            editor->setCursorPosition(row, col);
        }
        else
            return ;
    }
    refresh();
    scanSource();
#ifdef DEBUG
    qDebug() << "toDebug::compile end 2";
#endif
}

toDebug::~toDebug()
{
}

void toDebug::prevError(void)
{
    currentEditor()->previousError();
}

void toDebug::nextError(void)
{
    currentEditor()->nextError();
}

void toDebug::changeContent(toTreeWidgetItem *ci)
{
//     qDebug() << "toDebug::changeContent 1";
    toContentsItem *item = dynamic_cast<toContentsItem *>(ci);
    if (item)
    {
        while (ci->parent())
            ci = ci->parent();
        toHighlightedText *current = NULL;

        for (int i = 0; i < Editors->count(); i++)
        {
            if (Editors->widget(i)->objectName() == ci->text(1))
            {
                current = dynamic_cast<toDebugText *>(Editors->widget(i));
                break;
            }
        }
        if (current)
        {
            current->setCursorPosition(item->Line, 0);
            Editors->setCurrentIndex(Editors->indexOf(current));
            current->setFocus();
        }
    }
#ifdef AUTOEXPAND
    else
        ci->setOpen(true);
#endif
// qDebug() << "toDebug::changeContent 2";
}

void toDebug::scanSource(void)
{
//     qDebug() << "toDebug::scanSource 1";
    updateContent();
//     qDebug() << "toDebug::scanSource 2";
}

void toDebug::newSheet(void)
{
    toDebugText *text = new toDebugText(Breakpoints, Editors, this);
    // signal removed with qscintilla port.
//     connect(text, SIGNAL(insertedLines(int, int)),
//             this, SLOT(reorderContent(int, int)));
    if (!Schema->currentText().isEmpty())
        text->setSchema(Schema->currentText());
    else
        text->setSchema(connection().user().toUpper());
    Editors->addTab(text, tr("Unknown"));
    Editors->setCurrentIndex(Editors->indexOf(text));
}

void toDebug::showSource(toTreeWidgetItem *item)
{
    if (item)
        viewSource(item->text(2), item->text(0), item->text(3), item->text(1).toInt(), false);
}

void toDebug::toggleBreak(void)
{
    currentEditor()->toggleBreakpoint();
    currentEditor()->setFocus();
}

void toDebug::toggleEnable(void)
{
    currentEditor()->toggleBreakpoint( -1, true);
    currentEditor()->setFocus();
}

void toDebug::addWatch(void)
{
    toDebugWatch watch(this);
    if (watch.exec())
    {
        watch.createWatch(Watch);
        if (isRunning())
            updateState(TO_REASON_WHATEVER);
    }
}

void toDebug::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;
    if (widget->widget() == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Debug"), this);

            ToolMenu->addAction(newSheetAct);
            ToolMenu->addAction(scanSourceAct);
            ToolMenu->addAction(compileAct);
            ToolMenu->addAction(closeAct);
            ToolMenu->addAction(closeAllAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(executeAct);
            ToolMenu->addAction(stopAct);
            ToolMenu->addAction(stepAct);
            ToolMenu->addAction(nextAct);
            ToolMenu->addAction(returnAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(nextErrorAct);
            ToolMenu->addAction(previousErrorAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(toggleBreakAct);
            ToolMenu->addAction(disableBreakAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(addWatchAct);
            ToolMenu->addAction(deleteWatchAct);
            ToolMenu->addAction(changeWatchAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(refreshAct);
            ToolMenu->addAction(eraseLogAct);

            toMainWidget()->addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toDebug::selectedWatch()
{
    toTreeWidgetItem *item = Watch->selectedItem();
    if (item)
    {
        if (!item->text(5).isEmpty() &&
                item->text(5) != QString::fromLatin1("LIST") &&
                item->text(5) != QString::fromLatin1("NULL"))
        {
            deleteWatchAct->setEnabled(false);
        }
        else
            deleteWatchAct->setEnabled(true);

        changeWatchAct->setEnabled(item->text(4).isEmpty());
    }
    else
    {
        deleteWatchAct->setEnabled(false);
        changeWatchAct->setEnabled(false);
    }
}

void toDebug::deleteWatch(void)
{
    delete Watch->selectedItem();
}

void toDebug::clearLog(void)
{
    RuntimeLog->clear();
}

void toDebug::changeWatch(void)
{
    changeWatch(Watch->selectedItem());
}

static toSQL SQLChangeLocal("toDebug:ChangeLocalWatch",
                            "DECLARE\n"
                            "  ret BINARY_INTEGER;\n"
                            "  data VARCHAR2(4000);\n"
                            "BEGIN\n"
                            "  ret:=SYS.DBMS_DEBUG.SET_VALUE(0,:assign<char[4001],in>);\n"
                            "  SELECT ret INTO :ret<int,out> FROM sys.DUAL;\n"
                            "END;",
                            "Change local watch value, must have same bindings");
static toSQL SQLChangeGlobal("toDebug:ChangeGlobalWatch",
                             "DECLARE\n"
                             "  data VARCHAR2(4000);\n"
                             "  proginf SYS.DBMS_DEBUG.program_info;\n"
                             "  ret BINARY_INTEGER;\n"
                             "BEGIN\n"
                             "  proginf.Namespace:=SYS.DBMS_DEBUG.Namespace_pkg_body;\n"
                             "  proginf.Name:=:object<char[101],in>;\n"
                             "  proginf.Owner:=:owner<char[101],in>;\n"
                             "  proginf.DBLink:=NULL;\n"
                             "  ret:=SYS.DBMS_DEBUG.SET_VALUE(proginf,:assign<char[4001],in>);\n"
                             "  IF ret =SYS.DBMS_DEBUG.error_no_such_object THEN\n"
                             "    proginf.Namespace:=SYS.DBMS_DEBUG.namespace_pkgspec_or_toplevel;\n"
                             "    ret:=SYS.DBMS_DEBUG.SET_VALUE(proginf,:assign<char[4001],in>);\n"
                             "  END IF;\n"
                             "  SELECT ret INTO :ret<int,out> FROM sys.DUAL;\n"
                             "END;",
                             "Change global watch value, must have same bindings");

void toDebug::changeWatch(toTreeWidgetItem *item)
{
    if (item && item->text(4).isEmpty())
    {
        QString description = tr("Enter new value to the watch %1").arg(item->text(2));
        QString data;

        QDialog qdialog;
        Ui::toDebugChangeUI dialog; //, "WatchChange", true);
        dialog.setupUi(&qdialog);
        toHelp::connectDialog(&qdialog);

        dialog.HeadLabel->setText(description);
        QString index = item->text(5);

        if (item->text(5) == QString::fromLatin1("NULL"))
            dialog.NullValue->setChecked(true);
        else
            data = item->text(3);

        if (!index.isEmpty() && index != QString::fromLatin1("LIST"))
            dialog.Index->setValue(item->text(5).toInt());
        if (index != QString::fromLatin1("LIST"))
        {
            dialog.Index->setEnabled(false);
            dialog.Value->setText(data);
        }

        if (qdialog.exec())
        {
            int ret = -1;
            QString escdata;
            QString assign;
            if (dialog.NullValue->isChecked())
            {
                escdata = QString::fromLatin1("NULL");
            }
            else
            {
                escdata = data = dialog.Value->text();
                escdata.replace(QRegExp(QString::fromLatin1("'")), QString::fromLatin1("''"));
                escdata.prepend(QString::fromLatin1("'"));
                escdata += QString::fromLatin1("'");
            }
            assign = item->text(2);
            if (index == QString::fromLatin1("LIST"))
            {
                assign += QString::fromLatin1("(");
                assign += dialog.Index->text();
                assign += QString::fromLatin1(")");
            }

            assign += QString::fromLatin1(":=");
            assign += escdata;
            assign += QString::fromLatin1(";");
            try
            {
                if (item->text(0).isEmpty())
                {
                    debugSession->execute(SQLChangeLocal, assign);
                    ret = debugSession->readValue().toInt();
                }
                else
                {
                    QString tmp = item->text(1);
                    if (tmp.isEmpty())
                        tmp = "";
                    debugSession->execute(SQLChangeGlobal, tmp, item->text(0), assign);
                    ret = debugSession->readValue().toInt();
                }
                if (ret == TO_ERROR_UNIMPLEMENTED)
                {
                    toStatusMessage(tr("Unimplemented in PL/SQL debug interface"));
                }
                else if (ret != TO_SUCCESS)
                {
                    toStatusMessage(tr("Assignment failed (Reason %1)").arg(ret));
                }
                else
                    updateState(TO_REASON_WHATEVER);
            }
            TOCATCH
        }
    }
}

void toDebug::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    data[prefix + ":Editors"] = Editors->count();
    for (int i = 0; i < Editors->count(); i++)
    {
        toHighlightedText *editor = dynamic_cast<toHighlightedText *>(Editors->widget(i));
        QString num;
        num.setNum(i);
        editor->exportData(data, prefix + ":Editor:" + num);
    }
    data[prefix + ":Schema"] = Schema->currentText();

    int id = 1;
    for (toTreeWidgetItem *item = Breakpoints->firstChild(); item; item = item->nextSibling())
    {
        toBreakpointItem * point = dynamic_cast<toBreakpointItem *>(item);

        if (point)
        {
            QString key = prefix + ":Breaks:" + QString::number(id).toLatin1();

            data[key + ":Schema"] = point->text(2);
            data[key + ":Object"] = point->text(0);
            data[key + ":Type"] = point->text(3);
            data[key + ":Line"] = QString::number(point->line());
            if (point->text(4) == tr("DISABLED"))
                data[key + ":Status"] = "DISABLED";
        }

        id++;
    }
    id = 1;
    for (toTreeWidgetItem *qitem = Watch->firstChild(); qitem; qitem = qitem->nextSibling())
    {
        toResultViewItem * item = dynamic_cast<toResultViewItem *>(qitem);
        if (item)
        {
            QString key = prefix + ":Watch:" + QString::number(id).toLatin1();
            data[key + ":Schema"] = item->allText(0);
            data[key + ":Object"] = item->allText(1);
            data[key + ":Item"] = item->allText(2);
            data[key + ":Auto"] = item->allText(6);
        }
        id++;
    }
    if (debugPaneAct->isChecked())
        data[prefix + ":Debug"] = QString::fromLatin1("Show");

    toToolWidget::exportData(data, prefix);
}

void toDebug::importData(std::map<QString, QString> &data, const QString &prefix)
{
    QString str = data[prefix + ":Schema"];
    {
        for (int i = 0; i < Schema->count(); i++)
            if (Schema->itemText(i) == str)
            {
                Schema->setCurrentIndex(i);
                changeSchema(i);
                break;
            }
    }

    int count = data[prefix + ":Editors"].toInt();
    for (int j = 0; j < count; j++)
    {
        toDebugText *text = new toDebugText(Breakpoints, Editors, this);
        // signal removed with qscintilla port.
//         connect(text, SIGNAL(insertedLines(int, int)),
//                 this, SLOT(reorderContent(int, int)));
        QString num;
        num.setNum(j);
        text->importData(data, prefix + ":Editor:" + num);
        Editors->addTab(text, editorName(text));
    }

    int id = 1;
    std::map<QString, QString>::iterator i;
    toBreakpointItem *debug = NULL;
    while ((i = data.find(prefix + ":Breaks:" + QString::number(id).toLatin1() + ":Line")) != data.end())
    {
        QString key = prefix + ":Breaks:" + QString::number(id).toLatin1();
        int line = (*i).second.toInt();
        debug = new toBreakpointItem(Breakpoints, debug,
                                     data[key + ":Schema"],
                                     data[key + ":Type"],
                                     data[key + ":Object"],
                                     line);
        if (data[key + ":Status"] == QString::fromLatin1("DISABLED"))
            debug->setText(4, tr("DISABLED"));
        id++;
    }
    id = 1;
    toResultViewItem *item = NULL;
    while ((i = data.find(prefix + ":Watch:" + QString::number(id).toLatin1() + ":Item")) != data.end())
    {
        QString key = prefix + ":Watch:" + QString::number(id).toLatin1();
        item = new toResultViewItem(Watch, NULL, data[key + ":Schema"]);
        item->setText(1, data[key + ":Object"]);
        item->setText(2, data[key + ":Item"]);
        item->setText(4, QString::fromLatin1("NOCHANGE"));
        if (!data[key + ":Auto"].isEmpty())
            item->setText(6, "AUTO");
        id++;
    }
    scanSource();

    debugPaneAct->setChecked(data[prefix + ":Debug"] == QString::fromLatin1("Show"));

    toToolWidget::importData(data, prefix);
}

void toDebug::closeEditor()
{
    toDebugText *editor = currentEditor();
    closeEditor(editor);
}

void toDebug::closeAllEditor()
{
    int editorCount = Editors->count();
    while (editorCount > 0)
    {
        editorCount--;
        toDebugText *editor = dynamic_cast<toDebugText *>(Editors->widget(editorCount));
        if (editor)
            closeEditor(editor);
    }
}


void toDebug::closeEditor(toDebugText* &editor)
{

    if (editor && checkCompile(editor))
    {
        QString name = editor->objectName();
        for (toTreeWidgetItem *item = Contents->firstChild(); item; item = item->nextSibling())
        {
            if (item->text(1) == name)
            {
                delete item;
                break;
            }
        }

//         if (Objects->selectedItem() &&
//                 Objects->selectedItem()->text(0) == editor->object() &&
//                 Schema->currentText() == editor->schema())
//             Objects->clearSelection();

        Editors->removeTab(Editors->indexOf(editor));
        delete editor;
        if (Editors->count() == 0)
            newSheet();
    }
}
