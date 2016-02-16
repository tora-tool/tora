
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

#include "tools/tosession.h"

#include "core/utils.h"
#include "core/tochangeconnection.h"
#include "widgets/toresultschema.h"
#include "widgets/toresultlock.h"
#include "widgets/toresultitem.h"
#include "widgets/toresultstats.h"
#include "widgets/torefreshcombo.h"
#include "tools/tosgastatement.h"
#include "tools/tosgatrace.h"
#include "tools/towaitevents.h"
#include "widgets/toresulttableview.h"
#include "core/toeventquery.h"
#include "core/totimer.h"
#include "core/toglobalevent.h"
#include "core/totool.h"

#include <QLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QSplitter>
#include <QToolBar>

#include "icons/add.xpm"
#include "icons/clock.xpm"
#include "icons/kill.xpm"
#include "icons/minus.xpm"
#include "icons/noclock.xpm"
#include "icons/refresh.xpm"
#include "icons/tosession.xpm"
// #include "icons/filter.xpm"


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

        virtual toToolWidget* toolWindow(QWidget *parent, toConnection &connection)
        {
            if (connection.providerIs("Oracle") || connection.providerIs("QPSQL"))
                return new toSession(parent, connection);

            return NULL;
        }

        virtual bool canHandle(const toConnection &conn)
        {
            return conn.providerIs("Oracle") || conn.providerIs("QPSQL");
        }

        virtual void closeWindow(toConnection &connection) {};
};


static toSessionTool SessionTool;


class toSessionFilter  : public toViewFilter
{
        QRegExp Filter;

    public:
        toSessionFilter()
        {
        }


        virtual toViewFilter* clone()
        {
            toSessionFilter *f = new toSessionFilter;
            f->Filter = Filter;
            return f;
        }


        void setFilterString(const QString &f)
        {
            if (f.isEmpty())
            {
                QRegExp filter;
                Filter = filter;
            }
            else
            {
                QRegExp filter(QString("*") + f + QString("*"),
                               Qt::CaseInsensitive,
                               QRegExp::Wildcard);
                Filter = filter;
            }
        }


        /**
         * return true to show, false to hide
         *
         */
        virtual bool check(const toResultModel *model, const int row)
        {
            if (Filter.isEmpty())
                return true;

            int rows = model->rowCount();
            for (int col = 0; col < rows; col++)
            {
                QString data = model->data(row, col).toString();
                if (data.isEmpty())
                    continue;

                if (Filter.exactMatch(data))
                    return true;
            }

            return false;
        }
};


static toSQL SQLConnectInfo(
    "toSession:ConnectInfo",
    "select authentication_type,osuser,network_service_banner\n"
    "  from v$session_connect_info where sid = :f1<char[101]>",
    "Get connection info for a session");

static toSQL SQLLockedObject(
    "toSession:LockedObject",
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
    "QPSQL");

static toSQL SQLOpenCursors(
    "toSession:OpenCursor",
    "select SQL_Text \"SQL\", Address||':'||Hash_Value \" Address\"\n"
    "  from v$open_cursor where sid = :f1<char[101]>",
    "Display open cursors of this session");

