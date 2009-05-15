#include "toconfiguration.h"

// #include <qapplication.h>
// #include <qregexp.h>

#include "utils.h"

// A little magic to get lrefresh to work and get a check on qApp
#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

#include "toconf.h"

#include <QLocale>
#include <QSettings>
#include <QDir>
//#ifdef Q_OS_WIN32
//#include <windows.h>
//#include <shlobj.h>
//#endif


class toConfigurationPrivate
{
public:
    QString m_provider;
    bool    m_savePassword;
    QString m_defaultPassword;
    QString m_defaultDatabase;
    QString m_defaultUser;
    QString m_defaultHost;
    int     m_maxNumber;
    int     m_maxColSize;
    int     m_maxColDisp;
    QString m_planTable;
    QString m_planCheckpoint;
    QString m_textFont;
    QString m_codeFont;
    QString m_listFont;
    QString m_dateFormat;
    QString m_refresh;
    bool    m_highlight;
    bool    m_keywordUpper;
    QString m_pluginDir;
    QString m_cacheDir;
    bool    m_cacheDisk;
    QString m_sqlFile;
    int     m_statusMessage;
    bool    m_dbTitle;
    QString m_sizeUnit;
    QString m_helpPath;
    int     m_statusSave;
    bool    m_autoCommit;
    bool    m_changeConnection;
    int     m_connectSize;
    QString m_defaultTool;
    int     m_chartSamples;
    int     m_displaySamples;
    bool    m_codeCompletion;
    bool    m_completionSort;
    bool    m_autoIndent;
    bool    m_dontReread;
    int     m_autoLong;
    bool    m_messageStatusbar;
    bool    m_tabbedTools;
    int     m_objectCache;
    bool    m_bkgndConnect;
    bool    m_firewallMode;
    int     m_maxContent;
    bool    m_keepPlans;
    bool    m_vsqlPlans;
    bool    m_restoreSession;
    QString m_defaultSession;
    int     m_defaultFormat;
    bool    m_autoIndentRo;
    bool    m_indicateEmpty;
    QString m_indicateEmptyColor;
    QString m_extensions;
    int     m_recentMax;
    QString m_lastDir;
    QString m_locale;
    int     m_keepAlive;
    int     m_numberFormat;
    int     m_numberDecimals;
    int     m_cacheTimeout;
    int     m_tabStop;
    bool    m_tabSpaces;
    bool    m_editDragDrop;
    bool    m_toadBindings;
    bool    m_displayGridlines;
    QString m_smtp;
    int     m_smtpPort;
    QString m_style;

    // tooracleconnection
    int m_openCursors;
    int m_maxLong;

    // toqsqlconnection
    bool m_onlyForward;

    // main.cpp
    QString     m_lastVersion;
    QString     m_firstInstall;
    QByteArray  m_leftDockbarState;
    QByteArray  m_rightDockbarState;
    QStringList m_recentFiles;

    // tools
    ToolsMap m_tools;

    // toresultlistformat
    QString m_csvSeparator;
    QString m_csvDelimiter;

    // toeditextensions
    bool m_commaBefore;
    bool m_blockOpenLine;
    bool m_operatorSpace;
    bool m_rightSeparator;
    bool m_endBlockNewline;
    int  m_commentColumn;

    // tobrowser
    bool    m_filterIgnoreCase;
    bool    m_filterInvert;
    int     m_filterType;
    int     m_filterTablespaceType;
    QString m_filterText;

    // tohelp
    HelpsMap m_additionalHelp;

    // tochartmanager
    ChartsMap m_chartFiles;
    ChartsMap m_chartAlarms;

    // tooutput
    QString m_polling;
    int     m_logType;
    QString m_logUser;

    // torollback
    bool m_noExec;
    bool m_needRead;
    bool m_needTwo;
    bool m_alignLeft;
    bool m_oldEnable;

    // tosgatrace
    bool m_autoUpdate;

    // tostorage
    bool m_dispTablespaces;
    bool m_dispCoalesced;
    bool m_dispExtents;
    bool m_dispAvailableGraph;

    // totemplate
    TemplatesMap m_templates;

    // totuning
    /*! True if there are all features enabled.
      It's used when there is no m_tuningOverview defined in prefs. */
    bool m_tuningFirstRun;
    bool m_tuningOverview;
    bool m_tuningFileIO;
    bool m_tuningWaits;
    bool m_tuningCharts;

