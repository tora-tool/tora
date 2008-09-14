
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

#include "tobackup.h"
#include "tochangeconnection.h"
#include "toconf.h"
#include "tomain.h"
#include "toresulttableview.h"
#include "tosql.h"
#include "totool.h"

#include <qlabel.h>
#include <qmenubar.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <QMdiArea>

#include <QPixmap>
#include <QVBoxLayout>

#include <loki/Singleton.h>

#include "icons/refresh.xpm"


//static toBackupTool BackupTool;

static toSQL SQLLogSwitches("toBackup:LogSwitches",
                            "SELECT trunc(first_time)\"Date\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               0,\n"
                            "                               1))),\n"
                            "           ' ')\"-00\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               1,\n"
                            "                               1))),\n"
                            "           ' ')\"-01\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               2,\n"
                            "                               1))),\n"
                            "           ' ')\"-02\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               3,\n"
                            "                               1))),\n"
                            "           ' ')\"-03\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               4,\n"
                            "                               1))),\n"
                            "           ' ')\"-04\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               5,\n"
                            "                               1))),\n"
                            "           ' ')\"-05\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               6,\n"
                            "                               1))),\n"
                            "           ' ')\"-06\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               7,\n"
                            "                               1))),\n"
                            "           ' ')\"-07\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               8,\n"
                            "                               1))),\n"
                            "           ' ')\"-08\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               9,\n"
                            "                               1))),\n"
                            "           ' ')\"-09\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               10,\n"
                            "                               1))),\n"
                            "           ' ')\"-10\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               11,\n"
                            "                               1))),\n"
                            "           ' ')\"-11\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               12,\n"
                            "                               1))),\n"
                            "           ' ')\"-12\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               13,\n"
                            "                               1))),\n"
                            "           ' ')\"-13\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               14,\n"
                            "                               1))),\n"
                            "           ' ')\"-14\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               15,\n"
                            "                               1))),\n"
                            "           ' ')\"-15\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               16,\n"
                            "                               1))),\n"
                            "           ' ')\"-16\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               17,\n"
                            "                               1))),\n"
                            "           ' ')\"-17\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               18,\n"
                            "                               1))),\n"
                            "           ' ')\"-18\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               19,\n"
                            "                               1))),\n"
                            "           ' ')\"-19\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               20,\n"
                            "                               1))),\n"
                            "           ' ')\"-20\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               21,\n"
                            "                               1))),\n"
                            "           ' ')\"-21\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               22,\n"
                            "                               1))),\n"
                            "           ' ')\"-22\",\n"
                            "       nvl(to_char(SUM (decode(to_number(to_char(first_time,\n"
                            "                                                 'HH24')),\n"
                            "                               23,\n"
                            "                               1))),\n"
                            "           ' ')\"-23\"\n"
                            "  FROM v$log_history\n"
                            " GROUP BY trunc(first_time)\n"
                            " ORDER BY trunc(first_time)DESC",
                            "Log switch history");

static toSQL SQLLogHistory("toBackup:LogHistory",
                           "SELECT\n"
                           "   THREAD# \"Thread#\",\n"
                           "   SEQUENCE# \"Sequence#\",\n"
                           "   FIRST_TIME \"Time of first entry\",\n"
                           "   FIRST_CHANGE# \"Lowest  SCN\",\n"
                           "   NEXT_CHANGE# \"Highest SCN\",\n"
                           "   RECID \"Controlfile RecID\",\n"
                           "   STAMP \"Controlfile stamp\"\n"
                           "FROM V$LOG_HISTORY\n"
                           "ORDER BY SEQUENCE# DESC",
                           "Redo Log History",
                           "0800");

static toSQL SQLLogHistory7("toBackup:LogHistory",
                            "SELECT \n"
                            "   THREAD# \"Thread#\",\n"
                            "   SEQUENCE# \"Sequence#\",\n"
                            "   TIME \"Time of first entry\",\n"
                            "   LOW_CHANGE# \"Lowest  SCN\",\n"
                            "   (HIGH_CHANGE#-1) \"Highest SCN\",\n"
                            "   ARCHIVE_NAME \"Archived log name\"\n"
                            "FROM V$LOG_HISTORY\n"
                            "ORDER BY SEQUENCE# DESC",
                            "",
                            "0703");