static toSQL SQLSessionWait(
    TO_SESSION_WAIT,
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

static toSQL SQLSessionIO(
    TO_SESSION_IO,
    "select sysdate,\n"
    "       sum(block_gets) \"Block gets\",\n"
    "       sum(consistent_gets) \"Consistent gets\",\n"
    "       sum(physical_reads) \"Physical reads\",\n"
    "       sum(block_changes) \"Block changes\",\n"
    "       sum(consistent_changes) \"Consistent changes\"\n"
    "  from v$sess_io where sid in (select b.sid from v$session a,v$session b where a.sid = :f1<char[101]> and a.audsid = b.audsid)",
    "Display chart of session generated I/O");

static toSQL SQLSessionTXN(
    TO_SESSION_TXN,
    "SELECT TO_CHAR(s.sid)||','||TO_CHAR(s.serial#) sid_serial,                                     \n"
    "           NVL(s.username, 'None') OraUser,                                                    \n"
    "           s.program,                                                                          \n"
    "           case when t.used_ublk is not null then t.used_ublk * TO_NUMBER(x.value)/1024/1024||'M'   \n"
    "	        else null end as \"Undo\",                                                          \n"
    "	   t.*                                                                                      \n"
    "      FROM sys.v_$session     s,                                                               \n"
    "           sys.v_$transaction t,                                                               \n"
    "           sys.v_$parameter   x                                                                \n"
    "     WHERE s.taddr = t.addr(+)                                                                 \n"
    "       AND x.name  = 'db_block_size'                                                           \n"
    "       AND SID = :f1<char[101]> AND SERIAL# = :f2<char[101]>                                   \n",
    "Display transaction details");

static toSQL SQLAccessedObjects(
    "toSession:AccessedObjects",
    "SELECT owner,\n"
    "       OBJECT,\n"
    "       TYPE FROM v$access\n"
    " WHERE sid=:f1<char[101]>\n"
    " ORDER BY owner,\n"
    "   OBJECT,\n"
    "   TYPE",
    "Which objects are accessed by the current session");

static toSQL SQLSessions(
    "toSession:ListSession",
    "SELECT a.Sid \"Id\",\n"
    "       a.Serial# \"Serial#\",\n"
    "       a.UserName \"Username\",\n"
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
    "       b.Block_Gets \"Block Gets\",\n"
    "       b.Consistent_Gets \"Consistent Gets\",\n"
    "       b.Physical_Reads \"Physical Reads\",\n"
    "       b.Block_Changes \"Block Changes\",\n"
    "       b.Consistent_Changes \"Consistent Changes\",\n"
    "       c.Value*10 \"CPU (ms)\",\n"
    "       a.last_call_et \"Last SQL\",\n"
    "       a.Process \"Client PID\",\n"
    "       e.SPid \"Server PID\",\n"
    "       d.sql_text \"Current statement\",\n"
    "       a.SQL_Address||':'||a.SQL_Hash_Value \" SQL Address\",         \n"
    "       a.Prev_SQL_Addr||':'||a.Prev_Hash_Value \" Prev SQl Address\", \n"
    "       a.SQL_ID as \" SQL_ID\",                                       \n"
    "       a.SQL_CHILD_NUMBER as \" SQL_CHILD_NUMBER\",                   \n"
    "       a.PREV_SQL_ID as \" PREV_SQL_ID\",                             \n"
    "       a.PREV_CHILD_NUMBER as \" PREV_CHILD_NUMBER\"                  \n"
    "  FROM v$session a left join v$sess_io b on ( a.sid = b.sid)          \n"
    "       left join v$sesstat c on ( a.sid = c.sid)                      \n"
    "       left join v$sql d on (a.sql_address = d.address and            \n"
    "                             a.sql_hash_value=d.hash_value and        \n"
    "                             a.sql_child_number = d.child_number)     \n"
    "       left join v$process e on (a.paddr = e.addr)\n"
    " WHERE (c.statistic# = 12 OR c.statistic# IS NULL)\n"
    "%1 ORDER BY a.Sid",
    "List sessions, must have same number of columns and the first and last 2 must be the same",
    "1000");

static toSQL SQLSessions7(
    "toSession:ListSession",
    "SELECT a.Sid \"Id\",\n"
    "       a.Serial# \"Serial#\",\n"
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
    "       b.Block_Gets \"Block Gets\",\n"
    "       b.Consistent_Gets \"Consistent Gets\",\n"
    "       b.Physical_Reads \"Physical Reads\",\n"
    "       b.Block_Changes \"Block Changes\",\n"
    "       b.Consistent_Changes \"Consistent Changes\",\n"
    "       c.Value*10 \"CPU (ms)\",\n"
    "       a.last_call_et \"Last SQL\",\n"
    "       a.Process \"Client PID\",\n"
    "       e.SPid \"Server PID\",\n"
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
    "",
    "0703");

static toSQL SQLSessionsPg(
    "toSession:ListSession",
    "SELECT pg_stat_get_backend_pid ( s.backendid ) AS \"Backend ID\",\n"
    "       ( SELECT u.usename\n"
    "           FROM pg_user u\n"
    "          WHERE u.usesysid = pg_stat_get_backend_userid ( s.backendid ) ) AS \"User\",\n"
    "       ( SELECT datname\n"
    "           FROM pg_database db\n"
    "          WHERE db.OID = pg_stat_get_backend_dbid ( s.backendid ) ) AS \"Database\",\n"
    "       pg_stat_get_backend_client_addr ( s.backendid ) AS \"From\",\n"
    "       pg_stat_get_backend_client_port ( s.backendid ) AS \"Port\",\n"
    "       pg_stat_get_backend_activity_start ( s.backendid ) AS \"Started\",\n"
    "       pg_stat_get_backend_waiting ( s.backendid ) AS \"Waiting\",\n"
    "       pg_stat_get_backend_activity ( s.backendid ) AS \"Current Query\",\n"
    "       s.backendid AS \" current\",\n"
    "       NULL AS \" previous\"\n"
    "  FROM ( SELECT pg_stat_get_backend_idset ( ) AS backendid ) AS s",
    "",
    "",
    "QPSQL");

toSession::toSession(QWidget *main, toConnection &connection)
    : toToolWidget(SessionTool,
                   "session.html",
                   main,
                   connection,
                   "toSession")
    , SessionFilter(new toSessionFilter)
{
    QToolBar *toolbar = Utils::toAllocBar(this, tr("Session manager"));
    layout()->addWidget(toolbar);

    refreshAct =
        toolbar->addAction(
            QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
            tr("Update sessionlist"),
            this,
            SLOT(slotRefresh(void)));
    refreshAct->setShortcut(QKeySequence::Refresh);

    connect(this, SIGNAL(connectionChange(void)), this, SLOT(slotRefresh(void)));

    if (connection.providerIs("Oracle"))
    {
        toolbar->addSeparator();

        Select = new toResultSchema(toolbar);
        Select->additionalItem(tr("Only active users"));
        Select->additionalItem(tr("All"));
        Select->additionalItem(tr("No background"));
        Select->additionalItem(tr("No system"));
        Select->setSelected(tr("Only active users"));
        Select->refresh();
        toolbar->addWidget(Select);

        connect(Select, SIGNAL(activated(int)), this, SLOT(slotRefresh()));

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
                SLOT(slotDisconnectSession(void)));

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
                SLOT(slotCancelBackend()));
    }

    toolbar->addWidget(
        new QLabel(tr("Refresh") + " ", toolbar));

    Refresh = new toRefreshCombo(toolbar);
    connect(Refresh, SIGNAL(activated(const QString &)), this, SLOT(slotChangeRefresh(const QString &)));
    connect(Refresh->timer(), SIGNAL(timeout(void)), this, SLOT(slotRefreshTabs(void)));
    toolbar->addWidget(Refresh);

    toolbar->addSeparator();

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(add_xpm))),
                       tr("Select all sessions"),
                       this,
                       SLOT(slotSelectAll(void)));

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(minus_xpm))),
                       tr("Deselect all sessions"),
                       this,
                       SLOT(slotSelectNone(void)));

    toolbar->addWidget(new QLabel(tr("Filter")));

    QLineEdit *filter = new QLineEdit;
    filter->setMinimumWidth(200);
    toolbar->addWidget(filter);

    connect(filter,
            SIGNAL(textChanged(const QString &)),
            this,
            SLOT(slotFilterChanged(const QString &)));

    toolbar->addWidget(new Utils::toSpacer());

    Total = new QLabel(toolbar);
    Total->setAlignment(Qt::AlignRight | Qt::AlignVCenter/* | Qt::ExpandTabs*/);
    toolbar->addWidget(Total);

    new toChangeConnection(toolbar);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(splitter);

    Sessions = new toResultTableView(true,
                                     false,
                                     splitter,
                                     "session list",
                                     false);
    Sessions->setAlternatingRowColors(true);
    Sessions->horizontalHeader()->setStretchLastSection(true);
    Sessions->setSelectionBehavior(QAbstractItemView::SelectRows);
    Sessions->setSelectionMode(QAbstractItemView::ExtendedSelection);
    Sessions->setReadAll(true);
    Sessions->setFilter(SessionFilter);

    connect(Sessions, SIGNAL(slotDone()), this, SLOT(slotDone()));

    ResultTab = new QTabWidget(splitter);

    CurrentStatement = new toSGAStatement(ResultTab);
    ResultTab->addTab(CurrentStatement, tr("Current Statement"));

    if (connection.providerIs("Oracle"))
    {
        PreviousStatement = new toSGAStatement(ResultTab);
        ResultTab->addTab(PreviousStatement, tr("Previous Statement"));

        Transaction = new toResultItem(2, true, ResultTab, "SessionTXN");
        Transaction->setSQL(SQLSessionTXN);
        ResultTab->addTab(Transaction, tr("Transaction"));

        QString sql = toSQL::string(TOSQL_LONGOPS, connection);
        sql += " AND b.sid = :sid<char[101]> AND b.serial# = :ser<char[101]> order by b.start_time desc";
        LongOps = new toResultLong(true, false, ResultTab);
        LongOps->setSQL(sql);
        ResultTab->addTab(LongOps, tr("Long ops"));

        StatisticSplitter = new QSplitter(Qt::Horizontal, ResultTab);
        SessionStatistics = new toResultStats(false, 0, StatisticSplitter);
#ifdef TORA3_GRAPH
        WaitBar = new toResultBar(StatisticSplitter);
        WaitBar->setSQL(SQLSessionWait);
        WaitBar->setTitle(tr("Session wait states"));
        WaitBar->setYPostfix(QString::fromLatin1("ms/s"));
        IOBar = new toResultBar(StatisticSplitter);
        IOBar->setSQL(SQLSessionIO);
        IOBar->setTitle(tr("Session I/O"));
        IOBar->setYPostfix(QString::fromLatin1("blocks/s"));
#endif
        ResultTab->addTab(StatisticSplitter, tr("Statistics"));

        Waits = new toWaitEvents(0, ResultTab, "waits");
        ResultTab->addTab(Waits, tr("Wait events"));

        ConnectInfo = new toResultTableView(true, false, ResultTab);
        ConnectInfo->horizontalHeader()->setStretchLastSection(true);
        ConnectInfo->setSQL(SQLConnectInfo);
        ResultTab->addTab(ConnectInfo, tr("Connect Info"));

        PendingLocks = new toResultLock(ResultTab);
        ResultTab->addTab(PendingLocks, tr("Pending Locks"));

        LockedObjects = new toResultTableView(false, false, ResultTab);
        ResultTab->addTab(LockedObjects, tr("Locked Objects"));
        LockedObjects->setSQL(SQLLockedObject);

        AccessedObjects = new toResultTableView(false, false, ResultTab);
        AccessedObjects->setSQL(SQLAccessedObjects);
        ResultTab->addTab(AccessedObjects, tr("Accessing"));

        OpenSplitter = new QSplitter(Qt::Horizontal, ResultTab);
        ResultTab->addTab(OpenSplitter, tr("Open Cursors"));
        OpenCursors = new toResultTableView(false, true, OpenSplitter);
        OpenCursors->setSQL(SQLOpenCursors);
        OpenCursors->setSelectionBehavior(QAbstractItemView::SelectRows);

        connect(OpenCursors,
                SIGNAL(selectionChanged()),
                this,
                SLOT(slotChangeCursor()));

        OpenStatement = new toSGAStatement(OpenSplitter);
    }
    else
    {
        LongOps           = NULL;
        StatisticSplitter = NULL;
        SessionStatistics = NULL;
#ifdef TORA3_GRAPH
        WaitBar           = NULL;
        IOBar             = NULL;
#endif
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

    connect(Sessions,
            SIGNAL(selectionChanged()),
            this,
            SLOT(slotChangeItem()));

    connect(ResultTab,
            SIGNAL(currentChanged(int)),
            this,
            SLOT(slotChangeTab(int)));

    CurrentTab = CurrentStatement;

    ToolMenu = NULL;
    // connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
    //         this, SLOT(slotWindowActivated(QMdiSubWindow *)));
    slotRefresh();

    QList<int> list;
    list.append(999);
    list.append(1);
    splitter->setSizes(list);

    setFocusProxy(Sessions);
}

