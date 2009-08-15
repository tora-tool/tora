
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