static toSQL SQLOnlineBackup("toBackup:OnlineBackup",
                             "SELECT MIN(VBD.CHECKPOINT_CHANGE#)-MAX(VBD.CHECKPOINT_CHANGE#)\n"
                             "  FROM V$BACKUP_DATAFILE VBD, V$DATAFILE VDF,\n"
                             "       (SELECT CREATION_CHANGE#,\n"
                             "               MAX(COMPLETION_TIME) COMPLETION_TIME\n"
                             "          FROM V$BACKUP_DATAFILE \n"
                             "         WHERE CREATION_CHANGE# IN (SELECT CREATION_CHANGE# FROM V$DATAFILE)\n"
                             "         GROUP BY CREATION_CHANGE#) QUERY1\n"
                             " WHERE VBD.CREATION_CHANGE# = VDF.CREATION_CHANGE#\n"
                             "   AND VBD.CREATION_CHANGE# = QUERY1.CREATION_CHANGE#\n"
                             "   AND VBD.COMPLETION_TIME = QUERY1.COMPLETION_TIME",
                             "Check if online backup or not. Should return 0 if cold backup",
                             "0800");

static toSQL SQLOnlineBackup7("toBackup:OnlineBackup",
                              "SELECT 0 FROM DUAL",
                              "",
                              "0703");

static toSQL SQLLastBackup("toBackup:LastBackup",
                           "SELECT VDF.NAME \"Filename\",\n"
                           "       VBD.COMPLETION_TIME \"Completion time\",\n"
                           "       CASE WHEN VBD.COMPLETION_TIME > VDF.unrecoverable_time THEN 'Unrecoverable change' ELSE 'Backed up' END \"Type\",\n"
                           "       VBD.DATAFILE_BLOCKS*VBD.BLOCK_SIZE \"File size\",\n"
                           "       VBD.BLOCKS*VBD.BLOCK_SIZE \"Bytes written\",\n"
                           "       VBD.INCREMENTAL_LEVEL \"Level\",\n"
                           "       VBD.CHECKPOINT_CHANGE# \"Ckpt change#\",\n"
                           "       VBD.CHECKPOINT_TIME \"Checkpoint time\",\n"
                           "       VBD.MARKED_CORRUPT \"Mrkd crpt\",\n"
                           "       VBD.MEDIA_CORRUPT \"Media crpt\",\n"
                           "       VBD.LOGICALLY_CORRUPT \"Logically corrupt\"\n"
                           "  FROM V$BACKUP_DATAFILE VBD, V$DATAFILE VDF,\n"
                           "       (SELECT CREATION_CHANGE#,\n"
                           "               MAX(COMPLETION_TIME) COMPLETION_TIME\n"
                           "          FROM V$BACKUP_DATAFILE\n"
                           "         WHERE CREATION_CHANGE# IN (SELECT CREATION_CHANGE# FROM V$DATAFILE)\n"
                           "         GROUP BY CREATION_CHANGE#) QUERY1\n"
                           " WHERE VBD.CREATION_CHANGE# = VDF.CREATION_CHANGE#\n"
                           "   AND VBD.CREATION_CHANGE# = QUERY1.CREATION_CHANGE#\n"
                           "   AND VBD.COMPLETION_TIME = QUERY1.COMPLETION_TIME\n"
                           "UNION\n"
                           "SELECT NAME,\n"
                           "       CREATION_TIME,\n"
                           "       'Added since last backup',\n"
                           "       Bytes,\n"
                           "       0,\n"
                           "       0,\n"
                           "       TO_NUMBER(NULL),\n"
                           "       TO_DATE(NULL),\n"
                           "       0,\n"
                           "       0,\n"
                           "       0\n"
                           "  FROM V$DATAFILE\n"
                           " WHERE CREATION_CHANGE# NOT IN (SELECT DISTINCT CREATION_CHANGE# FROM V$BACKUP_DATAFILE)\n"
                           " ORDER BY 2 DESC, 5 DESC, 6 DESC",
                           "Get datafiles in most recent backup",
                           "0800");

static toSQL SQLLastBackup7("toBackup:LastBackup",
                            "SELECT 'Backup information not available in Oracle 7' \"Unsupported\" FROM DUAL",
                            "",
                            "0703");

