
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
#include <QtCore/QMetaEnum>

#ifndef ENUM_REF
#define ENUM_REF(o,e) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)))
#endif

namespace ToConfiguration {

	class ConfigContext : public QObject
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

#if 0
	class Editor : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Editor() : ConfigContext(ENUM_REF(Editor,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			SyntaxHighlight  = 3000 // #define CONF_HIGHLIGHT
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
			// 2nd tab
			, EditStyle             // #define CONF_EDIT_STYLE
		};
	};
#endif

	class Database : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Database() : ConfigContext("Database", ENUM_REF(Database,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			ObjectCache   = 4000  // #define CONF_OBJECT_CACHE
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
			, NumberDecimals      // #define CONF_NUMBER_DECIMALS
		};
	};

	class Oracle : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Oracle() : ConfigContext("Oracle", ENUM_REF(Oracle,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			ConfDateFormat = 5000 // #define CONF_DATE_FORMAT
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

	class MySQL : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		MySQL() : ConfigContext("MySQL", ENUM_REF(MySQL,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			BeforeCreateAction = 6000 // #define CONF_CREATE_ACTION
		};
	};

	class Worksheet : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Worksheet() : ConfigContext("Worksheet", ENUM_REF(Worksheet,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			AutoSave  = 7000    // #define CONF_AUTO_SAVE
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

	class Storage : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Storage() : ConfigContext("Storage", ENUM_REF(Storage,OptionTypeEnum)) {};
	    enum OptionTypeEnum {
		    DispTablespaces  = 8000  // #define CONF_DISP_TABLESPACES
		    , DispCoalesced          // #define CONF_DISP_COALESCED
		    , DispExtents            // #define CONF_DISP_EXTENTS
		    , AvailableGraph         // #define CONF_DISP_AVAILABLEGRAPH
	    };
	};

	class Browser : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Browser() : ConfigContext("Browser", ENUM_REF(Browser,OptionTypeEnum)) {};
	    enum OptionTypeEnum {
		    FilterIgnoreCase  = 9000  // #define CONF_FILTER_IGNORE_CASE
		    , FilterInvert            // #define CONF_FILTER_INVERT  
		    , FilterType              // #define CONF_FILTER_TYPE  
		    , FilterTablespaceType    // #define CONF_FILTER_TABLESPACE_TYPE  
		    , FilterText              // #define CONF_FILTER_TEXT  
	    };
	};

	class Rollback : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Rollback() : ConfigContext("Rollback", ENUM_REF(Rollback,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			NoExec       = 10000   // #define CONF_NO_EXEC
			, NeedRead             // #define CONF_NEED_READ 
			, NeedTwo              // #define CONF_NEED_TWO  
			, AlignLeft            // #define CONF_ALIGN_LEFT
			, OldEnable            // #define CONF_OLD_ENABLE
		};
	};

	class SgaTrace : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		SgaTrace() : ConfigContext("SgaTrace", ENUM_REF(SgaTrace,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			AutoUpdate   = 11000  // #define CONF_AUTO_UPDATE
		};
	};

	class Output : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Output() : ConfigContext("Output", ENUM_REF(Output,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			PollingInterval = 12000 // #define CONF_POLLING
			, Type                  // #define CONF_LOG_TYPE 
			, LogUser               // #define CONF_LOG_USER 
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
