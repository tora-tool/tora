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

#include "toresultpie.h"
#include "toconnection.h"
#include "tomain.h"
#include "toconf.h"
#include "tosql.h"
#include "tonoblockquery.h"

#include "toresultpie.moc"

toResultPie::toResultPie(QWidget *parent,const char *name)
  : toPieChart(parent,name)
{
  Query=NULL;
  Columns=0;
  connect(timer(),SIGNAL(timeout()),this,SLOT(refresh()));
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
}

void toResultPie::query(const QString &sql,const toQList &param)
{
  if (!handled()||Query)
    return;

  setSQL(sql);
  setParams(param);
  try {
    Query=new toNoBlockQuery(connection(),toQuery::Normal,sql,param);
    Poll.start(100);
  } TOCATCH
}

void toResultPie::poll(void)
{
  try {
    if (Query) {
      if (!Columns)
	Columns=Query->describe().size();
      while(Query->poll()&&!Query->eof()) {
	QString val=Query->readValue();
	Values.insert(Values.end(),val.toDouble());
	if (Columns>1) {
	  QString lab=Query->readValue();
	  Labels.insert(Labels.end(),lab);
	}
      }
      if (Query->eof()) {
	setValues(Values,Labels);
	Values.clear();
	Labels.clear();
	delete Query;
	Query=NULL;
	Columns=0;
	Poll.stop();
      }
    }
  } TOCATCH
}
