
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

#include "core/toConfiguration_new.h"
#include "core/utils.h"

#include "toconf.h"

#include <QtCore/QObject>
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtCore/QDir>

toConfigurationNew::toConfigurationNew(QObject *parent)
	: QObject(parent)
	, d_ptr(NULL)
{	
    setQSettingsEnv();
    d_ptr = new toConfigurationNewPrivate(this);
}

toConfigurationNew::~toConfigurationNew()
{
	Q_D(toConfigurationNew);
	delete d;
}

// static one
void toConfigurationNew::setQSettingsEnv()
{
    if (QCoreApplication::organizationName().isEmpty())
        QCoreApplication::setOrganizationName(TOORGNAME);
    if (QCoreApplication::organizationDomain().isEmpty())
        QCoreApplication::setOrganizationDomain(TOORGDOMAIN);
    if (QCoreApplication::applicationName().isEmpty())
        QCoreApplication::setApplicationName(TOAPPNAME);
}

QString toConfigurationNew::customSQL()
{
	Q_D(toConfigurationNew);
    return d->m_customSQL;
}
void toConfigurationNew::setCustomSQL(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_customSQL = v;
}

QString toConfigurationNew::helpDirectory()
{
	Q_D(toConfigurationNew);
    return d->m_helpDirectory;
}
void toConfigurationNew::setHelpDirectory(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_helpDirectory = v;
}

QString toConfigurationNew::defaultSession()
{
	Q_D(toConfigurationNew);
    return d->m_defaultSession;
}
void toConfigurationNew::setDefaultSession(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_defaultSession = v;
}



void toConfigurationNew::saveConfig()
{
	Q_D(toConfigurationNew);
    d->saveConfig();
}

void toConfigurationNew::loadMap(const QString &filename, std::map<QString, QString> &pairs)
{
	Q_D(toConfigurationNew);
    d->loadMap(filename, pairs);
}

bool toConfigurationNew::saveMap(const QString &file, std::map<QString, QString> &pairs)
{
	Q_D(toConfigurationNew);
    return d->saveMap(file, pairs);
}

bool toConfigurationNew::savePassword()
{
	Q_D(toConfigurationNew);
    return d->m_savePassword;
}
void toConfigurationNew::setSavePassword(bool v)
{
	Q_D(toConfigurationNew);
    d->m_savePassword = v;
}

int toConfigurationNew::cachedConnections()
{
	Q_D(toConfigurationNew);
    return d->m_cachedConnections;
}
void toConfigurationNew::setCachedConnections(int v)
{
	Q_D(toConfigurationNew);
    d->m_cachedConnections = v;
}

int toConfigurationNew::initialFetch()
{
	Q_D(toConfigurationNew);
    return d->m_initialFetch;
}
void toConfigurationNew::setInitialFetch(int v)
{
	Q_D(toConfigurationNew);
    d->m_initialFetch = v;
}

int toConfigurationNew::maxColDisp()
{
	Q_D(toConfigurationNew);
    return d->m_maxColDisp;
}
void toConfigurationNew::setMaxColDisp(int v)
{
	Q_D(toConfigurationNew);
    d->m_maxColDisp = v;
}

QString toConfigurationNew::planTable(QString schema)
{
	Q_D(toConfigurationNew);
    if(d->m_sharedPlan || d->m_planTable.contains('.') || schema.isNull())
        return d->m_planTable;

    return schema + '.' + d->m_planTable;
}
void toConfigurationNew::setPlanTable(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_planTable = v;
}

QString toConfigurationNew::textFontName()
{
	Q_D(toConfigurationNew);
    return d->m_textFontName;
}
void toConfigurationNew::setTextFontName(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_textFontName = v;
}

QString toConfigurationNew::codeFontName()
{
	Q_D(toConfigurationNew);
    return d->m_codeFontName;
}
void toConfigurationNew::setCodeFontName(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_codeFontName = v;
}

QString toConfigurationNew::listFontName()
{
	Q_D(toConfigurationNew);
    return d->m_listFontName;
}
void toConfigurationNew::setListFontName(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_listFontName = v;
}

QString toConfigurationNew::oracleHome()
{
	Q_D(toConfigurationNew);
	return d->m_oracleHome;
}

void toConfigurationNew::setOracleHome(QString const &v)
{
	Q_D(toConfigurationNew);
	d->m_oracleHome = v;
}

QString toConfigurationNew::dateFormat()
{
	Q_D(toConfigurationNew);
    return d->m_dateFormat;
}
void toConfigurationNew::setDateFormat(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_dateFormat = v;
}

QString toConfigurationNew::timestampFormat()
{
	Q_D(toConfigurationNew);
    return d->m_timestampFormat;
}
void toConfigurationNew::setTimestampFormat(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_timestampFormat = v;
}

