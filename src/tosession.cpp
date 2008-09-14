
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

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "toresultbar.h"
#include "toresultcombo.h"
#include "toresultlock.h"
#include "toresultlong.h"
#include "toresultstats.h"
#include "toresultview.h"
#include "tosession.h"
#include "tosgastatement.h"
#include "tosgatrace.h"
#include "tosql.h"
#include "totool.h"
#include "towaitevents.h"
#include "toresulttableview.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qsplitter.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <QMdiArea>

#include <QTabWidget>
#include <QList>
#include <QPixmap>

#include "icons/add.xpm"
#include "icons/clock.xpm"
#include "icons/kill.xpm"
#include "icons/minus.xpm"
#include "icons/noclock.xpm"
#include "icons/refresh.xpm"
#include "icons/tosession.xpm"
#include "icons/filter.xpm"

class toSessionTool : public toTool
{
protected:
    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(tosession_xpm);
    }
public:
    toSessionTool() : toTool(210, "Sessions")
    { }
    virtual const char *menuItem()
    {
        return "Sessions";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        if (toIsOracle(connection) || toIsPostgreSQL(connection))
            return new toSession(parent, connection);

        return NULL;
    }
    virtual bool canHandle(toConnection &conn)
    {
        return toIsOracle(conn) || toIsPostgreSQL(conn);
    }
    virtual void closeWindow(toConnection &connection) {};
};

static toSessionTool SessionTool;

static toSQL SQLConnectInfo("toSession:ConnectInfo",
                            "select authentication_type,osuser,network_service_banner\n"
                            "  from v$session_connect_info where sid = :f1<char[101]>",
                            "Get connection info for a session");

static toSQL SQLLockedObject("toSession:LockedObject",
                             "select b.Object_Name \"Object Name\",\n"
                             "       b.Object_Type \"Type\",\n"
                             "       DECODE(a.locked_mode,0,'None',1,'Null',2,'Row-S',\n"
                             "                            3,'Row-X',4,'Share',5,'S/Row-X',\n"
                             "                            6,'Exclusive',a.Locked_Mode) \"Locked Mode\"\n"
                             "  from v$locked_object a,sys.all_objects b\n"
                             " where a.object_id = b.object_id\n"
                             "   and a.session_id = :f1<char[101]>",
                             "Display info about objects locked by this session");

static toSQL SQLLockedObjectPg(
    "toSession:LockedObject",
    "SELECT c.relname AS \"Name\",\n"
    "       l.locktype,\n"
    "       n.nspname,\n"
    "       l.TRANSACTION,\n"
    "       l.MODE,\n"
    "       l.granted,\n"
    "       l.pid\n"
    "  FROM pg_catalog.pg_locks l,\n"
    "       pg_class c,\n"
    "       pg_namespace n\n"
    " WHERE l.relation = c.OID\n"
    "   AND c.relnamespace = n.OID\n"
    "   AND l.pid = :f1<int>",
    "",
    "",
    "PostgreSQL");

static toSQL SQLOpenCursors("toSession:OpenCursor",
                            "select SQL_Text \"SQL\", Address||':'||Hash_Value \" Address\"\n"
                            "  from v$open_cursor where sid = :f1<char[101]>",
                            "Display open cursors of this session");
