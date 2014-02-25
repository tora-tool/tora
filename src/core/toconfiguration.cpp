
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

#include "core/toconfiguration.h"
#include "core/toconfigurationpriv.h"
#include "core/utils.h"

#include "toconf.h"

#include <QtCore/QObject>
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtCore/QDir>

toConfiguration::toConfiguration(QObject *parent)
	: QObject(parent)
	, d_ptr(NULL)
{	
    setQSettingsEnv();
    d_ptr = new toConfigurationPrivate(this);
}

toConfiguration::~toConfiguration()
{
	Q_D(toConfiguration);
	delete d;
}

// static one
void toConfiguration::setQSettingsEnv()
{
    if (QCoreApplication::organizationName().isEmpty())
        QCoreApplication::setOrganizationName(TOORGNAME);
    if (QCoreApplication::organizationDomain().isEmpty())
        QCoreApplication::setOrganizationDomain(TOORGDOMAIN);
    if (QCoreApplication::applicationName().isEmpty())
        QCoreApplication::setApplicationName(TOAPPNAME);
}

//QString toConfiguration::customSQL()
//{
//	Q_D(toConfiguration);
//    return d->m_customSQL;
//}
void toConfiguration::setCustomSQL(const QString & v)
{
	Q_D(toConfiguration);
    d->m_customSQL = v;
}

//QString toConfiguration::helpDirectory()
//{
//	Q_D(toConfiguration);
//    return d->m_helpDirectory;
//}
void toConfiguration::setHelpDirectory(const QString & v)
{
	Q_D(toConfiguration);
    d->m_helpDirectory = v;
}

//QString toConfiguration::defaultSession()
//{
//	Q_D(toConfiguration);
//    return d->m_defaultSession;
//}
void toConfiguration::setDefaultSession(const QString & v)
{
	Q_D(toConfiguration);
    d->m_defaultSession = v;
}



void toConfiguration::saveConfig()
{
	Q_D(toConfiguration);
    d->saveConfig();
}

void toConfiguration::loadMap(const QString &filename, std::map<QString, QString> &pairs)
{
	Q_D(toConfiguration);
    d->loadMap(filename, pairs);
}

bool toConfiguration::saveMap(const QString &file, std::map<QString, QString> &pairs)
{
	Q_D(toConfiguration);
    return d->saveMap(file, pairs);
}

bool toConfiguration::savePassword()
{
	Q_D(toConfiguration);
    return d->m_savePassword;
}
void toConfiguration::setSavePassword(bool v)
{
	Q_D(toConfiguration);
    d->m_savePassword = v;
}

int toConfiguration::cachedConnections()
{
	Q_D(toConfiguration);
    return d->m_cachedConnections;
}
void toConfiguration::setCachedConnections(int v)
{
	Q_D(toConfiguration);
    d->m_cachedConnections = v;
}

int toConfiguration::initialFetch()
{
	Q_D(toConfiguration);
    return d->m_initialFetch;
}
void toConfiguration::setInitialFetch(int v)
{
	Q_D(toConfiguration);
    d->m_initialFetch = v;
}

int toConfiguration::maxColDisp()
{
	Q_D(toConfiguration);
    return d->m_maxColDisp;
}
void toConfiguration::setMaxColDisp(int v)
{
	Q_D(toConfiguration);
    d->m_maxColDisp = v;
}

QString toConfiguration::planTable(QString schema)
{
	Q_D(toConfiguration);
    if(d->m_sharedPlan || d->m_planTable.contains('.') || schema.isNull())
        return d->m_planTable;

    return schema + '.' + d->m_planTable;
}
void toConfiguration::setPlanTable(const QString & v)
{
	Q_D(toConfiguration);
    d->m_planTable = v;
}

//QString toConfiguration::textFontName()
//{
//	Q_D(toConfiguration);
//    return d->m_textFontName;
//}
void toConfiguration::setTextFontName(const QString & v)
{
	Q_D(toConfiguration);
    d->m_textFontName = v;
}

QString toConfiguration::codeFontName()
{
	Q_D(toConfiguration);
    return d->m_codeFontName;
}
void toConfiguration::setCodeFontName(const QString & v)
{
	Q_D(toConfiguration);
    d->m_codeFontName = v;
}

QString toConfiguration::listFontName()
{
	Q_D(toConfiguration);
    return d->m_listFontName;
}
void toConfiguration::setListFontName(const QString & v)
{
	Q_D(toConfiguration);
    d->m_listFontName = v;
}

//QString toConfiguration::oracleHome()
//{
//	Q_D(toConfiguration);
//	return d->m_oracleHome;
//}

void toConfiguration::setOracleHome(QString const &v)
{
	Q_D(toConfiguration);
	d->m_oracleHome = v;
}






QString toConfiguration::dateFormat()
{
	Q_D(toConfiguration);
    return d->m_dateFormat;
}
void toConfiguration::setDateFormat(const QString & v)
{
	Q_D(toConfiguration);
    d->m_dateFormat = v;
}

