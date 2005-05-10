/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#ifndef TOMAIN_H
#define TOMAIN_H

#include "config.h"
#include "tobackground.h"

#ifdef TO_KDE
#include "tomainwindow.kde.h"
#else
#include "tomainwindow.h"
#endif

#include <list>
#include <map>
#include <algorithm>

#include <qtimer.h>

#if QT_VERSION >= 0x030000
class QUrlOperator;
#endif

#define TOMessageBox QMessageBox

class QComboBox;
class QLabel;
class QListView;
class QListViewItem;
class QNetworkOperation;
class QPopupMenu;
class QToolBar;
class QToolButton;
class QWorkspace;
class toConnection;
class toEditWidget;
class toLineChart;
class toMarkedText;
class toSearchReplace;
class toTool;
class toToolWidget;

/** This class defines the main window. Observe that this class will have different baseclass
 * depending on if TOra is a Qt or KDE application. In the case of Qt this will be a
 * QMainWindow descendant.
 */
class toMain : public toMainWindow
{
    Q_OBJECT
private:
    /**
     * The connections that have been opened in TOra.
     */
    std::list<toConnection *> Connections;
    /**
     * Workspace of main window.
     */
    QWorkspace *Workspace;
    /**
     * Handles available connections list in toolbar.
     */
    QComboBox *ConnectionSelection;
    /**
     * Add a new connection. The connection itself must already be created.
     * Returns the connection or it's duplicate already opened connection.
     */
    toConnection *addConnection(toConnection *conn, bool def = true);
    /**
     * A map from menu ID:s to tools.
     */
    std::map<int, toTool *> Tools;
    /**
     * A list of toolbuttons that need an open connection to be enabled.
     */
    std::map<QToolButton *, bool> NeedConnection;
    /**
     * The ID of the tool providing the SQL editor.
     */
    int SQLEditor;

    /** Status messages to display
     */
    std::list<QString> StatusMessages;

    /**
     * Disconnect connection button.
     */
    QToolButton *DisconnectButton;
    /**
     * Load file button.
     */
    QToolButton *LoadButton;
    /**
     * Save file button.
     */
    QToolButton *SaveButton;
    /**
     * Print button.
     */
    QToolButton *PrintButton;
    /**
     * Undo button.
     */
    QToolButton *UndoButton;
    /**
     * Redo button.
     */
    QToolButton *RedoButton;
    /**
     * Cut button.
     */
    QToolButton *CutButton;
    /**
     * Copy button.
     */
    QToolButton *CopyButton;
    /**
     * Paste button.
     */
    QToolButton *PasteButton;
    /**
     * Paste button.
     */
    QToolButton *SearchButton;

    /**
     * Edit buttons toolbar.
     */
    QToolBar *EditToolbar;
    /**
     * Tool buttons toolbar.
     */
    QToolBar *ToolsToolbar;
    /**
     * Connection buttons toolbar.
     */
    QToolBar *ConnectionToolbar;

    /**
     * File menu.
     */
    QPopupMenu *FileMenu;
    /**
     * Edit menu.
     */
    QPopupMenu *EditMenu;
    /**
     * Tools menu.
     */
    QPopupMenu *ToolsMenu;
    /**
     * Windows menu.
     */
    QPopupMenu *WindowsMenu;
    /**
     * Help menu.
     */
    QPopupMenu *HelpMenu;
    /**
     * Display latest status messages
     */
    QPopupMenu *StatusMenu;
    /**
     * Current column label.
     */
    QLabel *ColumnLabel;
    /**
     * Current row label.
     */
    QLabel *RowLabel;
    toBackground Poll;
    /**
     * Search & replace dialog if available.
     */
    toSearchReplace *Search;
    /**
     * Default tool id
     */
    int DefaultTool;

    QTimer KeepAlive;

    toEditWidget *Edit;

    toEditWidget *findEdit(QWidget *edit);

    void updateRecent(void);

    void editEnable(toEditWidget *edit,
                    bool open, bool save, bool print,
                    bool undo, bool redo,
                    bool cut, bool copy, bool paste,
                    bool search,
                    bool selectAll, bool readAll);

    static const int TO_STATUS_ID = 4000;
    static const int TO_STATUS_ID_END = 4999;
    static const int TO_NEW_CONNECTION = 100;
    static const int TO_CLOSE_CONNECTION = 101;
    static const int TO_STOP_ALL = 102;
    static const int TO_FILE_OPEN = 103;
    static const int TO_FILE_SAVE = 104;
    static const int TO_FILE_SAVE_AS = 105;
    static const int TO_FILE_COMMIT = 106;
    static const int TO_FILE_ROLLBACK = 107;
    static const int TO_FILE_CURRENT = 108;
    static const int TO_FILE_CLEARCACHE = 109;
    static const int TO_FILE_PRINT = 110;
    static const int TO_FILE_QUIT = 111;
    static const int TO_FILE_OPEN_SESSION = 112;
    static const int TO_FILE_SAVE_SESSION = 113;
    static const int TO_FILE_CLOSE_SESSION = 114;
    static const int TO_FILE_LAST_SESSION = 115;