QString toConfigurationNew::refresh()
{
	Q_D(toConfigurationNew);
    return d->m_refresh;
}
void toConfigurationNew::setRefresh(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_refresh = v;
}

int toConfigurationNew::syntaxHighlighting()
{
	Q_D(toConfigurationNew);
    return d->m_syntaxHighlighting;
}
void toConfigurationNew::setSyntaxHighlighting(int v)
{
	Q_D(toConfigurationNew);
    d->m_syntaxHighlighting = v;
}

int toConfigurationNew::editorType()
{
	Q_D(toConfigurationNew);
    return d->m_editorType;
}
void toConfigurationNew::setEditorType(int v)
{
	Q_D(toConfigurationNew);
    d->m_editorType = v;
}

bool toConfigurationNew::keywordUpper()
{
	Q_D(toConfigurationNew);
    return d->m_keywordUpper;
}
void toConfigurationNew::setKeywordUpper(bool v)
{
	Q_D(toConfigurationNew);
    d->m_keywordUpper = v;
}

bool toConfigurationNew::objectNamesUpper()
{
	Q_D(toConfigurationNew);
    return d->m_objectNamesUpper;
}
void toConfigurationNew::setObjectNamesUpper(bool v)
{
	Q_D(toConfigurationNew);
    d->m_objectNamesUpper = v;
}

QString toConfigurationNew::cacheDir()
{
	Q_D(toConfigurationNew);
    return d->m_cacheDir;
}
void toConfigurationNew::setCacheDir(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_cacheDir = v;
}

bool toConfigurationNew::cacheDisk()
{
	Q_D(toConfigurationNew);
    return d->m_cacheDisk;
}
void toConfigurationNew::setCacheDisk(bool v)
{
	Q_D(toConfigurationNew);
    d->m_cacheDisk = v;
}

int toConfigurationNew::statusMessage()
{
	Q_D(toConfigurationNew);
    return d->m_statusMessage;
}
void toConfigurationNew::setStatusMessage(int v)
{
	Q_D(toConfigurationNew);
    d->m_statusMessage = v;
}

bool toConfigurationNew::includeDbCaption()
{
	Q_D(toConfigurationNew);
    return d->m_dbTitle;
}
void toConfigurationNew::setIncludeDbCaption(bool v)
{
	Q_D(toConfigurationNew);
    d->m_dbTitle = v;
}

QString toConfigurationNew::sizeUnit()
{
	Q_D(toConfigurationNew);
    return d->m_sizeUnit;
}
void toConfigurationNew::setSizeUnit(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_sizeUnit = v;
}

int toConfigurationNew::historySize()
{
	Q_D(toConfigurationNew);
    return d->m_historySize;
}
void toConfigurationNew::setHistorySize(int v)
{
	Q_D(toConfigurationNew);
    d->m_historySize = v;
}

bool toConfigurationNew::autoCommit()
{
	Q_D(toConfigurationNew);
    return d->m_autoCommit;
}
void toConfigurationNew::setAutoCommit(bool v)
{
	Q_D(toConfigurationNew);
    d->m_autoCommit = v;
}

bool toConfigurationNew::changeConnection()
{
	Q_D(toConfigurationNew);
    return d->m_changeConnection;
}
void toConfigurationNew::setChangeConnection(bool v)
{
	Q_D(toConfigurationNew);
    d->m_changeConnection = v;
}

QString toConfigurationNew::defaultTool()
{
	Q_D(toConfigurationNew);
    return d->m_defaultTool;
}
void toConfigurationNew::setDefaultTool(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_defaultTool = v;
}

int toConfigurationNew::chartSamples()
{
	Q_D(toConfigurationNew);
    return d->m_chartSamples;
}
void toConfigurationNew::setChartSamples(int v)
{
	Q_D(toConfigurationNew);
    d->m_chartSamples = v;
}

int toConfigurationNew::displaySamples()
{
	Q_D(toConfigurationNew);
    return d->m_displaySamples;
}
void toConfigurationNew::setDisplaySamples(int v)
{
	Q_D(toConfigurationNew);
    d->m_displaySamples = v;
}

bool toConfigurationNew::codeCompletion()
{
	Q_D(toConfigurationNew);
    return d->m_codeCompletion;
}
void toConfigurationNew::setCodeCompletion(bool v)
{
	Q_D(toConfigurationNew);
    d->m_codeCompletion = v;
}

bool toConfigurationNew::completionSort()
{
	Q_D(toConfigurationNew);
    return d->m_completionSort;
}
void toConfigurationNew::setCompletionSort(bool v)
{
	Q_D(toConfigurationNew);
    d->m_completionSort = v;
}

bool toConfigurationNew::autoIndent()
{
	Q_D(toConfigurationNew);
    return d->m_autoIndent;
}
void toConfigurationNew::setAutoIndent(bool v)
{
	Q_D(toConfigurationNew);
    d->m_autoIndent = v;
}