QString toConfiguration::timestampFormat()
{
	Q_D(toConfiguration);
    return d->m_timestampFormat;
}
void toConfiguration::setTimestampFormat(const QString & v)
{
	Q_D(toConfiguration);
    d->m_timestampFormat = v;
}

QString toConfiguration::refresh()
{
	Q_D(toConfiguration);
    return d->m_refresh;
}
void toConfiguration::setRefresh(const QString & v)
{
	Q_D(toConfiguration);
    d->m_refresh = v;
}

int toConfiguration::syntaxHighlighting()
{
	Q_D(toConfiguration);
    return d->m_syntaxHighlighting;
}
void toConfiguration::setSyntaxHighlighting(int v)
{
	Q_D(toConfiguration);
    d->m_syntaxHighlighting = v;
}

//int toConfiguration::editorType()
//{
//	Q_D(toConfiguration);
//    return d->m_editorType;
//}
void toConfiguration::setEditorType(int v)
{
	Q_D(toConfiguration);
    d->m_editorType = v;
}

//bool toConfiguration::keywordUpper()
//{
//	Q_D(toConfiguration);
//    return d->m_keywordUpper;
//}
void toConfiguration::setKeywordUpper(bool v)
{
	Q_D(toConfiguration);
    d->m_keywordUpper = v;
}

//bool toConfiguration::objectNamesUpper()
//{
//	Q_D(toConfiguration);
//    return d->m_objectNamesUpper;
//}
void toConfiguration::setObjectNamesUpper(bool v)
{
	Q_D(toConfiguration);
    d->m_objectNamesUpper = v;
}

QString toConfiguration::cacheDir()
{
	Q_D(toConfiguration);
    return d->m_cacheDir;
}
void toConfiguration::setCacheDir(const QString & v)
{
	Q_D(toConfiguration);
    d->m_cacheDir = v;
}

bool toConfiguration::cacheDisk()
{
	Q_D(toConfiguration);
    return d->m_cacheDisk;
}
void toConfiguration::setCacheDisk(bool v)
{
	Q_D(toConfiguration);
    d->m_cacheDisk = v;
}

int toConfiguration::statusMessage()
{
	Q_D(toConfiguration);
    return d->m_statusMessage;
}
void toConfiguration::setStatusMessage(int v)
{
	Q_D(toConfiguration);
    d->m_statusMessage = v;
}

bool toConfiguration::includeDbCaption()
{
	Q_D(toConfiguration);
    return d->m_dbTitle;
}
void toConfiguration::setIncludeDbCaption(bool v)
{
	Q_D(toConfiguration);
    d->m_dbTitle = v;
}

QString toConfiguration::sizeUnit()
{
	Q_D(toConfiguration);
    return d->m_sizeUnit;
}
void toConfiguration::setSizeUnit(const QString & v)
{
	Q_D(toConfiguration);
    d->m_sizeUnit = v;
}

int toConfiguration::historySize()
{
	Q_D(toConfiguration);
    return d->m_historySize;
}
void toConfiguration::setHistorySize(int v)
{
	Q_D(toConfiguration);
    d->m_historySize = v;
}

bool toConfiguration::autoCommit()
{
	Q_D(toConfiguration);
    return d->m_autoCommit;
}
void toConfiguration::setAutoCommit(bool v)
{
	Q_D(toConfiguration);
    d->m_autoCommit = v;
}

bool toConfiguration::changeConnection()
{
	Q_D(toConfiguration);
    return d->m_changeConnection;
}
void toConfiguration::setChangeConnection(bool v)
{
	Q_D(toConfiguration);
    d->m_changeConnection = v;
}

QString toConfiguration::defaultTool()
{
	Q_D(toConfiguration);
    return d->m_defaultTool;
}
void toConfiguration::setDefaultTool(const QString & v)
{
	Q_D(toConfiguration);
    d->m_defaultTool = v;
}

int toConfiguration::chartSamples()
{
	Q_D(toConfiguration);
    return d->m_chartSamples;
}
void toConfiguration::setChartSamples(int v)
{
	Q_D(toConfiguration);
    d->m_chartSamples = v;
}

int toConfiguration::displaySamples()
{
	Q_D(toConfiguration);
    return d->m_displaySamples;
}
void toConfiguration::setDisplaySamples(int v)
{
	Q_D(toConfiguration);
    d->m_displaySamples = v;
}

bool toConfiguration::codeCompletion()
{
	Q_D(toConfiguration);
    return d->m_codeCompletion;
}
void toConfiguration::setCodeCompletion(bool v)
{
	Q_D(toConfiguration);
    d->m_codeCompletion = v;
}

bool toConfiguration::completionSort()
{
	Q_D(toConfiguration);
    return d->m_completionSort;
}
void toConfiguration::setCompletionSort(bool v)
{
	Q_D(toConfiguration);
    d->m_completionSort = v;
}

