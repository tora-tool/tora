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

#include "toresultindexes.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"
#include "tosql.h"
#include "toconnection.h"

toResultIndexes::toResultIndexes(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
{
  setReadAll(true);
  addColumn("Index Name");
  addColumn("Columns");
  addColumn("Type");
  addColumn("Unique");
  setSQLName("toResultIndexes");
}

static toSQL SQLColumns("toResultIndexes:Columns",
			"SELECT Column_Name FROM All_Ind_Columns\n"
			" WHERE Index_Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>\n"
			" ORDER BY Column_Position",
			"List columns an index is built on");

QString toResultIndexes::indexCols(const QString &indOwner,const QString &indName)
{
  toQuery query(connection(),SQLColumns,indOwner,indName);

  QString ret;
  while(!query.eof()) {
    if (!ret.isEmpty())
      ret.append(",");
    ret.append(query.readValue());
  }
  return ret;
}

static toSQL SQLListIndex("toResultIndexes:ListIndex",
			  "SELECT Owner,\n"
			  "       Index_Name,\n"
			  "       Index_Type,\n"
			  "       Uniqueness\n"
			  "  FROM All_Indexes\n"
			  " WHERE Table_Owner = :f1<char[101]>\n"
			  "   AND Table_Name = :f2<char[101]>\n"
			  " ORDER BY Index_Name",
			  "List the indexes available on a table");

void toResultIndexes::query(const QString &sql,const toQList &param)
{
  QString Owner;
  QString TableName;
  toQList::iterator cp=((toQList &)param).begin();
  if (cp!=((toQList &)param).end())
    Owner=*cp;
  cp++;
  if (cp!=((toQList &)param).end())
    TableName=(*cp);

  RowNumber=0;

  clear();

  try {
    toQuery query(connection(),SQLListIndex,Owner,TableName);

    QListViewItem *item=NULL;
    while(!query.eof()) {
      item=new QListViewItem(this,item,NULL);

      QString indexOwner(query.readValue());
      QString indexName(query.readValue());
      item->setText(0,indexName);
      item->setText(1,indexCols(indexOwner,indexName));
      item->setText(2,query.readValue());
      item->setText(3,query.readValue());
    }
  } TOCATCH
  updateContents();
  return;
}
