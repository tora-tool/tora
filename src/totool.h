
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

#ifndef TOTOOL_H
#define TOTOOL_H

#include "config.h"
#include "toconfiguration.h"

#include <map>

#include <qobject.h>
#include <qstring.h>

#include <QPixmap>
#include <QAction>
#include <QMdiSubWindow>

class toConnection;
class toTimer;

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
    Q_OBJECT
private:
    /**
     * Name of the tool.
     */
    QString Name;
    /**
     * Key of the tool, this is used for sorting.
     */
    QString Key;
    /**
     * Priority, used to determine in which order the tools should be listed.
     */
    int Priority;
    /**
     * A map of @ref Key to tools. Used to keep track of the different tools
     * available.
     */
    static std::map<QString, toTool *> *Tools;
    /**
     * Contain the pixmap of this tool if any. Used for the toolbar and menu entries.
     */
    QPixmap *ButtonPicture;

    /**
     * For inserting into menus and toolbars.
     *
     */
    QAction *toolAction;


protected:
    /**
     * Should return the xpm used to create the @ref ButtonPicture.
     */
    virtual const char **pictureXPM(void);
public:
    /**
     * Get the name.
     *
     * @return Name of tool.
     */
    QString name() const
    {
        return Name;
    }
    /**
     * Get the name.
     *
     * @return Name of tool.
     */
    QString key() const
    {
        return Key;
    }
    /**
     * Get the priority.
     *
     * @return Priority of tool.
     */
    int priority() const
    {
        return Priority;
    }
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
     * Get the image to display in the toolbar.
     *
     * @return Pointer to image in toolbar or NULL if no image should be displayed.
     */
    virtual const QPixmap *toolbarImage();
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
    virtual const char *toolbarTip()
    {
        return menuItem();
    }

    /** Check if the tool can handle a specific connection. Default is to only handle
     * connections from the provider Oracle.
     * @return True if connection can be handled.
     */
    virtual bool canHandle(toConnection &conn);
    /**
     * This function is called as a last step after the main widget is created. It could
     * be used to insert the tool pretty much anywhere in the user interface if the toolmenu,
     * toolbar is not sufficient.
     *
     */
    virtual void customSetup();
    /**
     * Create a new tool window. See toToolWidget documentation for
     * exact tool vs. MDI windows handling.
     *
     * @param parent Parent window, which is the worksheet of the main window.
     * @param connection The database connection that this tool should operate on.
     */
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection) = 0;
    /**
     * Create and return configuration tab for this tool. The returned widget should also
     * be a childclass of @ref toSettingTab.
     *
     * @return A pointer to the widget containing the setup tab for this tool or NULL of
     * no settings are available.
     */
    virtual QWidget *configurationTab(QWidget *parent);

    /** Display an about dialog for this tool.
     * @param parent The parent widget of the about dialog.
     */
    virtual void about(QWidget *parent);
    /** Indicate whether or not this tool has an about dialog.
     */
    virtual bool hasAbout(void)
    {
        return false;
    }

    /**
     * Get access to the map of tools. Don't modify it. Observe that the index string is not
     * the name of the tool but an internal key used to get tools sorted in the correct
     * priority order.
     *
     * @see Tools
     * @return A reference to the tool map.
     */
    static std::map<QString, toTool *> &tools(void)
    {
        if (!Tools)
            Tools = new std::map<QString, toTool *>;
        return *Tools;
    }
    /**
     * Get a pointer to the tool with a specified key.
     *
     * @see Tools
     * @return A pointer to the tool or NULL if tool doesn't exist.
     */
    static toTool *tool(const QString &key);

    /**
     * Get tool specific settings.
     *
     * Setting names are hierachical separated by ':' instead of '/' usually used
     * in filenames. As an example all settings for the tool 'Example' would be
     * under the 'Example:{settingname}' name.
     *
     * @param tag The name of the configuration setting.
     * @param def Contents of this setting.
     */
//     const QString &config(const QString &tag, const QString &def);

    /**
     * Change toolspecific setting. Depending on the implementation this can change the
     * contents on disk or not.
     *
     * Setting names are hierachical separated by ':' instead of '/' usually used
     * in filenames. As an example all settings for the tool 'Example' would be
     * under the 'Example:{settingname}' name.
     *
     * @param tag The name of the configuration setting.
     * @param def Default value of the setting, if it is not available.
     */
