
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

#include "tools/toresultplan.h"

#include "core/utils.h"
#include "core/toeventquery.h"
#include "core/tomainwindow.h"
#include "widgets/toresultcombo.h"
#include "widgets/toworkingwidget.h"
#include "core/toconfiguration.h"
#include <QtCore/QTimer>
#include <QStackedLayout>
#include <QStatusBar>
#include <QPaintEvent>
#include <QPlainTextEdit>

#include "core/toeditorconfiguration.h"
#include "connection/tooracleconfiguration.h"

static toSQL SQLVSQLChildSel("toResultPlan:VSQLChildSel",
                             "SELECT distinct to_char(child_number)||' ('||to_char(plan_hash_value)||')' cn_disp, child_number, sql_id, plan_hash_value\n"
                             "FROM V$SQL_PLAN WHERE sql_id = :sql_id<char[40],in> \n"
                             "ORDER BY child_number",
                             "Get list of child plans for cursor",
                             "1000");

// see bug 2941533
static toSQL SQLViewVSQLPlan("toResultPlan:ViewVSQLPlan",
                             "SELECT /* +NO_MERGE */ ID,NVL(Parent_ID,0) \n"
                             "  , Operation    \n"
                             "  , Options      \n"
                             "  , Object_Name  \n"
                             "  , Optimizer    \n"
                             "  , cost         \n"
                             "  , io_cost      \n"
                             "  , Bytes        \n"
                             "  , Cardinality  \n"
                             "  , partition_start, partition_stop \n"
                             "  , temp_space,time,access_predicates,filter_predicates \n"
                             "FROM (SELECT * FROM V$SQL_PLAN WHERE sql_id = :sqlid<char[40],in> and child_number = :chld<char[10],in> ) \n"
                             //"WHERE sql_id = :sqlid<char[40],in> and child_number = :chld<char[10],in> \n"
                             "START WITH id = 0 \n"
                             "CONNECT BY PRIOR id = parent_id and prior address=address and prior nvl(hash_value, 0) = nvl(hash_value, 0) \n",
                             "Get the contents of SQL plan from V$SQL_PLAN",
                             "1000");

static toSQL SQLViewPlan("toResultPlan:ViewPlan",
                         "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name \n"
                         " ,Optimizer,cost \n"
                         " ,io_cost,Bytes,Cardinality,partition_start,partition_stop \n"
                         " ,temp_space,time,access_predicates,filter_predicates \n"
                         "FROM (select * from %1 WHERE Statement_ID = '%2') \n"
                         "START WITH id = 0 \n"
                         "CONNECT BY PRIOR id = parent_id \n"
                         "ORDER by id \n",
                         "Get the contents of a plan table. Observe the %1 and %2 which must be present. Must return same columns",
                         "1000");

static toSQL SQLViewPlan8("toResultPlan:ViewPlan",
                          "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer,cost,\n"
                          "  io_cost,Bytes,Cardinality,partition_start,partition_stop,\n"
                          "  ' ',' ',' 'access_predicates,' 'filter_predicates\n"
                          "  FROM %1 WHERE Statement_ID = '%2' ORDER BY NVL(Parent_ID,0),ID",
                          "",
                          "0800");

////////////////////////////////////////////////////////////////////////////////
//                     toResultPlanView
////////////////////////////////////////////////////////////////////////////////
toResultPlanView::toResultPlanView(QWidget *parent)
	: QTreeView(parent)
	, Ready(true)
{
    setAlternatingRowColors(true);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(true);

    Working = new toWorkingWidget(this);
    connect(Working, SIGNAL(stop()), this, SLOT(slotStop()));
};

void toResultPlanView::queryStarted()
{
	Ready = false;
	// sets visible true but won't show if parent is hidden
	QTimer::singleShot(300, Working, SLOT(forceShow()));
}

void toResultPlanView::queryDone()
{
	Ready = true;
	Working->hide();
}

void toResultPlanView::showEvent(QShowEvent * event)
{
    QTreeView::showEvent(event);
    QMainWindow *main = toMainWindow::lookup();
    if(main)
    {
        toExplainTypeButtonSingle::Instance().setEnabled(true);
        //main->statusBar()->insertWidget(0, &toExplainTypeButtonSingle::Instance(), 0);
        main->statusBar()->addWidget(&toExplainTypeButtonSingle::Instance(), 0);
        toExplainTypeButtonSingle::Instance().show();
    }
}

void toResultPlanView::hideEvent(QHideEvent * event)
{
    QTreeView::hideEvent(event);
    QMainWindow *main = toMainWindow::lookup();
    if(main)
    {
        main->statusBar()->removeWidget(&toExplainTypeButtonSingle::Instance());
    }
}

