
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
#include "core/toconfiguration.h"
#include "core/utils.h"
#include "core/tochangeconnection.h"
#include "core/toconnectionregistry.h"
#include "widgets/toresultschema.h"
#include "widgets/toresultitem.h"
#include "widgets/torefreshcombo.h"
#include "tools/tosgastatement.h"
#include "tools/tosgatrace.h"
#include "tools/towaitevents.h"
#include "core/toeventquery.h"
#include "core/toglobalevent.h"
#include "core/totool.h"

#include <QtCore/QTimer>
#include <QtCore/QEvent>
#include <QKeyEvent>
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

#include "tools/toresultbar.h"
#include "tools/toresultstats.h"
#include "tools/toresulttableview.h"

#include "result/toresultlock.h"

// #include "icons/filter.xpm"

#include "editor/tosyntaxanalyzernl.h"

using namespace ToConfiguration;

QVariant ToConfiguration::Session::defaultValue(int option) const
{
	switch (option)
	{
	case KillProcUseKillProcBool:
	    return QVariant(false);
	case KillProcName:
		return QVariant("KILL_ADMIN.KILL_SESSION");
	case KillProcSID:
		return QVariant("SID");
	case KillProcSerial:
		return QVariant("SERIAL");
	case KillProcInstance:
		return QVariant("INST_ID");
	case KillProcInstanceBool:
		return QVariant(false);
	case KillProcImmediate:
		return QVariant("IMMEDIAT");
	case KillProcImmediateBool:
		return QVariant(false);
	case KillSessionModeInt:
	    return QVariant(KillImmediate); // 1 disconnect, 2 kill, 3 kill immediate
	default:
		Q_ASSERT_X(false, qPrintable(__QHERE__), qPrintable(QString("Context Session un-registered enum value: %1").arg(option)));
		return QVariant();
	}
}

class toSessionTool : public toTool
{
    protected:
        const char **pictureXPM(void) override
        {
            return const_cast<const char**>(tosession_xpm);
        }
    public:
        toSessionTool() : toTool(210, "Sessions")
        { }

        const char *menuItem() override
        {
            return "Sessions";
        }

        toToolWidget* toolWindow(QWidget *parent, toConnection &connection) override
        {
            if (connection.providerIs("Oracle") || connection.providerIs("QPSQL") || connection.providerIs("QMYSQL"))
                return new toSession(parent, connection);

            return NULL;
        }
        QWidget *configurationTab(QWidget *parent) override
        {
            return new toSessionSetting(this, parent);
        }
        bool canHandle(const toConnection &conn) override
        {
            return conn.providerIs("Oracle") || conn.providerIs("QPSQL") || conn.providerIs("QMYSQL");
        }

        void closeWindow(toConnection &connection) override {};
    private:
        static ToConfiguration::Session s_sessionConf;
};


ToConfiguration::Session toSessionTool::s_sessionConf;

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
    "select * \n"
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
	" select SQL_Text SQL, CURSOR_TYPE, c.SQL_ID  as \" sql_id\", child_number as \" child_number\" \n"
	" from v$open_cursor c \n"
	" join (select distinct ADDRESS, HASH_VALUE, SQL_ID, child_number from v$sql_plan) p on c.ADDRESS = p.ADDRESS and c.HASH_VALUE = p.HASH_VALUE and c.sql_id = p.sql_id \n"
	" where sid = :f1<char[101]>",
    "Display open cursors of this session");

