//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tonoblockquery.h"
#include "toresultline.h"
#include "tosql.h"
#include "totool.h"

#include <qpopupmenu.h>

#include "toresultline.moc"

toResultLine::toResultLine(QWidget *parent,const char *name)
  : toLineChart(parent,name)
{
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
  LastStamp=0;
  Flow=true;
  Columns=0;
  Query=NULL;
  Started=false;
}

toResultLine::~toResultLine()
{
  delete Query;
}

void toResultLine::start(void)
{
  if (!Started) {
    connect(timer(),SIGNAL(timeout()),this,SLOT(refresh()));
    Started=true;
  }
}

void toResultLine::stop(void)
{
  if (Started) {
    disconnect(timer(),SIGNAL(timeout()),this,SLOT(refresh()));
    Started=false;
  }
}

void toResultLine::query(const QString &sql,const toQList &param,bool first)
{
  if (!handled()||Query)
    return;

  start();
  setSQLParams(sql,param);

  try {
    First=first;
    Query=new toNoBlockQuery(connection(),toQuery::Background,sql,param);
    Poll.start(100);
  } TOCATCH
}

void toResultLine::poll(void)
{
  try {
    if (Query&&Query->poll()) {
      toQDescList desc;
      if (!Columns) {
	desc=Query->describe();
	Columns=desc.size();
      }

      if (First) {
	if (desc.size()==0)
	  desc=Query->describe();
	clear();
	std::list<QString> labels;
	for(toQDescList::iterator i=desc.begin();i!=desc.end();i++)
	  if (i!=desc.begin())
	    labels.insert(labels.end(),(*i).Name);
	setLabels(labels);
      }

       while(Query->poll()&&!Query->eof()) {
	unsigned int num=0;
	QString lab=Query->readValue();
	num++;
	std::list<double> vals;
	while(!Query->eof()&&num<Columns) {
	  vals.insert(vals.end(),Query->readValue().toDouble());
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
      if (Query->eof()) {
	Poll.stop();
	Columns=0;
	delete Query;
	Query=NULL;
	update();
      }
    }
  } catch(const QString &exc) {
    delete Query;
    Query=NULL;
    Poll.stop();
    toStatusMessage(exc);
  }
}

std::list<double> toResultLine::transform(std::list<double> &input)
{
  return input;
}

void toResultLine::connectionChanged(void)
{
  toResult::connectionChanged();
  clear();
}

void toResultLine::addMenues(QPopupMenu *popup)
{
  if (sqlName().length()) {
    popup->insertSeparator();
    popup->insertItem("Edit SQL...",
		      this,SLOT(editSQL()));
  }
}

void toResultLine::editSQL(void)
{
  toMainWidget()->editSQL(sqlName());
}