void toResultPlanView::paintEvent(QPaintEvent *event)
{
    if (Ready)
    {
        Working->hide();
        super::paintEvent(event);
    }
    else
    {
        Working->setGeometry(this->viewport()->frameGeometry());
        Working->show();
        event->ignore();
    }

}

void toResultPlanView::resizeEvent(QResizeEvent *event)
{
	super::resizeEvent(event);

    if (!Ready)
    {
        Working->setGeometry(this->viewport()->frameGeometry());
        Working->repaint();
    }
}

////////////////////////////////////////////////////////////////////////////////
//                     toExplainTypeButton
////////////////////////////////////////////////////////////////////////////////
toExplainTypeButton::toExplainTypeButton(QWidget *parent, const char *name)
    : toToggleButton(ENUM_REF(toResultPlanView,ExplainTypeEnum), parent, name)
{
    enablePopUp();
}

toExplainTypeButton::toExplainTypeButton()
    : toToggleButton(ENUM_REF(toResultPlanView,ExplainTypeEnum), NULL)
{
    enablePopUp();
}



toPlanTreeItem::toPlanTreeItem(const QString& id, const QVariantList& data, toPlanTreeItem *parent)
	: m_id(id)
	, m_parentItem(parent)
	, m_itemData(data)
{}
;
toPlanTreeItem::~toPlanTreeItem()
{
    qDeleteAll(m_childItems);
}

void toPlanTreeItem::appendChild(toPlanTreeItem *item)
{
    m_childItems.append(item);
}

toPlanTreeItem *toPlanTreeItem::child(int row)
{
    return m_childItems.value(row);
}

int toPlanTreeItem::childCount() const
{
    return m_childItems.count();
}

int toPlanTreeItem::columnCount() const
{
    return m_itemData.count();
}

QVariant toPlanTreeItem::data(int column) const
{
    return m_itemData.value(column);
}

QVariantList& toPlanTreeItem::childData()
{
	return m_itemData;
}

toPlanTreeItem *toPlanTreeItem::parentItem()
{
    return m_parentItem;
}

int toPlanTreeItem::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<toPlanTreeItem*>(this));

    return 0;
}

QString toPlanTreeItem::id() const
{
	return m_id;
}

toResultPlanModel::toResultPlanModel(toEventQuery *query, QObject *parent)
	: QAbstractItemModel(parent)
	, Query(query)
	, HeadersRead(false)
{
	Query->setParent(this); // this will satisfy QObject's disposal

    Headers
    << HeaderDesc{"Operation", Qt::AlignHCenter}
	<< HeaderDesc{"Options", Qt::AlignHCenter }
	<< HeaderDesc{"Object name", Qt::AlignHCenter }
	<< HeaderDesc{"Mode", Qt::AlignHCenter }
	<< HeaderDesc{"Cost", Qt::AlignHCenter }
	<< HeaderDesc{"%CPU", Qt::AlignRight}
	<< HeaderDesc{"Bytes", Qt::AlignRight}
	<< HeaderDesc{"Rows",  Qt::AlignRight}
	<< HeaderDesc{"Time", Qt::AlignHCenter }
	<< HeaderDesc{"Access pred.", Qt::AlignHCenter }
	<< HeaderDesc{"Filter pred.", Qt::AlignHCenter }
	<< HeaderDesc{"TEMP Space", Qt::AlignRight }
	<< HeaderDesc{"Startpartition", Qt::AlignHCenter }
	<< HeaderDesc{"Endpartition", Qt::AlignHCenter }
	;

	rootItem = new toPlanTreeItem("root", QList<QVariant>());
	QVariantList sqlidData;
	for (int i = sqlidData.size(); i < Headers.size(); i++) sqlidData << "";
	rootItem->appendChild(sqlidItem = new toPlanTreeItem("sqlid", sqlidData, rootItem));

    connect(Query, &toEventQuery::dataAvailable, this, &toResultPlanModel::receiveData);
    connect(Query, &toEventQuery::done, this, &toResultPlanModel::slotQueryDone);
    connect(Query, SIGNAL(error(toEventQuery*,toConnection::exception const &)), this, SLOT(slotErrorHanler(toEventQuery*, toConnection::exception  const &)));
}

toResultPlanModel::~toResultPlanModel()
{
	cleanup();
	delete rootItem;
}

