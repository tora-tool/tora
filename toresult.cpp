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
#include "toresult.h"
#include "totool.h"

#include <qtabwidget.h>
#include <qtimer.h>

#include "toresult.moc"

toResult::toResult()
  : Slots(this)
{
  QTimer::singleShot(1,&Slots,SLOT(setup()));
  Handled=true;
  Tabs=NULL;
  ParametersSet=FromSQL=false;
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
      if (ParametersSet)
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
  toQList params;
  query(sql,params);
}

void toResult::query(const toSQL &sql)
{
  setSQLName(sql.name());
  FromSQL=true;
  try {
    toQList params;
    query(toSQL::string(sql,connection()),params);
    setHandle(true);
  } catch(...) {
    setHandle(false);
  }
}

void toResult::query(const toSQL &sql,toQList &par)
{
  setSQLName(sql.name());
  FromSQL=true;
  try {
    query(toSQL::string(sql,connection()),par);
    setHandle(true);
  } catch(...) {
    setHandle(false);
  }
}

void toResult::setSQL(const toSQL &sql)
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
  toQList params;
  toPush(params,toQValue(Param1));
  toPush(params,toQValue(Param2));
  toPush(params,toQValue(Param3));
  query(SQL,params);
}

void toResult::changeParams(const QString &Param1,const QString &Param2)
{
  toQList params;
  toPush(params,toQValue(Param1));
  toPush(params,toQValue(Param2));
  query(SQL,params);
}

void toResult::changeParams(const QString &Param1)
{
  toQList params;
  toPush(params,toQValue(Param1));
  query(SQL,params);
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
  if (!Result->Handled)
    Result->changeHandle();
}

bool toResult::setSQLParams(const QString &sql,const toQList &par)
{
  if (!toTool::globalConfig(CONF_DONT_REREAD,"Yes").isEmpty()) {
    if (SQL==sql&&par.size()==Params.size()) {
      toQList::iterator i=((toQList &)par).begin();
      toQList::iterator j=Params.begin();
      while(i!=((toQList &)par).end()&&j!=Params.end()) {
	if (QString(*i)!=QString(*j))
	  break;
	i++;
	j++;
      }
      if (i==((toQList &)par).end()&&j==Params.end())
	return false;
    }
  }
  SQL=sql;
  Params=par;
  return true;
}
