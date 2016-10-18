
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

#pragma once

#include "core/tora_export.h"

#include <loki/Singleton.h>

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QWidget>

class QPixmap;
class QAction;
class QFocusEvent;
class QPaintEvent;
class QAction;

class toConnection;
class toToolWidget;

/**
 * Abstract baseclass for tools.
 *
 * This class is the baseclass of all classes defining tools. It
 * contains functions for defining the priority and name of the tool,
 * as well as virtual functions to define it's place in the user
 * interface. Further it contains methods to access configuration
 * settings.
 *
 * To use this class you create a child which is then instantiated once
 * which inserts that tool in the global tool map (See @ref tools). You
 * should never delete a tool unless on exit. Usually tools are instantiated
 * statically in the global scope.
 */
class toTool : public QObject
{
        Q_OBJECT;
        friend class toWorkSpace;
        friend class toTemplate;
        friend class toToolsRegistry;
        friend class toToolWidget;
    public:
        /**
         * Get the name.
         * @return Name of tool.
         */
        QString const& name() const;

        /**
         * Get the name.
         *
         * @return Name of tool.
         */
        QString const& key() const;

        /**
         * Get the priority.
         *
         * @return Priority of tool.
         */
        const int priority() const;

        /**
         * This should never be called, but if it is. Erases the tool from the list of
         * available tools. WARNING: It will not remove any of it's open tools.
         */
        ~toTool();

        /**
         * Create a tool. Remember that usually the main window is not created here.
         *
         * @param priority Priority of the created tool.
         * @param name Name of tool.
         */
        toTool(int priority, const char *name);

        /**
         * Get the name of the menuitem to be displayed in the menu.
         *
         * @return A string containing the name of the menuentry or NULL if no menuentry should
         *         be created.
         */
        virtual const char *menuItem()
        {
            return NULL;
        }

        /**
         * Get toolbar tip of the toolbar button. Defaults to same as @ref menuItem.
         *
         * @return Toolbar tip string.
         */
        const char *toolbarTip()
        {
            return menuItem();
        }

        /** Check if the tool can handle a specific connection. Default is to only handle
         * connections from the provider Oracle.
         * @return True if connection can be handled.
         */
        virtual bool canHandle(const toConnection &conn);

        /**
         * This function is called as a last step after the main widget is created. It could
         * be used to insert the tool pretty much anywhere in the user interface if the toolmenu,
         * toolbar is not sufficient.
         *
         */
        virtual void customSetup() {};

        /**
         * Create and return configuration tab for this tool. The returned widget should also
         * be a childclass of @ref toSettingTab.
         *
         * @return A pointer to the widget containing the setup tab for this tool or NULL of
         * no settings are available.
         */
        virtual QWidget *configurationTab(QWidget *parent)
        {
            return NULL;
        };

        /** Display an about dialog for this tool.
         * @param parent The parent widget of the about dialog.
         */
        virtual void about(QWidget *parent) {};

        virtual void closeWindow(toConnection &connection) = 0;

        /**
         * Get the tool's QAction object
         */
        QAction* getAction(void);

        /**
         * Enable/disable tool's QAction
         */
        void enableAction(bool en);

        /**
         * Enable/disable tool's QAction if tool can support conn.
         */
        void enableAction(toConnection const& conn);

        /**
         * Set action visibility if tool can support toConnection
         * provider.
         */
        void setActionVisible(toConnection &conn);

        /**
         * Set action visibility if tool can support toConnection
         * provider.
         */
        void setActionVisible(bool en);

    public slots:
        /**
         * Create a window of the current tool. This function sets up a toolwindow for
         * this tool. It calls the @ref toolWindow function to get widget and sets it
         * up properly.
         */
        toToolWidget* createWindow(void);

    protected:
        /**
         * Create a new tool window. See toToolWidget documentation for
         * exact tool vs. MDI windows handling.
         *
         * @param parent Parent window, which is the worksheet of the main window.
         * @param connection The database connection that this tool should operate on.
         */
        virtual toToolWidget *toolWindow(QWidget *parent, toConnection &connection) = 0;

        /**
         * Get the image to display in the toolbar.
         *
         * @return Pointer to image in toolbar or NULL if no image should be displayed.
         */
        virtual const QPixmap *toolbarImage();

        /**
         * Should return the xpm used to create the @ref ButtonPicture.
         */
        virtual const char **pictureXPM(void);

    private:
        QString Name;            // Name of the tool.
        QString Key;             // Key of the tool, this is used for sorting.
        int Priority;            //Priority, used to determine in which order the tools should be listed.
        QPixmap *ButtonPicture;  // Contain the pixmap of this tool if any. Used for the toolbar and menu entries
        QAction *toolAction;     // For inserting into menus and toolbars
};

/*
 * Get access to the map of tools. Don't modify it. Observe that the index string is not
 * the name of the tool but an internal key used to get tools sorted in the correct
 * priority order.
 //  * @see Tools
 //  * @return A reference to the tool map.
 */
class QToolBar;
class QMenu;
class toToolsRegistry: public QMap<QString, toTool *>
{
    public:
        typedef QMap<QString, toTool*> super;
        QToolBar* toolsToolbar(QToolBar *toolbar) const;
        QMenu* toolsMenu(QMenu* menu) const;
    private:
};

typedef Loki::SingletonHolder<toToolsRegistry> ToolsRegistrySing;
