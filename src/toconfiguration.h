#ifndef TOCONFIGURATION_H
#define TOCONFIGURATION_H

#include "config.h"

#include <loki/Singleton.h>
#include <QStringList>
#include <QMap>

#include <qapplication.h>


typedef QMap<QString, bool> ToolsMap;
typedef QMap<QString, QString> HelpsMap;
typedef QMapIterator<QString, QString> HelpsMapIterator;
typedef QMap<QString, QString> ChartsMap;
typedef QMapIterator<QString, QString> ChartsMapIterator;
typedef QMap<QString, QString> TemplatesMap;
typedef QMapIterator<QString, QString> TemplatesMapIterator;


class toConfiguration
{
public:
    toConfiguration();
    ~toConfiguration();

    /*! \brief Set the QSettings access strings.
    QSettings uses these. It's used in the main.cpp before
    new QApplication instance init.
    The 2nd usage is in the toConfiguration constructor due
    some strange loading -- values are ignored with empty
    QSettings constructor (maybe due the Loki lib?) */
    static void setQSettingsEnv();

    /**
     * Save configuration to file.
     */
    void saveConfig(void);
    /**
    * Load a string to string map from file saved by the @ref saveMap function.
    * @param filename Filename to load
    * @param map Reference to the map to fill with the new values.
    */
    void loadMap(const QString &filename, std::map<QString, QString> &map);

    /**
    * A map containing the available configuration settings. By convention the
    * character ':' is used to separate parts of the path.
    *
    * @see globalConfig
    * @see globalSetConfig
    * @see config
    * @see setConfig
    */
    bool saveMap(const QString &file, std::map<QString, QString> &pairs);

    QString provider()
    {
        return m_provider;
    };
//   void setProvider(const QString & v) { m_provider = v; };

    bool savePassword()
    {
        return m_savePassword;
    };
    void setSavePassword(bool v)
    {
        m_savePassword = v;
    };

    QString defaultPassword()
    {
        return m_defaultPassword;
    };
    void setDefaultPassword(const QString & v)
    {
        m_defaultPassword = v;
    };

    QString defaultDatabase()
    {
        return m_defaultDatabase;
    };
    void setDefaultDatabase(const QString & v)
    {
        m_defaultDatabase = v;
    };

    QString defaultUser()
    {
        return m_defaultUser;
    };
    void setDefaultUser(const QString & v)
    {
        m_defaultUser = v;
    };

    QString defaultHost()
    {
        return m_defaultHost;
    };

    int maxNumber()
    {
        return m_maxNumber;
    };
    void setMaxNumber(int v)
    {
        m_maxNumber = v;
    };

    int maxColSize()
    {
        return m_maxColSize;
    };

    int maxColDisp()
    {
        return m_maxColDisp;
    };
    void setMaxColDisp(int v)
    {
        m_maxColDisp = v;
    };

    QString planTable()
    {
        return m_planTable;
    };
    void setPlanTable(const QString & v)
    {
        m_planTable = v;
    };

    QString planCheckpoint()
    {
        return m_planCheckpoint;
    };
    void setPlanCheckpoint(const QString & v)
    {
        m_planCheckpoint = v;
    };

    QString textFont()
    {
        return m_textFont;
    };
    void setTextFont(const QString & v)
    {
        m_textFont = v;
    };

    QString codeFont()
    {
        return m_codeFont;
    };
    void setCodeFont(const QString & v)
    {
        m_codeFont = v;
    };

    QString listFont()
    {
        return m_listFont;
    };
    void setListFont(const QString & v)
    {
        m_listFont = v;
    };

    QString dateFormat()
    {
        return m_dateFormat;
    };
    void setDateFormat(const QString & v)
    {
        m_dateFormat = v;
    };

    QString refresh()
    {
        return m_refresh;
    };
    void setRefresh(const QString & v)
    {
        m_refresh = v;
    };

    bool highlight()
    {
        return m_highlight;
    };
    void setHighlight(bool v)
    {
        m_highlight = v;
    };

