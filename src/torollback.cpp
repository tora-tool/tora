
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

#include "utils.h"

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "tohelp.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "toresultitem.h"
#include "toresulttableview.h"
#include "toresultview.h"
#include "torollback.h"
#include "tosgastatement.h"
#include "tosql.h"
#include "tostoragedefinition.h"
#include "totool.h"

#include <list>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlayout.h>
#include <qlineedit.h>
#include "totreewidget.h"
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwidget.h>
#include <QMdiArea>

#include <QGroupBox>
#include <QVBoxLayout>
#include <QPixmap>
#include <QMenu>

#include "icons/addrollback.xpm"
#include "icons/offline.xpm"
#include "icons/online.xpm"
#include "icons/refresh.xpm"
#include "icons/torollback.xpm"
#include "icons/trash.xpm"

// #define CONF_NO_EXEC    "NoExec"
// #define CONF_NEED_READ  "NeedRead"
// #define CONF_NEED_TWO   "NeedTwo"
// #define CONF_ALIGN_LEFT "AlignLeft"
// #define CONF_OLD_ENABLE "OldEnable"

class toRollbackPrefs : public QGroupBox, public toSettingTab
{
    QCheckBox *OldEnable;
    QCheckBox *NoExec;
    QCheckBox *NeedRead;
    QCheckBox *NeedTwo;
    QCheckBox *AlignLeft;

    toTool *Tool;

public:
    toRollbackPrefs(toTool *tool, QWidget* parent = 0, const char* name = 0);
    virtual void saveSetting(void);
};

toRollbackPrefs::toRollbackPrefs(toTool *tool, QWidget* parent, const char* name)
        : QGroupBox(parent), toSettingTab("rollback.html#options"), Tool(tool)
{
    if (name)
        setObjectName(name);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(6);
    vbox->setContentsMargins(11, 11, 11, 11);

    setLayout(vbox);

    setTitle(qApp->translate("toRollbackPrefs", "Rollback Tool" ));

    OldEnable = new QCheckBox(this);
    OldEnable->setText(qApp->translate("toRollbackPrefs",
                                       "&Enable snapshot too old detection." ));
    OldEnable->setToolTip(qApp->translate(
                              "toRollbackPrefs",
                              "Enable snapshot too old detection, will put load on large databases."));
    vbox->addWidget(OldEnable);

    AlignLeft = new QCheckBox(this);
    AlignLeft->setText(qApp->translate("toRollbackPrefs",
                                       "&Disregard start extent." ));
    AlignLeft->setEnabled(false);
    connect(OldEnable, SIGNAL(toggled(bool)), AlignLeft, SLOT(setEnabled(bool)));
    AlignLeft->setToolTip(qApp->translate(
                              "toRollbackPrefs",
                              "Always start from the left border when displaying extent usage."));
    vbox->addWidget(AlignLeft);

    NoExec = new QCheckBox(this);
    NoExec->setText(qApp->translate("toRollbackPrefs",
                                    "&Restart reexecuted statements" ));
    NoExec->setEnabled(false);
    connect(OldEnable, SIGNAL(toggled(bool)), NoExec, SLOT(setEnabled(bool)));
    NoExec->setToolTip(qApp->translate(
                           "toRollbackPrefs",
                           "Start statements again that have been reexecuted."));
    vbox->addWidget(NoExec);

    NeedRead = new QCheckBox(this);
    NeedRead->setText(qApp->translate("toRollbackPrefs",
                                      "&Must read buffers" ));
    NeedRead->setEnabled(false);
    connect(OldEnable, SIGNAL(toggled(bool)), NeedRead, SLOT(setEnabled(bool)));
    NeedRead->setToolTip(qApp->translate(
                             "toRollbackPrefs",
                             "Don't display statements that have not read buffers."));
    vbox->addWidget(NeedRead);

    NeedTwo = new QCheckBox(this);
    NeedTwo->setText(qApp->translate("toRollbackPrefs",
                                     "&Exclude first appearance" ));
    NeedTwo->setEnabled(false);
    connect(OldEnable, SIGNAL(toggled(bool)), NeedTwo, SLOT(setEnabled(bool)));
    NeedTwo->setToolTip(qApp->translate(
                            "toRollbackPrefs",
                            "A statement must be visible at least two consecutive polls to be displayed."));
    vbox->addWidget(NeedTwo);

    QSpacerItem *spacer = new QSpacerItem(
        20,
        20,
        QSizePolicy::Minimum,
        QSizePolicy::Expanding);
    vbox->addItem(spacer);

//     if (!tool->config(CONF_OLD_ENABLE, "").isEmpty())
//         OldEnable->setChecked(true);
//     if (!tool->config(CONF_NO_EXEC, "Yes").isEmpty())
//         NoExec->setChecked(true);
//     if (!tool->config(CONF_NEED_READ, "Yes").isEmpty())
//         NeedRead->setChecked(true);
//     if (!tool->config(CONF_NEED_TWO, "Yes").isEmpty())
//         NeedTwo->setChecked(true);
//     if (!tool->config(CONF_ALIGN_LEFT, "Yes").isEmpty())
//         AlignLeft->setChecked(true);
    OldEnable->setChecked(toConfigurationSingle::Instance().oldEnable());
    NoExec->setChecked(toConfigurationSingle::Instance().noExec());
    NeedRead->setChecked(toConfigurationSingle::Instance().needRead());
    NeedTwo->setChecked(toConfigurationSingle::Instance().needTwo());
    AlignLeft->setChecked(toConfigurationSingle::Instance().alignLeft());
}

