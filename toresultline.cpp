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

#include "toresultline.h"
#include "toconnection.h"
#include "tomain.h"
#include "toconf.h"
#include "tosql.h"

#include "toresultline.moc"

toResultLine::toResultLine(QWidget *parent,const char *name=NULL)
  : toLineChart(parent,name)
{
  connect(timer(),SIGNAL(timeout()),this,SLOT(refresh()));
}

void toResultLine::query(const QString &sql,const list<QString> &param,bool first)
{
  try {
    otl_stream str;
    str.set_all_column_types(otl_all_num2str|otl_all_date2str);
    str.open(1,
	     sql.utf8(),
	     otlConnection());
    {
      otl_null null;
      for (list<QString>::iterator i=((list<QString> &)param).begin();i!=((list<QString> &)param).end();i++) {
	if ((*i).isNull())
	  str<<null;
        else
    	  str<<(*i).utf8();
      }
    }

    list<double> values;
    int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();
    list<QString> labels;
    int len;
    otl_column_desc *desc=str.describe_select(len);

    if (first) {
      clear();
      for(int i=0;i<len;i++)
	labels.insert(labels.end(),QString::fromUtf8(desc->name));
      setLabels(labels);
    }

    int num=0;
    while(!str.eof()) {
      QString lab=toReadValue(desc[num%len],str,MaxColSize);
      num++;
      list<double> vals;
      while(num<len) {
	QString lab=toReadValue(desc[num%len],str,MaxColSize);
	vals.insert(vals.end(),lab.toDouble());
	num++;
      }
      addValues(vals,lab);
    }
  } TOCATCH
  update();
}

void toResultLine::setSQL(toSQL &sql)
{
  SQL=sql(connection());
}