//bool toConfiguration::autoIndent()
//{
//	Q_D(toConfiguration);
//    return d->m_autoIndent;
//}
void toConfiguration::setAutoIndent(bool v)
{
	Q_D(toConfiguration);
    d->m_autoIndent = v;
}

bool toConfiguration::dontReread()
{
	Q_D(toConfiguration);
    return d->m_dontReread;
}
void toConfiguration::setDontReread(bool v)
{
	Q_D(toConfiguration);
    d->m_dontReread = v;
}

bool toConfiguration::messageStatusbar()
{
	Q_D(toConfiguration);
    return d->m_messageStatusbar;
}
void toConfiguration::setMessageStatusbar(bool v)
{
	Q_D(toConfiguration);
    d->m_messageStatusbar = v;
}

bool toConfiguration::multiLineResults()
{
	Q_D(toConfiguration);
    return d->m_multiLineResults;
}
void toConfiguration::setMultiLineResults(bool v)
{
	Q_D(toConfiguration);
    d->m_multiLineResults = v;
}

bool toConfiguration::colorizedConnections()
{
	Q_D(toConfiguration);
    return d->m_colorizedConnections;
}
void toConfiguration::setColorizedConnections(bool v)
{
	Q_D(toConfiguration);
    d->m_colorizedConnections = v;
}
ConnectionColors toConfiguration::connectionColors()
{
	Q_D(toConfiguration);
    return d->m_connectionColors;
}
void toConfiguration::setConnectionColors(const ConnectionColors & v)
{
	Q_D(toConfiguration);
    d->m_connectionColors = v;
}

toConfiguration::ObjectCacheEnum toConfiguration::objectCache()
{
	Q_D(toConfiguration);
    return d->m_objectCache;
}
void toConfiguration::setObjectCache(toConfiguration::ObjectCacheEnum v)
{
	Q_D(toConfiguration);
    d->m_objectCache = v;
}

//bool toConfiguration::firewallMode()
//{
//	Q_D(toConfiguration);
//    return d->m_firewallMode;
//}
void toConfiguration::setFirewallMode(bool v)
{
	Q_D(toConfiguration);
    d->m_firewallMode = v;
}

//int toConfiguration::connTestInterval()
//{
//	Q_D(toConfiguration);
//    return d->m_connTestInterval;
//}
void toConfiguration::setConnTestInterval(int v)
{
	Q_D(toConfiguration);
    d->m_connTestInterval = v;
}

int toConfiguration::initialEditorContent()
{
	Q_D(toConfiguration);
    return d->m_initialEditorContent;
}
void toConfiguration::setInitialEditorContent(int v)
{
	Q_D(toConfiguration);
    d->m_initialEditorContent = v;
}

bool toConfiguration::keepPlans()
{
	Q_D(toConfiguration);
    return d->m_keepPlans;
}
void toConfiguration::setKeepPlans(bool v)
{
	Q_D(toConfiguration);
    d->m_keepPlans = v;
}

bool toConfiguration::vsqlPlans()
{
	Q_D(toConfiguration);
    return d->m_vsqlPlans;
}
void toConfiguration::setVsqlPlans(bool v)
{
	Q_D(toConfiguration);
    d->m_vsqlPlans = v;
}

bool toConfiguration::sharedPlan()
{
	Q_D(toConfiguration);
    return d->m_sharedPlan;
}
void toConfiguration::setSharedPlan(bool v)
{
	Q_D(toConfiguration);
    d->m_sharedPlan = v;
}

//bool toConfiguration::restoreSession()
//{
//	Q_D(toConfiguration);
//    return d->m_restoreSession;
//}
void toConfiguration::setRestoreSession(bool v)
{
	Q_D(toConfiguration);
    d->m_restoreSession = v;
}


int toConfiguration::defaultFormat()
{
	Q_D(toConfiguration);
    return d->m_defaultFormat;
}
void toConfiguration::setDefaultFormat(int v)
{
	Q_D(toConfiguration);
    d->m_defaultFormat = v;
}

bool toConfiguration::autoIndentRo()
{
	Q_D(toConfiguration);
    return d->m_autoIndentRo;
}
void toConfiguration::setAutoIndentRo(bool v)
{
	Q_D(toConfiguration);
    d->m_autoIndentRo = v;
}

bool toConfiguration::indicateEmpty()
{
	Q_D(toConfiguration);
    return d->m_indicateEmpty;
}
void toConfiguration::setIndicateEmpty(bool v)
{
	Q_D(toConfiguration);
    d->m_indicateEmpty = v;
}

QString toConfiguration::indicateEmptyColor()
{
	Q_D(toConfiguration);
    return d->m_indicateEmptyColor;
}
void toConfiguration::setIndicateEmptyColor(const QString & v)
{
	Q_D(toConfiguration);
    d->m_indicateEmptyColor = v;
}

