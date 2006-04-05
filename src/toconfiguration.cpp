#include "toconfiguration.h"

#include <qapplication.h>
#include <qregexp.h>

#include "utils.h"
#ifdef WIN32
#include "windows/cregistry.h"
#endif

#ifdef WIN32
#  ifdef TOAD
#  define APPLICATION_NAME "SOFTWARE\\Quest Software\\Toad for MySQL\\"
#  else
#  define APPLICATION_NAME "SOFTWARE\\Quest Software\\tora\\"
#  define FALLBACK_NAME    "SOFTWARE\\Underscore\\tora\\"
#  endif

static char *toKeyPath(const QString &str, CRegistry &registry)
{
    static char *buf = NULL;
    int pos = str.length() - 1;
    while (pos >= 0 && str.at(pos) != '\\')
        pos--;
    if (pos < 0)
        throw QT_TRANSLATE_NOOP("toKeyPath", "Couldn't find \\ in path");
    QString ret = str.mid(0, pos);
    if (buf)
        free(buf);
    buf = strdup(ret);
    registry.CreateKey(HKEY_CURRENT_USER, buf);
    return buf;
}

static char *toKeyValue(const QString &str)
{
    static char *buf = NULL;
    int pos = str.length() - 1;
    while (pos >= 0 && str.at(pos) != '\\')
        pos--;
    if (pos < 0)
        throw QT_TRANSLATE_NOOP("toKeyValue", "Couldn't find \\ in path");
    if (buf)
        free(buf);
    buf = strdup(str.mid(pos + 1));
    return buf;
}

#endif

toConfiguration::toConfiguration()
{

}

toConfiguration::~toConfiguration()
{
}

void toConfiguration::loadConfig()
{
   Configuration.clear();

#ifndef WIN32

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

void toConfiguration::loadMap(const QString &filename, std::map<QCString, QString> &pairs)
{
    QCString data = toReadFile(filename);

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
                throw QT_TRANSLATE_NOOP("toTool", "Malformed tag in config file. Missing = on row. (%1)").arg(data.mid(bol, wpos - bol));
            {
                QCString tag = ((const char *)data) + bol;
                QCString val = ((const char *)data) + endtag + 1;
                pairs[tag] = QString::fromUtf8(val);
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
#ifdef WIN32
        CRegistry registry;
        QRegExp re(":");
        for (std::map<QCString, QString>::iterator i = Configuration.begin();i != Configuration.end();i++)
        {
            QCString path = (*i).first;
            QString value = (*i).second;
            path.prepend(APPLICATION_NAME);
            path.replace(re, "\\");
            if (value.isEmpty())
            {
                if (!registry.SetStringValue(HKEY_CURRENT_USER,
                                             toKeyPath(path, registry),
                                             toKeyValue(path),
                                             ""))
                    toStatusMessage(QT_TRANSLATE_NOOP("toTool", "Couldn't save empty value at key %1").arg(path));
            }
            else
            {
                char *t = strdup(value.utf8());
                if (!registry.SetStringValue(HKEY_CURRENT_USER,
                                             toKeyPath(path, registry),
                                             toKeyValue(path),
                                             t))
                    toStatusMessage(QT_TRANSLATE_NOOP("toTool", "Couldn't save %1 value at key %2").arg(value).arg(path));
                free(t);
            }
        }
#else
        if ( Configuration.empty() )
           return ;
        QCString conf;
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

bool toConfiguration::saveMap(const QString &file, std::map<QCString, QString> &pairs)
{
    QCString data;

    {
        QRegExp newline(QString::fromLatin1("\n"));
        QRegExp backslash(QString::fromLatin1("\\"));
        for (std::map<QCString, QString>::iterator i = pairs.begin();i != pairs.end();i++)
        {
            QCString str = (*i).first;
            str.append(QString::fromLatin1("="));
            str.replace(backslash, QString::fromLatin1("\\\\"));
            str.replace(newline, QString::fromLatin1("\\n"));
            QString line = (*i).second;
            line.replace(backslash, QString::fromLatin1("\\\\"));
            line.replace(newline, QString::fromLatin1("\\n"));
            str += line.utf8();
            str += QString::fromLatin1("\n");
            data += str;
        }
    }
    return toWriteFile(file, data);
}

const QString &toConfiguration::globalConfig(const QCString &tag, const QCString &def)
{
    if ( Configuration.empty() )
       loadConfig();

    std::map<QCString, QString>::iterator i = Configuration.find(tag);
    if (i == Configuration.end())
    {
#if defined(WIN32)
        CRegistry registry;
        QRegExp re(QString::fromLatin1(":"));
        QCString path = tag;
        path.prepend(APPLICATION_NAME);
        path.replace(re, "\\");
        DWORD siz = 1024;
        char buffer[1024];
        try
        {
            if (registry.GetStringValue(HKEY_CURRENT_USER,
                                        toKeyPath(path, registry),
                                        toKeyValue(path),
                                        buffer, siz))
            {
                if (siz > 0)
                {
                    QString ret = QString::fromUtf8(buffer);
                    (Configuration)[tag] = ret;
                }
                else
                {
                    (Configuration)[tag] = "";
                }
                return (Configuration)[tag];
            }
        }
        catch (...)
        {
#ifdef FALLBACK_NAME
            try
            {
                path = tag;
                path.prepend(FALLBACK_NAME);
                path.replace(re, "\\");
                if (registry.GetStringValue(HKEY_CURRENT_USER,
                                            toKeyPath(path, registry),
                                            toKeyValue(path),
                                            buffer, siz))
                {
                    if (siz > 0)
                    {
                        QString ret = QString::fromUtf8(buffer);
                        (Configuration)[tag] = ret;
                    }
                    else
                    {
                        (Configuration)[tag] = "";
                    }
                    return (Configuration)[tag];
                }
            }
            catch (...)
            {}
#endif

        }
#endif

        (Configuration)[tag] = QString::fromLatin1(def);
        return (Configuration)[tag];
    }
    return (*i).second;
}



const QString& toConfiguration::config(const QCString &tag, const QCString &def, const QCString &name)
{
    QCString str = name;
    str.append(":");
    str.append(tag);
    return globalConfig(str, def);
}

void toConfiguration::eraseConfig(const QCString &tag, const QCString &name)
{
    QCString str = name;
    str.append(":");
    str.append(tag);
    globalEraseConfig(str);
}

void toConfiguration::setConfig(const QCString &tag, const QString &def, const QCString name)
{
    QCString str = name;
    str.append(":");
    str.append(tag);
    globalSetConfig(str, def);
}


void toConfiguration::globalEraseConfig(const QCString &tag)
{
    if ( Configuration.empty() )
       loadConfig();
    std::map<QCString, QString>::iterator i = Configuration.find(tag);
    if (i != Configuration.end())
    {
        Configuration.erase(i);
#if defined(WIN32)

        CRegistry registry;
        QRegExp re(QString::fromLatin1(":"));
        QCString path = tag;
        path.prepend(APPLICATION_NAME);
        path.replace(re, "\\");
        registry.DeleteKey(HKEY_CURRENT_USER, path); // Don't really care if it works.
#endif

    }
}

void toConfiguration::globalSetConfig(const QCString &tag, const QString &value)
{
    if ( Configuration.empty() )
       loadConfig();

    (Configuration)[tag] = value;
}


