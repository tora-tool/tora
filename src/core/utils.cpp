
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

#include "core/utils.h"
#include "core/toconf.h"
#include "core/tomainwindow.h"
#include "core/tosql.h"
#include "core/toconfiguration.h"
#include "core/toconfiguration_new.h"
#include "core/totimer.h"
#include "core/toglobalevent.h"
#include "core/toglobalsetting.h"
#include "core/toqvalue.h"
#include "core/toquery.h"

#include <QtCore/QTextCodec>
#include <QtCore/QThread>
#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QFileDialog>
#include <QtGui/QToolBar>
#include <QtGui/QStyleFactory>
#include <QtGui/QPixmapCache>
#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>
#include <QtGui/QDockWidget>

#include <stdlib.h>
#include <algorithm>

#ifdef Q_OS_WIN32
#   define NOMINMAX
#   include <windows.h>
#endif

#if defined(Q_OS_MACX)
#include <sys/param.h>
//#include <CoreServices/CoreServices.h>
#endif // Q_OS_MACX

#if defined(__linux__)
#include <sys/prctl.h>
#endif


#define CHUNK_SIZE 31

// A little magic to get lrefresh to work and get a check on qApp

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

// toSQL::TOSQL_USERLIST is used to populate toResultSchema

#if 0
namespace Utils
{

static toSQL SQLTextPiece("Global:SQLText",
                          "SELECT SQL_Text\n"
                          "  FROM V$SQLText_With_Newlines\n"
                          " WHERE Address||':'||Hash_Value = :f1<char[100]>\n"
                          " ORDER BY Piece",
                          "Get text of SQL statement.");

//tool QString toSQLString(toConnection &conn, const QString &address)
//tool {
//tool     QString sql;

//tool     toQList vals = toQuery::readQuery(conn, SQLTextPiece, address);

//tool     for (toQList::iterator i = vals.begin(); i != vals.end(); i++)
//tool     {
//tool         sql.append(*i);
//tool     }
//tool     if (sql.isEmpty())
//tool         throw qApp->translate("toSQLString", "SQL Address not found in SGA");
//tool     return sql;
//tool }

QString toSQLStripSpecifier(const QString &sql)
{
    QString ret;
    char inString = 0;
    for (int i = 0; i < sql.length(); i++)
    {
        QChar rc = sql.at(i);
        char c = rc.toLatin1();
        if (inString)
        {
            if (c == inString)
            {
                inString = 0;
            }
            ret += c;
        }
        else
        {
            switch (c)
            {
            case '\'':
                inString = '\'';
                ret += rc;
                break;
            case '\"':
                inString = '\"';
                ret += rc;
                break;
            case ':':
                ret += rc;
                for (i++; i < sql.length(); i++)
                {
                    rc = sql.at(i);
                    c = rc.toLatin1();
                    if (!rc.isLetterOrNumber())
                        break;
                    ret += rc;
                }
                if (c == '<')
                {
                    ret += QString::fromLatin1(" ");
                    for (i++; i < sql.length(); i++)
                    {
                        rc = sql.at(i);
                        c = rc.toLatin1();
                        ret += QString::fromLatin1(" ");
                        if (c == '>')
                        {
                            i++;
                            break;
                        }
                    }
                }
                i--;
                break;
            default:
                ret += rc;
            }
        }
    }
    return ret;
}

QString toSQLStripBind(const QString &sql)
{
    QString ret;
    char inString = 0;
    for (int i = 0; i < sql.length(); i++)
    {
        QChar rc = sql.at(i);
        char  c  = rc.toLatin1(); // current
        char  n  = 0;           // next
        if (i + 1 < sql.length())
            n = sql.at(i + 1).toLatin1();

        if (inString)
        {
            if (c == inString)
            {
                inString = 0;
            }
            ret += rc;
        }
        else
        {
            switch (char(c))
            {
            case '\'':
                inString = '\'';
                ret += rc;
                break;
            case '\"':
                inString = '\"';
                ret += rc;
                break;
            case ':':
                // don't nuke my postgres-style casts
                if (n == ':')
                {
                    ret += rc;
                    ret += n;
                    i++;
                    break;
                }

                ret += QString::fromLatin1("''");
                for (i++; i < sql.length(); i++)
                {
                    rc = sql.at(i);
                    c = rc.toLatin1();
                    if (!rc.isLetterOrNumber())
                        break;
                }
                if (c == '<')
                {
                    for (i++; i < sql.length(); i++)
                    {
                        rc = sql.at(i);
                        c = rc.toLatin1();
                        if (c == '>')
                        {
                            i++;
                            break;
                        }
                    }
                }
                i--;
                break;
            default:
                ret += rc;
            }
        }
    }
    return ret;
}

QString toDeepCopy(const QString &str)
{
    return QString(str.data(), str.length());
}

static toTreeWidgetItem *FindItem(toTreeWidget *lst, toTreeWidgetItem *first, const QString &str)
{
    while (first)
    {
        QString tmp = first->text(0);
        if (tmp == str)
            return first;
        else
        {
            tmp += QString::fromLatin1(":");
            if (str.startsWith(tmp))
            {
                toTreeWidgetItem *ret = FindItem(lst, first->firstChild(), str.mid(tmp.length()));
                if (ret)
                    return ret;
            }
        }
        first = first->nextSibling();
    }
    return NULL;
}

toTreeWidgetItem *toFindItem(toTreeWidget *lst, const QString &str)
{
    return FindItem(lst, lst->firstChild(), str);
}

void toMapExport(std::map<QString, QString> &data, const QString &prefix,
                 std::map<QString, QString> &src)
{
    std::map<QString, QString>::iterator i = src.begin();
    if (i != src.end())
    {
        data[prefix + ":First"] = QString((*i).first);
        QString key = prefix + ":d:";
        do
        {
            data[key + (*i).first] = (*i).second;
            i++;
        }
        while (i != src.end());
    }
}

void toMapImport(std::map<QString, QString> &data, const QString &prefix,
                 std::map<QString, QString> &dst)
{
    dst.clear();
    std::map<QString, QString>::iterator i = data.find(prefix + ":First");
    if (i != data.end())
    {
        QString key = prefix + ":d:";
        i = data.find(key + (*i).second.toLatin1());
        while (i != data.end() && (*i).first.mid(0, key.length()) == key)
        {
            QString t = (*i).first.mid(key.length());
            if (t.isNull())
                t = "";
            dst[t] = (*i).second;
            i++;
        }
    }
}

bool toCheckModal(QWidget *widget)
{
    QWidget *parent = QApplication::activeModalWidget();
    if (!parent)
        return true;
    while (widget && widget != parent)
        widget = widget->parentWidget();
    if (widget == parent)
        return true;
    return false;
}

int countChars(const QString &source, const char find)
{
    int found = 0;

    for (int i = 0; i < source.length(); i++)
    {
        if (source[i] == find)
            found++;
    }

    return found;
}

}
#endif

