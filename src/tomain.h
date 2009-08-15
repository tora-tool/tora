
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

#ifndef TOMAIN_H
#define TOMAIN_H

#include "config.h"
#include "tobackground.h"
#include "tobackuptool.h"

#include "tomainwindow.h"

#include <list>
#include <map>
#include <algorithm>

#include <qtimer.h>

#include <QLabel>
#include <QPointer>

#define TOMessageBox QMessageBox

class QComboBox;
class QLabel;
class toTreeWidget;
class toTreeWidgetItem;
class QToolBar;
class QToolButton;
class QMdiArea;
class QMdiSubWindow;
class toBackgroundLabel;
class toConnection;
class toEditWidget;
class toLineChart;
class toMarkedText;
class toSearchReplace;
class toTool;
class toToolWidget;
class toMessage;
class toDocklet;
class toDockbar;


/** This class defines the main window. Observe that this class will have different baseclass
 * depending on if TOra is a Qt or KDE application. In the case of Qt this will be a
 * QMainWindow descendant.
 */
class toMain : public toMainWindow
{
    Q_OBJECT;

private:
    std::auto_ptr<toBackupTool> toBackupTool_;
    /**
     * The connections that have been opened in TOra.
     */
    std::list<toConnection *> Connections;
    /**
     * Workspace of main window - MDI area.
     */
    QMdiArea *Workspace;
    /**
     * The last active sub window in workspace
     */
    QPointer<QMdiSubWindow> LastActiveWindow;
    /**
     * Handles available connections list in toolbar.
     */
    QComboBox *ConnectionSelection;
    /**
     * The ID of the tool providing the SQL editor.
     */
    QString SQLEditor;

    /**
     * A reference to the error dialog.
     */
    toMessage * Message;

    /**
     * Status message storage for status bar
     */
    std::list<QString> StatusMessages;

    /**
     * Edit buttons toolbar.
     */
    QToolBar *editToolbar;
    /**
     * Tool buttons toolbar.
     */
    QToolBar *toolsToolbar;
    /**
     * Connection buttons toolbar.
     */
    QToolBar *connectionToolbar;

    /**
     * Toolbar for minimizing docklets, left
     *
     */
    toDockbar *leftDockbar;

    /**
     * Toolbar for minimizing docklets, right
     *
     */
    toDockbar *rightDockbar;

    /**
     * File menu.
     */
    QMenu *fileMenu;
    /**
     * Recent files menu (child of fileMenu).
     */
    QMenu *recentMenu;
    /**
     * Edit menu.
     */
    QMenu *editMenu;
    /**
     * View menu.
     */
    QMenu *viewMenu;
    /**
     * Tools menu.
     */
    QMenu *toolsMenu;
    /**
     * Windows menu.
     */
    QMenu *windowsMenu;
    /**
     * Help menu.
     */
    QMenu *helpMenu;
    /**
     * Display latest status messages
     */
    QMenu *statusMenu;
    /**
     * Current column label.
     */
    QLabel *ColumnLabel;
    /**
     * Current row label.
     */
    QLabel *RowLabel;

    //! \brief Display current type of text edit selection (normal/block)
    QLabel * SelectionLabel;

    toBackground Poll;
    /**
     * Search & replace dialog if available.
     */
    toSearchReplace *Search;

    toBackgroundLabel* BackgroundLabel;

    toEditWidget *Edit;

    toEditWidget *findEdit(QWidget *edit);

    void updateRecent(void);

    /**
     * enable or disable buttons that need an active connection
     *
     */
    void enableConnectionActions(bool enabled);

    void editEnable(toEditWidget *edit,
                    bool open,
                    bool save,
                    bool print,
                    bool undo,
                    bool redo,
                    bool cut,
                    bool copy,
                    bool paste,
                    bool search,
                    bool selectAll,
                    bool readAll);

    QAction *newConnAct;
    QAction *closeConn;
    QAction *commitAct;
    QAction *rollbackAct;
    QAction *currentAct;
    QAction *stopAct;
    QAction *refreshAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *openSessionAct;
    QAction *saveSessionAct;
    QAction *restoreSessionAct;
    QAction *closeSessionAct;
    QAction *printAct;
    QAction *quitAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *searchReplaceAct;
    QAction *searchNextAct;
    QAction *selectAllAct;
#if 0
// TODO: this part is waiting for QScintilla backend feature (yet unimplemented).
    QAction *selectBlockAct;
#endif
    QAction *readAllAct;
    QAction *prefsAct;
    QAction *helpCurrentAct;
    QAction *helpContentsAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *licenseAct;
    QAction *windowCloseAct;
    QAction *windowCloseAllAct;
    QAction *cascadeAct;
    QAction *tileAct;

    void createActions();
    void createMenus();
    void createToolbars();
    void createStatusbar();
    void createToolMenus();
    void createDocklets();
    void createDockbars();
    //! \brief Sets tools displaying depending on preferences (tabs/windows)
    void handleToolsDisplay();

public:

    /**
     * Create main window. Always NULL as parent.
     */
    toMain();

