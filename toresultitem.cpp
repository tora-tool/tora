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

#include <qlabel.h>
#include <qgrid.h>

#include <qheader.h>
#include <qtooltip.h>

#include "toresultitem.h"
#include "toresultresources.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"
#include "toconnection.h"

#include "toresultitem.moc"

static toSQL SQLResource(TOSQL_RESULTRESOURCE,
			 "SELECT 'Total' \"-\",         'per Execution' \"-\",                                                   'per Row processed' \"-\",\n"
			 "       Sorts,                 DECODE(Executions,0,'N/A',ROUND(Sorts/Executions,3)) \" \",         DECODE(Rows_Processed,0,'N/A',ROUND(Sorts/Rows_Processed,3)) \" \",\n"
                         "       Parse_Calls \"Parse\", DECODE(Executions,0,'N/A',ROUND(Parse_Calls/Executions,3)) \" \",   DECODE(Rows_Processed,0,'N/A',ROUND(Parse_Calls/Rows_Processed,3)) \" \",\n"
			 "       Disk_Reads,            DECODE(Executions,0,'N/A',ROUND(Disk_Reads/Executions,3)) \" \",    DECODE(Rows_Processed,0,'N/A',ROUND(Disk_Reads/Rows_Processed,3)) \" \",\n"
			 "       Buffer_Gets,           DECODE(Executions,0,'N/A',ROUND(Buffer_Gets/Executions,3)) \" \",   DECODE(Rows_Processed,0,'N/A',ROUND(Buffer_Gets/Rows_Processed,3)) \" \",\n"
			 "       Rows_Processed,        DECODE(Executions,0,'N/A',ROUND(Rows_Processed/Executions,3)) \" \",' ' \"-\",\n"
			 "       Executions,            ' ' \"-\",                                                          ' ' \"-\",\n"
			 "       ' ' \"-\",             ' ' \"-\",                                                          ' ' \"-\",\n"
                         "       Loads,                 First_Load_Time,                                                    Parsing_User_Id,\n"
			 "       Parsing_Schema_Id,     Users_Executing,                                                    Users_Opening,\n"
			 "       Open_Versions,         Sharable_Mem,                                                       Kept_Versions,\n"
			 "       Persistent_Mem,        Optimizer_Mode,                                                     Loaded_Versions,\n"
			 "       Runtime_Mem,           Serializable_Aborts,                                                Invalidations\n"
			 " FROM v$sqlarea WHERE Address||':'||Hash_Value = :f1<char[100]>",
			 "Display information about an SQL statement");

void toResultItem::setup(int num,bool readable)
{
  enableClipper(true);
  ReadableColumns=readable;
  Widgets=NULL;
  NumWidgets=0;
  WidgetPos=0;
  viewport()->setBackgroundMode(PaletteBackground);
  Result=new QGrid(2*num,viewport());
  addChild(Result);
  Result->setSpacing(3);
  ShowTitle=true;
  Right=true;
  DataFont.setBold(true);
}

toResultItem::toResultItem(int num,bool readable,QWidget *parent,const char *name)
  : QScrollView(parent,name), DataFont(QFont::defaultFont())
{
  setup(num,readable);
}

toResultItem::toResultItem(int num,QWidget *parent,const char *name)
  : QScrollView(parent,name), DataFont(QFont::defaultFont())
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
  QString t;
  if (title!="-")
    t=title;
  QLabel *widget;
  if (!Widgets[WidgetPos]) {
    widget=new QLabel(t,Result);
    widget->setAlignment(AlignRight|AlignVCenter|ExpandTabs|WordBreak);
    if (ShowTitle)
      widget->show();
    Widgets[WidgetPos]=widget;
  } else {
    widget=((QLabel *)Widgets[WidgetPos]);
    widget->setText(t);
    if (ShowTitle)
      widget->show();
    else
      widget->hide();
  }
  WidgetPos++;
  if (!Widgets[WidgetPos]) {
    widget=new QLabel(value,Result);
    if (title!="-") {
      widget->setFont(DataFont);
      widget->setFrameStyle(StyledPanel|Sunken);
    }
    if (Right)
      widget->setAlignment(AlignRight|AlignVCenter|ExpandTabs|WordBreak);
    else
      widget->setAlignment(AlignLeft|AlignVCenter|ExpandTabs|WordBreak);
    Widgets[WidgetPos]=widget;
  } else {
    widget=((QLabel *)Widgets[WidgetPos]);
    if (title!="-") {
      widget->setFrameStyle(StyledPanel|Sunken);
      widget->setFont(DataFont);
    } else {
      widget->setFrameStyle(NoFrame);
      widget->setFont(qApp->font());
    }
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

void toResultItem::query(const QString &sql,const toQList &param)
{
  SQL=sql;

  start();

  try {
    toQuery query(connection(),sql,param);
    toQDescList desc=query.describe();

    for (toQDescList::iterator i=desc.begin();i!=desc.end();i++) {
      QString name=(*i).Name;
      if (ReadableColumns)
	toReadableColumn(name);

      addItem(name,query.readValue());
    }
    done();
  } catch (const QString &str) {
    done();
    toStatusMessage((const char *)str);
  }
}

