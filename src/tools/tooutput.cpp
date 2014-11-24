
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

#include "tools/tooutput.h"
#include "ui_tooutputsettingui.h"
#include "core/toresultview.h"
#include "core/totimer.h"
#include "core/toglobalevent.h"
#include "core/toconfiguration_new.h"
#include "core/utils.h"
#include "editor/toscintilla.h"

#include <map>

#include <QWidget>

#include "icons/eraselog.xpm"
#include "icons/offline.xpm"
#include "icons/online.xpm"
#include "icons/refresh.xpm"
#include "icons/tooutput.xpm"

QVariant ToConfiguration::Output::defaultValue(int option) const
{
	switch(option)
	{
	case PollingInterval:  return QVariant(QString("10 seconds"));
	case SourceTypeInt:          return QVariant((int)0);
	case LogUser:          return QVariant(QString("ULOG"));
	default:
		Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Output un-registered enum value: %1").arg(option)));
		return QVariant();
	}
};

class toOutputSetting
	: public QWidget
	, Ui::toOutputSettingUI
	, public toSettingTab
{
    toTool *Tool;
public:
    toOutputSetting(toTool *tool, QWidget* parent = 0, const char* name = 0)
	: QWidget(parent)
    , toSettingTab("output.html")
	, Tool(tool)
    {
    	using namespace ToConfiguration;
    	setupUi(this);
        if (name)
            setObjectName(name);

        Utils::toRefreshCreate(this,
        		"toRefreshCreate",
        		toConfigurationNewSingle::Instance().option(Output::PollingInterval).toString(),
        		PollingInterval);

        toSettingTab::loadSettings(this);
    }
    virtual void saveSetting(void)
    {
    	toSettingTab::saveSettings(this);
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

    virtual toToolWidget *toolWindow(QWidget *parent, toConnection &connection)
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
            toToolWidget* window = new toLogOutput(parent, connection);
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
        return new toOutputSetting(this, parent);
    }
private:
    static ToConfiguration::Output s_outputConf;
};

ToConfiguration::Output toOutputTool::s_outputConf;

static toOutputTool OutputTool;

toOutput::toOutput(QWidget *main, toConnection &connection, bool enabled)
    : toToolWidget(OutputTool, "output.html", main, connection, "toOutput")
{

    Toolbar = Utils::toAllocBar(this, tr("SQL Output"));
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


    Refresh = Utils::toRefreshCreate(Toolbar,
                                     "Refresh",
                                     toConfigurationNewSingle::Instance().option(ToConfiguration::Output::PollingInterval).toString());
    Toolbar->addWidget(Refresh);
    connect(Refresh,
            SIGNAL(activated(const QString &)),
            this,
            SLOT(changeRefresh(const QString &)));

    Toolbar->addWidget(new Utils::toSpacer());

    Output = new toMarkedEditor(this);
    layout()->addWidget(Output);

    ToolMenu = NULL;
    // connect(toMainWidget()->workspace(),
    //         SIGNAL(subWindowActivated(QMdiSubWindow *)),
    //         this,
    //         SLOT(windowActivated(QMdiSubWindow *)));

    try
    {
        connect(timer(), SIGNAL(timeout(void)), this, SLOT(refresh(void)));
        Utils::toRefreshParse(timer(), toConfigurationNewSingle::Instance().option(ToConfiguration::Output::PollingInterval).toString());
    }
    TOCATCH;

    if (enabled)
        disable(false);

    setFocusProxy(Output);
}


void toOutput::slotWindowActivated(toToolWidget* widget)
{
	if (!widget)
		return;
	if (widget == this)
	{
		if (!ToolMenu)
		{
			ToolMenu = new QMenu(tr("&Output"), this);

			ToolMenu->addAction(refreshAct);

			ToolMenu->addSeparator();

			ToolMenu->addAction(enableAct);
			ToolMenu->addAction(clearAct);

			toGlobalEventSingle::Instance().addCustomMenu(ToolMenu);
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
//        if (dis)
//            connection().allExecute(SQLDisable);
//        else
//            connection().allExecute(SQLEnable);
        QString str;

        if (dis)
        	str = toSQL::string(SQLDisable, connection());
        else
        	str = toSQL::string(SQLEnable, connection());
        connection().setInit("OUTPUT", str);
//        if (dis)
//            connection().delInit("OUTPUT");
//        else
//            connection().setInit("OUTPUT", str);
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
        Utils::toStatusMessage(tr("Couldn't enable/disable output for session"));
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
                      "    :lines<char[1000],out> := :lines<char[1000],out> || chr(10);\n"
                      "END;",
                      "Get lines from SQL Output, must use same bindings");

void toOutput::poll()
{
    try
    {
        bool any;
        do
        {
        	toConnectionSubLoan c(connection());
        	toQuery query(c, SQLLines, toQueryParams());  // TODO FETCH FROM ALL THE CONNECTIONS IN THE POOL - impossible

            any = false;
            while (!query.eof())
            {
                QString line = (QString)query.readValue();
                int status = query.readValue().toInt();
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
        Utils::toRefreshParse(timer(), str);
    }
    TOCATCH;
}

bool toOutput::enabled(void)
{
    return !enableAct->isChecked();
}

void toOutput::insertLine(const QString &str)
{
    Output->sciEditor()->append(str);
    Output->sciEditor()->setCursorPosition((*Output)->lines(), 0);
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
    , customizedLogSQL(SQLLog)
{
    Type = new QComboBox(Toolbar);
    Type->addItem(tr("SQL Output"));
    Type->addItem(tr("Log4PL/SQL"));
    Type->setCurrentIndex(toConfigurationNewSingle::Instance().option(ToConfiguration::Output::SourceTypeInt).toInt());
    Toolbar->addWidget(Type);
    connect(Type, SIGNAL(activated(int)), this, SLOT(changeType()));

    Log = new toResultView(false, false, this);
    changeType();
}

void toLogOutput::refresh(void)
{
    if (Type->currentIndex() == 1)
    {
    	toConnection &con = connection();

        Log->removeSQL();

        // Ugly woodo to replace %1 with the configured logging table name
        customizedLogSQL.updateSQL(
        		SQLLog.name(), // "toLogOutput:Poll"
        		toSQL::string(SQLLog.name(), connection()).arg( toConfigurationNewSingle::Instance().option(ToConfiguration::Output::LogUser).toString()),
        		toSQL::description(SQLLog.name()),
        		connection().version(),
        		connection().provider(),
                true);
        Log->setSQL(customizedLogSQL);
        Log->refresh();
    }
    toOutput::refresh();
}

void toLogOutput::changeType(void)
{
    if (Type->currentIndex() == 1)
    {
        Output->hide();
        Log->show();
        refresh();
    }
    else
    {
        Output->show();
        Log->hide();
    }
}
