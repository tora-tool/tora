/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
