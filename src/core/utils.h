
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

#ifndef UTILS_H
#define UTILS_H

#include "core/tora_export.h"

#include <QtCore/QAtomicInt>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QMessageBox>
#include <QMainWindow>

#include <map>

#ifdef Q_OS_WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <stdlib.h>
#include <dlfcn.h>
#endif

#if QT_VERSION >= 0x050000
#define TO_ESCAPE(string) string.toHtmlEscaped()
#else
#define TO_ESCAPE(string) Qt::escape(string)
#endif

#define TOPrinter QPrinter
#define TOFileDialog QFileDialog
#define TOMessageBox QMessageBox
#define TODock QWidget

#if QT_VERSION >= 0x050000
typedef Qt::WindowFlags toWFlags;
#else
typedef Qt::WFlags toWFlags;
#endif

class QComboBox;
class toTimer;
class toConnection;
class toConnectionRegistry;

// Copied form _Noreturn.h
#if !defined _Noreturn && __STDC_VERSION__ < 201112
# if (3 <= __GNUC__ || (__GNUC__ == 2 && 8 <= __GNUC_MINOR__) \
      || 0x5110 <= __SUNPRO_C)
#  define _Noreturn __attribute__ ((__noreturn__))
# elif 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn
# endif
#endif

#define QTRANS(x,y) (qApp?qApp->translate(x,y):QString::fromLatin1(y))

// Source http://blog.qt.digia.com/blog/2008/10/09/coding-tip-pretty-printing-enum-values/
// This macro translates enum value into human readable string (if registered using Q_ENUMS)
// Usage: QString txt = QLatin1String(ENUM_VALUE(toHighlightedTextEditor, HighlighterTypeEnum , enumValue));
#define ENUM_NAME(o,e,v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))

#define ENUM_REF(o,e) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)))

namespace Utils
{

    /** Get a pointer to the main window
     * @return Pointer to main window.
     */
    TORA_EXPORT QMainWindow* toQMainWindow();

    /**Get the current database time in the current sessions dateformat.
     * @param conn Connection to get address from.
     * @return String with the current date and time.
     */
    QString toNow(toConnection &conn);

    /** Get hash of SQL statement using Oracles' internal algorithm
     * @param sql statement
     * @return sql_id
     */
    QString toSQLToSql_Id(const QString &sql);

    /**Get the full SQL of an address (See @ref toSQLToAddress) from the
     * SGA.
     * @param conn Connection to get address from
     * @param sql Address of SQL.
     * @return String with SQL of statement.
     * @exception QString if address not found.
     */
    QString toSQLString(toConnection &conn, const QString &address);

    /** Decode a size string this basically converts "KB" to 1024, "MB" to 1024KB and
     * everything else to 1.
     * @return Multiplier specified by string.
     */
    int toSizeDecode(const QString &str);

    /** Check if the current widget is within the active modal widget (Or no modal widget exists)
     */
    bool toCheckModal(QWidget *widget);

    /** Get a brush for a chart item.
     * @param index Indicating which chart item to get color for.
     */
    QBrush toChartBrush(int index);

    /** Strip extra binds and replace with empty strings.
     * @param sql The sql to strip.
     * @return Return a string containing the same statement without binds.
     */
    QString toSQLStripBind(const QString &sql);

    /** Strip extra bind specifier from an SQL statement. (That means the extra <***> part after
     * the bind variable.
     * @param sql The sql to strip.
     * @return Return a string containing the same statement without qualifiers, which means the
     *         sql sent to Oracle and available in the SGA.
     */
    QString toSQLStripSpecifier(const QString &sql);

    /** Check if a character is valid for an identifier in Oracle.
     * @param c Character to check
     * @return True if it is a valid Oracle identifier.
     */
    inline bool toIsIdent(QChar c)
    {
        return c.isLetterOrNumber() || c == '_' || c == '%' || c == '$' || c == '#';
    }

    /** Set a timer with the value from a refresh combobox (See @ref toRefreshCreate).
     * @param timer Timer to set timeout in.
     * @param str String from currentText of combobox. If empty, set to default.
     */
    void toRefreshParse(toTimer *timer, const QString &str = QString::null);

    /**
     * Translate string if the context and text is strictly ASCII7 and the context doesn't
     * contain any spaces. Otherwise just return the text.
     * @param context The context of the text.
     * @param text The text to translate.
     * @return Translated text if appropriate, or original text otherwise.
     */
    QString toTranslateMayby(const QString &context, const QString &text);

