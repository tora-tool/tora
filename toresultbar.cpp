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

#include "toresultbar.h"
#include "toconnection.h"
#include "tomain.h"
#include "toconf.h"
#include "tosql.h"
#include "totool.h"

#include "toresultbar.moc"

toResultBar::toResultBar(QWidget *parent,const char *name)
  : toBarChart(parent,name)
{
  connect(timer(),SIGNAL(timeout()),this,SLOT(refresh()));
  LastStamp=0;
  Flow=true;
}

void toResultBar::query(const QString &sql,const toQList &param,bool first)
{
  if (!handled())
    return;

  setSQL(sql);
  setParams(param);
  try {
    toQuery query(connection(),sql,param);

    toQDescList desc=query.describe();

    if (first) {
      clear();
      std::list<QString> labels;
      for(toQDescList::iterator i=desc.begin();i!=desc.end();i++)
	if (i!=desc.begin())
	  labels.insert(labels.end(),(*i).Name);
      setLabels(labels);
    }

    while(!query.eof()) {
      int num=0;
      QString lab=query.readValue();
      num++;
      std::list<double> vals;
      while(num<query.columns()) {
	vals.insert(vals.end(),query.readValue().toDouble());
	num++;
      }
      if (Flow) {
	time_t now=time(NULL);
	if (now!=LastStamp) {
	  if (LastValues.size()>0) {
	    std::list<double> dispVal;
	    std::list<double>::iterator i=vals.begin();
	    std::list<double>::iterator j=LastValues.begin();
	    while(i!=vals.end()&&j!=LastValues.end()) {
	      dispVal.insert(dispVal.end(),(*i-*j)/(now-LastStamp));
	      i++;
	      j++;
	    }
	    std::list<double> tmp=transform(dispVal);
	    addValues(tmp,lab);
	  }
	  LastValues=vals;
	  LastStamp=now;
	}
      } else {
	std::list<double> tmp=transform(vals);
	addValues(tmp,lab);
      }
    }
  } TOCATCH
  update();
}

std::list<double> toResultBar::transform(std::list<double> &input)
{
  return input;
}

void toResultBar::connectionChanged(void)
{
  toResult::connectionChanged();
  clear();
}
