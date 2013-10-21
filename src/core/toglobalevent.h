
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

#ifndef TO_GLOBAL_EVENT_H
#define TO_GLOBAL_EVENT_H

#include "loki/Singleton.h"
#include "core/tora_export.h"

#include <QtCore/QObject>

class QString;
class QMenu;
class toToolWidget;
class toConnection;

/**
 * This class implements observer pattern using double dispatch
 * For example: toolWidget -> toMainWindow -> @ref toMain
 *
 * NOTE: instance of this class belongs to the main thread,
 * therefore all signals are also emitted from the main thread
 * do NOT use this class from BG("Worker") threads.
 */
class TORA_EXPORT toGlobalEvent : public QObject
{
    Q_OBJECT;

public:
    toGlobalEvent();

    /** Check if object caching is done.
     */
    void checkCaching(void);

    /**
     * Open a file in sql worksheet
     */
    void editOpenFile(const QString &file);

    /** Add recent file
     */
    void addRecentFile(const QString &filename);

    /**
     * allow tools to add custom menus (ie. File, Edit, Sql Editor)
     * without giving away public access to menubars.
     */
    void addCustomMenu(QMenu *menu);

    /** Called by @ref toTool when a new tool is created.
     * @param tool The tool widget created.
     */
    void toolWidgetAdded(toToolWidget *tool);

    /** Called by @ref toToolWidget when a new tool is about to be destroyed.
     * @param tool The tool widget.
     */
    void toolWidgetRemoved(toToolWidget *tool);

    void toolWidgetsReordered();

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
    void createDefaultTool(void);

    /**
     * Show a statusbar message.
     *
     * @param str message to show
     * @param save save to messages menu
     * @param log
     */
    void showMessage(const QString &str, bool save, bool log);

    /**
     * Add a new connection. The connection itself must already be created.
     * Returns the connection or it's duplicate already opened connection.
     */
    void addConnection(toConnection *conn, bool def = true);

    /** Set if a connection needs to be committed. Also updates visual feedback in interface.
     */
    void setNeedCommit(toToolWidget *tool, bool needCommit = true);

signals:
	void s_checkCaching(void);
    void s_editOpenFile(const QString &filename);
    void s_addRecentFile(const QString &filename);
    void s_addCustomMenu(QMenu *menu);
    void s_toolWidgetAdded(toToolWidget *tool);
    void s_toolWidgetRemoved(toToolWidget *tool);
    void s_toolWidgetsReordered();
    void s_setCoordinates(int x, int y);
    void s_createDefaultTool(void);
    void s_showMessage(QString str, bool save, bool log);
    void s_addConnection(toConnection *conn, bool def);
    void s_setNeedCommit(toToolWidget *tool, bool needCommit);
};

typedef Loki::SingletonHolder<toGlobalEvent> toGlobalEventSingle;

#endif