    static const int TO_EDIT_UNDO = 200;
    static const int TO_EDIT_REDO = 201;
    static const int TO_EDIT_CUT = 202;
    static const int TO_EDIT_COPY = 203;
    static const int TO_EDIT_PASTE = 204;
    static const int TO_EDIT_SELECT_ALL = 205;
    static const int TO_EDIT_OPTIONS = 206;
    static const int TO_EDIT_READ_ALL = 207;
    static const int TO_EDIT_SEARCH = 208;
    static const int TO_EDIT_SEARCH_NEXT = 209;  // Used by edit extension

    static const int TO_WINDOWS_TILE = 300;
    static const int TO_WINDOWS_CASCADE = 301;
    static const int TO_WINDOWS_CLOSE = 302;
    static const int TO_WINDOWS_CLOSE_ALL = 303;
    static const int TO_WINDOWS_WINDOWS = 310;
    static const int TO_WINDOWS_END = 399;

    static const int TO_HELP_CONTENTS = 900;
    static const int TO_HELP_CONTEXT = 901;
    static const int TO_HELP_ABOUT = 902;
    static const int TO_HELP_LICENSE = 903;
    static const int TO_HELP_QUOTES = 904;

    static const int TO_TOOLS = 1000;

public:

    /** ID of the file menu
     */
    static const int TO_FILE_MENU = 10;
    /** ID of the edit menu
     */
    static const int TO_EDIT_MENU = 20;
    /** ID of the tools menu
     */
    static const int TO_TOOLS_MENU = 30;
    /** ID of the windows menu
     */
    static const int TO_WINDOWS_MENU = 40;
    /** ID of the help menu
     */
    static const int TO_HELP_MENU = 50;

    /** First ID of the tool specific menu items.
     */
    static const int TO_TOOL_MENU_ID = 2000;
    /** Last ID of the tool specific menu items.
     */
    static const int TO_TOOL_MENU_ID_END = 2999;

    /** First ID of the tool specific menu items.
     */
    static const int TO_LAST_FILE_ID = 5000;
    /** Last ID of the tool specific menu items.
     */
    static const int TO_LAST_FILE_ID_END = 5999;

    /** First ID of the tool specific about menu items.
     */
    static const int TO_TOOL_ABOUT_ID = 3000;
    /** Last ID of the tool specific about menu items.
     */
    static const int TO_TOOL_ABOUT_ID_END = 3999;

    /**
     * Create main window. Always NULL as parent.
     */
    toMain();

    /**
     * Get the workspace widget of the main window.
     * @return Workspace widget.
     */
    QWorkspace *workspace()
    {
        return Workspace;
    }

    /**
     * Get the current database connection
     * @return Reference to current connection.
     */
    toConnection &currentConnection(void);

    /**
     * Set the widget to edit through menues and toolbar.
     */
    static void setEditWidget(toEditWidget *edit);
    /**
     * Set available menu items in user interface. It is not enough to just call
     * this function to make open for instance to actually work. It is a lot of
     * more especially in @ref commandCallback and @ref editFileMenu. Will only
     * update if this is the current editing widget.
     */
    static void editEnable(toEditWidget *edit);
    /**
     * Disable all the current available items in the user interface (That @ref
     * editEnable can enable). If specified widget has the focus the edit widget
     * is cleared.
     */
    static void editDisable(toEditWidget *edit);
    /** Get current edit widget
     */
    toEditWidget *editWidget()
    {
        return Edit;
    }

    /**
     * Close window
     * @param del If user interaction is allowed.
     * @return True if close was allowed, otherwise false.
     */
    virtual bool close(bool del);
    /**
     * Create the default tool for the current connection.
     *
     * This is the tool with the highest priority, usually the SQL worksheet.
     */
    void createDefault(void);
    /**
     * Set coordinates in the statusbar.
     *
     * Used to indicate current cursor position by child widgets.
     */
    void setCoordinates(int, int);

    /**
     * Get the file menu.
     * @return File menu
     */
    QPopupMenu *fileMenu()
    {
        return FileMenu;
    }
    /**
     * Get the edit menu.
     * @return Edit menu
     */
    QPopupMenu *editMenu()
    {
        return EditMenu;
    }
    /**
     * Get the tools menu.
     * @return Tools menu
     */
    QPopupMenu *toolsMenu()
    {
        return ToolsMenu;
    }
    /**
     * Get the help menu.
     * @return Help menu
     */
    QPopupMenu *helpMenu()
    {
        return HelpMenu;
    }

    /* Update the keepalive from configuration
     */
    void updateKeepAlive();

    /**
     * Get a list of currently open connections.
     * @return List of connection names. The returned list can then be used by
     * @ref connection to get the actual connection.
     */
    std::list<QString> connections(void);
    /** Get a connection identified by a string.
     * @return A reference to a connection.
     * @exception QString If connection wasn't found.
     */
    toConnection &connection(const QString &);
    /** Set if a connection needs to be commited. Also updates visual feedback in interface.
     */
    void setNeedCommit(toConnection &conn, bool needCommit = true);

