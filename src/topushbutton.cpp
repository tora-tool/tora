
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

#include "topushbutton.h"

#include <QStyleOptionButton>
#include <QPainter>
#include <QStylePainter>


/**
 * much inspiration from libqxt
 *
 * http://www.libqxt.org/page/
 */

static const int VERTICAL_MASK = 0x02;


toPushButton::toPushButton(QWidget *parent)
    : QPushButton(parent),
      rotate(NoRotation)
{
}


toPushButton::toPushButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent),
      rotate(NoRotation)
{
}


toPushButton::toPushButton(const QIcon &icon,
                           const QString &text,
                           QWidget *parent)
    : QPushButton(icon, text, parent),
      rotate(NoRotation)
{
}


void toPushButton::setRotation(Rotation rotation)
{
    if(rotate != rotation)
    {
        rotate = rotation;
        switch (rotation)
        {
        case NoRotation:
        case UpsideDown:
            setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
            break;

        case Clockwise:
        case CounterClockwise:
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
            break;

        default:
            // nothing to do
            break;
        }

        updateGeometry();
        update();
    }
}


QSize toPushButton::sizeHint() const
{
    QSize size = QPushButton::sizeHint();
    if(rotate & VERTICAL_MASK)
        size.transpose();

    return size;
}


QSize toPushButton::minimumSizeHint() const
{
    return sizeHint();
}


QStyleOptionButton toPushButton::getStyleOption()
{
    QStyleOptionButton option;
    QPushButton::initStyleOption(&option);

    if(rotate & VERTICAL_MASK)
    {
        QSize size = option.rect.size();
        size.transpose();
        option.rect.setSize(size);
    }

    return option;
}


void toPushButton::paintEvent(QPaintEvent *)
{
    updateGeometry();

    QStylePainter painter(this);
    painter.rotate(rotate);

    switch(rotate)
    {
    case UpsideDown:
        painter.translate(-width(), -height());
        break;

    case Clockwise:
        painter.translate(0, -width());
        break;

    case CounterClockwise:
        painter.translate(-height(), 0);
        break;

    default:
        // nothing to do
        break;
    }


    const QStyleOptionButton option = getStyleOption();
    painter.drawControl(QStyle::CE_PushButton, option);
}
