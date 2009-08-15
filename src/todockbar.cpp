
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

#include "todockbar.h"
#include "todocklet.h"

#include <QDataStream>
#include <QByteArray>


toDockbarButton::toDockbarButton(const QIcon &icon,
                                 const QString &text,
                                 QWidget *parent)
    : toPushButton(icon, text, parent)
{
}


toDockbar::toDockbar(Qt::ToolBarArea area,
                     const QString &title,
                     QWidget *parent)
    : QToolBar(title, parent)
{
    Area = area;

    setObjectName(title);
    setAllowedAreas(area);
    setFloatable(false);
    setMovable(false);

    if(Area == Qt::LeftToolBarArea || Qt::RightToolBarArea)
        setOrientation(Qt::Vertical);
}


void toDockbar::addDocklet(toDocklet *let, bool show)
{
    if(Docklets.contains(let->name()))
    {
        if(show)
            let->show();
        return;
    }

    // this action is not related to
    // QDockWidget::toggleViewAction. Clicking this activates a
    // Qt::Tool window that'll be dismissed after it looses focus.
    toDockbarButton *button = new toDockbarButton(let->icon(), let->name(), 0);
    if(Area == Qt::LeftToolBarArea)
        button->setRotation(toPushButton::CounterClockwise);
    else if(Area == Qt::RightToolBarArea)
        button->setRotation(toPushButton::Clockwise);

    button->setCheckable(true);
    addWidget(button);

    Docklets[let->name()] = button;

    this->show();

    if(show)
        let->setFocus(Qt::OtherFocusReason);
    else
        let->hide();

    connect(button,
            SIGNAL(clicked(bool)),
            let,
            SLOT(showPopup(bool)));

    connect(let,
            SIGNAL(popupVisibleToggled(bool)),
            button,
            SLOT(setChecked(bool)));
}


toDocklet* toDockbar::removeDocklet(QString name)
{
    toDocklet *let = toDocklet::docklet(name);
    if(!let)
        return 0;

    return removeDocklet(let);
}


toDocklet* toDockbar::removeDocklet(toDocklet *let)
{
    if(!Docklets.contains(let->name()))
        return 0;

    delete Docklets.take(let->name());

    // if that was the last docklet removed, hide the toolbar
    if(Docklets.size() < 1)
        hide();

    return let;
}


void toDockbar::setAllVisible(bool visible)
{
    foreach(QString name, Docklets.keys())
    {
        toDocklet *let = toDocklet::docklet(name);
        if(!let)
            continue;

        let->setVisible(visible);
    }
}


bool toDockbar::contains(toDocklet *let)
{
    return Docklets.contains(let->name());
}


QByteArray toDockbar::saveState() const
{
    QByteArray buf;
    QDataStream data(&buf, QIODevice::WriteOnly);

    foreach(QString name, Docklets.keys())
    {
        toDocklet *let = toDocklet::docklet(name);
        if(!let)
            continue;

        // these will be stored by main window saveState()
        if(let->isVisible())
            continue;

        data << let->name();
        data << let->geometry().width();
    }

    return buf;
}


void toDockbar::restoreState(QByteArray buf)
{
    QDataStream data(&buf, QIODevice::ReadOnly);

    while(!data.atEnd())
    {
        QString name;
        int     width;
        data >> name;
        data >> width;

        toDocklet *let = toDocklet::docklet(name);
        if(!let)
            continue;

        QRect geo = let->geometry();
        geo.setWidth(width);
        let->setGeometry(geo);
        addDocklet(let, false);
    }
}