void toRollbackPrefs::saveSetting(void)
{
    toConfigurationSingle::Instance().setNoExec(NoExec->isChecked());
    toConfigurationSingle::Instance().setNeedRead(NeedRead->isChecked());
    toConfigurationSingle::Instance().setNeedTwo(NeedTwo->isChecked());
    toConfigurationSingle::Instance().setAlignLeft(AlignLeft->isChecked());
    toConfigurationSingle::Instance().setOldEnable(OldEnable->isChecked());
}

class toRollbackTool : public toTool
{
protected:
    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(torollback_xpm);
    }
public:
    toRollbackTool()
            : toTool(220, "Rollback Segments")
    { }
    virtual const char *menuItem()
    {
        return "Rollback Segments";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toRollback(parent, connection);
    }
    virtual QWidget *configurationTab(QWidget *parent)
    {
        return new toRollbackPrefs(this, parent);
    }
    virtual void closeWindow(toConnection &connection){};
};

static toRollbackTool RollbackTool;

static toSQL SQLTablespace("toRollbackDialog:TablespaceName",
                           "select tablespace_name from sys.dba_tablespaces order by tablespace_name",
                           "Get a list of tablespace names, should only have one column and same binds");

toRollbackDialog::toRollbackDialog(toConnection &Connection, QWidget* parent, const char* name)
        : QDialog(parent)//, name, true)
{
    setupUi(this);

    toHelp::connectDialog(this);

    try
    {
        toQuery q(Connection, SQLTablespace);
        while (!q.eof())
            Tablespace->addItem(q.readValue());
    }
    catch (...)
        {}
    connect(Tablespace, SIGNAL(textChanged(const QString &)),
            this, SLOT(valueChanged(const QString &)));

    Storage = new toStorageDefinition(DialogTabs);
    Storage->forRollback(true);
    DialogTabs->addTab(Storage, tr("Storage"));

    OkButton->setEnabled(false);
}

std::list<QString> toRollbackDialog::sql(void)
{
    QString str(QString::fromLatin1("CREATE "));
    if (Public->isChecked())
        str.append(QString::fromLatin1("PUBLIC "));
    str.append(QString::fromLatin1("ROLLBACK SEGMENT \""));
    str.append(Name->text());
    str.append(QString::fromLatin1("\" TABLESPACE \""));
    str.append(Tablespace->currentText());
    str.append(QString::fromLatin1("\""));
    std::list<QString> lst = Storage->sql();
    for (std::list<QString>::iterator i = lst.begin();i != lst.end();i++)
    {
        str += QString::fromLatin1(" ");
        str += *i;
    }
    std::list<QString> ret;
    toPush(ret, str);
    return ret;
}