bool toConfigurationNew::dontReread()
{
	Q_D(toConfigurationNew);
    return d->m_dontReread;
}
void toConfigurationNew::setDontReread(bool v)
{
	Q_D(toConfigurationNew);
    d->m_dontReread = v;
}

bool toConfigurationNew::messageStatusbar()
{
	Q_D(toConfigurationNew);
    return d->m_messageStatusbar;
}
void toConfigurationNew::setMessageStatusbar(bool v)
{
	Q_D(toConfigurationNew);
    d->m_messageStatusbar = v;
}

bool toConfigurationNew::multiLineResults()
{
	Q_D(toConfigurationNew);
    return d->m_multiLineResults;
}
void toConfigurationNew::setMultiLineResults(bool v)
{
	Q_D(toConfigurationNew);
    d->m_multiLineResults = v;
}

bool toConfigurationNew::colorizedConnections()
{
	Q_D(toConfigurationNew);
    return d->m_colorizedConnections;
}
void toConfigurationNew::setColorizedConnections(bool v)
{
	Q_D(toConfigurationNew);
    d->m_colorizedConnections = v;
}
ConnectionColors toConfigurationNew::connectionColors()
{
	Q_D(toConfigurationNew);
    return d->m_connectionColors;
}
void toConfigurationNew::setConnectionColors(const ConnectionColors & v)
{
	Q_D(toConfigurationNew);
    d->m_connectionColors = v;
}

toConfigurationNew::ObjectCacheEnum toConfigurationNew::objectCache()
{
	Q_D(toConfigurationNew);
    return d->m_objectCache;
}
void toConfigurationNew::setObjectCache(toConfigurationNew::ObjectCacheEnum v)
{
	Q_D(toConfigurationNew);
    d->m_objectCache = v;
}

bool toConfigurationNew::firewallMode()
{
	Q_D(toConfigurationNew);
    return d->m_firewallMode;
}
void toConfigurationNew::setFirewallMode(bool v)
{
	Q_D(toConfigurationNew);
    d->m_firewallMode = v;
}

int toConfigurationNew::connTestInterval()
{
	Q_D(toConfigurationNew);
    return d->m_connTestInterval;
}
void toConfigurationNew::setConnTestInterval(int v)
{
	Q_D(toConfigurationNew);
    d->m_connTestInterval = v;
}

int toConfigurationNew::initialEditorContent()
{
	Q_D(toConfigurationNew);
    return d->m_initialEditorContent;
}
void toConfigurationNew::setInitialEditorContent(int v)
{
	Q_D(toConfigurationNew);
    d->m_initialEditorContent = v;
}

bool toConfigurationNew::keepPlans()
{
	Q_D(toConfigurationNew);
    return d->m_keepPlans;
}
void toConfigurationNew::setKeepPlans(bool v)
{
	Q_D(toConfigurationNew);
    d->m_keepPlans = v;
}

bool toConfigurationNew::vsqlPlans()
{
	Q_D(toConfigurationNew);
    return d->m_vsqlPlans;
}
void toConfigurationNew::setVsqlPlans(bool v)
{
	Q_D(toConfigurationNew);
    d->m_vsqlPlans = v;
}

bool toConfigurationNew::sharedPlan()
{
	Q_D(toConfigurationNew);
    return d->m_sharedPlan;
}
void toConfigurationNew::setSharedPlan(bool v)
{
	Q_D(toConfigurationNew);
    d->m_sharedPlan = v;
}

bool toConfigurationNew::restoreSession()
{
	Q_D(toConfigurationNew);
    return d->m_restoreSession;
}
void toConfigurationNew::setRestoreSession(bool v)
{
	Q_D(toConfigurationNew);
    d->m_restoreSession = v;
}


int toConfigurationNew::defaultFormat()
{
	Q_D(toConfigurationNew);
    return d->m_defaultFormat;
}
void toConfigurationNew::setDefaultFormat(int v)
{
	Q_D(toConfigurationNew);
    d->m_defaultFormat = v;
}

bool toConfigurationNew::autoIndentRo()
{
	Q_D(toConfigurationNew);
    return d->m_autoIndentRo;
}
void toConfigurationNew::setAutoIndentRo(bool v)
{
	Q_D(toConfigurationNew);
    d->m_autoIndentRo = v;
}

bool toConfigurationNew::indicateEmpty()
{
	Q_D(toConfigurationNew);
    return d->m_indicateEmpty;
}
void toConfigurationNew::setIndicateEmpty(bool v)
{
	Q_D(toConfigurationNew);
    d->m_indicateEmpty = v;
}

QString toConfigurationNew::indicateEmptyColor()
{
	Q_D(toConfigurationNew);
    return d->m_indicateEmptyColor;
}
void toConfigurationNew::setIndicateEmptyColor(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_indicateEmptyColor = v;
}