static toSQL SQLSessionWait(TO_SESSION_WAIT,
                            "select sysdate,\n"
                            "       cpu*10 \"CPU\",\n"
                            "       parallel*10 \"Parallel execution\",\n"
                            "       filewrite*10 \"DB File Write\",\n"
                            "       writecomplete*10 \"Write Complete\",\n"
                            "       fileread*10 \"DB File Read\",\n"
                            "       singleread*10 \"DB Single File Read\",\n"
                            "       control*10 \"Control File I/O\",\n"
                            "       direct*10 \"Direct I/O\",\n"
                            "       log*10 \"Log file\",\n"
                            "       net*10 \"SQL*Net\",\n"
                            "       (total-parallel-filewrite-writecomplete-fileread-singleread-control-direct-log-net)*10 \"Other\"\n"
                            "  from (select SUM(DECODE(SUBSTR(event,1,2),'PX',time_waited,0))-SUM(DECODE(event,'PX Idle Wait',time_waited,0)) parallel,\n"
                            "               SUM(DECODE(event,'db file parallel write',time_waited,'db file single write',time_waited,0)) filewrite,\n"
                            "               SUM(DECODE(event,'write complete waits',time_waited,NULL)) writecomplete,\n"
                            "               SUM(DECODE(event,'db file parallel read',time_waited,'db file sequential read',time_waited,0)) fileread,\n"
                            "               SUM(DECODE(event,'db file scattered read',time_waited,0)) singleread,\n"
                            "               SUM(DECODE(SUBSTR(event,1,12),'control file',time_waited,0)) control,\n"
                            "               SUM(DECODE(SUBSTR(event,1,11),'direct path',time_waited,0)) direct,\n"
                            "               SUM(DECODE(SUBSTR(event,1,3),'log',time_waited,0)) log,\n"
                            "               SUM(DECODE(SUBSTR(event,1,7),'SQL*Net',time_waited,0))-SUM(DECODE(event,'SQL*Net message from client',time_waited,0)) net,\n"
                            "      SUM(DECODE(event,'PX Idle Wait',0,'SQL*Net message from client',0,time_waited)) total\n"
                            "          from v$session_event where sid in (select b.sid from v$session a,v$session b where a.sid = :f1<char[101]> and a.audsid = b.audsid)),\n"
                            "       (select value*10 cpu from v$sesstat a\n"
                            "         where statistic# = 12 and a.sid in (select b.sid from v$session a,v$session b where a.sid = :f1<char[101]> and a.audsid = b.audsid))",
                            "Used to generate chart for session wait time.");
static toSQL SQLSessionIO(TO_SESSION_IO,
                          "select sysdate,\n"
                          "       sum(block_gets) \"Block gets\",\n"
                          "       sum(consistent_gets) \"Consistent gets\",\n"
                          "       sum(physical_reads) \"Physical reads\",\n"
                          "       sum(block_changes) \"Block changes\",\n"
                          "       sum(consistent_changes) \"Consistent changes\"\n"
                          "  from v$sess_io where sid in (select b.sid from v$session a,v$session b where a.sid = :f1<char[101]> and a.audsid = b.audsid)",
                          "Display chart of session generated I/O");

static toSQL SQLAccessedObjects("toSession:AccessedObjects",
                                "SELECT owner,\n"
                                "       OBJECT,\n"
                                "       TYPE FROM v$access\n"
                                " WHERE sid=:f1<CHAR [101]>\n"
                                " ORDER BY owner,\n"
                                "   OBJECT,\n"
                                "   TYPE",
                                "Which objects are accessed by the current session");

static toSQL SQLSessions("toSession:ListSession",
                         "SELECT a.Sid \"-Id\",\n"
                         "       a.Serial# \"-Serial#\",\n"
                         "       a.SchemaName \"Schema\",\n"
                         "       a.Status \"Status\",\n"
                         "       a.Server \"Server\",\n"
                         "       a.OsUser \"Osuser\",\n"
                         "       a.Machine \"Machine\",\n"
                         "       a.Program \"Program\",\n"
                         "       a.Type \"Type\",\n"
                         "       a.Module \"Module\",\n"
                         "       a.Action \"Action\",\n"
                         "       a.Client_Info \"Client Info\",\n"
                         "       b.Block_Gets \"-Block Gets\",\n"
                         "       b.Consistent_Gets \"-Consistent Gets\",\n"
                         "       b.Physical_Reads \"-Physical Reads\",\n"
                         "       b.Block_Changes \"-Block Changes\",\n"
                         "       b.Consistent_Changes \"-Consistent Changes\",\n"
                         "       c.Value*10 \"-CPU (ms)\",\n"
                         "       a.last_call_et \"Last SQL\",\n"
                         "       a.Process \"-Client PID\",\n"
                         "       e.SPid \"-Server PID\",\n"
                         "       d.sql_text \"Current statement\",\n"
                         "       a.SQL_Address||':'||a.SQL_Hash_Value \" SQL Address\",\n"
                         "       a.Prev_SQL_Addr||':'||a.Prev_Hash_Value \" Prev SQl Address\"\n"
                         "  FROM v$session a,\n"
                         "       v$sess_io b,\n"
                         "       v$sesstat c,\n"
                         "       v$sql d,\n"
                         "       v$process e\n"
                         " WHERE a.sid = b.sid(+)\n"
                         "   AND a.sid = c.sid(+) AND (c.statistic# = 12 OR c.statistic# IS NULL)\n"
                         "   AND a.sql_address = d.address(+) AND a.sql_hash_value = d.hash_value(+)\n"
                         "   AND (d.child_number = 0 OR d.child_number IS NULL)\n"
                         "   AND a.paddr = e.addr(+)\n"
                         "%1 ORDER BY a.Sid",
                         "List sessions, must have same number of columns and the first and last 2 must be the same");