toSession::~toSession()
{
    if (SessionFilter)
    {
        Sessions->setFilter(NULL);
        delete SessionFilter;
    }
}

bool toSession::canHandle(const toConnection &conn)
{
    return conn.providerIs("Oracle") || conn.providerIs("QPSQL");
}

void toSession::slotSelectAll()
{
    Sessions->selectAll();
}

void toSession::slotSelectNone()
{
    Sessions->clearSelection();
}


void toSession::slotWindowActivated(toToolWidget* widget)
{
    if (!widget)
        return;
    if (widget == this)
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

            toGlobalEventSingle::Instance().addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toSession::slotRefresh(void)
{
    try
    {
        QModelIndex item = Sessions->currentIndex();
        if (item.isValid())
        {
            Session = Sessions->model()->data(item.row(), 1).toString();
            Serial  = Sessions->model()->data(item.row(), 2).toString();
        }
        else
            Session = Serial = QString::null;
        QString sql = toSQL::string(SQLSessions, connection());

        if (connection().providerIs("Oracle"))
        {
            QString extra;
            switch (Select->currentIndex())
            {
                case 0:
                    extra = "   AND a.Type != 'BACKGROUND' AND a.Status != 'INACTIVE'\n";
                    break;
                case 1:
                    break;
                case 2:
                    extra = "   AND a.Type != 'BACKGROUND'\n";
                    break;
                case 3:
                    extra = "   AND a.SchemaName NOT IN ('SYS','SYSTEM')\n";
                    break;
                default:
                    extra = "   AND a.SchemaName = '" + Select->currentText() + "'\n";
                    break;
            }
            sql = sql.arg(extra);
        }

        Sessions->setSQL(sql);
        Sessions->refresh();
    }
    TOCATCH;
}

void toSession::slotDone(void)
{
    int system = 0;
    int total  = 0;
    int active = 0;

    for (toResultTableView::iterator it(Sessions); (*it).isValid(); it++)
    {
        QString session = Sessions->model()->data((*it).row(), 1).toString();
        QString serial  = Sessions->model()->data((*it).row(), 2).toString();
        QString user    = Sessions->model()->data((*it).row(), 9).toString();
        QString act     = Sessions->model()->data((*it).row(), 4).toString();

        if (session == Session && serial == Serial)
        {
            Sessions->selectionModel()->select(
                QItemSelection(*it, *it),
                QItemSelectionModel::ClearAndSelect);
            Sessions->setCurrentIndex(*it);
        }

        total++;
        if (user != "USER")
            system++;
        else if (act == "ACTIVE")
            active++;
    }

    Total->setText(QString("Total <B>%1</B> (Active <B>%3</B>, System <B>%2</B>)")
                   .arg(total).arg(system).arg(active));
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
        toConnectionSubLoan conn(connection());
        conn->execute(sql);
    }
    catch (...)
    {
        Utils::toStatusMessage(tr("No access to timed statistics flags"));
    }
}

