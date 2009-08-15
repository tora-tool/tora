
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

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "toresultcombo.h"
#include "toresulttableview.h"
#include "toresultresources.h"
#include "toresultview.h"
#include "tosgastatement.h"
#include "tosgatrace.h"
#include "tosql.h"
#include "totool.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <QGroupBox>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpixmap.h>

#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <QMdiArea>

#include <QList>
#include <QMenu>

#include "icons/refresh.xpm"
#include "icons/tosgatrace.xpm"

// #define CONF_AUTO_UPDATE    "AutoUpdate"

class toSGATracePrefs : public QGroupBox, public toSettingTab
{
    QCheckBox* AutoUpdate;
    toTool *Tool;

public:
    toSGATracePrefs(toTool *tool, QWidget* parent = 0, const char* name = 0)
            : QGroupBox(parent), toSettingTab("trace.html"), Tool(tool)
    {
        if (name)
            setObjectName(name);

        QVBoxLayout *vbox = new QVBoxLayout;
        vbox->setSpacing(6);
        vbox->setContentsMargins(11, 11, 11, 11);

        setLayout(vbox);

        setTitle(qApp->translate("toSGATracePrefs", "SGA Trace"));

        AutoUpdate = new QCheckBox(this);
        AutoUpdate->setText(qApp->translate("toSGATracePrefs", "&Auto update"));
        AutoUpdate->setToolTip(qApp->translate("toSGATracePrefs",
                                               "Update automatically after change of schema."));
        vbox->addWidget(AutoUpdate);

        QSpacerItem *spacer = new QSpacerItem(
            20,
            20,
            QSizePolicy::Minimum,
            QSizePolicy::Expanding);
        vbox->addItem(spacer);

//         if (!Tool->config(CONF_AUTO_UPDATE, "Yes").isEmpty())
//             AutoUpdate->setChecked(true);
        AutoUpdate->setChecked(toConfigurationSingle::Instance().autoUpdate());
    }
    virtual void saveSetting(void)
    {
//         if (AutoUpdate->isChecked())
//             Tool->setConfig(CONF_AUTO_UPDATE, "Yes");
//         else
//             Tool->setConfig(CONF_AUTO_UPDATE, "");
        toConfigurationSingle::Instance().setAutoUpdate(AutoUpdate->isChecked());
    }
};

class toSGATraceTool : public toTool
{
protected:
    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(tosgatrace_xpm);
    }
public:
    toSGATraceTool()
            : toTool(230, "SGA Trace") { }
    virtual const char *menuItem()
    {
        return "SGA Trace";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toSGATrace(parent, connection);
    }
    virtual QWidget *configurationTab(QWidget *parent)
    {
        return new toSGATracePrefs(this, parent);
    }
    virtual void closeWindow(toConnection &connection) {};
};

static toSGATraceTool SGATraceTool;

toSGATrace::toSGATrace(QWidget *main, toConnection &connection)
        : toToolWidget(SGATraceTool, "trace.html", main, connection, "toSGATrace")
{
    QToolBar *toolbar = toAllocBar(this, tr("SGA trace"));
    layout()->addWidget(toolbar);

    FetchAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                           tr("Fetch statements in SGA"), this);
    FetchAct->setShortcut(QKeySequence::Refresh);
    connect(FetchAct, SIGNAL(triggered()), this, SLOT(refresh(void)));
    toolbar->addAction(FetchAct);

    toolbar->addSeparator();

    QLabel * labSchema = new QLabel(tr("Schema") + " ", toolbar);
    toolbar->addWidget(labSchema);

    Schema = new toResultCombo(toolbar);
    Schema->additionalItem(tr("Any"));
    Schema->setSelected(connection.user().toUpper());
    Schema->query(toSQL::sql(toSQL::TOSQL_USERLIST));
    toolbar->addWidget(Schema);

    connect(Schema, SIGNAL(activated(const QString &)), this, SLOT(changeSchema(const QString &)));

    toolbar->addSeparator();

    QLabel * labRef = new QLabel(tr("Refresh") + " ", toolbar);
    toolbar->addWidget(labRef);
    connect(Refresh = toRefreshCreate(toolbar, TO_TOOLBAR_WIDGET_NAME),
            SIGNAL(activated(const QString &)), this, SLOT(changeRefresh(const QString &)));
    toolbar->addWidget(Refresh);

    toolbar->addSeparator();

    QLabel * labType = new QLabel(tr("Type") + " ", toolbar);
    toolbar->addWidget(labType);

    Type = new QComboBox(toolbar);
    Type->addItem(tr("SGA"));
    Type->addItem(tr("Long operations"));
    toolbar->addWidget(Type);

    toolbar->addSeparator();

    QLabel * labSelect = new QLabel(tr("Selection") + " ", toolbar);
    toolbar->addWidget(labSelect);

    Limit = new QComboBox(toolbar);
    Limit->addItem(tr("All"));
    Limit->addItem(tr("Unfinished"));
    Limit->addItem(tr("1 execution, 1 parse"));
    Limit->addItem(tr("Top executions"));
    Limit->addItem(tr("Top sorts"));
    Limit->addItem(tr("Top diskreads"));
    Limit->addItem(tr("Top buffergets"));
    Limit->addItem(tr("Top rows"));
    Limit->addItem(tr("Top sorts/exec"));
    Limit->addItem(tr("Top diskreads/exec"));
    Limit->addItem(tr("Top buffergets/exec"));
    Limit->addItem(tr("Top rows/exec"));
    Limit->addItem(tr("Top buffers/row"));
    toolbar->addWidget(Limit);

    toolbar->addWidget(new toSpacer());

    new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(splitter);

    Trace = new toResultTableView(false, false, splitter);

    QList<int> list;
    list.append(75);
    splitter->setSizes(list);

    Trace->setReadAll(true);
    Statement = new toSGAStatement(splitter);

    connect(Trace, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));
    CurrentSchema = connection.user().toUpper();
    updateSchemas();

    try
    {
        connect(timer(), SIGNAL(timeout(void)), this, SLOT(refresh(void)));
        toRefreshParse(timer(), toConfigurationSingle::Instance().refresh());
    }
    TOCATCH;

    setFocusProxy(Trace);
}