static toSQL SQLSessionsPg(
    "toSession:ListSession",
    "SELECT pg_stat_get_backend_pid ( s.backendid ) AS \"Backend ID\",\n"
    "       ( SELECT u.usename\n"
    "           FROM pg_user u\n"
    "          WHERE u.usesysid = pg_stat_get_backend_userid ( s.backendid ) ) AS \"User\",\n"
    "       pg_stat_get_backend_client_addr ( s.backendid ) AS \"From\",\n"
    "       pg_stat_get_backend_client_port ( s.backendid ) AS \"Port\",\n"
    "       pg_stat_get_backend_activity_start ( s.backendid ) AS \"Started\",\n"
    "       pg_stat_get_backend_waiting ( s.backendid ) AS \"Waiting\",\n"
    "       pg_stat_get_backend_activity ( s.backendid ) AS \"Current Query\"\n"
    "  FROM ( SELECT pg_stat_get_backend_idset ( ) AS backendid ) AS s",
    "",
    "",
    "PostgreSQL");

toSession::toSession(QWidget *main, toConnection &connection)
        : toToolWidget(SessionTool, "session.html", main, connection, "toSession")
{
    QToolBar *toolbar = toAllocBar(this, tr("Session manager"));
    layout()->addWidget(toolbar);

    refreshAct =
        toolbar->addAction(
            QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
            tr("Update sessionlist"),
            this,
            SLOT(refresh(void)));
    refreshAct->setShortcut(QKeySequence::Refresh);

    if (toIsOracle(connection))
    {
        toolbar->addSeparator();

        Select = new toResultCombo(toolbar, TO_TOOLBAR_WIDGET_NAME);
        Select->setSelected(tr("Only active users"));
        Select->additionalItem(tr("Only active users"));
        Select->additionalItem(tr("All"));
        Select->additionalItem(tr("No background"));
        Select->additionalItem(tr("No system"));
        Select->query(toSQL::sql(toSQL::TOSQL_USERLIST));
        toolbar->addWidget(Select);

        connect(Select, SIGNAL(activated(int)), this, SLOT(refresh()));

        toolbar->addSeparator();

        enableTimedAct =
            toolbar->addAction(
                QIcon(QPixmap(const_cast<const char**>(clock_xpm))),
                tr("Enable timed statistics"),
                this,
                SLOT(enableStatistics(void)));

        disableTimedAct =
            toolbar->addAction(
                QIcon(QPixmap(const_cast<const char**>(noclock_xpm))),
                tr("Disable timed statistics"),
                this,
                SLOT(disableStatistics(void)));

        toolbar->addSeparator();

        disconnectAct =
            toolbar->addAction(
                QIcon(QPixmap(const_cast<const char**>(kill_xpm))),
                tr("Disconnect selected session"),
                this,
                SLOT(disconnectSession(void)));

        toolbar->addSeparator();
    }
    else
    {
        enableTimedAct  = NULL;
        disableTimedAct = NULL;

        disconnectAct =
            toolbar->addAction(
                QIcon(QPixmap(const_cast<const char**>(kill_xpm))),
                tr("Cancel selected backend"),
                this,
                SLOT(cancelBackend(void)));
    }

    toolbar->addWidget(
        new QLabel(tr("Refresh") + " ", toolbar));

    Refresh = toRefreshCreate(toolbar, TO_TOOLBAR_WIDGET_NAME);
    connect(Refresh,
            SIGNAL(activated(const QString &)),
            this, SLOT(changeRefresh(const QString &)));
    toolbar->addWidget(Refresh);

    if (toIsOracle(connection))
    {
        toolbar->addSeparator();

        QToolButton *btn = new QToolButton(toolbar);
        btn->setCheckable(true);
        btn->setIcon(QIcon(filter_xpm));
        connect(btn, SIGNAL(toggled(bool)), this, SLOT(excludeSelection(bool)));
        btn->setToolTip(tr("Exclude selected sessions"));
        toolbar->addWidget(btn);

        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(add_xpm))),
                           tr("Select all sessions"),
                           this,
                           SLOT(selectAll(void)));

        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(minus_xpm))),
                           tr("Deselect all sessions"),
                           this,
                           SLOT(selectNone(void)));
    }

    toolbar->addWidget(new toSpacer());

    Total = new QLabel(toolbar);
    Total->setAlignment(Qt::AlignRight | Qt::AlignVCenter/* | Qt::ExpandTabs*/);
    toolbar->addWidget(Total);

    new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(splitter);

    Sessions = new toSessionList(splitter);
    Sessions->setAlternatingRowColors(true);

    QList<int> list;
    list.append(75);
    splitter->setSizes(list);

    Sessions->setReadAll(true);
    connect(Sessions, SIGNAL(done()), this, SLOT(done()));

    ResultTab = new QTabWidget(splitter);

    CurrentStatement = new toSGAStatement(ResultTab);
    ResultTab->addTab(CurrentStatement, tr("Current Statement"));

    if (toIsOracle(connection))
    {
        QString sql = toSQL::string(TOSQL_LONGOPS, connection);
        sql += " AND b.sid = :sid<char[101]> AND b.serial# = :ser<char[101]> order by b.start_time desc";
        LongOps = new toResultLong(true, false, toQuery::Background, ResultTab);
        LongOps->setSQL(sql);
        ResultTab->addTab(LongOps, tr("Long ops"));

        StatisticSplitter = new QSplitter(Qt::Horizontal, ResultTab);
        SessionStatistics = new toResultStats(false, 0, StatisticSplitter);
        WaitBar = new toResultBar(StatisticSplitter);
        WaitBar->setSQL(SQLSessionWait);
        WaitBar->setTitle(tr("Session wait states"));
        WaitBar->setYPostfix(QString::fromLatin1("ms/s"));
        IOBar = new toResultBar(StatisticSplitter);
        IOBar->setSQL(SQLSessionIO);
        IOBar->setTitle(tr("Session I/O"));
        IOBar->setYPostfix(QString::fromLatin1("blocks/s"));
        ResultTab->addTab(StatisticSplitter, tr("Statistics"));

        Waits = new toWaitEvents(0, ResultTab, "waits");
        ResultTab->addTab(Waits, tr("Wait events"));

        ConnectInfo = new toResultLong(true, false, toQuery::Background, ResultTab);
        ConnectInfo->setSQL(SQLConnectInfo);
        ResultTab->addTab(ConnectInfo, tr("Connect Info"));

        PendingLocks = new toResultLock(ResultTab);
        ResultTab->addTab(PendingLocks, tr("Pending Locks"));

        LockedObjects = new toResultTableView(false, false, ResultTab);
        ResultTab->addTab(LockedObjects, tr("Locked Objects"));
        LockedObjects->setSQL(SQLLockedObject);

        AccessedObjects = new toResultLong(false, false, toQuery::Background, ResultTab);
        AccessedObjects->setSQL(SQLAccessedObjects);
        ResultTab->addTab(AccessedObjects, tr("Accessing"));

        PreviousStatement = new toSGAStatement(ResultTab);
        ResultTab->addTab(PreviousStatement, tr("Previous Statement"));

        OpenSplitter = new QSplitter(Qt::Horizontal, ResultTab);
        ResultTab->addTab(OpenSplitter, tr("Open Cursors"));
        OpenCursors = new toResultLong(false, true, toQuery::Background, OpenSplitter);
        OpenCursors->setSQL(SQLOpenCursors);
        OpenStatement = new toSGAStatement(OpenSplitter);

        OpenCursors->setSelectionMode(toTreeWidget::Single);
        connect(OpenCursors, SIGNAL(selectionChanged(toTreeWidgetItem *)),
                this, SLOT(changeCursor(toTreeWidgetItem *)));
    }
    else
    {
        LongOps           = NULL;
        StatisticSplitter = NULL;
        SessionStatistics = NULL;
        WaitBar           = NULL;
        IOBar             = NULL;
        Waits             = NULL;
        ConnectInfo       = NULL;
        PendingLocks      = NULL;
        AccessedObjects   = NULL;
        PreviousStatement = NULL;
        OpenSplitter      = NULL;
        OpenCursors       = NULL;
        OpenStatement     = NULL;

        LockedObjects = new toResultTableView(false, false, ResultTab);
        ResultTab->addTab(LockedObjects, tr("Locked Objects"));
        LockedObjects->setSQL(SQLLockedObject);
    }

    Sessions->setSelectionMode(toTreeWidget::Single);
    connect(Sessions, SIGNAL(selectionChanged(toTreeWidgetItem *)),
            this, SLOT(changeItem(toTreeWidgetItem *)));
    connect(ResultTab, SIGNAL(currentChanged(int)),
            this, SLOT(changeTab(int)));

    try
    {
        connect(timer(), SIGNAL(timeout(void)), this, SLOT(refreshTabs(void)));
        toRefreshParse(timer());
    }
    TOCATCH;

    CurrentTab = CurrentStatement;

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(windowActivated(QMdiSubWindow *)));
    refresh();

    setFocusProxy(Sessions);
}