    bool keywordUpper()
    {
        return m_keywordUpper;
    };
    void setKeywordUpper(bool v)
    {
        m_keywordUpper = v;
    };

    QString pluginDir()
    {
        return m_pluginDir;
    };
    void setPluginDir(const QString & v)
    {
        m_pluginDir = v;
    };

    QString cacheDir()
    {
        return m_cacheDir;
    };
    void setCacheDir(const QString & v)
    {
        m_cacheDir = v;
    };

    bool cacheDisk()
    {
        return m_cacheDisk;
    };
    void setCacheDisk(bool v)
    {
        m_cacheDisk = v;
    };

    QString sqlFile()
    {
        return m_sqlFile;
    };
    void setSqlFile(const QString & v)
    {
        m_sqlFile = v;
    };

    int statusMessage()
    {
        return m_statusMessage;
    };
    void setStatusMessage(int v)
    {
        m_statusMessage = v;
    };

    bool dbTitle()
    {
        return m_dbTitle;
    };
    void setDbTitle(bool v)
    {
        m_dbTitle = v;
    };

    QString sizeUnit()
    {
        return m_sizeUnit;
    };
    void setSizeUnit(const QString & v)
    {
        m_sizeUnit = v;
    };

    QString helpPath()
    {
        return m_helpPath;
    };
    void setHelpPath(const QString & v)
    {
        m_helpPath = v;
    };

    int statusSave()
    {
        return m_statusSave;
    };
    void setStatusSave(int v)
    {
        m_statusSave = v;
    };

    bool autoCommit()
    {
        return m_autoCommit;
    };
    void setAutoCommit(bool v)
    {
        m_autoCommit = v;
    };

    bool changeConnection()
    {
        return m_changeConnection;
    };
    void setChangeConnection(bool v)
    {
        m_changeConnection = v;
    };

    int connectSize()
    {
        return m_connectSize;
    };
    void setConnectSize(int v)
    {
        m_connectSize = v;
    };

    QString defaultTool()
    {
        return m_defaultTool;
    };
    void setDefaultTool(const QString & v)
    {
        m_defaultTool = v;
    };

    int chartSamples()
    {
        return m_chartSamples;
    };
    void setChartSamples(int v)
    {
        m_chartSamples = v;
    };

    int displaySamples()
    {
        return m_displaySamples;
    };
    void setDisplaySamples(int v)
    {
        m_displaySamples = v;
    };

    bool codeCompletion()
    {
        return m_codeCompletion;
    };
    void setCodeCompletion(bool v)
    {
        m_codeCompletion = v;
    };

    bool completionSort()
    {
        return m_completionSort;
    };
    void setCodeCompletionSort(bool v)
    {
        m_completionSort = v;
    };

    bool autoIndent()
    {
        return m_autoIndent;
    };
    void setAutoIndent(bool v)
    {
        m_autoIndent = v;
    };

    bool dontReread()
    {
        return m_dontReread;
    };
    void setDontReread(bool v)
    {
        m_dontReread = v;
    };

    int autoLong()
    {
        return m_autoLong;
    };
    void setAutoLong(int v)
    {
        m_autoLong = v;
    };

    bool messageStatusbar()
    {
        return m_messageStatusbar;
    };
    void setMessageStatusbar(bool v)
    {
        m_messageStatusbar = v;
    };

    bool tabbedTools()
    {
        return m_tabbedTools;
    };
    void setTabbedTools(bool v)
    {
        m_tabbedTools = v;
    };

    int objectCache()
    {
        return m_objectCache;
    };
    void setObjectCache(int v)
    {
        m_objectCache = v;
    };

    bool bkgndConnect()
    {
        return m_bkgndConnect;
    };
    void setBkgndConnect(bool v)
    {
        m_bkgndConnect = v;
    };

    bool firewallMode()
    {
        return m_firewallMode;
    }
    void setFirewallMode(bool v)
    {
        m_firewallMode = v;
    }

    int maxContent()
    {
        return m_maxContent;
    };
    void setMaxContent(int v)
    {
        m_maxContent = v;
    };