namespace Utils
{

static toSQL SQLUserNamesMySQL(toSQL::TOSQL_USERLIST,
                               "SHOW DATABASES",
                               "List users in the database",
                               "3.0",
                               "QMYSQL");

static toSQL SQLUserNames(toSQL::TOSQL_USERLIST,
                          "SELECT UserName FROM sys.All_Users ORDER BY UserName",
                          "");

static toSQL SQLUserNamesPgSQL(toSQL::TOSQL_USERLIST,
                               "SELECT nspname AS UserName FROM pg_namespace ORDER BY nspname",
                               "",
                               "7.1",
                               "QPSQL");

static toSQL SQLUserNamesSapDB(toSQL::TOSQL_USERLIST,
                               "SELECT username \"UserName\" FROM users ORDER BY username",
                               "",
                               "",
                               "SapDB");

static toSQL SQLUserNamesTD(toSQL::TOSQL_USERLIST,
                            "SELECT trim ( databasename )\n"
                            "  FROM dbc.UserRights\n"
                            " UNION\n"
                            "SELECT trim ( databasename )\n"
                            "  FROM dbc.UserRoleRights\n"
                            " GROUP BY 1",
                            "",
                            "",
                            "Teradata");

QString toSQLStripSpecifier(const QString &sql)
{
    QString ret;
    char inString = 0;
    for (int i = 0; i < sql.length(); i++)
    {
        QChar rc = sql.at(i);
        char c = rc.toLatin1();
        if (inString)
        {
            if (c == inString)
            {
                inString = 0;
            }
            ret += c;
        }
        else
        {
            switch (c)
            {
            case '\'':
                inString = '\'';
                ret += rc;
                break;
            case '\"':
                inString = '\"';
                ret += rc;
                break;
            case ':':
                ret += rc;
                for (i++; i < sql.length(); i++)
                {
                    rc = sql.at(i);
                    c = rc.toLatin1();
                    if (!rc.isLetterOrNumber())
                        break;
                    ret += rc;
                }
                if (c == '<')
                {
                    ret += QString::fromLatin1(" ");
                    for (i++; i < sql.length(); i++)
                    {
                        rc = sql.at(i);
                        c = rc.toLatin1();
                        ret += QString::fromLatin1(" ");
                        if (c == '>')
                        {
                            i++;
                            break;
                        }
                    }
                }
                i--;
                break;
            default:
                ret += rc;
                break;
            }
        }
    }
    return ret;
}

QString toSQLStripBind(const QString &sql)
{
    QString ret;
    char inString = 0;
    for (int i = 0; i < sql.length(); i++)
    {
        QChar rc = sql.at(i);
        char  c  = rc.toLatin1(); // current
        char  n  = 0;           // next
        if (i + 1 < sql.length())
            n = sql.at(i + 1).toLatin1();

        if (inString)
        {
            if (c == inString)
            {
                inString = 0;
            }
            ret += rc;
        }
        else
        {
            switch (char(c))
            {
            case '\'':
                inString = '\'';
                ret += rc;
                break;
            case '\"':
                inString = '\"';
                ret += rc;
                break;
            case ':':
                // don't nuke my postgres-style casts
                if (n == ':')
                {
                    ret += rc;
                    ret += n;
                    i++;
                    break;
                }

                ret += QString::fromLatin1("''");
                for (i++; i < sql.length(); i++)
                {
                    rc = sql.at(i);
                    c = rc.toLatin1();
                    if (!rc.isLetterOrNumber())
                        break;
                }
                if (c == '<')
                {
                    for (i++; i < sql.length(); i++)
                    {
                        rc = sql.at(i);
                        c = rc.toLatin1();
                        if (c == '>')
                        {
                            i++;
                            break;
                        }
                    }
                }
                i--;
                break;
            default:
                ret += rc;
                break;
            }
        }
    }
    return ret;
}

static toSQL SQLNowMySQL("Global:Now",
                         "SELECT now()",
                         "Get current date/time from database",
                         "3.0",
                         "QMYSQL");
static toSQL SQLNow("Global:Now",
                    "SELECT TO_CHAR(SYSDATE) FROM sys.DUAL",
                    "");

static toSQL SQLNowPgSQL("Global:Now",
                         "SELECT now()",
                         "",
                         "7.1",
                         "QPSQL");

static toSQL SQLNowTD("Global:Now",
                      "SELECT CURRENT_DATE",
                      "",
                      "",
                      "Teradata");

QString toNow(toConnection &conn)
{
    try
    {
        toQList vals = toQuery::readQuery(conn, SQLNow, toQueryParams());
        return toPop(vals);
    }
    catch (...)
    {
        return qApp->translate("toNow", "Unexpected error");
    }
}

toSQL SQLTextPiece("Global:SQLText",
                   "SELECT SQL_Text\n"
                   "  FROM V$SQLText_With_Newlines\n"
                   " WHERE Address||':'||Hash_Value = :f1<char[100]>\n"
                   " ORDER BY Piece",
                   "Get text of SQL statement.");

/** Get the full SQL of an address (See @ref toSQLToAddress) from the
 * SGA.
 * @param conn Connection to get address from
 * @param sql Address of SQL.
 * @return String with SQL of statement.
 * @exception QString if address not found.
 */
QString toSQLString(toConnection &conn, const QString &address)
{
    QString sql;

    toQList vals = toQuery::readQuery(conn, SQLTextPiece, toQueryParams() << address);

    for (toQList::iterator i = vals.begin(); i != vals.end(); i++)
    {
        sql.append(*i);
    }
    if (sql.isEmpty())
        throw qApp->translate("toSQLString", "SQL Address not found in SGA");
    return sql;
}

int toSizeDecode(const QString &str)
{
    if (str == QString::fromLatin1("KB"))
        return 1024;
    if (str == QString::fromLatin1("MB"))
        return 1024 * 1024;
    return 1;
}

bool toCheckModal(QWidget *widget)
{
    QWidget *parent = QApplication::activeModalWidget();
    if (!parent)
        return true;
    while (widget && widget != parent)
        widget = widget->parentWidget();
    if (widget == parent)
        return true;
    return false;
}

#define COLORS 2
#define TYPES 7
#define STYLES 5

QBrush toChartBrush(int index)
{
    index %= (COLORS * TYPES * STYLES);
    index = COLORS * TYPES * STYLES - 1 - index;
    int type = index % TYPES;
    int color = (index / TYPES) % COLORS;
    int style = (index / TYPES / COLORS);
    int r = 0, g = 0, b = 0;
    int offset = 1;
    switch (type)
    {
    case 6:
        r = 1;
        break;
    case 5:
        g = 1;
        break;
    case 4:
        b = 1;
        break;
    case 3:
        r = g = 1;
        break;
    case 2:
        r = b = 1;
        break;
    case 1:
        b = g = 1;
        break;
    case 0:
        r = b = g = 1;
        offset = 0;
        break;
    }

    Qt::BrushStyle rets;

    switch (style)
    {
    default:
        rets = Qt::SolidPattern;
        break;
    case 3:
        rets = Qt::BDiagPattern;
        break;
    case 2:
        rets = Qt::FDiagPattern;
        break;
    case 1:
        rets = Qt::DiagCrossPattern;
        break;
    case 0:
        rets = Qt::CrossPattern;
        break;
    }

    color += offset;

    QBrush brush(QColor((std::min)(255, r * color * 256 / (COLORS)),
                        (std::min)(255, g * color * 256 / (COLORS)),
                        (std::min)(255, b * color * 256 / (COLORS))),
                 rets);
    return brush;
}

void toRefreshParse(toTimer *timer, const QString &str)
{
    QString t = str;
    if (t.isEmpty())
        t = toConfigurationSingle::Instance().refresh();

    if (t == qApp->translate("toRefreshCreate", "None") || t == "None")
        timer->stop();
    else if (t == qApp->translate("toRefreshCreate", "2 seconds") || t == "2 seconds")
        timer->start(2 * 1000);
    else if (t == qApp->translate("toRefreshCreate", "5 seconds") || t == "5 seconds")
        timer->start(5 * 1000);
    else if (t == qApp->translate("toRefreshCreate", "10 seconds") || t == "10 seconds")
        timer->start(10 * 1000);
    else if (t == qApp->translate("toRefreshCreate", "30 seconds") || t == "30 seconds")
        timer->start(30 * 1000);
    else if (t == qApp->translate("toRefreshCreate", "1 min") || t == "1 min")
        timer->start(60 * 1000);
    else if (t == qApp->translate("toRefreshCreate", "5 min") || t == "5 min")
        timer->start(300 * 1000);
    else if (t == qApp->translate("toRefreshCreate", "10 min") || t == "10 min")
        timer->start(600 * 1000);
    else
        throw qApp->translate("toRefreshParse", "Unknown timer value");
}

QString toTranslateMayby(const QString &ctx, const QString &text)
{
    if (ctx.contains(QString::fromLatin1(" ")) ||
            ctx.toLatin1() != ctx.toUtf8() ||
            text.toLatin1() != text.toUtf8() ||
            ctx.isEmpty() ||
            text.isEmpty()
       )
        return text;
    return QT_TRANSLATE_NOOP(ctx.toLatin1(), text.toLatin1());
}

void toReadableColumn(QString &name)
{
    bool inWord = false;
    for (int i = 0; i < name.length(); i++)
    {
        if (name.at(i) == '_')
        {
            name[i] = ' ';
            inWord = false;
        }
        else if (name.at(i).isSpace())
        {
            inWord = false;
        }
        else if (name.at(i).isLetter())
        {
            if (inWord)
                name[i] = name.at(i).toLower();
            else
                name[i] = name.at(i).toUpper();
            inWord = true;
        }
    }
}


bool toCheckKeyEvent(QKeyEvent *event, const QKeySequence &key)
{
    int state = 0;
    if (key.count() != 1)
        return false;
    int val = key[0];
    if ((val & Qt::META) == Qt::META)
        state |= Qt::META;
    if ((val & Qt::SHIFT) == Qt::SHIFT)
        state |= Qt::SHIFT;
    if ((val & Qt::CTRL) == Qt::CTRL)
        state |= Qt::CTRL;
    if ((val & Qt::ALT) == Qt::ALT)
        state |= Qt::ALT;

    val &= 0xfffff;

    return (event->modifiers() == Qt::NoModifier && event->key() == val);
}

QKeySequence toKeySequence(const QString &key)
{
    QKeySequence ret = key;
    // TODO printf? use logging
    if (key.isEmpty() && ret.isEmpty())
        printf("Key sequence %s is not valid\n", key.toAscii().constData());
    return ret;
}

toBusy::toBusy()
{
	if (m_mainThread == NULL)
		m_mainThread = qApp->thread();
	if (m_mainThread == QThread::currentThread() && m_busyCount.fetchAndAddAcquire(1) == 0 && m_enabled)
		qApp->setOverrideCursor(Qt::WaitCursor);
}

toBusy::~toBusy()
{
	if (m_mainThread == QThread::currentThread() && m_busyCount.deref() == false)
		qApp->restoreOverrideCursor();
}

bool toBusy::m_enabled(true);
QAtomicInt toBusy::m_busyCount(0);
QThread *toBusy::m_mainThread(NULL);

QToolBar *toAllocBar(QWidget *parent, const QString &str)
{
    QString db;
    try
    {
        db = toConnection::currentConnection(parent).description(false);
    }
    catch (QString const& e)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception: " << e << std::endl;
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }

    QString name = str;
    if (!db.isEmpty() && toConfigurationNewSingle::Instance().option(ToConfiguration::Global::IncludeDbCaptionBool).toBool())
    {
        name += QString::fromLatin1(" ");
        name += db;
    }

    QToolBar *tool;

    QMainWindow *main = dynamic_cast<QMainWindow *>(parent);
    if (main)
        tool = toMainWindow::lookup()->addToolBar(name);
    else
        tool = new QToolBar(parent);

    // Enforce smaller toolbars on mac
    tool->setIconSize(QSize(16, 16));

    tool->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,
                                    QSizePolicy::Fixed));
    tool->setFocusPolicy(Qt::NoFocus);
    return tool;
}

TODock *toAllocDock(const QString &name,
                    const QString &db,
                    const QPixmap &pix)
{
    QString str = name;
    if (!db.isEmpty() && toConfigurationNewSingle::Instance().option(ToConfiguration::Global::IncludeDbCaptionBool).toBool())
    {
        str += QString::fromLatin1(" ");
        str += db;
    }
    QDockWidget *dock = new QDockWidget(str, toMainWindow::lookup());
    dock->setFeatures(QDockWidget::DockWidgetClosable |
                      QDockWidget::DockWidgetMovable |
                      QDockWidget::DockWidgetFloatable);
    if (!pix.isNull())
        dock->setWindowIcon(QIcon(pix));
    return dock;
}