void toSession::slotChangeTab(int index)
{
    QWidget *tab = ResultTab->widget(index);

    if (tab != CurrentTab)
    {
        CurrentTab = tab;
        QModelIndex item = Sessions->currentIndex();

        if (!item.isValid())
            return;

        QString connectionId = Sessions->model()->data(item.row(), 1).toString();
        QString serial       = Sessions->model()->data(item.row(), 2).toString();

        if (CurrentTab == StatisticSplitter)
        {
            int ses = connectionId.toInt();
            try
            {
                SessionStatistics->slotChangeSession(ses);
            }
            TOCATCH;
        }
        else if (CurrentTab == ConnectInfo)
        {
            ConnectInfo->refreshWithParams(toQueryParams() << connectionId);
        }
        else if (CurrentTab == LongOps)
        {
            LongOps->refreshWithParams(toQueryParams() << connectionId << serial);
        }
        else if (CurrentTab == PendingLocks)
        {
            PendingLocks->clearParams();
            PendingLocks->query(connectionId, toQueryParams());
        }
        else if (CurrentTab == OpenSplitter)
        {
            QModelIndex openitem = OpenCursors->selectedIndex(2);
            QString address;
            if (openitem.isValid())
                address = openitem.data().toString();
            OpenCursors->clearParams();
            OpenCursors->refreshWithParams(toQueryParams() << connectionId);
            if (!address.isEmpty())
            {
                for (toResultTableView::iterator it(OpenCursors); (*it).isValid(); it++)
                {
                    if (address == OpenCursors->model()->data(*it).toString())
                    {
                        OpenCursors->selectionModel()->select(
                            QItemSelection(*it, *it),
                            QItemSelectionModel::ClearAndSelect);
                        OpenCursors->setCurrentIndex(*it);
                    }
                }
            }
        }
        else if (CurrentTab == CurrentStatement)
        {
        	QString sql_id = Sessions->model()->data(item.row(), " SQL_ID").toString();
        	QString cursor = Sessions->model()->data(item.row(), " SQL_CHILD_NUMBER").toString();
        	CurrentStatement->changeAddress(toQueryParams() << sql_id << cursor);
        }
        else if (CurrentTab == PreviousStatement)
        {
        	QString sql_id = Sessions->model()->data(item.row(), " PREV_SQL_ID").toString();
        	QString cursor = Sessions->model()->data(item.row(), " PREV_CHILD_NUMBER").toString();
        	CurrentStatement->changeAddress(toQueryParams() << sql_id << cursor);
        }
        else if (CurrentTab == AccessedObjects)
        {
            AccessedObjects->refreshWithParams(toQueryParams() << connectionId);
        }
        else if (CurrentTab == LockedObjects)
        {
            LockedObjects->refreshWithParams(toQueryParams() << connectionId);
        }
        else if (CurrentTab == Transaction)
        {
            Transaction->refreshWithParams(toQueryParams() << connectionId << serial);
        }
    }
}

