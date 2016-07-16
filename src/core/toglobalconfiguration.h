
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

#ifndef TOGLOBALSETTINGENUM_H
#define TOGLOBALSETTINGENUM_H

#include "core/toconfenum.h"

namespace ToConfiguration
{
    class Global : public ConfigContext
    {
            Q_OBJECT;
            Q_ENUMS(OptionTypeEnum);
        public:
            Global() : ConfigContext("Global", ENUM_REF(Global,OptionTypeEnum)) {};
            enum OptionTypeEnum
            {
                // Paths
                CustomSQL = 2000        // #define CONF_SQL_FILE
                , HelpDirectory         // #define CONF_HELP_PATH
                , DefaultSession        // #define CONF_DEFAULT_SESSION
                , CacheDirectory        // #define CONF_CACHE_DIR
                , OracleHomeDirectory   // #define CONF_ORACLE_HOME
                , MysqlHomeDirectory    // #define CONF_MYSQL_HOME
                , PgsqlHomeDirectory    // #define CONF_PGSQL_HOME
                , GraphvizHomeDirectory
                // Options (1st column)
                , ChangeConnectionBool  // #define CONF_CHANGE_CONNECTION
                , SavePasswordBool      // #define CONF_SAVE_PWD
                , IncludeDbCaptionBool  // #define CONF_DB_TITLE
                , RestoreSessionBool    // #define CONF_RESTORE_SESSION
                , ToadBindingsBool      // #define CONF_TOAD_BINDINGS
                , CacheDiskBool         // #define CONF_CACHE_DISK
                , DisplayGridlinesBool  // #define CONF_DISPLAY_GRIDLINES
                , MultiLineResultsBool  // #define CONF_MULTI_LINE_RESULTS
                , MessageStatusbarBool  // #define CONF_MESSAGE_STATUSBAR
                , ColorizedConnectionsBool
                , ColorizedConnectionsMap
                , UpdateStateInt        // 0 update is not running, 1 update is running, 2 TOra crashed, do not try again on startup
                , UpdateLastDate        // last date update was run
                , UpdatesCheckBool
                // Options (2nd column)
                , StatusMessageInt      // #define CONF_STATUS_MESSAGE
                , HistorySizeInt        // #define CONF_STATUS_SAVE
                , ChartSamplesInt       // #define CONF_CHART_SAMPLES
                , DisplaySamplesInt     // #define CONF_DISPLAY_SAMPLES
                , SizeUnit              // #define CONF_SIZE_UNIT
                , RefreshInterval    // #define CONF_REFRESH
                , DefaultListFormatInt  // #define CONF_DEFAULT_FORMAT // Text(0), Tab delimited(1), CSV(2), HTML(3), SQL(4)
                , Style                 // #define CONF_STYLE
                , Translation           // #define CONF_LOCALE (Translation)
                , ClipboardCHeadersBool // not displayed in the config gui (Copy format: include column headers)
                , ClipboardRHeadersBool // not displayed in the config gui (Copy format: include row headers)
#ifdef TORA_EXPERIMENTAL
                , RamThresholdInt
#endif
            };
            virtual QVariant defaultValue(int) const;

            virtual QVariant toraIIValue(int) const;
    };
};

#endif