    // toworksheet
    bool m_wsAutoSave;
    bool m_wsCheckSave;
    QString m_wsAutoLoad;
    bool m_wsLogAtEnd;
    bool m_wsLogMulti;
    bool m_wsStatistics;
    bool m_wsTimedStats;
    bool m_wsNumber;
    bool m_wsMoveToErr;
    bool m_wsHistory;
    bool m_wsExecLog;
    bool m_wsToplevelDescribe;

    // tosyntaxsetup
    QString m_syntaxDefault;
    QString m_syntaxComment;
    QString m_syntaxNumber;
    QString m_syntaxKeyword;
    QString m_syntaxString;
    QString m_syntaxDefaultBg;
    QString m_syntaxErrorBg;
    QString m_syntaxDebugBg;
    QString m_syntaxCurrentLineMarker;
    bool    m_useMaxTextWidthMark;
    int     m_maxTextWidthMark;

    // mainwindow size
    QByteArray m_mainWindowGeometry;
    QByteArray m_mainWindowState;

    // shortcut editor
    bool               m_useEditorShortcuts;
    EditorShortcutsMap m_editorShortcuts;


    toConfigurationPrivate()
    {
        loadConfig();
    }

    /*! \brief Get directory name where to store session files.
      It's APPDATA on Windows, HOME on UNIX like systems
     */
    QString getSpecialDir()
    {
//#if defined(Q_OS_WIN32)
//        QString qstr;
//       char dir[256];
//        if ( SHGetSpecialFolderPath(NULL, dir, CSIDL_APPDATA, false) )
//       {
//            qstr = dir;
//            if( !qstr.endsWith("\\") )
//                qstr += "\\";
//            qstr.replace( '\\', '/' );
//       }
//        return qstr;
//#else
        return QDir::homePath() + "/";
//#endif
}



    void loadConfig()
    {
        int cnt; // tmp count holder for setting arrays
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
        m_sqlFile = s.value(CONF_SQL_FILE, getSpecialDir() + DEFAULT_SQL_FILE).toString();
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
        m_vsqlPlans = s.value(CONF_VSQL_PLANS, true).toBool();
        m_restoreSession = s.value(CONF_RESTORE_SESSION, false).toBool();
        m_defaultSession = s.value(CONF_DEFAULT_SESSION, getSpecialDir() + DEFAULT_SESSION).toString();
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
        m_syntaxCurrentLineMarker = s.value("SyntaxCurrentLineMarker", "whitesmoke").toString();
        m_useMaxTextWidthMark = s.value("useMaxTextWidthMark", false).toBool();
        m_maxTextWidthMark = s.value("maxTextWidthMark", 75).toInt();
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
    }


    void saveConfig()
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
        s.setValue(CONF_VSQL_PLANS, m_vsqlPlans);
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
        s.setValue("SyntaxCurrentLineMarker", m_syntaxCurrentLineMarker);
        s.setValue("useMaxTextWidthMark", m_useMaxTextWidthMark);
        s.setValue("maxTextWidthMark", m_maxTextWidthMark);
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
    }


    void loadMap(const QString &filename, std::map<QString, QString> &pairs)
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

        return;
    }


    bool saveMap(const QString &file, std::map<QString, QString> &pairs)
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
};


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
    setQSettingsEnv();
    p = new toConfigurationPrivate;
}

toConfiguration::~toConfiguration()
{
    delete p;
}

void toConfiguration::saveConfig()
{
    p->saveConfig();
}

void toConfiguration::loadMap(const QString &filename, std::map<QString, QString> &pairs)
{
    p->loadMap(filename, pairs);
}

bool toConfiguration::saveMap(const QString &file, std::map<QString, QString> &pairs)
{
    return p->saveMap(file, pairs);
}

QColor toConfiguration::syntaxDefault()
{
    QColor c;
    c.setNamedColor(p->m_syntaxDefault);
    return c;
}
void toConfiguration::setSyntaxDefault(QColor v)
{
    p->m_syntaxDefault = v.name();
}

QColor toConfiguration::syntaxComment()
{
    QColor c;
    c.setNamedColor(p->m_syntaxComment);
    return c;
}
void toConfiguration::setSyntaxComment(QColor v)
{
    p->m_syntaxComment = v.name();
}