void toRollbackDialog::displaySQL(void)
{
    std::list<QString> lines = sql();
    QString res;
    for (std::list<QString>::iterator i = lines.begin();i != lines.end();i++)
    {
        res += *i;
        res += QString::fromLatin1(";\n");
    }
    if (res.length() > 0)
    {
        toMemoEditor memo(this, res, -1, -1, true, true);
        memo.exec();
    }
    else
        toStatusMessage(tr("No changes made"), false, false);
}

void toRollbackDialog::valueChanged(const QString &)
{
    if (Name->text().isEmpty() ||
            Tablespace->currentText().isEmpty())
        OkButton->setEnabled(false);
    else
        OkButton->setEnabled(true);
}

#define MIN_HEIGHT 4
#define TRANSCOL 17

static bool BarsAlignLeft = true;

static void PaintBars(toTreeWidgetItem *item, QPainter *p, const QColorGroup & cg,
                      int width, std::list<double> &val, std::list<double> &maxExt, std::list<double> &curExt)
{
#if 0                           // disabled, wrong override
    if ( val.empty() )
    {
        p->fillRect(0, 0, width, item->height(),
                    QBrush(item->isSelected() ? cg.highlight() : cg.base()));
    }
    else
    {
        int num = 0;
        int lastHeight = 0;

        std::list<double>::iterator j = curExt.begin();
        std::list<double>::iterator k = maxExt.begin();
        for (std::list<double>::iterator i = val.begin();
                i != val.end() && j != curExt.end() && k != maxExt.end();
                i++, j++, k++)
        {
            num++;

            QBrush bg(item->isSelected() ? cg.highlight() : cg.base());
            QBrush fg(num % 2 ? Qt::blue : Qt::red);

            double start = (*i);
            double end = (*j);

            if (BarsAlignLeft)
            {
                end -= start;
                start = 0;
                if (end < 0)
                    end += (*k);
            }

            int height = item->height() * num / val.size();
            int pos = int(start * width / (*k));
            int posEnd = int(end * width / (*k));

            if (start > end)
            {
                p->fillRect(0, lastHeight, posEnd, height, fg);
                p->fillRect(posEnd, lastHeight, pos, height, bg);
                p->fillRect(pos, lastHeight, width, height, fg);
            }
            else
            {
                p->fillRect(0, lastHeight, pos, height, bg);
                p->fillRect(pos, lastHeight, posEnd, height, fg);
                p->fillRect(posEnd, lastHeight, width, height, bg);
            }
            lastHeight = height;
        }
    }
#endif
}

static toSQL SQLRollback("toRollback:Information",
                         "SELECT a.segment_name \"Segment\",\n"
                         "       a.owner \"Owner\",\n"
                         "       a.tablespace_name \"Tablespace\",\n"
                         "       a.status \"Status\",\n"
                         "       b.xacts \"Transactions\",\n"
                         "       ROUND ( a.initial_extent / u.unit,\n"
                         "        3 ) \"Initial (UNIT)\",\n"
                         "       ROUND ( a.next_extent / u.unit,\n"
                         "        3 ) \"Next (UNIT)\",\n"
                         "       a.pct_increase \"PCT Increase\",\n"
                         "       ROUND ( b.rssize / u.unit,\n"
                         "        3 ) \"Current (UNIT)\",\n"
                         "       ROUND ( b.optsize / u.unit,\n"
                         "        3 ) \"Optimal (UNIT)\",\n"
                         "       ROUND ( b.aveactive / u.unit,\n"
                         "        3 ) \"Used (UNIT)\",\n"
                         "       b.EXTENTS \"Extents\",\n"
                         "       b.CurExt \"Curr. Extent\",\n"
                         "       b.CurBlk \"Curr. Block\",\n"
                         "       b.gets \"Reads\",\n"
                         "       ROUND ( ( b.gets - b.waits ) * 100 / b.gets,\n"
                         "        2 ) || '%' \"Hitrate\",\n"
                         "       a.segment_id \"USN\"\n"
                         "  FROM sys.dba_rollback_segs a,\n"
                         "       v$rollstat b,\n"
                         "       (select :unit<char[10]> as unit from dual) u\n"
                         " WHERE a.segment_id = b.usn ( + )\n"
                         " ORDER BY a.segment_name",
                         "Get information about rollback segments.");

