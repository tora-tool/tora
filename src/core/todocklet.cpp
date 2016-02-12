
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

#include "widgets/todockbar.h"
#include "core/todocklet.h"
#include "core/tomainwindow.h"
#include "widgets/toworkspace.h"
#include "core/utils.h"

#include <QApplication>
#include <QVBoxLayout>


static QMap<QString, toDockletHolder *>& Docklets()
{
    static QMap<QString, toDockletHolder *> Docklets;
    return Docklets;
}


toDocklet::toDocklet(const QString &title,
                     QWidget *parent,
                     toWFlags flags)
    : QDockWidget(title, parent, flags),
      isPopup(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    connect(this,
            SIGNAL(activated()),
            this,
            SLOT(childActivated()));

    connect(this,
            SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this,
            SLOT(handleDockletLocationChanged(Qt::DockWidgetArea)));
}


toDockletHolder* toDocklet::registerDocklet(QString name, toDockletHolder *holder)
{
    Docklets()[name] = holder;
    return holder;
}


toDocklet* toDocklet::docklet(QString name)
{
    foreach(toDockletHolder * holder, Docklets().values())
    {
        toDocklet *let = holder->docklet();
        if (let->name() == name)
            return let;
    }

    return 0;
}


QList<toDocklet *> toDocklet::docklets()
{
    QList<toDockletHolder *> holders = Docklets().values();
    QList<toDocklet *> ret;
    foreach(toDockletHolder * hold, holders)
    ret.append(hold->docklet());
    return ret;
}


void toDocklet::showPopup(bool shown)
{
    setFocusProxy(widget());

    if (shown)
    {
        // if this window is docked it'll screw with the viewport
        // measurements, so make sure to hide it and allow QMainWidget
        // to redraw the layout first.
        hide();
        qApp->processEvents();

        setFloating(true);
        // setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
        show();

        toDockbar *dockbar = toMainWindow::lookup()->dockbar(this);
        Qt::ToolBarArea area = dockbar->area();

        QPoint pos;
        if (area == Qt::LeftToolBarArea)
            pos.setX(dockbar->geometry().width());
        else if (area == Qt::RightToolBarArea)
            pos.setX(-(this->geometry().width()));
        pos = dockbar->mapToGlobal(pos);
        move(pos);

        QRect geom = geometry();
        geom.setHeight(toWorkSpaceSingle::Instance().geometry().height());
        setGeometry(geom);

        isPopup = shown;
        raise();
        qApp->setActiveWindow(this);            // works
        activateWindow();                       // doesn't work
        widget()->setFocus();
    }
    else
    {
        if (isPopup)                            // may be open in dock
        {
            // setWindowFlags(0);                  // defaults
            hide();
            isPopup = false;                    // not anymore
            setWindowModality(Qt::NonModal);
        }
    }

    emit popupVisibleToggled(isPopup);
}


bool toDocklet::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate)
    {
        showPopup(false);
        // not sure what i was trying to accomplish here, but this
        // gets in the way of various dialogs.

//         qApp->setActiveWindow(toMainWidget());
    }
    return QDockWidget::event(event);
}


void toDocklet::childActivated()
{
    showPopup(false);
}


void toDocklet::handleDockletLocationChanged(Qt::DockWidgetArea area)
{
    emit dockletLocationChanged(this, area);
}