void toAttachDock(TODock *dock, QWidget *container, Qt::DockWidgetArea area)
{
    QDockWidget *d = dynamic_cast<QDockWidget *>(dock);
    if (d)
    {
        toMainWindow::lookup()->addDockWidget(area, d);
        d->setWidget(container);
        container->show();
    }
}

QStringList toGetSessionTypes(void)
{
    return QStyleFactory::keys();
}

QString toGetSessionType(void)
{
    QStyle  *style = qApp->style();
    QString  sname = style->metaObject()->className();

    // This is probably really slow, but who cares.

    QStringList keys = QStyleFactory::keys();
    for (int i = 0; i < keys.size(); i++)
    {
        QString name = keys[i];
        QStyle *t = QStyleFactory::create(name);
        if (!t)
            continue;
        if (sname == t->metaObject()->className())
        {
            delete t;
            return name;
        }
        delete t;
    }

    // Weird should never get here.
    return sname;
}

void toSetSessionType(const QString &str)
{
    QStyle *style = QStyleFactory::create(str);
    if (style)
        qApp->setStyle(style);
    else
        toStatusMessage(qApp->translate("toSetSessionType", "Failed to find style %1").arg(str));
}

QPixmap connectionColorPixmap(const QString & name)
{
    QPixmap pm;
    if (name.isNull() || name.isEmpty())
        return pm;
    if (!QPixmapCache::find(name, pm))
    {
        // draw a "cool 3d" bullet here
        pm = QPixmap(16, 16);
        pm.fill(Qt::transparent);
        QColor col(name);

        QPainter painter(&pm);
        painter.setRenderHints(QPainter::HighQualityAntialiasing);
        QRadialGradient brush(16 / 2, 16 / 2, 16 * 1.5, 16 / 2, 16 / 4);
        brush.setColorAt(0, col.lighter());
        brush.setColorAt(0.2, col);
        brush.setColorAt(0.6, col.darker());
        brush.setColorAt(1, Qt::black);
        painter.setBrush(brush);

        QPen pen(Qt::black);
        pen.setWidth(1);
        pen.setCosmetic(true);
        painter.setPen(pen);

        painter.drawEllipse(1, 1, 14, 14);
        painter.end();

        QPixmapCache::insert(name, pm);
    }
    return pm;
}

