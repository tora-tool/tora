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

#include "toresultcols.h"
#include "tomain.h"
#include "tosql.h"
#include "toconnection.h"

static toSQL SQLInfo("toResultCols:Info",
		     "SELECT Data_Default,\n"
		     "       Num_Distinct,\n"
		     "       Low_Value,\n"
		     "       High_Value,\n"
		     "       Density,\n"
		     "       Num_Nulls,\n"
		     "       Num_Buckets,\n"
		     "       Last_Analyzed,\n"
		     "       Sample_Size,\n"
		     "       Avg_Col_Len\n"
		     "  FROM All_Tab_Columns\n"
		     " WHERE Owner = :f1<char[100]>\n"
		     "   AND Table_Name = :f2<char[100]>\n"
		     "   AND Column_Name = :f3<char[100]>",
		     "Display analyze statistics about a column");

class toResultColsItem : public toResultViewMLine {
public:
  toResultColsItem(QListView *parent,QListViewItem *after,const char *buffer)
    : toResultViewMLine(parent,after,buffer)
  { }
  virtual QString key (int column,bool ascending)
  {
    if (column==0) {
      QString ret;
      ret.sprintf("%04d",text(0).toInt());
      return ret;
    }
    return toResultViewMLine::key(column,ascending);
  }
  virtual QString tooltip(int col) const
  {
    toResultCols *view=dynamic_cast<toResultCols *>(listView());
    try {
      if (view->connection().provider()!="Oracle")
	return QString::null;
      toQList resLst=toQuery::readQuery(view->connection(),SQLInfo,
					text(10),text(11),text(1));
      QString result("<B>");
      result+=(text(1));
      result+=("</B><BR><BR>");

      int any=0;
      QString cur=toShift(resLst);
      if (!cur.isEmpty()) {
	result+=("Default value: <B>");
	result+=(cur);
	result+=("</B><BR><BR>");
	any++;
      }

      QString analyze;
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=("Distinct values: <B>");
	analyze+=(cur);
	analyze+=("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=("Low value: <B>");
	analyze+=(cur);
	analyze+=("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=("High value: <B>");
	analyze+=(cur);
	analyze+=("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=("Density: <B>");
	analyze+=(cur);
	analyze+=("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=("Number of nulls: <B>");
	analyze+=(cur);
	analyze+=("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=("Number of histogram buckets: <B>");
	analyze+=(cur);
	analyze+=("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=("Last analyzed: <B>");
	analyze+=(cur);
	analyze+=("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=("Sample size: <B>");
	analyze+=(cur);
	analyze+=("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=("Average column size: <B>");
	analyze+=(cur);
	analyze+=("</B><BR>");
	any++;
      }
      if (!analyze.isEmpty()) {	
	result+=("<B>Analyze statistics:</B><BR>");
	result+=(analyze);
      }
      if (!any)
	return text(col);
      return result;
    } catch (const QString  &exc) {
      toStatusMessage(exc);
      return text(col);
    }
  }
};

toResultCols::toResultCols(QWidget *parent,const char *name)
  : toResultView(false,true,parent,name)
{
  setReadAll(true);
  addColumn("Column Name");
  addColumn("Data Type");
  addColumn("NULL");
  addColumn("Comments");
  setSQLName("toResultCols");
}

static toSQL SQLComment("toResultCols:Comments",
			"SELECT Comments FROM All_Col_Comments\n"
			" WHERE Owner = :f1<char[100]>\n"
			"   AND Table_Name = :f2<char[100]>\n"
			"   AND Column_Name = :f3<char[100]>",
			"Display column comments");

void toResultCols::query(const QString &sql,const toQList &param)
{
  toConnection &conn=connection();
  enum {
    Oracle,
    MySQL
  } type;

  if(conn.provider()=="Oracle")
    type=Oracle;
  else if (conn.provider()=="MySQL")
    type=MySQL;
  else
    return;
    

  SQL=sql;
  QString Owner;
  QString TableName;
  toQList::iterator cp=((toQList &)param).begin();
  if (cp!=((toQList &)param).end()) {
    if (type==Oracle) {
      SQL="\"";
      SQL+=*cp;
      SQL+="\"";
    } else
      SQL="";
    Owner=*cp;
  }
  cp++;
  if (cp!=((toQList &)param).end()) {
    if (type==Oracle)
      SQL.append(".\"");
    SQL.append(*cp);
    if (type==Oracle)
      SQL+="\"";
    TableName=(*cp);
  } else {
    try {
      const toConnection::tableName &name=connection().realName(Owner);
      if (type==Oracle) {
	SQL="\"";
	SQL+=name.Owner;
	SQL+="\".\"";
	SQL+=name.Name;
	SQL+="\"";
      } else
	SQL=name.Name;
      Owner=name.Owner;
      TableName=name.Name;
    } catch(...) {
    }
  }
  LastItem=NULL;
  RowNumber=0;

  clear();
  setSorting(0);

  try {
    QString str("SELECT * FROM ");
    str.append(SQL);
    str.append(" WHERE NULL = NULL");

    toQuery Query(conn,str);
    toQuery Comment(conn);

    toQDescList desc=Query.describe();

    int col=1;
    for (toQDescList::iterator i=desc.begin();i!=desc.end();i++) {
      LastItem=new toResultColsItem(this,LastItem,NULL);

      LastItem->setText(10,Owner);
      LastItem->setText(11,TableName);
      LastItem->setText(1,(*i).Name);
      LastItem->setText(0,QString::number(col++));

      LastItem->setText(2,(*i).Datatype);
      if ((*i).Null)
	LastItem->setText(3,"NULL");
      else
	LastItem->setText(3,"NOT NULL");

      toQList lst;
      toPush(lst,toQValue(Owner));
      toPush(lst,toQValue(TableName));
      toPush(lst,toQValue((*i).Name));
      if(type==Oracle) {
	Comment.execute(SQLComment,lst);
	LastItem->setText(4,Comment.readValueNull());
      }
    }
  } TOCATCH
  updateContents();
}

bool toResultCols::canHandle(const toConnection &conn)
{
  if (conn.provider()=="Oracle"||conn.provider()=="MySQL")
    return true;
  return false;
}