QString toConfigurationNew::extensions()
{
	Q_D(toConfigurationNew);
    return d->m_extensions;
}
void toConfigurationNew::setExtensions(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_extensions = v;
}

int toConfigurationNew::recentMax()
{
	Q_D(toConfigurationNew);
    return d->m_recentMax;
}

QString toConfigurationNew::lastDir()
{
	Q_D(toConfigurationNew);
    return d->m_lastDir;
}
void toConfigurationNew::setLastDir(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_lastDir = v;
}

QString toConfigurationNew::translation()
{
	Q_D(toConfigurationNew);
    return d->m_translation;
}
void toConfigurationNew::setTranslation(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_translation = v;
}

int toConfigurationNew::numberFormat()
{
	Q_D(toConfigurationNew);
    return d->m_numberFormat;
}
void toConfigurationNew::setNumberFormat(int v)
{
	Q_D(toConfigurationNew);
    d->m_numberFormat = v;
}

int toConfigurationNew::numberDecimals()
{
	Q_D(toConfigurationNew);
    return d->m_numberDecimals;
}
void toConfigurationNew::setNumberDecimals(int v)
{
	Q_D(toConfigurationNew);
    d->m_numberDecimals = v;
}

int toConfigurationNew::cacheTimeout()
{
	Q_D(toConfigurationNew);
    return d->m_cacheTimeout;
}

int toConfigurationNew::tabStop()
{
	Q_D(toConfigurationNew);
    return d->m_tabStop;
}
void toConfigurationNew::setTabStop(int v)
{
	Q_D(toConfigurationNew);
    d->m_tabStop = v;
}

bool toConfigurationNew::useSpacesForIndent()
{
	Q_D(toConfigurationNew);
    return d->m_useSpacesForIndent;
}
void toConfigurationNew::setUseSpacesForIndent(bool v)
{
	Q_D(toConfigurationNew);
    d->m_useSpacesForIndent = v;
}

bool toConfigurationNew::editDragDrop()
{
	Q_D(toConfigurationNew);
    return d->m_editDragDrop;
}
void toConfigurationNew::setEditDragDrop(bool v)
{
	Q_D(toConfigurationNew);
    d->m_editDragDrop = v;
}

bool toConfigurationNew::toadBindings()
{
	Q_D(toConfigurationNew);
    return d->m_toadBindings;
}
void toConfigurationNew::setToadBindings(bool v)
{
	Q_D(toConfigurationNew);
    d->m_toadBindings = v;
}

bool toConfigurationNew::displayGridlines()
{
	Q_D(toConfigurationNew);
    return d->m_displayGridlines;
}
void toConfigurationNew::setDisplayGridlines(bool v)
{
	Q_D(toConfigurationNew);
    d->m_displayGridlines = v;
}

QString toConfigurationNew::style()
{
	Q_D(toConfigurationNew);
    return d->m_style;
}
void toConfigurationNew::setStyle(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_style = v;
}

int toConfigurationNew::maxLong()
{
	Q_D(toConfigurationNew);
    return d->m_maxLong;
}
void toConfigurationNew::setMaxLong(int v)
{
	Q_D(toConfigurationNew);
    d->m_maxLong = v;
}

// toqsqlconnection
QString toConfigurationNew::mysqlHome()
{
	Q_D(toConfigurationNew);
	return d->m_mysqlHome;
}

void toConfigurationNew::setMysqlHome(QString const &v)
{
	Q_D(toConfigurationNew);
	d->m_mysqlHome = v;
}

QString toConfigurationNew::pgsqlHome()
{
	Q_D(toConfigurationNew);
	return d->m_pgsqlHome;
}

void toConfigurationNew::setPgsqlHome(QString const &v)
{
	Q_D(toConfigurationNew);
	d->m_pgsqlHome = v;
}

int toConfigurationNew::beforeCreateAction()
{
	Q_D(toConfigurationNew);
    return d->m_beforeCreateAction;
}

void toConfigurationNew::setBeforeCreateAction(int v)
{
	Q_D(toConfigurationNew);
    d->m_beforeCreateAction = v;
}

// main.cpp
QString toConfigurationNew::lastVersion()
{
	Q_D(toConfigurationNew);
    return d->m_lastVersion;
}
void toConfigurationNew::setLastVersion(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_lastVersion = v;
}

QString toConfigurationNew::firstInstall()
{
	Q_D(toConfigurationNew);
    return d->m_firstInstall;
}
void toConfigurationNew::setFirstInstall(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_firstInstall = v;
}
// tomain.cpp
QStringList toConfigurationNew::recentFiles()
{
	Q_D(toConfigurationNew);
    return d->m_recentFiles;
}
void toConfigurationNew::setRecentFiles(QStringList v)
{
	Q_D(toConfigurationNew);
    d->m_recentFiles = v;
}
// tools
ToolsMap& toConfigurationNew::tools()
{
	Q_D(toConfigurationNew);
    return d->m_tools;
}
void toConfigurationNew::setTools(ToolsMap &v)
{
	Q_D(toConfigurationNew);
    d->m_tools = v;
}

