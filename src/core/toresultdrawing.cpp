
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

#include <QVBoxLayout>

#include "core/toresultdrawing.h"
#include "core/utils.h"

#include "dotgraph.h"
#include "dotgraphview.h"

#include "icons/refresh.xpm"

#include <QStyle>
#include <QSlider>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QMouseEvent>
#include <QStyleOptionSlider>

class MySlider : public QSlider
{
public:
	explicit MySlider(QWidget *parent = 0) : QSlider(parent) {};
	explicit MySlider(Qt::Orientation orientation, QWidget *parent = 0) : QSlider(orientation, parent) {};
protected:
	void mousePressEvent ( QMouseEvent * event )
	{
		QStyleOptionSlider opt;
		initStyleOption(&opt);
		QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

		if (event->button() == Qt::LeftButton &&
				sr.contains(event->pos()) == false)
		{
			int newVal;
			if (orientation() == Qt::Vertical)
				newVal = minimum() + ((maximum()-minimum()) * (height()-event->y())) / height();
			else
				newVal = minimum() + ((maximum()-minimum()) * event->x()) / width();

			if (invertedAppearance() == true)
				setValue( maximum() - newVal );
			else
				setValue(newVal);

			event->accept();
		}
		QSlider::mousePressEvent(event);  }
};

// BIG FAT WARNING - this query causes: ORA-07445: exception encountered: core dump [lnxmin()+2384] [SIGSEGV]
// on 11gR1. Use "alter session set OPTIMIZER_FEATURES_ENABLE='10.2.0.4';" as a workaround
// Note: the query is not bi-directional
static toSQL SQLTableFKWithDistance(
			"toResultDrawing:FKConstraintsWithDistance",
                        " SELECT /*+ optimizer_features_enable('10.2.0.4') */ R.*                      \n"
                        " --, LEVEL as LVL                                                             \n"
			            " FROM (                                                           \n"
                        " SELECT                                                                       \n"
                        " c.constraint_name                                                            \n" // c1
                        " -- max(a.constraint_name) as constraint_name                                 \n"
                        " -- , c.constraint_name                                                       \n"
                        " , max(r.constraint_name) as r_constraint_name                                \n" // c2
                        " , max(c.owner)           as owner                                            \n" // c3
                        " , max(c.table_name)      as table_name                                       \n" // c4
                        " , c.column_name          as column_name                                      \n" // c5
                        " , max(r.owner)           as r_owner                                          \n" // c6
                        " , max(r.table_name)      as r_table_name                                     \n" // c7
                        " , max(r.column_name)     as r_column_name                                    \n" // c8
                        " , max(a.constraint_type)                                                     \n" // c9
                        " FROM sys.all_constraints a                                                   \n"
                        " JOIN sys.all_cons_columns c ON (c.constraint_name = a.constraint_name AND c.owner = a.owner)                                 \n"
                        " JOIN sys.all_cons_columns r ON (r.constraint_name = a.r_constraint_name AND r.owner = a.r_owner AND r.position = c.position) \n"
                        " WHERE                                                                        \n"
                        " a.r_owner =                   :f1<char[101]>                                 \n"
                        " AND a.constraint_type = 'R'                                                  \n"
                        " GROUP BY c.constraint_name, ROLLUP (c.column_name)                           \n"
                        " ) R                                                                          \n"
						" WHERE LEVEL <= :lvl<int>                                                     \n"
                        " START WITH r_table_name = :f2<char[101]>                                     \n"
                        " CONNECT BY NOCYCLE r_table_name = PRIOR table_name AND r_owner = PRIOR owner \n"
                        " UNION ALL                                                                    \n"
                        " SELECT R.*                                                                   \n"
                        " --, LEVEL as LVL                                                             \n"
                        " FROM (                                                                       \n"
                        " SELECT                                                                       \n"
                        " c.constraint_name                                                            \n"
                        " -- max(a.constraint_name) as constraint_name                                 \n"
                        " -- , c.constraint_name                                                       \n"
                        " , max(r.constraint_name) as r_constraint_name                                \n"
                        " , max(c.owner)           as owner                                            \n"
                        " , max(c.table_name)      as table_name                                       \n"
                        " , c.column_name          as column_name                                      \n"
                        " , max(r.owner)           as r_owner                                          \n"
                        " , max(r.table_name)      as r_table_name                                     \n"
                        " , max(r.column_name)     as r_column_name                                    \n"
                        " , max(a.constraint_type)                                                     \n"
                        " FROM sys.all_constraints a                                                   \n"
                        " JOIN sys.all_cons_columns c ON (c.constraint_name = a.constraint_name AND c.owner = a.owner)                                 \n"
                        " JOIN sys.all_cons_columns r ON (r.constraint_name = a.r_constraint_name AND r.owner = a.r_owner AND r.position = c.position) \n"
                        " WHERE                                                                        \n"
                        " a.r_owner =                   :f1<char[101]>                                 \n"
                        " AND a.constraint_type = 'R'                                                  \n"
                        " GROUP BY c.constraint_name, ROLLUP( c.column_name)                           \n"
                        " ) R                                                                          \n"
						" WHERE LEVEL <= :lvl<int>                                                     \n"
                        " START WITH table_name = :f2<char[101]>                                       \n"
                        " CONNECT BY NOCYCLE PRIOR r_table_name = table_name AND PRIOR r_owner = owner \n",
			"Get list of all the FK referenced/referencing tables within specified distance",
			"100000",
			"Oracle");


