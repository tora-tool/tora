//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "totemplate.h"

std::list<toTemplateProvider *> *toTemplateProvider::Providers;
bool toTemplateProvider::Shown=false;
QCString toTemplateProvider::ToolKey;

toTemplateProvider::toTemplateProvider(const QCString &name)
  : Name(name)
{
  if (!Providers)
    Providers=new std::list<toTemplateProvider *>;
  Providers->insert(Providers->end(),this);
}

void toTemplateProvider::exportAllData(std::map<QCString,QString> &data,const QCString &prefix)
{
  if (!Providers)
    return;
  for (std::list<toTemplateProvider *>::iterator i=toTemplateProvider::Providers->begin();
       i!=toTemplateProvider::Providers->end();
       i++)
    (*i)->exportData(data,prefix+":"+(*i)->name());
  if (Shown)
    data[prefix+":Shown"]="Yes";
}

void toTemplateProvider::importAllData(std::map<QCString,QString> &data,const QCString &prefix)
{
  if (!Providers)
    return;
  for (std::list<toTemplateProvider *>::iterator i=toTemplateProvider::Providers->begin();
       i!=toTemplateProvider::Providers->end();
       i++)
    (*i)->importData(data,prefix+":"+(*i)->name());
  if(ToolKey.length()>0&&data[prefix+":Shown"]=="Yes") {
    try {
      toTool *tool=toTool::tool(ToolKey);
      if (tool)
	tool->createWindow();
    } catch(...) {
    }
  }
}

void toTemplateProvider::exportData(std::map<QCString,QString> &,const QCString &)
{
}

void toTemplateProvider::importData(std::map<QCString,QString> &,const QCString &)
{
}

void toTemplateProvider::setShown(bool shown)
{
  Shown=shown;
}