void toSGATrace::changeRefresh(const QString &str)
{
    try
    {
        toRefreshParse(timer(), str);
    }
    TOCATCH;
}

#define LARGE_BUFFER 4096

void toSGATrace::changeSchema(const QString &str)
{
    if (str != tr("Any"))
        CurrentSchema = str;
    else
        CurrentSchema = QString::null;
    if (toConfigurationSingle::Instance().autoUpdate())
        refresh();
}

static toSQL SQLSGATrace("toSGATrace:SGATrace",
                         "SELECT a.SQL_Text \"SQL Text\",\n"
                         "       a.First_Load_Time \"First Load Time\",\n"
                         "       b.username \"Parsing Schema\",\n"
                         "       a.Parse_Calls \"Parse Calls\",\n"
                         "       a.Executions \"Executions\",\n"
                         "       a.Sorts \"Sorts\",\n"
                         "       a.Disk_Reads \"Disk Reads\",\n"
                         "       a.Buffer_Gets \"Buffer Gets\",\n"
                         "       a.Rows_Processed \"Rows\",\n"
                         "       DECODE(a.Executions,0,'N/A',ROUND(a.Sorts/a.Executions,3)) \"Sorts/Exec\",\n"
                         "       DECODE(a.Executions,0,'N/A',ROUND(a.Disk_Reads/a.Executions,3)) \"Disk/Exec\",\n"
                         "       DECODE(a.Executions,0,'N/A',ROUND(a.Buffer_Gets/a.Executions,3)) \"Buffers/Exec\",\n"
                         "       DECODE(a.Executions,0,'N/A',ROUND(a.Rows_Processed/a.Executions,3)) \"Rows/Exec\",\n"
                         "       DECODE(a.Rows_Processed,0,'N/A',ROUND(a.Sorts/a.Rows_Processed,3)) \"Sorts/Rows\",\n"
                         "       DECODE(a.Rows_Processed,0,'N/A',ROUND(a.Disk_Reads/a.Rows_Processed,3)) \"Disk/Rows\",\n"
                         "       DECODE(a.Rows_Processed,0,'N/A',ROUND(a.Buffer_Gets/a.Rows_Processed,3)) \"Buffers/Rows\",\n"
                         "       a.Address||':'||a.Hash_Value \" \"\n"
                         "  from v$sqlarea a,\n"
                         "       sys.all_users b\n"
                         " where a.parsing_user_id = b.user_id",
                         "Display the contents of the SGA stack. Must have one hidden column "
                         "with SGA address at the end and a table name 'b' with a column username and must accept \"and ...\" clauses at end.");