int toResultPlanModel::columnCount(const QModelIndex &parent) const
{
// it looks like there is a bug in QT, column count in upper row affects
// column column displayed in lower rows
//    if (parent.isValid())
//        return static_cast<toPlanTreeItem*>(parent.internalPointer())->columnCount();
//    else
	return Headers.size();
}

QVariant toResultPlanModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    toPlanTreeItem *item = static_cast<toPlanTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

bool toResultPlanModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	// only sqlidItem can be changed
	if (rootIndex() != index)
		return QAbstractItemModel::setData(index, value, role);

	layoutAboutToBeChanged();
	toPlanTreeItem *item = static_cast<toPlanTreeItem*>(index.internalPointer());
	item->childData()[index.column()] = value;
	return true;
	layoutChanged();
}

Qt::ItemFlags toResultPlanModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant toResultPlanModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
	if (orientation != Qt::Horizontal)
		return QVariant();

	if (section > Headers.size() - 1)
		return QVariant();

	switch (role)
	{
	case Qt::DisplayRole:
		return Headers[section].name;
	case Qt::TextAlignmentRole:
		return (int)Headers[section].align;
	case Qt::SizeHintRole: // QSize(120, 0) col 10,11
	default:
		return QVariant();
	}

    return QVariant();
#if 0
            item->setTextAlignment (5, Qt::AlignRight);
            item->setText(6, cpupct);
            item->setTextAlignment (6, Qt::AlignRight);
            item->setText(7, bytes);
            item->setTextAlignment (7, Qt::AlignRight);
            item->setText(8, cardinality);
            item->setTextAlignment (8, Qt::AlignRight);
            item->setText(9, time);
            item->setText(10, accesspred);
            item->setToolTip(10, accesspred);
            item->setSizeHint(10, QSize(120, 0));
            item->setText(11, filterpred);
            item->setToolTip(11, filterpred);
            item->setSizeHint(11, QSize(120, 0));
            item->setText(12, tempspace);
            item->setTextAlignment (12, Qt::AlignRight);
#endif

}

QModelIndex toResultPlanModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    toPlanTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<toPlanTreeItem*>(parent.internalPointer());

    toPlanTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex toResultPlanModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    toPlanTreeItem *childItem = static_cast<toPlanTreeItem*>(index.internalPointer());
    toPlanTreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int toResultPlanModel::rowCount(const QModelIndex &parent) const
{
    toPlanTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<toPlanTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void toResultPlanModel::cleanup()
{
    if (Query)
    {
        disconnect(Query, 0, this, 0);

        Query->stop();
        delete Query;
        Query = NULL;
    }
    stack.clear();
}

QModelIndex toResultPlanModel::rootIndex() const
{
	return createIndex(0, 0, sqlidItem);
}

toResultPlanAbstr::toResultPlanAbstr(QWidget *parent)
	: QWidget(parent)
	, CursorChildSel(NULL)
	, Query(NULL)
	, DisplayChildCombo(false)
	, Explaining(false)
{
    using namespace ToConfiguration;
    planTreeView = new toResultPlanView(this);
    planTreeText = new QPlainTextEdit(this);
    planTreeText->setReadOnly(true);
    planTreeText->setFont(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfTextFont).toString()));

    //toExplainTypeButtonSingle::Instance().
    QStackedLayout  *mainLayout = new QStackedLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(planTreeView);
    mainLayout->addWidget(planTreeText);
    mainLayout->setCurrentIndex(0);
    //mainLayout->setStackingMode(QStackedLayout::StackAll);
    setLayout(mainLayout);
}

void toResultPlanAbstr::queryCursorPlan(toQueryParams const& params)
{
    // Prepare the query
    toConnection &c = toConnection::currentConnection(this);
    toEventQuery *Query = new toEventQuery(this
    		, c
			, toSQL::string(SQLViewVSQLPlan, c)
    		, params
			, toEventQuery::READ_ALL);

    // Allocate the model
	model = new toResultPlanModel(Query, this);
	connect(model, SIGNAL(queryDone(toEventQuery*)), this, SLOT(queryDone(toEventQuery*)));
	planTreeView->setModel(model);
	planTreeView->queryStarted();
	sql_id = params.at(0);
	child_id = params.at(1);
	DisplayChildCombo = true;
    // Start the query
    Query->start();
}