QString toObfuscate(const QString &str)
{
    if (str.isEmpty())
        return str;

    QByteArray arr = qCompress(str.toUtf8());
    QString ret = "\002";

    char buf[100]; // Just to be on the safe side
    for (int i = 0; i < arr.size(); i++)
    {
        sprintf(buf, "%02x", ((unsigned int)arr.at(i)) % 0xff);
        ret += buf;
    }
    return ret;
}

QString toUnobfuscate(const QString &str)
{
    if (str.isEmpty())
        return str;

    if (str.at(0) != '\001' && str.at(0) != '\002')
        return str;

    QByteArray arr;
    for (int i = 1; i < str.length(); i += 2)
        // qt4        arr.at(i / 2) = str.mid(i, 2).toInt(0, 16);
        arr[i / 2] = str.mid(i, 2).toInt(0, 16);
    if (str.at(0) == '\002')
    {
        QByteArray ret = qUncompress(arr);
        return QString::fromUtf8(ret);
    }
    else
        return QString::fromUtf8(arr);
}

QComboBox *toRefreshCreate(QWidget *parent, const char *name, const QString &def, QComboBox *item)
{
    QComboBox *refresh;
    if (item)
        refresh = item;
    else
    {
        refresh = new QComboBox(parent);
        refresh->setObjectName(name);
        refresh->setEditable(false);
    }

    refresh->addItem(qApp->translate("toRefreshCreate", "None"));
    refresh->addItem(qApp->translate("toRefreshCreate", "2 seconds"));
    refresh->addItem(qApp->translate("toRefreshCreate", "5 seconds"));
    refresh->addItem(qApp->translate("toRefreshCreate", "10 seconds"));
    refresh->addItem(qApp->translate("toRefreshCreate", "30 seconds"));
    refresh->addItem(qApp->translate("toRefreshCreate", "1 min"));
    refresh->addItem(qApp->translate("toRefreshCreate", "5 min"));
    refresh->addItem(qApp->translate("toRefreshCreate", "10 min"));
    QString str;
    if (!def.isNull())
        str = def;
    else
        str = toConfigurationSingle::Instance().refresh();
    if (str == "2 seconds")
        refresh->setCurrentIndex(1);
    else if (str == "5 seconds")
        refresh->setCurrentIndex(2);
    else if (str == "10 seconds")
        refresh->setCurrentIndex(3);
    else if (str == "30 seconds")
        refresh->setCurrentIndex(4);
    else if (str == "1 min")
        refresh->setCurrentIndex(5);
    else if (str == "5 min")
        refresh->setCurrentIndex(6);
    else if (str == "10 min")
        refresh->setCurrentIndex(7);
    else
        refresh->setCurrentIndex(0);
    return refresh;
}

