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

#include "toresultbar.h"
#include "toconnection.h"
#include "tomain.h"
#include "toconf.h"
#include "tosql.h"
#include "totool.h"

#include "toresultbar.moc"

toResultBar::toResultBar(QWidget *parent,const char *name=NULL)
  : toBarChart(parent,name)
{
  connect(timer(),SIGNAL(timeout()),this,SLOT(refresh()));
  connect(toCurrentTool(this),SIGNAL(connectionChange()),
	  this,SLOT(clear()));
  LastStamp=0;
  Flow=true;
}

void toResultBar::query(const QString &sql,const toQList &param,bool first)
{
  SQL=sql;
  Param=param;
  try {
    toQuery query(connection(),sql,param);

    toQDescList desc=query.describe();

    if (first) {
      clear();
      list<QString> labels;
      for(toQDescList::iterator i=desc.begin();i!=desc.end();i++)
	if (i!=desc.begin())
	  labels.insert(labels.end(),(*i).Name);
      setLabels(labels);
    }

    while(!query.eof()) {
      int num=0;
      QString lab=query.readValue();
      num++;
      list<double> vals;
      while(num<query.columns()) {
	vals.insert(vals.end(),query.readValue().toDouble());
	num++;
      }
      if (Flow) {
	time_t now=time(NULL);
	if (now!=LastStamp) {
	  if (LastValues.size()>0) {
	    list<double> dispVal;
	    list<double>::iterator i=vals.begin();
	    list<double>::iterator j=LastValues.begin();
	    while(i!=vals.end()&&j!=LastValues.end()) {
	      dispVal.insert(dispVal.end(),(*i-*j)/(now-LastStamp));
	      i++;
	      j++;
	    }
	    list<double> tmp=transform(dispVal);
	    addValues(tmp,lab);
	  }
	  LastValues=vals;
	  LastStamp=now;
	}
      } else {
	list<double> tmp=transform(vals);
	addValues(tmp,lab);
      }
    }
  } TOCATCH
  update();
}

list<double> toResultBar::transform(list<double> &input)
{
  return input;
}

void toResultBar::setSQL(toSQL &sql)
{
  SQL=toSQL::string(sql,connection());
}

void toResultBar::query(toSQL &sql)
{
  query(toSQL::string(sql,connection()));
}
