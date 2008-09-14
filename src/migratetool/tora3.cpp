/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "config.h"
#include "tora3.h"
#include "utils.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QMap>

#include <set>


namespace MigrateTool
{

void loadMap(const QString &filename, QMap<QString, QString> &pairs);
void loadConnections(QMap<QString, QString> &pairs, QList<toConnectionOptions> &ret);
std::set<QString> processOptions(const QString &str);


QMap<int,toConnectionOptions> tora3(QWidget * parent)
{
    QMap<int,toConnectionOptions> ret;
    int key = 0;

    // I can't get the stupid QFileDialog to show hidden files by
    // default, so I'm going to just try to open the file.

    // I found plenty of people complaining about it online but with
    // no solutions for qt4.

    QFile file(QDir::homePath() + QString("/.torarc"));
    if(!file.exists()) {
        QMessageBox::warning(parent,
                             "Load Error",
                             QString("Cannot open file %1 for reading.").arg(file.fileName()));
        return ret;
    }


    QMap<QString, QString> config;
    loadMap(file.fileName(), config);
    QList<toConnectionOptions> l;
    loadConnections(config, l);
    foreach(toConnectionOptions o, l)
    {
        ret[key] = o;
        ++key;
    }
    return ret;
}


// updated old code from toconfiguration.cpp rev 2575
void loadMap(const QString &filename, QMap<QString, QString> &pairs)
{
    QByteArray data = toReadFile(filename).toUtf8();

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
                throw QString(QT_TRANSLATE_NOOP(
                    "toTool",
                    "Malformed tag in config file. Missing = on row. (%1)")).arg(
                        QString(data.mid(bol, wpos - bol)));
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
    return;
}


// similar, but heavily modified from tonewconnection.cpp:toNewConnection() -r2535
void loadConnections(QMap<QString, QString> &pairs, QList<toConnectionOptions> &ret)
{
    static const QString HISTORY = "ConnectHistory:";

    QString strMax = pairs[QString("ConnectCurrent")];

    int max = 0;
    if(strMax.isEmpty())
        max = 40;
    else {
        bool ok;
        int t = strMax.toInt(&ok);
        if(ok)
            max = t;
    }

    for(int pos = 0; pos < max; pos++) {
        QString num;
        num.setNum(pos);

        QString db      = pairs[HISTORY + num + "DefaultDatabase"];
        QString pass    = pairs[HISTORY + num + "DefaultPassword"];
        QString user    = pairs[HISTORY + num + "DefaultUser"];
        QString host    = pairs[HISTORY + num + "Host"];
        QString options = pairs[HISTORY + num + "Options"];
        QString prov    = pairs[HISTORY + num + "Provider"];
        QString schema;

        if(prov.isEmpty())
            break;

        if(prov == "Oracle")
            prov = "Oracle (TNS)";
        if(!pass.isEmpty())
            pass = toUnobfuscate(pass);

        toConnectionOptions conn(prov,
                                 host,
                                 db,
                                 user,
                                 pass,
                                 schema,
                                 0,
                                 processOptions(options));
        ret.append(conn);
    }
}


// also old code from tonewconnection.cpp
std::set<QString> processOptions(const QString &str) {
    QStringList options = str.split(",");
    std::set<QString> values;

    Q_FOREACH(QString val, options) {
        if(val.startsWith("*"))
            values.insert(val.mid(1));
    }

    return values;
}

} // namespace
