//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
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

#include "toresultextract.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"
#include "tosql.h"
#include "toconnection.h"
#include "toextract.h"

#include "toresultextract.moc"

toResultExtract::toResultExtract(bool prompt,QWidget *parent,const char *name)
  : toWorksheet(parent,name,toCurrentConnection(parent))
{
  Prompt=prompt;
}

static toSQL SQLObjectType("toResultExtract:ObjectType",
			   "SELECT Object_Type FROM sys.All_Objects\n"
			   " WHERE Owner = :f1<char[101]> AND Object_Name = :f2<char[101]>",
			   "Get type of an object by name");

void toResultExtract::query(const QString &sql,const toQList &param)
{
  if (!setSQLParams(sql,param))
    return;

  toQList::iterator i=params().begin();
  QString owner;
  QString name;
  if (i!=params().end()) {
    owner=*i;
    i++;
  }
  if (i==params().end()) {
    name=owner;
    owner=toToolWidget::connection().user().upper();
  } else {
    name=*i;
    i++;
  }

  try {
    QString type;
    if (i==params().end()) {
      toQuery query(toToolWidget::connection(),SQLObjectType,owner,name);

      if(query.eof())
	throw QString("Object not found");
    
      type=query.readValue();
    } else
      type=*i;

    std::list<QString> objects;

    if (type=="TABLE"||type=="TABLE PARTITION") {
      objects.insert(objects.end(),"TABLE FAMILY:"+owner+"."+name);
      objects.insert(objects.end(),"TABLE REFERENCES:"+owner+"."+name);
    } else if (type.startsWith("PACKAGE")&&Prompt) {
      objects.insert(objects.end(),"PACKAGE:"+owner+"."+name);
      objects.insert(objects.end(),"PACKAGE BODY:"+owner+"."+name);
    } else
      objects.insert(objects.end(),type+":"+owner+"."+name);

    toExtract extract(toToolWidget::connection(),NULL);
    extract.setCode(true);
    extract.setHeading(false);
    extract.setPrompt(Prompt);
    editor()->setText(extract.create(objects));
  } TOCATCH
}

bool toResultExtract::canHandle(toConnection &conn)
{
  return toIsOracle(conn);
}
