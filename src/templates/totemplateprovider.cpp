
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

#include "templates/totemplate.h"
#include "core/totool.h"
#include "core/utils.h"
#include "core/tologger.h"

#include <QtCore/QString>

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
            toTool *pTool = ToolsRegistrySing::Instance().value(*ToolKey, NULL);
            if (pTool)
                pTool->createWindow();
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
        }
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