static toSQL SQLOpenCursors11g(
    "toSession:OpenCursor",
        " select SQL_Text SQL                        \n"
        "       , CURSOR_TYPE                        \n"
        "       , LAST_SQL_ACTIVE_TIME               \n"
        "       , SQL_EXEC_ID                        \n"
        "       , c.SQL_ID     as \" sql_id\"        \n"
        "       , child_number as \" child_number\"  \n"
        " from v$open_cursor c \n"
        " join (select distinct ADDRESS, HASH_VALUE, SQL_ID, child_number from v$sql_plan) p on c.ADDRESS = p.ADDRESS and c.HASH_VALUE = p.HASH_VALUE and c.sql_id = p.sql_id \n"
        " where sid = :f1<char[101]>",
        "",
        "1101");

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
//    "       c.Value*10 \"CPU (ms)\",\n"

    "       sw.wait_class, \n"
	"       sw.event,      \n"
    "       a.last_call_et \"Last Call(s)\",\n"
	"       a.Process \"Cl. PID\",\n"
	"       e.SPid \"Srv. PID\",\n"
    "       d.sql_text \"Current statement\",\n"
    "       a.SQL_Address||':'||a.SQL_Hash_Value \" SQL Address\",         \n"
    "       a.Prev_SQL_Addr||':'||a.Prev_Hash_Value \" Prev SQl Address\", \n"
    "       a.SQL_ID as \" SQL_ID\",                                       \n"
    "       a.SQL_CHILD_NUMBER as \" SQL_CHILD_NUMBER\",                   \n"
    "       a.PREV_SQL_ID as \" PREV_SQL_ID\",                             \n"
    "       a.PREV_CHILD_NUMBER as \" PREV_CHILD_NUMBER\"                  \n"
    "  FROM v$session a left join v$sess_io b on ( a.sid = b.sid)          \n"
//    "       left join v$sesstat c on ( a.sid = c.sid)                      \n"
    "       left join v$sql d on (a.sql_address = d.address and            \n"
    "                             a.sql_hash_value=d.hash_value and        \n"
    "                             a.sql_child_number = d.child_number)     \n"
    "       left join v$process e on (a.paddr = e.addr)                    \n"
    "       left join v$session_wait sw on (sw.sid = a.SID)                \n"
    " WHERE 1=1                                                            \n"
//    "       and (c.statistic# = 12 OR c.statistic# IS NULL)                \n"
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

static toSQL SQLSessionsMySQL(
    "toSession:ListSession",
    "show processlist",
    "",
    "",
    "QMYSQL");


toResultSessions::toResultSessions(QWidget *parent, const char *name)
    : Sessions::MVC(parent)
{
}

void toResultSessions::observeDone()
{
    emit queryDone();
}

toSession::toSession(QWidget *main, toConnection &connection)
    : super(SessionTool,
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
            tr("Update sessionlist(F5)\nRefresh Detail(R)"),
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

        enableTimedAct = toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(clock_xpm))), tr("Enable timed statistics"));
        QObject::connect(enableTimedAct, &QAction::triggered, this, [this]{ enableStatistics(true); });

        disableTimedAct = toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(noclock_xpm))), tr("Disable timed statistics"));
        connect(disableTimedAct, &QAction::triggered, this, [this]{ enableStatistics(false); });

        toolbar->addSeparator();

        disconnectAct =  toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(kill_xpm))), tr("Disconnect selected session"));
        connect(disconnectAct, &QAction::triggered, this, [this]{ slotDisconnectSession(); } );

        toolbar->addSeparator();
    }
    else
    {
        enableTimedAct  = NULL;
        disableTimedAct = NULL;

        disconnectAct =  toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(kill_xpm))), tr("Cancel selected backend"));
        connect(disconnectAct, &QAction::triggered, this, [this]{ slotCancelBackend(); } );
    }

    toolbar->addWidget(new QLabel(tr("Refresh") + " ", toolbar));

    Refresh = new toRefreshCombo(toolbar);
    connect(Refresh, SIGNAL(activated(const QString &)), this, SLOT(slotChangeRefresh(const QString &)));
    connect(Refresh->timer(), &QTimer::timeout, this, [this]{ slotRefreshTabs(); });
    toolbar->addWidget(Refresh);

    toolbar->addSeparator();
#if 0
    auto selectAll = toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(add_xpm))), tr("Select all sessions"));
    connect(selectAll, &QAction::triggered, this, [this]{ slotSelectAll(); } );

    auto deselectAll = toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(minus_xpm))), tr("Deselect all sessions"));
    connect(selectAll, &QAction::triggered, this, [this]{ slotSelectNone(); } );