bool toSession::canHandle(toConnection &conn)
{
    return toIsOracle(conn) || toIsPostgreSQL(conn);
}

void toSession::excludeSelection(bool tgl)
{
    toSessionList::sessionFilter *filt =
        dynamic_cast<toSessionList::sessionFilter *>(Sessions->filter());
    if (filt)
    {
        filt->setShow(!tgl);
        refresh();
    }
}

void toSession::selectAll(void)
{
    for (toTreeWidgetItem *item = Sessions->firstChild();
            item;
            item = item->nextSibling())
    {
        toResultViewCheck * chk = dynamic_cast<toResultViewCheck *>(item);
        if (chk)
            chk->setOn(true);
    }
}

void toSession::selectNone(void)
{
    for (toTreeWidgetItem *item = Sessions->firstChild();item;item = item->nextSibling())
    {
        toResultViewCheck * chk = dynamic_cast<toResultViewCheck *>(item);
        if (chk)
            chk->setOn(false);
    }
}

toTreeWidgetItem *toSessionList::createItem(toTreeWidgetItem *last, const QString &str)
{
    sessionFilter *filt = dynamic_cast<sessionFilter *>(filter());
    if (filt && filt->show() && toIsOracle(connection()))
        return new toResultViewCheck(this, last, str, toTreeWidgetCheck::CheckBox);
    else
        return new toResultViewItem(this, last, str);
}