void toSession::slotChangeCursor()
{
	//QModelIndex item = Sessions->currentIndex();
    QModelIndex item = OpenCursors->selectedIndex(2);
    if (item.isValid())
        OpenStatement->changeAddress(toQueryParams() << item.data().toString());
}

void toSession::slotCancelBackend()
{
    QModelIndexList selected = Sessions->selectionModel()->selectedRows(1);
    foreach(QModelIndex item, selected)
    {
        if (!item.isValid())
            return;

        try
        {
            toConnectionSubLoan conn(connection());
            conn->execute(QString("SELECT pg_cancel_backend ( %1 )").arg(item.data().toString()));
        }
        TOCATCH;
    }
}

void toSession::slotDisconnectSession()
{
    QModelIndexList selected = Sessions->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return;

    QString str(tr("Let transaction(s) finish before disconnecting?"));
    QString sql;

    bool letcommit = false;
    switch (TOMessageBox::warning(this,
                                  tr("Commit work?"),
                                  str,
                                  tr("&Yes"),
                                  tr("&No"),
                                  tr("Cancel")))
    {
        case 0:
            letcommit = true;
            break;
        case 1:
            letcommit = false;
            break;
        case 2:
            return;
    }

    foreach(QModelIndex item, selected)
    {
        if (!item.isValid())
            return;

        QString connectionId = Sessions->model()->data(item.row(), 1).toString();
        QString serial       = Sessions->model()->data(item.row(), 2).toString();

        QString sess = QString::fromLatin1("'");
        sess.append(connectionId);
        sess.append(QString::fromLatin1(","));
        sess.append(serial);
        sess.append(QString::fromLatin1("'"));

        if (letcommit)
        {
            sql = QString::fromLatin1("ALTER SYSTEM DISCONNECT SESSION ");
            sql.append(sess);
            sql.append(QString::fromLatin1(" POST_TRANSACTION"));
        }
        else
        {
            sql = QString::fromLatin1("ALTER SYSTEM KILL SESSION ");
            sql.append(sess);
        }

        try
        {
            // oracle can take an awful long time to return
            toEventQuery *query = new toEventQuery(this
                                                   , connection()
                                                   , sql
                                                   , toQueryParams()
                                                   , toEventQuery::READ_ALL);
            connect(query,
                    SIGNAL(slotDone()),
                    query,
                    SLOT(deleteLater()));
            query->start();
        }
        TOCATCH;
    }
}

void toSession::slotChangeItem()
{
    QModelIndex selected = Sessions->currentIndex();
    if (!selected.isValid())
        return;

    QString item = Sessions->model()->data(selected.row(), 1).toString();
    if (LastSession != item)
    {
        if (!item.isEmpty())
        {
#ifdef TORA3_GRAPH
            if (WaitBar)
                WaitBar->refreshWithParams(toQueryParams() << item);
            if (IOBar)
                IOBar->refreshWithParams(toQueryParams() << item);
#endif
            if (Waits)
                Waits->setSession(item.toInt());
        }

        LastSession = item;
    }

    QWidget *t = CurrentTab;
    CurrentTab = NULL;
    slotChangeTab(ResultTab->indexOf(t));
}

void toSession::slotRefreshTabs(void)
{
    slotChangeItem();
}


void toSession::slotFilterChanged(const QString &text)
{
    SessionFilter->setFilterString(text);
    Sessions->applyFilter();
}
