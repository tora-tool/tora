/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "toextract.h"
#include "tohighlightedtext.h"
#include "totemporary.h"
#include "tomain.h"
#include "toresultextract.h"
#include "toresultlong.h"
#include "toresultview.h"
#include "tosgastatement.h"
#include "tosql.h"
#include "totool.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qsplitter.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <QMdiArea>

#include <QPixmap>
#include <QList>

#include "icons/refresh.xpm"
#include "icons/totemporary.xpm"

static toSQL SQLListTemporaryObjects("toTemporary:ListTemporaryObjects",
                                     "SELECT s.sid || ',' || s.serial# \"Session\",\n"
                                     "       s.username \"User\",\n"
                                     "       u.TABLESPACE \"Tablespace\",\n"
                                     "       segtype \"Type\",\n"
                                     "       substr ( a.sql_text,1,50 ) \"SQL\",\n"
                                     "       round ( u.blocks * p.value / :siz<int>,2 )||:sizstr<char[50]> \"Size\",\n"
                                     "       s.sql_address || ':' || s.sql_hash_value \" \"\n"
                                     "  FROM v$sort_usage u,\n"
                                     "       v$session s,\n"
                                     "       v$sqlarea a,\n"
                                     "       v$parameter p\n"
                                     " WHERE s.saddr = u.session_addr\n"
                                     "   AND a.address ( + ) = s.sql_address\n"
                                     "   AND a.hash_value ( + ) = s.sql_hash_value\n"
                                     "   AND p.name = 'db_block_size'",
                                     "Get temporary usage.", "0800");

static toSQL SQLListTemporaryObjects9("toTemporary:ListTemporaryObjects",
                                      "SELECT s.sid || ',' || s.serial# \"Session\",\n"
                                      "       s.username \"User\",\n"
                                      "       u.TABLESPACE \"Tablespace\",\n"
                                      "       u.segtype \"Type\",\n"
                                      "       substr ( a.sql_text,1,50 ) \"SQL\",\n"
                                      "       round ( u.blocks * p.value / :siz<int>,2 )||:sizstr<char[50]> \"Size\",\n"
                                      "       s.sql_address || ':' || s.sql_hash_value \" \"\n"
                                      "  FROM v$tempseg_usage u,\n"
                                      "       v$session s,\n"
                                      "       v$sqlarea a,\n"
                                      "       v$parameter p\n"
                                      " WHERE s.saddr = u.session_addr\n"
                                      "   AND a.address ( + ) = s.sql_address\n"
                                      "   AND a.hash_value ( + ) = s.sql_hash_value\n"
                                      "   AND p.name = 'db_block_size'",
                                      QString::null,
                                      "0900");

class toTemporaryTool : public toTool
{
    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(totemporary_xpm);
    }
public:
    toTemporaryTool()
            : toTool(130, "Temporary Objects")
    { }
    virtual const char *menuItem()
    {
        return "Temporary Objects";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toTemporary(parent, connection);
    }
    virtual bool canHandle(toConnection &conn)
    {
        if (!toIsOracle(conn))
            return false;
        if (conn.version() < "0800")
            return false;
        return true;
    }
    virtual void closeWindow(toConnection &connection){};
};

static toTemporaryTool TemporaryTool;

toTemporary::toTemporary(QWidget *main, toConnection &connection)
        : toToolWidget(TemporaryTool, "temporary.html", main, connection, "toTemporary")
{
    QToolBar *toolbar = toAllocBar(this, tr("Temporary Objects"));
    layout()->addWidget(toolbar);

    refreshAct =
        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                           tr("Refresh list"),
                           this,
                           SLOT(refresh()));

    toolbar->addWidget(new toSpacer());

    new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout()->addWidget(splitter);

    Objects = new toResultLong(false, false, toQuery::Background, splitter);

    QList<int> list;
    list.append(75);
    splitter->setSizes(list);

    QString unit(toConfigurationSingle::Instance().sizeUnit());
    toQList args;
    toPush(args, toQValue(toSizeDecode(unit)));
    toPush(args, toQValue(unit));

    Objects->setSelectionMode(toTreeWidget::Single);
    Objects->query(SQLListTemporaryObjects, args);
    connect(Objects, SIGNAL(selectionChanged(toTreeWidgetItem *)),
            this, SLOT(changeItem(toTreeWidgetItem *)));

    Statement = new toSGAStatement(splitter);

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(windowActivated(QMdiSubWindow *)));

    setFocusProxy(Objects);
}


void toTemporary::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;
    if (widget->widget() == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Temporary"), this);
            ToolMenu->addAction(refreshAct);

            toMainWidget()->addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toTemporary::refresh(void)
{
    Objects->refresh();
}

void toTemporary::changeItem(toTreeWidgetItem *item)
{
    if (item)
        Statement->changeAddress(item->text(Objects->columns()));
}
