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

#include <stdio.h>

#include "toconf.h"
#include "tomain.h"
#include "toresultcontent.h"
#include "toresultview.h"

#include "toresultcontent.moc"

#define INC_SIZE 50

toResultContent::toResultContent(toConnection &conn,QWidget *parent,const char *name=NULL)
  : QTable(parent,name),Connection(conn)
{
  Query=NULL;
  connect(this,SIGNAL(currentChanged(int,int)),this,SLOT(changePosition(int,int)));
  CurrentRow=-1;
  setFocusPolicy(StrongFocus);
}

void toResultContent::wrongUsage(void)
{
  throw QString("Can't use these on toResultContent");
}

void toResultContent::changeParams(const QString &Param1,const QString &Param2)
{
  Owner=Param1;
  Table=Param2;
  setNumRows(0);
  setNumCols(0);

  delete Query;
  Query=NULL;

  try {
    QString sql;
    sql="SELECT * FROM \"";
    sql+=Owner;
    sql+="\".\"";
    sql+=Table;
    sql+="\"";
    Query=new otl_stream;
    Query->set_all_column_types(otl_all_num2str|otl_all_date2str);
    Query->open(1,
		sql.utf8(),
		Connection.connection());

    int descriptionLen;
    Description=Query->describe_select(descriptionLen);

    setNumCols(descriptionLen);
    QHeader *head=horizontalHeader();
    for (int i=0;i<descriptionLen;i++) {
      QString name(QString::fromUtf8(Description[i].name));
      head->setLabel(i,name);
    }
    int MaxNumber=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER).toInt();
    Row=0;
    setNumRows(INC_SIZE);
    for (int i=0;i<MaxNumber&&!Query->eof();i++) {
      if (Row+2>=numRows())
	setNumRows(numRows()+INC_SIZE);
      addRow();
    }
    setNumRows(Row+1);
  } TOCATCH
  OrigValues.clear();
  CurrentRow=-1;
}

void toResultContent::addRow(void)
{
  AddRow=false;
  int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();
  try {
    if (Query&&!Query->eof()) {
      if (Row+1>=numRows())
	setNumRows(Row+3);
      verticalHeader()->setLabel(Row,QString::number(Row+1));
      for (int j=0;j<numCols()&&!Query->eof();j++)
	setText(Row,j,toReadValue(Description[j],*Query,MaxColSize));
      Row++;
    }
  } TOCATCH
}

void toResultContent::keyPressEvent(QKeyEvent *e)
{
  if (e->key()==Key_PageDown) {
    int height=verticalHeader()->sectionSize(0);
    if (!Query->eof()&&height>0) {
      int num=visibleHeight()/height;
      setNumRows(Row+num+1);
      for (int i=0;i<num&&!Query->eof();i++) {
	if (Row+2>=numRows())
	  setNumRows(numRows()+INC_SIZE);
	addRow();
      }
      if (numRows()!=Row+1)
	setNumRows(Row+1);
    }
  }
  QTable::keyPressEvent(e);
}

void toResultContent::paintCell(QPainter *p,int row,int col,const QRect &cr,bool selected)
{
  if (row+1>=Row)
    AddRow=true;
  QTable::paintCell(p,row,col,cr,selected);
}

QWidget *toResultContent::beginEdit(int row,int col,bool replace)
{
  OrigValues.clear();
  for (int i=0;i<numCols();i++)
    OrigValues.insert(OrigValues.end(),text(row,i));

  return QTable::beginEdit(row,col,replace);
}

static bool nullString(const QString &str)
{
  return str=="{null}"||str.isNull();
}

void toResultContent::changePosition(int row,int col)
{
  if (CurrentRow!=row&&OrigValues.size()>0) {
    if (CurrentRow>=Row) {
      QString sql="INSERT INTO \"";
      sql+=Owner;
      sql+="\".\"";
      sql+=Table;
      sql+="\" VALUES (";
      for (int i=0;i<numCols();i++) {
	sql+=":f";
	sql+=QString::number(i);
	sql+="<char[4000]>";
	if (i+1<numCols())
	  sql+=",";
      }
      sql+=")";
      try {
	otl_stream exec(1,
			sql.utf8(),
			Connection.connection());
	otl_null null;
	for (int i=0;i<numCols();i++) {
	  QString str=text(CurrentRow,i);
	  otl_null null;
	  if (nullString(str))
	    exec<<null;
	  else
	    exec<<str.utf8();
	}
	Row++;
	setNumRows(Row+1);
      } TOCATCH
    } else {
      QString sql="UPDATE \"";
      sql+=Owner;
      sql+="\".\"";
      sql+=Table;
      sql+="\" SET ";
      QHeader *head=horizontalHeader();
      for(int i=0;i<numCols();i++) {
	sql+="\"";
	sql+=head->label(i);
	sql+="\" ";
	if (nullString(text(CurrentRow,i)))
	  sql+=" = NULL";
	else {
	  sql+="= :f";
	  sql+=QString::number(i);
	  sql+="<char[4000]>";
	}
	if (i+1<numCols())
	  sql+=", ";
      }
      sql+=" WHERE ";
      int col=0;
      for(list<QString>::iterator i=OrigValues.begin();i!=OrigValues.end();i++,col++) {
	sql+="\"";
	sql+=head->label(col);
	sql+="\" ";
	if (nullString(*i))
	  sql+=" IS NULL";
	else {
	  sql+="= :c";
	  sql+=QString::number(col);
	  sql+="<char[4000]>";
	}
	if (col+1<numCols())
	  sql+=" AND ";
      }
      try {
	otl_stream exec(1,
			sql.utf8(),
			Connection.connection());
      
	otl_null null;
	for (int i=0;i<numCols();i++) {
	  QString str=text(CurrentRow,i);
	  if (!nullString(str))
	    exec<<str.utf8();
	}
	for(list<QString>::iterator i=OrigValues.begin();i!=OrigValues.end();i++,col++) {
	  QString str=(*i);
	  if (!nullString(str))
	    exec<<str.utf8();
	}
      } TOCATCH
    }
    OrigValues.clear();
  }
  CurrentRow=row;
}

void toResultContent::drawContents(QPainter * p,int cx,int cy,int cw,int ch)
{
  QTable::drawContents(p,cx,cy,cw,ch);
  if (AddRow)
    addRow();
}

void toResultContent::readAll(void)
{
  while (Query&&!Query->eof()) {
    if (Row+2>=numRows())
      setNumRows(numRows()+INC_SIZE);
    addRow();
  }
  setNumRows(Row+1);
}

void toResultContent::print(void)
{
  toResultView print(false,true,Connection,this);
  print.hide();
  QString name="Content of ";
  name+=Owner;
  name+=".";
  name+=Table;
  print.setSQLName(name);
  QString sql="SELECT * FROM \"";
  sql+=Owner;
  sql+="\".\"";
  sql+=Table;
  sql+="\"";
  print.query(sql);
  print.print();
}

void toResultContent::activateNextCell()
{
  if (currentColumn()+1<numCols())
    setCurrentCell(currentRow(),currentColumn()+1);
  else {
    if (currentRow()+1>=numRows())
      setNumRows(Row+2);
    setCurrentCell(currentRow()+1,0);
  }
}

void toResultContent::focusInEvent (QFocusEvent *e)
{
  toMain::editEnable(false,false,true,
		     false,false,
		     false,false,false);
  QTable::focusInEvent(e);
}

void toResultContent::focusOutEvent (QFocusEvent *e)
{
  toMain::editDisable();
  QTable::focusOutEvent(e);
}

