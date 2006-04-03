/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toconnection.h"
#include "toglobalsetting.h"
#include "tohelp.h"
#include "tomain.h"
#include "topreferences.h"
#include "topreferencesui.h"
#include "tosyntaxsetup.h"

#include <qframe.h>
#include <qlayout.h>
#include <qlistbox.h>

#include "topreferences.moc"
#include "topreferencesui.moc"

void toPreferences::addWidget(QListBoxItem *item, QWidget *widget)
{
    Layout->addWidget(widget);
    Tabs[item] = widget;
    if (!Shown)
        Shown = widget;
    else
        widget->hide();
}

void toPreferences::selectTab(QListBoxItem *item)
{
    QWidget *tab = Tabs[item];
    if (tab)
    {
        if (Shown)
            Shown->hide();
        tab->show();
        Shown = tab;
    }
}

void toPreferences::displayPreferences(QWidget *parent)
{
    toPreferences dialog(parent, "Options", true);
    if (dialog.exec())
        dialog.saveSetting();
}

void toPreferences::saveSetting(void)
{

    for (std::map<QListBoxItem *, QWidget *>::iterator i = Tabs.begin();i != Tabs.end();i++)
    {
        toSettingTab *tab = dynamic_cast<toSettingTab *>((*i).second);
        if (tab)
            tab->saveSetting();
    }
    toConfigurationSingle::Instance().saveConfig();
}

toPreferences::toPreferences(QWidget* parent, const char* name, bool modal, WFlags fl)
        : toPreferencesUI(parent, name, modal, fl), toHelpContext(QString::fromLatin1("preferences.html"))
{
    try
    {
        toHelp::connectDialog(this);
        Shown = NULL;

        Layout = new QVBoxLayout(Parent);

        QListBoxText *item;
        item = new QListBoxText(TabSelection, tr("Global Settings"));
        addWidget(item, new toGlobalSetting(Parent));

        item = new QListBoxText(TabSelection, tr("Editor Settings"));
        addWidget(item, new toSyntaxSetup(Parent));

        item = new QListBoxText(TabSelection, tr("Database Settings"));
        addWidget(item, new toDatabaseSetting(Parent));

        std::list<QCString> prov = toConnectionProvider::providers();
        for (std::list<QCString>::iterator i = prov.begin();i != prov.end();i++)
        {
            QWidget *tab = toConnectionProvider::configurationTab(*i, Parent);
            if (tab)
            {
                QString str(QString::fromLatin1(" "));
                str.append(QString::fromLatin1(*i));
                addWidget(new QListBoxText(TabSelection, str), tab);
            }
        }

        item = new QListBoxText(TabSelection, tr("Tools"));
        addWidget(item, new toToolSetting(Parent));

        TabSelection->setCurrentItem(0);

        std::map<QCString, toTool *> tools = toTool::tools();
        std::map<QCString, toTool *> newSort;
        {
            for (std::map<QCString, toTool *>::iterator i = tools.begin();i != tools.end();i++)
                newSort[(*i).second->name()] = (*i).second;
        }

        {
            for (std::map<QCString, toTool *>::iterator i = newSort.begin();i != newSort.end();i++)
            {
                QWidget *tab = (*i).second->configurationTab(Parent);
                if (tab)
                {
                    QString str(QString::fromLatin1(" "));
                    str.append((*i).first);
                    addWidget(new QListBoxText(TabSelection, str), tab);
                }
            }
        }
    }
    TOCATCH
}

void toPreferences::help(void)
{
    if (Shown)
        Shown->setFocus();
    toHelp::displayHelp(this);
}
