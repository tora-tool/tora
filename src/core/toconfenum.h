
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

#include <QtCore/QObject>

namespace ToConfiguration {
	class Global : public QObject
	{
		Q_OBJECT;
	    Q_ENUMS(OptionTypeEnum);
	public:
	    enum OptionTypeEnum {
	    	// Paths
	    	SQLDictionary           // #define CONF_SQL_FILE
	    	, HelpPath              // #define CONF_HELP_PATH
	    	, DefaultSession        // #define CONF_DEFAULT_SESSION
	    	, CacheDir              // #define CONF_CACHE_DIR
	    	, ORACLE_HOME           // #define CONF_ORACLE_HOME
	    	, MYSQL_HOME            // #define CONF_MYSQL_HOME
	    	, PGSQL_HOME            // #define CONF_PGSQL_HOME
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

	class Editor : public QObject
	{
		Q_OBJECT;
	    Q_ENUMS(OptionTypeEnum);
	public:
	    enum OptionTypeEnum {
	    	SyntaxHighlight         // #define CONF_HIGHLIGHT
	    	, EditorType            // #define CONF_EDITOR_TYPE
	    	, useMaxTextWidthMark
	    	, KeywordUpper          // #define CONF_KEYWORD_UPPER
	    	, ObjectNamesUpper      // #define CONF_OBJECT_NAMES_UPPER
	    	, CodeComplete          // #define CONF_CODE_COMPLETION
	    	, CompleteSort          // #define CONF_COMPLETION_SORT
	    	, useEditorShortcuts
	    	, useEditorShortcutsMap
	    	, AutoIndent            // #define CONF_AUTO_INDENT
	    	, TabSpaces             // #define CONF_TAB_SPACES
	    	, TabStop               // #define CONF_TAB_STOP
	    	, ConfText              // #define CONF_TEXT
	    	, ConfCode              // #define CONF_CODE
	    	, ListText              // #define CONF_LIST
	    	, Extensions            // #define CONF_EXTENSIONS
	    };
	};

	class Database : public QObject
	{
		Q_OBJECT;
	    Q_ENUMS(OptionTypeEnum);
	public:
	    enum OptionTypeEnum {
	    	ObjectCache           // #define CONF_OBJECT_CACHE
	    	, AutoCommit          // #define CONF_AUTO_COMMIT
	    	, FirewallMode        // #define CONF_FIREWALL_MODE
	    	, ConnectionTestInterval // #define CONF_CONN_TEST_INTERVAL
	    	, CachedConnections
	    	, MaxNumber           // #define CONF_MAX_NUMBER (InitialFetch)
	    	, MaxContent          // #define CONF_MAX_CONTENT (InitialEditorContent)
	    	, MaxColDisp          // #define CONF_MAX_COL_DISP
	    	, IndicateEmpty       // #define CONF_INDICATE_EMPTY
	    	, IndicateEmptyColor  // #define CONF_INDICATE_EMPTY_COLOR
	    	, NumberFormat        // #define CONF_NUMBER_FORMAT
	    	, NumberDecimals      // #define CONF_NUMBER_DECIMALS "
	    };
	};

	class Oracle : public QObject
	{
		Q_OBJECT;
	    Q_ENUMS(OptionTypeEnum);
	public:
	    enum OptionTypeEnum {
	    	ConfDateFormat        // #define CONF_DATE_FORMAT
	    	, ConfTimestampFormat // #define CONF_TIMESTAMP_FORMAT
	    	, MaxLong             // #define CONF_MAX_LONG
	    	, PlanTable           // #define CONF_PLAN_TABLE
	    	, KeepPlans           // #define CONF_KEEP_PLANS  // default: false
	    	, VSqlPlans           // #define CONF_VSQL_PLANS  // default: true
	    	, SharedPlan          // #define CONF_SHARED_PLAN // default: false
	    	// Extractor. Controls the method used to produce database object extracts
	    	// as well as extraction settings (which information should be extracted).
	    	, ExtractorOptions     // #define CONF_EXT_GROUP_NAME
	    	, UseDbmsMetadata      // #define CONF_USE_DBMS_METADATA
	    	, IncludeStorage       // #define CONF_EXT_INC_STORAGE
	    	, SkipOrgMon           // #define CONF_EXT_SKIP_ORGMON
	    	, SkipStorageExceptTablespace // #define CONF_EXT_SKIP_STORAGE_EX_TABLESPACE
	    	, IncludeParallel      // #define CONF_EXT_INC_PARALLEL
	    	, IncludePartition     // #define CONF_EXT_INC_PARTITION
	    	, IncludeCode          // #define CONF_EXT_INC_CODE
	    	, IncludeHeader        // #define CONF_EXT_INC_HEADER
	    	, IncludePrompt        // #define CONF_EXT_INC_PROMPT
	    };
	};