void toSetThreadName(QThread const& p)
{
    // This was copied from QT trunk - as of 4.8.2 it is not used yet
    QByteArray objectName = p.objectName().toLatin1();	    
    if (objectName.isEmpty())
	objectName = p.metaObject()->className();	    
#if defined(__linux__)
    prctl(PR_SET_NAME, (unsigned long)objectName.constData(), 0, 0, 0);
#elif defined(Q_OS_MAC)
    pthread_setname_np(objectName.constData());
#endif
}

bool toLibrary::isValidLibrary(QFileInfo path)
{
    if( !path.exists())
        return false;

    QFile lib(path.absoluteFilePath());
    if( !lib.open(QIODevice::ReadOnly))
        return false;

#ifdef Q_OS_LINUX
    static char Elf_ident[16];

    if( lib.read(Elf_ident, sizeof(Elf_ident)) != sizeof(Elf_ident))
        return false;

    if( Elf_ident[0] != 0x7f ||
            Elf_ident[1] != 'E'  ||
            Elf_ident[2] != 'L'  ||
            Elf_ident[3] != 'F'  ||
#ifdef __x86_64__
            Elf_ident[4] != 0x2
#else
            Elf_ident[4] != 0x1
#endif
      )
        return false;

    lib.close();
#endif

#ifdef Q_OS_WIN32
    static char COFF_header[68];
    quint32 offset;
    static char PE_header[6];
    quint16 machine;

    if( lib.read(COFF_header, sizeof(COFF_header)) != sizeof(COFF_header))
        return false;

    if( COFF_header[0] != 'M' ||
            COFF_header[1] != 'Z' )
        return false;

    memcpy(&offset, COFF_header + 60, sizeof(offset));
    if( lib.seek(offset) == false)
        return false;

    if( lib.read(PE_header, sizeof(PE_header)) != sizeof(PE_header))
        return false;

    memcpy(&machine, PE_header + 4, sizeof(machine));
    if( PE_header[0] != 'P' ||
            PE_header[1] != 'E' ||
#ifdef Q_OS_WIN64
            machine != 0x8664
#else
            machine != 0x014c
#endif
      )
        return false;
#endif

    return true;
}

