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

#include "toresultpie.h"
#include "toconnection.h"
#include "tomain.h"
#include "toconf.h"
#include "tosql.h"

#include "toresultpie.moc"

toResultPie::toResultPie(QWidget *parent,const char *name)
  : toPieChart(parent,name)
{
  connect(timer(),SIGNAL(timeout()),this,SLOT(refresh()));
}

void toResultPie::query(const QString &sql,const toQList &param)
{
  if (!handled())
    return;

  setSQL(sql);
  setParams(param);
  try {
    toQuery query(connection(),sql,param);

    std::list<QString> labels;
    std::list<double> values;
    int len=query.columns();
    while(!query.eof()) {
      QString val=query.readValue();
      values.insert(values.end(),val.toDouble());
      if (len>1) {
	QString lab=query.readValue();
	labels.insert(labels.end(),lab);
      }
    }
    setValues(values,labels);
  } TOCATCH
  update();
}
