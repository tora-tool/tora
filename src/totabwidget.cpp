
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

#include <list>

#include "totabwidget.h"

#include <qobject.h>
#include <QTabWidget>

#define CONNECT_CHANGED                         \
    connect(this,                               \
            SIGNAL(currentChanged(int)),        \
            this,                               \
            SLOT(tabChanged(int)));

#define DISCONNECT_CHANGED                      \
    disconnect(this,                            \
               SIGNAL(currentChanged(int)),     \
               this,                            \
               SLOT(tabChanged(int)));

toTabWidget::toTabWidget(QWidget *parent)
        : QTabWidget(parent)
{

    CONNECT_CHANGED;
}


void toTabWidget::setTabShown(QWidget *w, bool show)
{
    int      pos;
    QWidget *parent = w;

    if (!show)
    {
        while ((pos = indexOf(parent)) < 0 && parent && parent != this)
            parent = parent->parentWidget();

        if (pos < 0 || parent == this)
            return;                 // not found

        DISCONNECT_CHANGED;
        QTabWidget::removeTab(pos);
        CONNECT_CHANGED;
    }
    else
    {
        while (!tabs.contains(parent) && parent && parent != this)
            parent = parent->parentWidget();

        if (!tabs.contains(parent) || parent == this || !parent)
            return;                 // not found

        struct page r = tabs[parent];

        DISCONNECT_CHANGED;
        QTabWidget::insertTab(r.index, parent, r.label);
        CONNECT_CHANGED;
    }
}


int toTabWidget::addTab(QWidget *widget, const QString &label)
{
    struct page p;
    p.label = label;
    p.index = QTabWidget::addTab(widget, label);
    tabs[widget] = p;
    return p.index;
}


int toTabWidget::addTab(QWidget *widget, const QIcon& icon, const QString &label)
{
    struct page p;
    p.label = label;
    p.index = QTabWidget::addTab(widget, icon, label);
    tabs[widget] = p;
    return p.index;
}


void toTabWidget::tabChanged(int index)
{
    QWidget *widget = QTabWidget::widget(index);
    if (widget)
        emit currentTabChanged(widget);
}


// ---------------------------------------- unused

int toTabWidget::insertTab(int index, QWidget *widget, const QString &label)
{
    return QTabWidget::insertTab(index, widget, label);
}


int toTabWidget::insertTab(int index,
                           QWidget *widget,
                           const QIcon& icon,
                           const QString &label)
{
    return QTabWidget::insertTab(index, widget, icon, label);
}


void toTabWidget::removeTab(int index)
{
    QTabWidget::removeTab(index);
}