    bool keepPlans()
    {
        return m_keepPlans;
    };
    void setKeepPlans(bool v)
    {
        m_keepPlans = v;
    };

    bool restoreSession()
    {
        return m_restoreSession;
    };
    void setRestoreSession(bool v)
    {
        m_restoreSession = v;
    };

    QString defaultSession()
    {
        return m_defaultSession;
    };
    void setDefaultSession(const QString & v)
    {
        m_defaultSession = v;
    };

    int defaultFormat()
    {
        return m_defaultFormat;
    };
    void setDefaultFormat(int v)
    {
        m_defaultFormat = v;
    };

    bool autoIndentRo()
    {
        return m_autoIndentRo;
    };
    void setAutoIndentRo(bool v)
    {
        m_autoIndentRo = v;
    };

    bool indicateEmpty()
    {
        return m_indicateEmpty;
    };
    void setIndicateEmpty(bool v)
    {
        m_indicateEmpty = v;
    };

    QString indicateEmptyColor()
    {
        return m_indicateEmptyColor;
    };
    void setIndicateEmptyColor(const QString & v)
    {
        m_indicateEmptyColor = v;
    };

    QString extensions()
    {
        return m_extensions;
    };
    void setExtensions(const QString & v)
    {
        m_extensions = v;
    };

    int recentMax()
    {
        return m_recentMax;
    };

    QString lastDir()
    {
        return m_lastDir;
    };
    void setLastDir(const QString & v)
    {
        m_lastDir = v;
    };

    QString locale()
    {
        return m_locale;
    };
    void setLocale(const QString & v)
    {
        m_locale = v;
    };

    int keepAlive()
    {
        return m_keepAlive;
    };
    void setKeepAlive(int v)
    {
        m_keepAlive = v;
    };

    int numberFormat()
    {
        return m_numberFormat;
    };
    void setNumberFormat(int v)
    {
        m_numberFormat = v;
    };

    int numberDecimals()
    {
        return m_numberDecimals;
    };
    void setNumberDecimals(int v)
    {
        m_numberDecimals = v;
    };

    int cacheTimeout()
    {
        return m_cacheTimeout;
    };

    int tabStop()
    {
        return m_tabStop;
    };
    void setTabStop(int v)
    {
        m_tabStop = v;
    };

    bool tabSpaces()
    {
        return m_tabSpaces;
    };
    void setTabSpaces(bool v)
    {
        m_tabSpaces = v;
    };

    bool editDragDrop()
    {
        return m_editDragDrop;
    };
    void setEditDragDrop(bool v)
    {
        m_editDragDrop = v;
    };

    bool toadBindings()
    {
        return m_toadBindings;
    };
    void setToadBindings(bool v)
    {
        m_toadBindings = v;
    };

    bool displayGridlines()
    {
        return m_displayGridlines;
    };
    void setDisplayGridlines(bool v)
    {
        m_displayGridlines = v;
    };

    QString smtp()
    {
        return m_smtp;
    };
    void setSmtp(const QString & v)
    {
        m_smtp = v;
    };

    int smtpPort()
    {
        return m_smtpPort;
    };
    void setSmtpPort(int v)
    {
        m_smtpPort = v;
    };

    QString style()
    {
        return m_style;
    };
    void setStyle(const QString & v)
    {
        m_style = v;
    };

    // tooracleconnection
    int openCursors()
    {
        return m_openCursors;
    };
    void setOpenCursors(int v)
    {
        m_openCursors = v;
    }

    int maxLong()
    {
        return m_maxLong;
    };
    void setMaxLong(int v)
    {
        m_maxLong = v;
    };

    // toqsqlconnection
    bool onlyForward()
    {
        return m_onlyForward;
    };
    void setOnlyForward(bool v)
    {
        m_onlyForward = v;
    };

    // main.cpp
    QString lastVersion()
    {
        return m_lastVersion;
    };
    void setLastVersion(const QString & v)
    {
        m_lastVersion = v;
    };