QString toConfiguration::extensions()
{
	Q_D(toConfiguration);
    return d->m_extensions;
}
void toConfiguration::setExtensions(const QString & v)
{
	Q_D(toConfiguration);
    d->m_extensions = v;
}

int toConfiguration::recentMax()
{
	Q_D(toConfiguration);
    return d->m_recentMax;
}

QString toConfiguration::lastDir()
{
	Q_D(toConfiguration);
    return d->m_lastDir;
}
void toConfiguration::setLastDir(const QString & v)
{
	Q_D(toConfiguration);
    d->m_lastDir = v;
}

QString toConfiguration::translation()
{
	Q_D(toConfiguration);
    return d->m_translation;
}
void toConfiguration::setTranslation(const QString & v)
{
	Q_D(toConfiguration);
    d->m_translation = v;
}

int toConfiguration::numberFormat()
{
	Q_D(toConfiguration);
    return d->m_numberFormat;
}
void toConfiguration::setNumberFormat(int v)
{
	Q_D(toConfiguration);
    d->m_numberFormat = v;
}

int toConfiguration::numberDecimals()
{
	Q_D(toConfiguration);
    return d->m_numberDecimals;
}
void toConfiguration::setNumberDecimals(int v)
{
	Q_D(toConfiguration);
    d->m_numberDecimals = v;
}

int toConfiguration::cacheTimeout()
{
	Q_D(toConfiguration);
    return d->m_cacheTimeout;
}

//int toConfiguration::tabStop()
//{
//	Q_D(toConfiguration);
//    return d->m_tabStop;
//}
void toConfiguration::setTabStop(int v)
{
	Q_D(toConfiguration);
    d->m_tabStop = v;
}

bool toConfiguration::useSpacesForIndent()
{
	Q_D(toConfiguration);
    return d->m_useSpacesForIndent;
}
void toConfiguration::setUseSpacesForIndent(bool v)
{
	Q_D(toConfiguration);
    d->m_useSpacesForIndent = v;
}

//bool toConfiguration::editDragDrop()
//{
//	Q_D(toConfiguration);
//    return d->m_editDragDrop;
//}
void toConfiguration::setEditDragDrop(bool v)
{
	Q_D(toConfiguration);
    d->m_editDragDrop = v;
}

//bool toConfiguration::toadBindings()
//{
//	Q_D(toConfiguration);
//    return d->m_toadBindings;
//}
void toConfiguration::setToadBindings(bool v)
{
	Q_D(toConfiguration);
    d->m_toadBindings = v;
}

bool toConfiguration::displayGridlines()
{
	Q_D(toConfiguration);
    return d->m_displayGridlines;
}
void toConfiguration::setDisplayGridlines(bool v)
{
	Q_D(toConfiguration);
    d->m_displayGridlines = v;
}

//QString toConfiguration::style()
//{
//	Q_D(toConfiguration);
//    return d->m_style;
//}
void toConfiguration::setStyle(const QString & v)
{
	Q_D(toConfiguration);
    d->m_style = v;
}

int toConfiguration::maxLong()
{
	Q_D(toConfiguration);
    return d->m_maxLong;
}
void toConfiguration::setMaxLong(int v)
{
	Q_D(toConfiguration);
    d->m_maxLong = v;
}

// toqsqlconnection
//QString toConfiguration::mysqlHome()
//{
//	Q_D(toConfiguration);
//	return d->m_mysqlHome;
//}

void toConfiguration::setMysqlHome(QString const &v)
{
	Q_D(toConfiguration);
	d->m_mysqlHome = v;
}

//QString toConfiguration::pgsqlHome()
//{
//	Q_D(toConfiguration);
//	return d->m_pgsqlHome;
//}

void toConfiguration::setPgsqlHome(QString const &v)
{
	Q_D(toConfiguration);
	d->m_pgsqlHome = v;
}

//QString  toConfiguration::graphvizHome()
//{
//	Q_D(toConfiguration);
//	return d->m_graphvizHome;
//}

void toConfiguration::setGraphvizHome(QString const&v)
{
	Q_D(toConfiguration);
	d->m_graphvizHome = v;
}

int toConfiguration::beforeCreateAction()
{
	Q_D(toConfiguration);
    return d->m_beforeCreateAction;
}

void toConfiguration::setBeforeCreateAction(int v)
{
	Q_D(toConfiguration);
    d->m_beforeCreateAction = v;
}

// main.cpp
QString toConfiguration::lastVersion()
{
	Q_D(toConfiguration);
    return d->m_lastVersion;
}
void toConfiguration::setLastVersion(const QString & v)
{
	Q_D(toConfiguration);
    d->m_lastVersion = v;
}

