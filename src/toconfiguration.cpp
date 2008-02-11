#include "toconfiguration.h"

#include <qapplication.h>
#include <qregexp.h>

#include "utils.h"

#ifdef Q_OS_WIN32
# include "windows.h"
# include "winreg.h"
# define APPLICATION_NAME "SOFTWARE\\tora\\"

static char *toKeyPath(const QString &str)
{
    static char *buf = NULL;
    HKEY         key = NULL;

    int pos = str.length() - 1;
    while (pos >= 0 && str.at(pos) != '\\')
        pos--;
    if (pos < 0)
        throw QT_TRANSLATE_NOOP("toKeyPath", "Couldn't find \\ in path");
    QString ret = str.mid(0, pos);
    if (buf)
        free(buf);
    buf = strdup(ret.toLatin1());
    RegCreateKey(HKEY_CURRENT_USER, buf, &key);
    RegCloseKey(key);
    return buf;
}


static char* toKeyValue(const QString &str)
{
    static char *buf = NULL;
    int pos = str.length() - 1;
    while (pos >= 0 && str.at(pos) != '\\')
        pos--;
    if (pos < 0)
        throw QT_TRANSLATE_NOOP("toKeyValue", "Couldn't find \\ in path");
    if (buf)
        free(buf);
    buf = strdup(str.mid(pos + 1).toLatin1());
    return buf;
}

#endif /* Q_OS_WIN32 */

toConfiguration::toConfiguration()
{

}

toConfiguration::~toConfiguration()
{
}

void toConfiguration::loadConfig()
{
    Configuration.clear();

#ifndef Q_OS_WIN32

    QString conf;
    if (getenv("HOME"))
    {
        conf = QString::fromLatin1(getenv("HOME"));
    }
    conf.append(QString::fromLatin1(CONFIG_FILE));
    try
    {
        loadMap(conf, Configuration);
    }
    catch (...)
    {
        try
        {
            loadMap(QString::fromLatin1(DEF_CONFIG_FILE), Configuration);
        }
        catch (...)
            {}
    }
#endif
}

void toConfiguration::loadMap(const QString &filename, std::map<QString, QString> &pairs)
{
    QByteArray data = toReadFile(filename);

    int pos = 0;
    int bol = 0;
    int endtag = -1;
    int wpos = 0;
    int size = data.length();
    while (pos < size)
    {
        switch (data[pos])
        {
        case '\n':
            data[wpos] = 0;
            if (endtag == -1)
                throw QT_TRANSLATE_NOOP("toTool", "Malformed tag in config file. Missing = on row. (%1)").arg(QString(data.mid(bol, wpos - bol)));
            {
                QString tag = ((const char *)data) + bol;
                QString val = ((const char *)data) + endtag + 1;
                pairs[tag] = val;
            }
            bol = pos + 1;
            endtag = -1;
            wpos = pos;
            break;
        case '=':
            if (endtag == -1)
            {
                endtag = pos;
                data[wpos] = 0;
                wpos = pos;
            }
            else
                data[wpos] = data[pos];
            break;
        case '\\':
            pos++;
            switch (data[pos])
            {
            case 'n':
                data[wpos] = '\n';
                break;
            case '\\':
                if (endtag >= 0)
                    data[wpos] = '\\';
                else
                    data[wpos] = ':';
                break;
            default:
                throw QT_TRANSLATE_NOOP("toTool", "Unknown escape character in string (Only \\\\ and \\n recognised)");
            }
            break;
        default:
            data[wpos] = data[pos];
        }
        wpos++;
        pos++;
    }
    return ;
}

void toConfiguration::saveConfig()
{
    try
    {
#ifdef Q_OS_WIN32
        for (std::map<QString, QString>::iterator i = Configuration.begin();
                i != Configuration.end(); i++)
        {

            QString path = (*i).first;
            QString value = (*i).second;
            path.prepend(APPLICATION_NAME);
            path.replace(':', '\\');

            HKEY key = NULL;
            if (RegOpenKeyA(HKEY_CURRENT_USER, toKeyPath(path), &key) != ERROR_SUCCESS)
            {
                toStatusMessage(QT_TRANSLATE_NOOP(
                                    "toTool",
                                    "Couldn't open key %1").arg(path));
                break;
            }

            if (value.isEmpty())
            {
                if (RegSetValueA(key,
                                 toKeyValue(path),
                                 REG_SZ,
                                 "",
                                 0) != ERROR_SUCCESS)
                {
                    toStatusMessage(QT_TRANSLATE_NOOP(
                                        "toTool",
                                        "Couldn't save empty value at key %1").arg(path));
                }
            }
            else
            {
                char *t = strdup(value.toUtf8());
                if (RegSetValueA(key,
                                 toKeyValue(path),
                                 REG_SZ,
                                 t,
                                 value.length()) != ERROR_SUCCESS)
                {
                    toStatusMessage(QT_TRANSLATE_NOOP(
                                        "toTool",
                                        "Couldn't save %1 value at key %2").arg(value).arg(path));
                }
                free(t);
            }

            RegCloseKey(key);
        }
#else
        if (Configuration.empty())
            return;
        QString conf;
        if (getenv("HOME"))
        {
            conf = getenv("HOME");
        }
        conf.append(CONFIG_FILE);
        saveMap(conf, Configuration);
#endif
    }
    TOCATCH
}