static toSQL SQLStartExt("toRollback:StartExtent",
                         "select to_char(b.start_uext)\n"
                         "  from v$transaction b\n"
                         " where b.xidusn = :f1<char[40]>",
                         "Get information about current extent in rollback of transactions");

static toSQL SQLStatementInfo("toRollback:StatementInfo",
                              "SELECT TO_CHAR(SYSDATE),\n"
                              "       a.User_Name,\n"
                              "       a.SQL_Text,\n"
                              "       a.Address||':'||a.Hash_Value,\n"
                              "       TO_CHAR(SUM(b.Executions)),\n"
                              "       TO_CHAR(SUM(b.Buffer_Gets))\n"
                              "  FROM v$open_cursor a,v$sql b\n"
                              " WHERE a.Address = b.Address AND a.Hash_Value = b.Hash_Value\n"
                              " GROUP BY TO_CHAR(SYSDATE),a.user_name,a.sql_text,a.address||':'||a.hash_value",
                              "Get information about statements in SGA. All columns must "
                              "be in exactly the same order.");
static toSQL SQLCurrentExtent("toRollback:CurrentExtent",
                              "select b.Extents,\n"
                              "       b.CurExt+b.CurBlk/c.Blocks\n"
                              "  from sys.dba_rollback_segs a,v$rollstat b,sys.dba_extents c\n"
                              " where a.segment_id = b.usn\n"
                              "   and a.owner = c.owner\n"
                              "   and a.segment_name = c.segment_name\n"
                              "   and c.segment_type = 'ROLLBACK'\n"
                              "   and b.curext = c.extent_id\n"
                              " order by a.segment_name",
                              "Get current extent (And fraction of) of rollback segments, "
                              "columns must be in exactly the same order");

class toRollbackOpen : public toResultView
{
    struct statementData
    {
        std::list<double> OpenExt;
        QString Opened;
        int Executed;
        int BufferGets;
        int Shown;
    };

    typedef std::map<QString, statementData> statements;
    statements Statements;
    int NumExtents;
    std::list<double> CurExt;
    std::list<double> MaxExt;
public:
class openItem : public toResultViewItem
    {
        toRollbackOpen *parent(void)
        {
            return (toRollbackOpen *)listView();
        }
    public:
        openItem(toTreeWidget *parent, toTreeWidgetItem *after, const QString &buf = QString::null)
                : toResultViewItem(parent, after, buf)
        { }
        virtual void paintCell (QPainter *pnt, const QColorGroup & cg,
                                int column, int width, int alignment)
        {
            if (column == 2)
            {
                QString address = text(4);
                std::list<double> &StartExt = parent()->Statements[address].OpenExt;
                PaintBars(this, pnt, cg, width, StartExt, parent()->MaxExt, parent()->CurExt);
            }
            else
                toResultViewItem::paintCell(pnt, cg, column, width, alignment);
        }

        virtual int width(const QFontMetrics &fm, const toTreeWidget *top, int column) const
        {
            if (column == 2)
                return 100;
            else
                return toResultViewItem::width(fm, top, column);
        }

        virtual void setup(void)
        {
            QString address = text(3);
            statementData &data = parent()->Statements[address];
            toResultViewItem::setup();
            setHeight(std::max(toResultViewItem::height(), int(MIN_HEIGHT*data.OpenExt.size())));
        }
    };

    friend class openItem;

