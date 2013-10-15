/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#include "core/toconfigurationpriv.h"
#include "core/toconf.h"

#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtCore/QLocale>
#include <QtGui/QApplication>

// A little magic to get lrefresh to work and get a check on qApp
#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

toConfigurationPrivate::toConfigurationPrivate(QObject *parent)
	: QObject(parent)
{
	// Note we must save copy of this before we change into some other directory
	// Note: m_applicationDir will be empty if QCoreApplication was not instantiated yet
	m_applicationDir.setPath("");
	loadConfig();
}

/*! \brief Get directory name where to store session files.
      It's APPDATA on Windows, HOME on UNIX like systems
 */
QString toConfigurationPrivate::getSpecialDir()
{
	return QDir::homePath() + "/";
}

void toConfigurationPrivate::loadConfig()
{
	int cnt; // tmp count holder for setting arrays
	QSettings s;
	s.beginGroup("preferences");
	// Global
	//  Paths
	m_customSQL = s.value(CONF_SQL_FILE, getSpecialDir() + DEFAULT_SQL_FILE).toString();
	m_helpDirectory = s.value(CONF_HELP_PATH, "qrc:/help/toc.html").toString();
	m_defaultSession = s.value(CONF_DEFAULT_SESSION, getSpecialDir() + DEFAULT_SESSION).toString();
	m_cacheDir = s.value(CONF_CACHE_DIR, "").toString();
	m_oracleHome = s.value(CONF_ORACLE_HOME, "").toString();
	m_mysqlHome = s.value(CONF_MYSQL_HOME, "").toString();
	m_pgsqlHome = s.value(CONF_PGSQL_HOME, "").toString();
	//  Options (1st column)
	m_changeConnection = s.value(CONF_CHANGE_CONNECTION, true).toBool();
	m_savePassword = s.value(CONF_SAVE_PWD, DEFAULT_SAVE_PWD).toBool();
	m_dbTitle = s.value(CONF_DB_TITLE, true).toBool();
	m_restoreSession = s.value(CONF_RESTORE_SESSION, false).toBool();
	m_toadBindings = s.value(CONF_TOAD_BINDINGS, DEFAULT_TOAD_BINDINGS).toBool();
	m_cacheDisk = s.value(CONF_CACHE_DISK, DEFAULT_CACHE_DISK).toBool();
	m_displayGridlines = s.value(CONF_DISPLAY_GRIDLINES, DEFAULT_DISPLAY_GRIDLINES).toBool();
	m_multiLineResults = s.value(CONF_MULTI_LINE_RESULTS, DEFAULT_MULTI_LINE_RESULTS).toBool();
	m_messageStatusbar = s.value(CONF_MESSAGE_STATUSBAR, false).toBool();

	m_colorizedConnections = s.value("ColorizedConnections", true).toBool();
	cnt = s.beginReadArray("ConnectionColors");
	for (int i = 0; i < cnt; ++i)
	{
		s.setArrayIndex(i);
		m_connectionColors[s.value("key").toString()] = s.value("value").toString();
	}
	s.endArray();
	if (m_connectionColors.empty())
	{
		m_connectionColors["#FF0000"] = "Production";
		m_connectionColors["#00FF00"] = "Development";
		m_connectionColors["#0000FF"] = "Testing";
	}
	//  Options (2nd column)
	m_cachedConnections = s.value("CachedConnections", 4).toInt();
	m_statusMessage = s.value(CONF_STATUS_MESSAGE, DEFAULT_STATUS_MESSAGE).toInt();
	m_historySize = s.value(CONF_STATUS_SAVE, DEFAULT_STATUS_SAVE).toInt();
	m_chartSamples = s.value(CONF_CHART_SAMPLES, DEFAULT_CHART_SAMPLES).toInt();
	m_displaySamples = s.value(CONF_DISPLAY_SAMPLES, DEFAULT_DISPLAY_SAMPLES).toInt();
	m_sizeUnit = s.value(CONF_SIZE_UNIT, DEFAULT_SIZE_UNIT).toString();
	m_refresh = s.value(CONF_REFRESH, DEFAULT_REFRESH).toString();
	m_defaultFormat = s.value(CONF_DEFAULT_FORMAT, "").toInt();
	m_style = s.value(CONF_STYLE, DEFAULT_STYLE).toString();
	m_translation = s.value(CONF_LOCALE, QLocale().name()).toString();

	m_initialFetch = s.value(CONF_MAX_NUMBER, DEFAULT_MAX_NUMBER).toInt();
	m_maxColDisp = s.value(CONF_MAX_COL_DISP, DEFAULT_MAX_COL_DISP).toInt();
	m_planTable = s.value(CONF_PLAN_TABLE, DEFAULT_PLAN_TABLE).toString();
	m_textFontName = s.value(CONF_TEXT, "").toString();
	m_codeFontName = s.value(CONF_CODE, "").toString();
	m_listFontName = s.value(CONF_LIST, "").toString();
	m_dateFormat = s.value(CONF_DATE_FORMAT, DEFAULT_DATE_FORMAT).toString();
	m_timestampFormat = s.value(CONF_TIMESTAMP_FORMAT, DEFAULT_TIMESTAMP_FORMAT).toString();

	m_syntaxHighlighting = s.value(CONF_HIGHLIGHT, true).toBool();
	m_keywordUpper = s.value(CONF_KEYWORD_UPPER, DEFAULT_KEYWORD_UPPER).toBool();
	m_objectNamesUpper = s.value(CONF_OBJECT_NAMES_UPPER, DEFAULT_OBJECT_NAMES_UPPER).toBool();

	m_autoCommit = s.value(CONF_AUTO_COMMIT, false).toBool();


	// #define CONF_CONNECT_CURRENT "ConnectCurrent"
	m_defaultTool = s.value(CONF_DEFAULT_TOOL).toString();
	// #define CONF_TOOL_ENABLE "Enabled"
	m_codeCompletion = s.value(CONF_CODE_COMPLETION, true).toBool();
	m_completionSort = s.value(CONF_COMPLETION_SORT, true).toBool();
	m_autoIndent = s.value(CONF_AUTO_INDENT, true).toBool();
	m_dontReread = s.value(CONF_DONT_REREAD, true).toBool();

	m_objectCache = (toConfiguration::ObjectCacheEnum)s.value(CONF_OBJECT_CACHE, DEFAULT_OBJECT_CACHE).toInt();
	m_firewallMode = s.value(CONF_FIREWALL_MODE, false).toBool();
	m_connTestInterval = s.value(CONF_CONN_TEST_INTERVAL, DEFAULT_CONN_TEST_INTERVAL).toInt();
	m_initialEditorContent = s.value(CONF_MAX_CONTENT, DEFAULT_MAX_CONTENT).toInt();
	m_keepPlans = s.value(CONF_KEEP_PLANS, false).toBool();
	m_vsqlPlans = s.value(CONF_VSQL_PLANS, true).toBool();
	m_sharedPlan = s.value(CONF_SHARED_PLAN, false).toBool();

	// FIXME!
	m_autoIndentRo = s.value(CONF_AUTO_INDENT_RO, true).toBool();
	m_indicateEmpty = s.value(CONF_INDICATE_EMPTY, true).toBool();
	m_indicateEmptyColor = s.value(CONF_INDICATE_EMPTY_COLOR, "#f2ffbc").toString();
	m_extensions = s.value(CONF_EXTENSIONS, DEFAULT_EXTENSIONS).toString();
	m_recentFiles = s.value(CONF_RECENT_FILES, QStringList()).toStringList();
	m_recentMax = s.value(CONF_RECENT_MAX, DEFAULT_RECENT_MAX).toInt();
	m_lastDir = s.value(CONF_LAST_DIR, "").toString();
	m_numberFormat = s.value(CONF_NUMBER_FORMAT, DEFAULT_NUMBER_FORMAT).toInt();
	m_numberDecimals = s.value(CONF_NUMBER_DECIMALS, DEFAULT_NUMBER_DECIMALS).toInt();
	m_cacheTimeout = s.value(CONF_CACHE_TIMEOUT, DEFAULT_CACHE_TIMEOUT).toInt();
	m_tabStop = s.value(CONF_TAB_STOP, DEFAULT_TAB_STOP).toInt();
	m_useSpacesForIndent = s.value(CONF_TAB_SPACES, DEFAULT_TAB_SPACES).toBool();
	m_editDragDrop = s.value(CONF_EDIT_DRAG_DROP, DEFAULT_EDIT_DRAG_DROP).toBool();
	// tooracleconnection.cpp

	m_maxLong = s.value(CONF_MAX_LONG, DEFAULT_MAX_LONG).toInt();
	// toqsqlconnection
	m_onlyForward = s.value(CONF_ONLY_FORWARD, DEFAULT_ONLY_FORWARD).toBool();
	m_createAction = s.value(CONF_CREATE_ACTION, DEFAULT_CREATE_ACTION).toInt();
	// main.cpp
	m_lastVersion = s.value(CONF_LAST_VERSION, "").toString();
	m_firstInstall = s.value(CONF_FIRST_INSTALL, "").toString();
	// tools
	cnt = s.beginReadArray(CONF_TOOLS);
	for (int i = 0; i < cnt; ++i)
	{
		s.setArrayIndex(i);
		m_tools[s.value("ToolName").toString()] = s.value("Enabled").toBool();
	}
	s.endArray();
	// toresultlistformat
	m_csvSeparator = s.value(CONF_CSV_SEPARATOR, DEFAULT_CSV_SEPARATOR).toString();
	m_csvDelimiter = s.value(CONF_CSV_DELIMITER, DEFAULT_CSV_DELIMITER).toString();
	// toeditextensions
	m_commaBefore = s.value(CONF_COMMA_BEFORE, true).toBool();
	m_blockOpenLine = s.value(CONF_BLOCK_OPEN_LINE, true).toBool();
	m_operatorSpace = s.value(CONF_OPERATOR_SPACE, true).toBool();
	m_rightSeparator = s.value(CONF_RIGHT_SEPARATOR, true).toBool();
	m_endBlockNewline = s.value(CONF_END_BLOCK_NEWLINE, true).toBool();
	m_commentColumn = s.value(CONF_COMMENT_COLUMN, DEFAULT_COMMENT_COLUMN).toInt();
	// tobrowser
	m_filterIgnoreCase = s.value(CONF_FILTER_IGNORE_CASE, false).toBool();
	m_filterInvert = s.value(CONF_FILTER_INVERT, false).toBool();
	m_filterType = s.value(CONF_FILTER_TYPE, 0).toInt();
	m_filterTablespaceType = s.value(CONF_FILTER_TABLESPACE_TYPE, 0).toInt();
	m_filterText = s.value(CONF_FILTER_TEXT, "").toString();
	// tohelp
	cnt = s.beginReadArray("AdditionalHelp");
	for (int i = 0; i < cnt; ++i)
	{
		s.setArrayIndex(i);
		m_additionalHelp[s.value("HelpName").toString()] = s.value("Path").toString();
	}
	s.endArray();

#ifdef TORA3_CHARTS
	// tochartmanager
	cnt = s.beginReadArray("ChartFiles");
	for (int i = 0; i < cnt; ++i)
	{
		s.setArrayIndex(i);
		m_chartFiles[s.value("Name").toString()] = s.value("Spec").toString();
	}
	s.endArray();
	cnt = s.beginReadArray("ChartAlarms");
	for (int i = 0; i < cnt; ++i)
	{
		s.setArrayIndex(i);
		m_chartAlarms[s.value("Name").toString()] = s.value("Spec").toString();
	}
	s.endArray();
#endif

	// tooutput
	m_polling = s.value(CONF_POLLING, DEFAULT_POLLING).toString();
	m_logType = s.value(CONF_LOG_TYPE, DEFAULT_LOG_TYPE).toInt();
	m_logUser = s.value(CONF_LOG_USER, DEFAULT_LOG_USER).toString();
	// torollback
	m_noExec = s.value(CONF_NO_EXEC, true).toBool();
	m_needRead = s.value(CONF_NEED_READ, true).toBool();
	m_needTwo = s.value(CONF_NEED_TWO, true).toBool();
	m_alignLeft = s.value(CONF_ALIGN_LEFT, true).toBool();
	m_oldEnable = s.value(CONF_OLD_ENABLE, false).toBool();
	// tosgatrace
	m_autoUpdate = s.value(CONF_AUTO_UPDATE, true).toBool();
	// tostorage
	m_dispTablespaces = s.value(CONF_DISP_TABLESPACES, true).toBool();
	m_dispCoalesced = s.value(CONF_DISP_COALESCED, false).toBool();
	m_dispExtents = s.value(CONF_DISP_EXTENTS, false).toBool();
	m_dispAvailableGraph = s.value(CONF_DISP_AVAILABLEGRAPH, true).toBool();
	// totemplate
	cnt = s.beginReadArray("Templates");
	for (int i = 0; i < cnt; ++i)
	{
		s.setArrayIndex(i);
		m_templates[s.value("Name").toString()] = s.value("File").toString();
	}
	s.endArray();
	// totuning
	m_tuningFirstRun = (s.value(CONF_OVERVIEW) == QVariant());
	m_tuningOverview = s.value(CONF_OVERVIEW, false).toBool();
	m_tuningFileIO = s.value(CONF_FILEIO, false).toBool();
	m_tuningWaits = s.value(CONF_WAITS, false).toBool();
	m_tuningCharts = s.value(CONF_CHART, false).toBool();
	// toworksheet
	m_wsAutoSave = s.value(CONF_AUTO_SAVE, false).toBool();
	m_wsCheckSave = s.value(CONF_CHECK_SAVE, true).toBool();
	m_wsAutoLoad = s.value(CONF_AUTO_LOAD, "").toString();
	m_wsLogAtEnd = s.value(CONF_LOG_AT_END, true).toBool();
	m_wsLogMulti = s.value(CONF_LOG_MULTI, true).toBool();
	m_wsStatistics = s.value(CONF_STATISTICS, false).toBool();
	m_wsTimedStats = s.value(CONF_TIMED_STATS, true).toBool();
	m_wsNumber = s.value(CONF_NUMBER, true).toBool();
	m_wsMoveToErr = s.value(CONF_MOVE_TO_ERR, true).toBool();
	m_wsHistory = s.value(CONF_HISTORY, false).toBool();
	m_wsExecLog = s.value(CONF_EXEC_LOG, false).toBool();
	m_wsToplevelDescribe = s.value(CONF_TOPLEVEL_DESCRIBE, true).toBool();
	// tosyntaxsetup
	const QPalette cg = qApp->palette();
	m_syntaxDefault = s.value("SyntaxDefault", cg.color(QPalette::Base).name()).toString();
	m_syntaxComment = s.value("SyntaxComment", "darkgray").toString();
	m_syntaxNumber = s.value("SyntaxNumber", "limegreen").toString();
	m_syntaxKeyword = s.value("SyntaxKeyword", "navy").toString();
	m_syntaxString = s.value("SyntaxString", "red").toString();
	m_syntaxDefaultBg = s.value("SyntaxDefaultBg", cg.color(QPalette::Text).name()).toString();
	m_syntaxErrorBg = s.value("SyntaxErrorBg", "darkred").toString();
	m_syntaxDebugBg = s.value("SyntaxDebugBg", "darkgreen").toString();
	m_syntaxCurrentLineMarker = s.value("SyntaxCurrentLineMarker", "whitesmoke").toString();
	m_syntaxStaticBg = s.value("SyntaxStaticBg", "darkblue").toString();
	m_useMaxTextWidthMark = s.value("useMaxTextWidthMark", false).toBool();
	m_maxTextWidthMark = s.value("maxTextWidthMark", 75).toInt();
	m_encoding = s.value(CONF_ENCODING, DEFAULT_ENCODING).toString();
	m_forcelineend = s.value(CONF_FORCELINEEND, DEFAULT_FORCELINEEND).toString();
	s.endGroup();

	// main window
	s.beginGroup("toMainWindow");
	m_mainWindowGeometry = s.value("geometry").toByteArray();
	// mainwindow's toolbars etc
	m_mainWindowState = s.value("state").toByteArray();
	m_leftDockbarState = s.value("leftDockbar").toByteArray();
	m_rightDockbarState = s.value("rightDockbar").toByteArray();
	s.endGroup();

	// editor shortcuts
	s.beginGroup("editorShortcuts");
	m_useEditorShortcuts = s.value("useEditorShortcuts", false).toBool();
	cnt = s.beginReadArray("shortcuts");
	for (int i = 0; i < cnt; ++i)
	{
		s.setArrayIndex(i);
		m_editorShortcuts[s.value("key").toString()] = s.value("value");
	}
	s.endArray();
	s.endGroup();

	s.beginGroup(CONF_EXT_GROUP_NAME);
	m_extractorUseDbmsMetadata = s.value(CONF_USE_DBMS_METADATA, DEFAULT_USE_DBMS_METADATA).toBool();
	m_extractorIncludeSotrage = s.value(CONF_EXT_INC_STORAGE, DEFAULT_EXT_INC_STORAGE).toBool();
	m_extractorSkipOrgMonInformation = s.value(CONF_EXT_SKIP_ORGMON, DEFAULT_EXT_SKIP_ORGMON).toBool();
	m_extractorSkipStorageExceptTablespaces = s.value(CONF_EXT_SKIP_STORAGE_EX_TABLESPACE, DEFAULT_EXT_SKIP_STORAGE_EX_TABLESPACE).toBool();
	m_extractorIncludeParallel = s.value(CONF_EXT_INC_PARALLEL, DEFAULT_EXT_INC_PARALLEL).toBool();
	m_extractorIncludePartition = s.value(CONF_EXT_INC_PARTITION, DEFAULT_EXT_INC_PARTITION).toBool();
	m_extractorIncludeCode = s.value(CONF_EXT_INC_CODE, DEFAULT_EXT_INC_CODE).toBool();
	m_extractorIncludeHeader = s.value(CONF_EXT_INC_HEADER, DEFAULT_EXT_INC_HEADER).toBool();
	m_extractorIncludePrompt = s.value(CONF_EXT_INC_PROMPT, DEFAULT_EXT_INC_PROMPT).toBool();
	s.endGroup(); // end extractor group
}


