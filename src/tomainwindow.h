
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

#ifndef TOMAINWINDOW_H
#define TOMAINWINDOW_H

#include "config.h"

#include <QMainWindow>

class QMdiArea;
class QMenu;
class QAction;
class QMdiSubWindow;
class toBackgroundLabel;
class toConnection;
class toDockbar;
class toDocklet;
class toLineChart;
class toEditWidget;
class toSearchReplace;
class toToolWidget;

/**
 * This class defines abstract base class for application's main window
 * is base class for @ref toMain
 */
class toMainWindow : public QMainWindow
{
	Q_OBJECT;
public:
	toMainWindow();

	/**
	 * Get the workspace widget of the main window.
	 * @return Workspace widget.
	 */
	virtual QMdiArea *workspace() const = 0;

	virtual toSearchReplace* searchDialog() = 0;
	
	/**
	 * The last active sub window in workspace. MDI area is buggy as
	 * hell and currentSubWindow will not tell you the correct window
	 * if a menu is open, or if a tool has a custom widget (sql
	 * editor), somebody sneezes, etc.
	 *
	 * Use this to find the last active sub window.
	 */
	virtual QMdiSubWindow* lastActiveWindow() const = 0;
	
	virtual toBackgroundLabel* getBackgroundLabel() = 0;
	
	/** Added chart.
	 */	
	virtual void addChart(toLineChart *chart) = 0;

	/** Removed chart.
	 */
	virtual void removeChart(toLineChart *chart) = 0;

	/** Setup chart.
	 */
	virtual void setupChart(toLineChart *chart) = 0;
	    

	/** Add recent file
	 */
	virtual void addRecentFile(const QString &filename) = 0;

	/**
	 * allow tools to add custom menus (ie. File, Edit, Sql Editor)
	 * without giving away public access to menubars.
	 *
	 */
	virtual void addCustomMenu(QMenu *) = 0;
	
	/**
	 * Add a new connection. The connection itself must already be created.
	 * Returns the connection or it's duplicate already opened connection.
	 */
	virtual toConnection *addConnection(toConnection *conn, bool def = true) = 0;

	/**
	 * Get a list of currently open connections.
	 * @return List of connection names. The returned list can then be used by
	 * @ref connection to get the actual connection.
	 */
	virtual std::list<QString> connections(void) = 0;

	/** Get a connection identified by a string.
	 * @return A reference to a connection.
	 * @exception QString If connection wasn't found.
	 */
	virtual toConnection &connection(const QString &) = 0;

	/**
	 * Get the current database connection
	 * @return Reference to current connection.
	 */
	virtual toConnection &currentConnection(void) = 0;

	// access needed to some QActions for creating popup menus
	virtual QAction* getUndoAction() = 0;
	virtual QAction* getRedoAction() = 0;
	virtual QAction* getCutAction() = 0;
	virtual QAction* getCopyAction() = 0;
	virtual QAction* getPasteAction() = 0;
	virtual QAction* getSelectAllAction() = 0;
	virtual QMenu* getEditMenu() = 0;	

	/** Get current edit widget
	 */
	virtual toEditWidget *editWidget() = 0;

	/**
	 * Create the default tool for the current connection.
	 *
	 * This is the tool with the highest priority, usually the SQL worksheet.
	 */
	virtual void createDefault(void) = 0;

	/**
	 * Set coordinates in the statusbar.
	 *
	 * Used to indicate current cursor position by child widgets.
	 */
	virtual void setCoordinates(int, int) = 0;

	/**
	 * Open a file in sql worksheet
	 */
	virtual void editOpenFile(QString file) = 0;

	/** Set if a connection needs to be committed. Also updates visual feedback in interface.
	 */	
	virtual void setNeedCommit(toConnection &conn, bool needCommit = true) = 0;

	/** Edit an SQL statement in the SQL editor if any connected.
	 * @param str Identifier of the SQL to edit.
	 */
	virtual void editSQL(const QString &str) = 0;

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
	virtual void registerSQLEditor(const QString &) = 0;

	/** Called by @ref toTool when a new tool is created.
	 * @param tool The tool widget created.
	 */
	virtual void toolWidgetAdded(toToolWidget *tool) = 0;

	/** Called by @ref toToolWidget when a new tool is about to be destroyed.
	 * @param tool The tool widget created.
	 */
	virtual void toolWidgetRemoved(toToolWidget *tool) = 0;
	
	/**
	 * Return the toDockbar that manages the docklet. If none
	 * currently manages the docklet, returns what will if the docklet
	 * is shown.
	 *
	 */
	virtual toDockbar* dockbar(toDocklet *let) = 0;

public slots:
	/** Check if object caching is done.
	 */
	virtual void checkCaching(void) = 0;

	/** Change current connection
	 */
	virtual void changeConnection(void) = 0;

	/**
	 * Show a statusbar message.
	 *
	 * @param str message to show
	 * @param save save to messages menu
	 * @param log
	 */
	virtual void showMessage(const QString &str, bool save, bool log) = 0;

	/** Used to enable/disable entries in the windows menu
	 */
	virtual void updateWindowsMenu() = 0;

	/** Called when active window is changed.
	 */
	virtual void windowActivated(QMdiSubWindow *) = 0;
};

#endif
