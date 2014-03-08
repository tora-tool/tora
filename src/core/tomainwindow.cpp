
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

#include "core/tomainwindow.h"
#include "core/totool.h"
#include "core/toconf.h"
#include "ts_log/ts_log_utils.h"

#include <QtGui/QApplication>
#include <QtGui/QAction>

namespace ToConfiguration
{
QVariant Main::defaultValue(int option) const
{
	switch(option)
	{
	case LastVersion: 	return QVariant(QString("")); // TORAVERSION
	case FirstInstall: 	return QVariant(QString("")); // QDateTime::currentDateTime().toString()
	case RecentFiles: 	return QVariant(QStringList());
	case RecentMax: 	return QVariant((int)25);
	case LastDir:       return QVariant(QString(""));
	case Encoding:      return QVariant(QString("Default"));
	case DefaultTool: 	return QVariant(QString(""));
	case StatusSave: 	return QVariant((int)10);
	case DontReread: 	return QVariant((bool)true);
	case EditDragDrop: 	return QVariant((bool)false);
	case LineEnd:       return QVariant(QString("Default"));
	case ToolsMap:
	{
		// by default all the tools (having menu item name) are enabled
		QMap<QString, QVariant> retval;
	    for (ToolsRegistrySing::ObjectType::iterator i = ToolsRegistrySing::Instance().begin();
	            i != ToolsRegistrySing::Instance().end();
	            ++i)
	    {
	        toTool *pTool = i.value();
	        if (pTool && pTool->menuItem())
	        {
	        	retval[i.key()] = QVariant((bool)true);
	        }
	    }
	    return retval;
	}
	case MainWindowGeometry: return QVariant(QByteArray());
	case MainWindowState:    return QVariant(QByteArray());
	case LeftDockbarState:   return QVariant(QByteArray());
	case RightDockbarState:  return QVariant(QByteArray());
	default:
		Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Main un-registered enum value: %1").arg(option)));
		return QVariant();
	}
}

QVariant Main::toraIIValue(int option) const
{
	QSettings s(TOORGNAME, "TOra"); // "old" Tora 2.x config namespace
	QVariant retval;

	switch(option)
	{
	case LastVersion: 	s.beginGroup("preferences"); retval = s.value("LastVersion"); break;
	case FirstInstall: 	s.beginGroup("preferences"); retval = s.value("FirstInstall"); break;
	case RecentFiles: 	s.beginGroup("preferences"); retval = s.value("RecentFiles"); break;
	case RecentMax: 	s.beginGroup("preferences"); retval = s.value("RecentMax"); break;
	case LastDir:       s.beginGroup("preferences"); retval = s.value("LastDir"); break;
//	case Encoding:      return QVariant(QString("Default"));
	case DefaultTool: 	s.beginGroup("preferences"); return s.value("DefaultTool"); break;
//	case StatusSave: 	return QVariant((int)10);
//	case DontReread: 	return QVariant((bool)true);
//	case EditDragDrop: 	return QVariant((bool)false);
//	case LineEnd:       return QVariant(QString("Default"));
//	case ToolsMap:
	case MainWindowGeometry: s.beginGroup("toMainWindow"); retval = s.value("geometry"); break;
	case MainWindowState:    s.beginGroup("toMainWindow"); retval = s.value("state"); break;
	case LeftDockbarState:   s.beginGroup("toMainWindow"); retval = s.value("leftDockbar"); break;
	case RightDockbarState:  s.beginGroup("toMainWindow"); retval = s.value("rightDockbar"); break;
	default:
		s.beginGroup("preferences");
		retval = QVariant(); break;
	}
	s.endGroup();
	return retval;
}
}

toMainWindow* toMainWindow::lookup()
{
    static toMainWindow *main = NULL;
    if (main)
        return main;

    QWidgetList widgets = qApp->topLevelWidgets();
    for (QWidgetList::iterator it = widgets.begin(); it != widgets.end(); it++)
    {
        main = dynamic_cast<toMainWindow *>((*it));
        if (main) 
            return main;
    }

    return NULL;
    // TODO: throw something here
}

ToConfiguration::Main s_mainConfig;
