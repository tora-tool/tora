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

#ifndef __TOMAIN_H
#define __TOMAIN_H

TO_NAMESPACE;

#include <list>
#include <map>

#include <qmainwindow.h>

#if 0
/**
 * The class to use for a printer object.
 */
class TOPrinter : public QPrinter {

};
/**
 * The class to use for a dock widget.
 */
class TODock : public QWidget {

};
/**
 * The class to use for a file dialog.
 */
class TOFileDialog : public QFileDialog {

};
/**
 * The class to use for a messagebox.
 */
class TOMessageBox : public QMessageBox {

};
#endif

#ifdef TO_KDE
#  include <kapp.h>
#  include <kdockwidget.h>
#  if KDE_VERSION < 220
#    define TOPrinter QPrinter
#  else
#    define TOPrinter KPrinter
#    define TO_HAS_KPRINT
#  endif
#  define TODock QWidget
#  define TOFileDialog KFileDialog
#  include "tomainwindow.kde.h"
#else
#  include <qtoolbar.h>
#  define TOPrinter QPrinter
#  define TODock QWidget
#  define TOFileDialog QFileDialog
#  include "tomainwindow.h"
#endif
#define TOMessageBox QMessageBox

class QWorkspace;
class QComboBox;
class QToolButton;
class QPopupMenu;
class QTimer;
class QLabel;
class toSearchReplace;

#include "toconnection.h"
#include "totool.h"

/** This class defines the main window. Observe that this class will have different baseclass
 * depending on if TOra is a Qt or KDE application. In the case of Qt this will be a
 * QMainWindow descendant.
 */
class toMain : public toMainWindow {
  Q_OBJECT
private:
  /**
   * The connections that have been opened in TOra.
   */
  list<toConnection *> Connections;
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
   */
  void addConnection(toConnection *conn);
  /**
   * A map from menu ID:s to tools.
   */
  map<int,toTool *> Tools;
  /**
   * A list of toolbuttons that need an open connection to be enabled.
   */
  list<QToolButton *> NeedConnection;
  /**
   * The ID of the tool providing the SQL editor.
   */
  int SQLEditor;

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
  /**
   * Search & replace dialog if available.
   */
  toSearchReplace *Search;
public:

  /** ID of the file menu
   */
  static const int TO_FILE_MENU;
  /** ID of the edit menu
   */
  static const int TO_EDIT_MENU;
  /** ID of the tools menu
   */
  static const int TO_TOOLS_MENU;
  /** ID of the windows menu
   */
  static const int TO_WINDOWS_MENU;
  /** ID of the help menu
   */
  static const int TO_HELP_MENU;

  /** First ID of the tool specific menu items.
   */
  static const int TO_TOOL_MENU_ID;
  /** Last ID of the tool specific menu items.
   */
  static const int TO_TOOL_MENU_ID_END;

  /**
   * Create main window. Always NULL as parent.
   */
  toMain();

  /**
   * Get the workspace widget of the main window.
   * @return Workspace widget.
   */
  QWorkspace *workspace()
  { return Workspace; }

  /**
   * Get the current database connection
   * @return Reference to current connection.
   */
  toConnection &currentConnection(void);

  /**
   * Set available menu items in user interface. It is not enough to just call
   * this function to make open for instance to actually work. It is a lot of
   * more especially in @ref commandCallback and @ref editFileMenu.
   */
  static void editEnable(bool open,bool save,bool print,
			 bool undo,bool redo,
			 bool cut,bool copy,bool paste,
			 bool search);
  /**
   * Disable all the current available items in the user interface (That @ref
   * editEnable can enable).
   */
  static void editDisable(void)
  { editEnable(false,false,false,false,false,false,false,false,false); }

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
  void setCoordinates(int,int);

  /**
   * Get the file menu.
   * @return File menu
   */
  QPopupMenu *fileMenu()
  { return FileMenu; }
  /**
   * Get the edit menu.
   * @return Edit menu
   */
  QPopupMenu *editMenu()
  { return EditMenu; }
  /**
   * Get the tools menu.
   * @return Tools menu
   */
  QPopupMenu *toolsMenu()
  { return ToolsMenu; }
  /**
   * Get the help menu.
   * @return Help menu
   */
  QPopupMenu *helpMenu()
  { return HelpMenu; }

  /**
   * Get a list of currently open connections.
   * @return List of connection names. The returned list can then be used by
   * @ref connection to get the actual connection.
   */
  list<QString> connections(void);
  /** Get a connection identified by a string.
   * @return A reference to a connection.
   * @exception QString If connection wasn't found.
   */
  toConnection &connection(const QString &);

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
signals:
  /** Invoked to start editing an SQL identifier.
   * @param str Identifier to start editing.
   */
  void sqlEditor(const QString &str);
  /** Invoked when a connection is added.
   * @param str Connection identifier.
   */
  void addedConnection(const QString &str);
  /** Invoked when a connection is removed.
   * @param str Connection identifier.
   */
  void removedConnection(const QString &str);
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
private slots:
  /** Used to enable/disable entries in the edit & file menu
   */
  void editFileMenu(void);
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
};

