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

#include <QPushButton>
#include <QDateTime>
#include <QSettings>

#include "tomessage.h"


toMessage::toMessage(QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    setupUi(this);
    setModal(false);
    Message->setReadOnly(true);

    QSettings s;
    s.beginGroup("toMessage");
    restoreGeometry(s.value("geometry", QByteArray()).toByteArray());
    s.endGroup();

    connect(buttonBox->button(QDialogButtonBox::Close),
            SIGNAL(clicked()), this, SLOT(hide()));
    connect(buttonBox->button(QDialogButtonBox::Reset),
            SIGNAL(clicked()), Message, SLOT(clear()));
}

void toMessage::appendText(const QString & text)
{
    if (!isVisible())
        show();

    if (Message->lines() != 1)
        Message->append("\n\n");
    Message->append(QDateTime::currentDateTime().toString(Qt::TextDate) + "\n");
    Message->append(text);
    Message->ensureLineVisible(Message->lines());
}

void toMessage::hideEvent(QHideEvent * event)
{
    QSettings s;
    s.beginGroup("toMessage");
    s.setValue("geometry", saveGeometry());
    s.endGroup();
    event->accept();
}