    /** Make a column name more readable.
    * @param col Name of column name, will be modified.
    */
    void toReadableColumn(QString &col);

    /**
     * Check if a key sequence and a key event represent the same key and return if that is the case.
     * Only support single key keysequences.
     * @param event Key event.
     * @param key Key sequence.
     */
    bool toCheckKeyEvent(QKeyEvent *event, const QKeySequence &key);

    /**
    * Check that a key sequence is valid and return same sequence
    */
    QKeySequence toKeySequence(const QString &keysequence);

    /** Whenever this class is instantiated the window will display a busy cursor. You
    * can instantiate this function as many times as you want, only when all of them are
    * destructed the cursor will revert back to normal.
    * If you want to control if busy cursor is actually shown for a particular case, you
    * can use a boolean parameter in constructor. For example debugger is setting this as
    * false in order for cursor NOT to be busy until program unit being debugged finishes.
    */
    class toBusy
    {
            friend class ::toConnectionRegistry;
        public:
            toBusy();
            ~toBusy();
        protected:
            static bool m_enabled; // disabled when main application window is already closed
        private:
            static QAtomicInt m_busyCount;
            static QThread* m_mainThread; // no nothing if instantiated from bg thread
    };

    /** Display a message in the statusbar of the main window.
    * @param str Message to display
    * @param save If true don't remove the message after a specified interval.
    * @param log Log message. Will never log saved messages.
    */
    TORA_EXPORT void toStatusMessage(const QString &str, bool save = false, bool log = true);

    /* This can't be documented in KDoc, anyway it is an easy way to catch any exception that
    * might be sent by TOra or OTL and display the message in the statusbar of the main window.
    */
#define TOCATCH                                 \
    catch (const QString &str) {                \
        Utils::toStatusMessage(str);                \
    }

    /* This can't be documented in KDoc, anyway it is an easy way to catch any exception that
    * might be sent by TOra or OTL and display the message in the statusbar of the main window.
    */
#if 0
#define TOROLLBACK(x)                           \
    catch (const QString &str) {                \
        toStatusMessage(str);                     \
        try {                                     \
            x.rollback();                           \
        } catch(...)                              \
        {                                         \
            get_log(1).ts<toDecorator>( __HERE__) << "  Ignored exception." << std::endl; \
        }                       \
    }
#endif

    /**
     * Get extensions for file save dialog from editor configuration
     *
     * @return Extensions.
     */
    QString GetExtensions(void);

    /** Pop up a dialog and choose a file to open.
    * @param filename Default filename to open.
    * @param filter Filter of filenames (See @ref QFileDialog)
    * @param parent Parent of dialog.
    * @return Selected filename.
    */
    QString toOpenFilename(const QString &filename, const QString &filter, QWidget *parent);

    /** Pop up a dialog and choose a file to open. Uses config property LastDir.
     * @param filter Filter of filenames (See @ref QFileDialog)
     * @param parent Parent of dialog.
     * @return Selected filename.
     */
    QString toOpenFilename(const QString &filter, QWidget *parent);

    /** Pop up a dialog and choose a file to save to.
    * @param filename Default filename to open.
    * @param filter Filter of filenames (See @ref QFileDialog)
    * @param parent Parent of dialog.
    * @return Selected filename.
    */
    QString toSaveFilename(const QString &filename, const QString &filter, QWidget *parent);

    /** Read file from filename and decode it according to current locale settings.
    * @param filename Filename to read file from.
    * @return Contents of file.
    * @exception QString describing I/O problem.
    */
    TORA_EXPORT QString toReadFile(const QString &filename);

    /** Read file from filename and return it as binary data (no decoding).
    * @param filename Filename to read file from.
    * @return Contents of file.
    * @exception QString describing I/O problem.
    */
    TORA_EXPORT QByteArray toReadFileB(const QString &filename);

    /*Write large data into a QFile
     *  T is instance of class that implements method "QByteArray next()" this method is continuesly called till whole data is written
     * @param file openned filehandle to write into
     * @param reference to a class that implements a method "nextData()"
     * @param progressbar show progressbar(true, false)
     * @param parent parent widget(for progressbar)
     */
    template<class T> bool toWriteLargeFile(QFile &file, T const &source, bool progressbar, QWidget *parent)
    {
        // TODO add progressbar
        while (true)
        {
            QByteArray const &data = source.nextData();
            if (data.isEmpty()) // End of stream is reported as an empty QByteArray
                return true;
            file.write(data);
            if (file.error() != QFile::NoError)
            {
                TOMessageBox::warning(
                    toQMainWindow(),
                    QT_TRANSLATE_NOOP("toWriteLargeFile", "File error"),
                    QT_TRANSLATE_NOOP("toWriteLargeFile", "Couldn't write data to file"));
                return false;
            }
        }
        return false;
    };

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

