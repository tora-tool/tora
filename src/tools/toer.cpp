
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

#include <QToolBar>

#include "tools/toer.h"
#include "core/tologger.h"
#include "widgets/toresultcombo.h"
#include "core/utils.h"
#include "core/totool.h"
#include "core/tochangeconnection.h"
#include "core/toeventquery.h"

#include "icons/execute.xpm"
#include "icons/awrtool.xpm"

#include "dotgraph.h"
#include "dotgraphview.h"

#include <QToolBar>
#include "toparamget.h"
#include "toresultview.h"

static toSQL SQLALLRefs (
    "toResultDrawing:ALLREFConstraints",
    " WITH REFS                                       \n"
    " AS                                              \n"
    " (                                               \n"
    "   SELECT DISTINCT                               \n"
    "     c.constraint_name                           \n" // c1
    "     , c.owner           as owner                \n" // c2
    "     , c.table_name      as table_name           \n" // c3
    "     , r.owner           as r_owner              \n" // c4
    "     , r.table_name      as r_table_name         \n" // c5
    "   FROM                                          \n"
    "      sys.all_constraints a                      \n"
    "   JOIN sys.all_cons_columns c ON (c.constraint_name = a.constraint_name AND c.owner = a.owner) \n"
    "   JOIN sys.all_cons_columns r ON (r.constraint_name = a.r_constraint_name AND r.owner = a.r_owner AND r.position = c.position) \n"
    "   WHERE                                         \n"
    "        a.owner =         :f1<char[101]>         \n"
    "       AND a.constraint_type = 'R'               \n"
    " )                                               \n"
    " SELECT REFS.*                                   \n"
    "    -- , SYS_CONNECT_BY_PATH(table_name, '/') Path, CONNECT_BY_ISCYCLE CYCLE, LEVEL \n"
    " FROM REFS                                       \n"
    " CONNECT BY NOCYCLE PRIOR r_owner = owner AND PRIOR r_table_name = table_name \n",
    "Get list of all references",
    "8000",
    "Oracle");

class toERSchemaTool : public toTool
{
    protected:
        std::map<toConnection *, QWidget *> Windows;

        virtual const char **pictureXPM(void);
    public:
        toERSchemaTool() : toTool(10004, "ER Schema")
        { }

        virtual const char *menuItem()
        {
            return "ER Schema";
        }

        virtual bool canHandle(const toConnection &conn)
        {
            return conn.providerIs("Oracle");
        }

        virtual toToolWidget* toolWindow(QWidget *parent, toConnection &connection)
        {
            std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
            if (i != Windows.end())
            {
                (*i).second->raise();
                (*i).second->setFocus();
                return NULL;
            }
            else
            {
                toToolWidget* window = new toERSchema(parent, connection);
                Windows[&connection] = window;
                return window;
            }
        }
        virtual void closeWindow(toConnection &connection)
        {
            std::map<toConnection *, QWidget *>::iterator i = Windows.find(&connection);
            if (i != Windows.end())
            {
                Windows.erase(i);
            }
        }
};

const char **toERSchemaTool::pictureXPM(void)
{
    return const_cast<const char**>(awrtool_xpm);
}

static toERSchemaTool ERSchemaTool;

toERSchema::toERSchema(/*toTool *tool,*/ QWidget *parent, toConnection &_connection)
    : toToolWidget(ERSchemaTool, "simplequery.html", parent, _connection, "toERSchema")
    , Query(NULL)
{
    QToolBar *toolbar = Utils::toAllocBar(this, tr("ER Schema"));
    layout()->addWidget(toolbar);

    toolbar->addWidget(new QLabel("ER Schema view - TODO add some icons here:", toolbar));
    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(execute_xpm))),
                       tr("Start reversing"),
                       this,
                       SLOT(slotExecute()));
    toolbar->addWidget(new Utils::toSpacer());

    m_DotGraphView = new DotGraphView(NULL, this);
    new toChangeConnection(toolbar);

//	QVBoxLayout *vbox = new QVBoxLayout;
//	vbox->setSpacing(0);
//	vbox->setContentsMargins(0, 0, 0, 0);
//	this->setLayout(vbox);

    layout()->addWidget(m_DotGraphView);
}

void toERSchema::slotExecute(void)
{
    try
    {
        toConnection &conn = connection();

        m_Tables.clear();
        m_References.clear();
        m_DotGraphView->initEmpty();

        // TODO use own toResultSchema combo box in this tool
        // TODO Disable all the actions till query done
        Query = new toEventQuery(this
                                 , conn
                                 , toSQL::sql(SQLALLRefs, conn)
                                 , toQueryParams() << conn.defaultSchema()
                                 , toEventQuery::READ_ALL);
        connect(Query, &toEventQuery::dataAvailable, this, &toERSchema::receiveData);
        connect(Query, SIGNAL(done(toEventQuery*,unsigned long)), this, SLOT(slotQueryDone()));
        Query->start();
    }
    catch (const toConnection::exception &t)
    {
        TOMessageBox::information(this, t, t);
    }
}

void toERSchema::receiveData(toEventQuery *e)
{
    toQValue c1, c2, c3, c4, c5;
    while (Query->hasMore())
    {
        c1 = Query->readValue(); // FK NAME
        c2 = Query->readValue(); // owner
        c3 = Query->readValue(); // table_name
        c4 = Query->readValue(); // r_owner
        c5 = Query->readValue(); // r_table_name

        m_Tables.insert((QString)c3);
        m_Tables.insert((QString)c5);
        m_References.insert(Reference((QString)c3, (QString)c5));
    }
}

void toERSchema::slotQueryDone(void)
{
    DotGraph m_Graph;
    m_Graph.setGraphAttributes(GraphAttributes);

    if (Query)
    {
        delete Query;
        Query = NULL;
    }

    Q_FOREACH(QString const&t, m_Tables)
    {
        QMap<QString,QString> ta; // table atributes
        ta["name"] = t;
        ta["label"] = t;
        ta["fontsize"] = "12";
        ta["comment"]= t;
        ta["id"]= t;
        ta["tooltip"] = t;
        m_Graph.addNewNode(ta);
    }
    m_Tables.clear();

    Q_FOREACH(Reference const&r, m_References)
    {
        QMap<QString,QString> ea; // edge attreibutes
        m_Graph.addNewEdge(r.first, r.second, ea);
    }
    m_DotGraphView->graph()->updateWithGraph(m_Graph);
    m_DotGraphView->graph()->update();
    m_DotGraphView->prepareSelectSinlgeElement();
}

void toERSchema::slotInstanceChanged(int pos)
{
};

toERSchema::~toERSchema()
{
}

void toERSchema::closeEvent(QCloseEvent *event)
{
    try
    {
        if (Query)
            Query->stop();
        ERSchemaTool.closeWindow(connection());
    }
    TOCATCH;

    event->accept();
}

QMap<QString, QString> toERSchema::GraphAttributesHelper()
{
    QMap<QString, QString>ga;
    ga["id"] = "Schema";
    ga["compound"] = "true";
    ga["shape"] = "box";
    ga["rankdir"] = "BT"; // BOTTOM to TOP arrows
    return ga;
}
const QMap<QString, QString> toERSchema::GraphAttributes = toERSchema::GraphAttributesHelper();
