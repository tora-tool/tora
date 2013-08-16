
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

#include "tools/toalert.h"

#include "core/utils.h"
#include "core/tologger.h"

#include "core/toconf.h"
#include "core/tomainwindow.h"
#include "core/tomemoeditor.h"
#include "core/toresultview.h"
#include "core/toconnection.h"
#include "core/toquery.h"

#include <algorithm>
#include <stdio.h>

#include <QtCore/QMutexLocker>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMenuBar>
#include <QtGui/QToolButton>
#include <QtGui/QMdiArea>
#include <QtGui/QPixmap>

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
        return const_cast<const char**>(toalert_xpm);
    }
public:
    toAlertTool()
        : toTool(330, "Alert Messenger")
    { }
    virtual const char *menuItem()
    {
        return "Alert Messenger";
    }
    virtual toToolWidget *toolWindow(QWidget *parent, toConnection &connection)
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
    virtual void closeWindow(toConnection &connection)
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
    : toToolWidget(AlertTool, "alert.html", main, connection, "toAlert")
    , Connection(connection)
{
    QToolBar *toolbar = Utils::toAllocBar(this, tr("Alert Messenger"));
    layout()->addWidget(toolbar);

    QString def;
    try
    {
        toQList lst = toQuery::readQuery(connection, SQLHost, toQueryParams());
        def += Utils::toShift(lst);
        if (!def.isEmpty())
            def += QString::fromLatin1(":");
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
    def += connection.user();

    toolbar->addWidget(
        new QLabel(tr("Registered") + " ", toolbar));

    Registered = new QComboBox(toolbar);
    Registered->addItem(def);
    Registered->setEditable(true);
    Registered->setDuplicatesEnabled(false);
    Registered->setCurrentIndex(0);
    connect(Registered, SIGNAL(activated(int)), this, SLOT(add()));
    toolbar->addWidget(Registered);

    AddNames.insert(AddNames.end(), def);

    addAct =
        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(commit_xpm))),
                           tr("Register current"),
                           this,
                           SLOT(add()));
    addAct->setShortcut(Qt::ALT + Qt::Key_R);

    removeAct =
        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(trash_xpm))),
                           tr("Remove registered"),
                           this,
                           SLOT(remove()));
    removeAct->setShortcut(Qt::CTRL + Qt::Key_Backspace);

    toolbar->addSeparator();

    toolbar->addWidget(
        new QLabel(tr("Name") + " ", toolbar));

    toolbar->addWidget(
        Name = new QLineEdit(toolbar));
    Name->setText(def);
    connect(Name, SIGNAL(returnPressed()), this, SLOT(send()));

    toolbar->addWidget(
        new QLabel(tr("Message") + " ", toolbar));

    Message = new QLineEdit(toolbar);
    connect(Message, SIGNAL(returnPressed()), this, SLOT(send()));
    Message->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                       QSizePolicy::Minimum));

    toolbar->addWidget(Message);

    memoEditAct =
        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(toworksheet_xpm))),
                           tr("Edit message in memo"),
                           this,
                           SLOT(memo()));
    memoEditAct->setShortcut(Qt::CTRL + Qt::Key_M);

    sendAct =
        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(return_xpm))),
                           tr("Send alert"),
                           this,
                           SLOT(send()));
    sendAct->setShortcut(Qt::CTRL + Qt::Key_Return);

    connect(&Timer, SIGNAL(timeout()), this, SLOT(poll()));
    Timer.start(TIMEOUT * 1000);

    Alerts = new toListView(this);
    Alerts->addColumn(tr("Time"));
    Alerts->addColumn(tr("Name"));
    Alerts->addColumn(tr("Message"));
    layout()->addWidget(Alerts);

    ToolMenu = NULL;

    State = Started;
    try
    {
        toThread *thread = new toThread(new pollTask(*this));
        thread->start();
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
        Utils::toStatusMessage(tr("Failed to start polling thread, try closing "
                           "some other tools and restart Alert Messenger"));
        State = Done;
    }

    setFocusProxy(Message);
}

void toAlert::slotWindowActivated(toToolWidget *widget)
{
    if (!widget)
        return;

    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Alert"), this);

            ToolMenu->addAction(addAct);
            ToolMenu->addAction(removeAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(memoEditAct);
            ToolMenu->addAction(sendAct);

            toMainWidget()->addCustomMenu(ToolMenu);
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
}

void toAlert::closeEvent(QCloseEvent *event)
{
    if(State == Done)
    {
        AlertTool.closeWindow(connection());
        return;
    }

    State = Quit;
    while (State != Done)
    {
        Semaphore.down();
    }
    AlertTool.closeWindow(connection());
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
                     "                         :stat<int,out>,:tim<char[10],in>);\n"
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
    bool namesEmpty;
    while (Parent.State != Quit && !fatal)
    {
        Parent.Lock.unlock();
        try
        {
            {
                QMutexLocker lock (&Parent.Lock)
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
                QMutexLocker lock (&Parent.Lock)
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

            {
                QMutexLocker lock (&Parent.Lock);
                namesEmpty = Parent.Names.empty();
            }

            if (!namesEmpty)
            {
                toQuery query(Parent.Connection, SQLPoll, QString::number(TIMEOUT));
                QString name = query.readValue();
                QString msg = query.readValue();
                if (query.readValue().toInt() == 0)
                {
                    QMutexLocker lock (&Parent.Lock)
                    ;
                    Parent.NewAlerts.insert(Parent.NewAlerts.end(), name);
                    Parent.NewMessages.insert(Parent.NewMessages.end(), msg);
                }
            }
            else
            {
                toThread::sleep(TIMEOUT);
            }
        }
        catch (const QString &str)
        {
            Parent.Lock.lock();
            Parent.Error.sprintf("Exception in alert polling:\n%s", (const char *)str.toLatin1());
            fprintf(stderr, "%s\n", Parent.Error.toAscii().constData());
            fatal = true;
            Parent.Lock.unlock();
        }
        catch (...)
        {
            Parent.Lock.lock();
            Parent.Error.sprintf("Unexpected exception in alert in polling.");
            fprintf(stderr, "%s\n", Parent.Error.toAscii().constData());
            fatal = true;
            Parent.Lock.unlock();
        }

        Parent.Lock.lock();
    }
    if (!Parent.Names.empty())
        try
        {
            Parent.Connection.execute(SQLRemoveAll);
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
            fatal = true;
        }
    Parent.State = Done;
    Parent.Semaphore.up();
    Parent.Lock.unlock();
}

void toAlert::poll(void)
{
    try
    {
        QMutexLocker lock (&Lock)
        ;
        if (!Error.isNull())
        {
            Utils::toStatusMessage(Error);
            Error = QString::null;
        }
        std::list<QString>::iterator i = NewAlerts.begin();
        std::list<QString>::iterator j = NewMessages.begin();
        while (i != NewAlerts.end() && j != NewMessages.end())
        {
            toTreeWidgetItem *item = new toResultViewMLine(Alerts, NULL, Utils::toNow(connection()));
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
        QMutexLocker lock (&Lock);
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
    QMutexLocker lock (&Lock);

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
        Registered->removeItem(Registered->currentIndex());
    if (Registered->count() > 0)
        Registered->setCurrentIndex(0);
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

    for (int i = 0; i < Registered->count(); i++)
        if (Registered->itemText(i) == name)
            return ;

    Registered->addItem(name);
    Name->setText(name);
}
