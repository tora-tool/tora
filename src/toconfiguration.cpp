#include "toconfiguration.h"

// #include <qapplication.h>
// #include <qregexp.h>

#include "utils.h"

// A little magic to get lrefresh to work and get a check on qApp
#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

#include <QLocale>
#include <QSettings>
#include "toconf.h"

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

toConfiguration::toConfiguration()
{
    int cnt; // tmp count holder for setting arrays
    setQSettingsEnv();
    QSettings s;
    s.beginGroup("preferences");
    m_provider = s.value(CONF_PROVIDER, DEFAULT_PROVIDER).toString();
    m_savePassword = s.value(CONF_SAVE_PWD, DEFAULT_SAVE_PWD).toBool();
    m_defaultPassword = s.value(CONF_PASSWORD, DEFAULT_PASSWORD).toString();
    m_defaultDatabase = s.value(CONF_DATABASE, DEFAULT_DATABASE).toString();
    m_defaultUser = s.value(CONF_USER, DEFAULT_USER).toString();
    m_defaultHost = s.value(CONF_HOST, DEFAULT_HOST).toString();

// #define CONF_OPTIONS  "Options"
// #define DEFAULT_OPTIONS  ""
    m_maxNumber = s.value(CONF_MAX_NUMBER, DEFAULT_MAX_NUMBER).toInt();
    m_maxColSize = s.value(CONF_MAX_COL_SIZE, DEFAULT_MAX_COL_SIZE).toInt();
    m_maxColDisp = s.value(CONF_MAX_COL_DISP, DEFAULT_MAX_COL_DISP).toInt();
    m_planTable = s.value(CONF_PLAN_TABLE, DEFAULT_PLAN_TABLE).toString();
    m_planCheckpoint = s.value(CONF_PLAN_CHECKPOINT, DEFAULT_PLAN_CHECKPOINT).toString();
    m_textFont = s.value(CONF_TEXT, "").toString();
    m_codeFont = s.value(CONF_CODE, "").toString();
    m_listFont = s.value(CONF_LIST, "").toString();
    m_dateFormat = s.value(CONF_DATE_FORMAT, DEFAULT_DATE_FORMAT).toString();
    m_refresh = s.value(CONF_REFRESH, DEFAULT_REFRESH).toString();
    m_highlight = s.value(CONF_HIGHLIGHT, true).toBool();
    m_keywordUpper = s.value(CONF_KEYWORD_UPPER, DEFAULT_KEYWORD_UPPER).toBool();

    m_pluginDir = s.value(CONF_PLUGIN_DIR, DEFAULT_PLUGIN_DIR).toString();
    m_cacheDir = s.value(CONF_CACHE_DIR, "").toString();
    m_cacheDisk = s.value(CONF_CACHE_DISK, DEFAULT_CACHE_DISK).toBool();
    m_sqlFile = s.value(CONF_SQL_FILE, DEFAULT_SQL_FILE).toString();
    m_statusMessage = s.value(CONF_STATUS_MESSAGE, DEFAULT_STATUS_MESSAGE).toInt();

    m_dbTitle = s.value(CONF_DB_TITLE, true).toBool();
    m_sizeUnit = s.value(CONF_SIZE_UNIT, DEFAULT_SIZE_UNIT).toString();
    m_helpPath = s.value(CONF_HELP_PATH, "qrc:/help/toc.html").toString();
    m_statusSave = s.value(CONF_STATUS_SAVE, DEFAULT_STATUS_SAVE).toInt();
    m_autoCommit = s.value(CONF_AUTO_COMMIT, false).toBool();
    m_changeConnection = s.value(CONF_CHANGE_CONNECTION, true).toBool();

// #define CONF_CONNECT_HISTORY "ConnectHistory"
    m_connectSize = s.value(CONF_CONNECT_SIZE, DEFAULT_CONNECT_SIZE).toInt();

// #define CONF_CONNECT_CURRENT "ConnectCurrent"
    m_defaultTool = s.value(CONF_DEFAULT_TOOL).toString();
// #define CONF_TOOL_ENABLE "Enabled"
    m_chartSamples = s.value(CONF_CHART_SAMPLES, DEFAULT_CHART_SAMPLES).toInt();
    m_displaySamples = s.value(CONF_DISPLAY_SAMPLES, DEFAULT_DISPLAY_SAMPLES).toInt();
    m_codeCompletion = s.value(CONF_CODE_COMPLETION, true).toBool();
    m_completionSort = s.value(CONF_COMPLETION_SORT, true).toBool();
    m_autoIndent = s.value(CONF_AUTO_INDENT, true).toBool();
    m_dontReread = s.value(CONF_DONT_REREAD, true).toBool();
    m_autoLong = s.value(CONF_AUTO_LONG, 0).toInt();
    m_messageStatusbar = s.value(CONF_MESSAGE_STATUSBAR, false).toBool();
    m_tabbedTools = s.value(CONF_TABBED_TOOLS, false).toBool();
    m_objectCache = s.value(CONF_OBJECT_CACHE, DEFAULT_OBJECT_CACHE).toInt();
    m_bkgndConnect = s.value(CONF_BKGND_CONNECT, false).toBool();
    m_firewallMode = s.value(CONF_FIREWALL_MODE, false).toBool();
    m_maxContent = s.value(CONF_MAX_CONTENT, DEFAULT_MAX_CONTENT).toInt();
    m_keepPlans = s.value(CONF_KEEP_PLANS, false).toBool();
    m_restoreSession = s.value(CONF_RESTORE_SESSION, false).toBool();
    m_defaultSession = s.value(CONF_DEFAULT_SESSION, DEFAULT_SESSION).toString();
    // FIXME!
    m_defaultFormat = s.value(CONF_DEFAULT_FORMAT, "").toInt();
    m_autoIndentRo = s.value(CONF_AUTO_INDENT_RO, true).toBool();
    m_indicateEmpty = s.value(CONF_INDICATE_EMPTY, true).toBool();
    m_indicateEmptyColor = s.value(CONF_INDICATE_EMPTY_COLOR, "#f2ffbc").toString();
    m_extensions = s.value(CONF_EXTENSIONS, DEFAULT_EXTENSIONS).toString();
    m_recentFiles = s.value(CONF_RECENT_FILES, QStringList()).toStringList();
    m_recentMax = s.value(CONF_RECENT_MAX, DEFAULT_RECENT_MAX).toInt();
    m_lastDir = s.value(CONF_LAST_DIR, "").toString();
    m_locale = s.value(CONF_LOCALE, QLocale().name()).toString();
    m_keepAlive = s.value(CONF_KEEP_ALIVE, DEFAULT_KEEP_ALIVE).toInt();
    m_numberFormat = s.value(CONF_NUMBER_FORMAT, DEFAULT_NUMBER_FORMAT).toInt();
    m_numberDecimals = s.value(CONF_NUMBER_DECIMALS, DEFAULT_NUMBER_DECIMALS).toInt();
    m_cacheTimeout = s.value(CONF_CACHE_TIMEOUT, DEFAULT_CACHE_TIMEOUT).toInt();
    m_tabStop = s.value(CONF_TAB_STOP, DEFAULT_TAB_STOP).toInt();
    m_tabSpaces = s.value(CONF_TAB_SPACES, DEFAULT_TAB_SPACES).toBool();
    m_editDragDrop = s.value(CONF_EDIT_DRAG_DROP, DEFAULT_EDIT_DRAG_DROP).toBool();
    m_toadBindings = s.value(CONF_TOAD_BINDINGS, DEFAULT_TOAD_BINDINGS).toBool();
    m_displayGridlines = s.value(CONF_DISPLAY_GRIDLINES, DEFAULT_DISPLAY_GRIDLINES).toBool();
    m_smtp = s.value(CONF_SMTP, DEFAULT_SMTP).toString();
    m_smtpPort = s.value(CONF_SMTP_PORT, DEFAULT_SMTP_PORT).toInt();
    m_style = s.value(CONF_STYLE, DEFAULT_STYLE).toString();
    // tooracleconnection.cpp
    m_openCursors = s.value(CONF_OPEN_CURSORS, DEFAULT_OPEN_CURSORS).toInt();
    m_maxLong = s.value(CONF_MAX_LONG, DEFAULT_MAX_LONG).toInt();
    // toqsqlconnection
    m_onlyForward = s.value(CONF_ONLY_FORWARD, DEFAULT_ONLY_FORWARD).toBool();
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

    s.endGroup();

    // main window
    s.beginGroup("toMainWindow");
    m_mainWindowGeometry = s.value("geometry").toByteArray();
    // mainwindow's toolbars etc
    m_mainWindowState = s.value("state").toByteArray();
    s.endGroup();
}