/** Display a message in the statusbar of the main window.
 * @param str Message to display
 * @param save If true don't remove the message after a specified interval.
 */
void toStatusMessage(const QString &str,bool save=false);
/** Get a pointer to the main window
 * @return Pointer to main window.
 */
toMain *toMainWidget(void);
/** Get an address to a SQL statement in the SGA. The address has the form
 * 'address:hash_value' which are resolved from the v$sqltext_with_newlines
 * view in Oracle.
 * @param conn Connection to get address from
 * @param sql Statement to get address for.
 * @return String with address in.
 * @exception QString if address not found.
 * @exception otl_exception on error on connection.
 */
QString toSQLToAddress(toConnection &conn,const QString &sql);
/** Get the full SQL of an address (See @ref toSQLToAddress) from the
 * SGA.
 * @param conn Connection to get address from
 * @param sql Address of SQL.
 * @return String with SQL of statement.
 * @exception QString if address not found.
 * @exception otl_exception on error on connection.
 */
QString toSQLString(toConnection &conn,const QString &address);
/** Get the current database time in the current sessions dateformat.
 * @param conn Connection to get address from.
 * @return String with the current date and time.
 * @exception otl_exception on error on connection.
 */
QString toNow(toConnection &conn);
/** Set the current session type (Style)
 * @param str Session to set, can be any of Motif, Motif Plus, SGI, CDE, Windows and Platinum
 * @exception QString if style not available.
 */
void toSetSessionType(const QString &str);
/** Get current session type (Style)
 * @return A string describing the current style.
 * @see toSetSessionType
 */
QString toGetSessionType(void);
/** Create or fill a combobox with refresh intervals.
 * @param parent Parent of created combobox.
 * @param name Name of created combobox.
 * @param def Default value of the combobox.
 * @param item Combo box to fill. If not specified a new combobox is created.
 */
QComboBox *toRefreshCreate(QWidget *parent,const char *name=NULL,const QString &def=QString::null,
			   QComboBox *item=NULL);
/** Set a timer with the value from a refresh combobox (See @ref toRefreshCreate).
 * @param timer Timer to set timeout in.
 * @param str String from currentText of combobox.
 */
void toRefreshParse(QTimer *timer,const QString &str);
/** Read a value from a query. (Warning, this function will change when OTL 3.2.20 is
 * integrated in TOra)
 * @param dsc Description of query to read values from.
 * @param q OTL Stream to read data from.
 * @param maxSize Maximum size of data to read from query.
 * @return Read value from query.
 * @exception otl_exception on error in input stream.
 * @exception QString other internal errors.
 */
QString toReadValue(const otl_column_desc &dsc,otl_stream &q,int maxSize);
/** Get information about wether this TOra has plugin support or not.
 * @return True if plugin support is enabled.
 */
bool toMonolithic(void);
/** Make a deep copy of a string. Usefull when sharing copying strings between threads.
 * @param str String to copy
 * @return Copied string.
 */
QString toDeepCopy(const QString &str);
/** Allocate a toolbar. This is needed since Qt and KDE use different toolbars.
 * @param parent Parent of toolbar.
 * @param name Name of toolbar.
 * @param db Database name or empty if N/A.
 * @return A newly created toolbar.
 */
QToolBar *toAllocBar(QWidget *parent,const QString &name,const QString &db);
/** Allocate a new docked window. This is needed since Qt and KDE docks windows differently
 * (Qt 2.x doesn't even have support for docked windows). Observe that you must attach a
 * dock using @ref toAttachDock after allocating it.
 * @param name Name of window.
 * @param db Database name or empty if N/A.
 * @param icon Icon of new dock.
 * @return A newly allocated fock.
 */
TODock *toAllocDock(const QString &name,
		    const QString &db,
		    const QPixmap &icon);
/** Attach a dock to a specified position.
 * @param dock Dock to attach
 * @param container Whatever container is supposed to be in the dock window.
 * @param place Where to place the dock.
 */
void toAttachDock(TODock *dock,QWidget *container,QMainWindow::ToolBarDock place);
/** Decode a size string this basically converts "KB" to 1024, "MB" to 1024KB and
 * everything else to 1.
 * @return Multiplier specified by string.
 */
int toSizeDecode(const QString &str);

/** Read all available data from a query.
 * @param str Stream to read data from.
 * @param args Input arguments of query.
 * @return List of read values.
 */
list<QString> toReadQuery(otl_stream &str,list<QString> &args);
/** Read all available data from a query.
 * @param str Stream to read data from.
 * @param arg1 Input arguments of query.
 * @return List of read values.
 */
list<QString> toReadQuery(otl_stream &str,
			  const QString &arg1=QString::null,const QString &arg2=QString::null,
			  const QString &arg3=QString::null,const QString &arg4=QString::null,
			  const QString &arg5=QString::null,const QString &arg6=QString::null,
			  const QString &arg7=QString::null,const QString &arg8=QString::null,
			  const QString &arg9=QString::null);
