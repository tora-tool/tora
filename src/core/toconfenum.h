
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

#ifndef TOCONF_ENUM_H
#define TOCONF_ENUM_H

#include "core/tora_export.h"

#include <QtCore/QObject>
#include <QtCore/QMetaEnum>

#ifndef ENUM_REF
#define ENUM_REF(o,e) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)))
#endif

namespace ToConfiguration {

	class TORA_EXPORT ConfigContext : public QObject
	{
		Q_OBJECT;
	public:
		ConfigContext(QString const& context, QMetaEnum const& e);
		ConfigContext(ConfigContext const& other);
		virtual QVariant defaultValue(int) const = 0;
	private:
		QMetaEnum m_configEnum;
		QString m_contextName;
	};

	class Main : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Main() : ConfigContext("Main", ENUM_REF(Main,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			LastVersion   = 1000 // #define CONF_LAST_VERSION
			, FirstInstall       // #define CONF_FIRST_INSTALL
			, RecentFiles        // #define CONF_RECENT_FILES
			, RecentMax          // #define CONF_RECENT_MAX
			, LastDir            // #define CONF_LAST_DIR
			, Encoding           // #define CONF_ENCODING
			, DefaultTool        // #define CONF_DEFAULT_TOOL
			, CacheTimeout       // #define CONF_CACHE_TIMEOUT
			, StatusSave         // #define CONF_STATUS_SAVE (Message history size)
			, DontReread         // #define CONF_DONT_REREAD
			, EditDragDrop       // #define CONF_EDIT_DRAG_DROP (toScintilla)
			, CSVSeparator       // #define CONF_CSV_SEPARATOR
			, CSVDelimiter       // #define CONF_CSV_DELIMITER
		};
	};

	class Global : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Global() : ConfigContext("Global", ENUM_REF(Global,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			// Paths
			SQLDictionary = 2000    // #define CONF_SQL_FILE
			, HelpPath              // #define CONF_HELP_PATH
			, DefaultSession        // #define CONF_DEFAULT_SESSION
			, CacheDir              // #define CONF_CACHE_DIR
			, ORACLE_HOME           // #define CONF_ORACLE_HOME
			, MYSQL_HOME            // #define CONF_MYSQL_HOME
			, PGSQL_HOME            // #define CONF_PGSQL_HOME
			, GV_HOME            // #define CONF_PGSQL_HOME
			// Options (1st column)
			, ChangeConnection      // #define CONF_CHANGE_CONNECTION
			, SavePassword          // #define CONF_SAVE_PWD
			, DBTitle               // #define CONF_DB_TITLE
			, RestoreSession        // #define CONF_RESTORE_SESSION
			, ToadBindings          // #define CONF_TOAD_BINDINGS
			, CacheDiskUse          // #define CONF_CACHE_DISK
			, Gridlines             // #define CONF_DISPLAY_GRIDLINES
			, MultiLineResults      // #define CONF_MULTI_LINE_RESULTS
			, MessageStatusbar      // #define CONF_MESSAGE_STATUSBAR
			, ColorizedConnections
			, ColorizedConnectionsMap
			// Options (2nd column)
			, StatusMessage         // #define CONF_STATUS_MESSAGE
			, ConnectHistory        // #define CONF_CONNECT_HISTORY
			, ChartSamples          // #define CONF_CHART_SAMPLES
			, SizeUnit              // #define CONF_SIZE_UNIT
			, RefreshInterval       // #define CONF_REFRESH
			, DefaultFormat         // #define CONF_DEFAULT_FORMAT
			, Style                 // #define CONF_STYLE
			, Locale                // #define CONF_LOCALE (Translation)
		};
	};

#if TORA_OBSOLETE
// toeditextensions
#define CONF_AUTO_INDENT_RO "AutoIndent"
#define CONF_COMMA_BEFORE "CommaBefore"
#define CONF_BLOCK_OPEN_LINE "BlockOpenLine"
#define CONF_OPERATOR_SPACE "OperatorSpace"
#define CONF_RIGHT_SEPARATOR "RightSeparator"
#define CONF_END_BLOCK_NEWLINE "EndBlockNewline"
#define CONF_COMMENT_COLUMN "CommentColumn"
// totuning
#define CONF_OVERVIEW "Overview"
#define CONF_FILEIO   "File I/O"
#define CONF_WAITS    "Wait events"
#define CONF_CHART    "Charts"
#endif 

}

#endif
