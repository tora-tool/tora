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

#include "toconf.h"
#include "todebug.h"
#include "todebugchangeui.h"
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

#ifdef TO_KDE
#include <kmenubar.h>
#endif

#include <stack>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qsizepolicy.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qworkspace.h>

#include "todebug.moc"
#include "todebugchangeui.moc"
#include "todebugwatch.moc"

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

#define TO_ID_NEW_SHEET  (toMain::TO_TOOL_MENU_ID+ 0)
#define TO_ID_SCAN_SOURCE (toMain::TO_TOOL_MENU_ID+ 1)
#define TO_ID_COMPILE  (toMain::TO_TOOL_MENU_ID+ 2)
#define TO_ID_EXECUTE  (toMain::TO_TOOL_MENU_ID+ 3)
#define TO_ID_STOP  (toMain::TO_TOOL_MENU_ID+ 4)
#define TO_ID_STEP_INTO  (toMain::TO_TOOL_MENU_ID+ 5)
#define TO_ID_STEP_OVER  (toMain::TO_TOOL_MENU_ID+ 6)
#define TO_ID_RETURN_FROM (toMain::TO_TOOL_MENU_ID+ 7)
#define TO_ID_HEAD_TOGGLE (toMain::TO_TOOL_MENU_ID+ 8)
#define TO_ID_DEBUG_PANE (toMain::TO_TOOL_MENU_ID+ 9)
#define TO_ID_DEL_WATCH  (toMain::TO_TOOL_MENU_ID+10)
#define TO_ID_CHANGE_WATCH (toMain::TO_TOOL_MENU_ID+11)
#define TO_ID_CLOSE_EDITOR (toMain::TO_TOOL_MENU_ID+12)
#define TO_ID_CLOSE_ALL_EDITOR      (toMain::TO_TOOL_MENU_ID+14)

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
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
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
    }
    void closeWindow(toConnection &connection)
    {
        std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
        if (i != Windows.end())
            Windows.erase(i);
    }
    virtual bool canHandle(toConnection &conn)
    {
        if (!toIsOracle(conn))
            return false;
#if 1

        if (conn.version() < "8.0")
            return false;
#endif

        return true;
    }
};

static toDebugTool DebugTool;

toDebugWatch::toDebugWatch(toDebug *parent)
        : toDebugWatchUI(parent, "AddWatch", true), Debugger(parent)
{
    toHelp::connectDialog(this);
    {
        int curline, curcol;
        Debugger->currentEditor()->getCursorPosition (&curline, &curcol);
        Default = Debugger->currentEditor()->textLine(curline);
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

    connect(Scope, SIGNAL(clicked(int)), this, SLOT(changeScope(int)));
    changeScope(1);
}

void toDebugWatch::changeScope(int num)
{
    switch (num)
    {
    default:
        Name->clear();
        Name->insertItem(Default);
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
            Name->insertItem(str);
        }
        break;
    }
}

QListViewItem *toDebugWatch::createWatch(QListView *watches)
{
    QString str;
    switch (Scope->id(Scope->selected()))
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
            item->setText(6, Scope->id(Scope->selected()) == 5 ? "AUTO" : "");
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
    int pos = str.find(QString::fromLatin1("."));
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
    pos = str.find(QString::fromLatin1("."));
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
}

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
                                "  SELECT ret,line INTO :ret<int,out>,:line<char[101,out> FROM sys.DUAL;\n"
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