    /** Convert a font to a string representation.
     * @param fnt Font to convert.
     * @return String representation of font.
     */
    QString toFontToString(const QFont &fnt);

    /** Convert a string representation to a font structure.
    * @param str String representing the font.
    * @return Font structure represented by the string.
    * @see toFontToString
    */
    QFont toStringToFont(const QString &str);

    /** Allocate a toolbar. This is needed since Qt and KDE use different toolbars.
    * @param parent Parent of toolbar.
    * @param name Name of toolbar.
    * @return A newly created toolbar.
    */
    QToolBar *toAllocBar(QWidget *parent, const QString &name);

    TODock *toAllocDock(const QString &name,
                        const QString &db,
                        const QPixmap &pix);

    void toAttachDock(TODock *dock, QWidget *container, Qt::DockWidgetArea area);

    /** Push an object to the end of a list.
    * @param lst List to push value in from of.
    * @param str Object to push.
    */
    template <class T>
    void toPush(std::list<T> &lst, const T &str)
    {
        lst.push_back(str);
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

    /** Get current session type (Style)
    * @return A string describing the current style.
    * @see toSetSessionType
    */
    QString toGetSessionType(void);

    /** Set the current session type (Style)
    * @param str Session to set, can be any of Motif, Motif Plus, SGI, CDE, Windows and Platinum
    * @exception QString if style not available.
    */
    void toSetSessionType(const QString &str);

    /** Create or fill a combobox with refresh intervals.
    * @param parent Parent of created combobox.
    * @param name Name of created combobox.
    * @param def Default value of the combobox.
    * @param item Combo box to fill. If not specified a new combobox is created.
    */
    QComboBox *toRefreshCreate(QWidget *parent, const char *name = NULL, const QString & def = QString::null,
                               QComboBox * item = NULL);

    QPixmap connectionColorPixmap(const QString & name);

    /** Take a string and make it illegible. Some security through obscurity here so you will
     * need to check the source to see what is actually done.
     * @param str String to be made unreadable.
     * @return Obfuscated string.
     */
    TORA_EXPORT QString toObfuscate(const QString &str);

    /** Unmake the actions of @ref toObfuscate.
    * @param Obfuscated string.
    * @return Original string.
    */
    TORA_EXPORT QString toUnobfuscate(const QString &str);

    /** Set name for the current thread. Should be called from run method
     *  @param use "*this" as parameter
     *
     * NOTE: body of this function is copied from QT trunk.
     * This is not used in the current QT release(4.8.2)
     */
    TORA_EXPORT void toSetThreadName(QThread const& p);

    /** This library has static clasess only
    */
    class toLibrary
    {
        public:
#ifdef Q_OS_WIN32
            typedef HMODULE LHandle;
#else
            typedef void* LHandle;
#endif

            /** Briefly check the library file
            * @param path to .so/.dll file
            * @return return true if the library matches our ELFCLASS
            */
            static bool isValidLibrary(QFileInfo path);

            static LHandle loadLibrary(QFileInfo const&);

            static bool unLoadLibrary(QFileInfo const&, LHandle const&);

            static void * lookupSymbol(LHandle const&, char const*);
    };

    /**
     * simple class to provide an empty widget that will resize horizontally */
    class toSpacer : public QWidget
    {
            Q_OBJECT;

        public:
            toSpacer(QWidget *parent = NULL) : QWidget(parent)
            {
                setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
            }

            inline ~toSpacer(void) {}
    };

    /**Compare two string lists.
     * @param l1 First list to compare.
     * @param l2 Second list to compare.
     * @param len Length of lists to compare.
     * @return True if all first len elements match.
     */
    bool toCompareLists(QStringList &l1, QStringList &l2, int len);

    /** Return the next power of two for a positive integer
     * 1=>2, 2=>4, 3=>4, 4=>8, 5=>8
     * */
    unsigned toNextPowerOfTwo(unsigned len);

    void toLoadMap(const QString &filename, std::map<QString, QString> &pairs);
    bool toSaveMap(const QString &file, std::map<QString, QString> &pairs);

}

#endif
