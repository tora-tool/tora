/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconnection.h"
#include "tonoblockquery.h"
#include "toresultdepend.h"
#include "tosql.h"


static toSQL SQLResultDepend("toResultDepend:Depends",
                             "SELECT DISTINCT\n"
                             "       referenced_owner \"Owner\",\n"
                             "       referenced_name \"Name\",\n"
                             "       referenced_type \"Type\",\n"
                             "       dependency_type \"Dependency Type\"\n"
                             "  FROM sys.all_dependencies\n"
                             " WHERE owner = :owner<char[101]>\n"
                             "   AND name = :name<char[101]>\n"
                             " ORDER BY referenced_owner,referenced_type,referenced_name",
                             "Display dependencies on an object, must have first two "
                             "columns same columns and same bindings",
                             "0800");
static toSQL SQLResultDepend7("toResultDepend:Depends",
                              "SELECT DISTINCT\n"
                              "       referenced_owner \"Owner\",\n"
                              "       referenced_name \"Name\",\n"
                              "       referenced_type \"Type\",\n"
                              "       'N/A' \"Dependency Type\"\n"
                              "  FROM sys.all_dependencies\n"
                              " WHERE owner = :owner<char[101]>\n"
                              "   AND name = :name<char[101]>\n"
                              " ORDER BY referenced_owner,referenced_type,referenced_name",
                              "",
                              "0703");

bool toResultDepend::canHandle(toConnection &conn)
{
    return toIsOracle(conn);
}

toResultDepend::toResultDepend(QWidget *parent, const char *name)
        : toResultView(false, false, parent, name)
{
    addColumn(tr("Owner"));
    addColumn(tr("Name"));
    addColumn(tr("Type"));
    addColumn(tr("Dependency"));

    setRootIsDecorated(true);
    setReadAll(true);
    setSQLName(QString::fromLatin1("toResultDepend"));

    Query = NULL;
    Current = NULL;
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
}

toResultDepend::~toResultDepend()
{
    delete Query;
}

bool toResultDepend::exists(const QString &owner, const QString &name)
{
    toTreeWidgetItem *item = firstChild();
    while (item)
    {
        if (item->text(0) == owner && item->text(1) == name)
            return true;
        if (item->firstChild())
            item = item->firstChild();
        else if (item->nextSibling())
            item = item->nextSibling();
        else
        {
            do
            {
                item = item->parent();
            }
            while (item && !item->nextSibling());
            if (item)
                item = item->nextSibling();
        }
    }
    return false;
}

void toResultDepend::query(const QString &sql, const toQList &param)
{
    if (!handled())
        return ;

    delete Query;
    Query = NULL;
    Current = NULL;

    if (!setSQLParams(sql, param))
        return ;

    clear();

    try
    {
        Query = new toNoBlockQuery(connection(),
                                   toQuery::Background,
                                   toSQL::string(SQLResultDepend, connection()),
                                   param);
        Poll.start(100);
    }
    TOCATCH
}

void toResultDepend::poll(void)
{
    try
    {
        if (!toCheckModal(this))
            return ;
        if (Query && Query->poll())
        {
            int cols = Query->describe().size();
            while (Query->poll() && !Query->eof())
            {
                toTreeWidgetItem *item;
                QString owner = Query->readValue();
                QString name = Query->readValue();
                if (!exists(owner, name))
                {
                    if (!Current)
                        item = new toResultViewItem(this, NULL, owner);
                    else
                        item = new toResultViewItem(Current, NULL, owner);
                    item->setText(1, name);
                    for (int i = 2;i < cols;i++)
                        item->setText(i, Query->readValue());
                }
                else
                {
                    for (int i = 2;i < cols;i++)
                        Query->readValue();
                }
            }
            if (Query->eof())
            {
                if (!Current)
                    Current = firstChild();
                else if (Current->firstChild())
                    Current = Current->firstChild();
                else if (Current->nextSibling())
                    Current = Current->nextSibling();
                else
                {
                    do
                    {
                        Current = Current->parent();
                    }
                    while (Current && !Current->nextSibling());
                    if (Current)
                        Current = Current->nextSibling();
                }
                delete Query;
                Query = NULL;
                if (Current)
                {
                    toQList param;
                    param.insert(param.end(), Current->text(0));
                    param.insert(param.end(), Current->text(1));
                    Query = new toNoBlockQuery(connection(),
                                               toQuery::Background,
                                               toSQL::string(SQLResultDepend, connection()),
                                               param);
                }
                else
                    Poll.stop();
            }
        }
        resizeColumnsToContents();
    }
    catch (const QString &exc)
    {
        delete Query;
        Query = NULL;
        Poll.stop();
        toStatusMessage(exc);
    }
}
