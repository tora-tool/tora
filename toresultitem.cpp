//***************************************************************************
/*
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

TO_NAMESPACE;

#include <qlabel.h>
#include <qgrid.h>

#include <qheader.h>
#include <qtooltip.h>
#include "toresultitem.h"
#include "toresultresources.h"
#include "toresultitem.moc"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"

static toSQL SQLResource(TOSQL_RESULTRESOURCE,
			 "SELECT Loads,           Sorts,               First_Load_Time,\n"
			 "       Parse_Calls,     Disk_Reads,          Parsing_User_Id,\n"
			 "       Executions,      Buffer_Gets,         Parsing_Schema_Id,\n"
			 "       Users_Executing, Users_Opening,       Open_Versions,\n"
			 "       Sharable_Mem,    Rows_Processed,      Kept_Versions,\n"
			 "       Persistent_Mem,  Optimizer_Mode,      Loaded_Versions,\n"
			 "       Runtime_Mem,     Serializable_Aborts, Invalidations\n"
			 " FROM v$sqlarea WHERE Address||':'||Hash_Value = :f1<char[100]>",
			 "Display information about an SQL statement");

void toResultItem::setup(int num,bool readable)
{
  ReadableColumns=readable;
  Widgets=NULL;
  NumWidgets=0;
  WidgetPos=0;
  viewport()->setBackgroundMode(PaletteBackground);
  Result=new QGrid(2*num,viewport());
  addChild(Result);
  Result->setSpacing(3);
  ShowTitle=true;
  AlignRight=true;
  DataFont.setBold(true);
}

toResultItem::toResultItem(int num,bool readable,toConnection &conn,QWidget *parent,const char *name)
  : QScrollView(parent,name), DataFont(QFont::defaultFont()), Connection(conn)
{
  setup(num,readable);
}

toResultItem::toResultItem(int num,toConnection &conn,QWidget *parent,const char *name)
  : QScrollView(parent,name), DataFont(QFont::defaultFont()), Connection(conn)
{
  setup(num,false);
}

void toResultItem::start(void)
{
  WidgetPos=0;
}

// Must be alloced in multiples of 2
#define ALLOC_SIZE 1000

void toResultItem::addItem(const QString &title,const QString &value)
{
  if (WidgetPos>=NumWidgets) {
    NumWidgets+=ALLOC_SIZE;
    QWidget **tmp=new QWidget *[NumWidgets];
    for (int i=0;i<WidgetPos;i++)
      tmp[i]=Widgets[i];
    delete Widgets;
    Widgets=tmp;
    for (int j=WidgetPos;j<NumWidgets;j++)
      Widgets[j]=NULL;
  }
  QLabel *widget;
  if (!Widgets[WidgetPos]) {
    widget=new QLabel(title,Result);
    widget->setAlignment(AlignRight|AlignTop|ExpandTabs|WordBreak);
    if (ShowTitle)
      widget->show();
    Widgets[WidgetPos]=widget;
  } else {
    widget=((QLabel *)Widgets[WidgetPos]);
    widget->setText(title);
    if (ShowTitle)
      widget->show();
    else
      widget->hide();
  }
  WidgetPos++;
  if (!Widgets[WidgetPos]) {
    widget=new QLabel(value,Result);
    widget->setFont(DataFont);
    widget->setFrameStyle(StyledPanel|Sunken);
    if (AlignRight)
      widget->setAlignment(AlignRight|AlignTop|ExpandTabs|WordBreak);
    else
      widget->setAlignment(AlignLeft|AlignTop|ExpandTabs|WordBreak);
    Widgets[WidgetPos]=widget;
  } else {
    widget=((QLabel *)Widgets[WidgetPos]);
    widget->setText(value);
  }
  widget->show();
  WidgetPos++;
}

void toResultItem::done(void)
{
  for (int i=WidgetPos;i<NumWidgets;i++)
    if (Widgets[i])
      Widgets[i]->hide();
}

void toResultItem::query(const QString &sql,const list<QString> &param)
{
  SQL=sql;

  start();

  try {
    int DescriptionLen;
    otl_column_desc *Description;
    otl_stream Query;

    int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();

    Query.set_all_column_types(otl_all_num2str|otl_all_date2str);

    Query.open(1,
	       sql.utf8(),
	       Connection.connection());

    {
      for (list<QString>::iterator i=((list<QString> &)param).begin();i!=((list<QString> &)param).end();i++)
	Query<<(*i).utf8();
    }

    Description=Query.describe_select(DescriptionLen);

    char *buffer=new char[MaxColSize+1];
    buffer[MaxColSize]=0;
    try {
      for (int i=0;i<DescriptionLen&&!Query.eof();i++) {
	QString name=QString::fromUtf8(Description[i].name);
	if (ReadableColumns) {
	  bool inWord=false;
	  for (unsigned int j=0;j<name.length();j++) {
	    if (name.at(j)=='_')
	      name.ref(j)=' ';
	    if (name.at(j).isSpace())
	      inWord=false;
	    else if (name.at(j).isLetter()) {
	      if (inWord)
		name.ref(j)=name.at(j).lower();
	      else
		name.ref(j)=name.at(j).upper();
	      inWord=true;
	    }
	  }
	}

	addItem(name,toReadValue(Description[i],Query,MaxColSize));
      }
    } catch (...) {
      delete buffer;
      throw;
    }
    delete buffer;
    done();
  } catch (const QString &str) {
    done();
    toStatusMessage((const char *)str);
  } catch (const otl_exception &exc) {
    done();
    toStatusMessage(QString::fromUtf8((const char *)exc.msg));
  }
}