toResultDrawing::toResultDrawing(QWidget *parent, const char *name, toWFlags f)
	: QWidget(parent, f)
	, toResult()
	, distance(1)
{
	if (name)
		setObjectName(name);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(0);

	QToolBar *toolbar = Utils::toAllocBar(this, tr("Schema attributes"));
	vbox->addWidget(toolbar);

	refreshAct = toolbar->addAction(
		QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
		tr("Refresh data"));
	connect(refreshAct, SIGNAL(triggered()), this, SLOT(refreshAction()));

	toolbar->addSeparator();

	slider = new MySlider(Qt::Horizontal, toolbar);
	slider->setMinimum(1);
	slider->setMaximum(10);
	slider->setTickPosition(QSlider::TicksAbove);
	slider->setTickInterval(1);
	toolbar->addWidget(slider);
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setDistance(int)));
	
	m_dotGraphView = new DotGraphView(NULL, this);
	m_dotGraphView ->initEmpty();
	m_dotGraphView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	m_dotGraphView->prepareSelectSinlgeElement();
	vbox->addWidget(m_dotGraphView);
	this->setFocusProxy(m_dotGraphView);
	this->setLayout(vbox);
};


void toResultDrawing::query(const QString &, toQueryParams const& params)
{
	toResult::setParams(params);
	typedef QPair<QString, QString> Reference;
	toConnection &conn = connection();

	m_dotGraphView->initEmpty();
	QString schema(params.at(0)), table(params.at(1));
	QSet<QString> tables;
	QSet<Reference> references;

	tables.insert(table);

	DotGraph newGraph("dot");
	QMap<QString,QString> ga;
	ga["id"] = "Schema";
	ga["compound"] = "true";
	ga["shape"] = "box";
	ga["rankdir"] = "BT"; // BOTTOM to TOP arrows

	newGraph.setGraphAttributes(ga);

	toQValue c1, c2, c3, c4, c5, c6, c7, c8, c9;
	// TODO: use toCache here - additional attributes
	toConnectionSubLoan c(conn);
	toQuery QueryC(c, SQLTableFKWithDistance, toQueryParams() << schema.toUpper() << distance << table.toUpper());
	while (!QueryC.eof())
	{
		c1 = QueryC.readValue();
		c2 = QueryC.readValue();
		c3 = QueryC.readValue();
		c4 = QueryC.readValue();
		c5 = QueryC.readValue();
		c6 = QueryC.readValue();
		c7 = QueryC.readValue();
		c8 = QueryC.readValue();
		c9 = QueryC.readValue();

		if( c5.isNull() && c1.isNull())
		{
			// Here collect FK details (a column list for compound keys)
		}

		if( c5.isNull() && !c1.isNull()) // c5 (column_name) is null - see rollup def
		{
			tables.insert(c4); // table_name
			tables.insert(c7); // r_table_name
			references.insert( Reference( c4, c7));
		}
	}


	Q_FOREACH(QString const&t, tables)
	{
		QMap<QString,QString> ta; // table atributes
		ta["name"] = t;
		ta["label"] = t;
		ta["fontsize"] = "12";
		ta["comment"]= t;
		ta["id"]= t;
		ta["tooltip"] = t;
		newGraph.addNewNode(ta);
	}

	Q_FOREACH(Reference const&r, references)
	{
		QMap<QString,QString> ea; // edge attreibutes
		newGraph.addNewEdge(r.first, r.second, ea);
	}

	m_dotGraphView->graph()->updateWithGraph(newGraph);
	m_dotGraphView->graph()->update();
}

/** Clear result widget */
void toResultDrawing::clearData()
{
    m_dotGraphView->initEmpty();
}

void toResultDrawing::setDistance(int d)
{
	distance = d;
	refresh();
}