// toresultlistformat
QString toConfigurationNew::csvSeparator()
{
	Q_D(toConfigurationNew);
    return d->m_csvSeparator;
}
QString toConfigurationNew::csvDelimiter()
{
	Q_D(toConfigurationNew);
    return d->m_csvDelimiter;
}

// toeditextensions
bool toConfigurationNew::commaBefore()
{
	Q_D(toConfigurationNew);
    return d->m_commaBefore;
}
void toConfigurationNew::setCommaBefore(bool v)
{
	Q_D(toConfigurationNew);
    d->m_commaBefore = v;
}

bool toConfigurationNew::blockOpenLine()
{
	Q_D(toConfigurationNew);
    return d->m_blockOpenLine;
}
void toConfigurationNew::setBlockOpenLine(bool v)
{
	Q_D(toConfigurationNew);
    d->m_blockOpenLine = v;
}

bool toConfigurationNew::operatorSpace()
{
	Q_D(toConfigurationNew);
    return d->m_operatorSpace;
}
void toConfigurationNew::setOperatorSpace(bool v)
{
	Q_D(toConfigurationNew);
    d->m_operatorSpace = v;
}

bool toConfigurationNew::rightSeparator()
{
	Q_D(toConfigurationNew);
    return d->m_rightSeparator;
}
void toConfigurationNew::setRightSeparator(bool v)
{
	Q_D(toConfigurationNew);
    d->m_rightSeparator = v;
}

bool toConfigurationNew::endBlockNewline()
{
	Q_D(toConfigurationNew);
    return d->m_endBlockNewline;
}
void toConfigurationNew::setEndBlockNewline(bool v)
{
	Q_D(toConfigurationNew);
    d->m_endBlockNewline = v;
}

int toConfigurationNew::commentColumn()
{
	Q_D(toConfigurationNew);
    return d->m_commentColumn;
}
void toConfigurationNew::setCommentColumn(int v)
{
	Q_D(toConfigurationNew);
    d->m_commentColumn = v;
}

// tobrowser
bool toConfigurationNew::filterIgnoreCase()
{
	Q_D(toConfigurationNew);
    return d->m_filterIgnoreCase;
}
void toConfigurationNew::setFilterIgnoreCase(bool v)
{
	Q_D(toConfigurationNew);
    d->m_filterIgnoreCase = v;
}

bool toConfigurationNew::filterInvert()
{
	Q_D(toConfigurationNew);
    return d->m_filterInvert;
}
void toConfigurationNew::setFilterInvert(bool v)
{
	Q_D(toConfigurationNew);
    d->m_filterInvert = v;
}

int toConfigurationNew::filterType()
{
	Q_D(toConfigurationNew);
    return d->m_filterType;
}
void toConfigurationNew::setFilterType(int v)
{
	Q_D(toConfigurationNew);
    d->m_filterType = v;
}

int toConfigurationNew::filterTablespaceType()
{
	Q_D(toConfigurationNew);
    return d->m_filterTablespaceType;
}
void toConfigurationNew::setFilterTablespaceType(int v)
{
	Q_D(toConfigurationNew);
    d->m_filterTablespaceType = v;
}

QString toConfigurationNew::filterText()
{
	Q_D(toConfigurationNew);
    return d->m_filterText;
}
void toConfigurationNew::setFilterText(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_filterText = v;
}

// tohelp
HelpsMap toConfigurationNew::additionalHelp()
{
	Q_D(toConfigurationNew);
    return d->m_additionalHelp;
}
void toConfigurationNew::setAdditionalHelp(HelpsMap v)
{
	Q_D(toConfigurationNew);
    d->m_additionalHelp = v;
}

#ifdef TORA3_CHARTS
// tochartmanager
ChartsMap toConfigurationNew::chartFiles()
{
	Q_D(toConfigurationNew);
    return d->m_chartFiles;
}
void toConfigurationNew::setChartFiles(ChartsMap v)
{
	Q_D(toConfigurationNew);
    d->m_chartFiles = v;
}

ChartsMap toConfigurationNew::chartAlarms()
{
	Q_D(toConfigurationNew);
    return d->m_chartAlarms;
}
void toConfigurationNew::setChartAlarms(ChartsMap v)
{
	Q_D(toConfigurationNew);
    d->m_chartAlarms = v;
}
#endif

// toooutput
QString toConfigurationNew::polling()
{
	Q_D(toConfigurationNew);
    return d->m_polling;
}
void toConfigurationNew::setPolling(const QString & v )
{
	Q_D(toConfigurationNew);
    d->m_polling = v;
}

