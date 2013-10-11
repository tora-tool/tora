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
    int     m_cachedConnections;
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
    bool               m_useEditorShortcuts;
    EditorShortcutsMap m_editorShortcuts;
    bool    m_autoIndent;
    bool    m_useSpacesForIndent;


    int     m_maxNumber;
    int     m_maxColDisp;
    QString m_planTable;
    QString m_planCheckpoint;
    QString m_textFont;
    QString m_codeFont;
    QString m_listFont;
    QString m_dateFormat;
    QString m_timestampFormat;
    mutable QDir    m_applicationDir;

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
    QString m_syntaxStaticBg;

    bool    m_autoCommit;

    QString m_defaultTool;

    int     m_tabStop;

    bool    m_editDragDrop;



    bool    m_dontReread;

    toConfiguration::ObjectCache m_objectCache;
    bool    m_firewallMode;
    int     m_connTestInterval;
    int     m_maxContent;
    bool    m_keepPlans;
    bool    m_vsqlPlans;
    bool    m_sharedPlan;

    bool    m_autoIndentRo;
    bool    m_indicateEmpty;
    QString m_indicateEmptyColor;
    QString m_extensions;
    int     m_recentMax;
    QString m_lastDir;
    int     m_keepAlive;
    int     m_numberFormat;
    int     m_numberDecimals;
    int     m_cacheTimeout;

    int     m_connectionTestTimeout; // after how many seconds connection should be tested
    QString m_encoding;
    QString m_forcelineend; // force line end type (win, linux, mac) when saving text files

    // tooracleconnection
    int m_openCursors;
    int m_maxLong;

    // toqsqlconnection


    bool m_onlyForward;
    int  m_createAction;

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

    // mainwindow size
    QByteArray m_mainWindowGeometry;
    QByteArray m_mainWindowState;

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