    /** Edit an SQL statement in the SQL editor if any connected.
     * @param str Identifier of the SQL to edit.
     */
    void editSQL(const QString &str);
    /**
     * Register a tool which to use as an SQL editor. When something is to be edited
     * first a toolwindow will be created and the @ref sqlEditor will be emited, so
     * the SQL editor must connect to this signal as well.
     *
     * @param toolid Which tool identifier to register as the SQL editor. This
     *               is the value which is passed to the @ref toTool::customSetup
     *               member.
     */
    void registerSQLEditor(int toolid);

    /** Called by @ref toTool when a new tool is created.
     * @param tool The tool widget created.
     */
    void toolWidgetAdded(toToolWidget *tool);
    /** Called by @ref toToolWidget when a new tool is about to be destroyed.
     * @param tool The tool widget created.
     */
    void toolWidgetRemoved(toToolWidget *tool);

    /** Export data to a map.
     * @param data A map that can be used to recreate the session.
     * @param prefix Prefix to add to the map.
     */
    virtual void exportData(std::map<QCString, QString> &data, const QCString &prefix);
    /** Import data
     * @param data Data to read from a map.
     * @param prefix Prefix to read data from.
     */
    virtual void importData(std::map<QCString, QString> &data, const QCString &prefix);
    /** Get edit toolbar pointer.
     */
    QToolBar *editToolbar()
    {
        return EditToolbar;
    }
    /** Get connection toolbar pointer.
     */
    QToolBar *connectionToolbar()
    {
        return ConnectionToolbar;
    }
    /** Added chart.
     */
    void addChart(toLineChart *chart);
    /** Removed chart.
     */
    void removeChart(toLineChart *chart);
    /** Setup chart.
     */
    void setupChart(toLineChart *chart);
    /** Display status message in dialog.
     */
    void displayMessage(const QString &str);
    /** Add recent file
     */
    void addRecentFile(const QString &filename);
signals:
    /** Invoked to start editing an SQL identifier.
     * @param str Identifier to start editing.
     */
    void sqlEditor(const QString &str);
    /** Invoked when a new chart is created.
     */
    void chartAdded(toLineChart *chart);
    /** Invoked before a chart is destroyed.
     */
    void chartSetup(toLineChart *chart);
    /** Invoked before a chart is destroyed.
     */
    void chartRemoved(toLineChart *chart);
    /** Invoked when a connection is added.
     * @param str Connection identifier.
     */
    void addedConnection(const QString &str);
    /** Invoked when a connection is removed.
     * @param str Connection identifier.
     */
    void removedConnection(const QString &str);
    /** Invoked when a tool window is created.
     * @param tool The tool created.
     */
    void addedToolWidget(toToolWidget *tool);
    /** Invoked when a tool window is closed.
     * @param tool Tool about to be removed.
     */
    void removedToolWidget(toToolWidget *tool);
    /** Emitted before a commit or rollback is made to the current connection.
     * @param conn Connection that is commited
     * @param cmt True if commit, false if rollback.
     */
    void willCommit(toConnection &conn, bool cmt);
public slots:
    /** Used to enable/disable entries in the windows menu
     */
    void windowsMenu(void);
    /** Used to fill the status menu with the latest status entries
     */
    void statusMenu(void);
    /** Display context help.
     */
    void contextHelp(void);
    /** Called when active window is changed.
     */
    void windowActivated(QWidget *);
    /** Used to enable/disable entries in the file menu
     */
    void showFileMenu(void);
    /** Check if object caching is done.
     */
    void checkCaching(void);
    /** Save the current TOra session
     */
    void saveSession(void);
    /** Load a session (Merge with current)
     */
    void loadSession(void);
    /** Close all windows and connections.
     */
    void closeSession(void);
    /** Change current connection
     */
    void changeConnection(void);
private slots:
    /** Add a connection
     */
    void addConnection(void);
    /** Remove a connection
     */
    bool delConnection(void);
    /** Handles menu selections
     * @param cmd Menu entry ID selected
     */
    void commandCallback(int cmd);

    /** Commit button pressed
     */
    void commitButton(void);
    /** Rollback button pressed
     */
    void rollbackButton(void);

    /** Load button pressed
     */
    void loadButton(void);
    /** Save button pressed
     */
    void saveButton(void);
    /** Print button pressed
     */
    void printButton(void);
    /** Options button pressed
     */
    void optionButton(void);
    /** Undo button pressed
     */
    void undoButton(void);
    /** Redo button pressed
     */
    void redoButton(void);
    /** Copy button pressed
     */
    void copyButton(void);
    /** Cut button pressed
     */
    void cutButton(void);
    /** Paste button pressed
     */
    void pasteButton(void);
    /** Paste button pressed
     */
    void searchButton(void);
    /** Stop running queries on connection
     */
    void stopButton(void);
    /** Display status message
     */
    void displayMessage(void);

    void keepAlive();
};

/** Get a pointer to the main window
 * @return Pointer to main window.
 */
toMain *toMainWidget(void);

#endif