void toResultPlanAbstr::queryDone(toEventQuery*)
{
	// explain query finished resize view columns
	planTreeView->expandAll();
	for (int col = 0; col < model->columnCount(); col++)
		planTreeView->resizeColumnToContents(col);

	if (DisplayChildCombo)
	{
		// refresh child_number selection combo
		CursorChildSel = new toResultCombo(planTreeView, "toResultPlan");
		CursorChildSel->setSQL(SQLVSQLChildSel);
		CursorChildSel->setSelectedData(child_id);
		connect(CursorChildSel, SIGNAL(done()), this, SLOT(childComboReady())); //Wait for cursor children combo to fill
		try
		{
			CursorChildSel->refreshWithParams(toQueryParams() << sql_id);
		}
		TOCATCH;
	}
	if (Explaining)
	{
		if (toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::KeepPlansBool).toBool())
			(*LockedConnection)->commit();
		else
			(*LockedConnection)->rollback();
		LockedConnection.clear();
		Explaining = false;
	}

	planTreeView->queryDone();
}

void toResultPlanAbstr::childComboReady()
{
    QStringList cur_sel = CursorChildSel->itemData(CursorChildSel->currentIndex()).toStringList();
	if (cur_sel.empty())
		return;
    plan_hash = cur_sel.at(2);
    QString SInfo = QString::fromLatin1("V$SQL_PLAN: %1\nChild: %2 SQL_ID: %3")
    	.arg(plan_hash)
		.arg(child_id)
		.arg(sql_id);

	model->setData(model->rootIndex(), SInfo, Qt::DisplayRole);
	planTreeView->setIndexWidget(model->rootIndex().sibling(0,2), CursorChildSel);
	connect(CursorChildSel, SIGNAL(currentIndexChanged(int)), this, SLOT(childComboChanged(int)));
}

void toResultPlanAbstr::childComboChanged(int NewIndex)
{

}

void toResultPlanAbstr::queryPlanTable(toQueryParams const& params)
{
	toConnection &conn = toConnection::currentConnection(this);
#ifndef TO_NO_ORACLE
	if (Explaining)
	{
	    // avoid double run of explanation query
#pragma message WARN("TODO: find a way how to disable QAction button")
	    return;
	}

	if (conn.providerIs("Oracle"))
	{
		{
			QSharedPointer<toConnectionSubLoan> c(new toConnectionSubLoan(conn));
			this->LockedConnection = c;
		}

    	planId = QString::fromLatin1("TOra ") + QString::number(QDateTime::currentMSecsSinceEpoch()/1000 +  rand());

    	QString planTable = ToConfiguration::Oracle::planTable(conn.user());

    	QString explain = QString::fromLatin1("EXPLAIN PLAN SET STATEMENT_ID = '%1' INTO %2 FOR %3")
    	.arg(planId)
    	.arg(planTable)
    	.arg(params.first());

    	explainQuery = new toEventQuery(this, LockedConnection, explain, toQueryParams(), toEventQuery::READ_ALL);
        connect(explainQuery, SIGNAL(done(toEventQuery*, unsigned long)), this, SLOT(explainDone(toEventQuery*)));
        auto c2 = connect(explainQuery, &toEventQuery::error, this, &toResultPlanAbstr::queryError);
    	Explaining = true;
    	explainQuery->start();
    }
#endif
}

void toResultPlanAbstr::explainDone(toEventQuery*q)
{
    Q_ASSERT_X(explainQuery == q, qPrintable(__QHERE__), "Double explain detected");
    disconnect(explainQuery, 0, this, 0);
    explainQuery->stop();
    explainQuery = NULL;
#ifndef TO_NO_ORACLE
    toConnection &conn = toConnection::currentConnection(this);
    if (conn.providerIs("Oracle"))
    {
    	toEventQuery *Query = new toEventQuery(this
    	                                       , LockedConnection
    	                                       // Since EXPLAIN PLAN is always to conn.user() plan_table
    	                                       // and current_schema can be different
    	                                       , toSQL::string(SQLViewPlan, conn).arg(ToConfiguration::Oracle::planTable(conn.user())).arg(planId)
    	                                       , toQueryParams()
    	                                       , toEventQuery::READ_ALL);

    	model = new toResultPlanModel(Query, this);
    	connect(model, SIGNAL(queryDone(toEventQuery*)), this, SLOT(queryDone(toEventQuery*)));
    	planTreeView->setModel(model);

    	Query->start();
    }
#endif
}

