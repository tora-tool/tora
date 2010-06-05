
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

#ifndef UTILS_H
#define UTILS_H

#include "config.h"

#include <list>
#include <map>
#include <algorithm>

#include <QToolButton>
#include <QString>
#include <QPixmap>
#include <QKeyEvent>
#include <QMainWindow>

#define TOPrinter QPrinter
#define TOFileDialog QFileDialog
#define TOMessageBox QMessageBox
#define TODock QWidget

class QComboBox;
class toTreeWidget;
class toTreeWidgetItem;
class QToolBar;
class QWidget;
class toConnection;
class toMain;
class toQValue;
class toTimer;
class toToolWidget;

/** Display a message in the statusbar of the main window.
 * @param str Message to display
 * @param save If true don't remove the message after a specified interval.
 * @param log Log message. Will never log saved messages.
 */
void toStatusMessage(const QString &str, bool save = false, bool log = true);
/** Get an address to a SQL statement in the SGA. The address has the form
 * 'address:hash_value' which are resolved from the v$sqltext_with_newlines
 * view in Oracle.
 * @param conn Connection to get address from
 * @param sql Statement to get address for.
 * @return String with address in.
 * @exception QString if address not found.
 */
QString toSQLToAddress(toConnection &conn, const QString &sql);
/** Get the full SQL of an address (See @ref toSQLToAddress) from the
 * SGA.
 * @param conn Connection to get address from
 * @param sql Address of SQL.
 * @return String with SQL of statement.
 * @exception QString if address not found.
 */
QString toSQLString(toConnection &conn, const QString &address);
/** Make a column name more readable.
 * @param col Name of column name, will be modified.
 */
void toReadableColumn(QString &col);
/** Get the current database time in the current sessions dateformat.
 * @param conn Connection to get address from.
 * @return String with the current date and time.
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
/** Get the available styles.
 */
QStringList toGetSessionTypes(void);
/** Create or fill a combobox with refresh intervals.
 * @param parent Parent of created combobox.
 * @param name Name of created combobox.
 * @param def Default value of the combobox.
 * @param item Combo box to fill. If not specified a new combobox is created.
 */
QComboBox *toRefreshCreate(QWidget *parent, const char *name = NULL, const QString & def = QString::null,
                           QComboBox * item = NULL);
/** Set a timer with the value from a refresh combobox (See @ref toRefreshCreate).
 * @param timer Timer to set timeout in.
 * @param str String from currentText of combobox. If empty, set to default.
 */
void toRefreshParse(toTimer *timer, const QString &str = QString::null);
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
 * @return A newly created toolbar.
 */
QToolBar *toAllocBar(QWidget *parent, const QString &name);
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
void toAttachDock(TODock *, QWidget *, Qt::DockWidgetArea);
/** Decode a size string this basically converts "KB" to 1024, "MB" to 1024KB and
 * everything else to 1.
 * @return Multiplier specified by string.
 */
int toSizeDecode(const QString &str);
/**
 * Check that a key sequence is valid and return same sequence
 */
QKeySequence toKeySequence(const QString &keysequence);
/**
 * Check if a key sequence and a key event represent the same key and return if that is the case.
 * Only support single key keysequences.
 * @param event Key event.
 * @param key Key sequence.
 */
bool toCheckKeyEvent(QKeyEvent *event, const QKeySequence &key);

/** Shift the first value out of a list.
 * @param lst List to shift value from (Also modified).
 * @return The first value in the list.
 */
template <class T>
T toShift(std::list<T> &lst)
{
    if (lst.begin() == lst.end())
    {
        T ret;
        return ret;
    }
    T ret = (*lst.begin());
    lst.erase(lst.begin());
    return ret;
}

/** Push an object to the beginning of a list.
 * @param lst List to push value in from of.
 * @param str Object to push.
 */
template <class T>
void toUnShift(std::list<T> &lst, const T &str)
{
    lst.insert(lst.begin(), str);
}

/** Pop the last value out of a list.
 * @param lst List to pop value from (Also modified).
 * @return The value in the list of objects.
 */
template <class T>
T toPop(std::list<T> &lst)
{
    if (lst.begin() == lst.end())
    {
        T ret;
        return ret;
    }
    T ret = (*lst.rbegin());
    lst.pop_back();
    return ret;
}

/** Push an object to the end of a list.
 * @param lst List to push value in from of.
 * @param str Object to push.
 */
