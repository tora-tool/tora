
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

#include "toworkingwidget.h"
#include "icons/stop.xpm"


toWorkingWidget::toWorkingWidget(QWidget * parent)
    : QWidget(parent)
{
    CurrentType = toWorkingWidget::Interactive;

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

    WorkingStop = new QPushButton(QIcon(stop_xpm), tr("Stop"), HWorking);
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
}

void toWorkingWidget::show()
{
    if (CurrentType == toWorkingWidget::Interactive)
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
    bool e = (type == toWorkingWidget::Interactive);
    WorkingStop->setEnabled(e);
    WorkingStop->setVisible(e);
    WorkingStop->blockSignals(!e);
}
