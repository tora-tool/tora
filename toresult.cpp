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

#include <qtabwidget.h>

#include "toresult.h"
#include "totool.h"
#include "tomain.h"

#include "toresult.moc"

toResult::toResult()
  : Slots(this)
{
  Setup=new QTimer;
  QObject::connect(Setup,SIGNAL(timeout()),&Slots,SLOT(setup()));
  Setup->start(1,true);
  Handled=true;
  Tabs=NULL;
  FromSQL=false;
}

void toResult::changeHandle(void)
{
  QWidget *widget=dynamic_cast<QWidget *>(this);
  if (widget) {
    widget->setEnabled(handled());
    QWidget *par=widget->parentWidget();
    while(par&&(par->isA("QVBox")||par->isA("QWidgetStack")||(Tabs&&Tabs!=par))) {
      if (!par->isA("QWidgetStack"))
	widget=par;
      par=par->parentWidget();
    }
    if (par&&par->isA("QTabWidget")) {
      QTabWidget *tab=(QTabWidget *)par;
      tab->setTabEnabled(widget,handled());
    }
  }
}

void toResult::setHandle(bool ena)
{
  bool last=Handled;
  if (!ena)
    Handled=false;
  else
    Handled=canHandle(connection());
  if (last!=Handled)
    changeHandle();
}

void toResult::connectionChanged(void)
{
  if(FromSQL) {
    try {
      query(toSQL::string(sqlName(),connection()),Params);
      setHandle(true);
    } catch(...) {
      setHandle(false);
    }
  } else
    setHandle(true);
}

toTimer *toResult::timer(void)
{
  return toCurrentTool(dynamic_cast<QWidget *>(this))->timer();
}

toConnection &toResult::connection(void)
{
  return toCurrentConnection(dynamic_cast<QWidget *>(this));
}

void toResult::query(const QString &sql)
{
  Params.clear();
  query(sql,Params);
}

void toResult::query(toSQL &sql)
{
  setSQLName(sql.name());
  FromSQL=true;
  try {
    Params.clear();
    query(toSQL::string(sql,connection()),Params);
    setHandle(true);
  } catch(...) {
    setHandle(false);
  }
}

void toResult::setSQL(toSQL &sql)
{
  setSQLName(sql.name());
  FromSQL=true;

  try {
    Params.clear();
    setSQL(toSQL::string(sql,connection()));
    setHandle(true);
  } catch(...) {
    setHandle(false);
  }
}

void toResult::changeParams(const QString &Param1,const QString &Param2,const QString &Param3)
{
  Params.clear();
  toPush(Params,toQValue(Param1));
  toPush(Params,toQValue(Param2));
  toPush(Params,toQValue(Param3));
  query(SQL,Params);
}

void toResult::changeParams(const QString &Param1,const QString &Param2)
{
  Params.clear();
  toPush(Params,toQValue(Param1));
  toPush(Params,toQValue(Param2));
  query(SQL,Params);
}

void toResultObject::connectionChanged(void)
{
  Result->connectionChanged();
}

void toResultObject::setup(void)
{
  QObject *obj=dynamic_cast<QObject *>(Result);
  if (!obj) {
    toStatusMessage("Internal error, toResult is not a descendant of toResult");
    return;
  }
  QObject::connect(toCurrentTool(obj),SIGNAL(connectionChange()),this,SLOT(connectionChanged()));
  if (Result->Handled)
    Result->Handled=Result->canHandle(Result->connection());
  Result->changeHandle();
  delete Result->Setup;
  Result->Setup=NULL;
}