	class MySQL : public QObject
	{
		Q_OBJECT;
	    Q_ENUMS(OptionTypeEnum);
	public:
	    enum OptionTypeEnum {
	    	BeforeCreateAction // #define CONF_CREATE_ACTION
	    };
	};

	class Worksheet : public QObject
	{
		Q_OBJECT;
	    Q_ENUMS(OptionTypeEnum);
	public:
	    enum OptionTypeEnum {
	    	AutoSave             // #define CONF_AUTO_SAVE
	    	, CheckSave          // #define CONF_CHECK_SAVE
	    	, LogAtEnd           // #define CONF_LOG_AT_END
	    	, LogMulti           // #define CONF_LOG_MULTI
	    	, Statistics         // #define CONF_STATISTICS
	    	, TimedStats         // #define CONF_TIMED_STATS
	    	, Number             // #define CONF_NUMBER (Display row number)
	    	, MoveToError        // #define CONF_MOVE_TO_ERR
	    	, History            // #define CONF_HISTORY (Save previous results)
	    	, ExecLog            // #define CONF_EXEC_LOG
	    	, ToplevelDescribe   // #define CONF_TOPLEVEL_DESCRIBE
	    	, AutoLoad           // #define CONF_AUTO_LOAD (Default file)
	    };
	};

	class Storage : public QObject
	{
		Q_OBJECT;
	    Q_ENUMS(OptionTypeEnum);
	public:
	    enum OptionTypeEnum {
#define CONF_DISP_TABLESPACES "DispTablespaces"
#define CONF_DISP_COALESCED "DispCoalesced"
#define CONF_DISP_EXTENTS "DispExtents"
#define CONF_DISP_AVAILABLEGRAPH "AvailableGraph"
	    };
	};

	class Browser : public QObject
	{
		Q_OBJECT;
	    Q_ENUMS(OptionTypeEnum);
	public:
	    enum OptionTypeEnum {
#define CONF_FILTER_IGNORE_CASE "FilterIgnoreCase"
#define CONF_FILTER_INVERT   "FilterInvert"
#define CONF_FILTER_TYPE   "FilterType"
#define CONF_FILTER_TABLESPACE_TYPE   "FilterTablespaceType"
#define CONF_FILTER_TEXT   "FilterText"
	    };
	};

	class Rollback : public QObject
	{
		Q_OBJECT;
	    Q_ENUMS(OptionTypeEnum);
	public:
	    enum OptionTypeEnum {
#define CONF_NO_EXEC    "NoExec"
#define CONF_NEED_READ  "NeedRead"
#define CONF_NEED_TWO   "NeedTwo"
#define CONF_ALIGN_LEFT "AlignLeft"
#define CONF_OLD_ENABLE "OldEnable"
	    };
	};

	class SgaTrace : public QObject
	{
		Q_OBJECT;
	    Q_ENUMS(OptionTypeEnum);
	public:
	    enum OptionTypeEnum {
#define CONF_AUTO_UPDATE    "AutoUpdate"
	    };
	};

	class Output : public QObject
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		enum OptionTypeEnum {
#define CONF_POLLING     "PollingInterval"
#define CONF_LOG_TYPE  "Type"
#define CONF_LOG_USER  "LogUser"
		};
	};

#define CONF_AUTO_INDENT_RO "AutoIndent"

#define CONF_STATUS_SAVE "StatusSave"
#define CONF_DEFAULT_TOOL "DefaultTool"
#define CONF_TOOL_ENABLE "Enabled"

#define CONF_DONT_REREAD "DontReread"

#define CONF_CSV_SEPARATOR "CSVSeparator"
#define CONF_CSV_DELIMITER "CSVDelimiter"

#define CONF_RECENT_FILES "RecentFiles"
#define CONF_RECENT_MAX  "RecentMax"
#define CONF_LAST_DIR  "LastDir"

#define CONF_CACHE_TIMEOUT "CacheTimeout"
#define CONF_EDIT_DRAG_DROP "EditDragDrop"

#define CONF_EDIT_STYLE  "EditStyle"
#define CONF_ENCODING "Encoding"
#define CONF_FORCELINEEND "ForceLineEnd"

// main.cpp
#define CONF_LAST_VERSION "LastVersion"
#define CONF_FIRST_INSTALL "FirstInstall"

// toeditextensions
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
}

#endif