QColor toConfiguration::syntaxNumber()
{
    QColor c;
    c.setNamedColor(p->m_syntaxNumber);
    return c;
}
void toConfiguration::setSyntaxNumber(QColor v)
{
    p->m_syntaxNumber = v.name();
}

QColor toConfiguration::syntaxKeyword()
{
    QColor c;
    c.setNamedColor(p->m_syntaxKeyword);
    return c;
}
void toConfiguration::setSyntaxKeyword(QColor v)
{
    p->m_syntaxKeyword = v.name();
}

QColor toConfiguration::syntaxString()
{
    QColor c;
    c.setNamedColor(p->m_syntaxString);
    return c;
}
void toConfiguration::setSyntaxString(QColor v)
{
    p->m_syntaxString = v.name();
}

QColor toConfiguration::syntaxDefaultBg()
{
    QColor c;
    c.setNamedColor(p->m_syntaxDefaultBg);
    return c;
}
void toConfiguration::setSyntaxDefaultBg(QColor v)
{
    p->m_syntaxDefaultBg = v.name();
}

QColor toConfiguration::syntaxDebugBg()
{
    QColor c;
    c.setNamedColor(p->m_syntaxDebugBg);
    return c;
}
void toConfiguration::setSyntaxDebugBg(QColor v)
{
    p->m_syntaxDebugBg = v.name();
}

QColor toConfiguration::syntaxErrorBg()
{
    QColor c;
    c.setNamedColor(p->m_syntaxErrorBg);
    return c;
}
void toConfiguration::setSyntaxErrorBg(QColor v)
{
    p->m_syntaxErrorBg = v.name();
}

QColor toConfiguration::syntaxCurrentLineMarker()
{
    QColor c;
    c.setNamedColor(p->m_syntaxCurrentLineMarker);
    return c;
}
void toConfiguration::setSyntaxCurrentLineMarker(QColor v)
{
    p->m_syntaxCurrentLineMarker = v.name();
}


QString toConfiguration::provider()
{
    return p->m_provider;
}
//   void setProvider(const QString & v) { p->m_provider = v; }

bool toConfiguration::savePassword()
{
    return p->m_savePassword;
}
void toConfiguration::setSavePassword(bool v)
{
    p->m_savePassword = v;
}

QString toConfiguration::defaultPassword()
{
    return p->m_defaultPassword;
}
void toConfiguration::setDefaultPassword(const QString & v)
{
    p->m_defaultPassword = v;
}

QString toConfiguration::defaultDatabase()
{
    return p->m_defaultDatabase;
}
void toConfiguration::setDefaultDatabase(const QString & v)
{
    p->m_defaultDatabase = v;
}

QString toConfiguration::defaultUser()
{
    return p->m_defaultUser;
}
void toConfiguration::setDefaultUser(const QString & v)
{
    p->m_defaultUser = v;
}

QString toConfiguration::defaultHost()
{
    return p->m_defaultHost;
}

int toConfiguration::maxNumber()
{
    return p->m_maxNumber;
}
void toConfiguration::setMaxNumber(int v)
{
    p->m_maxNumber = v;
}

int toConfiguration::maxColSize()
{
    return p->m_maxColSize;
}

int toConfiguration::maxColDisp()
{
    return p->m_maxColDisp;
}
void toConfiguration::setMaxColDisp(int v)
{
    p->m_maxColDisp = v;
}

QString toConfiguration::planTable()
{
    return p->m_planTable;
}
void toConfiguration::setPlanTable(const QString & v)
{
    p->m_planTable = v;
}

QString toConfiguration::planCheckpoint()
{
    return p->m_planCheckpoint;
}
void toConfiguration::setPlanCheckpoint(const QString & v)
{
    p->m_planCheckpoint = v;
}

QString toConfiguration::textFont()
{
    return p->m_textFont;
}
void toConfiguration::setTextFont(const QString & v)
{
    p->m_textFont = v;
}

QString toConfiguration::codeFont()
{
    return p->m_codeFont;
}
void toConfiguration::setCodeFont(const QString & v)
{
    p->m_codeFont = v;
}

QString toConfiguration::listFont()
{
    return p->m_listFont;
}
void toConfiguration::setListFont(const QString & v)
{
    p->m_listFont = v;
}

