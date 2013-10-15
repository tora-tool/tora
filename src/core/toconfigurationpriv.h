#ifndef TOCONFIGURATION_PRIV_H
#define TOCONFIGURATION_PRIV_H

#include "core/tora_export.h"
#include "core/toconfiguration.h"
#include "core/utils.h"

#include <QtCore/QDir>
#include <QtCore/QVariant>

class toConfigurationPrivate : public QObject
{
	Q_OBJECT;
public:
	// Global Settings
	// Paths
    QString m_customSQL;
    QString m_helpDirectory;
    QString m_defaultSession;
    QString m_cacheDir;
    QString m_oracleHome;
    QString m_mysqlHome;
    QString m_pgsqlHome;
    // Options (1st column)
    bool    m_changeConnection;
    bool    m_savePassword;
    bool    m_dbTitle;
    bool    m_restoreSession;
    bool    m_toadBindings;
    bool    m_cacheDisk;
    bool    m_displayGridlines;
    bool    m_multiLineResults;
    bool    m_messageStatusbar;
    bool    m_colorizedConnections;
    ConnectionColors m_connectionColors;
    // Options (2nd column)
    int     m_statusMessage;
    int     m_historySize;
    int     m_chartSamples;
    int     m_displaySamples;
    QString m_sizeUnit;
    QString m_refresh;
    int     m_defaultFormat;
    QString m_style;
    QString m_translation;

    // Editor
    //  Options
    bool    m_syntaxHighlighting;
    bool    m_useMaxTextWidthMark;
    int     m_maxTextWidthMark;
    bool    m_keywordUpper;
    bool    m_objectNamesUpper;
    bool    m_codeCompletion;
    bool    m_completionSort;
    bool    m_useEditorShortcuts;
    EditorShortcutsMap m_editorShortcuts;
    bool    m_autoIndent;
    bool    m_useSpacesForIndent;
    int     m_tabStop;
    QString m_textFontName;
    QString m_codeFontName;
    QString m_listFontName;
    QString m_extensions;

    //  Syntax
    QString m_syntaxDefault;
    QString m_syntaxComment;
    QString m_syntaxNumber;
    QString m_syntaxKeyword;
    QString m_syntaxString;
    QString m_syntaxDefaultBg;
    QString m_syntaxErrorBg;
    QString m_syntaxDebugBg;
    QString m_syntaxCurrentLineMarker;
    QString m_syntaxStaticBg;

    // Database
    toConfiguration::ObjectCacheEnum m_objectCache;
    bool    m_autoCommit;
    bool    m_firewallMode;
    int     m_connTestInterval;
    int     m_cachedConnections;
    int     m_initialFetch;
    int     m_initialEditorContent;
    int     m_maxColDisp;
    bool    m_indicateEmpty;
    QString m_indicateEmptyColor;
    int     m_numberFormat;
    int     m_numberDecimals;
    // Database
    //  Oracle
    QString m_dateFormat;
    QString m_timestampFormat;
    int m_maxLong;
    QString m_planTable;
    bool    m_keepPlans;
    bool    m_vsqlPlans;
    bool    m_sharedPlan;
    // Extractor. Controls the method used to produce database object extracts
    // as well as extraction settings (which information should be extracted).
    bool m_extractorUseDbmsMetadata;
    bool m_extractorIncludeSotrage;
    bool m_extractorSkipOrgMonInformation;
    bool m_extractorSkipStorageExceptTablespaces;
    bool m_extractorIncludeParallel;
    bool m_extractorIncludePartition;
    bool m_extractorIncludeCode;
    bool m_extractorIncludeHeader;
    bool m_extractorIncludePrompt;

    // toEditExtensions (moved into obsolete ATM)
    bool    m_editDragDrop;
    bool    m_autoIndentRo;
    bool    m_commaBefore;
    bool    m_blockOpenLine;
    bool    m_operatorSpace;
    bool    m_rightSeparator;
    bool    m_endBlockNewline;
    int     m_commentColumn;

    // TODO these attributes can be useful but can not be set from GUI
    bool    m_dontReread;
    int     m_cacheTimeout;
    QString m_encoding;
    QString m_forcelineend; // force line end type (win, linux, mac) when saving text files

    // toqsqlconnection
    bool m_onlyForward;
    int  m_createAction;

    // main.cpp
    QString     m_lastVersion;
    QString     m_firstInstall;
    QByteArray  m_leftDockbarState;
    QByteArray  m_rightDockbarState;
    QStringList m_recentFiles;
    int         m_recentMax;
    mutable QDir    m_applicationDir;
    QString m_lastDir;

    // mainwindow size
    QByteArray m_mainWindowGeometry;
    QByteArray m_mainWindowState;

    // tools
    ToolsMap m_tools;
    QString m_defaultTool;

    // toresultlistformat
    QString m_csvSeparator;
    QString m_csvDelimiter;

    // tobrowser
    bool    m_filterIgnoreCase;
    bool    m_filterInvert;
    int     m_filterType;
    int     m_filterTablespaceType;
    QString m_filterText;

    // tohelp
    HelpsMap m_additionalHelp;

#ifdef TORA3_CHARTS
    // tochartmanager
    ChartsMap m_chartFiles;
    ChartsMap m_chartAlarms;
#endif

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

    toConfigurationPrivate(QObject *parent = 0);

    /*! \brief Get directory name where to store session files.
      It's APPDATA on Windows, HOME on UNIX like systems
     */
    QString getSpecialDir();

    void loadConfig();

    void saveConfig();

    void loadMap(const QString &filename, std::map<QString, QString> &pairs);
    bool saveMap(const QString &file, std::map<QString, QString> &pairs);
};

#endif