QString toConfiguration::firstInstall()
{
	Q_D(toConfiguration);
    return d->m_firstInstall;
}
void toConfiguration::setFirstInstall(const QString & v)
{
	Q_D(toConfiguration);
    d->m_firstInstall = v;
}
// tomain.cpp
QStringList toConfiguration::recentFiles()
{
	Q_D(toConfiguration);
    return d->m_recentFiles;
}
void toConfiguration::setRecentFiles(QStringList v)
{
	Q_D(toConfiguration);
    d->m_recentFiles = v;
}
// tools
ToolsMap& toConfiguration::tools()
{
	Q_D(toConfiguration);
    return d->m_tools;
}
void toConfiguration::setTools(ToolsMap &v)
{
	Q_D(toConfiguration);
    d->m_tools = v;
}

// toresultlistformat
QString toConfiguration::csvSeparator()
{
	Q_D(toConfiguration);
    return d->m_csvSeparator;
}
QString toConfiguration::csvDelimiter()
{
	Q_D(toConfiguration);
    return d->m_csvDelimiter;
}

// toeditextensions
bool toConfiguration::commaBefore()
{
	Q_D(toConfiguration);
    return d->m_commaBefore;
}
void toConfiguration::setCommaBefore(bool v)
{
	Q_D(toConfiguration);
    d->m_commaBefore = v;
}

bool toConfiguration::blockOpenLine()
{
	Q_D(toConfiguration);
    return d->m_blockOpenLine;
}
void toConfiguration::setBlockOpenLine(bool v)
{
	Q_D(toConfiguration);
    d->m_blockOpenLine = v;
}

bool toConfiguration::operatorSpace()
{
	Q_D(toConfiguration);
    return d->m_operatorSpace;
}
void toConfiguration::setOperatorSpace(bool v)
{
	Q_D(toConfiguration);
    d->m_operatorSpace = v;
}

bool toConfiguration::rightSeparator()
{
	Q_D(toConfiguration);
    return d->m_rightSeparator;
}
void toConfiguration::setRightSeparator(bool v)
{
	Q_D(toConfiguration);
    d->m_rightSeparator = v;
}

bool toConfiguration::endBlockNewline()
{
	Q_D(toConfiguration);
    return d->m_endBlockNewline;
}
void toConfiguration::setEndBlockNewline(bool v)
{
	Q_D(toConfiguration);
    d->m_endBlockNewline = v;
}

int toConfiguration::commentColumn()
{
	Q_D(toConfiguration);
    return d->m_commentColumn;
}
void toConfiguration::setCommentColumn(int v)
{
	Q_D(toConfiguration);
    d->m_commentColumn = v;
}

// tobrowser
//bool toConfiguration::filterIgnoreCase()
//{
//	Q_D(toConfiguration);
//    return d->m_filterIgnoreCase;
//}
void toConfiguration::setFilterIgnoreCase(bool v)
{
	Q_D(toConfiguration);
    d->m_filterIgnoreCase = v;
}

//bool toConfiguration::filterInvert()
//{
//	Q_D(toConfiguration);
//    return d->m_filterInvert;
//}
void toConfiguration::setFilterInvert(bool v)
{
	Q_D(toConfiguration);
    d->m_filterInvert = v;
}

//int toConfiguration::filterType()
//{
//	Q_D(toConfiguration);
//    return d->m_filterType;
//}
void toConfiguration::setFilterType(int v)
{
	Q_D(toConfiguration);
    d->m_filterType = v;
}

//int toConfiguration::filterTablespaceType()
//{
//	Q_D(toConfiguration);
//    return d->m_filterTablespaceType;
//}
void toConfiguration::setFilterTablespaceType(int v)
{
	Q_D(toConfiguration);
    d->m_filterTablespaceType = v;
}

//QString toConfiguration::filterText()
//{
//	Q_D(toConfiguration);
//    return d->m_filterText;
//}
void toConfiguration::setFilterText(const QString & v)
{
	Q_D(toConfiguration);
    d->m_filterText = v;
}

// tohelp
HelpsMap toConfiguration::additionalHelp()
{
	Q_D(toConfiguration);
    return d->m_additionalHelp;
}
void toConfiguration::setAdditionalHelp(HelpsMap v)
{
	Q_D(toConfiguration);
    d->m_additionalHelp = v;
}

#ifdef TORA3_CHARTS
// tochartmanager
ChartsMap toConfiguration::chartFiles()
{
	Q_D(toConfiguration);
    return d->m_chartFiles;
}
void toConfiguration::setChartFiles(ChartsMap v)
{
	Q_D(toConfiguration);
    d->m_chartFiles = v;
}

ChartsMap toConfiguration::chartAlarms()
{
	Q_D(toConfiguration);
    return d->m_chartAlarms;
}
void toConfiguration::setChartAlarms(ChartsMap v)
{
	Q_D(toConfiguration);
    d->m_chartAlarms = v;
}
#endif

// toooutput
QString toConfiguration::polling()
{
	Q_D(toConfiguration);
    return d->m_polling;
}
void toConfiguration::setPolling(const QString & v )
{
	Q_D(toConfiguration);
    d->m_polling = v;
}

int toConfiguration::logType()
{
	Q_D(toConfiguration);
    return d->m_logType;
}
void toConfiguration::setLogType(int v)
{
	Q_D(toConfiguration);
    d->m_logType = v;
}