static toSQL SQLLongOps(TOSQL_LONGOPS,
                        "SELECT b.opname \"Type\",\n"
                        "       a.SQL_Text \"SQL Text\",\n"
                        "       b.start_time \"Start Time\",\n"
                        "       b.sofar||'/'||b.totalwork \"Completed/Total\",\n"
                        "       b.elapsed_seconds||'/'||(b.elapsed_seconds+b.time_remaining) \"Completed/Total Time\",\n"
                        "       b.username \"Parsing Schema\",\n"
                        "       a.Parse_Calls \"Parse Calls\",\n"
                        "       a.Executions \"Executions\",\n"
                        "       a.Sorts \"Sorts\",\n"
                        "       a.Disk_Reads \"Disk Reads\",\n"
                        "       a.Buffer_Gets \"Buffer Gets\",\n"
                        "       a.Rows_Processed \"Rows\",\n"
                        "       DECODE(a.Executions,0,'N/A',ROUND(a.Sorts/a.Executions,3)) \"Sorts/Exec\",\n"
                        "       DECODE(a.Executions,0,'N/A',ROUND(a.Disk_Reads/a.Executions,3)) \"Disk/Exec\",\n"
                        "       DECODE(a.Executions,0,'N/A',ROUND(a.Buffer_Gets/a.Executions,3)) \"Buffers/Exec\",\n"
                        "       DECODE(a.Executions,0,'N/A',ROUND(a.Rows_Processed/a.Executions,3)) \"Rows/Exec\",\n"
                        "       DECODE(a.Rows_Processed,0,'N/A',ROUND(a.Sorts/a.Rows_Processed,3)) \"Sorts/Rows\",\n"
                        "       DECODE(a.Rows_Processed,0,'N/A',ROUND(a.Disk_Reads/a.Rows_Processed,3)) \"Disk/Rows\",\n"
                        "       DECODE(a.Rows_Processed,0,'N/A',ROUND(a.Buffer_Gets/a.Rows_Processed,3)) \"Buffers/Rows\",\n"
                        "       b.SQL_Address||':'||b.SQL_Hash_Value \" \"\n"
                        "  from v$sqlarea a,\n"
                        "       v$session_longops b\n"
                        " where b.sql_address = a.address(+)\n"
                        "   and b.sql_hash_value = a.hash_value(+)\n"
                        "   and b.opname is not null",
                        "Display the contents of long the long operations list. Must have a hidden column "
                        "with SGA address at the end and a table name 'b' with a column username and sid "
                        "and must accept \"and ...\" clauses at end.");

void toSGATrace::refresh(void)
{
    try
    {
        updateSchemas();

        QString select;
        switch (Type->currentIndex())
        {
        case 0:
            select = toSQL::string(SQLSGATrace, connection());
            break;
        case 1:
            select = toSQL::string(SQLLongOps, connection());
            break;
        default:
            toStatusMessage(tr("Unknown type of trace"));
            return ;
        }
        if (!CurrentSchema.isEmpty())
            select.append(QString::fromLatin1("\n   and b.username = :f1<char[101]>"));

        QString order;
        switch (Limit->currentIndex())
        {
        case 0:
            break;
        case 1:
            if (Type->currentIndex() == 1)
                select += QString::fromLatin1("\n   and b.sofar != b.totalwork");
            else
                toStatusMessage(tr("Unfinished is only available for long operations"));
            break;
        case 2:
            select += QString::fromLatin1("\n   and a.executions = 1 and a.parse = 1");
            break;
        case 3:
            order = QString::fromLatin1("a.Executions");
            break;
        case 4:
            order = QString::fromLatin1("a.Sorts");
            break;
        case 5:
            order = QString::fromLatin1("a.Disk_Reads");
            break;
        case 6:
            order = QString::fromLatin1("a.Buffer_Gets");
            break;
        case 7:
            order = QString::fromLatin1("a.Rows_Processed");
            break;
        case 8:
            order = QString::fromLatin1("DECODE(a.Executions,0,0,a.Sorts/a.Executions)");
            break;
        case 9:
            order = QString::fromLatin1("DECODE(a.Executions,0,0,a.Disk_Reads/a.Executions)");
            break;
        case 10:
            order = QString::fromLatin1("DECODE(a.Executions,0,0,a.Buffer_Gets/a.Executions)");
            break;
        case 11:
            order = QString::fromLatin1("DECODE(a.Executions,0,0,a.Rows_Processed/a.Executions)");
            break;
        case 12:
            order = QString::fromLatin1("DECODE(a.Rows_Processed,0,0,a.Buffer_Gets/a.Rows_Processed)");
            break;
        default:
            toStatusMessage(tr("Unknown selection"));
            break;
        }

        if (!order.isEmpty())
            select = QString("SELECT * FROM (\n") + select +
                     QString("\n ORDER BY " + order + " DESC)\n WHERE ROWNUM < 20");

        Trace->setSQL(QString::null);
        if (!CurrentSchema.isEmpty())
        {
            toQList p;
            p.insert(p.end(), CurrentSchema);
            Trace->query(select, p);
        }
        else
            Trace->query(select);

        Statement->refresh();
    }
    TOCATCH;
}

void toSGATrace::updateSchemas(void)
{
    try
    {
        Schema->refresh();
    }
    TOCATCH;
}

void toSGATrace::changeItem()
{
    QModelIndex s = Trace->selectedIndex(Trace->model()->columnCount() - 1);
    if (s.isValid())
        Statement->changeAddress(s.data(Qt::EditRole).toString());
}
