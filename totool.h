//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003 Quest Software, Inc
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
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef TOTOOL_H
#define TOTOOL_H

#include <map>

#include <qobject.h>
#include <qstring.h>
#include <qvbox.h>

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

class toTool : public QObject {
  Q_OBJECT
private:
  /**
   * Name of the tool.
   */
  QCString Name;
  /**
   * Key of the tool, this is used for sorting.
   */
  QCString Key;
  /**
   * Priority, used to determine in which order the tools should be listed.
   */
  int Priority;
  /**
   * A map of @ref Key to tools. Used to keep track of the different tools
   * available.
   */
  static std::map<QCString,toTool *> *Tools;
  /**
   * A map containing the available configuration settings. By convention the
   * character ':' is used to separate parts of the path.
   *
   * @see globalConfig
   * @see globalSetConfig
   * @see config
   * @see setConfig
   */
  static std::map<QCString,QString> *Configuration;
  /**
   * Contain the pixmap of this tool if any. Used for the toolbar and menu entries.
   */
  QPixmap *ButtonPicture;

  /**
   * Load configuration from file.
   */
  static void loadConfig(void);
protected:
  /**
   * Should return the xpm used to create the @ref ButtonPicture.
   */
  virtual char **pictureXPM(void);
public:
  /**
   * Get the name.
   *
   * @return Name of tool.
   */
  QCString name() const
  { return Name; }
  /**
   * Get the name.
   *
   * @return Name of tool.
   */
  QCString key() const
  { return Key; }
  /**
   * Get the priority.
   *
   * @return Priority of tool.
   */
  int priority() const
  { return Priority; }
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
  toTool(int priority,const char *name);
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
  { return NULL; }
  /**
   * Get toolbar tip of the toolbar button. Defaults to same as @ref menuItem.
   *
   * @return Toolbar tip string.
   */
  virtual const char *toolbarTip()
  { return menuItem(); }

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
   * @param toolid The tool menu id that should be used if it inserts a custom menu entry.
   */
  virtual void customSetup(int toolid);
  /**
   * Create a new tool window.
   *
   * @param parent Parent window, which is the worksheet of the main window.
   * @param connection The database connection that this tool should operate on.
   */
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection) = 0;
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
  { return false; }

  /**
   * Get access to the map of tools. Don't modify it. Observe that the index string is not
   * the name of the tool but an internal key used to get tools sorted in the correct
   * priority order.
   *
   * @see Tools
   * @return A reference to the tool map.
   */
  static std::map<QCString,toTool *> &tools(void)
  { if (!Tools) Tools=new std::map<QCString,toTool *>; return *Tools; }
  /**
   * Get a pointer to the tool with a specified key.
   *
   * @see Tools
   * @return A pointer to the tool or NULL if tool doesn't exist.
   */
  static toTool *tool(const QCString &key);
  /**
   * Save configuration to file.
   */
  static void saveConfig(void);
  /**
   * Get value of a setting.
   *
   * Setting names are hierachical separated by ':' instead of '/' usually used
   * in filenames. As an example all settings for the tool 'Example' would be
   * under the 'Example:{settingname}' name.
   *
   * @param tag The name of the configuration setting.
   * @param def Default value of the setting, if it is not available.
   */
  static const QString &globalConfig(const QCString &tag,const QCString &def);
  /**
   * Change a setting. Depending on the implementation this can change the
   * contents on disk or not.
   *
   * Setting names are hierachical separated by ':' instead of '/' usually used
   * in filenames. As an example all settings for the tool 'Example' would be
   * under the 'Example:{settingname}' name.
   *
   * @param tag The name of the configuration setting.
   * @param def Contents of this setting.
   */
  static void globalSetConfig(const QCString &tag,const QString &value);
  /**
   * Remove a setting. Can be usefull for removing sensetive information.
   * @param tag The name of the configuration setting to remove.
   */
  static void globalEraseConfig(const QCString &tag);

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
  const QString &config(const QCString &tag,const QCString &def);
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
  void setConfig(const QCString &tag,const QString &value);
  /**
   * Remove a toolspecific setting. Can be usefull for removing sensetive information.
   * @param tag The name of the configuration setting to remove.
   */
  void eraseConfig(const QCString &tag);
  /**
   * Load a string to string map from file saved by the @ref saveMap function.
   * @param filename Filename to load
   * @param map Reference to the map to fill with the new values.
   */
  static void loadMap(const QString &filename,std::map<QCString,QString> &map);
  /**
   * Save a string to string map to file.
   * @see loadMap
   * @param filename Filename to load
   * @param map Reference to the map to fill with the new values.
   */
  static bool saveMap(const QString &filename,std::map<QCString,QString> &map);
public slots:
  /**
   * Create a window of the current tool. This function sets up a toolwindow for
   * this tool. It calls the @ref toolWindow function to get widget and sets it
   * up properly.
   */
  void createWindow(void);
};

#include "tohelp.h"

/**
 * Abstract baseclass for widgets defining tool settings.
 */

class toSettingTab : public toHelpContext {
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

/** Simple baseclass for widgets defining the main tool widget. It is in
 * no way mandatory and all it does is register the widget in the connetion.
 */
class toToolWidget : public QVBox, public toHelpContext {
  Q_OBJECT
  toConnection *Connection;
  toTimer *Timer;
  toTool &Tool;
private slots:
  void parentConnection(void);
signals:
  /** Emitted when the connection is changed.
   */
  void connectionChange(void);
public:
  /** Create widget.
   * @param ctx Help context for this tool.
   * @param parent Parent widget.
   * @param conn Connection of widget.
   * @param name Name of widget.
   */
  toToolWidget(toTool &tool,
	       const QString &ctx,
	       QWidget *parent,
	       toConnection &conn,
	       const char *name=NULL);
  ~toToolWidget();
  /** Get the current connection.
   * @return Reference to connection.
   */
  toConnection &connection()
  { return *Connection; }
  /** Get the tool for this tool widget.
   * @return Reference to a tool object.
   */
  toTool &tool(void)
  { return Tool; }
  /** Check if this tool can handle a specific connection.
   * @param provider Name of connection.
   * @return True if connection is handled.
   */
  virtual bool canHandle(toConnection &conn)
  { return Tool.canHandle(conn); }
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
  virtual void exportData(std::map<QCString,QString> &data,const QCString &prefix);
  /** Import data
   * @param data Data to read from a map.
   * @param prefix Prefix to read data from.
   */
  virtual void importData(std::map<QCString,QString> &data,const QCString &prefix);
};

#endif