template <class T>
void toPush(std::list<T> &lst, const T &str)
{
    lst.push_back(str);
}

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
// QString toHelpPath(void);
/** Read file from filename and return it as binary data (no decoding).
 * @param filename Filename to read file from.
 * @return Contents of file.
 * @exception QString describing I/O problem.
 */
QByteArray toReadFileB(const QString &filename);
/** Read file from filename and decode it according to current locale settings.
 * @param filename Filename to read file from.
 * @return Contents of file.
 * @exception QString describing I/O problem.
 */
QString toReadFile(const QString &filename);
/** Write file to filename. (binary data, no encoding is performed)
 * @param filename Filename to write file to.
 * @param data Data to write to file.
 */
bool toWriteFile(const QString &filename, const QByteArray &data);
/** Write file to filename, encoded according to current locale settings.
 * @param filename Filename to write file to.
 * @param data Data to write to file.
 */
bool toWriteFile(const QString &filename, const QString &data);
/** Compare two string lists.
 * @param l1 First list to compare.
 * @param l2 Second list to compare.
 * @param len Length of lists to compare.
 * @return True if all first len elements match.
 */
bool toCompareLists(QStringList &l1, QStringList &l2, int len);
/** Set environment variable.
 * @param var Variable name to set.
 * @param val Value to set variable to.
 */
void toSetEnv(const QString &var, const QString &val);
/** Delete an environment variable.
 * @param var Environment variable to delete.
 */
void toUnSetEnv(const QString &var);
/** Pop up a dialog and choose a file to open.
 * @param filename Default filename to open.
 * @param filter Filter of filenames (See @ref QFileDialog)
 * @param parent Parent of dialog.
 * @return Selected filename.
 */
QString toOpenFilename(const QString &filename, const QString &filter, QWidget *parent);
/** Pop up a dialog and choose a file to save to.
 * @param filename Default filename to open.
 * @param filter Filter of filenames (See @ref QFileDialog)
 * @param parent Parent of dialog.
 * @return Selected filename.
 */
QString toSaveFilename(const QString &filename, const QString &filter, QWidget *parent);

/** Check if a character is valid for an identifier in Oracle.
 * @param c Character to check
 * @return True if it is a valid Oracle identifier.
 */
inline bool toIsIdent(QChar c)
{
    return c.isLetterOrNumber() || c == '_' || c == '%' || c == '$' || c == '#';
}
/** Get installation directory of application (Plugin directory on unix, installation
 * target on windows).
 * @return String containing directory
 */
// QString toPluginPath(void);
/** Get a brush for a chart item.
 * @param index Indicating which chart item to get color for.
 */
QBrush toChartBrush(int index);
/** Return the connection most closely associated with a widget. Currently connections are
 * only stored in toToolWidgets.
 * @return Reference toConnection object closest to the current.
 */
toConnection &toCurrentConnection(QObject *widget);
/** Return the tool widget most closely associated with a widget.
 * @return Pointer to tool widget.
 */
toToolWidget *toCurrentTool(QObject *widget);
/** Check if this connection is an oracle connection.
 */
bool toIsOracle(const toConnection &);
/** Check if this connection is an sapdb connection.
 */
bool toIsSapDB(const toConnection &);
/** Check if this connection is an MySQL connection.
 */
bool toIsMySQL(const toConnection &);
/** Check if this connection is an PostgreSQL connection.
 */
bool toIsPostgreSQL(const toConnection &);
/** Check if this connection is a Teradata connection.
 */
bool toIsTeradata(const toConnection &);
/** Strip extra bind specifier from an SQL statement. (That means the extra <***> part after
 * the bind variable.
 * @param sql The sql to strip.
 * @return Return a string containing the same statement without qualifiers, which means the
 *         sql sent to Oracle and available in the SGA.
 */
QString toSQLStripSpecifier(const QString &sql);
/** Strip extra binds and replace with empty strings.
 * @param sql The sql to strip.
 * @return Return a string containing the same statement without binds.
 */
QString toSQLStripBind(const QString &sql);
/** Expand filename with $HOME to be replaced with home directory or my documents.
 */
QString toExpandFile(const QString &file);
/** Convert string read by readValue to value read by readValueNull
 */
toQValue toUnnull(const toQValue &str);
/** Convert string read by readValueNull to value to be read by readValue.
 */
toQValue toNull(const toQValue &str);

