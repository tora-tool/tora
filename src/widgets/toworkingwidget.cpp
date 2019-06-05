
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

#include "widgets/toworkingwidget.h"
#include "core/tologger.h"

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QtCore/QEvent>
#include <QtCore/QTimer>

#include "icons/stop.xpm"

toWorkingWidget::toWorkingWidget(QWidget * parent)
    : QWidget(parent)
{
    CurrentType = Interactive;

    setAutoFillBackground(true);
    setPalette(QPalette(Qt::white));

    HWorking = new QWidget(this);
    HWorking->setAutoFillBackground(true);
    HWorking->setPalette(QPalette(QColor(241, 241, 169)));

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *hbox = new QHBoxLayout;

    WorkingLabel = new QLabel(tr("Waiting..."), HWorking);
    WorkingLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    hbox->addWidget(WorkingLabel);

    WorkingStop = new QPushButton(QIcon(QPixmap(stop_xpm)), tr("Stop"), HWorking);
    WorkingStop->setAutoFillBackground(true);
    WorkingStop->setBackgroundRole(QPalette::Window);
    WorkingStop->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    WorkingStop->setEnabled(false);

    connect(WorkingStop, SIGNAL(clicked()), this, SLOT(stopWorking()));

    hbox->addWidget(WorkingStop);

    HWorking->setLayout(hbox);

    vbox->addWidget(HWorking);
    vbox->addStretch(0);

    setLayout(vbox);
    hide();
}

void toWorkingWidget::show()
{
    if (CurrentType == Interactive)
        HWorking->hide();
    QWidget::show();
}

void toWorkingWidget::forceShow()
{
    HWorking->show();
}

void toWorkingWidget::stopWorking()
{
    emit stop();
}

void toWorkingWidget::setText(const QString & text)
{
    WorkingLabel->setText(text);
    WorkingStop->setEnabled(true);
}

void toWorkingWidget::setType(WorkingWidgetType type)
{
    CurrentType = type;
    bool e = (type == Interactive);
    WorkingStop->setEnabled(e);
    WorkingStop->setVisible(e);
    WorkingStop->blockSignals(!e);
}

toWorkingWidgetNew::toWorkingWidgetNew(QWidget * parent)
    : QWidget(parent)
{
    CurrentType = Interactive;

    setAutoFillBackground(true);
    setPalette(QPalette(Qt::white));

    HWorking = new QWidget(this);
    HWorking->setAutoFillBackground(true);
    HWorking->setPalette(QPalette(QColor(241, 241, 169)));

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *hbox = new QHBoxLayout;

    WorkingLabel = new QLabel(tr("Waiting..."), HWorking);
    WorkingLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    hbox->addWidget(WorkingLabel);

    WorkingStop = new QPushButton(QIcon(QPixmap(stop_xpm)), tr("Stop"), HWorking);
    WorkingStop->setAutoFillBackground(true);
    WorkingStop->setBackgroundRole(QPalette::Window);
    WorkingStop->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    WorkingStop->setEnabled(CurrentType == Interactive);

    connect(WorkingStop, SIGNAL(clicked()), this, SLOT(stopWorking()));

    hbox->addWidget(WorkingStop);

    HWorking->setLayout(hbox);

    vbox->addWidget(HWorking);
    vbox->addStretch(0);

    setLayout(vbox);
    hide();

}

void toWorkingWidgetNew::show()
{
    if (CurrentType == Interactive)
        HWorking->hide();
    QWidget::show();
}

void toWorkingWidgetNew::forceShow()
{
    HWorking->show();
}

void toWorkingWidgetNew::stopWorking()
{
    emit stop();
}

void toWorkingWidgetNew::setText(const QString & text)
{
    WorkingLabel->setText(text);
    WorkingStop->setEnabled(true);
}

void toWorkingWidgetNew::setType(WorkingWidgetType type)
{
    CurrentType = type;
    bool e = (type == Interactive);
    WorkingStop->setEnabled(e);
    WorkingStop->setVisible(e);
    WorkingStop->blockSignals(!e);
}

bool toWorkingWidgetNew::eventFilter(QObject *obj, QEvent *event)
{
    auto type = event->type();
    switch (event->type())
    {
        case QEvent::Paint:
            setGeometry(parentWidget()->frameGeometry());
            show();
            TLOG(4, toDecorator, __HERE__) << "Paint" << std::endl;
            return true;
        case QEvent::Resize:
        case QEvent::Move:
            setGeometry(parentWidget()->frameGeometry());
            repaint();
            event->ignore();
            TLOG(4, toDecorator, __HERE__) << "Resize/Move" << std::endl;
            return true;
        default:
            // standard event processing
            return QWidget::eventFilter(obj, event);
    }
}

void toWorkingWidgetNew::display()
{
    parentWidget()->installEventFilter(this);
    QTimer::singleShot(300, this, SLOT(forceShow()));
    TLOG(4, toDecorator, __HERE__) << "display" << std::endl;
}

void toWorkingWidgetNew::undisplay()
{
    parentWidget()->removeEventFilter(this);
    if (isVisible())
    {
        hide();
        parentWidget()->repaint();
        TLOG(4, toDecorator, __HERE__) << "undisplay(1)" << std::endl;
    }
    TLOG(4, toDecorator, __HERE__) << "undisplay(2)" << std::endl;
}