QString toConfiguration::dateFormat()
{
    return p->m_dateFormat;
}
void toConfiguration::setDateFormat(const QString & v)
{
    p->m_dateFormat = v;
}

QString toConfiguration::refresh()
{
    return p->m_refresh;
}
void toConfiguration::setRefresh(const QString & v)
{
    p->m_refresh = v;
}

bool toConfiguration::highlight()
{
    return p->m_highlight;
}
void toConfiguration::setHighlight(bool v)
{
    p->m_highlight = v;
}

bool toConfiguration::keywordUpper()
{
    return p->m_keywordUpper;
}
void toConfiguration::setKeywordUpper(bool v)
{
    p->m_keywordUpper = v;
}

QString toConfiguration::pluginDir()
{
    return p->m_pluginDir;
}
void toConfiguration::setPluginDir(const QString & v)
{
    p->m_pluginDir = v;
}

QString toConfiguration::cacheDir()
{
    return p->m_cacheDir;
}
void toConfiguration::setCacheDir(const QString & v)
{
    p->m_cacheDir = v;
}

bool toConfiguration::cacheDisk()
{
    return p->m_cacheDisk;
}
void toConfiguration::setCacheDisk(bool v)
{
    p->m_cacheDisk = v;
}

QString toConfiguration::sqlFile()
{
    return p->m_sqlFile;
}
void toConfiguration::setSqlFile(const QString & v)
{
    p->m_sqlFile = v;
}

int toConfiguration::statusMessage()
{
    return p->m_statusMessage;
}
void toConfiguration::setStatusMessage(int v)
{
    p->m_statusMessage = v;
}

bool toConfiguration::dbTitle()
{
    return p->m_dbTitle;
}
void toConfiguration::setDbTitle(bool v)
{
    p->m_dbTitle = v;
}

QString toConfiguration::sizeUnit()
{
    return p->m_sizeUnit;
}
void toConfiguration::setSizeUnit(const QString & v)
{
    p->m_sizeUnit = v;
}

QString toConfiguration::helpPath()
{
    return p->m_helpPath;
}
void toConfiguration::setHelpPath(const QString & v)
{
    p->m_helpPath = v;
}

int toConfiguration::statusSave()
{
    return p->m_statusSave;
}
void toConfiguration::setStatusSave(int v)
{
    p->m_statusSave = v;
}

bool toConfiguration::autoCommit()
{
    return p->m_autoCommit;
}
void toConfiguration::setAutoCommit(bool v)
{
    p->m_autoCommit = v;
}

bool toConfiguration::changeConnection()
{
    return p->m_changeConnection;
}
void toConfiguration::setChangeConnection(bool v)
{
    p->m_changeConnection = v;
}

int toConfiguration::connectSize()
{
    return p->m_connectSize;
}
void toConfiguration::setConnectSize(int v)
{
    p->m_connectSize = v;
}

QString toConfiguration::defaultTool()
{
    return p->m_defaultTool;
}
void toConfiguration::setDefaultTool(const QString & v)
{
    p->m_defaultTool = v;
}

int toConfiguration::chartSamples()
{
    return p->m_chartSamples;
}
void toConfiguration::setChartSamples(int v)
{
    p->m_chartSamples = v;
}

int toConfiguration::displaySamples()
{
    return p->m_displaySamples;
}
void toConfiguration::setDisplaySamples(int v)
{
    p->m_displaySamples = v;
}

bool toConfiguration::codeCompletion()
{
    return p->m_codeCompletion;
}
void toConfiguration::setCodeCompletion(bool v)
{
    p->m_codeCompletion = v;
}

bool toConfiguration::completionSort()
{
    return p->m_completionSort;
}
void toConfiguration::setCodeCompletionSort(bool v)
{
    p->m_completionSort = v;
}

bool toConfiguration::autoIndent()
{
    return p->m_autoIndent;
}
void toConfiguration::setAutoIndent(bool v)
{
    p->m_autoIndent = v;
}

bool toConfiguration::dontReread()
{
    return p->m_dontReread;
}
void toConfiguration::setDontReread(bool v)
{
    p->m_dontReread = v;
}

int toConfiguration::autoLong()
{
    return p->m_autoLong;
}
void toConfiguration::setAutoLong(int v)
{
    p->m_autoLong = v;
}