int toConfigurationNew::logType()
{
	Q_D(toConfigurationNew);
    return d->m_logType;
}
void toConfigurationNew::setLogType(int v)
{
	Q_D(toConfigurationNew);
    d->m_logType = v;
}

QString toConfigurationNew::logUser()
{
	Q_D(toConfigurationNew);
    return d->m_logUser;
}
void toConfigurationNew::setLogUser(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_logUser = v;
}

// torollback
bool toConfigurationNew::noExec()
{
	Q_D(toConfigurationNew);
    return d->m_noExec;
}
void toConfigurationNew::setNoExec(bool v)
{
	Q_D(toConfigurationNew);
    d->m_noExec = v;
}

bool toConfigurationNew::needRead()
{
	Q_D(toConfigurationNew);
    return d->m_needRead;
}
void toConfigurationNew::setNeedRead(bool v)
{
	Q_D(toConfigurationNew);
    d->m_needRead = v;
}

bool toConfigurationNew::needTwo()
{
	Q_D(toConfigurationNew);
    return d->m_needTwo;
}
void toConfigurationNew::setNeedTwo(bool v)
{
	Q_D(toConfigurationNew);
    d->m_needTwo = v;
}

bool toConfigurationNew::alignLeft()
{
	Q_D(toConfigurationNew);
    return d->m_alignLeft;
}
void toConfigurationNew::setAlignLeft(bool v)
{
	Q_D(toConfigurationNew);
    d->m_alignLeft = v;
}

bool toConfigurationNew::oldEnable()
{
	Q_D(toConfigurationNew);
    return d->m_oldEnable;
}
void toConfigurationNew::setOldEnable(bool v)
{
	Q_D(toConfigurationNew);
    d->m_oldEnable = v;
}

// tosgatrace
bool toConfigurationNew::autoUpdate()
{
	Q_D(toConfigurationNew);
    return d->m_autoUpdate;
}
void toConfigurationNew::setAutoUpdate(bool v)
{
	Q_D(toConfigurationNew);
    d->m_autoUpdate = v;
}

// tostorage
bool toConfigurationNew::dispTablespaces()
{
	Q_D(toConfigurationNew);
    return d->m_dispTablespaces;
}
void toConfigurationNew::setDispTablespaces(bool v)
{
	Q_D(toConfigurationNew);
    d->m_dispTablespaces = v;
}

bool toConfigurationNew::dispCoalesced()
{
	Q_D(toConfigurationNew);
    return d->m_dispCoalesced;
}
void toConfigurationNew::setDispCoalesced(bool v)
{
	Q_D(toConfigurationNew);
    d->m_dispCoalesced = v;
}

bool toConfigurationNew::dispExtents()
{
	Q_D(toConfigurationNew);
    return d->m_dispExtents;
}
void toConfigurationNew::setDispExtents(bool v)
{
	Q_D(toConfigurationNew);
    d->m_dispExtents = v;
}

bool toConfigurationNew::dispAvailableGraph()
{
	Q_D(toConfigurationNew);
    return d->m_dispAvailableGraph;
}
void toConfigurationNew::setDispAvailableGraph(bool v)
{
	Q_D(toConfigurationNew);
    d->m_dispAvailableGraph = v;
}

// totemplate
TemplatesMap toConfigurationNew::templates()
{
	Q_D(toConfigurationNew);
    return d->m_templates;
}
void toConfigurationNew::setTemplates(TemplatesMap v)
{
	Q_D(toConfigurationNew);
    d->m_templates = v;
}

// totuning
/*! True if there are no prefs for tuning.
  It's used when there is no d->m_tuningOverview defined in prefs. */
bool toConfigurationNew::tuningFirstRun()
{
	Q_D(toConfigurationNew);
    return d->m_tuningFirstRun;
}

bool toConfigurationNew::tuningOverview()
{
	Q_D(toConfigurationNew);
    return d->m_tuningOverview;
}
void toConfigurationNew::setTuningOverview(bool v)
{
	Q_D(toConfigurationNew);
    d->m_tuningOverview = v;
}

bool toConfigurationNew::tuningFileIO()
{
	Q_D(toConfigurationNew);
    return d->m_tuningFileIO;
}
void toConfigurationNew::setTuningFileIO(bool v)
{
	Q_D(toConfigurationNew);
    d->m_tuningFileIO = v;
}

bool toConfigurationNew::tuningWaits()
{
	Q_D(toConfigurationNew);
    return d->m_tuningWaits;
}
void toConfigurationNew::setTuningWaits(bool v)
{
	Q_D(toConfigurationNew);
    d->m_tuningWaits = v;
}

bool toConfigurationNew::tuningCharts()
{
	Q_D(toConfigurationNew);
    return d->m_tuningCharts;
}
void toConfigurationNew::setTuningCharts(bool v)
{
	Q_D(toConfigurationNew);
    d->m_tuningCharts = v;
}