    QString firstInstall()
    {
        return m_firstInstall;
    };
    void setFirstInstall(const QString & v)
    {
        m_firstInstall = v;
    };
    // tomain.cpp
    QStringList recentFiles()
    {
        return m_recentFiles;
    };
    void setRecentFiles(QStringList v)
    {
        m_recentFiles = v;
    };
    // tools
    ToolsMap tools()
    {
        return m_tools;
    };
    void setTools(ToolsMap v)
    {
        m_tools = v;
    };

    // toresultlistformat
    QString csvSeparator()
    {
        return m_csvSeparator;
    };
    QString csvDelimiter()
    {
        return m_csvDelimiter;
    };

    // toeditextensions
    bool commaBefore()
    {
        return m_commaBefore;
    };
    void setCommaBefore(bool v)
    {
        m_commaBefore = v;
    };

    bool blockOpenLine()
    {
        return m_blockOpenLine;
    };
    void setBlockOpenLine(bool v)
    {
        m_blockOpenLine = v;
    };

    bool operatorSpace()
    {
        return m_operatorSpace;
    };
    void setOperatorSpace(bool v)
    {
        m_operatorSpace = v;
    };

    bool rightSeparator()
    {
        return m_rightSeparator;
    };
    void setRightSeparator(bool v)
    {
        m_rightSeparator = v;
    };

    bool endBlockNewline()
    {
        return m_endBlockNewline;
    };
    void setEndBlockNewline(bool v)
    {
        m_endBlockNewline = v;
    };

    int commentColumn()
    {
        return m_commentColumn;
    };
    void setCommentColumn(int v)
    {
        m_commentColumn = v;
    };

    // tobrowser
    bool filterIgnoreCase()
    {
        return m_filterIgnoreCase;
    };
    void setFilterIgnoreCase(bool v)
    {
        m_filterIgnoreCase = v;
    };

    bool filterInvert()
    {
        return m_filterInvert;
    };
    void setFilterInvert(bool v)
    {
        m_filterInvert = v;
    };

    int filterType()
    {
        return m_filterType;
    };
    void setFilterType(int v)
    {
        m_filterType = v;
    };

    int filterTablespaceType()
    {
        return m_filterTablespaceType;
    };
    void setFilterTablespaceType(int v)
    {
        m_filterTablespaceType = v;
    };

    QString filterText()
    {
        return m_filterText;
    };
    void setFilterText(const QString & v)
    {
        m_filterText = v;
    };

    // tohelp
    HelpsMap additionalHelp()
    {
        return m_additionalHelp;
    };
    void setAdditionalHelp(HelpsMap v)
    {
        m_additionalHelp = v;
    };

    // tochartmanager
    ChartsMap chartFiles()
    {
        return m_chartFiles;
    };
    void setChartFiles(ChartsMap v)
    {
        m_chartFiles = v;
    };

    ChartsMap chartAlarms()
    {
        return m_chartAlarms;
    };
    void setChartAlarms(ChartsMap v)
    {
        m_chartAlarms = v;
    };

    // toooutput
    QString polling()
    {
        return m_polling;
    };
    void setPolling(const QString & v )
    {
        m_polling = v;
    };

    int logType()
    {
        return m_logType;
    };
    void setLogType(int v)
    {
        m_logType = v;
    };

    QString logUser()
    {
        return m_logUser;
    };
    void setLogUser(const QString & v)
    {
        m_logUser = v;
    };

    // torollback
    bool noExec()
    {
        return m_noExec;
    };
    void setNoExec(bool v)
    {
        m_noExec = v;
    };

    bool needRead()
    {
        return m_needRead;
    };
    void setNeedRead(bool v)
    {
        m_needRead = v;
    };

    bool needTwo()
    {
        return m_needTwo;
    };
    void setNeedTwo(bool v)
    {
        m_needTwo = v;
    };

    bool alignLeft()
    {
        return m_alignLeft;
    };
    void setAlignLeft(bool v)
    {
        m_alignLeft = v;
    };

