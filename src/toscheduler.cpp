
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

#include <QSettings>
#include <QToolBar>
#include <QtDebug>

#include "toscheduler.h"
#include "tosql.h"
#include "utils.h"
#include "toresultschema.h"
#include "tochangeconnection.h"
#include "toconnection.h"

#include "icons/refresh.xpm"
#include "icons/scheduler.xpm"


static toSQL SQLJobsList("toScheduler:JobsList",
                            "select JOB, LOG_USER, PRIV_USER, SCHEMA_USER,\n"
                            "       LAST_DATE, LAST_SEC, THIS_DATE, THIS_SEC,\n"
                            "       NEXT_DATE, NEXT_SEC, TOTAL_TIME, BROKEN,\n"
                            "       INTERVAL, FAILURES, WHAT, NLS_ENV,\n"
                            "       INSTANCE\n"
                            "   from dba_jobs\n"
                            "   where schema_user = upper(:f1<char[101]>)\n"
                            "   order by job\n",
                            "List DB Jobs (not scheduler related). Don't change columns order.",
                          "0800");

static toSQL SQLJobsSubmit("toScheduler:JobsSubmit",
                            "DECLARE\n"
                            "   jobno NUMBER;\n"
                            "BEGIN\n"
                            "   sys.dbms_job.submit(job => jobno,"
                            "                       what => :f1<char[4000]>,"
                            "                       next_date => to_date(:f2<char[101]>, \"yyyymmdd hh24miss\"),"
                            "                       interval => :f3<char[101]>"
                            "                      );\n"
                            "END;",
                            "Submit a DB Job (not scheduler related).",
                            "0800");

static toSQL SQLJobsChange("toScheduler:JobsChange",
                            "BEGIN\n"
                            "   sys.dbms_job.chamge(job => to_number(:f1<char[101]>),"
                            "                       what => :f1<char[4000]>,"
                            "                       next_date => to_date(:f2<char[101]>, \"yyyymmdd hh24miss\"),"
                            "                       interval => :f3<char[101]>"
                            "                      );\n"
                            "END;",
                            "Change a DB Job (not scheduler related).",
                            "0800");


class toSchedulerTool : public toTool
{
    protected:
        virtual const char **pictureXPM(void)
        {
            return const_cast<const char**>(scheduler_xpm);
        }
    public:
        toSchedulerTool()
        : toTool(290, "Scheduler/Jobs Manager") { }
        virtual const char *menuItem()
        {
            return "Scheduler/Jobs Manager";
        }
        virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
        {
            return new toScheduler(parent, connection);
        }
//         virtual QWidget *configurationTab(QWidget *parent)
//         {
//             return new toSGATracePrefs(this, parent);
//         }
        virtual bool canHandle(toConnection &conn)
        {
            return toIsOracle(conn);
        }
        virtual void closeWindow(toConnection &connection) {};
};


static toSchedulerTool SchedulerTool;


// TODO/FIXME : help file
toScheduler::toScheduler(QWidget * parent, toConnection &connection)
    : toToolWidget(SchedulerTool, "scheduler.html", parent, connection, "toScheduler"),
    m_owner("")
{
    setObjectName("scheduler");

    m_owner = connection.user();

    QToolBar *toolbar = toAllocBar(this, tr("SchedulerBrowser"));
    layout()->addWidget(toolbar);

    QAction *refreshAct =
            toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                               tr("Refresh list"),
                               this, SLOT(refresh()));
    refreshAct->setShortcut(QKeySequence::Refresh);

    // TODO/FIXME : icon
    toolbar->addAction(//QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                        tr("New Job"),
                        this, SLOT(jobNew()));

    // TODO/FIXME : icon
    toolbar->addAction(//QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                        tr("Copy of the current Job"),
                        this, SLOT(jobCopyOf()));

    toolbar->addWidget(new toSpacer());

    QLabel * labSchema = new QLabel(tr("Schema") + " ", toolbar);
    toolbar->addWidget(labSchema);
    Schema = new toResultSchema(connection, toolbar,
                                "SchedulerResultSchema");
    try
    {
        Schema->refresh();
    }
    catch (...) {}
    connect(Schema, SIGNAL(activated(const QString &)),
            this, SLOT(changeSchema(const QString &)));
    toolbar->addWidget(Schema);

    new toChangeConnection(toolbar, "SchedulerChangeConnection");

    m_gui = new toSchedulerWidget(this);
    m_predefinedIntervals = m_gui->intervalComboBox->count() - 1;

    layout()->addWidget(m_gui);

    m_gui->jobsTableView->setSQL(SQLJobsList);
    m_gui->jobsTableView->setReadAll(true);
    connect(m_gui->jobsTableView, SIGNAL(selectionChanged()),
             this, SLOT(jobsTableView_selectionChanged()));
    connect(m_gui->jobSaveButton, SIGNAL(clicked()),
             this, SLOT(jobSaveButton_clicked()));

    refresh();
}

