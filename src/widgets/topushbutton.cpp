
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

#include "widgets/topushbutton.h"

#include <QStyleOptionButton>
#include <QtGui/QPainter>
#include <QStylePainter>
#include <QToolButton>
#include <QToolBar>
#include <QGestureEvent>
#include <QtCore/QStringListModel>
#include <QListView>

/**
 * much inspiration from libqxt
 *
 * http://www.libqxt.org/page/
 */

static const int VERTICAL_MASK = 0x02;


toRotatingButton::toRotatingButton(QWidget *parent)
    : QPushButton(parent),
      rotate(NoRotation)
{
}


toRotatingButton::toRotatingButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent),
      rotate(NoRotation)
{
}


toRotatingButton::toRotatingButton(const QIcon &icon,
                                   const QString &text,
                                   QWidget *parent)
    : QPushButton(icon, text, parent),
      rotate(NoRotation)
{
}


void toRotatingButton::setRotation(Rotation rotation)
{
    if (rotate != rotation)
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


QSize toRotatingButton::sizeHint() const
{
    QSize size = QPushButton::sizeHint();
    if (rotate & VERTICAL_MASK)
        size.transpose();

    return size;
}


QSize toRotatingButton::minimumSizeHint() const
{
    return sizeHint();
}


QStyleOptionButton toRotatingButton::getStyleOption()
{
    QStyleOptionButton option;
    QPushButton::initStyleOption(&option);

    if (rotate & VERTICAL_MASK)
    {
        QSize size = option.rect.size();
        size.transpose();
        option.rect.setSize(size);
    }

    return option;
}


void toRotatingButton::paintEvent(QPaintEvent *)
{
    updateGeometry();

    QStylePainter painter(this);
    painter.rotate(rotate);

    switch (rotate)
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

toPopupButton::toPopupButton(const QIcon &iconSet,
                             const QString &textLabel,
                             const QString &grouptext,
                             QToolBar *parent,
                             const char *name)
    : QToolButton(parent)
{

    setObjectName(name);
    setIcon(iconSet);
    setText(textLabel);
    setToolTip(grouptext);
}

toPopupButton::toPopupButton(QWidget *parent, const char *name)
    : QToolButton(parent)
{
    setObjectName(name);
}

toToggleButton::toToggleButton(QMetaEnum const& e, QWidget *parent, const char* name)
    : QPushButton(parent)
    , m_enum(e)
    , m_idx(0)
    , listView(NULL)
{
    if (m_enum.keyCount() == 0)
        throw QString("ctor toToggleButton - empty enum provided: %1").arg(m_enum.name());

    setObjectName(name);
    setFocusPolicy(Qt::NoFocus);
    setFlat(true);
    setText(m_enum.key(m_idx));

    connect(this, SIGNAL(released()), this, SLOT(toggle()));
}

// Note popup list view steals focus, it can not be used in toEditorTypeButton, toHighlighterTypeButton yet
void toToggleButton::enablePopUp()
{
    /* TapAndHoldGesture list menu part */
    grabGesture(Qt::TapAndHoldGesture);
    QStringList values;
    for (int idx = 0; idx < m_enum.keyCount(); idx++)
    {
        values.append(m_enum.key(idx));
    }
    QStringListModel *model = new QStringListModel(values, this);
    listView = new QListView(this);
    listView->setWindowFlags(Qt::ToolTip);
    listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listView->setModel(model);
    listView->setFixedSize(listView->sizeHintForColumn(0) + 2 * listView->frameWidth()
                           , listView->sizeHintForRow(0) * values.size() + 2 * listView->frameWidth());
    listView->setFocusPolicy(Qt::NoFocus);

    connect(listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(toggle(const QModelIndex &)));
}

void toToggleButton::setValue(int val)
{
    for (int idx = 0; idx < m_enum.keyCount(); idx++)
    {
        if (val == m_enum.value(idx))
        {
            m_idx = idx;
            setText(m_enum.key(m_idx));
        }
    }
}

void toToggleButton::setValue(QString const& val)
{
    for (int idx = 0; idx < m_enum.keyCount(); idx++)
    {
        if (val == m_enum.key(idx))
        {
            m_idx = idx;
            setText(m_enum.key(m_idx));
        }
    }
}

void toToggleButton::toggle()
{
    // Ignore events when in ListView
    if (listView && listView->isVisible())
        return;

    int idx = ++m_idx < m_enum.keyCount() ? m_idx : m_idx=0;
    setText(m_enum.key(idx));
    emit toggled(text());
    emit toggled(m_enum.value(idx));
}

void toToggleButton::toggle(const QModelIndex &index)
{
    QString value = listView->model()->data(index, Qt::DisplayRole).toString();
    listView->hide();
    setValue(value);
    emit toggled(text());
    emit toggled(m_idx);
}

bool toToggleButton::event(QEvent *event)
{
    if (listView && event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent*>(event));
    return QPushButton::event(event);
}

bool toToggleButton::gestureEvent(QGestureEvent *event)
{
    if (QGesture *swipe = event->gesture(Qt::TapAndHoldGesture))
    {
        QPoint p(0, height());
        int x = mapToGlobal(p).x();
        int y = mapToGlobal(p).y() - listView->height();
        listView->move(x, y);
        listView->show();
    }
    return true;
}

void toToggleButton::focusOutEvent(QFocusEvent *e)
{
    if(listView)
        listView->hide();
    QPushButton::focusOutEvent(e);
}