    /**
     * Get the workspace widget of the main window.
     * @return Workspace widget.
     */
    QMdiArea *workspace() const
    {
        return Workspace;
    }

    toSearchReplace * searchDialog()
    {
        return Search;
    }

    /**
     * The last active sub window in workspace. MDI area is buggy as
     * hell and currentSubWindow will not tell you the correct window
     * if a menu is open, or if a tool has a custom widget (sql
     * editor), somebody sneezes, etc.
     *
     * Use this to find the last active sub window.
     */
    QMdiSubWindow* lastActiveWindow() const
    {
        return LastActiveWindow;
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

    toBackgroundLabel* getBackgroundLabel();

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
     * Register a tool which to use as an SQL editor. When something
     * is to be edited first a toolwindow will be created and the @ref
     * sqlEditor will be emited, so the SQL editor must connect to
     * this signal as well.
     *
     * @param toolid Which tool identifier to register as the SQL
     *               editor. This is the value which is passed to the
     *               @ref toTool::customSetup member.
     */
    void registerSQLEditor(const QString &);

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
    virtual void exportData(std::map < QString,
                            QString > &data,
                            const QString &prefix);
    /** Import data
     * @param data Data to read from a map.
     * @param prefix Prefix to read data from.
     */
    virtual void importData(std::map < QString,
                            QString > &data,
                            const QString &prefix);
    /** Added chart.
     */
    void addChart(toLineChart *chart);
    /** Removed chart.
     */
    void removeChart(toLineChart *chart);
    /** Setup chart.
     */
    void setupChart(toLineChart *chart);
    /** Add recent file
     */
    void addRecentFile(const QString &filename);

    /**
     * allow tools to add custom menus (ie. File, Edit, Sql Editor)
     * without giving away public access to menubars.
     *
     */
    void addCustomMenu(QMenu *);

    // access needed to some QActions for creating popup menus
    QAction* getUndoAction()
    {
        return undoAct;
    }

    QAction* getRedoAction()
    {
        return redoAct;
    }

    QAction* getCutAction()
    {
        return cutAct;
    }

    QAction* getCopyAction()
    {
        return copyAct;
    }

    QAction* getPasteAction()
    {
        return pasteAct;
    }

    QAction* getSelectAllAction()
    {
        return selectAllAct;
    }

    QMenu* getEditMenu()
    {
        return editMenu;
    }

    /**
     * add a button to the application toolbar
     \warning Do not use it. It screw realoading of the state of toolbars
     \warning The toolbar just after editToolbar the top row of the schema
     browser is migrating to the right - basically the whitespace after
     the editToolbar is expanding too far.
     */
    void addButtonApplication(QAction *);

    /**
     * Add a new connection. The connection itself must already be created.
     * Returns the connection or it's duplicate already opened connection.
     */
    toConnection *addConnection(toConnection *conn, bool def = true);

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
    /**
     * emitted when widgets related to editing.
     */
    void editEnabled(bool en);
    /**
     * emitted to internally handle status bar messages
     */
    void messageRequested(const QString &str, bool save, bool log);

public slots:
    /**
     * Close window
     * @param del If user interaction is allowed.
     * @return True if close was allowed, otherwise false.
     */
    bool close();
    /** Used to enable/disable entries in the windows menu
     */
    void updateWindowsMenu();
    /** Called when active window is changed.
     */
    void windowActivated(QMdiSubWindow *);
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
    /**
     * Show a statusbar message.
     *
     * @param str message to show
     * @param save save to messages menu
     * @param log
     */
    void showMessage(const QString &str, bool save, bool log);

    /**
     * Show a busy cursor or other indicators. Must be called from the
     * main thread.
     *
     * class toBusy calls this method.
     *
     */
    void showBusy(void);

    /**
     * Remove busy cursor or other indicators. Must be called from the
     * main thread.
     *
     * class toBusy calls this method.
     */
    void removeBusy(void);

    /**
     * Open a file in sql worksheet
     *
     */
    void editOpenFile(QString file);

    /**
     * Return the toDockbar that manages the docklet. If none
     * currently manages the docklet, returns what will if the docklet
     * is shown.
     *
     */
    toDockbar* dockbar(toDocklet *let);

    /**
     * Handles moving docklet to new dockbar
     *
     */
    void moveDocklet(toDocklet *let, Qt::DockWidgetArea area);

protected:
    /** intercept close event from parent
     */
    void closeEvent(QCloseEvent *event);

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
    void commandCallback(QAction *);

    /**
     * handles callbacks for recent files menu
     *
     */
    void recentCallback(QAction *);

    /**
     * handles callbacks for the status bar
     *
     */
    void statusCallback(QAction *action);

    /**
     * handles callbacks for the status bar
     *
     */
    void windowCallback(QAction *action);

    /**
     * handles callbacks for the view menu
     *
     */
    void viewCallback(QAction *action);

    void updateStatusMenu(void);

    /** Display status message
     */
    void displayMessage(void);

    /**
     * Internal slot for handling status bar messages
     *
     */
    void showMessageImpl(const QString &str, bool save, bool log);
};

/** Get a pointer to the main window
 * @return Pointer to main window.
 */
toMain *toMainWidget(void);

#endif