    bool oldEnable()
    {
        return m_oldEnable;
    };
    void setOldEnable(bool v)
    {
        m_oldEnable = v;
    };

    // tosgatrace
    bool autoUpdate()
    {
        return m_autoUpdate;
    };
    void setAutoUpdate(bool v)
    {
        m_autoUpdate = v;
    };

    // tostorage
    bool dispTablespaces()
    {
        return m_dispTablespaces;
    };
    void setDispTablespaces(bool v)
    {
        m_dispTablespaces = v;
    };

    bool dispCoalesced()
    {
        return m_dispCoalesced;
    };
    void setDispCoalesced(bool v)
    {
        m_dispCoalesced = v;
    };

    bool dispExtents()
    {
        return m_dispExtents;
    };
    void setDispExtents(bool v)
    {
        m_dispExtents = v;
    };

    bool dispAvailableGraph()
    {
        return m_dispAvailableGraph;
    };
    void setDispAvailableGraph(bool v)
    {
        m_dispAvailableGraph = v;
    };

    // totemplate
    TemplatesMap templates()
    {
        return m_templates;
    };
    void setTemplates(TemplatesMap v)
    {
        m_templates = v;
    };

    // totuning
    /*! True if there are no prefs for tuning.
    It's used when there is no m_tuningOverview defined in prefs. */
    bool tuningFirstRun()
    {
        return m_tuningFirstRun;
    };

    bool tuningOverview()
    {
        return m_tuningOverview;
    };
    void setTuningOverview(bool v)
    {
        m_tuningOverview = v;
    };

    bool tuningFileIO()
    {
        return m_tuningFileIO;
    };
    void setTuningFileIO(bool v)
    {
        m_tuningFileIO = v;
    };

    bool tuningWaits()
    {
        return m_tuningWaits;
    };
    void setTuningWaits(bool v)
    {
        m_tuningWaits = v;
    };

    bool tuningCharts()
    {
        return m_tuningCharts;
    };
    void setTuningCharts(bool v)
    {
        m_tuningCharts = v;
    };

    // toworksheet
    bool wsAutoSave()
    {
        return m_wsAutoSave;
    };
    void setWsAutoSave(bool v)
    {
        m_wsAutoSave = v;
    };

    bool wsCheckSave()
    {
        return m_wsCheckSave;
    };
    void setWsCheckSave(bool v)
    {
        m_wsCheckSave = v;
    };

    QString wsAutoLoad()
    {
        return m_wsAutoLoad;
    };
    void setWsAutoLoad(const QString & v)
    {
        m_wsAutoLoad = v;
    };

    bool wsLogAtEnd()
    {
        return m_wsLogAtEnd;
    };
    void setWsLogAtEnd(bool v)
    {
        m_wsLogAtEnd = v;
    };

    bool wsLogMulti()
    {
        return m_wsLogMulti;
    };
    void setWsLogMulti(bool v)
    {
        m_wsLogMulti = v;
    };

    bool wsStatistics()
    {
        return m_wsStatistics;
    };
    void setWsStatistics(bool v)
    {
        m_wsStatistics = v;
    };

    bool wsTimedStats()
    {
        return m_wsTimedStats;
    };
    void setWsTimedStats(bool v)
    {
        m_wsTimedStats = v;
    };

    bool wsNumber()
    {
        return m_wsNumber;
    };
    void setWsNumber(bool v)
    {
        m_wsNumber = v;
    };

    bool wsMoveToErr()
    {
        return m_wsMoveToErr;
    };
    void setWsMoveToErr(bool v)
    {
        m_wsMoveToErr = v;
    };

    bool wsHistory()
    {
        return m_wsHistory;
    };
    void setWsHistory(bool v)
    {
        m_wsHistory = v;
    };

    bool wsExecLog()
    {
        return m_wsExecLog;
    };
    void setWsExecLog(bool v)
    {
        m_wsExecLog = v;
    };

