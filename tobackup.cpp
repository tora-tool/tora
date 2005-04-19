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

#include "tobackup.h"
#include "tochangeconnection.h"
#include "toconf.h"
#include "tomain.h"
#include "toresultlong.h"
#include "tosql.h"
#include "totool.h"

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <qlabel.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include "tobackup.moc"

#include "icons/refresh.xpm"
#include "icons/tobackup.xpm"

class toBackupTool : public toTool
{
protected:
    std::map<toConnection *, QWidget *> Windows;

    virtual const char **pictureXPM(void)
    {
        return tobackup_xpm;
    }
public:
    toBackupTool()
            : toTool(240, "Backup Manager")
    { }
    virtual const char *menuItem()
    {
        return "Backup Manager";
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
            QWidget *window = new toBackup(parent, connection);
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

static toBackupTool BackupTool;

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
                           "8.0");

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
                            "7.3");

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
                             "8.0");

static toSQL SQLOnlineBackup7("toBackup:OnlineBackup",
                              "SELECT 0 FROM DUAL",
                              "",
                              "7.3");

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
                           "8.0");

static toSQL SQLLastBackup7("toBackup:LastBackup",
                            "SELECT 'Backup information not available in Oracle 7' \"Unsupported\" FROM DUAL",
                            "",
                            "7.3");

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
                              "8.1");

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
                               "8.0");

static toSQL SQLCurrentBackup7("toBackup:CurrentBackup",
                               "SELECT * FROM DUAL WHERE NULL = NULL",
                               "",
                               "7.3");

toBackup::toBackup(QWidget *main, toConnection &connection)
        : toToolWidget(BackupTool, "backup.html", main, connection)
{
    QToolBar *toolbar = toAllocBar(this, tr("Backup Manager"));

    new QToolButton(QPixmap((const char **)refresh_xpm),
                    tr("Update"),
                    tr("Update"),
                    this, SLOT(refresh(void)),
                    toolbar);
    toolbar->setStretchableWidget(new QLabel(toolbar, TO_KDE_TOOLBAR_WIDGET));
    new toChangeConnection(toolbar, TO_KDE_TOOLBAR_WIDGET);

    Tabs = new QTabWidget(this);

    QVBox *box = new QVBox(Tabs, "history");
    new QLabel(tr("Logswitches per day and hour"), box);
    LogSwitches = new toResultLong(true, false, toQuery::Background, box);
    LogSwitches->setSQL(SQLLogSwitches);
    Tabs->addTab(box, tr("Redo Switches"));

    LogHistory = new toResultLong(true, false, toQuery::Background, Tabs);
    LogHistory->setSQL(SQLLogHistory);
    Tabs->addTab(LogHistory, tr("Archived Logs"));

    box = new QVBox(Tabs);
    LastLabel = new QLabel(box);
    LastBackup = new toResultLong(true, false, toQuery::Background, box);
    LastBackup->setSQL(SQLLastBackup);
    Tabs->addTab(box, tr("Last Backup"));

    CurrentBackup = new toResultLong(true, false, toQuery::Background, Tabs);
    CurrentBackup->setSQL(SQLCurrentBackup);
    Tabs->addTab(CurrentBackup, tr("Backup Progress"));

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(windowActivated(QWidget *)),
            this, SLOT(windowActivated(QWidget *)));

    refresh();

    setFocusProxy(Tabs);
}

void toBackup::windowActivated(QWidget *widget)
{
    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QPopupMenu(this);
            ToolMenu->insertItem(QPixmap((const char **)refresh_xpm), tr("&Refresh"),
                                 this, SLOT(refresh(void)), toKeySequence(tr("F5", "Backup|Refresh")));
            toMainWidget()->menuBar()->insertItem(tr("&Backup Manager"), ToolMenu, -1, toToolMenuIndex());
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
        BackupTool.closeWindow(connection());
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

