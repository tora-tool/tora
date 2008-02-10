/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include <list>

#include "totabwidget.h"

#include <qobject.h>
#include <QTabWidget>


toTabWidget::toTabWidget(QWidget *parent)
    : QTabWidget(parent) {

    connect(this,
            SIGNAL(currentChanged(int)),
            this,
            SLOT(tabChanged(int)));
}


void toTabWidget::setTabShown(QWidget *w, bool show) {
    int      pos;
    QWidget *parent = w;

    if(!show) {
        while((pos = indexOf(parent)) < 0 && parent && parent != this)
            parent = parent->parentWidget();

        if(pos < 0 || parent == this)
            return;                 // not found

        QTabWidget::removeTab(pos);
    }
    else {
        while(!tabs.contains(parent) && parent && parent != this)
            parent = parent->parentWidget();

        if(!tabs.contains(parent) || parent == this || !parent)
            return;                 // not found

        struct page r = tabs[parent];

        QTabWidget::insertTab(r.index, parent, r.label);
    }
}


int toTabWidget::addTab(QWidget *widget, const QString &label) {
    struct page p;
    p.label = label;
    p.index = QTabWidget::addTab(widget, label);
    tabs[widget] = p;
    return p.index;
}


int toTabWidget::addTab(QWidget *widget, const QIcon& icon, const QString &label) {
    struct page p;
    p.label = label;
    p.index = QTabWidget::addTab(widget, icon, label);
    tabs[widget] = p;
    return p.index;
}


void toTabWidget::tabChanged(int index) {
    QWidget *widget = QTabWidget::widget(index);
    if(widget)
        emit currentTabChanged(widget);
}


// ---------------------------------------- unused

int toTabWidget::insertTab(int index, QWidget *widget, const QString &label) {
    return QTabWidget::insertTab(index, widget, label);
}


int toTabWidget::insertTab(int index,
                           QWidget *widget,
                           const QIcon& icon,
                           const QString &label) {
    return QTabWidget::insertTab(index, widget, icon, label);
}


void toTabWidget::removeTab(int index) {
    QTabWidget::removeTab(index);
}