#endif

    toolbar->addWidget(new QLabel(tr("Filter")));

    QLineEdit *filter = new QLineEdit;
    filter->setMinimumWidth(50);
    filter->setMaximumWidth(150);
    toolbar->addWidget(filter);

    connect(filter,
            SIGNAL(textChanged(const QString &)),
            this,
            SLOT(slotFilterChanged(const QString &)));

    toolbar->addWidget(new Utils::toSpacer());

    Total = new QLabel("");
    Total->setAlignment(Qt::AlignRight | Qt::AlignVCenter/* | Qt::ExpandTabs*/);
    toolbar->addWidget(Total);

    new toChangeConnection(toolbar);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(splitter);

    Sessions = new toResultSessions(splitter, "session list");
//    Sessions = new toResultTableView(true,
//                                     false,
//                                     splitter,
//                                     "session list",
//                                     false);
    Sessions->view()->setAlternatingRowColors(true);
    Sessions->view()->setAlternatingRowColors(true);
//    Sessions->view()->horizontalHeader()->setStretchLastSection(true);
    Sessions->view()->setSelectionBehavior(QAbstractItemView::SelectRows);
    Sessions->view()->setSelectionMode(QAbstractItemView::ExtendedSelection);

//    Sessions->setAlternatingRowColors(true);
//    Sessions->horizontalHeader()->setStretchLastSection(true);
//    Sessions->setSelectionBehavior(QAbstractItemView::SelectRows);
//    Sessions->setSelectionMode(QAbstractItemView::ExtendedSelection);
//    Sessions->setReadAll(true);
//    Sessions->view()->setFilter(SessionFilter);

    connect(Sessions, &toResultSessions::queryDone, this, [this]{ slotDone(); });

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
#ifdef TORA_EXPERIMENTAL
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

        ConnectInfo = new toResultItem(1, true, ResultTab);
        ConnectInfo->setSQL(SQLConnectInfo);
        ResultTab->addTab(ConnectInfo, tr("Connect Info"));

        PendingLocks = new toResultLock(ResultTab);
        ResultTab->addTab(PendingLocks->view(), tr("Pending Locks"));

        LockedObjects = new toResultTableView(false, false, ResultTab);
        ResultTab->addTab(LockedObjects, tr("Locked Objects"));
        LockedObjects->setSQL(SQLLockedObject);

        AccessedObjects = new toResultTableView(false, false, ResultTab);
        AccessedObjects->setSQL(SQLAccessedObjects);
        ResultTab->addTab(AccessedObjects, tr("Accessing"));

        OpenSplitter = new QSplitter(Qt::Horizontal, ResultTab);
        ResultTab->addTab(OpenSplitter, tr("Open Cursors"));
        OpenCursors = new toResultTableView(true, false, OpenSplitter);
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
#ifdef TORA_EXPERIMENTAL
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

    connect(Sessions->view()->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex &cur, const QModelIndex &old) { slotChangeItem(cur, old); });
    connect(ResultTab, SIGNAL(currentChanged(int)), this, SLOT(slotChangeTab(int)));

    CurrentTab = CurrentStatement;

    ToolMenu = NULL;
    slotRefresh();

    QList<int> list;
    list.append(999);
    list.append(1);
    splitter->setSizes(list);

// dunno what is this
#if 0
    QList<toResultItem*> resultItems = findChildren<toResultItem*>();
    for(toResultItem* i: resultItems)
    {
    	i->installEventFilter(this);
    }
    QList<toResultTableView*> resultViews = findChildren<toResultTableView*>();
    for(toResultItem* i: resultItems)
    {
    	i->installEventFilter(this);
    }
#endif

    setFocusProxy(Sessions->view());
}

toSession::~toSession()
{
    if (SessionFilter)
    {
        //Sessions->setFilter(NULL);
        //delete SessionFilter;
    }
}