QString toConfiguration::logUser()
{
	Q_D(toConfiguration);
    return d->m_logUser;
}
void toConfiguration::setLogUser(const QString & v)
{
	Q_D(toConfiguration);
    d->m_logUser = v;
}

// torollback
bool toConfiguration::noExec()
{
	Q_D(toConfiguration);
    return d->m_noExec;
}
void toConfiguration::setNoExec(bool v)
{
	Q_D(toConfiguration);
    d->m_noExec = v;
}

bool toConfiguration::needRead()
{
	Q_D(toConfiguration);
    return d->m_needRead;
}
void toConfiguration::setNeedRead(bool v)
{
	Q_D(toConfiguration);
    d->m_needRead = v;
}

bool toConfiguration::needTwo()
{
	Q_D(toConfiguration);
    return d->m_needTwo;
}
void toConfiguration::setNeedTwo(bool v)
{
	Q_D(toConfiguration);
    d->m_needTwo = v;
}

bool toConfiguration::alignLeft()
{
	Q_D(toConfiguration);
    return d->m_alignLeft;
}
void toConfiguration::setAlignLeft(bool v)
{
	Q_D(toConfiguration);
    d->m_alignLeft = v;
}

bool toConfiguration::oldEnable()
{
	Q_D(toConfiguration);
    return d->m_oldEnable;
}
void toConfiguration::setOldEnable(bool v)
{
	Q_D(toConfiguration);
    d->m_oldEnable = v;
}

// tosgatrace
bool toConfiguration::autoUpdate()
{
	Q_D(toConfiguration);
    return d->m_autoUpdate;
}
void toConfiguration::setAutoUpdate(bool v)
{
	Q_D(toConfiguration);
    d->m_autoUpdate = v;
}

// tostorage
bool toConfiguration::dispTablespaces()
{
	Q_D(toConfiguration);
    return d->m_dispTablespaces;
}
void toConfiguration::setDispTablespaces(bool v)
{
	Q_D(toConfiguration);
    d->m_dispTablespaces = v;
}

bool toConfiguration::dispCoalesced()
{
	Q_D(toConfiguration);
    return d->m_dispCoalesced;
}
void toConfiguration::setDispCoalesced(bool v)
{
	Q_D(toConfiguration);
    d->m_dispCoalesced = v;
}

bool toConfiguration::dispExtents()
{
	Q_D(toConfiguration);
    return d->m_dispExtents;
}
void toConfiguration::setDispExtents(bool v)
{
	Q_D(toConfiguration);
    d->m_dispExtents = v;
}

bool toConfiguration::dispAvailableGraph()
{
	Q_D(toConfiguration);
    return d->m_dispAvailableGraph;
}
void toConfiguration::setDispAvailableGraph(bool v)
{
	Q_D(toConfiguration);
    d->m_dispAvailableGraph = v;
}

// totemplate
TemplatesMap toConfiguration::templates()
{
	Q_D(toConfiguration);
    return d->m_templates;
}
void toConfiguration::setTemplates(TemplatesMap v)
{
	Q_D(toConfiguration);
    d->m_templates = v;
}

// totuning
/*! True if there are no prefs for tuning.
  It's used when there is no d->m_tuningOverview defined in prefs. */
bool toConfiguration::tuningFirstRun()
{
	Q_D(toConfiguration);
    return d->m_tuningFirstRun;
}

bool toConfiguration::tuningOverview()
{
	Q_D(toConfiguration);
    return d->m_tuningOverview;
}
void toConfiguration::setTuningOverview(bool v)
{
	Q_D(toConfiguration);
    d->m_tuningOverview = v;
}

bool toConfiguration::tuningFileIO()
{
	Q_D(toConfiguration);
    return d->m_tuningFileIO;
}
void toConfiguration::setTuningFileIO(bool v)
{
	Q_D(toConfiguration);
    d->m_tuningFileIO = v;
}

bool toConfiguration::tuningWaits()
{
	Q_D(toConfiguration);
    return d->m_tuningWaits;
}
void toConfiguration::setTuningWaits(bool v)
{
	Q_D(toConfiguration);
    d->m_tuningWaits = v;
}

bool toConfiguration::tuningCharts()
{
	Q_D(toConfiguration);
    return d->m_tuningCharts;
}
void toConfiguration::setTuningCharts(bool v)
{
	Q_D(toConfiguration);
    d->m_tuningCharts = v;
}

// toworksheet
//bool toConfiguration::wsAutoSave()
//{
//	Q_D(toConfiguration);
//    return d->m_wsAutoSave;
//}
void toConfiguration::setWsAutoSave(bool v)
{
	Q_D(toConfiguration);
    d->m_wsAutoSave = v;
}

//bool toConfiguration::wsCheckSave()
//{
//	Q_D(toConfiguration);
//    return d->m_wsCheckSave;
//}
void toConfiguration::setWsCheckSave(bool v)
{
	Q_D(toConfiguration);
    d->m_wsCheckSave = v;
}

