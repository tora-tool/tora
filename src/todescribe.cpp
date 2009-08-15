
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

#include <QSettings>
#include <QHideEvent>
#include <QMessageBox>

#include "toconnection.h"
#include "tobrowsertablewidget.h"
#include "tobrowserviewwidget.h"
#include "tobrowserindexwidget.h"
#include "tobrowsersequencewidget.h"
#include "tobrowsercodewidget.h"
#include "tobrowsersynonymwidget.h"
#include "tobrowsertriggerwidget.h"
#include "utils.h"
#include "todescribe.h"



toDescribe::toDescribe(QWidget * parent)
    : QDialog(parent)
{
    setupUi(this);

    QSettings s;
    s.beginGroup("toDescribe");
    restoreGeometry(s.value("geometry", QByteArray()).toByteArray());
    s.endGroup();
}

void toDescribe::hideEvent(QHideEvent * event)
{
    QSettings s;
    s.beginGroup("toDescribe");
    s.setValue("geometry", saveGeometry());
    s.endGroup();
    event->accept();
}

void toDescribe::changeParams(const QString & owner, const QString & object)
{
    if (widget)
    {
        delete widget;
        widget = 0;
    }

    QString objectType;

    std::list<toConnection::objectName> objects = toCurrentConnection(this).objects(false);
    for (std::list<toConnection::objectName>::iterator i = objects.begin();i != objects.end();i++)
    {
        if ((*i).Name == object && (*i).Owner == owner)
        {
            objectType = (*i).Type;
            break;
        }
    }

    if (objectType == "TABLE")
        widget = new toBrowserTableWidget(this);
    else if (objectType == "VIEW")
        widget = new toBrowserViewWidget(this);
    else if (objectType == "INDEX")
        widget = new toBrowserIndexWidget(this);
    else if (objectType == "SEQUENCE")
        widget = new toBrowserSequenceWidget(this);
    else if (objectType == "SYNONYM")
        widget = new toBrowserSynonymWidget(this);
    else if (objectType == "FUNCTION"
             || objectType == "PROCEDURE"
             || objectType == "PACKAGE"
             || objectType == "PACKAGE BODY")
        widget = new toBrowserCodeWidget(this);
    else if (objectType == "TRIGGER")
        widget = new toBrowserTriggerWidget(this);
    else
        QMessageBox::information(this, "Describe",
                                  tr("Object %1 (%2) cannot be described").arg(object, owner));

    if (!widget)
        return;

    layout()->addWidget(widget);
    widget->changeParams(owner, object);
    show();
}