class toDebugOutput : public toOutput
{
    toDebug *Debugger;
public:
    toDebugOutput(toDebug *debug, QWidget *parent, toConnection &conn)
            : toOutput(parent, conn), Debugger(debug)
    {}
    virtual void refresh(void)
    {
        if (Debugger->isRunning() && enabled())
        {
            try
            {
                int ret = -1;
                do
                {
                    toQuery poll(connection(), SQLDebugOutputPoll);
                    ret = poll.readValue().toInt();
                    QString str = poll.readValueNull();
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
        if (Debugger->isRunning())
        {
            try
            {
                if (dis)
                    connection().execute(SQLDebugOutputDisable);
                else
                    connection().execute(SQLDebugOutputEnable);
            }
            catch (...)
            {
                toStatusMessage(qApp->translate("toDebugOutput", "Couldn't enable/disable output for session"));
            }
        }
    }
};

bool toDebug::isRunning(void)
{
    toLocker lock (Lock)
        ;
    return RunningTarget;
}

static toSQL SQLDebugEnable("toDebug:EnableDebug",
                            "ALTER SESSION SET PLSQL_DEBUG = TRUE",
                            "Enable PL/SQL debugging");

static toSQL SQLDebugInit("toDebug:Initialize",
                          "DECLARE\n"
                          "  ret VARCHAR2(200);\n"
                          "BEGIN\n"
                          "  \n"
                          "  ret:=SYS.DBMS_DEBUG.INITIALIZE;\n"
                          "  SYS.DBMS_DEBUG.DEBUG_ON;\n"
                          "  SELECT ret INTO :f2<char[201],out> FROM sys.DUAL;\n"
                          "END;",
                          "Initialize the debug session, must have same bindings");

void toDebug::targetTask::run(void)
{
    try
    {
        toConnection Connection(Parent.connection());
        try
        {
            Connection.execute(SQLDebugEnable);
        }
        catch (...)
        {
            toLocker lock (Parent.Lock)
                ;
            Parent.TargetLog += QString::fromLatin1("Couldn't enable debugging for target session\n");
        }
        try
        {
            toQuery init(Connection, SQLDebugInit);

            Parent.DebuggerStarted = true;
            toLocker lock (Parent.Lock)
                ;
            Parent.TargetID = init.readValue();
            Parent.ChildSemaphore.up();
            Parent.TargetLog += QString::fromLatin1("Debug session connected\n");
        }
        catch (const QString &exc)
        {
            toLocker lock (Parent.Lock)
                ;
            Parent.TargetLog += QString::fromLatin1("Couldn't start debugging:");
            Parent.TargetLog += exc;
            Parent.DebuggerStarted = false;
            Parent.ChildSemaphore.up();
            return ;
        }
        int colSize;
        while (1)
        {
            {
                toLocker lock (Parent.Lock)
                    ;
                Parent.RunningTarget = false;
                colSize = Parent.ColumnSize;
            }
            Parent.TargetSemaphore.down();

            QString sql;
            toQList inParams;
            toQList outParams;
            {
                toLocker lock (Parent.Lock)
                    ;
                Parent.RunningTarget = true;
                sql = Parent.TargetSQL;
                Parent.TargetSQL = "";
                inParams = Parent.InputData;
                Parent.InputData.clear(); // To make sure data is not shared
                Parent.OutputData.clear();
            }
            Parent.StartedSemaphore.up();
            if (sql.isEmpty())
                break;

            try
            {
                Parent.Lock.lock();
                Parent.TargetLog += QString::fromLatin1("Executing SQL\n");
                Parent.Lock.unlock();

                outParams = toQuery::readQuery(Connection, sql, inParams);
            }
            catch (const QString &str)
            {
                Parent.Lock.lock();
                Parent.TargetLog += QString::fromLatin1("Encountered error: ");
                Parent.TargetLog += str;
                if (!str.startsWith("ORA-06543:"))
                    Parent.TargetException += str;
                Parent.TargetLog += QString::fromLatin1("\n");
                Parent.Lock.unlock();
            }
            catch (...)
            {
                Parent.Lock.lock();
                Parent.TargetLog += QString::fromLatin1("Encountered unknown exception\n");
                Parent.TargetException += QString::fromLatin1("Encountered unknown exception\n");
                Parent.Lock.unlock();
            }

            {
                toLocker lock (Parent.Lock)
                    ;
                Parent.OutputData = outParams;
                Parent.TargetLog += QString::fromLatin1("Execution ended\n");
            }
            Parent.ChildSemaphore.up();
        }

    }
    TOCATCH
    toLocker lock (Parent.Lock)
        ;
    Parent.DebuggerStarted = false;
    Parent.TargetLog += QString::fromLatin1("Closing debug session\n");
    Parent.TargetThread = NULL;
    Parent.ChildSemaphore.up();
}

static QListViewItem *toLastItem(QListViewItem *parent)
{
    QListViewItem *lastItem = NULL;
    for (QListViewItem *item = parent->firstChild();item;item = item->nextSibling())
        lastItem = item;
    return lastItem;
}

class toContentsItem : public QListViewItem
{
public:
    int Line;
    toContentsItem(QListViewItem *parent, const QString &name, int line)
            : QListViewItem(parent, toLastItem(parent), name)
    {
        Line = line;
    }
    toContentsItem(QListView *parent, const QString &name, const QString &id, int line)
            : QListViewItem(parent, name, id)
    {
        Line = line;
    }
};

void toDebug::reorderContent(QListViewItem *parent, int start, int diff)
{
    QListViewItem *next;
    for (QListViewItem *item = parent->firstChild();item;item = next)
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

QListViewItem *toDebug::contents(void)
{
    return Contents->firstChild();
}

void toDebug::reorderContent(int start, int diff)
{
    QString name = currentEditor()->name();
    for (QListViewItem *item = Contents->firstChild();item;item = item->nextSibling())
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

void toDebug::execute(void)
{
    if (RunningTarget)
    {
        continueExecution(TO_BREAK_CONTINUE);
        return ;
    }

    if (!checkCompile())
        return ;

    QString curName = currentEditor()->name();
    toHighlightedText *current = currentEditor();
    int curline, curcol;
    current->getCursorPosition (&curline, &curcol);

    bool valid = false;
    int line = -1;
    if (hasMembers(currentEditor()->type()))
    {
        for (QListViewItem *parent = Contents->firstChild();parent;parent = parent->nextSibling())
        {
            printf("%s\n", (const char *)parent->text(1));
            if (parent->text(1) == curName)
            {
                for (parent = parent->firstChild();parent;parent = parent->nextSibling())
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

    if (valid)
    {
        try
        {
            QString token;
            int level = 0;

            enum {
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
            while (token.upper() == QString::fromLatin1("CREATE") || token.upper() == QString::fromLatin1("OR") ||
                    token.upper() == QString::fromLatin1("REPLACE"));

            if (token.upper() != QString::fromLatin1("FUNCTION") && token.upper() != QString::fromLatin1("PROCEDURE"))
            {
                toStatusMessage(tr("Expected function or procedure, internal error"));
                return ;
            }
            do
            {
                token = tokens.getToken();
                if (token.isEmpty())
                {
                    toStatusMessage(tr("Unexpected end of declaration."));
                    return ;
                }
                if (state == returnType)
                {
                    if (retType.isEmpty() || retType.at(retType.length() - 1) == '.' || token == QString::fromLatin1("."))
                        retType += token;
                    else
                        state = done;
                }
                else if (token.upper() == QString::fromLatin1("RETURN") && level == 0)
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
                        if (token.upper() == QString::fromLatin1("IN"))
                        {
                            (*cp).In = true;
                            break;
                        }
                        else if (token.upper() == QString::fromLatin1("OUT"))
                        {
                            (*cp).Out = true;
                            break;
                        }
                        else if (token.upper() == QString::fromLatin1("NOCOPY"))
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
                        else if (token.upper() == QString::fromLatin1("DEFAULT") || token == QString::fromLatin1(":="))
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
                            if (token.upper() == QString::fromLatin1("NULL"))
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
            while (state != done && token.upper() != "IS" && token.upper() != "AS" && token != ";");

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

            Parameters->clear();
            QListViewItem *head = new toResultViewItem(Parameters, NULL, tr("Input"));
            QListViewItem *last = NULL;
            head->setOpen(true);

            for (std::list<debugParam>::iterator i = CurrentParams.begin();i != CurrentParams.end();i++)
            {
                if ((*i).In)
                    last = new toResultViewItem(head, last, (*i).Name);
                sql += sep;
                sql += QString::fromLatin1(":");
                QString nam = (*i).Name;
                nam.replace(QRegExp(QString::fromLatin1("[^a-zA-Z0-9]+")), QString::fromLatin1("_"));
                sql += nam;
                sql += QString::fromLatin1("<char[");
                sql += toTool::globalConfig(CONF_MAX_COL_SIZE, DEFAULT_MAX_COL_SIZE);
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
                sql += toTool::globalConfig(CONF_MAX_COL_SIZE, DEFAULT_MAX_COL_SIZE);
                sql += QString::fromLatin1("],out> FROM sys.DUAL");
            }
            sql += QString::fromLatin1(";\nEND;\n");

            {
                // Can't hold lock since refresh of output will try to lock
                toQList input;
                try
                {
                    input = toParamGet::getParam(connection(), this, sql);
                }
                catch (...)
                {
                    return ;
                }
                toLocker lock (Lock)
                    ;
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
                ColumnSize = toTool::globalConfig(CONF_MAX_COL_SIZE, DEFAULT_MAX_COL_SIZE).toInt();
                TargetSQL = toDeepCopy(sql); // Deep copy of SQL
                TargetSemaphore.up(); // Go go power rangers!
            }
            StartedSemaphore.down();
            if (sync() >= 0 && RunningTarget)
                continueExecution(TO_BREAK_ANY_CALL);
        }
        TOCATCH
    }
    else
        toStatusMessage(tr("Couldn't find any function or procedure under cursor."));
}

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
    try
    {
        toQList args;
        toPush(args, toQValue(0));
        int ret;
        int reason;
        do
        {
            toQuery sync(connection(), SQLSync, args);

            ret = sync.readValue().toInt();
            reason = sync.readValue().toInt();
            {
                toLocker lock (Lock)
                    ;
                TargetLog += QString::fromLatin1("Syncing debug session\n");
                if (!RunningTarget)
                {
                    return TO_REASON_KNL_EXIT;
                }
            }
#if 0
            qApp->processEvents();
#endif

        }
        while (reason == TO_REASON_TIMEOUT || ret == TO_ERROR_TIMEOUT);
        setDeferedBreakpoints();
        if (Output->enabled())
            Output->disable(false);
        else
            Output->disable(true);
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
            name = statements.String.upper();

            int j;
            for (j = 0;TypeMap[j].Type && TypeMap[j].Type != name;j++)
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
        else if (statements.String.upper() != "BODY")
        {
            name += " " + statements.String;
            return true;
        }
        else
            name += " body";
    }
    for (std::list<toSQLParse::statement>::iterator i = statements.subTokens().begin();i != statements.subTokens().end();i++)
    {
        bool ret = FindKeyword(*i, onlyNames, declaration, line, name);
        if (ret)
            return ret;
    }
    return false;
}

void toDebug::updateArguments(toSQLParse::statement &statements, QListViewItem *parent)
{
    for (std::list<toSQLParse::statement>::iterator i = statements.subTokens().begin();i != statements.subTokens().end();i++)
    {
        if ((*i).Type == toSQLParse::statement::List)
        {
            bool first = true;
            for (std::list<toSQLParse::statement>::iterator j = (*i).subTokens().begin();j != (*i).subTokens().end();j++)
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

void toDebug::updateContent(toSQLParse::statement &statements, QListViewItem *parent, const QString &id)
{
    QListViewItem *item = NULL;
    int line;
    QString name;
    bool declaration;
    if (!FindKeyword(statements, statements.Type == toSQLParse::statement::Statement, declaration, line, name) || name.isNull())
        return ;

    if (parent)
        item = new toContentsItem(parent, name, line);
    else
    {
        for (item = Contents->firstChild();item;item = item->nextSibling())
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
                    if ((*j).String.upper() == "BEGIN")
                        declaration = false;
                    else if ((*j).Type == toSQLParse::statement::Token && (*j).String.upper() != "END")
                        new toContentsItem(item, "Variable " + (*j).String, (*j).Line);
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

    QListViewItem *item;

    for (item = Contents->firstChild();item;item = item->nextSibling())
        if (item->text(1) == current->name())
            item->setText(2, "DELETE");

    for (std::list<toSQLParse::statement>::iterator i = statements.begin();i != statements.end();i++)
        updateContent(*i, NULL, current->name());

    QListViewItem *ni;
    for (item = Contents->firstChild();item;item = ni)
    {
        ni = item->nextSibling();
        if (item->text(2) == "DELETE")
            delete item;
    }
}

void toDebug::readLog(void)
{
    toLocker lock (Lock)
        ;
    if (!TargetLog.isEmpty())
    {
        TargetLog.replace(TargetLog.length() - 1, 1, QString::null);
        RuntimeLog->insertLine(TargetLog);
        RuntimeLog->setCursorPosition(RuntimeLog->numLines() - 1, 0);
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

void toDebug::updateState(int reason)
{
    switch (reason)
    {
    case TO_REASON_EXIT:
    case TO_REASON_KNL_EXIT:
        ChildSemaphore.down();
        // Intentionally no break here
    case TO_REASON_NO_SESSION:
        StopButton->setEnabled(false);
        StepOverButton->setEnabled(false);
        StepIntoButton->setEnabled(false);
        ReturnButton->setEnabled(false);
        if (ToolMenu)
        {
            ToolMenu->setItemEnabled(TO_ID_STOP, false);
            ToolMenu->setItemEnabled(TO_ID_STEP_INTO, false);
            ToolMenu->setItemEnabled(TO_ID_STEP_OVER, false);
            ToolMenu->setItemEnabled(TO_ID_RETURN_FROM, false);
        }
        {
            for (int i = 0;i < Editors->count();i++)
            {
                toDebugText *editor = dynamic_cast<toDebugText *>(Editors->page(i));
                editor->setCurrent( -1);
            }
            StackTrace->clear();
            toLocker lock (Lock)
                ;
            if (OutputData.begin() != OutputData.end())
            {
                QListViewItem *head = Parameters->firstChild();
                while (head && head->nextSibling())
                    head = head->nextSibling();
                head = new toResultViewItem(Parameters, head, tr("Output"));
                head->setOpen(true);
                std::list<debugParam>::iterator cp;
                for (cp = CurrentParams.begin();cp != CurrentParams.end() && !(*cp).Out;cp++)
                    ;

                QListViewItem *last = NULL;
                {
                    for (toQList::iterator i = OutputData.begin();i != OutputData.end();i++)
                    {
                        QString name;
                        if (cp != CurrentParams.end())
                        {
                            name = (*cp).Name;
                            for (cp++;cp != CurrentParams.end() && !(*cp).Out;cp++)
                                ;
                        }
                        if (name.isEmpty())
                            name = tr("Returning");
                        last = new toResultViewItem(head, last, name);
                        last->setText(1, toDeepCopy(*i)); // Deep copy just to be sure
                    }
                }
            }
            QListViewItem *next = NULL;
            for (QListViewItem *item = Watch->firstChild();item;item = next)
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
        DebugButton->setOn(true);
        StopButton->setEnabled(true);
        StepOverButton->setEnabled(true);
        StepIntoButton->setEnabled(true);
        ReturnButton->setEnabled(true);
        if (ToolMenu)
        {
            ToolMenu->setItemEnabled(TO_ID_STOP, true);
            ToolMenu->setItemEnabled(TO_ID_STEP_INTO, true);
            ToolMenu->setItemEnabled(TO_ID_STEP_OVER, true);
            ToolMenu->setItemEnabled(TO_ID_RETURN_FROM, true);
        }
        try
        {
            toQuery info(connection(), SQLRuntimeInfo);
            int ret, depth;
            ret = info.readValue().toInt();
            depth = info.readValue().toInt();
            if (ret != TO_SUCCESS)
            {
                toStatusMessage(tr("Failed to get runtime info (Reason %1)").arg(ret));
                return ;
            }

            QListViewItem *item = NULL;
            StackTrace->clear();
            QString name;
            QString schema;
            QString line;
            QString type;

            for (int num = 2;num <= depth;num++)
            {
                toQList args;
                toPush(args, toQValue(num));
                toQuery stack(connection(), SQLStackTrace, args);

                name = stack.readValue();
                schema = stack.readValue();
                line = stack.readValue();
                type = stack.readValue();

                if (!item)
                    item = new QListViewItem(StackTrace, name, line, schema, type);
                else
                    item = new QListViewItem(item, name, line, schema, type);
                item->setOpen(true);
            }
            Output->refresh();
            try
            {
                {
                    for (QListViewItem *item = Watch->firstChild();item;item = item->nextSibling())
                    {
                        while (item->firstChild())
                            delete item->firstChild();
                    }
                }

                QListViewItem *next = NULL;
                for (QListViewItem *item = Watch->firstChild();item;item = next)
                {
                    int ret = -1;
                    int space = 0;
                    QString value;
                    bool local = false;
                    QString object;
                    QString schema;
                    if (!item->text(6).isEmpty())
                    {
                        local = true;
                        toQuery query(connection(), SQLLocalWatch, item->text(2));
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
                                       item->text(2));
                            ret = q2.readValue().toInt();
                            value = q2.readValue();
                            space = q2.readValue().toInt();
                            item->setText(0, schema);
                            item->setText(1, object);
                        }
                    }
                    else if (item->text(0).isEmpty())
                    {
                        toQuery query(connection(), SQLLocalWatch, item->text(2));
                        ret = query.readValue().toInt();
                        value = query.readValue();
                        local = true;
                    }
                    else
                    {
                        object = item->text(1);
                        schema = item->text(0);
                        toQuery query(connection(), SQLGlobalWatch,
                                      object, schema, item->text(2));
                        ret = query.readValue().toInt();
                        value = query.readValue();
                        space = query.readValue().toInt();
                        local = false;
                    }
                    item->setText(4, QString::null);
                    if (ret == TO_SUCCESS)
                        item->setText(3, value);
                    else if (ret == TO_ERROR_NULLVALUE)
                    {
                        if (toTool::globalConfig(CONF_INDICATE_EMPTY, "").isEmpty())
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
                            toQuery query(connection(), SQLLocalIndex, item->text(2));
                            value = query.readValue();
                        }
                        else
                        {
                            toQList args;
                            toPush(args, toQValue(space));
                            toPush(args, toQValue(object));
                            toPush(args, toQValue(schema));
                            toPush(args, toQValue(item->text(2)));
                            toQuery query(connection(), SQLGlobalIndex, args);
                            value = query.readValue();
                        }
                        unsigned int start = 0;
                        unsigned int end;
                        QListViewItem *last = NULL;
                        int num = 0;
                        for (end = start;end < value.length();end++)
                        {
                            if (value.at(end) == ',')
                            {
                                if (start < end)
                                {
                                    QString name = item->text(2);
                                    name += QString::fromLatin1("(");
                                    // Why do I have to add 1 here for it to work?
                                    name += QString::number(value.mid(start, end - start).toInt() + 1);
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
            }
            TOCATCH
            if (depth >= 2)
            {
                viewSource(schema, name, type, line.toInt(), true);
            }
            else
            {
                if (RunningTarget)
                    continueExecution(TO_BREAK_NEXT_LINE);
                return ;
            }
        }
        TOCATCH
        break;
    }
    selectedWatch();
    readLog();
}

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
        for (end = start;end < value.length();end++)
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
        for (int i = 0;i < Editors->count();i++)
        {
            QString tabname = editorName(schema, name, type);
            toDebugText *te = dynamic_cast<toDebugText *>(Editors->page(i));
            if (Editors->tabLabel(te) == tabname)
            {
                editor = te;
                break;
            }
            if (Editors->tabLabel(te) == tr("Unknown") && !te->edited())
                editor = te;
        }
        if (!editor)
        {
            editor = new toDebugText(Breakpoints, Editors, this);
            connect(editor, SIGNAL(insertedLines(int, int)),
                    this, SLOT(reorderContent(int, int)));
            Editors->addTab(editor, editorName(editor));
        }
        if (editor->numLines() <= 1)
        {
            editor->setData(schema, type, name);
            editor->readData(connection(), StackTrace);
            updateContent(editor);
            Editors->changeTab(editor, editorName(editor));
            if (editor->hasErrors())
                Editors->setTabIconSet(editor, QIconSet(QPixmap(const_cast<const char**>(nextbug_xpm))));
            else
                Editors->setTabIconSet(editor, QIconSet());
        }
        Editors->showPage(editor);
        if (setCurrent)
            editor->setCurrent(line - 1);
        else
            editor->setCursorPosition(line - 1, 0);
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
    for (QListViewItem *item = Breakpoints->firstChild();item;item = item->nextSibling())
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
    Lock.lock();
    if (RunningTarget)
    {
        Lock.unlock();
        try
        {
            int ret, reason;
            setDeferedBreakpoints();
            toQList args;
            toPush(args, toQValue(stopon));
            toQuery cont(connection(), SQLContinue, args);
            ret = cont.readValue().toInt();
            reason = cont.readValue().toInt();
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
    toBusy busy;
    {
        toLocker lock (Lock)
            ;
        TargetSQL = toDeepCopy(str);
        InputData = params;
        TargetSemaphore.up();
    }
    StartedSemaphore.down();
    int ret = sync();
    while (ret >= 0 && ret != TO_REASON_EXIT && ret != TO_REASON_KNL_EXIT && RunningTarget)
    {
        ret = continueExecution(TO_BREAK_ANY_RETURN);
    }
    readLog();
}

void toDebug::stop(void)
{
    if (RunningTarget)
        continueExecution(TO_ABORT_EXECUTION);
}

toDebug::toDebug(QWidget *main, toConnection &connection)
        : toToolWidget(DebugTool, "debugger.html", main, connection), TargetThread()
{
    QToolBar *toolbar = toAllocBar(this, tr("Debugger"));

    new QToolButton(QPixmap(const_cast<const char**>(refresh_xpm)),
                    tr("Update object list"),
                    tr("Update object list"),
                    this, SLOT(refresh(void)),
                    toolbar);
    toolbar->addSeparator();
    Schema = new QComboBox(toolbar, TO_KDE_TOOLBAR_WIDGET);
    connect(Schema, SIGNAL(activated(int)),
            this, SLOT(changeSchema(int)));

    toolbar->addSeparator();
    new QToolButton(QPixmap(const_cast<const char**>(toworksheet_xpm)),
                    tr("New sheet"),
                    tr("New sheet"),
                    this, SLOT(newSheet(void)),
                    toolbar);
    new QToolButton(QPixmap(const_cast<const char**>(scansource_xpm)),
                    tr("Rescan source"),
                    tr("Rescan source"),
                    this, SLOT(scanSource(void)),
                    toolbar);
    new QToolButton(QPixmap(const_cast<const char**>(compile_xpm)),
                    tr("Compile"),
                    tr("Compile"),
                    this, SLOT(compile(void)),
                    toolbar);
    toolbar->addSeparator();
    new QToolButton(QPixmap(const_cast<const char**>(execute_xpm)),
                    tr("Execute or continue execution"),
                    tr("Execute or continue execution"),
                    this, SLOT(execute(void)),
                    toolbar);
    StopButton = new QToolButton(QPixmap(const_cast<const char**>(stop_xpm)),
                                 tr("Stop running"),
                                 tr("Stop running"),
                                 this, SLOT(stop(void)),
                                 toolbar);
    StopButton->setEnabled(false);
    toolbar->addSeparator();
    StepIntoButton = new QToolButton(QPixmap(const_cast<const char**>(stepinto_xpm)),
                                     tr("Step into procedure or function"),
                                     tr("Step into procedure or function"),
                                     this, SLOT(stepInto(void)),
                                     toolbar);
    StepIntoButton->setEnabled(false);
    StepOverButton = new QToolButton(QPixmap(const_cast<const char**>(stepover_xpm)),
                                     tr("Step over procedure or function"),
                                     tr("Step over procedure or function"),
                                     this, SLOT(stepOver(void)),
                                     toolbar);
    StepOverButton->setEnabled(false);
    ReturnButton = new QToolButton(QPixmap(const_cast<const char**>(returnfrom_xpm)),
                                   tr("Return from procedure or function"),
                                   tr("Return from procedure or function"),
                                   this, SLOT(returnFrom(void)),
                                   toolbar);
    ReturnButton->setEnabled(false);

    toolbar->addSeparator();

    DebugButton = new QToolButton(toolbar);
    DebugButton->setToggleButton(true);
    DebugButton->setIconSet(QIconSet(QPixmap(const_cast<const char**>(todebug_xpm))));
    connect(DebugButton, SIGNAL(toggled(bool)), this, SLOT(showDebug(bool)));
    QToolTip::add
        (DebugButton, tr("Show/hide debug info pane."));

    toolbar->addSeparator();
    new QToolButton(QPixmap(const_cast<const char**>(nextbug_xpm)),
                    tr("Go to next error"),
                    tr("Go to next error"),
                    this, SLOT(nextError(void)),
                    toolbar);
    new QToolButton(QPixmap(const_cast<const char**>(prevbug_xpm)),
                    tr("Go to previous error"),
                    tr("Go to previous error"),
                    this, SLOT(prevError(void)),
                    toolbar);

    toolbar->addSeparator();
    new QToolButton(QPixmap(const_cast<const char**>(togglebreak_xpm)),
                    tr("Toggle breakpoint on current line"),
                    tr("Toggle breakpoint on current line"),
                    this, SLOT(toggleBreak(void)),
                    toolbar);
    new QToolButton(QPixmap(const_cast<const char**>(enablebreak_xpm)),
                    tr("Enable/disable breakpoint on current line"),
                    tr("Enable/disable breakpoint on current line"),
                    this, SLOT(toggleEnable(void)),
                    toolbar);

    toolbar->addSeparator();
    new QToolButton(QPixmap(const_cast<const char**>(addwatch_xpm)),
                    tr("Add new variable watch"),
                    tr("Add new variable watch"),
                    this, SLOT(addWatch(void)),
                    toolbar);
    DelWatchButton = new QToolButton(QPixmap(const_cast<const char**>(delwatch_xpm)),
                                     tr("Delete variable watch"),
                                     tr("Delete variable watch"),
                                     this, SLOT(deleteWatch(void)),
                                     toolbar);
    ChangeWatchButton = new QToolButton(QPixmap(const_cast<const char**>(changewatch_xpm)),
                                        tr("Change value of watched variable"),
                                        tr("Change value of watched variable"),
                                        this, SLOT(changeWatch(void)),
                                        toolbar);
    DelWatchButton->setEnabled(false);
    ChangeWatchButton->setEnabled(false);

    toolbar->setStretchableWidget(new QLabel(toolbar, TO_KDE_TOOLBAR_WIDGET));

    QSplitter *splitter = new QSplitter(Vertical, this);
    QSplitter *hsplitter = new QSplitter(Horizontal, splitter);
    DebugTabs = new QTabWidget(splitter);
    DebugTabs->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum));
    DebugTabs->hide();


#if 0

    {
        QValueList<int> sizes = splitter->sizes();
        sizes[0] += sizes[1] - 200;
        sizes[1] = 200;
        splitter->setSizes(sizes);
    }
#endif
    splitter->setResizeMode(DebugTabs, QSplitter::KeepSize);

    QSplitter *objSplitter = new QSplitter(Vertical, hsplitter);

    Objects = new toListView(objSplitter);
    Objects->addColumn(tr("Objects"));
    Objects->setRootIsDecorated(true);
    Objects->setTreeStepSize(10);
    Objects->setSorting(0);
    Objects->setSelectionMode(QListView::Single);
    Objects->setResizeMode(QListView::AllColumns);
    connect(Objects, SIGNAL(selectionChanged(QListViewItem *)),
            this, SLOT(changePackage(QListViewItem *)));
    Contents = new toListView(objSplitter);
    Contents->addColumn(tr("Contents"));
    Contents->setRootIsDecorated(true);
    Contents->setSorting( -1);
    Contents->setTreeStepSize(10);
    Contents->setSelectionMode(QListView::Single);
    Contents->setResizeMode(QListView::AllColumns);
    connect(Contents, SIGNAL(selectionChanged(QListViewItem *)),
            this, SLOT(changeContent(QListViewItem *)));

    StackTrace = new toListView(DebugTabs);
    StackTrace->addColumn(tr("Object"));
    StackTrace->addColumn(tr("Line"));
    StackTrace->addColumn(tr("Schema"));
    StackTrace->addColumn(tr("Type"));
    StackTrace->setColumnAlignment(1, AlignRight);
    StackTrace->setSorting( -1);
    StackTrace->setRootIsDecorated(true);
    StackTrace->setTreeStepSize(10);
    StackTrace->setAllColumnsShowFocus(true);
    StackTrace->setResizeMode(QListView::AllColumns);
    DebugTabs->addTab(StackTrace, tr("&Stack Trace"));
    connect(StackTrace, SIGNAL(clicked(QListViewItem *)),
            this, SLOT(showSource(QListViewItem *)));

    Watch = new toListView(DebugTabs);
    Watch->addColumn(tr("Schema"));
    Watch->addColumn(tr("Object"));
    Watch->addColumn(tr("Variable"));
    Watch->addColumn(tr("Data"));
    Watch->setRootIsDecorated(true);
    Watch->setTreeStepSize(10);
    Watch->setAllColumnsShowFocus(true);
    Watch->setResizeMode(QListView::AllColumns);
    DebugTabs->addTab(Watch, tr("W&atch"));
    Watch->setSelectionMode(QListView::Single);
    connect(Watch, SIGNAL(selectionChanged(void)),
            this, SLOT(selectedWatch(void)));
    connect(Watch, SIGNAL(doubleClicked(QListViewItem *)),
            this, SLOT(changeWatch(QListViewItem *)));

    Breakpoints = new toListView(DebugTabs);
    Breakpoints->addColumn(tr("Object"));
    Breakpoints->addColumn(tr("Line"));
    Breakpoints->addColumn(tr("Schema"));
    Breakpoints->addColumn(tr("Object Type"));
    Breakpoints->addColumn(tr("Enabled"));
    Breakpoints->setColumnAlignment(1, AlignRight);
    Breakpoints->setSorting( -1);
    Breakpoints->setAllColumnsShowFocus(true);
    Breakpoints->setResizeMode(QListView::AllColumns);
    DebugTabs->addTab(Breakpoints, tr("&Breakpoints"));
    connect(Breakpoints, SIGNAL(clicked(QListViewItem *)),
            this, SLOT(showSource(QListViewItem *)));

    Parameters = new toListView(DebugTabs);
    Parameters->addColumn(tr("Name"));
    Parameters->addColumn(tr("Content"));
    Parameters->setSorting( -1);
    Parameters->setTreeStepSize(10);
    Parameters->setRootIsDecorated(true);
    Parameters->setAllColumnsShowFocus(true);
    Parameters->setResizeMode(QListView::AllColumns);
    DebugTabs->addTab(Parameters, tr("&Parameters"));

    Output = new toDebugOutput(this, DebugTabs, connection);
    DebugTabs->addTab(Output, tr("Debug &Output"));

    RuntimeLog = new toMarkedText(DebugTabs);
    DebugTabs->addTab(RuntimeLog, tr("&Runtime Log"));

    Editors = new QTabWidget(hsplitter);
    Editors->setTabPosition(QTabWidget::Bottom);

#if QT_VERSION >= 0x030200

    QToolButton *closeButton = new toPopupButton(Editors);
    closeButton->setIconSet(QPixmap(const_cast<const char**>(close_xpm)));
    closeButton->setFixedSize(20, 18);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeEditor()));
    Editors->setCornerWidget(closeButton);
#endif

    setFocusProxy(Editors);
    newSheet();

#if 0

    {
        QValueList<int> sizes = hsplitter->sizes();
        sizes[0] = 200;
        hsplitter->setSizes(sizes);
        hsplitter->setResizeMode(objSplitter, QSplitter::KeepSize);
    }
#endif

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(windowActivated(QWidget *)),
            this, SLOT(windowActivated(QWidget *)));

    refresh();
    connect(&StartTimer, SIGNAL(timeout(void)), this, SLOT(startTarget(void)));
    StartTimer.start(1, true);
}

static toSQL SQLAttach("toDebug:Attach",
                       "DECLARE\n"
                       "    timeout BINARY_INTEGER;\n"
                       "BEGIN\n"
                       "    SYS.DBMS_DEBUG.ATTACH_SESSION(:sess<char[201],in>);\n"
                       "    timeout:=SYS.DBMS_DEBUG.SET_TIMEOUT(1);\n"
                       "END;",
                       "Connect to the debugging session");

void toDebug::startTarget(void)
{
    try
    {
        toLocker lock (Lock)
            ;
        TargetThread = new toThread(new targetTask(*this));
        TargetThread->start();
    }
    catch (...)
    {
        toStatusMessage(tr("Failed to start target task thread, close some other tools and try again"));
        return ;
    }

    ChildSemaphore.down();
    if (!DebuggerStarted)
    {
        {
            toLocker lock (Lock)
                ;
            TOMessageBox::critical(this, tr("Couldn't start debugging"),
                                   tr("Couldn't connect to target session:\n") +
                                   TargetLog,
                                   tr("&Ok"));
        }
        close(false);
        return ;
    }
    try
    {
        connection().execute(SQLAttach, TargetID);
    }
    TOCATCH  // Trying to run somthing after this won't work (And will hang tora I think)
    readLog();
}

toDebugText *toDebug::currentEditor(void)
{
    return dynamic_cast<toDebugText *>(Editors->currentPage());
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
    try
    {
        QString selected = Schema->currentText();
        QString currentSchema;
        if (selected.isEmpty())
        {
            selected = connection().user().upper();
            Schema->clear();
            toQList users = toQuery::readQuery(connection(),
                                               toSQL::string(toSQL::TOSQL_USERLIST, connection()));
            for (toQList::iterator i = users.begin();i != users.end();i++)
                Schema->insertItem(*i);
        }
        if (!selected.isEmpty())
        {
            {
                for (int i = 0;i < Schema->count();i++)
                    if (Schema->text(i) == selected)
                    {
                        Schema->setCurrentItem(i);
                        break;
                    }
            }
            Objects->clear();

            std::list<toConnection::objectName> &objs = connection().objects(true);

            std::map<QString, QListViewItem *> typeItems;
            bool any = false;
            for (std::list<toConnection::objectName>::iterator i = objs.begin();
                    i != objs.end();i++)
            {
                if ((*i).Owner == selected)
                {
                    any = true;
                    QString type = (*i).Type;
                    if (type == QString::fromLatin1("FUNCTION") ||
                            type == QString::fromLatin1("PACKAGE") ||
                            type == QString::fromLatin1("PROCEDURE") ||
                            type == QString::fromLatin1("TYPE"))
                    {
                        QListViewItem *typeItem;
                        std::map<QString, QListViewItem *>::iterator j = typeItems.find(type);
                        if (j == typeItems.end())
                        {
                            typeItem = new QListViewItem(Objects, type);
                            typeItems[type] = typeItem;
#ifndef AUTOEXPAND

                            typeItem->setSelectable(false);
#endif

                        }
                        else
                            typeItem = (*j).second;

                        QString bodyType(type);
                        bodyType += QString::fromLatin1(" BODY");
                        QString name = (*i).Name;
                        QListViewItem *item = new QListViewItem(typeItem, name, type);
                        if (selected == currentEditor()->schema() &&
                                (type == currentEditor()->type() ||
                                 bodyType == currentEditor()->type()) &&
                                name == currentEditor()->object())
                        {
                            Objects->setOpen(typeItem, true);
                            Objects->setSelected(item, true);
                        }
                    }
                }
                else if (any)
                    break;
            }
        }
    }
    TOCATCH
}

bool toDebug::checkStop(void)
{
    Lock.lock();
    if (RunningTarget)
    {
        Lock.unlock();
        if (TOMessageBox::information(this, tr("Stop execution?"),
                                      tr("Do you want to abort the current execution?"),
                                      tr("&Ok"), tr("Cancel")) != 0)
            return false;
        stop();
    }
    else
        Lock.unlock();
    return true;
}

bool toDebug::checkCompile(toDebugText *editor)
{
    if (editor->edited())
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
            editor->setEdited(false);
            break;
        case 2:
            return false;
        }
    }
    return true;
}

bool toDebug::checkCompile(void)
{
    for (int i = 0;i < Editors->count();i++)
    {
        toDebugText *editor = dynamic_cast<toDebugText *>(Editors->page(i));
        if (!checkCompile(editor))
            return false;
    }
    return true;
}

bool toDebug::close(bool del)
{
    if (checkCompile())
        return QVBox::close(del);
    return false;
}

void toDebug::updateCurrent()
{
    try
    {
        toDebugText *editor = currentEditor();

        editor->readData(connection(), StackTrace);
        editor->setFocus();

        updateContent();
    }
    TOCATCH
}

void toDebug::changePackage(QListViewItem *item)
{
    if (item && item->parent())
    {
        viewSource(Schema->currentText(), item->text(0), item->text(1), 0);
        if (item->text(1) == "PACKAGE" || item->text(1) == "TYPE")
            viewSource(Schema->currentText(), item->text(0), item->text(1) + " BODY", 0);
    }
#ifdef AUTOEXPAND
    else if (item && !item->parent())
        item->setOpen(true);
#endif
}

void toDebug::showDebug(bool show)
{
    if (show)
        DebugTabs->show();
    else
        DebugTabs->hide();
    if (ToolMenu)
        ToolMenu->setItemChecked(TO_ID_DEBUG_PANE, show);
}

bool toDebugText::compile(void)
{
    QString str = text();
    bool ret = true;
    if (!str.isEmpty())
    {
        bool body = false;

        toSQLParse::stringTokenizer tokens(str);

        QString token = tokens.getToken();

        if (token.upper() == "CREATE")
        {
            token = tokens.getToken();
            if (token.upper() == "OR")
            {
                token = tokens.getToken();
                if (token.upper() == "REPLACE")
                    token = tokens.getToken();
            }
        }

        QString type = token.upper();
        if (type != QString::fromLatin1("PROCEDURE") &&
                type != QString::fromLatin1("TYPE") &&
                type != QString::fromLatin1("FUNCTION") &&
                type != QString::fromLatin1("PACKAGE"))
        {
            toStatusMessage(tr("Invalid start of code"));
            return false;
        }

        token = tokens.getToken();
        if (token.upper() == "BODY")
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
            Schema = schema.upper();
            Object = Debugger->connection().unQuote(object.upper());
            Type = type.upper();
            if (body)
                Type += QString::fromLatin1(" BODY");
            readErrors(Debugger->connection());
            setEdited(false);
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
    return ret;
}

void toDebug::compile(void)
{
    if (!checkStop())
        return ;

    QString lastSchema = currentEditor()->schema();
    for (int i = 0;i < Editors->count();i++)
    {
        toDebugText *editor = dynamic_cast<toDebugText *>(Editors->page(i));
        if (editor->compile())
        {
            if (editor == currentEditor() &&
                    lastSchema != currentEditor()->schema())
            {
                for (int i = 0;i < Schema->count();i++)
                    if (Schema->text(i) == lastSchema)
                    {
                        Schema->setCurrentItem(i);
                        break;
                    }
            }
            if (editor->hasErrors())
                Editors->setTabIconSet(editor, QIconSet(QPixmap(const_cast<const char**>(nextbug_xpm))));
            else
                Editors->setTabIconSet(editor, QIconSet());
            Editors->changeTab(editor, editorName(editor));
        }
        else
            return ;

    }
    refresh();
}

toDebug::~toDebug()
{
    try
    {
        Lock.lock();
        if (DebuggerStarted)
        {
            Lock.unlock();
            stop();
            {
                toLocker lock (Lock)
                    ;
                TargetSQL = "";
                TargetSemaphore.up();
            }
            ChildSemaphore.down();
        }
        else
            Lock.unlock();
    }
    TOCATCH
    try
    {
        DebugTool.closeWindow(connection());
    }
    TOCATCH
}

void toDebug::prevError(void)
{
    currentEditor()->previousError();
}

void toDebug::nextError(void)
{
    currentEditor()->nextError();
}

void toDebug::changeContent(QListViewItem *ci)
{
    toContentsItem *item = dynamic_cast<toContentsItem *>(ci);
    if (item)
    {
        while (ci->parent())
            ci = ci->parent();
        toHighlightedText *current = NULL;

        for (int i = 0;i < Editors->count();i++)
        {
            if (Editors->page(i)->name() == ci->text(1))
            {
                current = dynamic_cast<toDebugText *>(Editors->page(i));
                break;
            }
        }
        if (current)
        {
            current->setCursorPosition(item->Line, 0);
            Editors->showPage(current);
            current->setFocus();
        }
    }
#ifdef AUTOEXPAND
    else
        ci->setOpen(true);
#endif
}

void toDebug::scanSource(void)
{
    updateContent();
}

void toDebug::newSheet(void)
{
    toDebugText *text = new toDebugText(Breakpoints, Editors, this);
    connect(text, SIGNAL(insertedLines(int, int)),
            this, SLOT(reorderContent(int, int)));
    if (!Schema->currentText().isEmpty())
        text->setSchema(Schema->currentText());
    else
        text->setSchema(connection().user().upper());
    Editors->addTab(text, tr("Unknown"));
    Editors->showPage(text);
}

void toDebug::showSource(QListViewItem *item)
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

void toDebug::windowActivated(QWidget *widget)
{
    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QPopupMenu(this);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(toworksheet_xpm)),
                                 tr("&New Sheet"), this, SLOT(newSheet(void)),
                                 0, TO_ID_NEW_SHEET);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(scansource_xpm)),
                                 tr("S&can Source"), this, SLOT(scanSource(void)),
                                 toKeySequence(tr("Ctrl+F9", "Debug|Scan source")), TO_ID_SCAN_SOURCE);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(compile_xpm)),
                                 tr("&Compile"), this, SLOT(compile(void)),
                                 toKeySequence(tr("F9", "Debug|Compile")), TO_ID_COMPILE);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(close_xpm)),
                                 tr("Close"), this, SLOT(closeEditor(void)),
                                 0, TO_ID_CLOSE_EDITOR);
            ToolMenu->insertItem(tr("CloseAll"), this, SLOT(closeAllEditor(void)),
                                 0, TO_ID_CLOSE_ALL_EDITOR);
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(execute_xpm)),
                                 tr("&Execute or continue"), this, SLOT(execute(void)),
                                 toKeySequence(tr("Ctrl+Return", "Debug|Execute")), TO_ID_EXECUTE);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(stop_xpm)),
                                 tr("&Stop"), this, SLOT(stop(void)),
                                 toKeySequence(tr("F12", "Debug|Stop")), TO_ID_STOP);
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(stepinto_xpm)),
                                 tr("Step &Into"), this, SLOT(stepInto(void)),
                                 toKeySequence(tr("F7", "Debug|Step into")), TO_ID_STEP_INTO);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(stepover_xpm)),
                                 tr("&Next Line"), this, SLOT(stepOver(void)),
                                 toKeySequence(tr("F8", "Debug|Stop over")), TO_ID_STEP_OVER);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(returnfrom_xpm)),
                                 tr("&Return From"), this, SLOT(returnFrom(void)),
                                 toKeySequence(tr("F6", "Debug|Return from")), TO_ID_RETURN_FROM);
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(tr("&Debug Pane"), this, SLOT(toggleDebug(void)),
                                 toKeySequence(tr("F11", "Debug|Debug pane")), TO_ID_DEBUG_PANE);
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(nextbug_xpm)),
                                 tr("Next &Error"), this, SLOT(nextError(void)),
                                 toKeySequence(tr("Ctrl+N", "Debug|Next error")));
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(prevbug_xpm)),
                                 tr("Pre&vious Error"), this, SLOT(prevError(void)),
                                 toKeySequence(tr("Ctrl+P", "Debug|Previous error")));
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(togglebreak_xpm)),
                                 tr("&Toggle Breakpoint"), this, SLOT(toggleBreak(void)),
                                 toKeySequence(tr("Ctrl+F5", "Debug|Toggle breakpoint")));
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(enablebreak_xpm)),
                                 tr("D&isable Breakpoint"),
                                 this, SLOT(toggleEnable(void)),
                                 toKeySequence(tr("Ctrl+F6", "Debug|Disable breakpoint")));
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(addwatch_xpm)),
                                 tr("&Add Watch..."), this, SLOT(addWatch(void)),
                                 toKeySequence(tr("F4", "Debug|Add watch")));
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(delwatch_xpm)),
                                 tr("Delete &Watch"), this, SLOT(deleteWatch(void)),
                                 toKeySequence(tr("Ctrl+Delete", "Debug|Delete watch")), TO_ID_DEL_WATCH);
            ToolMenu->insertItem(QPixmap(const_cast<const char**>(changewatch_xpm)),
                                 tr("Chan&ge Watch..."), this, SLOT(changeWatch(void)),
                                 toKeySequence(tr("Ctrl+F4", "Debug|Change watch")), TO_ID_CHANGE_WATCH);
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(tr("Refresh Object List"), this, SLOT(refresh()),
                                 toKeySequence(tr("F5", "Debug|Refresh objectlist")));
            ToolMenu->insertItem(tr("Select Schema"), Schema, SLOT(setFocus(void)),
                                 toKeySequence(tr("Alt+S", "Debug|Select schema")));
            ToolMenu->insertItem(tr("Erase Runtime &Log"), this, SLOT(clearLog(void)));

            toMainWidget()->menuBar()->insertItem(tr("&Debug"), ToolMenu, -1, toToolMenuIndex());
            if (!isRunning())
            {
                ToolMenu->setItemEnabled(TO_ID_STOP, false);
                ToolMenu->setItemEnabled(TO_ID_STEP_INTO, false);
                ToolMenu->setItemEnabled(TO_ID_STEP_OVER, false);
                ToolMenu->setItemEnabled(TO_ID_RETURN_FROM, false);
            }
            if (!DebugTabs->isHidden())
                ToolMenu->setItemChecked(TO_ID_DEBUG_PANE, true);

            if (!DelWatchButton->isEnabled())
                ToolMenu->setItemEnabled(TO_ID_DEL_WATCH, false);
            if (!ChangeWatchButton->isEnabled())
                ToolMenu->setItemEnabled(TO_ID_CHANGE_WATCH, false);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toDebug::toggleDebug(void)
{
    DebugButton->setOn(!DebugButton->isOn());
}