bool toConfiguration::messageStatusbar()
{
    return p->m_messageStatusbar;
}
void toConfiguration::setMessageStatusbar(bool v)
{
    p->m_messageStatusbar = v;
}

bool toConfiguration::tabbedTools()
{
    return p->m_tabbedTools;
}
void toConfiguration::setTabbedTools(bool v)
{
    p->m_tabbedTools = v;
}

int toConfiguration::objectCache()
{
    return p->m_objectCache;
}
void toConfiguration::setObjectCache(int v)
{
    p->m_objectCache = v;
}

bool toConfiguration::bkgndConnect()
{
    return p->m_bkgndConnect;
}
void toConfiguration::setBkgndConnect(bool v)
{
    p->m_bkgndConnect = v;
}

bool toConfiguration::firewallMode()
{
    return p->m_firewallMode;
}
void toConfiguration::setFirewallMode(bool v)
{
    p->m_firewallMode = v;
}

int toConfiguration::maxContent()
{
    return p->m_maxContent;
}
void toConfiguration::setMaxContent(int v)
{
    p->m_maxContent = v;
}

bool toConfiguration::keepPlans()
{
    return p->m_keepPlans;
}
void toConfiguration::setKeepPlans(bool v)
{
    p->m_keepPlans = v;
}

bool toConfiguration::vsqlPlans()
{
    return p->m_vsqlPlans;
}
void toConfiguration::setVsqlPlans(bool v)
{
    p->m_vsqlPlans = v;
}

bool toConfiguration::restoreSession()
{
    return p->m_restoreSession;
}
void toConfiguration::setRestoreSession(bool v)
{
    p->m_restoreSession = v;
}

QString toConfiguration::defaultSession()
{
    return p->m_defaultSession;
}
void toConfiguration::setDefaultSession(const QString & v)
{
    p->m_defaultSession = v;
}

int toConfiguration::defaultFormat()
{
    return p->m_defaultFormat;
}
void toConfiguration::setDefaultFormat(int v)
{
    p->m_defaultFormat = v;
}

bool toConfiguration::autoIndentRo()
{
    return p->m_autoIndentRo;
}
void toConfiguration::setAutoIndentRo(bool v)
{
    p->m_autoIndentRo = v;
}

bool toConfiguration::indicateEmpty()
{
    return p->m_indicateEmpty;
}
void toConfiguration::setIndicateEmpty(bool v)
{
    p->m_indicateEmpty = v;
}

QString toConfiguration::indicateEmptyColor()
{
    return p->m_indicateEmptyColor;
}
void toConfiguration::setIndicateEmptyColor(const QString & v)
{
    p->m_indicateEmptyColor = v;
}

QString toConfiguration::extensions()
{
    return p->m_extensions;
}
void toConfiguration::setExtensions(const QString & v)
{
    p->m_extensions = v;
}

int toConfiguration::recentMax()
{
    return p->m_recentMax;
}

QString toConfiguration::lastDir()
{
    return p->m_lastDir;
}
void toConfiguration::setLastDir(const QString & v)
{
    p->m_lastDir = v;
}

QString toConfiguration::locale()
{
    return p->m_locale;
}
void toConfiguration::setLocale(const QString & v)
{
    p->m_locale = v;
}

int toConfiguration::keepAlive()
{
    return p->m_keepAlive;
}
void toConfiguration::setKeepAlive(int v)
{
    p->m_keepAlive = v;
}

int toConfiguration::numberFormat()
{
    return p->m_numberFormat;
}
void toConfiguration::setNumberFormat(int v)
{
    p->m_numberFormat = v;
}

int toConfiguration::numberDecimals()
{
    return p->m_numberDecimals;
}
void toConfiguration::setNumberDecimals(int v)
{
    p->m_numberDecimals = v;
}

int toConfiguration::cacheTimeout()
{
    return p->m_cacheTimeout;
}

int toConfiguration::tabStop()
{
    return p->m_tabStop;
}
void toConfiguration::setTabStop(int v)
{
    p->m_tabStop = v;
}

bool toConfiguration::tabSpaces()
{
    return p->m_tabSpaces;
}
void toConfiguration::setTabSpaces(bool v)
{
    p->m_tabSpaces = v;
}

bool toConfiguration::editDragDrop()
{
    return p->m_editDragDrop;
}
void toConfiguration::setEditDragDrop(bool v)
{
    p->m_editDragDrop = v;
}

