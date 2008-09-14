
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

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tomarkedtext.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"

#include <map>

#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qstring.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <QMdiArea>

#include <QPixmap>
#include <QGroupBox>
#include <QVBoxLayout>

#include "tooutput.h"

#include "icons/eraselog.xpm"
#include "icons/offline.xpm"
#include "icons/online.xpm"
#include "icons/refresh.xpm"
#include "icons/tooutput.xpm"

// #define CONF_POLLING     "Refresh"
// #define DEFAULT_POLLING  "10 seconds"
//
// #define CONF_LOG_TYPE  "Type"
// #define DEFAULT_LOG_TYPE "0"
//
// #define CONF_LOG_USER  "LogUser"
// #define DEFAULT_LOG_USER "ULOG"

class toOutputPrefs : public QGroupBox, public toSettingTab
{
    QComboBox *AutoPolling;
    QComboBox *Type;
    QLineEdit *User;
    toTool *Tool;

public:
    toOutputPrefs(toTool *tool, QWidget* parent = 0, const char* name = 0)
            : QGroupBox(parent),
            toSettingTab("output.html"), Tool(tool)
    {

        if (name)
            setObjectName(name);

        QVBoxLayout *vbox = new QVBoxLayout;

        setLayout(vbox);

        setTitle(qApp->translate("toOutputPrefs", "SQL Output"));

        QLabel *label = new QLabel(qApp->translate("toOutputPrefs",
                                   "&Polling timeout"),
                                   this);
        label->setGeometry(QRect(20, 30, 100, 20));
        label->setToolTip(qApp->translate("toOutputPrefs",
                                          "Time between trying to poll for output."));
        vbox->addWidget(label);

        AutoPolling = toRefreshCreate(
                          this,
                          TO_TOOLBAR_WIDGET_NAME,
                          toConfigurationSingle::Instance().polling());
        label->setBuddy(AutoPolling);
        vbox->addWidget(AutoPolling);

        label = new QLabel(qApp->translate("toOutputPrefs",
                                           "Default &source"),
                           this);
        vbox->addWidget(label);

        Type = new QComboBox(this);
        Type->addItem(qApp->translate("toLogOutput", "SQL Output"));
        Type->addItem(qApp->translate("toLogOutput", "Log4PL/SQL"));
        Type->setCurrentIndex(toConfigurationSingle::Instance().logType());
        label->setBuddy(Type);
        vbox->addWidget(Type);

        label = new QLabel(qApp->translate("toOutputPrefs",
                                           "Log4PL/SQL &User"),
                           this);
        vbox->addWidget(label);

        User = new QLineEdit(toConfigurationSingle::Instance().logUser(),
                             this);
        label->setBuddy(User);
        vbox->addWidget(User);

        vbox->addStretch();
    }
    virtual void saveSetting(void)
    {
        toConfigurationSingle::Instance().setPolling(AutoPolling->currentText());
        toConfigurationSingle::Instance().setLogType(Type->currentIndex());
        toConfigurationSingle::Instance().setLogUser(User->text());
    }
};

class toOutputTool : public toTool
{
protected:
    std::map<toConnection *, QWidget *> Windows;

    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(tooutput_xpm);
    }

public:
    toOutputTool()
            : toTool(340, "SQL Output")
    {
    }

    virtual const char *menuItem()
    {
        return "SQL Output";
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
            QWidget *window = new toLogOutput(parent, connection);
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

    virtual QWidget *configurationTab(QWidget *parent)
    {
        return new toOutputPrefs(this, parent);
    }
};

static toOutputTool OutputTool;

toOutput::toOutput(QWidget *main, toConnection &connection, bool enabled)
        : toToolWidget(OutputTool, "output.html", main, connection, "toOutput")
{

    Toolbar = toAllocBar(this, tr("SQL Output"));
    layout()->addWidget(Toolbar);

    refreshAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                             tr("Poll for output now"),
                             this);
    refreshAct->setShortcut(QKeySequence::Refresh);
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refresh(void)));
    Toolbar->addAction(refreshAct);

    Toolbar->addSeparator();

    enableAct = new QAction(tr("Output enabled"), this);
    enableAct->setShortcut(Qt::Key_F4);
    enableAct->setCheckable(true);
    QIcon iconset;
    iconset.addPixmap(QPixmap(const_cast<const char**>(online_xpm)),
                      QIcon::Normal,
                      QIcon::Off);
    iconset.addPixmap(QPixmap(const_cast<const char**>(offline_xpm)),
                      QIcon::Normal,
                      QIcon::On);
    enableAct->setIcon(iconset);
    connect(enableAct, SIGNAL(triggered()), this, SLOT(toggleMenu(void)));

    clearAct = new QAction(QIcon(QPixmap(const_cast<const char**>(eraselog_xpm))),
                           tr("Clear output"),
                           this);
    clearAct->setShortcut(Qt::CTRL + Qt::Key_Backspace);
    connect(clearAct, SIGNAL(triggered()), this, SLOT(clear(void)));
    Toolbar->addAction(clearAct);

    Toolbar->addSeparator();

    Toolbar->addWidget(
        new QLabel(tr("Refresh") + " ", Toolbar));


    Refresh = toRefreshCreate(Toolbar,
                              TO_TOOLBAR_WIDGET_NAME,
                              toConfigurationSingle::Instance().polling());
    Toolbar->addWidget(Refresh);
    connect(Refresh,
            SIGNAL(activated(const QString &)),
            this,
            SLOT(changeRefresh(const QString &)));

    Toolbar->addWidget(new toSpacer());

    Output = new toMarkedText(this);
    layout()->addWidget(Output);

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(),
            SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this,
            SLOT(windowActivated(QMdiSubWindow *)));

    try
    {
        connect(timer(), SIGNAL(timeout(void)), this, SLOT(refresh(void)));
        toRefreshParse(timer(), toConfigurationSingle::Instance().polling());
    }
    TOCATCH;

    if (enabled)
        disable(false);

    setFocusProxy(Output);
}