toConfiguration::~toConfiguration()
{
}

void toConfiguration::saveConfig()
{
    QString key; // tmp key for maps
    QSettings s;
    s.beginGroup("preferences");
    s.setValue(CONF_PROVIDER, m_provider);
    s.setValue(CONF_SAVE_PWD, m_savePassword);
    s.setValue(CONF_PASSWORD, m_defaultPassword);
    s.setValue(CONF_DATABASE, m_defaultDatabase);
    s.setValue(CONF_USER, m_defaultUser);
    s.setValue(CONF_HOST, m_defaultHost);
    s.setValue(CONF_MAX_NUMBER, m_maxNumber);
    s.setValue(CONF_MAX_COL_SIZE, m_maxColSize);
    s.setValue(CONF_MAX_COL_DISP, m_maxColDisp);
    s.setValue(CONF_PLAN_TABLE, m_planTable);
    s.setValue(CONF_PLAN_CHECKPOINT, m_planCheckpoint);
    s.setValue(CONF_TEXT, m_textFont);
    s.setValue(CONF_CODE, m_codeFont);
    s.setValue(CONF_LIST, m_listFont);
    s.setValue(CONF_DATE_FORMAT, m_dateFormat);
    s.setValue(CONF_REFRESH, m_refresh);
    s.setValue(CONF_HIGHLIGHT, m_highlight);
    s.setValue(CONF_KEYWORD_UPPER, m_keywordUpper);
    s.setValue(CONF_PLUGIN_DIR, m_pluginDir);
    s.setValue(CONF_CACHE_DIR, m_cacheDir);
    s.setValue(CONF_CACHE_DISK, m_cacheDisk);
    s.setValue(CONF_SQL_FILE, m_sqlFile);
    s.setValue(CONF_STATUS_MESSAGE, m_statusMessage);
    s.setValue(CONF_DB_TITLE, m_dbTitle);
    s.setValue(CONF_SIZE_UNIT, m_sizeUnit);
    s.setValue(CONF_HELP_PATH, m_helpPath);
    s.setValue(CONF_STATUS_SAVE, m_statusSave);
    s.setValue(CONF_AUTO_COMMIT, m_autoCommit);
    s.setValue(CONF_CHANGE_CONNECTION, m_changeConnection);
    s.setValue(CONF_CONNECT_SIZE, m_connectSize);
    s.setValue(CONF_DEFAULT_TOOL, m_defaultTool);
    s.setValue(CONF_CHART_SAMPLES, m_chartSamples);
    s.setValue(CONF_DISPLAY_SAMPLES, m_displaySamples);
    s.setValue(CONF_CODE_COMPLETION, m_codeCompletion);
    s.setValue(CONF_COMPLETION_SORT, m_completionSort);
    s.setValue(CONF_AUTO_INDENT, m_autoIndent);
    s.setValue(CONF_DONT_REREAD, m_dontReread);
    s.setValue(CONF_AUTO_LONG, m_autoLong);
    s.setValue(CONF_MESSAGE_STATUSBAR, m_messageStatusbar);
    s.setValue(CONF_TABBED_TOOLS, m_tabbedTools);
    s.setValue(CONF_OBJECT_CACHE, m_objectCache);
    s.setValue(CONF_BKGND_CONNECT, m_bkgndConnect);
    s.setValue(CONF_FIREWALL_MODE, m_firewallMode);
    s.setValue(CONF_MAX_CONTENT, m_maxContent);
    s.setValue(CONF_KEEP_PLANS, m_keepPlans);
    s.setValue(CONF_RESTORE_SESSION, m_restoreSession);
    s.setValue(CONF_DEFAULT_SESSION, m_defaultSession);
    s.setValue(CONF_DEFAULT_FORMAT, m_defaultFormat);
    s.setValue(CONF_AUTO_INDENT_RO, m_autoIndentRo);
    s.setValue(CONF_INDICATE_EMPTY, m_indicateEmpty);
    s.setValue(CONF_INDICATE_EMPTY_COLOR, m_indicateEmptyColor);
    s.setValue(CONF_EXTENSIONS, m_extensions);
    s.setValue(CONF_RECENT_FILES, m_recentFiles);
    s.setValue(CONF_RECENT_MAX, m_recentMax);
    s.setValue(CONF_LAST_DIR, m_lastDir);
    s.setValue(CONF_LOCALE, m_locale);
    s.setValue(CONF_KEEP_ALIVE, m_keepAlive);
    s.setValue(CONF_NUMBER_FORMAT, m_numberFormat);
    s.setValue(CONF_NUMBER_DECIMALS, m_numberDecimals);
    s.setValue(CONF_CACHE_TIMEOUT, m_cacheTimeout);
    s.setValue(CONF_TAB_STOP, m_tabStop);
    s.setValue(CONF_TAB_SPACES, m_tabSpaces);
    s.setValue(CONF_EDIT_DRAG_DROP, m_editDragDrop);
    s.setValue(CONF_TOAD_BINDINGS, m_toadBindings);
    s.setValue(CONF_DISPLAY_GRIDLINES, m_displayGridlines);
    s.setValue(CONF_SMTP, m_smtp);
    s.setValue(CONF_SMTP_PORT, m_smtpPort);
    s.setValue(CONF_STYLE, m_style);
    // tooracleconnection
    s.setValue(CONF_OPEN_CURSORS, m_openCursors);
    s.setValue(CONF_MAX_LONG, m_maxLong);
    // toqsqlconnection
    s.setValue(CONF_ONLY_FORWARD, m_onlyForward);
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

    s.endGroup();

    // main window
    s.beginGroup("toMainWindow");
    s.setValue("geometry", m_mainWindowGeometry);
    // mainwindow's toolbars etc
    s.setValue("state", m_mainWindowState);
    s.endGroup();
}