void toDebug::selectedWatch()
{
    QListViewItem *item = Watch->selectedItem();
    if (item)
    {
        if (!item->text(5).isEmpty() && item->text(5) != QString::fromLatin1("LIST") && item->text(5) != QString::fromLatin1("NULL"))
        {
            DelWatchButton->setEnabled(false);
            if (ToolMenu)
                ToolMenu->setItemEnabled(TO_ID_DEL_WATCH, false);
        }
        else
        {
            DelWatchButton->setEnabled(true);
            if (ToolMenu)
                ToolMenu->setItemEnabled(TO_ID_DEL_WATCH, true);
        }
        if (item->text(4).isEmpty())
        {
            ChangeWatchButton->setEnabled(true);
            if (ToolMenu)
                ToolMenu->setItemEnabled(TO_ID_CHANGE_WATCH, true);
        }
        else
        {
            ChangeWatchButton->setEnabled(false);
            if (ToolMenu)
                ToolMenu->setItemEnabled(TO_ID_CHANGE_WATCH, false);
        }
    }
    else
    {
        DelWatchButton->setEnabled(false);
        ChangeWatchButton->setEnabled(false);
        if (ToolMenu)
        {
            ToolMenu->setItemEnabled(TO_ID_DEL_WATCH, false);
            ToolMenu->setItemEnabled(TO_ID_CHANGE_WATCH, false);
        }
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

void toDebug::changeWatch(QListViewItem *item)
{
    if (item && item->text(4).isEmpty())
    {
        QString description = tr("Enter new value to the watch %1").arg(item->text(2));
        QString data;

        toDebugChangeUI dialog(this, "WatchChange", true);
        toHelp::connectDialog(&dialog);

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

        if (dialog.exec())
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
                    toQuery local(connection(), SQLChangeLocal, assign);
                    ret = local.readValue().toInt();
                }
                else
                {
                    QString tmp = item->text(1);
                    if (tmp.isEmpty())
                        tmp = "";
                    toQuery local(connection(), SQLChangeGlobal, tmp, item->text(0), assign);
                    ret = local.readValue().toInt();
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

void toDebug::exportData(std::map<QCString, QString> &data, const QCString &prefix)
{
    data[prefix + ":Editors"] = Editors->count();
    for (int i = 0;i < Editors->count();i++)
    {
        toHighlightedText *editor = dynamic_cast<toHighlightedText *>(Editors->page(i));
        QCString num;
        num.setNum(i);
        editor->exportData(data, prefix + ":Editor:" + num);
    }
    data[prefix + ":Schema"] = Schema->currentText();

    int id = 1;
    for (QListViewItem *item = Breakpoints->firstChild();item;item = item->nextSibling())
    {
        toBreakpointItem * point = dynamic_cast<toBreakpointItem *>(item);

        if (point)
        {
            QCString key = prefix + ":Breaks:" + QString::number(id).latin1();

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
    for (QListViewItem *qitem = Watch->firstChild();qitem;qitem = qitem->nextSibling())
    {
        toResultViewItem * item = dynamic_cast<toResultViewItem *>(qitem);
        if (item)
        {
            QCString key = prefix + ":Watch:" + QString::number(id).latin1();
            data[key + ":Schema"] = item->allText(0);
            data[key + ":Object"] = item->allText(1);
            data[key + ":Item"] = item->allText(2);
            data[key + ":Auto"] = item->allText(6);
        }
        id++;
    }
    if (DebugButton->isOn())
        data[prefix + ":Debug"] = QString::fromLatin1("Show");

    toToolWidget::exportData(data, prefix);
}

void toDebug::importData(std::map<QCString, QString> &data, const QCString &prefix)
{
    QString str = data[prefix + ":Schema"];
    {
        for (int i = 0;i < Schema->count();i++)
            if (Schema->text(i) == str)
            {
                Schema->setCurrentItem(i);
                changeSchema(i);
                break;
            }
    }

    int count = data[prefix + ":Editors"].toInt();
    for (int j = 0;j < count;j++)
    {
        toDebugText *text = new toDebugText(Breakpoints, Editors, this);
        connect(text, SIGNAL(insertedLines(int, int)),
                this, SLOT(reorderContent(int, int)));
        QCString num;
        num.setNum(j);
        text->importData(data, prefix + ":Editor:" + num);
        Editors->addTab(text, editorName(text));
    }

    int id = 1;
    std::map<QCString, QString>::iterator i;
    toBreakpointItem *debug = NULL;
    while ((i = data.find(prefix + ":Breaks:" + QString::number(id).latin1() + ":Line")) != data.end())
    {
        QCString key = prefix + ":Breaks:" + QString::number(id).latin1();
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
    while ((i = data.find(prefix + ":Watch:" + QString::number(id).latin1() + ":Item")) != data.end())
    {
        QCString key = prefix + ":Watch:" + QString::number(id).latin1();
        item = new toResultViewItem(Watch, NULL, data[key + ":Schema"]);
        item->setText(1, data[key + ":Object"]);
        item->setText(2, data[key + ":Item"]);
        item->setText(4, QString::fromLatin1("NOCHANGE"));
        if (!data[key + ":Auto"].isEmpty())
            item->setText(6, "AUTO");
        id++;
    }
    scanSource();

    DebugButton->setOn(data[prefix + ":Debug"] == QString::fromLatin1("Show"));

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
        toDebugText *editor = dynamic_cast<toDebugText *>(Editors->page(editorCount));
        if (editor)
            closeEditor(editor);
    }
}


void toDebug::closeEditor(toDebugText* &editor)
{

    if (editor && checkCompile(editor))
    {
        QString name = editor->name();
        for (QListViewItem *item = Contents->firstChild();item;item = item->nextSibling())
        {
            if (item->text(1) == name)
            {
                delete item;
                break;
            }
        }

        if (Objects->selectedItem() &&
                Objects->selectedItem()->text(0) == editor->object() &&
                Schema->currentText() == editor->schema())
            Objects->clearSelection();

        Editors->removePage(editor);
        delete editor;
        if (Editors->count() == 0)
            newSheet();
    }
}