void toResultPlanAbstr::queryError(toEventQuery*, toConnection::exception  const &str)
{
	try
    {
        if (str.contains(QString::fromLatin1("ORA-02404")))
        {
#ifndef TO_NO_ORACLE
            QString planTable = ToConfiguration::Oracle::planTable(connection().user());

            // if shared plan table does not exist, do not try to create it
            if (toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::SharedPlanBool).toBool())
            {
                TOMessageBox::warning(this,
                                      tr("Plan table doesn't exist"),
                                      tr("Specified plan table %1 doesn't exist.").arg(planTable),
                                      tr("&OK"));
            }
            else
            {
                int ret = TOMessageBox::warning(this,
                                                tr("Plan table doesn't exist"),
                                                tr("Specified plan table %1 doesn't exist.\n"
                                                   "Should TOra try to create it?").arg(planTable),
                                                tr("&Yes"), tr("&No"), QString(), 0, 1);
                if (ret == 0)
                {
                    Utils::toBusy busy;
                    toConnectionSubLoan conn(connection());
                    toQuery createPlanTable(conn, toSQL::string(toSQL::TOSQL_CREATEPLAN, connection()).arg(planTable), toQueryParams());
                    createPlanTable.eof();
                }
            }
#endif
        }
        else
            Utils::toStatusMessage(str);
    }
	TOCATCH
	Explaining = false;
}

void toResultPlanAbstr::queryXPlan(toQueryParams const& params)
{
	toConnectionSubLoan conn(toConnection::currentConnection(this));
	QString SQL("SELECT t.* FROM table(DBMS_XPLAN.DISPLAY_CURSOR(:sql_id<char[10]>, :child_number<int>)) t");

	planTreeText->clear();
	toQuery q(conn, SQL, params);
	while (!q.eof())
		planTreeText->appendPlainText((QString)q.readValue());
}

bool toResultPlanAbstr::canHandle(const toConnection &conn)
{
    return
        conn.providerIs("Oracle") ||
        conn.providerIs("QMYSQL") ||
        conn.providerIs("QPSQL");
}

void toResultPlanModel::receiveData(toEventQuery*Query)
{
    if (this->Query != Query)
        return;

    try
    {
        while (Query->hasMore())
        {
        	QList<QVariant> columnData, itemData;
        	toPlanTreeItem *itemNew;

        	for (unsigned i=0; i < Query->columnCount(); i++)
        		itemData << (QString)Query->readValue();

            QString id              = itemData[0].toString();
            QString parentid        = itemData[1].toString();
            QString operation       = itemData[2].toString();
            QString options         = itemData[3].toString();
            QString object          = itemData[4].toString();
            QString optimizer       = itemData[5].toString();
            QString cost            = itemData[6].toString();
            QString iocost          = itemData[7].toString();
            QString bytes           = itemData[8].toString();
            QString cardinality     = itemData[9].toString();
            QString startpartition  = itemData[10].toString();
            QString endpartition    = itemData[11].toString();
            QString tempspace       = itemData[12].toString();
            QString time            = itemData[13].toString();
            QString accesspred      = itemData[14].toString();
            QString filterpred      = itemData[15].toString();

            QString cpupct;
            if (!cost.isEmpty())
            {
                double pct = 100;
                if (cost.toDouble() > 0)
                {
                    pct = 100 - (iocost.toDouble() / cost.toDouble() * 100);
                }
                cpupct.setNum(pct, 'f', 2);
            }

            if (!time.isEmpty())
            {
                double seconds = time.toDouble();
                int hours = (int) (seconds / 3600);
                int mins = (int) (( seconds - hours * 3600) / 60);
                int secs = (int) seconds - (hours * 3600 + mins * 60);
                time.asprintf("%d:%02d:%02d", hours, mins, secs);
            }

            columnData
			<< operation
			<< options
			<< object
			<< optimizer
			<< cost
			<< cpupct
			<< bytes
			<< cardinality
			<< time
			<< accesspred
			<< filterpred
			<< tempspace
			<< startpartition
			<< endpartition
				;

            if (stack.empty()) // Root item
            {
            	sqlidItem->appendChild(itemNew = new toPlanTreeItem(id, columnData, sqlidItem));
            } else { // Child item, pop stack until child's parent is found (assuming "connect by" was used)
            	while (!stack.empty() && stack.last()->id() != parentid)
            	{
            		stack.pop_back();
            	}
            	if (stack.empty())
            		throw QString::fromLatin1("toResultPlan:ViewVSQLPlan returned rows in wrong order");

            	stack.last()->appendChild(itemNew = new toPlanTreeItem(id, columnData, stack.last()));
            }
            // set new stack top
            stack.append(itemNew);

        }
    }
    catch (const QString &str)
    {
        Utils::toStatusMessage(str);
        disconnect(Query, 0, this, 0);
        Query->stop();
        Query->deleteLater();
        Query = NULL;
        stack.clear();
    }
}

void toResultPlanModel::slotQueryDone(toEventQuery*q)
{
	emit layoutChanged();
	cleanup();
	emit queryDone(q);
}
