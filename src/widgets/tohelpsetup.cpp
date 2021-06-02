
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

#include "widgets/tohelpsetup.h"
#include "core/utils.h"
#include "core/tohtml.h"
#include "core/toconfiguration.h"
#include "core/tomainwindow.h"
#include "tohelp.h"

#include <QRegExp>

toHelpAddFile::toHelpAddFile(QWidget *parent, const char *name)
    : QDialog(parent)
{

    if (name)
        setObjectName(name);

    setupUi(this);
    OkButton->setEnabled(false);
    toHelp::connectDialog(this);
}


void toHelpAddFile::browse()
{
    QString filename = Utils::toOpenFilename(Filename->text(), QString::fromLatin1("toc.htm*"), this);
    if (!filename.isEmpty())
        Filename->setText(filename);
}


void toHelpAddFile::valid()
{
    if (Filename->text().isEmpty() || Root->text().isEmpty())
        OkButton->setEnabled(false);
    else
        OkButton->setEnabled(true);
}


toHelpSetting::toHelpSetting(toTool *tool, QWidget *parent, const char *name)
    : QWidget(parent)
    , toSettingTab("additionalhelp.html")
    , Tool(tool)
{

    if (name)
        setObjectName(name);

    setupUi(this);
    toSettingTab::loadSettings(this);
//     int tot = Tool->config("Number", "-1").toInt();
//     if (tot != -1)
//     {
//         for (int i = 0;i < tot;i++)
//         {
//             QString num = QString::number(i);
//             QString root = Tool->config(num.toLatin1(), "");
//             num += QString::fromLatin1("file");
//             QString file = Tool->config(num.toLatin1(), "");
//             new toTreeWidgetItem(FileList, root, file);
//         }
//     }
    const QMap<QString, QVariant> hMap = toConfigurationNewSingle::Instance().option(ToConfiguration::Help::AdditionalHelpMap).toMap();
    QMapIterator<QString, QVariant> i(hMap);
    while (i.hasNext())
    {
        i.next();
        new toTreeWidgetItem(FileList, i.key(), i.value().toString());
    }
}


void toHelpSetting::saveSetting()
{
    toSettingTab::saveSettings(this);
//     int i = 0;
    QMap<QString, QVariant> hMap;
    for (toTreeWidgetItem *item = FileList->firstChild(); item; item = item->nextSibling())
    {
//         QString nam = QString::number(i);
//         Tool->setConfig(nam.toLatin1(), item->text(0));
//         nam += QString::fromLatin1("file");
//         Tool->setConfig(nam.toLatin1(), item->text(1));
//         i++;
        hMap[item->text(0)] = QVariant(item->text(1));
    }
//     Tool->setConfig("Number", QString::number(i));
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Help::AdditionalHelpMap, hMap);
    delete toHelp::Window;
}


void toHelpSetting::addFile()
{
    toHelpAddFile file(this);
    if (file.exec())
        new toTreeWidgetItem(FileList, file.Root->text(), file.Filename->text());
}


void toHelpSetting::delFile()
{
    delete FileList->selectedItem();
}


void toHelpSetting::oracleManuals()
{
    // For Oracle 11g look for a file /nav/portal_3.htm which contains a list of all books
    QString filename = Utils::toOpenFilename(QString::fromLatin1("*index.htm*"), this);
    try
    {
        toHtml file(Utils::toReadFile(filename));
        QString dsc;
        bool inDsc = false;
        QRegularExpression isToc(QString::fromLatin1("toc\\.htm?$"));
        while (!file.eof())
        {
            file.nextToken();
            if (file.isTag())
            {
                if (file.open() && file.tag() == "a")
                {
                    QString href = toHelp::path(filename);
                    href += file.value("href");
                    if (!href.isEmpty() &&
                            !dsc.isEmpty() &&
                            href.indexOf(isToc) >= 0 &&
                            !file.value("title").isNull())
                    {
                        new toTreeWidgetItem(FileList, dsc.simplified(), href);
                        inDsc = false;
                        dsc.clear();
                    }
                }
                // This one is for oracle manuals with version 11
                else if ((file.open() && file.tag() == "td") && !file.value("id").isNull())
                {
                    dsc.clear();
                    inDsc = true;
                }
                // This one is for old version of oracle manuals
                else if ((file.open() && file.tag() == "dd") || file.tag() == "book")
                {
                    dsc.clear();
                    inDsc = true;
                }
            }
            else if (inDsc)
                dsc += file.text();
        }
    }
    catch (const QString &str)
    {
        TOMessageBox::warning(toMainWindow::lookup(), qApp->translate("toHelpPrefs", "File error"), str);
    }
}
