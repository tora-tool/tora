/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "totemplate.h"

#include <QString>

std::list<toTemplateProvider *> *toTemplateProvider::Providers;
bool toTemplateProvider::Shown = false;
QString *toTemplateProvider::ToolKey;

toTemplateProvider::toTemplateProvider(const QString &name)
        : Name(name)
{
    if (!Providers)
        Providers = new std::list<toTemplateProvider *>;
    Providers->insert(Providers->end(), this);
}

void toTemplateProvider::exportAllData(std::map<QString, QString> &data, const QString &prefix)
{
    if (!Providers)
        return ;
    for (std::list<toTemplateProvider *>::iterator i = toTemplateProvider::Providers->begin();
            i != toTemplateProvider::Providers->end();
            i++)
        (*i)->exportData(data, prefix + ":" + (*i)->name());
    if (Shown)
        data[prefix + ":Shown"] = "Yes";
}

void toTemplateProvider::importAllData(std::map<QString, QString> &data, const QString &prefix)
{
    if (!Providers)
        return ;
    for (std::list<toTemplateProvider *>::iterator i = toTemplateProvider::Providers->begin();
            i != toTemplateProvider::Providers->end();
            i++)
        (*i)->importData(data, prefix + ":" + (*i)->name());
    if (ToolKey != NULL && data[prefix + ":Shown"] == "Yes")
    {
        try
        {
            toTool *tool = toTool::tool(*ToolKey);
            if (tool)
                tool->createWindow();
        }
        catch (...)
            {}
    }
}

void toTemplateProvider::exportData(std::map<QString, QString> &, const QString &)
{}

void toTemplateProvider::importData(std::map<QString, QString> &, const QString &)
{}

void toTemplateProvider::setShown(bool shown)
{
    Shown = shown;
}

void toTemplateProvider::setToolKey(const QString &key)
{
    if (!ToolKey)
        ToolKey = new QString;

    (*ToolKey) = key;
}