bool toConfiguration::toadBindings()
{
    return p->m_toadBindings;
}
void toConfiguration::setToadBindings(bool v)
{
    p->m_toadBindings = v;
}

bool toConfiguration::displayGridlines()
{
    return p->m_displayGridlines;
}
void toConfiguration::setDisplayGridlines(bool v)
{
    p->m_displayGridlines = v;
}

QString toConfiguration::smtp()
{
    return p->m_smtp;
}
void toConfiguration::setSmtp(const QString & v)
{
    p->m_smtp = v;
}

int toConfiguration::smtpPort()
{
    return p->m_smtpPort;
}
void toConfiguration::setSmtpPort(int v)
{
    p->m_smtpPort = v;
}

QString toConfiguration::style()
{
    return p->m_style;
}
void toConfiguration::setStyle(const QString & v)
{
    p->m_style = v;
}

// tooracleconnection
int toConfiguration::openCursors()
{
    return p->m_openCursors;
}
void toConfiguration::setOpenCursors(int v)
{
    p->m_openCursors = v;
}

int toConfiguration::maxLong()
{
    return p->m_maxLong;
}
void toConfiguration::setMaxLong(int v)
{
    p->m_maxLong = v;
}

// toqsqlconnection
bool toConfiguration::onlyForward()
{
    return p->m_onlyForward;
}
void toConfiguration::setOnlyForward(bool v)
{
    p->m_onlyForward = v;
}

// main.cpp
QString toConfiguration::lastVersion()
{
    return p->m_lastVersion;
}
void toConfiguration::setLastVersion(const QString & v)
{
    p->m_lastVersion = v;
}

QString toConfiguration::firstInstall()
{
    return p->m_firstInstall;
}
void toConfiguration::setFirstInstall(const QString & v)
{
    p->m_firstInstall = v;
}
// tomain.cpp
QStringList toConfiguration::recentFiles()
{
    return p->m_recentFiles;
}
void toConfiguration::setRecentFiles(QStringList v)
{
    p->m_recentFiles = v;
}
// tools
ToolsMap toConfiguration::tools()
{
    return p->m_tools;
}
void toConfiguration::setTools(ToolsMap v)
{
    p->m_tools = v;
}

// toresultlistformat
QString toConfiguration::csvSeparator()
{
    return p->m_csvSeparator;
}
QString toConfiguration::csvDelimiter()
{
    return p->m_csvDelimiter;
}

// toeditextensions
bool toConfiguration::commaBefore()
{
    return p->m_commaBefore;
}
void toConfiguration::setCommaBefore(bool v)
{
    p->m_commaBefore = v;
}

bool toConfiguration::blockOpenLine()
{
    return p->m_blockOpenLine;
}
void toConfiguration::setBlockOpenLine(bool v)
{
    p->m_blockOpenLine = v;
}

bool toConfiguration::operatorSpace()
{
    return p->m_operatorSpace;
}
void toConfiguration::setOperatorSpace(bool v)
{
    p->m_operatorSpace = v;
}

bool toConfiguration::rightSeparator()
{
    return p->m_rightSeparator;
}
void toConfiguration::setRightSeparator(bool v)
{
    p->m_rightSeparator = v;
}

bool toConfiguration::endBlockNewline()
{
    return p->m_endBlockNewline;
}
void toConfiguration::setEndBlockNewline(bool v)
{
    p->m_endBlockNewline = v;
}

int toConfiguration::commentColumn()
{
    return p->m_commentColumn;
}
void toConfiguration::setCommentColumn(int v)
{
    p->m_commentColumn = v;
}

// tobrowser
bool toConfiguration::filterIgnoreCase()
{
    return p->m_filterIgnoreCase;
}
void toConfiguration::setFilterIgnoreCase(bool v)
{
    p->m_filterIgnoreCase = v;
}

bool toConfiguration::filterInvert()
{
    return p->m_filterInvert;
}
void toConfiguration::setFilterInvert(bool v)
{
    p->m_filterInvert = v;
}

int toConfiguration::filterType()
{
    return p->m_filterType;
}
void toConfiguration::setFilterType(int v)
{
    p->m_filterType = v;
}

int toConfiguration::filterTablespaceType()
{
    return p->m_filterTablespaceType;
}
void toConfiguration::setFilterTablespaceType(int v)
{
    p->m_filterTablespaceType = v;
}