void toSessionList::updateFilter()
{
    sessionFilter *filt = dynamic_cast<sessionFilter *>(filter());
    if (filt)
        filt->updateList(this);
}

bool toSessionList::sessionFilter::check(const toTreeWidgetItem *item)
{
    if (!OnlyDatabase.isEmpty())
    {
        if (OnlyDatabase == "/")
        {
            if (item->text(4) == "Sleep")
                return false;
        }
        else if (OnlyDatabase != item->text(3))
            return false;
    }

    sessionID serial(item->text(0).toInt(), item->text(1).toInt());
    bool checked = false;
    for (std::list<sessionID>::iterator i = Serials.begin();i != Serials.end();i++)
        if ((*i) == serial)
        {
            checked = true;
            break;
        }
    const toResultViewCheck *chk = dynamic_cast<const toResultViewCheck *>(item);
    if (chk)
    {
        const_cast<toResultViewCheck *>(chk)->setOn(checked);
        return true;
    }
    return !checked;
}

void toSessionList::sessionFilter::updateList(toResultLong *lst)
{
    bool first = true;
    for (toTreeWidgetItem *item = lst->firstChild();item;item = item->nextSibling())
    {
        toResultViewCheck * chk = dynamic_cast<toResultViewCheck *>(item);
        if (chk)
        {
            if (first)
            {
                Serials.clear();
                first = false;
            }
            if (chk->isOn())
                Serials.insert(Serials.end(), sessionID(item->text(0).toInt(), item->text(1).toInt()));
        }
    }
}