void toConfiguration::loadMap(const QString &filename, std::map<QString, QString> &pairs)
{
    QByteArray data = toReadFile(filename).toUtf8();

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
        }
        wpos++;
        pos++;
    }
    return ;
}

bool toConfiguration::saveMap(const QString &file, std::map<QString, QString> &pairs)
{
    QString data;

    {
// qt4        QRegExp newline(QString::fromLatin1("\n"));
// qt4        QRegExp backslash(QString::fromLatin1("\\"));
        QString newline("\n");
        QString backslash("\\");
        for (std::map<QString, QString>::iterator i = pairs.begin();i != pairs.end();i++)
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
    return toWriteFile(file, data);
}

QColor toConfiguration::syntaxDefault()
{
    QColor c;
    c.setNamedColor(m_syntaxDefault);
    return c;
}
void toConfiguration::setSyntaxDefault(QColor v)
{
    m_syntaxDefault = v.name();
}

QColor toConfiguration::syntaxComment()
{
    QColor c;
    c.setNamedColor(m_syntaxComment);
    return c;
}
void toConfiguration::setSyntaxComment(QColor v)
{
    m_syntaxComment = v.name();
}

QColor toConfiguration::syntaxNumber()
{
    QColor c;
    c.setNamedColor(m_syntaxNumber);
    return c;
}
void toConfiguration::setSyntaxNumber(QColor v)
{
    m_syntaxNumber = v.name();
}

QColor toConfiguration::syntaxKeyword()
{
    QColor c;
    c.setNamedColor(m_syntaxKeyword);
    return c;
}
void toConfiguration::setSyntaxKeyword(QColor v)
{
    m_syntaxKeyword = v.name();
}

QColor toConfiguration::syntaxString()
{
    QColor c;
    c.setNamedColor(m_syntaxString);
    return c;
}
void toConfiguration::setSyntaxString(QColor v)
{
    m_syntaxString = v.name();
}

QColor toConfiguration::syntaxDefaultBg()
{
    QColor c;
    c.setNamedColor(m_syntaxDefaultBg);
    return c;
}
void toConfiguration::setSyntaxDefaultBg(QColor v)
{
    m_syntaxDefaultBg = v.name();
}

QColor toConfiguration::syntaxDebugBg()
{
    QColor c;
    c.setNamedColor(m_syntaxDebugBg);
    return c;
}
void toConfiguration::setSyntaxDebugBg(QColor v)
{
    m_syntaxDebugBg = v.name();
}

QColor toConfiguration::syntaxErrorBg()
{
    QColor c;
    c.setNamedColor(m_syntaxErrorBg);
    return c;
}
void toConfiguration::setSyntaxErrorBg(QColor v)
{
    m_syntaxErrorBg = v.name();
}
