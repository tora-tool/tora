
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

#include "core/toglobalevent.h"
#include "core/toeditwidget.h"

#include <QtCore/QObject>
#include <QtCore/QString>

toGlobalEvent::toGlobalEvent() : QObject()
{
}

void toGlobalEvent::checkCaching()
{
    emit s_checkCaching();
}

void toGlobalEvent::editOpenFile(const QString &filename)
{
    emit s_editOpenFile(filename);
}

void toGlobalEvent::addRecentFile(const QString &filename)
{
    emit s_addRecentFile(filename);
}

void toGlobalEvent::openPreferences()
{
    emit s_openPreferences();
}

void toGlobalEvent::addCustomMenu(QMenu *menu)
{
    emit s_addCustomMenu(menu);
}

void toGlobalEvent::toolWidgetAdded(toToolWidget *tool)
{
    emit s_toolWidgetAdded(tool);
}

void toGlobalEvent::toolWidgetRemoved(toToolWidget *tool)
{
    emit s_toolWidgetRemoved(tool);
}

void toGlobalEvent::toolWidgetsReordered()
{
    emit s_toolWidgetsReordered();
}

void toGlobalEvent::setCoordinates(int x, int y)
{
    emit s_setCoordinates(x, y);
}

void toGlobalEvent::createDefaultTool(void)
{
    emit s_createDefaultTool();
}

void toGlobalEvent::showMessage(const QString &str, bool save, bool log)
{
    emit s_showMessage(str, save, log);
}

void toGlobalEvent::addConnection(toConnection *conn, bool def)
{
    emit s_addConnection(conn, def);
}

void toGlobalEvent::setNeedCommit(toToolWidget *tool, bool needCommit)
{
    emit s_setNeedCommit(tool, needCommit);
}