QString toConfiguration::filterText()
{
    return p->m_filterText;
}
void toConfiguration::setFilterText(const QString & v)
{
    p->m_filterText = v;
}

// tohelp
HelpsMap toConfiguration::additionalHelp()
{
    return p->m_additionalHelp;
}
void toConfiguration::setAdditionalHelp(HelpsMap v)
{
    p->m_additionalHelp = v;
}

// tochartmanager
ChartsMap toConfiguration::chartFiles()
{
    return p->m_chartFiles;
}
void toConfiguration::setChartFiles(ChartsMap v)
{
    p->m_chartFiles = v;
}

ChartsMap toConfiguration::chartAlarms()
{
    return p->m_chartAlarms;
}
void toConfiguration::setChartAlarms(ChartsMap v)
{
    p->m_chartAlarms = v;
}

// toooutput
QString toConfiguration::polling()
{
    return p->m_polling;
}
void toConfiguration::setPolling(const QString & v )
{
    p->m_polling = v;
}

int toConfiguration::logType()
{
    return p->m_logType;
}
void toConfiguration::setLogType(int v)
{
    p->m_logType = v;
}

QString toConfiguration::logUser()
{
    return p->m_logUser;
}
void toConfiguration::setLogUser(const QString & v)
{
    p->m_logUser = v;
}

// torollback
bool toConfiguration::noExec()
{
    return p->m_noExec;
}
void toConfiguration::setNoExec(bool v)
{
    p->m_noExec = v;
}

bool toConfiguration::needRead()
{
    return p->m_needRead;
}
void toConfiguration::setNeedRead(bool v)
{
    p->m_needRead = v;
}

bool toConfiguration::needTwo()
{
    return p->m_needTwo;
}
void toConfiguration::setNeedTwo(bool v)
{
    p->m_needTwo = v;
}

bool toConfiguration::alignLeft()
{
    return p->m_alignLeft;
}
void toConfiguration::setAlignLeft(bool v)
{
    p->m_alignLeft = v;
}

bool toConfiguration::oldEnable()
{
    return p->m_oldEnable;
}
void toConfiguration::setOldEnable(bool v)
{
    p->m_oldEnable = v;
}

// tosgatrace
bool toConfiguration::autoUpdate()
{
    return p->m_autoUpdate;
}
void toConfiguration::setAutoUpdate(bool v)
{
    p->m_autoUpdate = v;
}

// tostorage
bool toConfiguration::dispTablespaces()
{
    return p->m_dispTablespaces;
}
void toConfiguration::setDispTablespaces(bool v)
{
    p->m_dispTablespaces = v;
}

bool toConfiguration::dispCoalesced()
{
    return p->m_dispCoalesced;
}
void toConfiguration::setDispCoalesced(bool v)
{
    p->m_dispCoalesced = v;
}

bool toConfiguration::dispExtents()
{
    return p->m_dispExtents;
}
void toConfiguration::setDispExtents(bool v)
{
    p->m_dispExtents = v;
}

bool toConfiguration::dispAvailableGraph()
{
    return p->m_dispAvailableGraph;
}
void toConfiguration::setDispAvailableGraph(bool v)
{
    p->m_dispAvailableGraph = v;
}

// totemplate
TemplatesMap toConfiguration::templates()
{
    return p->m_templates;
}
void toConfiguration::setTemplates(TemplatesMap v)
{
    p->m_templates = v;
}

// totuning
/*! True if there are no prefs for tuning.
  It's used when there is no p->m_tuningOverview defined in prefs. */
bool toConfiguration::tuningFirstRun()
{
    return p->m_tuningFirstRun;
}

bool toConfiguration::tuningOverview()
{
    return p->m_tuningOverview;
}
void toConfiguration::setTuningOverview(bool v)
{
    p->m_tuningOverview = v;
}

bool toConfiguration::tuningFileIO()
{
    return p->m_tuningFileIO;
}
void toConfiguration::setTuningFileIO(bool v)
{
    p->m_tuningFileIO = v;
}

bool toConfiguration::tuningWaits()
{
    return p->m_tuningWaits;
}
void toConfiguration::setTuningWaits(bool v)
{
    p->m_tuningWaits = v;
}