bool toSession::canHandle(const toConnection &conn)
{
    return conn.providerIs("Oracle") || conn.providerIs("QPSQL");
}

const QString toSession::DISCONNECT = "ALTER SYSTEM DISCONNECT SESSION '%1,%2' %3;\n";
const QString toSession::ALTER = "ALTER SYSTEM KILL SESSION '%1,%2' %3 ;\n";
const QString toSession::KPROC =
        "begin\n"
        " %KILL_PROC%(\n"
        "%SID%\n"
        "%SERIAL%\n"
        "%INST_ID%"
        "%IMMEDIAT%"
        " );\n"
        "end;\n";

QString toSession::sessionKillProcOracle(Session::KillSessionModeEnum mode, const QMap<QString,QString> params)
{
    bool useKillProc = toConfigurationNewSingle::Instance().option(Session::KillProcUseKillProcBool).toBool();
    QString killProcName        = toConfigurationNewSingle::Instance().option(Session::KillProcName).toString();
    QString killProcSid         = toConfigurationNewSingle::Instance().option(Session::KillProcSID).toString();
    QString killProcSerial      = toConfigurationNewSingle::Instance().option(Session::KillProcSerial).toString();
    QString killProcInstance    = toConfigurationNewSingle::Instance().option(Session::KillProcInstance).toString();
    QString killProcImmediate   = toConfigurationNewSingle::Instance().option(Session::KillProcImmediate).toString();
    bool killProcInstanceBool   = toConfigurationNewSingle::Instance().option(Session::KillProcInstanceBool).toBool();
    bool killProcImmediateBool  = toConfigurationNewSingle::Instance().option(Session::KillProcImmediateBool).toBool();

    QString retval;

    if (!useKillProc)
    {
        if (mode == Session::Disconnect)
            retval = DISCONNECT.arg(params.value("SID")).arg(params.value("SERIAL")).arg(params.value("IMMEDIAT") == "true" ? "IMMEDIATE" : "POST_TRANSACTION");
        else
            retval = ALTER.arg(params.value("SID")).arg(params.value("SERIAL")).arg(params.value("IMMEDIAT") == "true" ? "IMMEDIATE" : "");
    } else {
        retval = QString(KPROC)
            .replace("%KILL_PROC%", killProcName)
            .replace("%SID%", QString("   %1 => %2").arg(killProcSid).arg(params.value("SID")))
            .replace("%SERIAL%", QString("   , %1 => %2").arg(killProcSerial).arg(params.value("SERIAL")));
        if (killProcInstanceBool)
            retval = retval.replace("%INST_ID%", QString("   , %1 => %2\n").arg(killProcInstance).arg(params.value("INST_ID")));
        else
            retval = retval.replace("%INST_ID%", QString(""));

        if (killProcImmediateBool)
            retval = retval.replace("%IMMEDIAT%", QString("   , %1 => %2\n").arg(killProcImmediate).arg(params.value("IMMEDIAT")));
        else
            retval = retval.replace("%IMMEDIAT%", QString(""));
    }

    return retval;
}

void toSession::slotSelectAll()
{
    //Sessions->selectAll();
}

void toSession::slotSelectNone()
{
    //Sessions->clearSelection();
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
        QModelIndex item = Sessions->view()->currentIndex();
        if (item.isValid())
        {
            Session = Sessions->data(item.siblingAtColumn(1)).toString(); // Qt 5.11
            Serial  = Sessions->data(item.siblingAtColumn(2)).toString(); // Qt 5.11
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
        Sessions->refreshWithParams(toQueryParams());
    }
    TOCATCH;
}

