/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include <qsplitter.h>

#include "tomarkedtext.h"
#include "toresultview.h"
#include "totool.h"
#include "totemplate.h"
#include "tomain.h"

#include "totemplate.moc"

#include "icons/totemplate.xpm"

class toTemplateTool : public toTool {
  TODock *Dock;
protected:
  virtual char **pictureXPM(void)
  { return totemplate_xpm; }
public:
  toTemplateTool()
    : toTool(301,"SQL Template")
  { Dock=NULL; }
  virtual const char *menuItem()
  { return "SQL Template"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    if (!Dock) {
      Dock=toAllocDock("Template",connection.connectString(),*toolbarImage());
      QWidget *window=new toTemplate(Dock);
      toAttachDock(Dock,window,QMainWindow::Left);
      return Dock;
    }
    Dock->show();
    Dock->setFocus();
    return NULL;
  }
  void closeWindow(toConnection &connection)
  { Dock=NULL; }
};

static toTemplateTool TemplateTool;

list<toTemplateProvider *> *toTemplateProvider::Providers;

toTemplateProvider::toTemplateProvider()
{
  if (!Providers)
    Providers=new list<toTemplateProvider *>;
  Providers->insert(Providers->end(),this);
}

toTemplate::toTemplate(QWidget *parent)
  : QVBox(parent)
{
  Splitter=new QSplitter(this);
  List=new toListView(Splitter);
  List->addColumn("Template");
  List->setRootIsDecorated(true);
  List->setSorting(0);
  List->setShowSortIndicator(false);

  connect(List,SIGNAL(expanded(QListViewItem *)),this,SLOT(expand(QListViewItem *)));
  connect(List,SIGNAL(collapsed(QListViewItem *)),this,SLOT(collapse(QListViewItem *)));

  if (toTemplateProvider::Providers)
    for (list<toTemplateProvider *>::iterator i=toTemplateProvider::Providers->begin();i!=toTemplateProvider::Providers->end();i++) {
      toTemplateItem *item=(*i)->insertItem(List);
      Providers[item]=(*i);
    }
}

toTemplate::~toTemplate()
{
  for(map<toTemplateItem *,toTemplateProvider *>::iterator i=Providers.begin();
      i!=Providers.end();
      i++) {
    toTemplateItem *item=(*i).first;
    toTemplateProvider *provider=(*i).second;
    provider->removeItem(item);
  }
}

void toTemplate::expand(QListViewItem *item)
{
  toTemplateItem *ti=dynamic_cast<toTemplateItem *>(item);
  if (ti)
    ti->expand();
}

void toTemplate::collapse(QListViewItem *item)
{
  toTemplateItem *ti=dynamic_cast<toTemplateItem *>(item);
  if (ti)
    ti->collapse();
}
