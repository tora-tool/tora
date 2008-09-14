/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "sqldeveloper.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QXmlStreamReader>

#include <QtDebug>


namespace MigrateTool
{

QMap<int,toConnectionOptions> sqlDeveloper(QWidget * parent)
{

    QString fileName = QFileDialog::getOpenFileName(parent,
                                                    "SQL Developer Connections",
                                                    QDir::homePath(),
                                                    "XML Files (*.xml);;All Files (*.*)");
    QMap<int,toConnectionOptions> ret;
    if (fileName.isNull())
        return ret;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(parent, "Load Error", QString("Cannot open file %1 for reading.").arg(fileName));
        return ret;
    }

    QXmlStreamReader xml(&file);
    bool isXML = false;
    toConnectionOptions opt;
    QString attr;
    int key = 0;

    while (!xml.atEnd())
    {
        xml.readNext();
        if (isXML && xml.isEndElement() && xml.name() == "RefAddresses")
        {
//             qDebug() << "connection -----";
            if (!opt.username.isEmpty() && !opt.provider.isEmpty())
            {
                if (opt.provider == "Oracle")
                {
                    if (opt.host.isEmpty())
                        opt.provider = "Oracle (TNS)";
                    else
                        opt.provider = "Oracle (Instant Client)";
                }
                ret[key] = opt;
                ++key;
                opt.username = "";
                opt.database = "";
                opt.host = "";
                opt.provider = "";
            }
            attr = "";
        }
        if (xml.isStartElement())
        {
//             qDebug() << "debug" <<xml.name().toString();
            if (xml.name() == "References")
            {
                isXML = true;
//                 qDebug() << "connections";
            }
            if (isXML && xml.name() == "StringRefAddr")
            {
                attr = xml.attributes().value("addrType").toString();
            }
            if (isXML && xml.name() == "Contents")
            {
//                 qDebug() << "StringRefAddr" << attr;//.toString();
                QString val(xml.readElementText());
                // "oraDriverType"
                if (attr == "sid")
                    opt.database = val;
//                     qDebug() << "sid" << val;
                else if (attr == "port")
                    opt.port = val.toInt();
//                     qDebug() << "port" << val;
                else if (attr == "subtype")
                {
                    val = val.toLower();
                    if (val == "mysql")
                        opt.provider = "MySQL";
                    else if (val == "posgtresql")
                        opt.provider = "PostgreSQL";
                    else
                        opt.provider = "Oracle";
//                     qDebug() << "subtype" << val;
                }
                // "DeployPassword"
                else if (attr == "user")
                    opt.username = val;
//                     qDebug() << "user" << val;
                else if (attr == "hostname")
                    opt.host  = val;
//                     qDebug() << "hostname" << val;
                // "password"
                // "SavePassword"
//                 else if (attr == "customUrl")
//                     qDebug() << "customUrl" << val;
//                 else
//                     qDebug() << "Skipped" << attr << val;
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