toLibrary::LHandle toLibrary::loadLibrary(QFileInfo const& path)
{
    LHandle handle;
    QString oldDir = QDir::currentPath();
    QDir::setCurrent(path.absolutePath());
#ifndef Q_OS_WIN32
    QString libFilename = path.absoluteFilePath();
    handle = dlopen(libFilename.toStdString().c_str(), RTLD_NOW | RTLD_GLOBAL);
    if( !handle)
    {
        QDir::setCurrent(oldDir);
        throw QString(dlerror());
    }
#else
    QString libpath;
    if(path.exists()) // lib is in CWD or has absolute opath
        libpath = QDir::toNativeSeparators(path.absoluteFilePath()).toStdString().c_str();
    else              // only filename was provided.
        libpath = path.fileName();

    handle = LoadLibraryEx( libpath.toStdString().c_str(), NULL, NULL);
    if( !handle)
    {
        LPVOID lpMsgBuf;
        DWORD dw = GetLastError();

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
        QString errmsg((char*)lpMsgBuf);
        LocalFree(lpMsgBuf);
        QDir::setCurrent(oldDir);
        throw errmsg;
    }
#endif
    QDir::setCurrent(oldDir);
    return handle;
}

bool toLibrary::unLoadLibrary(QFileInfo const& path, LHandle const& hmodule)
{
#ifndef Q_OS_WIN32
    return dlclose(hmodule);
#else
    return FreeLibrary(hmodule);
    // Once the thick client OCI.dll is loaded, it can not be unloaded and no other OCI.dll can be used.
    //{
    //  HANDLE hProc;
    //  HANDLE hHeap = GetProcessHeap();
    //  HMODULE * hmodules = NULL;
    //  DWORD cb = 0;

    //  hProc= GetCurrentProcess(); //OpenProcess( PROCESS_QUERY_INFORMATION |  PROCESS_VM_READ, FALSE, _getpid() );

    //  typedef BOOL (WINAPI *t_EnumProcessModules) (HANDLE, HMODULE*, DWORD, LPDWORD);
    //  static t_EnumProcessModules p_EnumProcessModules = NULL;
    //  if( p_EnumProcessModules == NULL)
    //      p_EnumProcessModules = (t_EnumProcessModules) GetProcAddress (LoadLibrary ("kernel32.dll"), "EnumProcessModules");
    //  if( p_EnumProcessModules == NULL)
    //      p_EnumProcessModules = (t_EnumProcessModules) GetProcAddress (LoadLibrary ("Psapi.dll"), "EnumProcessModules");

    //  typedef BOOL (WINAPI *t_GetModuleInformation) (HANDLE, HMODULE, LPMODULEINFO, DWORD);
    //  static t_GetModuleInformation p_GetModuleInformation = NULL;
    //  if(  p_GetModuleInformation == NULL)
    //      p_GetModuleInformation = (t_GetModuleInformation) GetProcAddress (LoadLibrary ("kernel32.dll"), "GetModuleInformation");
    //  if(  p_GetModuleInformation == NULL)
    //      p_GetModuleInformation = (t_GetModuleInformation) GetProcAddress (LoadLibrary ("Psapi.dll"), "GetModuleInformation");

    //  typedef BOOL (WINAPI *t_GetModuleFileNameEx) (HANDLE, HMODULE, LPTSTR, DWORD);
    //  static t_GetModuleFileNameEx p_GetModuleFileNameExA = NULL;
    //  if(  p_GetModuleFileNameExA == NULL)
    //      p_GetModuleFileNameExA = (t_GetModuleFileNameEx) GetProcAddress (LoadLibrary ("kernel32.dll"), "GetModuleFileNameExA");
    //  if(  p_GetModuleFileNameExA == NULL)
    //      p_GetModuleFileNameExA = (t_GetModuleFileNameEx) GetProcAddress (LoadLibrary ("Psapi.dll"), "GetModuleFileNameExA");
    //  for(;;)
    //  {
    //      DWORD cbNeeded = 0;

    //      if(!p_EnumProcessModules(hProc, hmodules, cb, &cbNeeded))
    //          return NULL;

    //      if(cb >= cbNeeded)
    //      {
    //          cb = cbNeeded;
    //          break;
    //      }

    //      PVOID p = HeapAlloc(hHeap, 0, cbNeeded);
    //      HeapFree(hHeap, 0, hmodules);

    //      if(p == NULL)
    //          return NULL;

    //      hmodules = (HMODULE *)p;
    //      cb = cbNeeded;
    //  }

    //  DWORD i = 0;
    //  DWORD ccModules = cb / sizeof(HMODULE);
    //  HMODULE hm = NULL;

    //  for(; i < ccModules; ++ i)
    //  {
    //      MODULEINFO modinfo;
    //      TCHAR szModName[MAX_PATH];

    //      if(!p_GetModuleInformation(hProc, hmodules[i], &modinfo, sizeof(modinfo)))
    //          continue;

    //      if (!p_GetModuleFileNameExA( hProc, hmodules[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
    //          continue;

    //      printf( TEXT("\t%s (0x%08X)\n"), szModName, hmodules[i] );

    //      hm = (HMODULE)modinfo.lpBaseOfDll;

    //      QFileInfo lib(szModName);
    //      if( ocilib.dir() == lib.dir())
    //      {
    //          bool ret;
    //          ret = FreeLibrary(hm);
    //          TLOG(5, toNoDecorator, __HERE__) << "freeing lib: " << lib.absoluteFilePath()  << ' ' << ret << std::endl;
    //      }
    //      QString fn = lib.fileName();
    //      if( lib.fileName().startsWith("oracommon", Qt::CaseInsensitive))
    //          break;
    //  }
    //}
#endif
}

void * toLibrary::lookupSymbol(LHandle const& handle, char const* symbol)
{
#ifndef Q_OS_WIN32
    void *retval = dlsym(handle, symbol);
    const char *dlsym_error = dlerror();
    if( !retval || dlsym_error)
    {
        // TODO throw here?
        return NULL;
    }
    return retval;
#else
    return GetProcAddress (handle, symbol);
#endif
}

bool toCompareLists(QStringList &lsta, QStringList &lstb, int len)
{
    if (lsta.count() < len || lstb.count() < len)
        return false;
    for (int i = 0; i < len; i++)
        if (lsta[i] != lstb[i])
            return false;
    return true;
}

} // namespace utils