void toSession::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;
    if (widget->widget() == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Session"), this);

            // don't use toIs<connection type> here. causes crash on
            // shutdown when windows are closed/changed.

            if (refreshAct)
                ToolMenu->addAction(refreshAct);

            ToolMenu->addSeparator();

            if (enableTimedAct)
                ToolMenu->addAction(enableTimedAct);
            if (disableTimedAct)
                ToolMenu->addAction(disableTimedAct);

            if (enableTimedAct || disableTimedAct)
                ToolMenu->addSeparator();

            if (disconnectAct)
            {
                ToolMenu->addAction(disconnectAct);
                ToolMenu->addSeparator();
            }

            toMainWidget()->addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toSession::refresh(void)
{
    try
    {
        toTreeWidgetItem *item = Sessions->selectedItem();
        if (item)
        {
            Session = item->text(0);
            Serial = item->text(1);
        }
        else
            Session = Serial = QString::null;
        QString sql = toSQL::string(SQLSessions, connection());

        if (toIsOracle(connection()))
        {
            QString extra;
            if (Select->currentIndex() == 0)
                extra = "   AND a.Type != 'BACKGROUND' AND a.Status != 'INACTIVE'\n";
            else if (Select->currentIndex() == 1)
                ; // Do nothing
            else if (Select->currentIndex() == 2)
                extra = "   AND a.Type != 'BACKGROUND'\n";
            else if (Select->currentIndex() == 3)
                extra = "   AND a.SchemaName NOT IN ('SYS','SYSTEM')\n";
            else
                extra = "   AND a.SchemaName = '" + Select->currentText() + "'\n";

            sql = sql.arg(extra);
        }

        Sessions->setSQL(sql);
        Sessions->refresh();
    }
    TOCATCH;
}

void toSession::done(void)
{
    int system = 0, total = 0, active = 0;
    for (toTreeWidgetItem *item = Sessions->firstChild();
            item;
            item = item->nextSibling())
    {
        if (item->text(0) == Session &&
                item->text(1) == Serial)
        {
            Sessions->setSelected(item, true);
        }
        total++;
        if (item->text(8) != "USER")
            system++;
        else if (item->text(3) == "ACTIVE")
            active++;
    }
    Total->setText(QString("Total <B>%1</B> (Active <B>%3</B>, System <B>%2</B>)")
                   .arg(total).arg(system).arg(active));
    Sessions->resizeColumnsToContents();
}

void toSession::enableStatistics(bool enable)
{
    QString sql;
    if (enable)
        sql = QString::fromLatin1("ALTER SYSTEM SET TIMED_STATISTICS = TRUE");
    else
        sql = QString::fromLatin1("ALTER SYSTEM SET TIMED_STATISTICS = FALSE");
    try
    {
        connection().execute(sql);
    }
    catch (...)
    {
        toStatusMessage(tr("No access to timed statistics flags"));
    }
}

void toSession::changeTab(int index)
{
    QWidget *tab = ResultTab->widget(index);

    if (tab != CurrentTab)
    {
        CurrentTab = tab;
        toTreeWidgetItem *item = Sessions->selectedItem();
        if (item)
        {
            if (CurrentTab == StatisticSplitter)
            {
                int ses = item->text(0).toInt();
                try
                {
                    SessionStatistics->changeSession(ses);
                }
                TOCATCH;
            }
            else if (CurrentTab == ConnectInfo)
            {
                ConnectInfo->clearParams();
                ConnectInfo->changeParams(item->text(0));
            }
            else if (CurrentTab == LongOps)
            {
                LongOps->clearParams();
                LongOps->changeParams(item->text(0), item->text(1));
            }
            else if (CurrentTab == PendingLocks)
            {
                PendingLocks->clearParams();
                PendingLocks->query(item->text(0));
            }
            else if (CurrentTab == OpenSplitter)
            {
                toTreeWidgetItem *openitem = OpenCursors->currentItem();
                QString address;
                if (openitem)
                    address = openitem->text(2);
                OpenCursors->clearParams();
                OpenCursors->changeParams(item->text(0));
                if (!address.isEmpty())
                    for (openitem = OpenCursors->firstChild();
                            openitem;openitem = openitem->nextSibling())
                        if (address == openitem->text(2))
                        {
                            OpenCursors->setSelected(item, true);
                            break;
                        }
            }
            else if (CurrentTab == CurrentStatement)
            {
                CurrentStatement->changeAddress(item->text(Sessions->columns() + 0));
            }
            else if (CurrentTab == AccessedObjects)
            {
                AccessedObjects->clearParams();
                AccessedObjects->changeParams(item->text(0));
            }
            else if (CurrentTab == LockedObjects)
            {
                LockedObjects->clearParams();
                LockedObjects->changeParams(item->text(0));
            }
            else if (CurrentTab == PreviousStatement)
            {
                PreviousStatement->changeAddress(item->text(Sessions->columns() + 1));
            }
        }
    }
}

void toSession::changeCursor(toTreeWidgetItem *item)
{
    if (item)
        OpenStatement->changeAddress(item->text(2));
}

void toSession::cancelBackend()
{
    toTreeWidgetItem *item = Sessions->selectedItem();
    if (item)
    {
        try
        {
            connection().execute(
                QString("SELECT pg_cancel_backend ( %1 )").arg(item->text(0)));
        }
        TOCATCH;
    }
}

void toSession::disconnectSession()
{
    toTreeWidgetItem *item = Sessions->selectedItem();
    if (item)
    {
        QString sess = QString::fromLatin1("'");
        sess.append(item->text(0));
        sess.append(QString::fromLatin1(","));
        sess.append(item->text(1));
        sess.append(QString::fromLatin1("'"));
        QString str(tr("Let current transaction finish before "
                       "disconnecting this session?"));
        QString sql;
        switch (TOMessageBox::warning(this,
                                      tr("Commit work?"),
                                      str,
                                      tr("&Yes"),
                                      tr("&No"),
                                      tr("Cancel")))
        {
        case 0:
            sql = QString::fromLatin1("ALTER SYSTEM DISCONNECT SESSION ");
            sql.append(sess);
            sql.append(QString::fromLatin1(" POST_TRANSACTION"));
            break;
        case 1:
            sql = QString::fromLatin1("ALTER SYSTEM KILL SESSION ");
            sql.append(sess);
            break;
        case 2:
            return ;
        }
        try
        {
            connection().execute(sql);
        }
        TOCATCH;
    }
}

void toSession::changeRefresh(const QString &str)
{
    try
    {
        toRefreshParse(timer(), str);
    }
    TOCATCH;
}

void toSession::changeItem(toTreeWidgetItem *item)
{
    if (item && LastSession != item->text(0))
    {
        if (!item->text(0).isEmpty())
        {
            if (WaitBar)
                WaitBar->changeParams(item->text(0));
            if (IOBar)
                IOBar->changeParams(item->text(0));
            if (Waits)
                Waits->setSession(item->text(0).toInt());
        }
        LastSession = item->text(0);
    }
    QWidget *t = CurrentTab;
    CurrentTab = NULL;
    changeTab(ResultTab->indexOf(t));
}

void toSession::refreshTabs(void)
{
    toTreeWidgetItem *item = Sessions->selectedItem();
    if (item)
        changeItem(item);
}