    virtual toTreeWidgetItem *createItem(toTreeWidgetItem *last, const QString &str)
    {
        return new openItem(this, last, str);
    }

    toRollbackOpen(QWidget *parent)
            : toResultView(false, false, parent)
    {
        addColumn(qApp->translate("toRollbackOpen", "Started"));
        addColumn(qApp->translate("toRollbackOpen", "User"));
        addColumn(qApp->translate("toRollbackOpen", "Snapshot"));
        addColumn(qApp->translate("toRollbackOpen", "SQL"));
        setSorting(0);
        NumExtents = 0;
        setSQLName(QString::fromLatin1("toRollbackOpen"));
    }
    virtual void query(const QString &, const toQList &)
    {
        try
        {
            clear();
            toConnection &conn = connection();
            toBusy busy;
            toQuery sql(conn, SQLStatementInfo);
            toTreeWidgetItem *last = NULL;
            while (!sql.eof())
            {
                toTreeWidgetItem *item = createItem(last, QString::null);
                last = item;
                item->setText(0, sql.readValue());
                item->setText(1, sql.readValue());
                item->setText(3, sql.readValue());
                item->setText(4, sql.readValue());
                item->setText(5, sql.readValue());
                item->setText(6, sql.readValue());
            }

            toQuery rlb(conn, SQLCurrentExtent);

            CurExt.clear();
            MaxExt.clear();

            int num = 0;
            while (!rlb.eof())
            {
                MaxExt.insert(MaxExt.end(), rlb.readValue().toDouble());
                CurExt.insert(CurExt.end(), rlb.readValue().toDouble());
                num++;
            }
            statementData data;
            data.Shown = 0;
            data.OpenExt = CurExt;
            if (num != NumExtents)
            {
                Statements.clear();
                NumExtents = num;
            }

            bool noExec = toConfigurationSingle::Instance().noExec();
            bool needRead = toConfigurationSingle::Instance().needRead();
            bool needTwo = toConfigurationSingle::Instance().needTwo();
//             if (RollbackTool.config(CONF_NO_EXEC, "Yes").isEmpty())
//                 noExec = false;
//             if (RollbackTool.config(CONF_NEED_READ, "Yes").isEmpty())
//                 needRead = false;
//             if (RollbackTool.config(CONF_NEED_TWO, "Yes").isEmpty())
//                 needTwo = false;

            std::map<QString, int> Exists;
            for (toTreeWidgetItem *i = firstChild();i;)
            {
                QString address = i->text(4);
                Exists[address] = 1;
                statements::iterator j = Statements.find(address);
                int exec = i->text(5).toInt();
                int bufget = i->text(6).toInt();
                if (j == Statements.end() ||
                        ((*j).second.Executed != exec && noExec))
                {
                    data.Opened = i->text(0);
                    data.Executed = exec;
                    data.BufferGets = bufget;
                    Statements[address] = data;
                    toTreeWidgetItem *next = i->nextSibling();
                    if (needTwo)
                        delete i;
                    i = next;
                }
                else if ((*j).second.BufferGets == bufget && needRead)
                {
                    // Don't reset, just don't show
                    toTreeWidgetItem *next = i->nextSibling();
                    delete i;
                    i = next;
                }
                else
                {
                    i->setText(0, (*j).second.Opened);
                    i = i->nextSibling();
                    (*j).second.BufferGets = bufget;
                }
            }
            sort();

            // Erase unused

            for (statements::iterator j = Statements.begin();j != Statements.end();j++)
            {
                if (Exists.find((*j).first) == Exists.end())
                {
                    Statements.erase((*j).first);
                    j = Statements.begin();
                }
            }
        }
        TOCATCH
    }
};

toSQL SQLTransactionUsers("toRollback:TransactionUsers",
                          "select r.name \"Rollback Seg\",\n"
                          "       s.sid \"Session ID\",\n"
                          "       s.osuser \"Os User\",\n"
                          "       s.username \"Oracle User\"\n"
                          "from v$rollname r, v$session s, v$lock l\n"
                          "where l.type = 'TX'\n"
                          "and s.sid = l.sid\n"
                          "and r.usn = trunc(l.id1/65536)",
                          "Get users currently having open transactions");