void toConfigurationPrivate::saveConfig()
{
	QString key; // tmp key for maps
	QSettings s;
	s.beginGroup("preferences");
	// Global setting
	//  Paths
	s.setValue(CONF_SQL_FILE, m_customSQL);
	s.setValue(CONF_HELP_PATH, m_helpDirectory);
	s.setValue(CONF_DEFAULT_SESSION, m_defaultSession);
	s.setValue(CONF_CACHE_DIR, m_cacheDir);
	s.setValue(CONF_ORACLE_HOME, m_oracleHome);
	s.setValue(CONF_MYSQL_HOME, m_mysqlHome);
	s.setValue(CONF_PGSQL_HOME, m_pgsqlHome);
	//  Options (1st column)
	s.setValue(CONF_CHANGE_CONNECTION, m_changeConnection);
	s.setValue(CONF_SAVE_PWD, m_savePassword);
	s.setValue(CONF_DB_TITLE, m_dbTitle);
	s.setValue(CONF_RESTORE_SESSION, m_restoreSession);
	s.setValue(CONF_TOAD_BINDINGS, m_toadBindings);
	s.setValue(CONF_CACHE_DISK, m_cacheDisk);
	s.setValue(CONF_DISPLAY_GRIDLINES, m_displayGridlines);
	s.setValue(CONF_MULTI_LINE_RESULTS, m_multiLineResults);
	s.setValue(CONF_MESSAGE_STATUSBAR, m_messageStatusbar);
	s.setValue("ColorizedConnections", m_colorizedConnections);
	s.beginWriteArray("ConnectionColors");
	for (int i = 0; i < m_connectionColors.count(); ++i)
	{
		s.setArrayIndex(i);
		key = m_connectionColors.keys().at(i);
		s.setValue("key", key);
		s.setValue("value", m_connectionColors[key]);
	}
	s.endArray();
	//  Options (2nd column)
	s.setValue("CachedConnections", m_cachedConnections);
	s.setValue(CONF_STATUS_MESSAGE, m_statusMessage);
	s.setValue(CONF_STATUS_SAVE, m_historySize);
	s.setValue(CONF_CHART_SAMPLES, m_chartSamples);
	s.setValue(CONF_DISPLAY_SAMPLES, m_displaySamples);
	s.setValue(CONF_SIZE_UNIT, m_sizeUnit);
	s.setValue(CONF_REFRESH, m_refresh);
	s.setValue(CONF_DEFAULT_FORMAT, m_defaultFormat);
	s.setValue(CONF_STYLE, m_style);
	s.setValue(CONF_LOCALE, m_translation);

	// Editor setting
	s.setValue(CONF_HIGHLIGHT, m_syntaxHighlighting);

	s.setValue(CONF_TEXT, m_textFontName);
	s.setValue(CONF_CODE, m_codeFontName);
	s.setValue(CONF_LIST, m_listFontName);

	s.setValue(CONF_KEYWORD_UPPER, m_keywordUpper);
	s.setValue(CONF_OBJECT_NAMES_UPPER, m_objectNamesUpper);
	s.setValue(CONF_CODE_COMPLETION, m_codeCompletion);
	s.setValue(CONF_COMPLETION_SORT, m_completionSort);
	s.setValue(CONF_AUTO_INDENT, m_autoIndent);
	s.setValue("useMaxTextWidthMark", m_useMaxTextWidthMark);
	s.setValue("maxTextWidthMark", m_maxTextWidthMark);
	s.setValue(CONF_EXTENSIONS, m_extensions);
	s.setValue(CONF_ENCODING, m_encoding);
	s.setValue(CONF_FORCELINEEND, m_forcelineend);
	s.setValue(CONF_TAB_STOP, m_tabStop);
	s.setValue(CONF_TAB_SPACES, m_useSpacesForIndent);

	// Database settings
	s.setValue(CONF_OBJECT_CACHE, m_objectCache);
	s.setValue(CONF_AUTO_COMMIT, m_autoCommit);
	s.setValue(CONF_FIREWALL_MODE, m_firewallMode);
	s.setValue(CONF_CONN_TEST_INTERVAL, m_connTestInterval);
	s.setValue(CONF_MAX_NUMBER, m_initialFetch);
	s.setValue(CONF_MAX_CONTENT, m_initialEditorContent);
	s.setValue(CONF_MAX_COL_DISP, m_maxColDisp);
	s.setValue(CONF_PLAN_TABLE, m_planTable);
	//  Oracle
	s.setValue(CONF_KEEP_PLANS, m_keepPlans);
	s.setValue(CONF_VSQL_PLANS, m_vsqlPlans);
	s.setValue(CONF_SHARED_PLAN, m_sharedPlan);
	s.setValue(CONF_DATE_FORMAT, m_dateFormat);
	s.setValue(CONF_TIMESTAMP_FORMAT, m_timestampFormat);


	s.setValue(CONF_DEFAULT_TOOL, m_defaultTool);
	s.setValue(CONF_DONT_REREAD, m_dontReread);

	s.setValue(CONF_AUTO_INDENT_RO, m_autoIndentRo);
	s.setValue(CONF_INDICATE_EMPTY, m_indicateEmpty);
	s.setValue(CONF_INDICATE_EMPTY_COLOR, m_indicateEmptyColor);

	s.setValue(CONF_RECENT_FILES, m_recentFiles);
	s.setValue(CONF_RECENT_MAX, m_recentMax);
	s.setValue(CONF_LAST_DIR, m_lastDir);

	s.setValue(CONF_NUMBER_FORMAT, m_numberFormat);
	s.setValue(CONF_NUMBER_DECIMALS, m_numberDecimals);
	s.setValue(CONF_CACHE_TIMEOUT, m_cacheTimeout);
	s.setValue(CONF_EDIT_DRAG_DROP, m_editDragDrop);

	// tooracleconnection
	s.setValue(CONF_MAX_LONG, m_maxLong);

	// toqsqlconnection
	s.setValue(CONF_ONLY_FORWARD, m_onlyForward);
	s.setValue(CONF_CREATE_ACTION, m_createAction);
	// main.cpp
	s.setValue(CONF_LAST_VERSION, m_lastVersion);
	s.setValue(CONF_FIRST_INSTALL, m_firstInstall);

	// tools
	s.beginWriteArray(CONF_TOOLS);
	for (int i = 0; i < m_tools.count(); ++i)
	{
		s.setArrayIndex(i);
		key = m_tools.keys().at(i);
		s.setValue("ToolName", key);
		s.setValue("Enabled", m_tools[key]);
	}
	s.endArray();

	// toresultlistformat
	s.setValue(CONF_CSV_SEPARATOR, m_csvSeparator);
	s.setValue(CONF_CSV_DELIMITER, m_csvDelimiter);
	// toeditextensions
	s.setValue(CONF_COMMA_BEFORE, m_commaBefore);
	s.setValue(CONF_BLOCK_OPEN_LINE, m_blockOpenLine);
	s.setValue(CONF_OPERATOR_SPACE, m_operatorSpace);
	s.setValue(CONF_RIGHT_SEPARATOR, m_rightSeparator);
	s.setValue(CONF_END_BLOCK_NEWLINE, m_endBlockNewline);
	s.setValue(CONF_COMMENT_COLUMN, m_commentColumn);
	// tobrowser
	s.setValue(CONF_FILTER_IGNORE_CASE, m_filterIgnoreCase);
	s.setValue(CONF_FILTER_INVERT, m_filterInvert);
	s.setValue(CONF_FILTER_TYPE, m_filterType);
	s.setValue(CONF_FILTER_TABLESPACE_TYPE, m_filterTablespaceType);
	s.setValue(CONF_FILTER_TEXT, m_filterText);
	// tohelp
	s.beginWriteArray("AdditionalHelp");
	for (int i = 0; i < m_additionalHelp.count(); ++i)
	{
		s.setArrayIndex(i);
		key = m_additionalHelp.keys().at(i);
		s.setValue("HelpName", key);
		s.setValue("Path", m_additionalHelp[key]);
	}
	s.endArray();

#ifdef TORA3_CHARTS
	// tochartmanager
	s.beginWriteArray("ChartFiles");
	for (int i = 0; i < m_chartFiles.count(); ++i)
	{
		s.setArrayIndex(i);
		key = m_chartFiles.keys().at(i);
		s.setValue("Name", key);
		s.setValue("Spec", m_chartFiles[key]);
	}
	s.endArray();
	s.beginWriteArray("ChartAlarms");
	for (int i = 0; i < m_chartAlarms.count(); ++i)
	{
		s.setArrayIndex(i);
		key = m_chartAlarms.keys().at(i);
		s.setValue("Name", key);
		s.setValue("Spec", m_chartAlarms[key]);
	}
	s.endArray();
#endif

	// tooutput
	s.setValue(CONF_POLLING, m_polling);
	s.setValue(CONF_LOG_TYPE, m_logType);
	s.setValue(CONF_LOG_USER, m_logUser);
	// torollback
	s.setValue(CONF_NO_EXEC, m_noExec);
	s.setValue(CONF_NEED_READ, m_needRead);
	s.setValue(CONF_NEED_TWO, m_needTwo);
	s.setValue(CONF_ALIGN_LEFT, m_alignLeft);
	s.setValue(CONF_OLD_ENABLE, m_oldEnable);
	// tosgatrace
	s.setValue(CONF_AUTO_UPDATE, m_autoUpdate);
	// tostorage
	s.setValue(CONF_DISP_TABLESPACES, m_dispTablespaces);
	s.setValue(CONF_DISP_COALESCED, m_dispCoalesced);
	s.setValue(CONF_DISP_EXTENTS, m_dispExtents);
	s.setValue(CONF_DISP_AVAILABLEGRAPH, m_dispAvailableGraph);
	// totemplate
	s.beginWriteArray("Templates");
	for (int i = 0; i < m_templates.count(); ++i)
	{
		s.setArrayIndex(i);
		key = m_templates.keys().at(i);
		s.setValue("Name", key);
		s.setValue("File", m_templates[key]);
	}
	s.endArray();
	// totuning
	s.setValue(CONF_OVERVIEW, m_tuningOverview);
	s.setValue(CONF_FILEIO, m_tuningFileIO);
	s.setValue(CONF_WAITS, m_tuningWaits);
	s.setValue(CONF_CHART, m_tuningCharts);
	// toworksheet
	s.setValue(CONF_AUTO_SAVE, m_wsAutoSave);
	s.setValue(CONF_CHECK_SAVE, m_wsCheckSave);
	s.setValue(CONF_AUTO_LOAD, m_wsAutoLoad);
	s.setValue(CONF_LOG_AT_END, m_wsLogAtEnd);
	s.setValue(CONF_LOG_MULTI, m_wsLogMulti);
	s.setValue(CONF_STATISTICS, m_wsStatistics);
	s.setValue(CONF_TIMED_STATS, m_wsTimedStats);
	s.setValue(CONF_NUMBER, m_wsNumber);
	s.setValue(CONF_MOVE_TO_ERR, m_wsMoveToErr);
	s.setValue(CONF_HISTORY, m_wsHistory);
	s.setValue(CONF_EXEC_LOG, m_wsExecLog);
	s.setValue(CONF_TOPLEVEL_DESCRIBE, m_wsToplevelDescribe);
	// tosyntaxsetup
	s.setValue("SyntaxDefault", m_syntaxDefault);
	s.setValue("SyntaxComment", m_syntaxComment);
	s.setValue("SyntaxNumber", m_syntaxNumber);
	s.setValue("SyntaxKeyword", m_syntaxKeyword);
	s.setValue("SyntaxString", m_syntaxString);
	s.setValue("SyntaxDefaultBg", m_syntaxDefaultBg);
	s.setValue("SyntaxErrorBg", m_syntaxErrorBg);
	s.setValue("SyntaxDebugBg", m_syntaxDebugBg);
	s.setValue("SyntaxCurrentLineMarker", m_syntaxCurrentLineMarker);
	s.setValue("SyntaxStaticBg", m_syntaxStaticBg);
	s.endGroup();

	// main window
	s.beginGroup("toMainWindow");
	s.setValue("geometry", m_mainWindowGeometry);
	// mainwindow's toolbars etc
	s.setValue("state", m_mainWindowState);
	s.setValue("leftDockbar", m_leftDockbarState);
	s.setValue("rightDockbar", m_rightDockbarState);
	s.endGroup();

	// editor shortcuts
	s.beginGroup("editorShortcuts");
	s.setValue("useEditorShortcuts", m_useEditorShortcuts);
	s.beginWriteArray("shortcuts");
	for (int i = 0; i < m_editorShortcuts.count(); ++i)
	{
		s.setArrayIndex(i);
		key = m_editorShortcuts.keys().at(i);
		s.setValue("key", key);
		s.setValue("value", m_editorShortcuts[key]);
	}
	s.endArray();
	s.endGroup();

	s.beginGroup(CONF_EXT_GROUP_NAME);
	s.setValue(CONF_USE_DBMS_METADATA, m_extractorUseDbmsMetadata);
	s.setValue(CONF_EXT_INC_STORAGE, m_extractorIncludeSotrage);
	s.setValue(CONF_EXT_SKIP_ORGMON, m_extractorSkipOrgMonInformation);
	s.setValue(CONF_EXT_SKIP_STORAGE_EX_TABLESPACE, m_extractorSkipStorageExceptTablespaces);
	s.setValue(CONF_EXT_INC_PARALLEL, m_extractorIncludeParallel);
	s.setValue(CONF_EXT_INC_PARTITION, m_extractorIncludePartition);
	s.setValue(CONF_EXT_INC_CODE, m_extractorIncludeCode);
	s.setValue(CONF_EXT_INC_HEADER, m_extractorIncludeHeader);
	s.setValue(CONF_EXT_INC_PROMPT, m_extractorIncludePrompt);
	s.endGroup(); // end extractor group
}


