#include <QtDebug>
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

#include <QApplication>

#include "toresult.h"
#include "toresultdata.h"
#include "tobrowserbasewidget.h"


toBrowserBaseWidget::toBrowserBaseWidget(QWidget * parent)
    : QTabWidget(parent),
    m_schema(0),
    m_object(0)
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
        toResultData * d = dynamic_cast<toResultData*>(i);
        if (d)
            ret &= d->maybeSave();
    }
    return ret;
}

void toBrowserBaseWidget::addTab(QWidget * page, const QString & label)
{
    // show must go *before* QTabWidget::addTab() to prevent
    // widgets glitches in the other tabs.
    page->setVisible(true);
    QTabWidget::addTab(page, label);

    toResult * r = dynamic_cast<toResult*>(page);

//     qDebug() << objectName() << label <<  page;
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
}

void toBrowserBaseWidget::changeParams(const QString & schema, const QString & object)
{
    if (m_schema != schema || m_object != object)
    {
        m_schema = schema;
        m_object = object;
        updateData(currentWidget()->objectName());
    }
}

void toBrowserBaseWidget::changeConnection()
{
    m_schema = "";
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

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QString key(currentWidget()->objectName());

    if ((!m_cache.contains(key))
        || (m_cache[key].first != m_schema)
        || (m_cache[key].second != m_object))
    {
        m_cache[key] = qMakePair(m_schema, m_object);
        updateData(key);
    }
    QApplication::restoreOverrideCursor();
}

void toBrowserBaseWidget::updateData(const QString & ix)
{
    if (schema().isEmpty() || object().isEmpty())
        return;
    m_tabs[ix]->changeParams(schema(), object());
}