bool toConfiguration::saveMap(const QString &file, std::map<QString, QString> &pairs)
{
    QString data;

    {
// qt4        QRegExp newline(QString::fromLatin1("\n"));
// qt4        QRegExp backslash(QString::fromLatin1("\\"));
        char* newline = "\n";
        char* backslash = "\\";
        for (std::map<QString, QString>::iterator i = pairs.begin();i != pairs.end();i++)
        {
            QString str = (*i).first;
            str.append(QString::fromLatin1("="));
            str.replace(backslash, QString::fromLatin1("\\\\"));
            str.replace(newline, QString::fromLatin1("\\n"));
            QString line = (*i).second;
            line.replace(backslash, QString::fromLatin1("\\\\"));
            line.replace(newline, QString::fromLatin1("\\n"));
            str += line.toUtf8();
            str += QString::fromLatin1("\n");
            data += str;
        }
    }
    return toWriteFile(file, data);
}

const QString &toConfiguration::globalConfig(const QString &tag, const QString &def)
{
    if (Configuration.empty())
        loadConfig();

    std::map<QString, QString>::iterator i = Configuration.find(tag);
    if (i == Configuration.end())
    {

#if defined(Q_OS_WIN32)

        QString path = tag;
        path.prepend(APPLICATION_NAME);
        path.replace(':', '\\');
        LONG siz = 1024;
        char buffer[1024];
        try
        {
            HKEY key = NULL;
            if (RegOpenKeyA(HKEY_CURRENT_USER, toKeyPath(path), &key) != ERROR_SUCCESS)
            {
                toStatusMessage(QT_TRANSLATE_NOOP(
                                    "toTool",
                                    "Couldn't open key %1").arg(path));
            }
            else
            {
                if (RegQueryValueA(key,
                                   toKeyValue(path),
                                   buffer,
                                   &siz) == ERROR_SUCCESS)
                {
                    if (siz > 0)
                    {
                        QString ret = QString::fromUtf8(buffer);
                        (Configuration)[tag] = ret;
                    }
                    else
                        (Configuration)[tag] = "";

                    return (Configuration)[tag];
                }

                RegCloseKey(key);
            }
        }
        catch (...)
        {
            toStatusMessage(QT_TRANSLATE_NOOP(
                                "toTool",
                                "Couldn't get value at key %1").arg(path));
        }
#endif

        (Configuration)[tag] = QString(def);
        return (Configuration)[tag];
    }

    return (*i).second;
}



const QString& toConfiguration::config(const QString &tag, const QString &def, const QString &name)
{
    QString str = name;
    str.append(":");
    str.append(tag);
    return globalConfig(str, def);
}

void toConfiguration::eraseConfig(const QString &tag, const QString &name)
{
    QString str = name;
    str.append(":");
    str.append(tag);
    globalEraseConfig(str);
}

void toConfiguration::setConfig(const QString &tag, const QString &def, const QString name)
{
    QString str = name;
    str.append(":");
    str.append(tag);
    globalSetConfig(str, def);
}


void toConfiguration::globalEraseConfig(const QString &tag)
{
    if (Configuration.empty())
        loadConfig();
    std::map<QString, QString>::iterator i = Configuration.find(tag);
    if (i != Configuration.end())
    {
        Configuration.erase(i);

#if defined(Q_OS_WIN32)
        QString path = tag;
        path.prepend(APPLICATION_NAME);
        path.replace(':', '\\');
        RegDeleteKeyA(HKEY_CURRENT_USER, path.toLatin1()); // Don't really care if it works.
#endif

    }
}

void toConfiguration::globalSetConfig(const QString &tag, const QString &value)
{
    if ( Configuration.empty() )
        loadConfig();

    (Configuration)[tag] = value;
}
