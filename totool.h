//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef __TOTOOL_H
#define __TOTOOL_H

#include <map>
#include <qstring.h>
#include <qobject.h>
#include <qvbox.h>

class toConnection;
class otl_connect;
class QTimer;

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
  static map<QString,toTool *> *Tools;
  /**
   * A map containing the available configuration settings. By convention the
   * character ':' is used to separate parts of the path.
   *
   * @see globalConfig
   * @see globalSetConfig
   * @see config
   * @see setConfig
   */
  static map<QString,QString> *Configuration;
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
  QString name() const
  { return Name; }
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

  /**
   * This function is called as a last step after the main widget is created. It could
   * be used to insert the tool pretty much anywhere in the user interface if the toolmenu,
   * toolbar is not sufficient.
   *
   * @param toolid The tool menu id that should be used if it inserts a custom menu entry.
   */
  virtual void customSetup(int toolid)
  { }
  /**
   * Create a new tool window.
   *
   * @param parent Parent window, which is the worksheet of the main window.
   * @param connection The database connection that this tool should operate on.
   */
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection) = 0;
  /**
   * Create and return configuration tab for this tool. The returned widget should also
   * be a childclass of @ref toSettingTab. The widget will be resized to 400x400 pixels.
   *
   * @return A pointer to the widget containing the setup tab for this tool or NULL of
   * no settings are available.
   */
  virtual QWidget *configurationTab(QWidget *parent)
  { return NULL; }

  /**
   * Get access to the map of tools. Don't modify it. Observe that the index string is not
   * the name of the tool but an internal key used to get tools sorted in the correct
   * priority order.
   *
   * @see Tools
   * @return A reference to the tool map.
   */
  static map<QString,toTool *> &tools(void)
  { if (!Tools) Tools=new map<QString,toTool *>; return *Tools; }
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
  static const QString &globalConfig(const QString &tag,const QString &def);
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
  static void globalSetConfig(const QString &tag,const QString &value);

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
  const QString &config(const QString &tag,const QString &def);
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
  void setConfig(const QString &tag,const QString &value);
  
  /**
   * Load a string to string map from file saved by the @ref saveMap function.
   * @param filename Filename to load
   * @param map Reference to the map to fill with the new values.
   */
  static bool loadMap(const QString &filename,map<QString,QString> &map);
  /**
   * Save a string to string map to file.
   * @see loadMap
   * @param filename Filename to load
   * @param map Reference to the map to fill with the new values.
   */
  static bool saveMap(const QString &filename,map<QString,QString> &map);
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
  QTimer *Timer;
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
  toToolWidget(const QString &ctx,
	       QWidget *parent,
	       toConnection &conn,
	       const char *name=NULL);
  ~toToolWidget();
  /** Get the otl_connect structure of the current connection.
   * @return Reference to otl_connect object.
   */
  otl_connect &otlConnect();
  /** Get the current connection.
   * @return Reference to connection.
   */
  toConnection &connection()
  { return *Connection; }
  /** Change connection of tool.
   */
  void setConnection(toConnection &conn);
  /** Get timer of tool. Used by some results to get update time.
   * @return Pointer to a timer object.
   */
  QTimer *timer(void);
};

#endif
