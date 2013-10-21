
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

#include "core/topreferences.h"
#include "core/toglobalsetting.h"
#include "core/toconfiguration.h"
#include "core/toconnectionprovider.h"
#include "core/tomainwindow.h"
#include "core/utils.h"
#include "core/tohelp.h"
#include "core/tosyntaxsetup.h"

#include <QtCore/QString>
#include <QtGui/QFrame>
#include <QtGui/QLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QListWidget>


void toPreferences::addWidget(QListWidgetItem *item, QWidget *widget)
{
    Parent->layout()->addWidget(widget);
    Tabs[item] = widget;
    if (!Shown)
        Shown = widget;
    else
        widget->hide();
}

void toPreferences::selectTab(QListWidgetItem *item)
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
    for (std::map<QListWidgetItem *, QWidget *>::iterator i = Tabs.begin(); i != Tabs.end(); i++)
    {
        toSettingTab *tab = dynamic_cast<toSettingTab *>((*i).second);
        if (tab)
            tab->saveSetting();
    }
    toConfigurationSingle::Instance().saveConfig();
}

toPreferences::toPreferences(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent)
    , toHelpContext(QString::fromLatin1("preferences.html"))
{

    setupUi(this);

    try
    {
        toHelp::connectDialog(this);
        Shown = NULL;

//         QVBoxLayout *vbox = new QVBoxLayout;
//         vbox->setSpacing(0);
//         vbox->setContentsMargins(0, 0, 0, 0);
//         setLayout(vbox);

        QListWidgetItem *item;
        item = new QListWidgetItem(tr("Global Settings"), TabSelection);
        addWidget(item, new toGlobalSetting(Parent));

        item = new QListWidgetItem(tr("Editor Settings"), TabSelection);
        addWidget(item, new toSyntaxSetup(Parent));

        item = new QListWidgetItem(tr("Database Settings"), TabSelection);
        addWidget(item, new toDatabaseSetting(Parent));

        QList<QString> providers = toConnectionProviderRegistrySing::Instance().providers();
        foreach(QString const & provider, providers)
        {
            QWidget *tab = toConnectionProviderRegistrySing::Instance().get(provider).configurationTab(Parent);
            if (tab)
            {
                QString str(QString::fromLatin1(" ") + provider);
                addWidget(new QListWidgetItem(str, TabSelection), tab);
            }
        }

        item = new QListWidgetItem(tr("Tools"), TabSelection);
        addWidget(item, new toToolSetting(Parent));

        TabSelection->setCurrentItem(0);

        for (ToolsRegistrySing::ObjectType::iterator i = ToolsRegistrySing::Instance().begin();
                i != ToolsRegistrySing::Instance().end();
                ++i)
        {
            QWidget *tab = i.value()->configurationTab(Parent);
            if (tab)
            {
                QString str(QString::fromLatin1(" "));
                str.append(i.value()->name());
                addWidget(new QListWidgetItem(str, TabSelection), tab);
            }
        }
    }
    TOCATCH;
}

void toPreferences::help(void)
{
    if (Shown)
        Shown->setFocus();
    toHelp::displayHelp(this);
}