void toConfigurationPrivate::loadMap(const QString &filename, std::map<QString, QString> &pairs)
{
	QByteArray data = Utils::toReadFile(filename).toUtf8();

	int pos = 0;
	int bol = 0;
	int endtag = -1;
	int wpos = 0;
	int size = data.length();
	while (pos < size)
	{
		switch (data[pos])
		{
		case '\n':
		data[wpos] = 0;
		if (endtag == -1)
			throw QT_TRANSLATE_NOOP("toTool", "Malformed tag in config file. Missing = on row. (%1)").arg(QString(data.mid(bol, wpos - bol)));
		{
			QString tag = ((const char *)data) + bol;
			QString val = ((const char *)data) + endtag + 1;
			pairs[tag] = val;
		}
		bol = pos + 1;
		endtag = -1;
		wpos = pos;
		break;
		case '=':
			if (endtag == -1)
			{
				endtag = pos;
				data[wpos] = 0;
				wpos = pos;
			}
			else
				data[wpos] = data[pos];
			break;
		case '\\':
			pos++;
			switch (data[pos])
			{
			case 'n':
				data[wpos] = '\n';
				break;
			case '\\':
				if (endtag >= 0)
					data[wpos] = '\\';
				else
					data[wpos] = ':';
				break;
			default:
				throw QT_TRANSLATE_NOOP("toTool", "Unknown escape character in string (Only \\\\ and \\n recognised)");
			}
			break;
			default:
				data[wpos] = data[pos];
				break;
		}
		wpos++;
		pos++;
	}

	return;
}


bool toConfigurationPrivate::saveMap(const QString &file, std::map<QString, QString> &pairs)
{
	QString data;

	{
		// qt4        QRegExp newline(QString::fromLatin1("\n"));
		// qt4        QRegExp backslash(QString::fromLatin1("\\"));
		QString newline("\n");
		QString backslash("\\");
		for (std::map<QString, QString>::iterator i = pairs.begin(); i != pairs.end(); i++)
		{
			QString str = (*i).first;
			str.append(QString::fromLatin1("="));
			str.replace(backslash, QString::fromLatin1("\\\\"));
			str.replace(newline, QString::fromLatin1("\\n"));
			QString line = (*i).second;
			line.replace(backslash, QString::fromLatin1("\\\\"));
			line.replace(newline, QString::fromLatin1("\\n"));
			str += line.toUtf8();
			str += QString::fromLatin1("\n");
			data += str;
		}
	}
	return Utils::toWriteFile(file, data);
}