toRollback::toRollback(QWidget *main, toConnection &connection)
        : toToolWidget(RollbackTool, "rollback.html", main, connection, "toRollback")
{
    QToolBar *toolbar = toAllocBar(this, tr("Rollback analyzer"));
    layout()->addWidget(toolbar);

    UpdateSegmentsAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                                    tr("Update segment list"), this);
    UpdateSegmentsAct->setShortcut(QKeySequence::Refresh);
    connect(UpdateSegmentsAct, SIGNAL(triggered()), this, SLOT(refresh(void)));
    toolbar->addAction(UpdateSegmentsAct);

    toolbar->addSeparator();

    enableOldAct = new QAction(QPixmap(const_cast<const char**>(torollback_xpm)),
                               tr("Enable snapshot too old detection."), this);
    enableOldAct->setCheckable(true);
    connect(enableOldAct, SIGNAL(toggled(bool)), this, SLOT(enableOld(bool)));
    toolbar->addAction(enableOldAct);

    toolbar->addSeparator();

    OnlineAct = new QAction(QPixmap(const_cast<const char**>(online_xpm)),
                            tr("Take segment online"), this);
    connect(OnlineAct, SIGNAL(triggered()), this, SLOT(online(void)));
    toolbar->addAction(OnlineAct);
    OnlineAct->setEnabled(false);

    OfflineAct = new QAction(QPixmap(const_cast<const char**>(offline_xpm)),
                             tr("Take segment offline"), this);
    connect(OfflineAct, SIGNAL(triggered()), this, SLOT(offline(void)));
    toolbar->addAction(OfflineAct);
    OfflineAct->setEnabled(false);

    toolbar->addSeparator();

    NewAct = new QAction(QPixmap(const_cast<const char**>(addrollback_xpm)),
                         tr("Create new rollback segment"), this);
    connect(NewAct, SIGNAL(triggered()), this, SLOT(addSegment(void)));
    toolbar->addAction(NewAct);

    DropAct = new QAction(QPixmap(const_cast<const char**>(trash_xpm)),
                          tr("Drop segment"), this);
    connect(DropAct, SIGNAL(triggered()), this, SLOT(dropSegment(void)));
    toolbar->addAction(DropAct);
    DropAct->setEnabled(false);

    toolbar->addSeparator();

    QLabel * lab1 = new QLabel(tr("Refresh") + " ", toolbar);
    toolbar->addWidget(lab1);
    connect(Refresh = toRefreshCreate(toolbar, TO_TOOLBAR_WIDGET_NAME),
            SIGNAL(activated(const QString &)), this, SLOT(changeRefresh(const QString &)));
    toolbar->addWidget(Refresh);

    toolbar->addWidget(new toSpacer());

    new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(splitter);

    Segments = new toResultTableView(true, false, splitter, "Segments");
    Segments->setSQL(SQLRollback);
    Segments->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(Segments, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));
    connect(Segments, SIGNAL(firstResult(const QString &, const toConnection::exception &, bool)),
            this, SLOT(updateHeaders(const QString &, const toConnection::exception &, bool)));

    QTabWidget *tab = new QTabWidget(splitter);
    TransactionUsers = new toResultTableView(false, false, tab);
    tab->addTab(TransactionUsers, tr("Transaction Users"));
    TransactionUsers->setSQL(SQLTransactionUsers);

    QSplitter *horsplit = new QSplitter(Qt::Horizontal, splitter);
    tab->addTab(horsplit, tr("Open Cursors"));

    Statements = new toRollbackOpen(horsplit);
    Statements->setSelectionMode(toTreeWidget::Single);
    connect(Statements, SIGNAL(selectionChanged(toTreeWidgetItem *)),
            this, SLOT(changeStatement(toTreeWidgetItem *)));
    CurrentStatement = new toSGAStatement(horsplit);

    try
    {
        connect(timer(), SIGNAL(timeout(void)), this, SLOT(refresh(void)));
        toRefreshParse(timer(), toConfigurationSingle::Instance().refresh());
    }
    TOCATCH

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(windowActivated(QMdiSubWindow *)));

    if (toConfigurationSingle::Instance().oldEnable())
        enableOldAct->setChecked(true);
    else
        Statements->setEnabled(false);

    refresh();

    setFocusProxy(Segments);
}

