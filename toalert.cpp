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

#include "toalert.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "tonoblockquery.h"
#include "toresultview.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"

#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <algorithm>

#include <stdio.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include "toalert.moc"

#include "icons/commit.xpm"
#include "icons/return.xpm"
#include "icons/toalert.xpm"
#include "icons/toworksheet.xpm"
#include "icons/trash.xpm"

class toAlertTool : public toTool
{
protected:
    std::map<toConnection *, QWidget *> Windows;

    virtual const char **pictureXPM(void)
    {
        return toalert_xpm;
    }
public:
    toAlertTool()
            : toTool(330, "Alert Messenger")
    { }
    virtual const char *menuItem()
    {
        return "Alert Messenger";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
        if (i != Windows.end())
        {
            (*i).second->setFocus();
            return NULL;
        }
        else
        {
            QWidget *window = new toAlert(parent, connection);
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
};

static toAlertTool AlertTool;

#define TIMEOUT 1

static toSQL SQLHost("toAlert:Hostname",
                     "SELECT machine\n"
                     "  FROM v$session\n"
                     " WHERE sid=(SELECT MIN(sid)\n"
                     "       FROM v$mystat)",
                     "Get hostname of current connection");

toAlert::toAlert(QWidget *main, toConnection &connection)
        : toToolWidget(AlertTool, "alert.html", main, connection), Connection(connection)
{
    QToolBar *toolbar = toAllocBar(this, tr("Alert Messenger"));

    QString def;
    try
    {
        toQList lst = toQuery::readQuery(connection, SQLHost);
        def += toShift(lst);
        if (!def.isEmpty())
            def += QString::fromLatin1(":");
    }
    catch (...)
    {}
    def += connection.user();

    new QLabel(tr("Registered") + " ", toolbar, TO_KDE_TOOLBAR_WIDGET);
    Registered = new QComboBox(toolbar, TO_KDE_TOOLBAR_WIDGET);
    Registered->insertItem(def);
    Registered->setEditable(true);
    Registered->setDuplicatesEnabled(false);
    Registered->setCurrentItem(0);
    connect(Registered, SIGNAL(activated(int)), this, SLOT(add
            ()));

    AddNames.insert(AddNames.end(), def);

    new QToolButton(QPixmap((const char **)commit_xpm),
                    tr("Register current"),
                    tr("Register current"),
                    this, SLOT(add
                               ()),
                    toolbar);

    new QToolButton(QPixmap((const char **)trash_xpm),
                    tr("Remove registered"),
                    tr("Remove registered"),
                    this, SLOT(remove
                               ()),
                    toolbar);

    toolbar->addSeparator();

    new QLabel(tr("Name") + " ", toolbar, TO_KDE_TOOLBAR_WIDGET);
    Name = new QLineEdit(toolbar, TO_KDE_TOOLBAR_WIDGET);
    Name->setText(def);
    connect(Name, SIGNAL(returnPressed()), this, SLOT(send()));
    new QLabel(tr("Message") + " ", toolbar, TO_KDE_TOOLBAR_WIDGET);
    Message = new QLineEdit(toolbar, TO_KDE_TOOLBAR_WIDGET);
    connect(Message, SIGNAL(returnPressed()), this, SLOT(send()));
    new QToolButton(QPixmap((const char **)toworksheet_xpm),
                    tr("Edit message in memo"),
                    tr("Edit message in memo"),
                    this, SLOT(memo()),
                    toolbar);
    toolbar->setStretchableWidget(Message);
    new QToolButton(QPixmap((const char **)return_xpm),
                    tr("Send alert"),
                    tr("Send alert"),
                    this, SLOT(send()),
                    toolbar);

    connect(&Timer, SIGNAL(timeout()), this, SLOT(poll()));
    Timer.start(TIMEOUT*1000);

    Alerts = new toListView(this);
    Alerts->addColumn(tr("Time"));
    Alerts->addColumn(tr("Name"));
    Alerts->addColumn(tr("Message"));

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(windowActivated(QWidget *)),
            this, SLOT(windowActivated(QWidget *)));

    State = Started;
    try
    {
        toThread *thread = new toThread(new pollTask(*this));
        thread->start();
    }
    catch (...)
    {
        toStatusMessage(tr("Failed to start polling thread, try closing some other tools and restart Alert Messenger"));
    }

    setFocusProxy(Message);
}

void toAlert::windowActivated(QWidget *widget)
{
    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QPopupMenu(this);
            ToolMenu->insertItem(tr("&Add name"), Registered, SLOT(setFocus()), toKeySequence(tr("Alt+R", "Alert|Add Name")));
            ToolMenu->insertItem(QPixmap((const char **)trash_xpm), tr("&Remove name"),
                                 this, SLOT(remove
                                            (void)), toKeySequence(tr("Ctrl+Backspace", "Alert|Remove name")));
            ToolMenu->insertSeparator();
            ToolMenu->insertItem(tr("Edit &name"), Name, SLOT(setFocus()), toKeySequence(tr("Alt+N", "Alert|Edit name")));
            ToolMenu->insertItem(tr("Edit &message"), Message, SLOT(setFocus()), toKeySequence(tr("Alt+M", "Alert|Message")));
            ToolMenu->insertItem(QPixmap((const char **)toworksheet_xpm), tr("&Message in memo..."),
                                 this, SLOT(memo(void)), toKeySequence(tr("Ctrl+M", "Alert|Message in memo")));
            ToolMenu->insertItem(QPixmap((const char **)return_xpm), tr("&Send alert"),
                                 this, SLOT(send(void)), toKeySequence(tr("Ctrl+Return", "Alert|Send alert")));

            toMainWidget()->menuBar()->insertItem(tr("&Alert"), ToolMenu, -1, toToolMenuIndex());
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

toAlert::~toAlert()
{
    try
    {
        Lock.lock();
        State = Quit;
        while (State != Done)
        {
            Lock.unlock();
            Semaphore.down();
            Lock.lock();
        }
        Lock.unlock();
        AlertTool.closeWindow(connection());
    }
    TOCATCH
}

static toSQL SQLRegister("toAlert:Register",
                         "BEGIN SYS.DBMS_ALERT.REGISTER(:name<char[4000]>); END;",
                         "Register an alert name, must have same binds");
static toSQL SQLRemove("toAlert:Remove",
                       "BEGIN SYS.DBMS_ALERT.REMOVE(:name<char[4000]>); END;",
                       "Remove a registered an alert name, must have same binds");
static toSQL SQLRemoveAll("toAlert:RemoveAll",
                          "BEGIN SYS.DBMS_ALERT.REMOVEALL; END;",
                          "Remove all registered alerts");
static toSQL SQLPoll("toAlert:PollAlert",
                     "BEGIN\n"
                     "  SYS.DBMS_ALERT.WAITANY(:name<char[4000],out>,:msg<char[4000],out>,\n"
                     "                         :stat<in,out>,:tim<char[10],in>);\n"
                     "END;",
                     "Poll for alert, must have same binds");
static toSQL SQLSignal("toAlert:Signal",
                       "BEGIN\n"
                       "  SYS.DBMS_ALERT.SIGNAL(:name<char[4000],in>,:msg<char[4000],in>);\n"
                       "END;",
                       "Signal alert, must have same binds");

void toAlert::pollTask::run(void)
{
    Parent.Lock.lock();
    bool fatal = false;
    while (Parent.State != Quit && !fatal)
    {
        Parent.Lock.unlock();
        try
        {
            {
                toLocker lock (Parent.Lock)
                    ;
                fatal = true;
                {
                    for (std::list<QString>::iterator i = Parent.AddNames.begin();
                            i != Parent.AddNames.end();
                            i++)
                    {
                        Parent.Names.insert(Parent.Names.end(), *i);
                        Parent.Connection.execute(SQLRegister, *i);
                    }
                }
                Parent.AddNames.clear();
                for (std::list<QString>::iterator i = Parent.DelNames.begin();
                        i != Parent.DelNames.end();
                        i++)
                {
                    std::list<QString>::iterator j = std::find(Parent.Names.begin(),
                                                     Parent.Names.end(),
                                                     *i);
                    if (j != Parent.Names.end())
                    {
                        Parent.Names.erase(j);
                        Parent.Connection.execute(SQLRemove, *i);
                    }
                }
                Parent.DelNames.clear();
                fatal = false;
            }

            {
                toLocker lock (Parent.Lock)
                    ;
                std::list<QString>::iterator i = Parent.SendAlerts.begin();
                std::list<QString>::iterator j = Parent.SendMessages.begin();
                while (i != Parent.SendAlerts.end() && j != Parent.SendMessages.end())
                {
                    Parent.Connection.execute(SQLSignal, *i, *j);
                    i++;
                    j++;
                }
                Parent.SendAlerts.clear();
                Parent.SendMessages.clear();
                Parent.Connection.commit();
            }

            Parent.Lock.lock();
            if (Parent.Names.size())
            {
                Parent.Lock.unlock();
                toQuery query(Parent.Connection, SQLPoll, QString::number(TIMEOUT));
                QString name = query.readValue();
                QString msg = query.readValue();
                if (query.readValue().toInt() == 0)
                {
                    toLocker lock (Parent.Lock)
                        ;
                    Parent.NewAlerts.insert(Parent.NewAlerts.end(), name);
                    Parent.NewMessages.insert(Parent.NewMessages.end(), msg);
                }
            }
            else
            {
                Parent.Lock.unlock();
#ifndef WIN32

                sleep(TIMEOUT);
#else

                Sleep(TIMEOUT*1000);
#endif

            }
        }
        catch (const QString &str)
        {
            Parent.Lock.lock();
            Parent.Error.sprintf("Exception in alert polling:\n%s", (const char *)str.latin1());
            fprintf(stderr, "%s\n", (const char *)Parent.Error);
            Parent.Lock.unlock();
        }
        catch (...)
        {
            Parent.Lock.lock();
            Parent.Error.sprintf("Unexpected exception in alert in polling.");
            fprintf(stderr, "%s\n", (const char *)Parent.Error);
            Parent.Lock.unlock();
        }

        Parent.Lock.lock();
    }
    if (Parent.Names.size() > 0)
        try
        {
            Parent.Connection.execute(SQLRemoveAll);
        }
        catch (...)
        {}
    Parent.State = Done;
    Parent.Semaphore.up();
    Parent.Lock.unlock();
}

void toAlert::poll(void)
{
    try
    {
        toLocker lock (Lock)
            ;
        if (Error)
        {
            toStatusMessage(Error);
            Error = QString::null;
        }
        std::list<QString>::iterator i = NewAlerts.begin();
        std::list<QString>::iterator j = NewMessages.begin();
        while (i != NewAlerts.end() && j != NewMessages.end())
        {
            QListViewItem *item = new toResultViewMLine(Alerts, NULL, toNow(connection()));
            item->setText(1, *i);
            item->setText(2, *j);
            i++;
            j++;
        }
        NewAlerts.clear();
        NewMessages.clear();
    }
    TOCATCH
}

void toAlert::send(void)
{
    if (!Name->text().isEmpty() && !Message->text().isEmpty())
    {
        toLocker lock (Lock)
            ;
        SendAlerts.insert(SendAlerts.end(), Name->text());
        SendMessages.insert(SendMessages.end(), Message->text());
        Message->clear();
    }
}

void toAlert::memo(void)
{
    toMemoEditor *memo = new toMemoEditor(this, Message->text(), 0, 0);
    connect(memo, SIGNAL(changeData(int, int, const QString &)),
            this, SLOT(changeMessage(int, int, const QString &)));
}

void toAlert::changeMessage(int, int, const QString &str)
{
    Message->setText(str);
}

void toAlert::remove
    (void)
{
    toLocker lock (Lock)
        ;
    QString name = Registered->currentText();
    if (!name.isEmpty())
    {
        std::list<QString>::iterator i = std::find(AddNames.begin(), AddNames.end(), name);
        if (i != AddNames.end())
            AddNames.erase(i);
        i = std::find(Names.begin(), Names.end(), name);
        if (i != Names.end())
        {
            if (std::find(DelNames.begin(), DelNames.end(), name) == DelNames.end())
                DelNames.insert(DelNames.end(), name);
        }
    }

    if (Registered->count() > 0)
        Registered->removeItem(Registered->currentItem());
    if (Registered->count() > 0)
        Registered->setCurrentItem(0);
}

void toAlert::add
    (void)
{
    Registered->lineEdit()->selectAll();
    QString name = Registered->currentText();
    if (!name.isEmpty())
    {
        std::list<QString>::iterator i = std::find(DelNames.begin(), DelNames.end(), name);
        if (i == DelNames.end())
        {
            i = std::find(Names.begin(), Names.end(), name);
            if (i == Names.end())
            {
                i = std::find(AddNames.begin(), AddNames.end(), name);
                if (i == AddNames.end())
                {
                    AddNames.insert(AddNames.end(), name);
                }
            }
        }
        else
        {
            DelNames.erase(i);
        }
    }

    for (int i = 0;i < Registered->count();i++)
        if (Registered->text(i) == name)
            return ;

    Registered->insertItem(name);
    Name->setText(name);
}
