/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "squirrelsql.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QXmlStreamReader>

#include <QtDebug>


namespace MigrateTool
{

QMap<int,toConnectionOptions> squirrelSql(QWidget * parent)
{
    QMap<int,toConnectionOptions> ret;
    int key = 0;

    QString fileName = QFileDialog::getOpenFileName(parent,
                                                    "SquirrelSQL Configuration file (SQLAliases23.xml)",
                                                    QDir::homePath() + "/.squirrel-sql/",
                                                    "XML Files (*.xml);;All Files (*.*)");
    if (fileName.isNull())
        return ret;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(parent, "Load Error",
                              QString("Cannot open file %1 for reading.").arg(fileName));
        return ret;
    }

    QXmlStreamReader xml(&file);
    bool isXML = false;
    toConnectionOptions opt;

    while (!xml.atEnd())
    {
        xml.readNext();
        if (xml.isEndElement() && isXML && xml.name() == "Bean")
        {
            if (!opt.username.isEmpty() && !opt.provider.isEmpty())
            {
                ret[key] = opt;
                ++key;
                opt.username = "";
                opt.database = "";
                opt.host = "";
                opt.password = "";
                opt.provider = "";
            }
        }

        if (xml.isStartElement())
        {
//             qDebug() << "debug" <<xml.name().toString();
            if (xml.name() == "Beans")
            {
                isXML = true;
//                 qDebug() << "connections";
            }
            if (isXML && xml.name() == "password")
                opt.password = xml.readElementText();
            if (isXML && xml.name() == "userName")
                opt.username = xml.readElementText();
            if (isXML && xml.name() == "url")
            {
                QString url(xml.readElementText());
                if (url.indexOf("jdbc:mysql://") != -1) // mysql found
                {
                    // format: server[:port]/dbname
                    QString connStr(url.replace("jdbc:mysql://", ""));
                    QStringList l = connStr.split("/", QString::SkipEmptyParts);
                    if (l.size() > 1)
                        opt.database = l.at(1);
                    QStringList k = l.at(0).split(":", QString::SkipEmptyParts);
                    if (k.size() > 1)
                        opt.port = k.at(1).toInt();
                    else
                        opt.port = 3306;
                    opt.host = k.at(0);
                    opt.provider = "MySQL";
                }
                else if (url.indexOf("jdbc:oracle:oci8:@") != -1) // oracle found
                {
                    opt.database = url.replace("jdbc:oracle:oci8:@", "");
                    opt.provider = "Oracle (TNS)";
                }
                else if (url.indexOf("jdbc:oracle:thin:@") != -1)
                {
                    // format: server[:port]/sid
                    QString connStr(url.replace("dbc:oracle:thin:@", ""));
                    QStringList l = connStr.split("/", QString::SkipEmptyParts);
                    if (l.size() > 1)
                        opt.database = l.at(1);
                    QStringList k = l.at(0).split(":", QString::SkipEmptyParts);
                    if (k.size() > 1)
                        opt.port = k.at(1).toInt();
                    else
                        opt.port = 1521;
                    opt.host = k.at(0);
                    opt.provider = "Oracle (Instant Client)";
                }
                else if (url.indexOf("jdbc:postgresql:") != -1) // pgsql
                {
                    // format: server[:port]/database
                    QString connStr(url.replace("jdbc:postgresql:", ""));
                    QStringList l = connStr.split("/", QString::SkipEmptyParts);
                    if (l.size() > 1)
                        opt.database = l.at(1);
                    QStringList k = l.at(0).split(":", QString::SkipEmptyParts);
                    if (k.size() > 1)
                        opt.port = k.at(1).toInt();
                    else
                        opt.port = 5432;
                    opt.host = k.at(0);
                    opt.provider = "Oracle (Instant Client)";
                }
                else
                    qDebug() << "TOra cannot handle provider:" << url;
            }
        }
    }
    if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError)
    {
        qWarning() << "XML ERROR:" << xml.lineNumber() << ": " << xml.errorString();
    }

    file.close();
    return ret;
}

} // namespace