void toRollback::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;
    if (widget->widget() == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Rollback"), this);

            ToolMenu->addAction(UpdateSegmentsAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(OnlineAct);
            ToolMenu->addAction(OfflineAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(NewAct);
            ToolMenu->addAction(DropAct);

            ToolMenu->addSeparator();

            toMainWidget()->addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toRollback::refresh(void)
{
    QString unit(toConfigurationSingle::Instance().sizeUnit());
    Segments->changeParams(QString::number(toSizeDecode(unit)));
    Segments->refresh();

    TransactionUsers->refresh();        
}

void toRollback::updateHeaders(const QString &sql, const toConnection::exception &res, bool error)
{
    QString d;
    QString unit("(" + toConfigurationSingle::Instance().sizeUnit() + ")");

    for (int i = 0; i < Segments->model()->columnCount(); ++i) {
        d = Segments->model()->headerData(i, Qt::Horizontal).toString();
        if (d.indexOf("(UNIT)", 0, Qt::CaseInsensitive) != -1) {
            d.replace("(UNIT)", unit, Qt::CaseInsensitive);
            Segments->model()->setHeaderData(i, Qt::Horizontal, d, Qt::DisplayRole);
        }
    }

}

void toRollback::changeStatement(toTreeWidgetItem *item)
{
    if (item)
        CurrentStatement->changeAddress(item->text(4));
}

void toRollback::changeItem()
{
    QString stat(Segments->selectedIndex(4).data().toString());
    OfflineAct->setEnabled(stat != "OFFLINE");
    OnlineAct->setEnabled(stat != "ONLINE");
    DropAct->setEnabled(true);
}

void toRollback::changeRefresh(const QString &str)
{
    try
    {
        toRefreshParse(timer(), str);
    }
    TOCATCH
}

QString toRollback::currentSegment(void)
{
    return Segments->selectedIndex(1).data().toString();
}

void toRollback::addSegment(void)
{
    try
    {
        toRollbackDialog newSegment(connection(), this);
        if (newSegment.exec())
        {
            std::list<QString> sql = newSegment.sql();
            for (std::list<QString>::iterator i = sql.begin();i != sql.end();i++)
                connection().execute(*i);
            refresh();
        }
    }
    TOCATCH
}

void toRollback::offline(void)
{
    try
    {
        QString str;
        str = QString::fromLatin1("ALTER ROLLBACK SEGMENT \"");
        str.append(currentSegment());
        str.append(QString::fromLatin1("\" OFFLINE"));
        connection().execute(str);
        refresh();
    }
    TOCATCH
}

void toRollback::dropSegment(void)
{
    try
    {
        if (TOMessageBox::warning(this,
                                  tr("Drop rollback segment"),
                                  tr("Are you sure you want to drop the segment %1.").arg(currentSegment()),
                                  tr("&Drop"),
                                  tr("Cancel")) == 0)
        {
            QString str;
            str = QString::fromLatin1("DROP ROLLBACK SEGMENT \"");
            str.append(currentSegment());
            str.append(QString::fromLatin1("\""));
            connection().execute(str);
            refresh();
        }
    }
    TOCATCH
}

void toRollback::online(void)
{
    try
    {
        QString str;
        str = QString::fromLatin1("ALTER ROLLBACK SEGMENT \"");
        str.append(currentSegment());
        str.append(QString::fromLatin1("\" ONLINE"));
        connection().execute(str);
        refresh();
    }
    TOCATCH
}

void toRollback::enableOld(bool ena)
{
    Statements->setEnabled(ena);
}