//     void setConfig(const QString &tag, const QString &value);

    virtual void closeWindow(toConnection &connection) = 0;

    /**
     * Get the tool's QAction object
     *
     */
    QAction* getAction(void);

    /**
     * Enable/disable tool's QAction
     *
     */
    void enableAction(bool en)
    {
        getAction()->setEnabled(en);
    }

    /**
     * Enable/disable tool's QAction if tool can support conn.
     *
     */
    void enableAction(toConnection &conn)
    {
        enableAction(canHandle(conn));
    }

    /**
     * Set action visibility if tool can support toConnection
     * provider.
     *
     */
    void setActionVisible(toConnection &conn)
    {
        getAction()->setVisible(canHandle(conn));
    }

    /**
     * Set action visibility if tool can support toConnection
     * provider.
     *
     */
    void setActionVisible(bool en)
    {
        getAction()->setVisible(en);
    }

public slots:
    /**
     * Create a window of the current tool. This function sets up a toolwindow for
     * this tool. It calls the @ref toolWindow function to get widget and sets it
     * up properly.
     */
    QWidget* createWindow(void);


};

#include "tohelp.h"

/**
 * Abstract baseclass for widgets defining tool settings.
 */

class toSettingTab : public toHelpContext
{
public:
    /**
     * Default constructor.
     * @param ctx Help context for this setting tab.
     */
    toSettingTab(const QString &ctx)
            : toHelpContext(ctx)
    { }
    /**
     * This function is called to save the contents of the widget when
     * a user has pressed the ok button of the dialog. It should simply
     * save the values in the dialog to the appropriate configuration
     * entry using the @ref toTool::setConfig function.
     */
    virtual void saveSetting(void) = 0;
};

/** This class is used to hold connections for @ref toResult classes.
 * Must be multiply inherited by a widget otherwise it will go kaboom.
 * It will dynamic cast itself to a QWidget from time to time so if that
 * doesn't resolve correctly it will not work.
 */
class toConnectionWidget
{
    toConnection *Connection;
    QWidget *Widget;
public:
    /** Constructor with the connection it should be set to initially.
     */
    toConnectionWidget(toConnection &conn, QWidget *widget);
    /** Constructor without a connection. Will inherit the connection from a parent connection widget.
     */
    toConnectionWidget(QWidget *widget);
    /** Destructor.
     */
    virtual ~toConnectionWidget();
    /** Change connection of the widget.
     */
    virtual void setConnection(toConnection &conn);
    /** Get the connection it is pointed to.
     */
    virtual toConnection &connection();
};

/** Simple baseclass for widgets defining the main tool widget. It is in
 * no way mandatory and all it does is register the widget in the connetion.
 *
 * Tools are common QWidget based objects. The main window - the MDI area -
 * requires them wrapped as QMdiSubWindows. So the tool widget is set as widget()
 * in the toToolWidget constructor.
 *
 * \note: Note that all access to tool widget is done via QMdiSubWindow::widget() in
 * the code. See all available tools' windowActivated() slots for examples.
 *
 * \warning Remember that windowActivated() slot called by toMainWidget()->workspace(),
 * SIGNAL(subWindowActivated(QMdiSubWindow *) has to handle "0" as input parameter
 * so an appropriate test is mandatory.
 */
class toToolWidget : public QWidget, public toHelpContext, public toConnectionWidget
{
    Q_OBJECT
    toTimer *Timer;
    toTool &Tool;
private slots:
    void parentConnection(void);
signals:
    /** Emitted when the connection is changed.
     */
    void connectionChange(void);
public:
    /** Create widget and its QMdiSubWindow.
     * @param ctx Help context for this tool.
     * @param parent Parent widget.
     * @param conn Connection of widget.
     * @param name Name of widget.
     */
    toToolWidget(toTool &tool,
                 const QString &ctx,
                 QWidget *parent,
                 toConnection &conn,
                 const char *name = NULL);
    ~toToolWidget();
    /** Get the current connection.
     * @return Reference to connection.
     */
    toConnection &connection()
    {
        return toConnectionWidget::connection();
    }
    /** Get the tool for this tool widget.
     * @return Reference to a tool object.
     */
    toTool &tool(void)
    {
        return Tool;
    }
    /** Check if this tool can handle a specific connection.
     * @param provider Name of connection.
     * @return True if connection is handled.
     */
    virtual bool canHandle(toConnection &conn)
    {
        return Tool.canHandle(conn);
    }
    /** Change connection of tool.
     */
    void setConnection(toConnection &conn);
    /** Get timer of tool. Used by some results to get update time.
     * @return Pointer to a timer object.
     */
    toTimer *timer(void);

    /** Export data to a map.
     * @param data A map that can be used to recreate the data of a chart.
     * @param prefix Prefix to add to the map.
     */
    virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
    /** Import data
     * @param data Data to read from a map.
     * @param prefix Prefix to read data from.
     */
    virtual void importData(std::map<QString, QString> &data, const QString &prefix);
};

#endif