/** Take a string and make it illegible. Some security through obscurity here so you will
 * need to check the source to see what is actually done.
 * @param str String to be made unreadable.
 * @return Obfuscated string.
 */
QString toObfuscate(const QString &str);
/** Unmake the actions of @ref toObfuscate.
 * @param Obfuscated string.
 * @return Original string.
 */
QString toUnobfuscate(const QString &str);
/** Check if the current widget is within the active modal widget (Or no modal widget exists)
 */
bool toCheckModal(QWidget *widget);

/** Export the contents of a map into another map.
 * @param data The destination map.
 * @param prefix Prefix to save map using.
 * @param src The map to save.
 */
void toMapExport(std::map<QString, QString> &data, const QString &prefix,
                 std::map<QString, QString> &src);
/** Import the contents of a map from another map.
 * @param data The source map.
 * @param prefix Prefix to use for restoring from map.
 * @param dst The map to save into.
 */
void toMapImport(std::map<QString, QString> &data, const QString &prefix,
                 std::map<QString, QString> &dst);

/** Find an item in a listview.
 * @param list The list to search for the item.
 * @param str The string to search for. You can specify parent/child with : in the string.
 */
toTreeWidgetItem *toFindItem(toTreeWidget *list, const QString &str);
/** Whenever this class is instantiated the window will display a busy cursor. You
 * can instantiate this function as many times as you want, only when all of them are
 * destructed the curser will revert back to normal.
 * If you want to control if busy cursor is actually shown for a particular case, you
 * can use a boolean parameter in constructor. For example debugger is setting this as
 * false in order for cursor NOT to be busy until program unit being debugged finishes.
 */
class toBusy
{
private:
    bool Busy;
public:
    toBusy(bool busy = true);
    ~toBusy();
};
/**
 * Set or change title of a tool window.
 * @param tool Widget of tool.
 * @param caption Caption to set to the tool.
 */
void toToolCaption(toToolWidget *tool, const QString &caption);
/**
 * Translate string if the context and text is strictly ASCII7 and the context doesn't
 * contain any spaces. Otherwise just return the text.
 * @param context The context of the text.
 * @param text The text to translate.
 * @return Translated text if appropriate, or original text otherwise.
 */
QString toTranslateMayby(const QString &context, const QString &text);

/**
 * Count occurrences of char in source.
 *
 * @param source The string to search.
 * @param find The char to find in string
 * @return Occurrences.
 */
int countChars(const QString &source, const char find);

/**
 * Get extensions for file save dialog from editor configuration
 *
 * @return Extensions.
 */
QString GetExtensions(void);

QPixmap connectionColorPixmap(const QString & name);

/** Popup toolbutton that works like I think they should under Qt 3.0 as well.
 * this means they will pop up a tool menu when you click on the button as well.
 */
class toPopupButton : public QToolButton
{
    Q_OBJECT
public:
    /** Create button, same constructor as toolbutton except no slots
     */
    toPopupButton(const QIcon &iconSet, const QString &textLabel,
                  const QString &grouptext, QToolBar *parent, const char *name = 0);
    /** Create button somewhere else than in a toolbar.
     */
    toPopupButton(QWidget *parent, const char *name = 0);
};

/**
 * functor for cleanup containers of pointers
 */
struct DeleteObject
{
    template <typename T>
    void operator()(const T* ptr) const
    {
        delete ptr;
    }
};

/* This can't be documented in KDoc, anyway it is an easy way to catch any exception that
 * might be sent by TOra or OTL and display the message in the statusbar of the main window.
 */
#define TOCATCH                                 \
    catch (const QString &str) {                \
      toStatusMessage(str);                     \
    }
/* This can't be documented in KDoc, anyway it is an easy way to catch any exception that
 * might be sent by TOra or OTL and display the message in the statusbar of the main window.
 */
#define TOROLLBACK(x)                           \
    catch (const QString &str) {                \
      toStatusMessage(str);                     \
      try {                                     \
        x.rollback();                           \
      } catch(...) { }                          \
    }

/**
 * std::min name collisions on windows/cygwin
 *
 */
#define TO_MIN qMin

#define QTRANS(x,y) (qApp?qApp->translate(x,y):QString::fromLatin1(y))

/**
 * simple class to provide an empty widget that will resize horizontally
 */
class toSpacer : public QWidget
{
    Q_OBJECT;

public:
    toSpacer(QWidget *parent = NULL);
    ~toSpacer(void)
    {
    }
};

#endif
