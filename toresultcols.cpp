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


#include "toresultcols.h"
#include "tomain.h"

class toResultColsItem : public toResultViewMLine {
public:
  toResultColsItem(QListView *parent,QListViewItem *after,const char *buffer)
    : toResultViewMLine(parent,after,buffer)
  { }
  virtual QString allText(int col) const
  {
    toResultCols *view=dynamic_cast<toResultCols *>(listView());
    try {
      otl_stream ColInfo;
      for (int i=0;i<10;i++)
	ColInfo.set_column_type(i+1,otl_var_char,4001);
      ColInfo.open(1,
		   "SELECT Data_Default,"
		   "       Num_Distinct,"
		   "       Low_Value,"
		   "       High_Value,"
		   "       Density,"
		   "       Num_Nulls,"
		   "       Num_Buckets,"
		   "       Last_Analyzed,"
		   "       Sample_Size,"
		   "       Avg_Col_Len"
		   "  FROM All_Tab_Columns"
		   " WHERE Owner = :f1<char[31]>"
		   "   AND Table_Name = :f2<char[31]>"
		   "   AND Column_Name = :f3<char[31]>",
		   view->Connection.connection());
      ColInfo<<(const char *)text(10)<<(const char *)text(11)<<(const char *)text(0);
      char buffer[4001];
      QString result("<B>");
      result.append(text(0));
      result.append("</B><BR><BR>");

      int any=0;
      ColInfo>>buffer;
      if (strlen(buffer)) {
	result.append("Default value: <B>");
	result.append(buffer);
	result.append("</B><BR><BR>");
	any++;
      }

      QString analyze;
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Distinct values: <B>");
	analyze.append(buffer);
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Low value: <B>");
	analyze.append(buffer);
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("High value: <B>");
	analyze.append(buffer);
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Density: <B>");
	analyze.append(buffer);
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Number of nulls: <B>");
	analyze.append(buffer);
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Number of histogram buckets: <B>");
	analyze.append(buffer);
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Last analyzed: <B>");
	analyze.append(buffer);
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Sample size: <B>");
	analyze.append(buffer);
	analyze.append("</B><BR>");
	any++;
      }
      ColInfo>>buffer;
      if (strlen(buffer)) {
	analyze.append("Average column size: <B>");
	analyze.append(buffer);
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
      toStatusMessage((const char *)exc.msg);
      return text(col);
    }
  }
};

toResultCols::toResultCols(toConnection &conn,QWidget *parent,const char *name=NULL)
  : toResultView(false,false,conn,parent,name)
{
  setReadAll(true);
  addColumn("Column Name");
  addColumn("Data Type");
  addColumn("NULL");
  addColumn("Comments");
}

QString toResultCols::query(const QString &sql,const QString *Param1,const QString *Param2,const QString *Param3)
{
  delete Query;
  SQL=sql;
  QString Owner;
  QString TableName;
  if (Param1) {
    SQL=*Param1;
    Owner=*Param1;
  }
  if (Param2) {
    SQL.append(".");
    SQL.append(*Param2);
    TableName=(*Param2);
  }
  Query=NULL;
  LastItem=NULL;
  RowNumber=0;

  clear();

  try {
    otl_stream ColComment(1,
			  "SELECT Comments FROM All_Col_Comments"
			  " WHERE Owner = :f1<char[31]>"
			  "   AND Table_Name = :f2<char[31]>"
			  "   AND Column_Name = :f3<char[31]>",
			  Connection.connection());

    Query=new otl_stream;

    QString str("SELECT * FROM ");
    str.append(SQL);
    str.append(" WHERE NULL = NULL");

    Query->open(1,
		(const char *)str,
		Connection.connection());

    Description=Query->describe_select(DescriptionLen);

    toResultViewMLine *item;
    for (int i=0;i<DescriptionLen;i++) {
      item=new toResultColsItem(this,LastItem,NULL);
      LastItem=item;

      item->setText(10,Owner);
      item->setText(11,TableName);
      item->setText(0,Description[i].name);

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
	if (Description[i].scale) {
	  datatype.append(" (");
	  datatype.append(QString::number(Description[i].scale));
	  if (Description[i].prec!=0) {
	    datatype.append(",");
	    datatype.append(QString::number(Description[i].prec));
	  }
	  datatype.append(")");
	}
      } else {
	datatype.append(" (");
	datatype.append(QString::number(Description[i].dbsize));
	datatype.append(")");
      }
      item->setText(1,datatype);
      if (Description[i].nullok)
	item->setText(2,"NULL");
      else
	item->setText(2,"NOT NULL");
      ColComment<<(const char *)Owner<<(const char *)TableName<<
	Description[i].name;
      if (!ColComment.eof()) {
	char buffer[4001];
	ColComment>>buffer;
	item->setText(3,buffer);
      }
    }
  } catch (const QString &str) {
    toStatusMessage((const char *)str);
    updateContents();
    return str;
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
    updateContents();
    return QString((const char *)exc.msg);
  }
  updateContents();
  return "";
}