void toSession::slotDone(void)
{
    int system = 0;
    int total  = 0;
    int active = 0;
#if 0
    for (toResultTableView::iterator it(Sessions); (*it).isValid(); it++)
    {
        QString session = Sessions->data((*it).row(), 1).toString();
        QString serial  = Sessions->data((*it).row(), 2).toString();
        QString user    = Sessions->data((*it).row(), 9).toString();
        QString act     = Sessions->data((*it).row(), 4).toString();

        if (session == Session && serial == Serial)
        {
            Sessions->view()->selectionModel()->select(
                QItemSelection(*it, *it),
                QItemSelectionModel::ClearAndSelect);
            Sessions->view()->setCurrentIndex(*it);
        }

        total++;
        if (user != "USER")
            system++;
        else if (act == "ACTIVE")
            active++;
    }
#endif
    Total->setText(QString("Total <B>%1</B> (Active <B>%3</B>, System <B>%2</B>)")
                   .arg(total).arg(system).arg(active));
}

void toSession::enableStatistics(bool enable)
{
    TLOG(0, toDecorator, __HERE__) << std::endl;
//    QString sql;
//    if (enable)
//        sql = QString::fromLatin1("ALTER SYSTEM SET TIMED_STATISTICS = TRUE");
//    else
//        sql = QString::fromLatin1("ALTER SYSTEM SET TIMED_STATISTICS = FALSE");
//    try
//    {
//        toConnectionSubLoan conn(connection());
//        conn.execute(sql);
//    }
//    catch (...)
//    {
//        Utils::toStatusMessage(tr("No access to timed statistics flags"));
//    }
//    Sessions->model()->setHeaderData()

    toQueryAbstr::HeaderList &headers = Sessions->headers();
    for(auto& h : headers)
    {
        if (h.name_orig != "Action")
            continue;

        if (h.name.startsWith(' '))
        {
            h.name = h.name.remove(0, 1);
        } else {
            h.name = QString(" ").append(h.name);
        }
    }
    Sessions->view()->applyColumnRules(); //TODO
    int h1 = Sessions->view()->height();
    int h2 = Sessions->view()->viewport()->height();
    int h3 = Sessions->rowCount();
}

