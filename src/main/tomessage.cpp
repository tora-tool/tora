
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

#include "main/tomessage.h"

#include "editor/toscintilla.h"

#include <QPushButton>
#include <QtCore/QDateTime>
#include <QtCore/QSettings>

#include <algorithm>

toMessage::toMessage(QWidget * parent, toWFlags f)
    : QDialog(parent, f)
{
    setupUi(this);
    setModal(false);
    Message->setReadOnly(true);
    buttonBox->button(QDialogButtonBox::Close)->setFocus(Qt::OtherFocusReason);

    QSettings s;
    s.beginGroup("toMessage");
    restoreGeometry(s.value("geometry", QByteArray()).toByteArray());
    s.endGroup();

    Message->installEventFilter(this);

    connect(buttonBox->button(QDialogButtonBox::Close),
            SIGNAL(clicked()), this, SLOT(hide()));
    connect(buttonBox->button(QDialogButtonBox::Reset),
            SIGNAL(clicked()), Message, SLOT(clear()));
}

void toMessage::appendText(const QString & text)
{
    if (!isVisible())
        show();

    int firstLine = Message->lines();
    if (Message->lines() != 1)
        Message->append("\n\n");
    Message->append(QDateTime::currentDateTime().toString(Qt::TextDate) + "\n");
    Message->append(text);
    Message->ensureLineVisible((std::min)(firstLine+10, Message->lines()));
}

void toMessage::hideEvent(QHideEvent * event)
{
    QSettings s;
    s.beginGroup("toMessage");
    s.setValue("geometry", saveGeometry());
    s.endGroup();
    event->accept();
}

bool toMessage::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != Message)
        return false;
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Escape || keyEvent->key() == Qt::Key_Return)
        {
            close();
            return true; // mark the event as handled
        }
        if(keyEvent->key() == Qt::Key_X && (keyEvent->modifiers() & Qt::ControlModifier))
        {
            Message->clear();
            return true;
        }
    }
    return false;
}
