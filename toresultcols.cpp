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
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include "toresultcols.h"
#include "tomain.h"
#include "tosql.h"

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
		     " WHERE Owner = :f1<char[31]>\n"
		     "   AND Table_Name = :f2<char[31]>\n"
		     "   AND Column_Name = :f3<char[31]>",
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
      otl_stream ColInfo(1,
			 SQLInfo(view->Connection),
			 view->Connection.connection());
      ColInfo.set_all_column_types(otl_all_num2str|otl_all_date2str);
      ColInfo<<text(10).utf8()<<text(11).utf8()<<text(0).utf8();
      char buffer[4001];
      QString result("<B>");
      result.append(text(0));
      result.append("</B><BR><BR>");

      int any=0;
      ColInfo>>buffer;
      if (strlen(buffer)) {
	result.append("Default value: <B>");
	result.append(QString::fromUtf8(buffer));
	result.append("</B><BR><BR>");
	any++;
      }

      QString analyze;
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Distinct values: <B>");
	analyze.append(QString::fromUtf8(buffer));
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Low value: <B>");
	analyze.append(QString::fromUtf8(buffer));
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("High value: <B>");
	analyze.append(QString::fromUtf8(buffer));
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Density: <B>");
	analyze.append(QString::fromUtf8(buffer));
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Number of nulls: <B>");
	analyze.append(QString::fromUtf8(buffer));
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Number of histogram buckets: <B>");
	analyze.append(QString::fromUtf8(buffer));
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Last analyzed: <B>");
	analyze.append(QString::fromUtf8(buffer));
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Sample size: <B>");
	analyze.append(QString::fromUtf8(buffer));
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Average column size: <B>");
	analyze.append(QString::fromUtf8(buffer));
	analyze.append("</B><BR>");
	any++;
      }
      if (!analyze.isEmpty()) {	
	result.append("<B>Analyze statistics:</B><BR>");
	result.append(analyze);
      }
      if (!any)
	return text(col);
      return result;
    } catch (const otl_exception &exc) {
      toStatusMessage(QString::fromUtf8((const char *)exc.msg));
      return text(col);
    }
  }
};

toResultCols::toResultCols(toConnection &conn,QWidget *parent,const char *name)
  : toResultView(false,true,conn,parent,name)
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
			" WHERE Owner = :f1<char[31]>\n"
			"   AND Table_Name = :f2<char[31]>\n"
			"   AND Column_Name = :f3<char[31]>",
			"Display column comments");

void toResultCols::query(const QString &sql,const list<QString> &param)
{
  SQL=sql;
  QString Owner;
  QString TableName;
  list<QString>::iterator cp=((list<QString> &)param).begin();
  if (cp!=((list<QString> &)param).end()) {
    SQL="\"";
    SQL+=*cp;
    Owner=*cp;
  }
  cp++;
  if (cp!=((list<QString> &)param).end()) {
    SQL.append("\".\"");
    SQL.append(*cp);
    TableName=(*cp);
    SQL+="\"";
  }
  LastItem=NULL;
  RowNumber=0;

  clear();
  setSorting(0);

  try {
    otl_stream ColComment(1,
			  SQLComment(Connection),
			  Connection.connection());

    Query=new otl_stream;

    QString str("SELECT * FROM ");
    str.append(SQL);
    str.append(" WHERE NULL = NULL");

    otl_stream Query(1,
		     str.utf8(),
		     Connection.connection());

    Description=Query.describe_select(DescriptionLen);

    toResultViewMLine *item;
    for (int i=0;i<DescriptionLen;i++) {
      item=new toResultColsItem(this,LastItem,NULL);
      LastItem=item;

      item->setText(10,Owner);
      item->setText(11,TableName);
      item->setText(1,QString::fromUtf8(Description[i].name));
      item->setText(0,QString::number(i+1));

      QString datatype;
      switch(Description[i].dbtype) {
      case 1:
      case 5:
      case 9:
      case 155:
	datatype="VARCHAR2";
	break;
      case 2:
      case 3:
      case 4:
      case 6:
      case 68:
	datatype="NUMBER";
	break;
      case 8:
      case 94:
      case 95:
	datatype="LONG";
	break;
      case 11:
      case 104:
	datatype="ROWID";
	break;
      case 12:
      case 156:
	datatype="DATE";
	break;
      case 15:
      case 23:
      case 24:
	datatype="RAW";
	break;
      case 96:
      case 97:
	datatype="CHAR";
	break;
      case 108:
	datatype="NAMED DATA TYPE";
	break;
      case 110:
	datatype="REF";
	break;
      case 112:
	datatype="CLOB";
	break;
      case 113:
      case 114:
	datatype="BLOB";
	break;
      }
      if (datatype=="NUMBER") {
	if (Description[i].prec) {
	  datatype.append(" (");
	  datatype.append(QString::number(Description[i].prec));
	  if (Description[i].scale!=0) {
	    datatype.append(",");
	    datatype.append(QString::number(Description[i].scale));
	  }
	  datatype.append(")");
	}
      } else {
	datatype.append(" (");
	datatype.append(QString::number(Description[i].dbsize));
	datatype.append(")");
      }
      item->setText(2,datatype);
      if (Description[i].nullok)
	item->setText(3,"NULL");
      else
	item->setText(3,"NOT NULL");
      ColComment<<Owner.utf8()<<TableName.utf8()<<Description[i].name;
      if (!ColComment.eof()) {
	char buffer[4001];
	ColComment>>buffer;
	item->setText(4,QString::fromUtf8(buffer));
      }
    }
  } TOCATCH
  updateContents();
}