/** Read all available data from a query.
 * @param conn Connection to execute query on.
 * @param query Query to execute, observe that this is in QCString to utf8 should be used
 *              if passing a QString object.
 * @param args Input arguments of query.
 * @return List of read values.
 */
list<QString> toReadQuery(toConnection &conn,const QCString &query,list<QString> &args);
/** Read all available data from a query.
 * @param conn Connection to execute query on.
 * @param query Query to execute, observe that this is in QCString to utf8 should be used
 *              if passing a QString object.
 * @param arg1 Input arguments of query.
 * @return List of read values.
 */
list<QString> toReadQuery(toConnection &conn,const QCString &query,
			  const QString &arg1=QString::null,const QString &arg2=QString::null,
			  const QString &arg3=QString::null,const QString &arg4=QString::null,
			  const QString &arg5=QString::null,const QString &arg6=QString::null,
			  const QString &arg7=QString::null,const QString &arg8=QString::null,
			  const QString &arg9=QString::null);
/** Shift the first value out of a string list.
 * @param lst List to shift value from (Also modified).
 * @return The first value in the list of strings.
 */
QString toShift(list<QString> &lst);
/** Push a string to the beginning of a list.
 * @param lst List to push value in from of.
 * @param str String to push.
 */
void toUnShift(list<QString> &lst,const QString &str);
/** Pop the last value out of a string list.
 * @param lst List to pop value from (Also modified).
 * @return The value in the list of strings.
 */
QString toPop(list<QString> &lst);
/** Push a string to the end of a list.
 * @param lst List to push value in from of.
 * @param str String to push.
 */
void toPush(list<QString> &lst,const QString &);
/** Convert a string representation to a font structure.
 * @param str String representing the font.
 * @return Font structure represented by the string.
 * @see toFontToString
 */
QFont toStringToFont(const QString &str);
/** Convert a font to a string representation.
 * @param fnt Font to convert.
 * @return String representation of font.
 */
QString toFontToString(const QFont &fnt);
/** Get the path to the help directory.
 * @return Path to the help directory.
 */
QString toHelpPath(void);
/** Read file from filename.
 * @param filename Filename to read file from.
 * @return Contents of file.
 * @exception QString describing I/O problem.
 */
QCString toReadFile(const QString &filename);
/** Write file to filename.
 * @param filename Filename to write file to.
 * @param data Data to write to file.
 */
bool toWriteFile(const QString &filename,const QCString &data);
/** Write file to filename.
 * @param filename Filename to write file to.
 * @param data Data to write to file.
 */
bool toWriteFile(const QString &filename,const QString &data);
/** Compare two string lists.
 * @param l1 First list to compare.
 * @param l2 Second list to compare.
 * @param len Length of lists to compare.
 * @return True if all first len elements match.
 */
bool toCompareLists(QStringList &l1,QStringList &l2,unsigned int len);
/** Set environment variable.
 * @param var Variable name to set.
 * @param val Value to set variable to.
 */
void toSetEnv(const QCString &var,const QCString &val);
/** Delete an environment variable.
 * @param var Environment variable to delete.
 */
void toUnSetEnv(const QCString &var);
/** Pop up a dialog and choose a file to open.
 * @param filename Default filename to open.
 * @param filter Filter of filenames (See @ref QFileDialog)
 * @param parent Parent of dialog.
 * @return Selected filename.
 */
QString toOpenFilename(const QString &filename,const QString &filter,QWidget *parent);
/** Pop up a dialog and choose a file to save to.
 * @param filename Default filename to open.
 * @param filter Filter of filenames (See @ref QFileDialog)
 * @param parent Parent of dialog.
 * @return Selected filename.
 */
QString toSaveFilename(const QString &filename,const QString &filter,QWidget *parent);
/** Get index of menu to insert tool specific menu at.
 * @return Index of the tool menu entry.
 */
int toToolMenuIndex(void);
/** Check if a character is valid for an identifier in Oracle.
 * @param c Character to check
 * @return True if it is a valid Oracle identifier.
 */
bool inline toIsIdent(QChar c)
{
  return c.isLetterOrNumber()||c=='_'||c=='%'||c=='$'||c=='#';
}
/** Get installation directory of application (Plugin directory on unix, installation
 * target on windows).
 * @return String containing directory
 */
QString toPluginPath(void);
/** Get a list of the latest status messages that have been shown.
 */
list<QString> toStatusMessages(void);
/* This can't be documented in KDoc, anyway it is an easy way to catch any exception that
 * might be sent by TOra or OTL and display the message in the statusbar of the main window.
 */
#define TOCATCH \
    catch (const otl_exception &exc) {\
      toStatusMessage(QString::fromUtf8((const char *)exc.msg));\
    } catch (const QString &str) {\
      toStatusMessage(str);\
    }

#ifdef WIN32
/**
 * @internal
 * @deprecated
 * Only for WIN32
 */
template <class T>
T max(T a, T b)
{
    return a > b ? a : b ;
}

/**
 * @internal
 * @deprecated
 * Only for WIN32
 */
template <class T>
T min(T a, T b)
{
    return a < b ? a : b ;
}

#endif

#endif
