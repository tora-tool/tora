
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

#include <QtGui/QKeySequence>
#include <QtWidgets/QAction>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QLabel>

#include "core/toglobalconfiguration.h"
#include "core/utils.h"
#include "core/tosql.h"
#include "core/tologger.h"
#include "core/toqvalue.h"
#include "core/toquery.h"
#include "core/toconfiguration.h"

#include "widgets/tooutput.h"
#include "widgets/torefreshcombo.h"

#include "widgets/toscintilla.h"

#include "icons/eraselog.xpm"
#include "icons/refresh.xpm"
#include "icons/tooutput.xpm"

toOutputWidget::toOutputWidget(QWidget *parent)
    : QWidget(parent)
{
    using namespace ToConfiguration;

    // have to set the basic layout for widgets. It's requested later
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    Toolbar = Utils::toAllocBar(this, tr("SQL Output"));
    layout()->addWidget(Toolbar);

    refreshAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                             tr("Poll for output now"),
                             this);
    refreshAct->setShortcut(QKeySequence::Refresh);
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refresh(void)));
    Toolbar->addAction(refreshAct);

    Toolbar->addSeparator();

    clearAct = new QAction(QIcon(QPixmap(const_cast<const char**>(eraselog_xpm))),
                           tr("Clear output"),
                           this);
    clearAct->setShortcut(Qt::CTRL + Qt::Key_Backspace);
    connect(clearAct, SIGNAL(triggered()), this, SLOT(clear(void)));
    Toolbar->addAction(clearAct);

    Toolbar->addSeparator();

    Toolbar->addWidget(new QLabel(tr("Refresh") + " ", Toolbar));

    Refresh = new toRefreshCombo(Toolbar, toConfigurationNewSingle::Instance().option(Global::OutputPollingInterval).toString());
    Toolbar->addWidget(Refresh);
    connect(Refresh->timer(), SIGNAL(timeout(void)), this, SLOT(refresh(void)));

    Toolbar->addWidget(new Utils::toSpacer());

    Output = new toScintilla(this);
    Output->setReadOnly(true);
    layout()->addWidget(Output);

    setFocusProxy(Output);
}

static toSQL SQLEnable("toOutputWidget:Enable",
                       "BEGIN\n"
                       "    SYS.DBMS_OUTPUT.ENABLE;\n"
                       "END;",
                       "Enable output collection");
static toSQL SQLDisable("toOutputWidget:Disable",
                        "BEGIN\n"
                        "    SYS.DBMS_OUTPUT.DISABLE;\n"
                        "END;",
                        "Disable output collection");

toOutputWidget::~toOutputWidget()
{
}

void toOutputWidget::setConnection(QSharedPointer<toConnectionSubLoan> &conn)
{
    LockedConnection = conn;
}

void toOutputWidget::closeEvent(QCloseEvent *event)
{
    try
    {
        if (LockedConnection.data())
        {
            LockedConnection.data()->execute(SQLDisable);
            LockedConnection.data()->delInit("OUTPUT");
        }
    }
    TOCATCH;
    QWidget::closeEvent(event);
}

static toSQL SQLLines("toOutputWidget:Poll",
                      "BEGIN\n"
                      "    SYS.DBMS_OUTPUT.GET_LINE(:lines<char[1000],out>,\n"
                      "                             :stat<int,out>);\n"
                      "    :lines<char[1000],out> := :lines<char[1000],out> || chr(10);\n"
                      "END;",
                      "Get lines from SQL Output, must use same bindings");

void toOutputWidget::poll()
{
    if (!LockedConnection)
        return;
#if 0
    try
    {
        toQuery query(*LockedConnection.data(), SQLLines, toQueryParams());
        int status;
        while (!query.eof())
        {
            QString line = (QString)query.readValue();
            status = query.readValue().toInt();
            if (status == 0)
            {
                Output->append(line);
                Output->setCursorPosition(Output->lines(), 0);
                Output->ensureLineVisible(Output->lines());
            }
        }
        while (status == 0);
    }
    TOCATCH;
#endif

    try
    {
        bool any;
        do
        {
            toQuery query(*LockedConnection.data(), SQLLines, toQueryParams());  // TODO FETCH FROM ALL THE CONNECTIONS IN THE POOL - impossible

            any = false;
            while (!query.eof())
            {
                QString line = (QString)query.readValue();
                int status = query.readValue().toInt();
                if (status == 0)
                {
                    any = true;
                    if (!line.isNull())
                    {
                        Output->append(line);
                        Output->setCursorPosition(Output->lines(), 0);
                        Output->ensureLineVisible(Output->lines());

                    }
                }
            }
        }
        while (any);
    }
    TOCATCH;
}

void toOutputWidget::refresh(void)
{
    poll();
}

void toOutputWidget::clear(void)
{
    Output->clear();
}

void toOutputWidget::setEnabled(bool enabled)
{
    if (enabled)
    {
        if (LockedConnection.data())
        {
            LockedConnection.data()->execute(SQLEnable);
            LockedConnection.data()->setInit("OUTPUT", "");
        }
        Refresh->timer()->blockSignals(false);
    } else {
        if (LockedConnection.data())
        {
            LockedConnection.data()->execute(SQLDisable);
            LockedConnection.data()->delInit("OUTPUT");
        }
        LockedConnection.clear();
        Refresh->timer()->blockSignals(true);
    }
}

// TODO addd support to PLSQL LOG (see tooutputsettingui.ui)
#if 0
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

        // replace %1 with the configured logging table name
        QString sql = toSQL::string(SQLLog, connection());
        QString planTableName = toConfigurationNewSingle::Instance().option(ToConfiguration::Output::LogUser).toString();
        QString customizedSQL = sql.arg(planTableName);
        Log->setSQL(customizedSQL);
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

#endif
