
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

#include "core/toeditwidget.h"
#include "core/tomainwindow.h"

std::list<toEditWidget::editHandler *> *toEditWidget::Handlers;

toEditWidget::toEditWidget()
	: FlagSet()
{
}

toEditWidget::~toEditWidget()
{
	lostFocus();
}

void toEditWidget::addHandler(toEditWidget::editHandler *handler)
{
    if (!Handlers)
        Handlers = new std::list<toEditWidget::editHandler *>;
    for (std::list<toEditWidget::editHandler *>::iterator i = (*Handlers).begin();
            i != (*Handlers).end();
            i++)
        if (*i == handler)
            return ;
    (*Handlers).insert((*Handlers).end(), handler);
}

void toEditWidget::delHandler(toEditWidget::editHandler *handler)
{
    for (std::list<toEditWidget::editHandler *>::iterator i = (*Handlers).begin();
            i != (*Handlers).end();
            i++)
        if (*i == handler)
        {
            (*Handlers).erase(i);
            break;
        }
}

void toEditWidget::lostFocus(void)
{
    if (!Handlers)
        return ;
    for (std::list<editHandler *>::iterator i = (*Handlers).begin();
            i != (*Handlers).end();
            i++)
    {
        (*i)->lostFocus(this);
    }
}

void toEditWidget::receivedFocus(void)
{
    if (!Handlers)
        return ;
    for (std::list<editHandler *>::iterator i = (*Handlers).begin();
            i != (*Handlers).end();
            i++)
    {
        (*i)->receivedFocus(this);
    }
}

toEditWidget::editHandler::editHandler()
{
    toEditWidget::addHandler(this);
}

toEditWidget::editHandler::~editHandler()
{
    toEditWidget::delHandler(this);
}

//bool toEditWidget::editOpen(QString)
//{
//    return true;
//}

//bool toEditWidget::editSave(bool)
//{
//    return true;
//}

toEditWidget *toEditWidget::findEdit(QWidget *widget)
{
    while (widget)
    {
        toEditWidget *edit = dynamic_cast<toEditWidget *>(widget);
        if (edit)
            return edit;
        widget = widget->parentWidget();
    }
    return NULL;
}