void toSession::slotChangeTab(int index)
{
    QWidget *tab = ResultTab->widget(index);

    if (tab != CurrentTab)
    {
        CurrentTab = tab;
        QModelIndex item = Sessions->view()->currentIndex();

        if (!item.isValid())
            return;

        int idx1 = Sessions->headers().indexOf("Id");
        int idx2 = Sessions->headers().indexOf("Serial#");
        QString connectionId = Sessions->data(item.siblingAtColumn(idx1)).toString(); // Qt 5.11
        QString serial       = Sessions->data(item.siblingAtColumn(idx2)).toString(); // Qt 5.11

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
        else if (PendingLocks && CurrentTab == PendingLocks->view())
        {
            PendingLocks->refreshWithParams(toQueryParams() << connectionId);
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
// TODO (PostgreSQL, MySQL)
            int idx1 = Sessions->headers().indexOf(" SQL_ID");
            int idx2 = Sessions->headers().indexOf(" SQL_CHILD_NUMBER");
            QString sql_id = Sessions->data(item.siblingAtColumn(idx1)).toString();
            QString cursor = Sessions->data(item.siblingAtColumn(idx2)).toString();
            CurrentStatement->changeAddress(toQueryParams() << sql_id << cursor);
        }
        else if (CurrentTab == PreviousStatement)
        {
// TODO (PostgreSQL, MySQL)
            int idx1 = Sessions->headers().indexOf(" PREV_SQL_ID");
            int idx2 = Sessions->headers().indexOf(" PREV_CHILD_NUMBER");
            QString sql_id = Sessions->data(item.siblingAtColumn(idx1)).toString();
            QString cursor = Sessions->data(item.siblingAtColumn(idx2)).toString();
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
    QModelIndex sql_id_item = OpenCursors->selectedIndex(3);
    QModelIndex cursor_item = OpenCursors->selectedIndex(2);
    if (sql_id_item.isValid() && cursor_item.isValid())
        OpenStatement->changeAddress(toQueryParams() << sql_id_item.data().toString() << cursor_item.data().toString());
}

void toSession::slotCancelBackend()
{
    QModelIndexList selected = Sessions->view()->selectionModel()->selectedRows(1);
    foreach(QModelIndex item, selected)
    {
        if (!item.isValid())
            return;

        try
        {
            toConnectionSubLoan conn(connection());
            conn.execute(QString("SELECT pg_cancel_backend ( %1 )").arg(item.data().toString()));
        }
        TOCATCH;
    }
}

void toSession::slotDisconnectSession()
{
    QModelIndexList selected = Sessions->view()->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return;

    QString str(tr("Let transaction(s) finish before disconnecting?"));
    QString sql;

#if 0
	toSessionDisconnect *widget = new toSessionDisconnect(Sessions, this, "toSessionDisconnect");
    switch(widget->exec())
    {
    case toSessionDisconnect::Accepted:
        break;
    case toSessionDisconnect::Rejected:
        break;
    case toSessionDisconnect::Copy:
        break;
    default:
        Utils::toStatusMessage(tr("Aborted execution"), false, false);
        throw tr("Aborted execution");
    }
#endif
}

void toSession::slotChangeItem(const QModelIndex &current, const QModelIndex &previous)
{
    if (!current.isValid())
        return;

    QString item = Sessions->data(current.siblingAtColumn(1)).toString();
    if (LastSession != item)
    {
        if (!item.isEmpty())
        {
#ifdef TORA_EXPERIMENTAL
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
    slotChangeItem(Sessions->view()->currentIndex(), QModelIndex());
}


void toSession::slotFilterChanged(const QString &text)
{
#if 0
    SessionFilter->setFilterString(text);
    Sessions->applyFilter();
#endif
}

bool toSession::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        if (obj != Sessions)
        {
        	if (toResult *result = dynamic_cast<toResult*>(obj))
            {
        		result->refresh();
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

toSessionDisconnect::toSessionDisconnect(toResultTableView *sessionView, QWidget *parent, const char *name)
    : QDialog(parent)
    , SessionView(sessionView)
{
    setupUi(this);
    progressBar->setHidden(true);

    SQLText->setMarginWidth(2, 0);

    int killMode = toConfigurationNewSingle::Instance().option(Session::KillSessionModeInt).toInt();
    DisconnectRdio->setChecked(killMode==Session::Disconnect);
    KillRdio->setChecked(killMode==Session::Kill);
    KImmediateRdio->setChecked(killMode==Session::KillImmediate);

    switch(killMode)
    {
    case Session::Disconnect:
        slotKillDisconnect();
        break;
    case Session::Kill:
        slotKill();
        break;
    case Session::KillImmediate:
        slotKillImmediate();
        break;
    }

    connect(DisconnectBtn, SIGNAL(clicked()), this, SLOT(slotExecuteAll()));
    connect(CopyBtn, SIGNAL(clicked()), this, SLOT(slotCopy()));
    connect(CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

    connect(DisconnectRdio, SIGNAL(pressed()), this, SLOT(slotKillDisconnect()));
    connect(KillRdio, SIGNAL(pressed()), this, SLOT(slotKill()));
    connect(KImmediateRdio, SIGNAL(pressed()), this, SLOT(slotKillImmediate()));
}

void toSessionDisconnect::slotKillDisconnect()
{
    toConfigurationNewSingle::Instance().setOption(Session::KillSessionModeInt, Session::Disconnect);

    QModelIndexList selected = SessionView->selectionModel()->selectedRows();
    QString sql;

    foreach(QModelIndex item, selected)
    {
        if (!item.isValid())
            continue;

        QMap<QString,QString> params;
        QString sid          = SessionView->model()->data(item.row(), 1).toString();
        QString serial       = SessionView->model()->data(item.row(), 2).toString();
        params["SID"] = sid;
        params["SERIAL"] = serial;
        params["IMMEDIAT"] = "true";
        sql += toSession::sessionKillProcOracle(Session::Disconnect, params);
        sql += "\n";
    }
    SQLText->setText(sql);
}

void toSessionDisconnect::slotKill()
{
    toConfigurationNewSingle::Instance().setOption(Session::KillSessionModeInt, Session::Kill);
    QModelIndexList selected = SessionView->selectionModel()->selectedRows();
    QString sql;

    foreach(QModelIndex item, selected)
    {
        if (!item.isValid())
            continue;

        QMap<QString,QString> params;
        QString sid          = SessionView->model()->data(item.row(), 1).toString();
        QString serial       = SessionView->model()->data(item.row(), 2).toString();
        params["SID"] = sid;
        params["SERIAL"] = serial;
        params["IMMEDIAT"] = "false";
        sql += toSession::sessionKillProcOracle(Session::Kill, params);
        sql += "\n";
    }

    SQLText->setText(sql);
}

void toSessionDisconnect::slotKillImmediate()
{
    toConfigurationNewSingle::Instance().setOption(Session::KillSessionModeInt, Session::KillImmediate);
    QModelIndexList selected = SessionView->selectionModel()->selectedRows();
    QString sql;

    foreach(QModelIndex item, selected)
    {
        if (!item.isValid())
            continue;

        QMap<QString,QString> params;
        QString sid          = SessionView->model()->data(item.row(), 1).toString();
        QString serial       = SessionView->model()->data(item.row(), 2).toString();
        params["SID"] = sid;
        params["SERIAL"] = serial;
        params["IMMEDIAT"] = "true";
        sql += toSession::sessionKillProcOracle(Session::KillImmediate, params);
        sql += "\n";
    }
    SQLText->setText(sql);
}

void toSessionDisconnect::slotCopy()
{
    this->setResult(Copy);
    SQLText->selectAll();
    SQLText->copy();
    close();
}

toSessionSetting::toSessionSetting(toTool *tool, QWidget* parent, const char* name)
    : QWidget(parent)
    , toSettingTab("TODO")
    , Tool(tool)
{
    setupUi(this);
    toSettingTab::loadSettings(this);

    SQLText->setMarginWidth(2, 0);

    connect(radioButtonKillProc, SIGNAL(toggled(bool)), this, SLOT(killProcToggled(bool)));
    connect(radioButtonKillProc, SIGNAL(toggled(bool)), this, SLOT(composeKillProc()));
    connect(KillProcName, SIGNAL(textChanged(const QString&)), this, SLOT(composeKillProc()));
    connect(KillProcSID, SIGNAL(textChanged(const QString&)), this, SLOT(composeKillProc()));
    connect(KillProcSerial, SIGNAL(textChanged(const QString&)), this, SLOT(composeKillProc()));
    connect(KillProcInstance, SIGNAL(textChanged(const QString&)), this, SLOT(composeKillProc()));
    connect(KillProcImmediate, SIGNAL(textChanged(const QString&)), this, SLOT(composeKillProc()));
    connect(KillProcInstance, SIGNAL(stateChanged(int)), this, SLOT(composeKillProc()));
    connect(KillProcInstanceBool, SIGNAL(stateChanged(int)), this, SLOT(composeKillProc()));
    connect(KillProcImmediateBool, SIGNAL(stateChanged(int)), this, SLOT(composeKillProc()));

    toSettingTab::loadSettings(this);
    radioButtonAlter->setChecked(!toConfigurationNewSingle::Instance().option(Session::KillProcUseKillProcBool).toBool());
    radioButtonKillProc->setChecked(toConfigurationNewSingle::Instance().option(Session::KillProcUseKillProcBool).toBool());
}

void toSessionSetting::saveSetting(void)
{
    toSettingTab::saveSettings(this);
}

static toSQL SQLKillProcName(
    "toSession:QueryKillProcName",
    "select owner, object_name from all_procedures where object_name like 'KILL%' and rownum = 1",
    "Guess name of kill_session procedure",
    "0801",
    "Oracle"
);

static toSQL SQLKillProcArgs(
    "toSession:QueryKillProcArgs",
    "select ARGUMENT_NAME from ALL_ARGUMENTS where owner = :owner<char[32],in> and object_name = :name<char[32],in> order by position",
    "Guess parameter names of kill_session procedure",
    "0801",
    "Oracle"
);

void toSessionSetting::killProcToggled(bool toggled)
{
    KillProcInstance->setEnabled(toggled && KillProcInstanceBool->isChecked());
    KillProcImmediate->setEnabled(toggled && KillProcImmediateBool->isChecked());
    toConfigurationNewSingle::Instance().setOption(Session::KillProcUseKillProcBool, toggled);

    if (!toggled)
        return;
    try
    {
        // Check if connection exists
        toConnection &conn = toConnectionRegistrySing::Instance().currentConnection();
        if (conn.providerIs("Oracle"))
        {
            toConnectionSubLoan connSub(conn);
            QString owner, name;
            {
                toQuery queryKill(connSub, SQLKillProcName, toQueryParams());
                if (queryKill.eof())
                    return;
                owner = (QString)queryKill.readValue();
                name  = (QString)queryKill.readValue();
                KillProcName->setText(owner + '.' + name);
            }
            {
                toQuery queryKillArgs(connSub, SQLKillProcArgs, toQueryParams() << owner << name);
                if (!queryKillArgs.eof())
                    KillProcSID->setText((QString)queryKillArgs.readValue());
                if (!queryKillArgs.eof())
                    KillProcSerial->setText((QString)queryKillArgs.readValue());
                if (!queryKillArgs.eof())
                    KillProcImmediate->setText((QString)queryKillArgs.readValue());
                if (!queryKillArgs.eof())
                    KillProcInstance->setText((QString)queryKillArgs.readValue());
            }
        }
    }
	catch (QString const &s)
	{
		QString a = s;
		std::cout << qPrintable(a) << std::endl;
	}
    catch (...)
    {
    }

}

void toSessionSetting::composeKillProc()
{
    QString retval;
    if (radioButtonAlter->isChecked())
    {
        retval = toSession::ALTER;
    } else {
		retval = QString(toSession::KPROC)
			.replace("%KILL_PROC%", KillProcName->text())
			.replace("%SID%", QString("   %1 => :SID").arg(KillProcSID->text()))
			.replace("%SERIAL%", QString("   , %1 => :SERIAL").arg(KillProcSerial->text()));
		if (KillProcInstanceBool->isChecked())
		    retval = retval.replace("%INST_ID%", QString("   , %1 => :INST_ID\n").arg(KillProcInstance->text()));
		else
		    retval = retval.replace("%INST_ID%", QString(""));

		if (KillProcImmediateBool->isChecked())
		    retval = retval.replace("%IMMEDIAT%", QString("   , %1 => :IMMEDIAT\n").arg(KillProcImmediate->text()));
		else
		    retval = retval.replace("%IMMEDIAT%", QString(""));
    }
    SQLText->setText(retval);
};

void toSessionDisconnect::slotExecuteAll()
{
    toSyntaxAnalyzerNL analyzer(SQLText);
    toSyntaxAnalyzer::statementList stats = analyzer.getStatements(SQLText->text());

    int currentStatement = 1;
    progressBar->setMaximum(1);
    progressBar->setMaximum(stats.size());
    progressBar->setVisible(true);

    toConnection &conn = toConnection::currentConnection(SessionView);

    Q_FOREACH(toSyntaxAnalyzer::statement statement, stats)
    {
        progressBar->setValue(currentStatement);
        analyzer.sanitizeStatement(statement);
        SQLText->SendScintilla(QsciScintilla::SCI_SETSEL, statement.posFrom, statement.posTo);

        qApp->processEvents();

        try
        {
        	toConnectionSubLoan c(conn);
            toQuery query(c, statement.sql, toQueryParams());
        	query.eof();
        }
        catch (QString const& e)
        {
        	Utils::toStatusMessage(e, true, true);
        }
        currentStatement++;
    }
    CancelBtn->setText(tr("Close"));
}