bool toConfiguration::tuningCharts()
{
    return p->m_tuningCharts;
}
void toConfiguration::setTuningCharts(bool v)
{
    p->m_tuningCharts = v;
}

// toworksheet
bool toConfiguration::wsAutoSave()
{
    return p->m_wsAutoSave;
}
void toConfiguration::setWsAutoSave(bool v)
{
    p->m_wsAutoSave = v;
}

bool toConfiguration::wsCheckSave()
{
    return p->m_wsCheckSave;
}
void toConfiguration::setWsCheckSave(bool v)
{
    p->m_wsCheckSave = v;
}

QString toConfiguration::wsAutoLoad()
{
    return p->m_wsAutoLoad;
}
void toConfiguration::setWsAutoLoad(const QString & v)
{
    p->m_wsAutoLoad = v;
}

bool toConfiguration::wsLogAtEnd()
{
    return p->m_wsLogAtEnd;
}
void toConfiguration::setWsLogAtEnd(bool v)
{
    p->m_wsLogAtEnd = v;
}

bool toConfiguration::wsLogMulti()
{
    return p->m_wsLogMulti;
}
void toConfiguration::setWsLogMulti(bool v)
{
    p->m_wsLogMulti = v;
}

bool toConfiguration::wsStatistics()
{
    return p->m_wsStatistics;
}
void toConfiguration::setWsStatistics(bool v)
{
    p->m_wsStatistics = v;
}

bool toConfiguration::wsTimedStats()
{
    return p->m_wsTimedStats;
}
void toConfiguration::setWsTimedStats(bool v)
{
    p->m_wsTimedStats = v;
}

bool toConfiguration::wsNumber()
{
    return p->m_wsNumber;
}
void toConfiguration::setWsNumber(bool v)
{
    p->m_wsNumber = v;
}

bool toConfiguration::wsMoveToErr()
{
    return p->m_wsMoveToErr;
}
void toConfiguration::setWsMoveToErr(bool v)
{
    p->m_wsMoveToErr = v;
}

bool toConfiguration::wsHistory()
{
    return p->m_wsHistory;
}
void toConfiguration::setWsHistory(bool v)
{
    p->m_wsHistory = v;
}

bool toConfiguration::wsExecLog()
{
    return p->m_wsExecLog;
}
void toConfiguration::setWsExecLog(bool v)
{
    p->m_wsExecLog = v;
}

bool toConfiguration::wsToplevelDescribe()
{
    return p->m_wsToplevelDescribe;
}
void toConfiguration::setWsToplevelDescribe(bool v)
{
    p->m_wsToplevelDescribe = v;
}


bool toConfiguration::useEditorShortcuts()
{
    return p->m_useEditorShortcuts;
}

void toConfiguration::setUseEditorShortcuts(bool v)
{
    p->m_useEditorShortcuts = v;
}

EditorShortcutsMap toConfiguration::editorShortcuts()
{
    return p->m_editorShortcuts;
}

void toConfiguration::setEditorShortcuts(const EditorShortcutsMap & v)
{
    p->m_editorShortcuts = v;
}

bool toConfiguration::useMaxTextWidthMark()
{
    return p->m_useMaxTextWidthMark;
}

void toConfiguration::setUseMaxTextWidthMark(bool v)
{
    p->m_useMaxTextWidthMark = v;
}

int toConfiguration::maxTextWidthMark()
{
    return p->m_maxTextWidthMark;
}

void toConfiguration::setMaxTextWidthMark(int v)
{
    p->m_maxTextWidthMark = v;
}

QByteArray toConfiguration::mainWindowGeometry()
{
    return p->m_mainWindowGeometry;
}

void toConfiguration::setMainWindowGeometry(QByteArray v)
{
    p->m_mainWindowGeometry = v;
}

QByteArray toConfiguration::mainWindowState()
{
    return p->m_mainWindowState;
}

void toConfiguration::setMainWindowState(QByteArray v)
{
    p->m_mainWindowState = v;
}


void toConfiguration::setLeftDockbarState(QByteArray v)
{
    p->m_leftDockbarState = v;
}

QByteArray toConfiguration::leftDockbarState()
{
    return p->m_leftDockbarState;
}

void toConfiguration::setRightDockbarState(QByteArray v)
{
    p->m_rightDockbarState = v;
}

QByteArray toConfiguration::rightDockbarState()
{
    return p->m_rightDockbarState;
}
