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

#include "toresultpie.h"
#include "toconnection.h"
#include "tomain.h"
#include "toconf.h"
#include "tosql.h"

#include "toresultpie.moc"

toResultPie::toResultPie(QWidget *parent,const char *name=NULL)
  : toPieChart(parent,name)
{ }

void toResultPie::query(const QString &sql,const list<QString> &param)
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

    list<QString> labels;
    list<double> values;
    int MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();
    int len;
    otl_column_desc *desc=str.describe_select(len);
    int num=0;
    while(!str.eof()) {
      QString val=toReadValue(desc[num%len],str,MaxColSize);
      values.insert(values.end(),val.toDouble());
      num++;
      if (len>1) {
	QString lab=toReadValue(desc[num%len],str,MaxColSize);
	labels.insert(labels.end(),lab);
	num++;
      }
    }
    setValues(values,labels);
  } TOCATCH
  update();
}

void toResultPie::setSQL(toSQL &sql)
{
  SQL=sql(connection());
}
