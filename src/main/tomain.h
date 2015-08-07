
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

#ifndef TOMAIN_H
#define TOMAIN_H

#include "core/tora_export.h"
#include "core/tomainwindow.h"
#include "core/toeditwidget.h"
#include "core/tobackground.h"

#include <QLabel>
#include <QtCore/QPointer>
#include <QtCore/QMap>

class QComboBox;
class QLabel;
class QPlainTextEdit;
class QToolBar;
class QToolButton;
class QTimer;
class toBackgroundLabel;
class toConnection;
class toConnectionRegistry;
class toDockbar;
class toDocklet;
class toEditMenu;
class toEditWidget;
class toLineChart;
class toMarkedEditor;
class toMessage;
class toTool;
class toToolWidget;
class toToolWidget;
class toTreeWidget;
class toTreeWidgetItem;
class toWorkSpace;

/**
 * This class defines the main window for TOra application
 *
 * NOTE: the abstract class toMainWindow defines an interface have to implement.
 */
class toMain : public toMainWindow
{
        Q_OBJECT;

        void updateRecent(void);

        /**
         * enable or disable buttons that need an active connection
         */
        //    void enableConnectionActions(bool enabled);

        void createActions();
        void createMenus();
        void createToolbars();
        void createStatusbar();
        void createDocklets();
        void createDockbars();

    public:

        /**
         * Create main window. Always NULL as parent.
         */
        toMain();

        /**
         * Get the current database connection
         * @return Reference to current connection.
         */
        //toConnection &currentConnection(void);

#ifdef TORA3_SESSION
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
#endif


        /** Virtual methods inherited from toEditWidget */

        /** Handle events from toEditWidget subclasses */
        virtual void receivedFocus(toEditWidget *widget);

        /** Handle events from toEditWidget subclasses */
        virtual void lostFocus(toEditWidget *widget);

    signals:
        /** Invoked to start editing an SQL identifier.
         * @param str Identifier to start editing.
         */
        //void sqlEditor(const QString &str);

#ifdef TORA3_CHART
        /** Invoked when a new chart is created.
         */
        void chartAdded(toLineChart *chart);
        /** Invoked before a chart is destroyed.
         */
        void chartSetup(toLineChart *chart);
        /** Invoked before a chart is destroyed.
         */
        void chartRemoved(toLineChart *chart);
#endif

        /**
         * emitted when widgets related to editing.
         */
        void editEnabled(bool en);

    protected slots:
        /**
         * Close window
         * @param del If user interaction is allowed.
         * @return True if close was allowed, otherwise false.
         */
        //bool close();

        /** Used to enable/disable entries in the windows menu */
        void updateWindowsMenu();

        void updateConnectionsMenu();

        /** Used to enable/disable entries in the file menu */
        void showFileMenu(void);

        /** Check if object caching is done. */
        void checkCaching(void);

#ifdef TORA3_SESSION
        /** Save the current TOra session
         */
        void saveSession(void);
        /** Load a session (Merge with current)
         */
        void loadSession(void);
        /** Close all windows and connections.
         */
        void closeSession(void);
#endif

        /** Change current connection
         */
        void connectionSelectionChanged(void);


        /** Open a file in sql worksheet */
        void editOpenFile(const QString &file);

        /**
         * Return the toDockbar that manages the docklet. If none
         * currently manages the docklet, returns what will if the docklet
         * is shown.
         */
        virtual toDockbar* dockbar(toDocklet *let);

        /** Handles moving docklet to new dockbar */
        void moveDocklet(toDocklet *let, Qt::DockWidgetArea area);

        /** Set if a connection needs to be committed. Also updates visual feedback in interface.
         */
        void setNeedCommit(toToolWidget *tool, bool needCommit = true);

        /**
         * Add a new connection. The connection itself must already be created.
         * Returns the connection or it's duplicate already opened connection.
         */
        void addConnection(toConnection *conn);

        /** Add recent file
         */
        void addRecentFile(const QString &filename);

        /**
         * Set coordinates in the statusbar.
         *
         * Used to indicate current cursor position by child widgets.
         */
        void setCoordinates(int, int);

        /**
         * Create the default tool for the current connection.
         *
         * This is the tool with the highest priority, usually the SQL worksheet.
         */
        void createDefault(void);

        /**
         * allow tools to add custom menus (ie. File, Edit, Sql Editor)
         * without giving away public access to menubars.
         */
        void addCustomMenu(QMenu *);

        /** Add a connection */
        void addConnection(void);

        /** Remove a connection */
        bool delCurrentConnection(void);

        /** Handles menu selections
         * @param cmd Menu entry ID selected
         */
        void commandCallback(QAction *);

        /** handles callbacks for recent files menu */
        void recentCallback(QAction *);

        /** handles callbacks for the status bar */
        void statusCallback(QAction *action);

        /** handles callbacks for the status bar */
        void windowCallback(QAction *action);

        /** handles callbacks for the view menu */
        void viewCallback(QAction *action);

        void updateStatusMenu(void);

        /** Display status message */
        void displayMessage(void);

        /** Internal slot for handling status bar messages  */
        void showMessageImpl(QString, bool save, bool log);

        void slotActiveToolChaged(toToolWidget*);

#ifdef QT_DEBUG
        // This function should diagnose focus "stealing"
        void reportFocus();
#endif

    protected:
        /** intercept close event from parent */
        void closeEvent(QCloseEvent *event);

    private:
        toWorkSpace &Workspace;

        /** The connections that have been opened in TOra. */
        toConnectionRegistry &Connections;

        /**
         * Handles available connections list in toolbar.
         */
        QComboBox *ConnectionSelection;
        /**
         * The ID of the tool providing the SQL editor.
         */
        // QString SQLEditor;

        /**
         * A reference to the error dialog.
         */
        toMessage * Message;

        /**
         * Status message storage for status bar
         */
        std::list<QString> StatusMessages;

        /** Toolbars, Edit, Tools, Connection */
        QToolBar *editToolbar, *toolsToolbar, *connectionToolbar;

        /** Toolbars for minimizing docklets*/
        toDockbar *leftDockbar, *rightDockbar;

        /** File menu, Recent files menu (child of fileMenu). */
        QMenu *fileMenu, *recentMenu;

        /** Edit menu - reference to a value held in singleton*/
        toEditMenu &editMenu;

        /** View, Tools, Window, Help, Status menu. */
        QMenu *viewMenu, *toolsMenu, *windowsMenu, *connectionsMenu, *helpMenu, *statusMenu;

        /** Current column/row label - diplayed in bottom right,  updated by worksheet widget*/
        QLabel *ColumnLabel, *RowLabel;

        //! \brief Display current type of text edit selection (normal/block)
        QLabel * SelectionLabel;

        toBackground Poll;

        toBackgroundLabel* BackgroundLabel;

        QAction *newConnAct, *closeConn, *commitAct, *rollbackAct, *currentAct, *stopAct;
        QAction *refreshAct, *openAct, *saveAct, *saveAsAct, *openSessionAct, *saveSessionAct;
        QAction *restoreSessionAct , *closeSessionAct, *printAct, *quitAct;
        QAction *helpCurrentAct, *helpContentsAct, *aboutAct, *aboutQtAct, *licenseAct;
        QAction *windowCloseAct, *windowCloseAllAct;

        QPlainTextEdit &loggingWidget;

        toToolWidget *lastToolWidget;

#ifdef QT_DEBUG
        QTimer *reportTimer;
#endif
};

#endif