//QString toConfiguration::wsAutoLoad()
//{
//	Q_D(toConfiguration);
//    return d->m_wsAutoLoad;
//}
void toConfiguration::setWsAutoLoad(const QString & v)
{
	Q_D(toConfiguration);
    d->m_wsAutoLoad = v;
}

//bool toConfiguration::wsLogAtEnd()
//{
//	Q_D(toConfiguration);
//    return d->m_wsLogAtEnd;
//}
void toConfiguration::setWsLogAtEnd(bool v)
{
	Q_D(toConfiguration);
    d->m_wsLogAtEnd = v;
}

//bool toConfiguration::wsLogMulti()
//{
//	Q_D(toConfiguration);
//    return d->m_wsLogMulti;
//}
void toConfiguration::setWsLogMulti(bool v)
{
	Q_D(toConfiguration);
    d->m_wsLogMulti = v;
}

//bool toConfiguration::wsStatistics()
//{
//	Q_D(toConfiguration);
//    return d->m_wsStatistics;
//}
void toConfiguration::setWsStatistics(bool v)
{
	Q_D(toConfiguration);
    d->m_wsStatistics = v;
}

//bool toConfiguration::wsTimedStats()
//{
//	Q_D(toConfiguration);
//    return d->m_wsTimedStats;
//}
void toConfiguration::setWsTimedStats(bool v)
{
	Q_D(toConfiguration);
    d->m_wsTimedStats = v;
}

//bool toConfiguration::wsNumber()
//{
//	Q_D(toConfiguration);
//    return d->m_wsNumber;
//}
void toConfiguration::setWsNumber(bool v)
{
	Q_D(toConfiguration);
    d->m_wsNumber = v;
}

//bool toConfiguration::wsMoveToErr()
//{
//	Q_D(toConfiguration);
//    return d->m_wsMoveToErr;
//}
void toConfiguration::setWsMoveToErr(bool v)
{
	Q_D(toConfiguration);
    d->m_wsMoveToErr = v;
}

//bool toConfiguration::wsHistory()
//{
//	Q_D(toConfiguration);
//    return d->m_wsHistory;
//}
void toConfiguration::setWsHistory(bool v)
{
	Q_D(toConfiguration);
    d->m_wsHistory = v;
}

//bool toConfiguration::wsExecLog()
//{
//	Q_D(toConfiguration);
//    return d->m_wsExecLog;
//}
void toConfiguration::setWsExecLog(bool v)
{
	Q_D(toConfiguration);
    d->m_wsExecLog = v;
}

//bool toConfiguration::wsToplevelDescribe()
//{
//	Q_D(toConfiguration);
//    return d->m_wsToplevelDescribe;
//}
void toConfiguration::setWsToplevelDescribe(bool v)
{
	Q_D(toConfiguration);
    d->m_wsToplevelDescribe = v;
}


//bool toConfiguration::useEditorShortcuts()
//{
//	Q_D(toConfiguration);
//    return d->m_useEditorShortcuts;
//}

void toConfiguration::setUseEditorShortcuts(bool v)
{
	Q_D(toConfiguration);
    d->m_useEditorShortcuts = v;
}

EditorShortcutsMap toConfiguration::editorShortcuts()
{
	Q_D(toConfiguration);
    return d->m_editorShortcuts;
}

void toConfiguration::setEditorShortcuts(const EditorShortcutsMap & v)
{
	Q_D(toConfiguration);
    d->m_editorShortcuts = v;
}

QColor toConfiguration::styleFgColor(int style)
{
	Q_D(toConfiguration);
    return d->m_styles[style].FGColor;
}

QColor toConfiguration::styleBgColor(int style)
{
	Q_D(toConfiguration);
    return d->m_styles[style].BGColor;
}

QFont  toConfiguration::styleFont(int style)
{
	Q_D(toConfiguration);
    return d->m_styles[style].Font;
}

const toStylesMap& toConfiguration::styles()
{
	Q_D(toConfiguration);
    return d->m_styles;
}

void toConfiguration::setStyles(const toStylesMap & styles)
{
	Q_D(toConfiguration);
    d->m_styles = styles;
}

bool toConfiguration::useMaxTextWidthMark()
{
	Q_D(toConfiguration);
    return d->m_useMaxTextWidthMark;
}

void toConfiguration::setUseMaxTextWidthMark(bool v)
{
	Q_D(toConfiguration);
    d->m_useMaxTextWidthMark = v;
}

int toConfiguration::maxTextWidthMark()
{
	Q_D(toConfiguration);
    return d->m_maxTextWidthMark;
}

void toConfiguration::setMaxTextWidthMark(int v)
{
	Q_D(toConfiguration);
    d->m_maxTextWidthMark = v;
}

QByteArray toConfiguration::mainWindowGeometry()
{
	Q_D(toConfiguration);
    return d->m_mainWindowGeometry;
}