static toSQL SQLCurrentBackup("toBackup:CurrentBackup",
                              "SELECT SID \"Sid\",\n"
                              "       SERIAL# \"Serial#\",\n"
                              "       CONTEXT \"Context\",\n"
                              "       SOFAR \"So far\",\n"
                              "       TOTALWORK \"Total\",\n"
                              "       ROUND(SOFAR/TOTALWORK*100,2) \"% Complete\"\n"
                              "  FROM V$SESSION_LONGOPS\n"
                              " WHERE OPNAME LIKE 'RMAN%'\n"
                              "   AND OPNAME NOT LIKE '%aggregate%'\n"
                              "   AND TOTALWORK != 0\n"
                              "   AND SOFAR <> TOTALWORK",
                              "Display current RMAN progress",
                              "0801");

static toSQL SQLCurrentBackup8("toBackup:CurrentBackup",
                               "SELECT SID \"Sid\",\n"
                               "       SERIAL# \"Serial#\",\n"
                               "       CONTEXT \"Context\",\n"
                               "       SOFAR \"So far\",\n"
                               "       TOTALWORK \"Total\",\n"
                               "       ROUND(SOFAR/TOTALWORK*100,2)   \"% Complete\"\n"
                               "  FROM GV$SESSION_LONGOPS\n"
                               " WHERE COMPNAM = 'dbms_backup_restore'\n"
                               "   AND ROUND(SOFAR/TOTALWORK*100,2) < 100\n"
                               " ORDER BY 1 DESC",
                               "",
                               "0800");

static toSQL SQLCurrentBackup7("toBackup:CurrentBackup",
                               "SELECT * FROM DUAL WHERE NULL = NULL",
                               "",
                               "0703");

toBackup::toBackup(toTool* tool, QWidget *main, toConnection &connection)
        : toToolWidget(*tool, "backup.html", main, connection, "toBackup")
        , tool_(tool)
{
    QToolBar *toolbar = toAllocBar(this, tr("Backup Manager"));
    layout()->addWidget(toolbar);

    updateAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                            tr("Update"), this);
    updateAct->setShortcut(QKeySequence::Refresh);
    connect(updateAct, SIGNAL(triggered()), this, SLOT(refresh(void)));
    toolbar->addAction(updateAct);

    toolbar->addWidget(new toSpacer());

    new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

    Tabs = new QTabWidget(this);
    layout()->addWidget(Tabs);

    QWidget *box = new QWidget(Tabs);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    box->setLayout(vbox);

    vbox->addWidget(new QLabel(tr("Logswitches per day and hour"), box));
    LogSwitches = new toResultTableView(true, false, box);
    LogSwitches->setSQL(SQLLogSwitches);
    vbox->addWidget(LogSwitches);
    Tabs->addTab(box, tr("Redo Switches"));

    LogHistory = new toResultTableView(true, false, Tabs);
    LogHistory->setSQL(SQLLogHistory);
    Tabs->addTab(LogHistory, tr("Archived Logs"));

    box = new QWidget(Tabs);
    vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    box->setLayout(vbox);

    LastLabel = new QLabel(box);
    vbox->addWidget(LastLabel);
    LastBackup = new toResultTableView(true, false, box);
    vbox->addWidget(LastBackup);
    LastBackup->setSQL(SQLLastBackup);
    Tabs->addTab(box, tr("Last Backup"));

    CurrentBackup = new toResultTableView(true, false, Tabs);
    CurrentBackup->setSQL(SQLCurrentBackup);
    Tabs->addTab(CurrentBackup, tr("Backup Progress"));

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(windowActivated(QMdiSubWindow *)));

    refresh();

    setFocusProxy(Tabs);
}

void toBackup::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;
    if (widget->widget() == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Backup Manager"), this);

            ToolMenu->addAction(updateAct);

            toMainWidget()->addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

toBackup::~toBackup()
{
    try
    {
        tool_->closeWindow(connection());
    }
    TOCATCH
}

void toBackup::refresh()
{
    LogSwitches->refresh();
    LogHistory->refresh();
    int val = 0;
    try
    {
        toQuery query(connection(), SQLOnlineBackup);
        val = query.readValue().toInt();
    }
    catch (...)
        {}
    if (val == 0)
        LastLabel->setText(tr("This appears to be a cold backup database"));
    else
        LastLabel->setText(tr("This appears to be a hot backup database"));
    LastBackup->refresh();
    CurrentBackup->refresh();
}

