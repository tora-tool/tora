
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

#include "core/tobrowserbasewidget.h"
#include "core/toconnectiontraits.h"
#include "core/utils.h"
#include "result/toresulttabledata.h"

#include <QApplication>

toBrowserBaseWidget::toBrowserBaseWidget(QWidget * parent)
    : QTabWidget(parent)
{
    setObjectName("toBrowserBaseWidget");

    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(tabWidget_currentChanged(int)));
}

bool toBrowserBaseWidget::maybeSave()
{
    bool ret = true;
    foreach (toResult * i, m_tabs.values())
    {
        toResultTableData * d = dynamic_cast<toResultTableData*>(i);
        if (d)
            ret &= d->maybeSave();
    }
    return ret;
}

int toBrowserBaseWidget::addTab(QWidget * page, const QString & label)
{
    // show must go *before* QTabWidget::addTab() to prevent
    // widgets glitches in the other tabs.
    page->setVisible(true);
    int pos = QTabWidget::addTab(page, label);

    toResult * r = dynamic_cast<toResult*>(page);

//     TLOG(2,toDecorator,__HERE__) << objectName() << label <<  page;
    Q_ASSERT_X(r,
               "toBrowserBaseWidget::addTab",
               "new tab is not toResult child");
    Q_ASSERT_X(!page->objectName().isEmpty(),
               "toBrowserBaseWidget::addTab",
               "widget objectName cannot be empty; page must have objectName property set");
    Q_ASSERT_X(!m_tabs.contains(page->objectName()),
               "toBrowserBaseWidget::addTab",
               "widget objectName is already used; page objectName must be unique");

    m_tabs[page->objectName()] = r;
    return pos;
}

void toBrowserBaseWidget::changeParams(const QString & schema, const QString & object, const QString & type)
{
    // Note: "type" may be set when creating an object of a class and that value should
    //       be preserved even when empty value for "type" is given in changeParams.
    //       This also means that "type" cannot be reset to NULL here but that should
    //       never be required.
    if (m_schema != schema || m_object != object || (m_type != type && !type.isEmpty()))
    {
        m_schema = schema;
        m_object = object;
        if (!type.isEmpty())
            m_type   = type;
        updateData(currentWidget()->objectName());
    }
}

void toBrowserBaseWidget::changeConnection()
{
    m_schema = "";
    m_type   = "";
    m_object = "";

    m_tabs.clear();
    clear();
}

void toBrowserBaseWidget::tabWidget_currentChanged(int ix)
{
    // Re-read data from sql only when there is no cache for given
    // schema/object
    if (m_schema.isEmpty() || m_object.isEmpty())
        return;

    Utils::toBusy busy;
    QString key(currentWidget()->objectName());

    if ((!m_cache.contains(key))
            || (m_cache[key].first != m_schema)
            || (m_cache[key].second != m_object))
    {
        m_cache[key] = qMakePair(m_schema, m_object);
        updateData(key);
    }
}

void toBrowserBaseWidget::updateData(const QString & ix)
{
    // When changing connection or refreshing a list of objects (tables, indexes etc.)
    // updateData will be called with shema/object being empty (as after refreshing
    // nothing is selected yet. We have to clear result pane then without executing
    // any queries.
    if (schema().isEmpty() || object().isEmpty())
    {
        m_tabs[ix]->clearData();
        return;
    }

    toConnection &conn = toConnection::currentConnection(this);
    QString Schema = conn.getTraits().unQuote(schema());
    QString Object = conn.getTraits().unQuote(object());

    // Some result types need a type specified in order to get information on correct
    // object (when the same object name is used for objects of different types).
    if (currentWidget()->objectName() == "extractView")
    {
        m_tabs[ix]->refreshWithParams(toQueryParams() << Schema << Object << type());
        return;
    }

    if ((conn.providerIs("QMYSQL") || conn.providerIs("Teradata")) &&
            !type().isEmpty() &&
            (type() == "PROCEDURE" || type() == "FUNCTION" || type() == "MACRO"))
    {
        // MySQL requires additional parameter to fetch routine (procedure/function) creation script
        // Parameter type must be passed first because it is not possible to rearrange parameters
        // used in SQL.
        m_tabs[ix]->refreshWithParams(toQueryParams() <<  type() << Schema << Object);
    }
    else
    {
        m_tabs[ix]->refreshWithParams(toQueryParams() << Schema << Object);
    }
}

void toBrowserBaseWidget::setType(const QString & type)
{
    m_type = type;
} // setType