void toConfiguration::setMainWindowGeometry(QByteArray v)
{
	Q_D(toConfiguration);
    d->m_mainWindowGeometry = v;
}

QByteArray toConfiguration::mainWindowState()
{
	Q_D(toConfiguration);
    return d->m_mainWindowState;
}

void toConfiguration::setMainWindowState(QByteArray v)
{
	Q_D(toConfiguration);
    d->m_mainWindowState = v;
}


void toConfiguration::setLeftDockbarState(QByteArray v)
{
	Q_D(toConfiguration);
    d->m_leftDockbarState = v;
}

QByteArray toConfiguration::leftDockbarState()
{
	Q_D(toConfiguration);
    return d->m_leftDockbarState;
}

void toConfiguration::setRightDockbarState(QByteArray v)
{
	Q_D(toConfiguration);
    d->m_rightDockbarState = v;
}

QByteArray toConfiguration::rightDockbarState()
{
	Q_D(toConfiguration);
    return d->m_rightDockbarState;
}

// Default encoding used when reading/writing files
QString toConfiguration::encoding()
{
	Q_D(toConfiguration);
    return d->m_encoding;
}
void toConfiguration::setEncoding(const QString & v)
{
	Q_D(toConfiguration);
    d->m_encoding = v;
}

// Force using a particular line end type (win, linux, mac) when saving text files
QString toConfiguration::forceLineEnd()
{
	Q_D(toConfiguration);
    return d->m_forcelineend;
}
void toConfiguration::setForceLineEnd(const QString & v)
{
	Q_D(toConfiguration);
    d->m_forcelineend = v;
}

//bool toConfiguration::extractorUseDbmsMetadata()
//{
//	Q_D(toConfiguration);
//    return d->m_extractorUseDbmsMetadata;
//}
void toConfiguration::setExtractorUseDbmsMetadata(bool v)
{
	Q_D(toConfiguration);
    d->m_extractorUseDbmsMetadata = v;
}

//bool toConfiguration::extractorIncludeSotrage()
//{
//	Q_D(toConfiguration);
//    return d->m_extractorIncludeSotrage;
//}
void toConfiguration::setExtractorIncludeSotrage(bool v)
{
	Q_D(toConfiguration);
    d->m_extractorIncludeSotrage = v;
}

//bool toConfiguration::extractorSkipOrgMonInformation()
//{
//	Q_D(toConfiguration);
//    return d->m_extractorSkipOrgMonInformation;
//}
void toConfiguration::setExtractorSkipOrgMonInformation(bool v)
{
	Q_D(toConfiguration);
    d->m_extractorSkipOrgMonInformation = v;
}

//bool toConfiguration::extractorSkipStorageExceptTablespaces()
//{
//	Q_D(toConfiguration);
//    return d->m_extractorSkipStorageExceptTablespaces;
//}
void toConfiguration::setExtractorSkipStorageExceptTablespaces(bool v)
{
	Q_D(toConfiguration);
    d->m_extractorSkipStorageExceptTablespaces = v;
}

//bool toConfiguration::extractorIncludeParallel()
//{
//	Q_D(toConfiguration);
//    return d->m_extractorIncludeParallel;
//}
void toConfiguration::setExtractorIncludeParallel(bool v)
{
	Q_D(toConfiguration);
    d->m_extractorIncludeParallel = v;
}

//bool toConfiguration::extractorIncludePartition()
//{
//	Q_D(toConfiguration);
//    return d->m_extractorIncludePartition;
//}
void toConfiguration::setExtractorIncludePartition(bool v)
{
	Q_D(toConfiguration);
    d->m_extractorIncludePartition = v;
}

//bool toConfiguration::extractorIncludeCode()
//{
//	Q_D(toConfiguration);
//    return d->m_extractorIncludeCode;
//}
void toConfiguration::setExtractorIncludeCode(bool v)
{
	Q_D(toConfiguration);
    d->m_extractorIncludeCode = v;
}

//bool toConfiguration::extractorIncludeHeader()
//{
//	Q_D(toConfiguration);
//    return d->m_extractorIncludeHeader;
//}
void toConfiguration::setExtractorIncludeHeader(bool v)
{
	Q_D(toConfiguration);
    d->m_extractorIncludeHeader = v;
}

//bool toConfiguration::extractorIncludePrompt()
//{
//	Q_D(toConfiguration);
//    return d->m_extractorIncludePrompt;
//}
void toConfiguration::setExtractorIncludePrompt(bool v)
{
	Q_D(toConfiguration);
    d->m_extractorIncludePrompt = v;
}

QString toConfiguration::sharePath()
{
#ifdef Q_OS_WIN32
	QString appl = QCoreApplication::applicationDirPath();
	appl += QDir::separator();
	appl += "i18n";
	appl += QDir::separator();
	return appl;
#else
	return QString::fromAscii(SHARE_DIR);
#endif
}
