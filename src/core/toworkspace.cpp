
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

#include "core/toworkspace.h"
#include "core/totool.h"
#include "core/toglobalevent.h"
#include "core/utils.h"
#include "ts_log/ts_log_utils.h"

#include <QtGui/QTabBar>
#include <QtGui/QStackedWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtCore/QSignalMapper>
#include <QtCore/QVariant>
#include <QtGui/QPixmap>
#include <QtGui/QIcon>

toWorkSpace::toWorkSpace(QWidget *parent)
    : QWidget(parent)
	, m_lastWidget(NULL)
{
	//setAutoFillBackground(true);
	//setPalette(QPalette(Qt::white));

	m_signalMapper = new QSignalMapper(this);

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setSpacing(0);
	vbox->setContentsMargins(0, 0, 0, 0);

	m_tabBar = new QTabBar(this);
	m_tabBar->setObjectName(QString::fromUtf8("MainToolBar"));
	m_tabBar->setTabsClosable(true);
	m_tabBar->setMovable(true);
	m_tabBar->setFocusPolicy(Qt::NoFocus);
	connect(m_tabBar, SIGNAL(currentChanged(int))   , this, SLOT(slotCurrentIndexChanged(int)));
	connect(m_tabBar, SIGNAL(tabMoved(int,int))     , this, SLOT(slotTabMoved(int,int)));
	connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(slotTabCloseRequested(int)));
	connect(m_signalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(slotToolCaptionChanged(QWidget*)));
	
	vbox->addWidget(m_tabBar);

	m_stackedWidget = new QStackedWidget(this);
	m_tabBar->setObjectName(QString::fromUtf8("ToolWidgetArea"));
	QWidget *page = new QWidget();
	page->setObjectName(QString::fromUtf8("page"));
	m_stackedWidget->addWidget(page);

	vbox->addWidget(m_stackedWidget);

	m_label = new QLabel("Blablabla", this);
	
	vbox->addWidget(m_label);
}

void toWorkSpace::addToolWidget(toToolWidget *toolWidget)
{
    Q_ASSERT_X(!m_toolsRegistry.contains(toolWidget), qPrintable(__QHERE__), "Tool window already added");
	// Create the tab and the stacked widget
    ToolIndex i;
    i.WidgetIndex = m_stackedWidget->addWidget(toolWidget);
	i.TabBarIndex = m_tabBar->insertTab(m_tabBar->count(), toolWidget->windowTitle()); // note this emits currentIndexChanged
	m_toolsRegistry.insert(toolWidget, i);

	const QPixmap *image = toolWidget->tool().toolbarImage();
	if (image)
		m_tabBar->setTabIcon(i.TabBarIndex, *image);

	// associate the tab with some useful QVariant data
	QVariant data = qVariantFromValue(static_cast<QWidget*>(toolWidget));
	m_tabBar->setTabData(i.TabBarIndex, data);
	if(m_lastWidget == NULL) // 0th tab was added (slotCurrentIndexChanged did nothing)
	{
		m_lastWidget = toolWidget;
		m_stackedWidget->setCurrentWidget(m_lastWidget);
		emit activeToolChaged(toolWidget); // => toTool::slotWindowActivated
		///toolWidget->setFocus(); //(does not work for unknown reason - toTool::slotWindowActivated should handle this)
	} else {
		m_tabBar->setCurrentIndex(i.TabBarIndex); // also sometimes called from m_tabBar->insertTab
	}

	m_signalMapper->setMapping(toolWidget, toolWidget);
	connect(toolWidget, SIGNAL(toolCaptionChanged()), m_signalMapper, SLOT(map()));

	Q_ASSERT_X(m_lastWidget != NULL, qPrintable(__QHERE__), "Tool widgets list corrupted");
	toGlobalEventSingle::Instance().toolWidgetAdded(toolWidget);
}

void toWorkSpace::slotCurrentIndexChanged(int idx)
{
	if(idx == -1)
		return; // no tab opened

	QWidget *wp = m_tabBar->tabData(idx).value<QWidget*>();
	toToolWidget *w  = dynamic_cast<toToolWidget*>(wp);
	if(w)
	{
		ToolIndex i = m_toolsRegistry.value(w);
		if(i.TabBarIndex != idx) // Some tool was closed and numerical indexes were shifted
		{
			i.TabBarIndex = idx;
			i.WidgetIndex = m_stackedWidget->indexOf(w);
			m_toolsRegistry.insert(w, i);
		}

		m_lastWidget = w;
		int WidgetIndex = m_toolsRegistry.value(w).WidgetIndex;
		m_stackedWidget->setCurrentWidget(m_lastWidget);
		emit activeToolChaged(w);  // => toTool::slotWindowActivated
		///w->setFocus(); // (does not work for unknown reason - toTool::slotWindowActivated should handle this)
	} else {
		// w == NULL => this was called from insertTab
		// new tab was added but so far it has NO widget
		// and also has no UserData
		// NOTE: this applies to 0th tab only
		//Q_ASSERT_X(false, (__HERE__).c_str(), "No tool widget for index");
		m_lastWidget = NULL;
	}

	m_label->setText(QString("%1").arg(idx));
}

