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

#include <map>

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
		     "Display analyze statistics about a column",
		     "8.0");
static toSQL SQLInfo7("toResultCols:Info",
		      "SELECT Data_Default,\n"
		      "       Num_Distinct,\n"
		      "       Low_Value,\n"
		      "       High_Value,\n"
		      "       Density,\n"
		      "       NULL,\n"
		      "       NULL,\n"
		      "       NULL,\n"
		      "       NULL,\n"
		      "       NULL,\n"
		      "  FROM All_Tab_Columns\n"
		      " WHERE Owner = :f1<char[100]>\n"
		      "   AND Table_Name = :f2<char[100]>\n"
		      "   AND Column_Name = :f3<char[100]>",
		      QString::null,
		      "7.3");

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
      toQList resLst=toQuery::readQueryNull(view->connection(),SQLInfo,
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
  : QVBox(parent,name)
{
  Title=new QLabel(this);
  Columns=new resultCols(this);
}

static toSQL SQLTableComment("toResultCols:TableComments",
			     "SELECT Comments FROM All_Tab_Comments\n"
			     " WHERE Owner = :f1<char[100]>\n"
			     "   AND Table_Name = :f2<char[100]>",
			     "Display column comments");

void toResultCols::query(const QString &,const toQList &param)
{
  toQList subp;

  toConnection &conn=connection();

  QString sql;
  QString Owner;
  QString TableName;
  toQList::iterator cp=((toQList &)param).begin();
  if (cp!=((toQList &)param).end()) {
    sql=conn.quote(*cp);
    Owner=*cp;
  }
  cp++;
  if (cp!=((toQList &)param).end()) {
    sql+=".";
    sql+=conn.quote(*cp);
    TableName=(*cp);
  } else {
    try {
      sql=QString::null;
      const toConnection::tableName &name=conn.realName(Owner);
      if (!name.Synonym.isEmpty())
	sql=conn.quote(name.Synonym)+"</B> synonym for <B>";

      sql+=conn.quote(name.Owner);
      if (!sql.isEmpty())
	sql+=".";
      sql+=conn.quote(name.Name);
      Owner=name.Owner;
      TableName=name.Name;
    } catch(...) {
    }
  }
  if (!Owner.isEmpty())
    toPush(subp,toQValue(Owner));
  toPush(subp,toQValue(TableName));

  Columns->query(sql,subp);

  sql.prepend("<B>");
  sql+="</B>";
  try {
    QString comment;
    toQuery query(conn,SQLTableComment,Owner,TableName);
    while(!query.eof()) {
      if (comment)
	comment+=" ";
      comment+=query.readValueNull();
    }
    if (comment) {
      sql+=" - ";
      sql+=comment;
    }
  } TOCATCH
  Title->setText(sql);
}

toResultCols::resultCols::resultCols(QWidget *parent,const char *name)
  : toResultView(false,true,parent,name)
{
  setReadAll(true);
  addColumn("Column Name");
  addColumn("Data Type");
  addColumn("NULL");
  addColumn("Comments");
  setSQLName("toResultCols");
  setSorting(0);
}

static toSQL SQLComment("toResultCols:Comments",
			"SELECT Column_name,Comments FROM All_Col_Comments\n"
			" WHERE Owner = :f1<char[100]>\n"
			"   AND Table_Name = :f2<char[100]>",
			"Display column comments");

void toResultCols::resultCols::query(const QString &,const toQList &param)
{
  setParams(param);
  if (!handled())
    return;

  toConnection &conn=connection();

  QString sql;
  QString Owner;
  QString TableName;

  toQList::iterator cp=((toQList &)param).begin();
  if (cp!=((toQList &)param).end()) {
    sql=conn.quote(*cp);
    Owner=*cp;
  }
  cp++;
  if (cp!=((toQList &)param).end()) {
    sql+=".";
    sql+=conn.quote(*cp);
    TableName=*cp;
  } else {
    TableName=Owner;
    Owner=QString::null;
  }

  LastItem=NULL;
  RowNumber=0;

  clear();

  try {
    QString str("SELECT * FROM ");
    str.append(sql);
    str.append(" WHERE NULL = NULL");

    toQuery Query(conn,str);
    std::map<QString,QString> comments;
    try {
      toQuery comment(conn,SQLComment,Owner,TableName);
      while(!comment.eof()) {
	QString col=comment.readValue();
	comments[col]=comment.readValueNull();
      }
    } catch (...) {
    }

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
      LastItem->setText(4,comments[(*i).Name]);
    }
  } TOCATCH
  updateContents();
}