    bool wsToplevelDescribe()
    {
        return m_wsToplevelDescribe;
    };
    void setWsToplevelDescribe(bool v)
    {
        m_wsToplevelDescribe = v;
    };

    // tosyntaxsetup
    QColor syntaxDefault();
    void setSyntaxDefault(QColor v);
    QColor syntaxComment();
    void setSyntaxComment(QColor v);
    QColor syntaxNumber();
    void setSyntaxNumber(QColor v);
    QColor syntaxKeyword();
    void setSyntaxKeyword(QColor v);
    QColor syntaxString();
    void setSyntaxString(QColor v);
    QColor syntaxDefaultBg();
    void setSyntaxDefaultBg(QColor v);
    QColor syntaxDebugBg();
    void setSyntaxDebugBg(QColor v);
    QColor syntaxErrorBg();
    void setSyntaxErrorBg(QColor v);

    // main window size
    QByteArray mainWindowGeometry() { return m_mainWindowGeometry; };
    void setMainWindowGeometry(QByteArray v) { m_mainWindowGeometry = v; };
    // mainwindow's toolbars etc
    QByteArray mainWindowState() { return m_mainWindowState; };
    void setMainWindowState(QByteArray v) { m_mainWindowState = v; };

private:
    QString m_provider;
    bool m_savePassword;
    QString m_defaultPassword;
    QString m_defaultDatabase;
    QString m_defaultUser;
    QString m_defaultHost;
    int m_maxNumber;
    int m_maxColSize;
    int m_maxColDisp;
    QString m_planTable;
    QString m_planCheckpoint;
    QString m_textFont;
    QString m_codeFont;
    QString m_listFont;
    QString m_dateFormat;
    QString m_refresh;
    bool m_highlight;
    bool m_keywordUpper;
    QString m_pluginDir;
    QString m_cacheDir;
    bool m_cacheDisk;
    QString m_sqlFile;
    int m_statusMessage;
    bool m_dbTitle;
    QString m_sizeUnit;
    QString m_helpPath;
    int m_statusSave;
    bool m_autoCommit;
    bool m_changeConnection;
    int m_connectSize;
    QString m_defaultTool;
    int m_chartSamples;
    int m_displaySamples;
    bool m_codeCompletion;
    bool m_completionSort;
    bool m_autoIndent;
    bool m_dontReread;
    int m_autoLong;
    bool m_messageStatusbar;
    bool m_tabbedTools;
    int m_objectCache;
    bool m_bkgndConnect;
    bool m_firewallMode;
    int m_maxContent;
    bool m_keepPlans;
    bool m_restoreSession;
    QString m_defaultSession;
    int m_defaultFormat;
    bool m_autoIndentRo;
    bool m_indicateEmpty;
    QString m_indicateEmptyColor;
    QString m_extensions;
    int m_recentMax;
    QString m_lastDir;
    QString m_locale;
    int m_keepAlive;
    int m_numberFormat;
    int m_numberDecimals;
    int m_cacheTimeout;
    int m_tabStop;
    bool m_tabSpaces;
    bool m_editDragDrop;
    bool m_toadBindings;
    bool m_displayGridlines;
    QString m_smtp;
    int m_smtpPort;
    QString m_style;
    // tooracleconnection
    int m_openCursors;
    int m_maxLong;
    // toqsqlconnection
    bool m_onlyForward;
    // main.cpp
    QString m_lastVersion;
    QString m_firstInstall;
    // tomain.cpp
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
    int m_commentColumn;

    // tobrowser
    bool m_filterIgnoreCase;
    bool m_filterInvert;
    int m_filterType;
    int m_filterTablespaceType;
    QString m_filterText;

    // tohelp
    HelpsMap m_additionalHelp;

    // tochartmanager
    ChartsMap m_chartFiles;
    ChartsMap m_chartAlarms;

    // tooutput
    QString m_polling;
    int m_logType;
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

    // mainwindow size
    QByteArray m_mainWindowGeometry;
    QByteArray m_mainWindowState;
};

typedef Loki::SingletonHolder<toConfiguration> toConfigurationSingle;

#endif
