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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include "toresultdepend.h"
#include "tosql.h"
#include "tomain.h"

#define TO_BUFFER_SIZE 100

static toSQL SQLResultDepend("toResultDepend:Depends",
			     "SELECT DISTINCT\n"
			     "       referenced_owner \"Owner\",\n"
			     "       referenced_name \"Name\",\n"
			     "       referenced_type \"Type\",\n"
			     "       dependency_type \"Dependency Type\"\n"
			     "  FROM dba_dependencies\n"
			     " WHERE owner = :owner<char[31]>\n"
			     "   AND name = :name<char[31]>\n"
			     " ORDER BY referenced_owner,referenced_type,referenced_name",
			     "Display dependencies on an object, must have first two "
			     "columns same and same bindings");

toResultDepend::toResultDepend(toConnection &conn,QWidget *parent,const char *name)
  : toResultView(false,false,conn,parent,name)
{
  setSQL(SQLResultDepend(conn));
  setRootIsDecorated(true);
  setReadAll(true);
  setSQLName("toResultDepend");
}

void toResultDepend::addChilds(QListViewItem *item)
{
  try {
    otl_stream query(1,
		     SQL.utf8(),
		     Connection.connection());
    query<<item->text(0).utf8();
    query<<item->text(1).utf8();
    QListViewItem *last=NULL;
    while(!query.eof()) {
      QString owner=toReadValue(Description[0],query,TO_BUFFER_SIZE);
      QString name=toReadValue(Description[0],query,TO_BUFFER_SIZE);
      bool old=exists(owner,name);
      last=new QListViewItem(item,last);
      last->setText(0,owner);
      last->setText(1,name);
      for (int i=2;i<DescriptionLen;i++)
	last->setText(i,toReadValue(Description[i],query,TO_BUFFER_SIZE));
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

void toResultDepend::query(const QString &sql,
			   const list<QString> &param)
{
  toResultView::query(sql,param);

  for(QListViewItem *item=firstChild();item;item=item->nextSibling())
    addChilds(item);

  updateContents();
}
