//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "toresultline.h"
#include "toconnection.h"
#include "tomain.h"
#include "toconf.h"
#include "tosql.h"
#include "totool.h"

#include "toresultline.moc"

toResultLine::toResultLine(QWidget *parent,const char *name=NULL)
  : toLineChart(parent,name)
{
  connect(timer(),SIGNAL(timeout()),this,SLOT(refresh()));
  connect(toCurrentTool(this),SIGNAL(connectionChange()),
	  this,SLOT(clear()));
  LastStamp=0;
  Flow=true;
}

void toResultLine::query(const QString &sql,const toQList &param,bool first)
{
  SQL=sql;
  Param=param;
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

std::list<double> toResultLine::transform(std::list<double> &input)
{
  return input;
}

void toResultLine::setSQL(toSQL &sql)
{
  SQL=toSQL::string(sql,connection());
}

void toResultLine::query(toSQL &sql)
{
  query(toSQL::string(sql,connection()));
}