void toOutput::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;
    if (widget->widget() == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Output"), this);

            ToolMenu->addAction(refreshAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(enableAct);
            ToolMenu->addAction(clearAct);

            toMainWidget()->addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

static toSQL SQLEnable("toOutput:Enable",
                       "BEGIN\n"
                       "    SYS.DBMS_OUTPUT.ENABLE;\n"
                       "END;",
                       "Enable output collection");
static toSQL SQLDisable("toOutput:Disable",
                        "BEGIN\n"
                        "    SYS.DBMS_OUTPUT.DISABLE;\n"
                        "END;",
                        "Disable output collection");

void toOutput::toggleMenu()
{
    enableAct->setChecked(!enableAct->isChecked());
}

void toOutput::disable(bool dis)
{
    try
    {
        if (dis)
            connection().allExecute(SQLDisable);
        else
            connection().allExecute(SQLEnable);
        QString str = toSQL::string(SQLEnable, connection());
        if (dis)
            connection().delInit(str);
        else
            connection().addInit(str);
    }
    catch (...)
    {
        toStatusMessage(tr("Couldn't enable/disable output for session"));
    }
}

toOutput::~toOutput()
{
}


void toOutput::closeEvent(QCloseEvent *event)
{
    try
    {
        disable(true);
        OutputTool.closeWindow(connection());
    }
    TOCATCH;

    event->accept();
}


static toSQL SQLLines("toOutput:Poll",
                      "BEGIN\n"
                      "    SYS.DBMS_OUTPUT.GET_LINE(:lines<char[1000],out>,\n"
                      "                             :stat<int,out>);\n"
                      "    :lines := :lines || chr(10);\n"
                      "END;",
                      "Get lines from SQL Output, must use same bindings");

void toOutput::poll()
{
    try
    {
        bool any;
        do
        {
            toQList params;
            toQuery query(connection(), toQuery::All, SQLLines, params);

            any = false;
            while (!query.eof())
            {
                QString line = query.readValueNull();
                int status = query.readValueNull().toInt();
                if (status == 0)
                {
                    any = true;
                    if (!line.isNull()) insertLine(line);
                }
            }
        }
        while (any);
    }
    TOCATCH;
}

void toOutput::refresh(void)
{
    poll();
}

void toOutput::clear(void)
{
    Output->clear();
}

void toOutput::changeRefresh(const QString &str)
{
    try
    {
        toRefreshParse(timer(), str);
    }
    TOCATCH;
}

bool toOutput::enabled(void)
{
    return !enableAct->isChecked();
}

static toSQL SQLLog("toLogOutput:Poll",
                    "SELECT LDATE||'.'||to_char(mod(LHSECS,100),'09') \"Timestamp\",\n"
                    "       decode(llevel,1,'OFF',\n"
                    "                     2,'FATAL',\n"
                    "                     3,'ERROR',\n"
                    "                     4,'WARNING',\n"
                    "                     5,'INFO',\n"
                    "                     6,'DEBUG',\n"
                    "                     7,'ALL' ,\n"
                    "                     'UNDEFINED') \"Level\",\n"
                    "       LUSER \"User\",\n"
                    "       LSECTION \"Section\",\n"
                    "       LTEXTE \"Text\"\n"
                    "  from %1.tlog order by id desc\n",
                    "Poll data from PL/SQL log table");

toLogOutput::toLogOutput(QWidget *parent, toConnection &connection)
        : toOutput(parent, connection)
{
    Type = new QComboBox(toolBar());
    Type->addItem(tr("SQL Output"));
    Type->addItem(tr("Log4PL/SQL"));
    Type->setCurrentIndex(toConfigurationSingle::Instance().logType());
    toolBar()->addWidget(Type);
    connect(Type, SIGNAL(activated(int)), this, SLOT(changeType()));

    Log = new toResultView(false, false, this);
    changeType();
}

void toLogOutput::refresh(void)
{
    if (Type->currentIndex() == 1)
    {
        Log->setSQL(QString::null);
        Log->query(SQLLog(connection()).arg(toConfigurationSingle::Instance().logUser()));
    }
    toOutput::refresh();
}

void toLogOutput::changeType(void)
{
    if (Type->currentIndex() == 1)
    {
        output()->hide();
        Log->show();
        refresh();
    }
    else
    {
        output()->show();
        Log->hide();
    }
}
