
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

#include "core/utils.h"
#include "core/toconf.h"
#include "core/toconfiguration.h"
#include "core/toglobalevent.h"
#include "core/tomainwindow.h"

#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QtGui/QFileDialog>

// A little magic to get lrefresh to work and get a check on qApp
#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

namespace Utils {

QMainWindow* toQMainWindow()
{
    static QMainWindow *main = NULL;
    if (main)
        return main;

    QWidgetList widgets = qApp->topLevelWidgets();
    for (QWidgetList::iterator it = widgets.begin(); it != widgets.end(); it++)
    {
        main = dynamic_cast<QMainWindow *>((*it));
        if (main)
            return main;
    }
    return NULL;
    // TODO: throw something here
}
  
// This code is taken from (Q)Scintilla source
#define T_EOL_CRLF 0
#define T_EOL_CR 1
#define T_EOL_LF 2  
void changeLineEnds(QByteArray * text, int eolModeSet)
{
    for (int pos = 0; pos < text->length(); pos++)
    {
        if (text->at(pos) == '\r')
        {
            if (text->at(pos + 1) == '\n')
            {
                // CRLF
                if (eolModeSet == T_EOL_CR)
                {
                    text->remove(pos + 1, 1); // Delete the LF
                }
                else if (eolModeSet == T_EOL_LF)
                {
                    text->remove(pos, 1); // Delete the CR
                }
                else
                {
                    pos++;
                }
            }
            else
            {
                // CR
                if (eolModeSet == T_EOL_CRLF)
                {
                    text->insert(pos + 1, "\n"); // Insert LF
                    pos++;
                }
                else if (eolModeSet == T_EOL_LF)
                {
                    text->insert(pos, "\n"); // Insert LF
                    text->remove(pos + 1, 1); // Delete CR
                }
            }
        }
        else if (text->at(pos) == '\n')
        {
            // LF
            if (eolModeSet == T_EOL_CRLF)
            {
                text->insert(pos, "\r"); // Insert CR
                pos++;
            }
            else if (eolModeSet == T_EOL_CR)
            {
                text->insert(pos, "\r"); // Insert CR
                text->remove(pos + 1, 1); // Delete LF
            }
        }
    }
} // changeLineEnds

QString GetExtensions(void)
{
    static QRegExp repl(QString::fromLatin1("\\s*,\\s*"));
    QString t(toConfigurationSingle::Instance().extensions());
    t.replace(repl, QString::fromLatin1(";;")); // multiple filters are separated by double semicolons
    return t;
}

static QString AddExt(QString t, const QString &filter)
{
    static QRegExp hasext(QString::fromLatin1("\\.[^\\/]*$"));
    if (t.isEmpty())
        return t;

    toConfigurationSingle::Instance().setLastDir(t);

    if (hasext.indexIn(t) < 0)
    {
        static QRegExp findext(QString::fromLatin1("\\.[^ \t\r\n\\)\\|]*"));
        int len = 0;
        int pos = findext.indexIn(filter, 0);
        len = findext.matchedLength();
        if (pos >= 0)
            t += filter.mid(pos, len);
        else
        {
            QFile file(t);
            if (!file.exists())
                t += QString::fromLatin1(".sql");
        }
    }
    return t;
}

/* Get encoding used to read/write files.
*/
QTextCodec * toGetCodec(void)
{
    QString codecConf = toConfigurationSingle::Instance().encoding();
    if (codecConf == DEFAULT_ENCODING)
        return QTextCodec::codecForLocale();
    else
        return QTextCodec::codecForName(codecConf.toAscii());
} // toGetCodec

QString toExpandFile(const QString &file)
{
    QString ret(file);
    QString home;

//#ifdef Q_OS_WIN32
//
//      CRegistry registry;
//      DWORD siz = 1024;
//      char buffer[1024];
//      try
//      {
//          if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
//              "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
//              "Personal",
//              buffer, siz))
//          {
//              if (siz > 0)
//                  home = buffer;
//          }
//      }
//      catch (...)
//      {
//          TLOG(1,toDecorator,__HERE__) << "   Ignored exception." << std::endl;
//      }
//
//#else
    home = QDir::homePath();
//#endif

    ret.replace(QRegExp(QString::fromLatin1("\\$HOME")), home);
    return ret;
}

QString toOpenFilename(const QString &filename, const QString &filter, QWidget *parent)
{
    QString t = filter;
    QString retval;

    if (t.isEmpty())
        t = GetExtensions();

    QFileInfo fi(filename);
    if (!filename.isEmpty() && fi.absoluteDir().exists())
        retval = AddExt(TOFileDialog::getOpenFileName(parent, QObject::tr("Open File", "utils/toOpenFilename"), fi.absoluteFilePath(), t), t);
    else
    {
        QString const& lastDir = toConfigurationSingle::Instance().lastDir();
        retval = AddExt(TOFileDialog::getOpenFileName(parent, QObject::tr("Open File", "utils/toOpenFilename"), lastDir, t), t);
    }
    if (!retval.isEmpty())
        toConfigurationSingle::Instance().setLastDir(retval);
    return retval;
}

QString toOpenFilename(const QString &filter, QWidget *parent)
{
    QFileInfo fi(toConfigurationSingle::Instance().lastDir());
    if ( fi.absoluteDir().exists())
        return toOpenFilename( fi.absolutePath(), filter, parent);
    return toOpenFilename(QDir::currentPath(), filter, parent);
}

QString toSaveFilename(const QString &filename, const QString &filter, QWidget *parent)
{
    QString t = filter;
    if (t.isEmpty())
        t = GetExtensions();

    QString dir = filename;
    if (dir.isNull())
        dir = toConfigurationSingle::Instance().lastDir();

    return AddExt(TOFileDialog::getSaveFileName(parent, QObject::tr("Save File", "utils/toSaveFilename"), dir, t), t);
}

QString toReadFile(const QString &filename)
{
    QTextCodec *codec = toGetCodec();
    return codec->toUnicode(toReadFileB(filename));
}

QByteArray toReadFileB(const QString &filename)
{
    QString expanded = toExpandFile(filename);
    // for some reason qrc:/ urls fail with QFile but are required for
    // QTextBrowser
    if (expanded.startsWith("qrc"))
        expanded = expanded.right(expanded.length() - 3);

    QFile file(expanded);
    if (!file.open(QIODevice::ReadOnly))
        throw QT_TRANSLATE_NOOP("toReadFile", "Couldn't open file %1.").arg(expanded);

    return file.readAll();
}

// saves a QString to filename,
// encoded according to the current locale settings
bool toWriteFile(const QString &filename, const QString &data)
{
    return toWriteFile(filename, data.toLocal8Bit());
}

// saves a QByteArray (binary data) to filename
bool toWriteFile(const QString &filename, const QByteArray &data)
{
    QString expanded = toExpandFile(filename);
    QFile file(expanded);
    if (!file.open(QIODevice::WriteOnly))
    {
        TOMessageBox::warning(
            toQMainWindow(),
            QT_TRANSLATE_NOOP("toWriteFile", "File error"),
            QT_TRANSLATE_NOOP(
                "toWriteFile",
                QString("Couldn't open %1 for writing").arg(filename).toAscii().constData()));
        return false;
    }
    QTextCodec *codec = toGetCodec();

    // Check if line end type should be changed to particular one
    // Note that line end type can be changed manually via menu
    QString forceLineEndSetting = toConfigurationSingle::Instance().forceLineEnd();
    if (forceLineEndSetting == "Linux" ||
            forceLineEndSetting == "Windows" ||
            forceLineEndSetting == "Mac")
    {
        QByteArray ba = data;
        if (toConfigurationSingle::Instance().forceLineEnd() == "Linux")
            changeLineEnds(&ba, T_EOL_LF);
        else if (toConfigurationSingle::Instance().forceLineEnd() == "Windows")
            changeLineEnds(&ba, T_EOL_CRLF);
        else if (toConfigurationSingle::Instance().forceLineEnd() == "Mac")
            changeLineEnds(&ba, T_EOL_CR);
        file.write(codec->fromUnicode(ba));
    }
    else
        file.write(codec->fromUnicode(data));

    if (file.error() != QFile::NoError)
    {
        TOMessageBox::warning(
            toQMainWindow(),
            QT_TRANSLATE_NOOP("toWriteFile", "File error"),
            QT_TRANSLATE_NOOP("toWriteFile", "Couldn't write data to file"));
        return false;
    }
    toStatusMessage(QT_TRANSLATE_NOOP("toWriteFile", "File saved successfully"), false, false);
    return true;
}

void toStatusMessage(const QString &str, bool save, bool log)
{
    // If there is no main widget yet (e.g. style init error)
    // display error message in standard dialog.
    if (toMainWindow::lookup())
        toGlobalEventSingle::Instance().showMessage(str, save, log);
    else if (!str.isEmpty())
        QMessageBox::warning(toQMainWindow(), qApp->translate("toStatusMessage", "TOra Message"), str );
}
  
QString toFontToString(const QFont &fnt)
{
    return fnt.toString();
}

QFont toStringToFont(const QString &str)
{
    if (str.isEmpty())
        return QFont(QString::fromLatin1("Courier New"), 12);
    QFont fnt;
    if (fnt.fromString(str))
        return fnt;

    return QFont(QString::fromLatin1("Courier New"), 12);
}

/** Return next power of two for positive integer( 1=>2, 2=>4, 3=>4, 4=>8, 5=>8) */
unsigned toNextPowerOfTwo(unsigned len)
{
	unsigned retval = 2;
	while( len >>= 1)
		retval <<= 1;
	return retval;
}

} // namespace utils
