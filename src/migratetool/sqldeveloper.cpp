
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
                else if (attr == "customUrl" && opt.database.isEmpty())
                    opt.database = val;
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
