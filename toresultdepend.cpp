//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "toresultdepend.h"
#include "tosql.h"
#include "tomain.h"
#include "toconnection.h"

static toSQL SQLResultDepend("toResultDepend:Depends",
			     "SELECT DISTINCT\n"
			     "       referenced_owner \"Owner\",\n"
			     "       referenced_name \"Name\",\n"
			     "       referenced_type \"Type\",\n"
			     "       dependency_type \"Dependency Type\"\n"
			     "  FROM dba_dependencies\n"
			     " WHERE owner = :owner<char[101]>\n"
			     "   AND name = :name<char[101]>\n"
			     " ORDER BY referenced_owner,referenced_type,referenced_name",
			     "Display dependencies on an object, must have first two "
			     "columns same and same bindings");

toResultDepend::toResultDepend(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
{
  setSQL(SQLResultDepend);
  setRootIsDecorated(true);
  setReadAll(true);
  setSQLName("toResultDepend");
}

void toResultDepend::addChilds(QListViewItem *item)
{
  try {
    toQuery query(connection(),sql(),item->text(0),item->text(1));
    QListViewItem *last=NULL;
    while(!query.eof()) {
      QString owner=query.readValue();
      QString name=query.readValue();
      bool old=exists(owner,name);
      last=new QListViewItem(item,last);
      last->setText(0,owner);
      last->setText(1,name);
      for (int i=2;i<query.columns();i++)
	last->setText(i,query.readValue());
      if (!old)
	addChilds(last);
    }
  } TOCATCH
}

bool toResultDepend::exists(const QString &owner,const QString &name)
{
  QListViewItem *item=firstChild();
  while (item) {
    if (item->text(0)==owner&&item->text(1)==name)
      return true;
    if (item->firstChild())
      item=item->firstChild();
    else if (item->nextSibling())
      item=item->nextSibling();
    else {
      do {
	item=item->parent();
      } while(item&&!item->nextSibling());
      if (item)
	item=item->nextSibling();
    }
  }
  return false;
}

void toResultDepend::query(const QString &sql,const toQList &param)
{
  if (!handled())
    return;

  toResultView::query(sql,param);

  for(QListViewItem *item=firstChild();item;item=item->nextSibling())
    addChilds(item);

  updateContents();
}
