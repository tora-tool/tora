
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

#include "tools/tosgatrace.h"
#include "ui_tosgatracesettingui.h"
#include "tools/tosgastatement.h"
#include "core/tochangeconnection.h"
#include "widgets/toresultschema.h"
#include "widgets/torefreshcombo.h"
#include "core/utils.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"
#include "core/tosettingtab.h"
#include "core/totool.h"

#include <QtCore/QTimer>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QToolBar>
#include <QFontMetrics>
#include <QLineEdit>

#include "icons/refresh.xpm"
#include "icons/tosgatrace.xpm"

#include "core/toeditorconfiguration.h"
#include "toresulttableview.h"

class toSGATraceSetting
    : public QWidget
    , public Ui::toSGATraceSettingUI
    , public toSettingTab
{
        toTool *Tool;

    public:
        toSGATraceSetting(toTool *tool, QWidget* parent = 0, const char* name = 0)
            : QWidget(parent)
            , toSettingTab("trace.html"), Tool(tool)
        {
            setupUi(this);
            if (name)
                setObjectName(name);

            toSettingTab::loadSettings(this);
        }
        virtual void saveSetting(void)
        {
            toSettingTab::saveSettings(this);
        }
};

class toSGATraceTool : public toTool
{
    protected:
        const char **pictureXPM(void) override
        {
            return const_cast<const char**>(tosgatrace_xpm);
        }
    public:
        toSGATraceTool()
            : toTool(230, "SGA Trace") { }

        const char *menuItem() override
        {
            return "SGA Trace";
        }

        toToolWidget* toolWindow(QWidget *parent, toConnection &connection) override
        {
            return new toSGATrace(parent, connection);
        }

        QWidget *configurationTab(QWidget *parent) override
        {
            return new toSGATraceSetting(this, parent);
        }

        bool canHandle(const toConnection &conn) override
        {
            return conn.providerIs("Oracle");
        }

        void closeWindow(toConnection &connection) override {};
    private:
        static ToConfiguration::SgaTrace s_sgaTraceConf;
};

ToConfiguration::SgaTrace toSGATraceTool::s_sgaTraceConf;

static toSGATraceTool SGATraceTool;

toSGATrace::toSGATrace(QWidget *main, toConnection &connection)
    : toToolWidget(SGATraceTool, "trace.html", main, connection, "toSGATrace")
{
    QToolBar *toolbar = Utils::toAllocBar(this, tr("SGA trace"));
    layout()->addWidget(toolbar);

    FetchAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                           tr("Fetch statements in SGA"), this);
    FetchAct->setShortcut(QKeySequence::Refresh);
    connect(FetchAct, SIGNAL(triggered()), this, SLOT(refresh(void)));
    toolbar->addAction(FetchAct);

    toolbar->addSeparator();

    QLabel * labSchema = new QLabel(tr("Schema") + " ", toolbar);
    toolbar->addWidget(labSchema);

    Schema = new toResultSchema(toolbar);
    Schema->additionalItem(tr("Any"));
    Schema->setSelected(connection.user().toUpper());
    Schema->refresh();
    toolbar->addWidget(Schema);

    connect(Schema, SIGNAL(activated(const QString &)), this, SLOT(changeSchema(const QString &)));

    toolbar->addSeparator();

    QLabel * labRef = new QLabel(tr("Refresh") + " ", toolbar);
    toolbar->addWidget(labRef);
    Refresh = new toRefreshCombo(toolbar);
    connect(Refresh, SIGNAL(activated(const QString &)), this, SLOT(changeRefresh(const QString &)));
	connect(Refresh->timer(), SIGNAL(timeout(void)), this, SLOT(refresh(void)));
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

    toolbar->addSeparator();

    SQL_ID = new QLineEdit(toolbar);
    QFont fixed(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(ToConfiguration::Editor::ConfCodeFont).toString()));
    QFontMetrics mtr(fixed);
    SQL_ID->setFont(fixed);
    SQL_ID->setMaximumWidth(mtr.width("1234567890123") + 10);
    SQL_ID->setMaxLength(13); // sql_id VARCHAR2(13)
    SQL_ID->setPlaceholderText("sql_id");
    connect(SQL_ID, SIGNAL(returnPressed()), this, SLOT(refresh(void)));
    toolbar->addWidget(SQL_ID);

    toolbar->addWidget(new Utils::toSpacer());

    new toChangeConnection(toolbar);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(splitter);

    Trace = new toResultTableView(true, false, splitter);

    QList<int> list;
    list.append(75);
    splitter->setSizes(list);

    Trace->setReadAll(true);
    Statement = new toSGAStatement(splitter);

    connect(Trace, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));
    CurrentSchema = connection.user().toUpper();
    updateSchemas();

    setFocusProxy(Trace);
    refresh();
}

QString toSGATrace::schema() const
{
	return CurrentSchema;
}

void toSGATrace::changeSchema(const QString &str)
{
    if (str != tr("Any"))
        CurrentSchema = str;
    else
        CurrentSchema = QString();
    if (toConfigurationNewSingle::Instance().option(ToConfiguration::SgaTrace::AutoUpdate).toBool())
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
                         "       a.Address||':'||a.Hash_Value \" \", \n"
                         "       a.sql_id \" SQL_ID\"               \n"
						 "  from v$sqlarea a,\n"
                         "       sys.all_users b\n"
                         " where a.parsing_user_id = b.user_id",
                         "Display the contents of the SGA stack. Must have one hidden column "
                         "with SGA address at the end and a table name 'b' with a column username and must accept \"and ...\" clauses at end.");

static toSQL SQLLongOps(TOSQL_LONGOPS,
                        "SELECT "
                        //"       b.opname \"Type\",                        \n"
                        "       b.message \"Message\",                    \n"
                        "       a.SQL_Text \"SQL Text\",                  \n"
                        "       b.start_time \"Start Time\",              \n"
                        "       case when totalwork = 0 then 0            \n"
                        "            else round(100 * sofar/totalwork, 2) \n"
                        "       end as \"%\",                             \n"
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
                        "       b.SQL_Address||':'||b.SQL_Hash_Value \" \",\n"
                        "       a.SQL_ID as \" SQL_ID\"                    \n"
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
                Utils::toStatusMessage(tr("Unknown type of trace"));
                return ;
        }

        toQueryParams params;
        if (!CurrentSchema.isEmpty())
        {
            select.append(QString::fromLatin1("\n   and b.username = :f1<char[101]>"));
            params << CurrentSchema;
        }

        if (!SQL_ID->text().trimmed().isEmpty())
        {
            select.append(QString::fromLatin1("\n   and a.sql_id = :f2<char[101]>"));
            params << SQL_ID->text().trimmed();
        }

        QString order;
        switch (Limit->currentIndex())
        {
            case 0:
                break;
            case 1:
                if (Type->currentIndex() == 1)
                    select += QString::fromLatin1("\n   and b.sofar != b.totalwork");
                else
                    Utils::toStatusMessage(tr("Unfinished is only available for long operations"));
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
                Utils::toStatusMessage(tr("Unknown selection"));
                break;
        }

        if (!order.isEmpty())
            select = QString("SELECT * FROM (\n") + select +
                     QString("\n ORDER BY " + order + " DESC)\n WHERE ROWNUM < 20");

        Trace->removeSQL();
        Trace->query(select, params);

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
	QString sql_id = Trace->model()->data(Trace->selectedIndex().row(), " SQL_ID").toString();
	Statement->changeAddress(toQueryParams() << sql_id << QString("0"));
}