// toworksheet
bool toConfigurationNew::wsAutoSave()
{
	Q_D(toConfigurationNew);
    return d->m_wsAutoSave;
}
void toConfigurationNew::setWsAutoSave(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsAutoSave = v;
}

bool toConfigurationNew::wsCheckSave()
{
	Q_D(toConfigurationNew);
    return d->m_wsCheckSave;
}
void toConfigurationNew::setWsCheckSave(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsCheckSave = v;
}

QString toConfigurationNew::wsAutoLoad()
{
	Q_D(toConfigurationNew);
    return d->m_wsAutoLoad;
}
void toConfigurationNew::setWsAutoLoad(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_wsAutoLoad = v;
}

bool toConfigurationNew::wsLogAtEnd()
{
	Q_D(toConfigurationNew);
    return d->m_wsLogAtEnd;
}
void toConfigurationNew::setWsLogAtEnd(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsLogAtEnd = v;
}

bool toConfigurationNew::wsLogMulti()
{
	Q_D(toConfigurationNew);
    return d->m_wsLogMulti;
}
void toConfigurationNew::setWsLogMulti(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsLogMulti = v;
}

bool toConfigurationNew::wsStatistics()
{
	Q_D(toConfigurationNew);
    return d->m_wsStatistics;
}
void toConfigurationNew::setWsStatistics(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsStatistics = v;
}

bool toConfigurationNew::wsTimedStats()
{
	Q_D(toConfigurationNew);
    return d->m_wsTimedStats;
}
void toConfigurationNew::setWsTimedStats(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsTimedStats = v;
}

bool toConfigurationNew::wsNumber()
{
	Q_D(toConfigurationNew);
    return d->m_wsNumber;
}
void toConfigurationNew::setWsNumber(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsNumber = v;
}

bool toConfigurationNew::wsMoveToErr()
{
	Q_D(toConfigurationNew);
    return d->m_wsMoveToErr;
}
void toConfigurationNew::setWsMoveToErr(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsMoveToErr = v;
}

bool toConfigurationNew::wsHistory()
{
	Q_D(toConfigurationNew);
    return d->m_wsHistory;
}
void toConfigurationNew::setWsHistory(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsHistory = v;
}

bool toConfigurationNew::wsExecLog()
{
	Q_D(toConfigurationNew);
    return d->m_wsExecLog;
}
void toConfigurationNew::setWsExecLog(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsExecLog = v;
}

bool toConfigurationNew::wsToplevelDescribe()
{
	Q_D(toConfigurationNew);
    return d->m_wsToplevelDescribe;
}
void toConfigurationNew::setWsToplevelDescribe(bool v)
{
	Q_D(toConfigurationNew);
    d->m_wsToplevelDescribe = v;
}


bool toConfigurationNew::useEditorShortcuts()
{
	Q_D(toConfigurationNew);
    return d->m_useEditorShortcuts;
}

void toConfigurationNew::setUseEditorShortcuts(bool v)
{
	Q_D(toConfigurationNew);
    d->m_useEditorShortcuts = v;
}

EditorShortcutsMap toConfigurationNew::editorShortcuts()
{
	Q_D(toConfigurationNew);
    return d->m_editorShortcuts;
}

void toConfigurationNew::setEditorShortcuts(const EditorShortcutsMap & v)
{
	Q_D(toConfigurationNew);
    d->m_editorShortcuts = v;
}

QColor toConfigurationNew::styleFgColor(int style)
{
	Q_D(toConfigurationNew);
    return d->m_styles[style].FGColor;
}

QColor toConfigurationNew::styleBgColor(int style)
{
	Q_D(toConfigurationNew);
    return d->m_styles[style].BGColor;
}

QFont  toConfigurationNew::styleFont(int style)
{
	Q_D(toConfigurationNew);
    return d->m_styles[style].Font;
}

const toStylesMap& toConfigurationNew::styles()
{
	Q_D(toConfigurationNew);
    return d->m_styles;
}

void toConfigurationNew::setStyles(const toStylesMap & styles)
{
	Q_D(toConfigurationNew);
    d->m_styles = styles;
}

bool toConfigurationNew::useMaxTextWidthMark()
{
	Q_D(toConfigurationNew);
    return d->m_useMaxTextWidthMark;
}

void toConfigurationNew::setUseMaxTextWidthMark(bool v)
{
	Q_D(toConfigurationNew);
    d->m_useMaxTextWidthMark = v;
}

int toConfigurationNew::maxTextWidthMark()
{
	Q_D(toConfigurationNew);
    return d->m_maxTextWidthMark;
}

void toConfigurationNew::setMaxTextWidthMark(int v)
{
	Q_D(toConfigurationNew);
    d->m_maxTextWidthMark = v;
}

