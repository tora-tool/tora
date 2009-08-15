
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

#include "utils.h"

#include "toconnection.h"
#include "toglobalsetting.h"
#include "tohelp.h"
#include "tomain.h"
#include "topreferences.h"
#include "tosyntaxsetup.h"

#include <QFrame>
#include <qlayout.h>

#include <QString>
#include <QVBoxLayout>
#include <QListWidget>


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

    for (std::map<QListWidgetItem *, QWidget *>::iterator i = Tabs.begin();i != Tabs.end();i++)
    {
        toSettingTab *tab = dynamic_cast<toSettingTab *>((*i).second);
        if (tab)
            tab->saveSetting();
    }
    toConfigurationSingle::Instance().saveConfig();
}

toPreferences::toPreferences(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
        : QDialog(parent), toHelpContext(QString::fromLatin1("preferences.html"))
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

        std::list<QString> prov = toConnectionProvider::providers();
        for (std::list<QString>::iterator i = prov.begin();i != prov.end();i++)
        {
            QWidget *tab = toConnectionProvider::configurationTab(*i, Parent);
            if (tab)
            {
                QString str(QString::fromLatin1(" "));
                str.append((*i));
                addWidget(new QListWidgetItem(str, TabSelection), tab);
            }
        }

        item = new QListWidgetItem(tr("Tools"), TabSelection);
        addWidget(item, new toToolSetting(Parent));

        TabSelection->setCurrentItem(0);

        std::map<QString, toTool *> tools = toTool::tools();
        std::map<QString, toTool *> newSort;
        {
            for (std::map<QString, toTool *>::iterator i = tools.begin();i != tools.end();i++)
                newSort[(*i).second->name()] = (*i).second;
        }

        {
            for (std::map<QString, toTool *>::iterator i = newSort.begin();i != newSort.end();i++)
            {
                QWidget *tab = (*i).second->configurationTab(Parent);
                if (tab)
                {
                    QString str(QString::fromLatin1(" "));
                    str.append((*i).first);
                    addWidget(new QListWidgetItem(str, TabSelection), tab);
                }
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