toScheduler::~toScheduler()
{
    QSettings s;
    s.beginGroup("toScheduler");
    s.endGroup();
}

bool toScheduler::canHandle(toConnection &conn)
{
    try
    {
        return toIsOracle(conn);
    }
    TOCATCH
    return false;
}

void toScheduler::changeSchema(const QString & name)
{
    Q_UNUSED(name)
    refresh();
}

void toScheduler::refresh()
{
    if (!Schema->currentText().isEmpty())
        m_owner = Schema->currentText();
    else
        m_owner = connection().user().toUpper();

    m_gui->jobsTableView->changeParams(/*connection(), */m_owner);
}

#define JOBDATA(column) m_gui->jobsTableView->selectedIndex((column)).data(Qt::EditRole).toString()
void toScheduler::jobsTableView_selectionChanged()
{
    m_gui->jobEdit->setText(JOBDATA(1));
    QDateTime dt(QDateTime::fromString(JOBDATA(7)));
    m_gui->nextDateTimeEdit->setDateTime(dt);

    QString interval(JOBDATA(13));
    int ix = m_gui->intervalComboBox->findText(interval);
    if (ix != -1)
        m_gui->intervalComboBox->setCurrentIndex(ix);
    else
    {
        m_gui->intervalComboBox->addItem(interval);
        m_gui->intervalComboBox->setCurrentIndex(m_gui->intervalComboBox->count()-1);
    }

    m_gui->whatEditor->setText(JOBDATA(15));
}
#include <QtDebug>
void toScheduler::jobSaveButton_clicked()
{
    QString interval(m_gui->intervalComboBox->currentText());
    if (m_gui->intervalComboBox->currentIndex() < m_predefinedIntervals) // predefined intervals
    {
        int start = interval.indexOf("(") + 1;
        int end = interval.indexOf(")");
        interval = interval.mid(start, end-start);
    }
    qDebug() << "params:" << m_gui->jobEdit->text() << m_gui->whatEditor->text() << m_gui->nextDateTimeEdit->dateTime().toString("yyyyMMdd hhmmss") << interval;
    if (m_gui->jobEdit->text().isEmpty())
    {
        qDebug() << "1";
        // submit
        connection().execute(SQLJobsSubmit,
                              m_gui->whatEditor->text(),
                              m_gui->nextDateTimeEdit->dateTime().toString("yyyyMMdd hhmmss"),
                              interval);
        qDebug() << "2";
    }
    else
    {
        // change
        connection().execute(SQLJobsChange,
                              m_gui->jobEdit->text(),
                              m_gui->whatEditor->text(),
                              m_gui->nextDateTimeEdit->dateTime().toString("yyyyMMdd hhmmss"),
                              interval);
    }
}

void toScheduler::jobNew()
{
    jobCopyOf();
    m_gui->nextDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    m_gui->intervalComboBox->setCurrentIndex(0);
    m_gui->whatEditor->clear();
}

void toScheduler::jobCopyOf()
{
    m_gui->jobEdit->clear();
    m_gui->nextDateTimeEdit->setFocus(Qt::OtherFocusReason);
}