QByteArray toConfigurationNew::mainWindowGeometry()
{
	Q_D(toConfigurationNew);
    return d->m_mainWindowGeometry;
}

void toConfigurationNew::setMainWindowGeometry(QByteArray v)
{
	Q_D(toConfigurationNew);
    d->m_mainWindowGeometry = v;
}

QByteArray toConfigurationNew::mainWindowState()
{
	Q_D(toConfigurationNew);
    return d->m_mainWindowState;
}

void toConfigurationNew::setMainWindowState(QByteArray v)
{
	Q_D(toConfigurationNew);
    d->m_mainWindowState = v;
}


void toConfigurationNew::setLeftDockbarState(QByteArray v)
{
	Q_D(toConfigurationNew);
    d->m_leftDockbarState = v;
}

QByteArray toConfigurationNew::leftDockbarState()
{
	Q_D(toConfigurationNew);
    return d->m_leftDockbarState;
}

void toConfigurationNew::setRightDockbarState(QByteArray v)
{
	Q_D(toConfigurationNew);
    d->m_rightDockbarState = v;
}

QByteArray toConfigurationNew::rightDockbarState()
{
	Q_D(toConfigurationNew);
    return d->m_rightDockbarState;
}

// Default encoding used when reading/writing files
QString toConfigurationNew::encoding()
{
	Q_D(toConfigurationNew);
    return d->m_encoding;
}
void toConfigurationNew::setEncoding(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_encoding = v;
}

// Force using a particular line end type (win, linux, mac) when saving text files
QString toConfigurationNew::forceLineEnd()
{
	Q_D(toConfigurationNew);
    return d->m_forcelineend;
}
void toConfigurationNew::setForceLineEnd(const QString & v)
{
	Q_D(toConfigurationNew);
    d->m_forcelineend = v;
}

bool toConfigurationNew::extractorUseDbmsMetadata()
{
	Q_D(toConfigurationNew);
    return d->m_extractorUseDbmsMetadata;
}
void toConfigurationNew::setExtractorUseDbmsMetadata(bool v)
{
	Q_D(toConfigurationNew);
    d->m_extractorUseDbmsMetadata = v;
}

bool toConfigurationNew::extractorIncludeSotrage()
{
	Q_D(toConfigurationNew);
    return d->m_extractorIncludeSotrage;
}
void toConfigurationNew::setExtractorIncludeSotrage(bool v)
{
	Q_D(toConfigurationNew);
    d->m_extractorIncludeSotrage = v;
}

bool toConfigurationNew::extractorSkipOrgMonInformation()
{
	Q_D(toConfigurationNew);
    return d->m_extractorSkipOrgMonInformation;
}
void toConfigurationNew::setExtractorSkipOrgMonInformation(bool v)
{
	Q_D(toConfigurationNew);
    d->m_extractorSkipOrgMonInformation = v;
}

bool toConfigurationNew::extractorSkipStorageExceptTablespaces()
{
	Q_D(toConfigurationNew);
    return d->m_extractorSkipStorageExceptTablespaces;
}
void toConfigurationNew::setExtractorSkipStorageExceptTablespaces(bool v)
{
	Q_D(toConfigurationNew);
    d->m_extractorSkipStorageExceptTablespaces = v;
}

bool toConfigurationNew::extractorIncludeParallel()
{
	Q_D(toConfigurationNew);
    return d->m_extractorIncludeParallel;
}
void toConfigurationNew::setExtractorIncludeParallel(bool v)
{
	Q_D(toConfigurationNew);
    d->m_extractorIncludeParallel = v;
}

bool toConfigurationNew::extractorIncludePartition()
{
	Q_D(toConfigurationNew);
    return d->m_extractorIncludePartition;
}
void toConfigurationNew::setExtractorIncludePartition(bool v)
{
	Q_D(toConfigurationNew);
    d->m_extractorIncludePartition = v;
}

bool toConfigurationNew::extractorIncludeCode()
{
	Q_D(toConfigurationNew);
    return d->m_extractorIncludeCode;
}
void toConfigurationNew::setExtractorIncludeCode(bool v)
{
	Q_D(toConfigurationNew);
    d->m_extractorIncludeCode = v;
}

bool toConfigurationNew::extractorIncludeHeader()
{
	Q_D(toConfigurationNew);
    return d->m_extractorIncludeHeader;
}
void toConfigurationNew::setExtractorIncludeHeader(bool v)
{
	Q_D(toConfigurationNew);
    d->m_extractorIncludeHeader = v;
}

bool toConfigurationNew::extractorIncludePrompt()
{
	Q_D(toConfigurationNew);
    return d->m_extractorIncludePrompt;
}
void toConfigurationNew::setExtractorIncludePrompt(bool v)
{
	Q_D(toConfigurationNew);
    d->m_extractorIncludePrompt = v;
}

QString toConfigurationNew::sharePath()
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