void toWorkSpace::slotTabMoved(int from, int to)
{
	m_label->setText(QString("%1 %2").arg(from).arg(to));

	toToolWidget *w  = dynamic_cast<toToolWidget*>(m_tabBar->tabData(to).value<QWidget*>());
	if(!w)
		return;

	ToolIndex i = m_toolsRegistry.value(w);
	if(i.TabBarIndex != to) // Some tool was closed and numerical indexes were shifted
	{
		i.TabBarIndex = to;
		i.WidgetIndex = m_stackedWidget->indexOf(w);
		m_toolsRegistry.insert(w, i);
	}
}

void toWorkSpace::slotTabCloseRequested(int idx)
{
	toToolWidget *w = dynamic_cast<toToolWidget*>(m_tabBar->tabData(idx).value<QWidget*>());
	Q_ASSERT_X(w != NULL, qPrintable(__QHERE__), "Trying to close non-existing tab");

	m_tabBar->setCurrentIndex(idx); // show tab before showing Save dialog
	if(w->close())
	{		
		m_lastWidget = NULL;
		m_stackedWidget->removeWidget(w);
		m_tabBar->removeTab(idx);            // this may emit currentChanged => m_lastWidget will be overwritten

		int i = m_toolsRegistry.remove(w);
		Q_ASSERT_X(i == 1, qPrintable(__QHERE__), "m_toolsRegistry.remove() <> 1");

		delete w;
		m_label->setText(QString("*%1->%2").arg(idx).arg(m_tabBar->currentIndex()));
	}
}

void toWorkSpace::slotToolCaptionChanged(QWidget *w)
{
	toToolWidget *tool = dynamic_cast<toToolWidget*>(w);
	Q_ASSERT_X(tool != NULL, qPrintable(__QHERE__), "signal has unknown source");

	int index = m_toolsRegistry.value(tool).TabBarIndex;
	QString caption = m_tabBar->tabText(index);
	if( caption != tool->windowTitle())
		m_tabBar->setTabText(index, tool->windowTitle());
	//m_tabBar->setTabIcon(index, tool->windowIcon());
}

QList<toToolWidget*> toWorkSpace::toolWindowList() const
{
	QList<toToolWidget*> retval;
	for(int idx = 0; idx < m_tabBar->count(); idx++)
	{
		toToolWidget *w = dynamic_cast<toToolWidget*>(m_tabBar->tabData(idx).value<QWidget*>());
		if(w)
			retval << w;
	}
	return retval;
}

toToolWidget* toWorkSpace::currentTool() const
{
	int idx = m_tabBar->currentIndex();
	if (idx == -1)
		return NULL;
	toToolWidget *w = dynamic_cast<toToolWidget*>(m_tabBar->tabData(idx).value<QWidget*>());
	return w;
}

void toWorkSpace::setCurrentTool(toToolWidget* tool)
{
	Q_ASSERT_X(tool != NULL, qPrintable(__QHERE__), "Tool widget == NULL");
	int idx = -1;
	for(int i = 0; i < m_tabBar->count(); i++)
	{
		toToolWidget *w = dynamic_cast<toToolWidget*>(m_tabBar->tabData(i).value<QWidget*>());
		if(tool == w)
		{
			idx = i;
			break;
		}
	}
	if (idx == -1)
		return;

	ToolIndex i = m_toolsRegistry.value(tool);
	if(i.TabBarIndex != idx) // Some tool was closed and numerical indexes were shifted
	{
		i.TabBarIndex = idx;
		i.WidgetIndex = m_stackedWidget->indexOf(tool);
		m_toolsRegistry.insert(tool, i);
	}

	m_lastWidget = tool;
	int WidgetIndex = m_toolsRegistry.value(tool).WidgetIndex;
	m_stackedWidget->setCurrentWidget(m_lastWidget);
	m_tabBar->setCurrentIndex(idx);
	emit activeToolChaged(tool);  // => toTool::slotWindowActivated
}

bool toWorkSpace::closeToolWidget(toToolWidget* tool)
{
	Q_ASSERT_X(tool != NULL, qPrintable(__QHERE__), "Tool widget == NULL");
	int idx = -1;
	for(int i = 0; i < m_tabBar->count(); i++)
	{
		toToolWidget *w = dynamic_cast<toToolWidget*>(m_tabBar->tabData(i).value<QWidget*>());
		if(tool == w)
		{
			idx = i;
			break;
		}
	}
	Q_ASSERT_X(idx != -1, qPrintable(__QHERE__), "Unknown Tool widget to close");

	ToolIndex i = m_toolsRegistry.value(tool);
	m_tabBar->setCurrentIndex(idx); // show tab before showing Save dialog
	if(tool->close())
	{
		m_stackedWidget->removeWidget(tool);
		m_tabBar->removeTab(idx);

		int i = m_toolsRegistry.remove(tool);
		Q_ASSERT_X(i == 1, qPrintable(__QHERE__), "m_toolsRegistry.remove() <> 1");

		delete tool;
		m_label->setText(QString("*%1").arg(idx));
		m_lastWidget = NULL;

		return true;
	}
	return false;
}

void toWorkSpace::closeAllToolWidgets()
{
	// 1st close the current tool window
	toToolWidget *tool;
	while ((tool = currentTool()) != NULL)
	{
		if (closeToolWidget(tool))
			continue;
		break;
	}
}
