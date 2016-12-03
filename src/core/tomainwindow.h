
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

#ifndef TOMAINWINDOW_H
#define TOMAINWINDOW_H

#include "core/tora_export.h"
#include "core/toconfenum.h"
#include "loki/Singleton.h"

#include <QMainWindow>

class toDockbar;
class toDocklet;

namespace ToConfiguration
{
    class Main : public ConfigContext
    {
            Q_OBJECT;
            Q_ENUMS(OptionTypeEnum);
        public:
            Main() : ConfigContext("Main", ENUM_REF(Main,OptionTypeEnum)) {};
            enum OptionTypeEnum
            {
                LastVersion   = 1000 // #define CONF_LAST_VERSION
                , FirstInstall       // #define CONF_FIRST_INSTALL
                , RecentFiles        // #define CONF_RECENT_FILES
                , RecentMax          // #define CONF_RECENT_MAX
                , LastDir            // #define CONF_LAST_DIR
                , Encoding           // #define CONF_ENCODING
                , DefaultTool        // #define CONF_DEFAULT_TOOL
                , StatusSave         // #define CONF_STATUS_SAVE (Message history size)
                , DontReread         // #define CONF_DONT_REREAD
                , EditDragDrop       // #define CONF_EDIT_DRAG_DROP (toScintilla)
                , LineEnd
                , ToolsMap
                , MainWindowGeometry
                , MainWindowState
                , LeftDockbarState
                , RightDockbarState
                , ConnectionModelSortColumnInt
            };
            virtual QVariant defaultValue(int option) const;

            virtual QVariant toraIIValue(int) const;
    };
};

class TORA_EXPORT toMainWindow : public QMainWindow
{
        Q_OBJECT;
    public:

        /**
         * Return the toDockbar that manages the docklet. If none
         * currently manages the docklet, returns what will if the docklet
         * is shown.
         */
        virtual toDockbar* dockbar(toDocklet *let) = 0;

        /** Get a pointer to the main window
         * @return Pointer to main window.
         */
        static toMainWindow* lookup(void);
    private:
        static ToConfiguration::Main s_mainConfig;
};

#endif
