/****************************************************************************
 *
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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include <stdio.h>
#include <map>

#include "toresultplan.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"
#include "tosql.h"

#include "toresultplan.moc"


toResultPlan::toResultPlan(toConnection &conn,QWidget *parent,const char *name)
  : toResultView(false,false,conn,parent,name)
{
  setAllColumnsShowFocus(true);
  setSorting(-1);
  setRootIsDecorated(true);
  addColumn("#");
  addColumn("Operation");
  addColumn("Options");
  addColumn("Object name");
  addColumn("Mode");
  addColumn("Cost");
  addColumn("Bytes");
  addColumn("Cardinality");
  setSQLName("toResultPlan");
}

static toSQL SQLViewPlan("toResultPlan:ViewPlan",
			 "SELECT ID,NVL(Parent_ID,0),Operation, Options, Object_Name, Optimizer, to_char(Cost), to_char(Bytes), to_char(Cardinality)\n"
			 "  FROM %s WHERE Statement_ID = 'Tora %d' ORDER BY NVL(Parent_ID,0),ID",
			 "Get the contents of a plan table. Observe the %s and %s which must be present and in the same order. Must return same columns");

void toResultPlan::query(const QString &sql,
			 const list<QString> &param)
{
  clear();

  try {
    char buffer[1000];

    QString chkPoint=toTool::globalConfig(CONF_PLAN_CHECKPOINT,DEFAULT_PLAN_CHECKPOINT);

    sprintf(buffer,"SAVEPOINT %s",(const char *)chkPoint.utf8());
    otl_cursor::direct_exec(Connection.connection(),buffer);

    int ident=(int)time(NULL);

    QString planTable=toTool::globalConfig(CONF_PLAN_TABLE,DEFAULT_PLAN_TABLE);

    sprintf(buffer,"EXPLAIN PLAN SET STATEMENT_ID = 'Tora %d' INTO %s FOR ",
	    ident,(const char *)planTable.utf8());
    QString explain=QString::fromUtf8(buffer);
    explain.append(sql);
    otl_cursor::direct_exec (Connection.connection(),explain.utf8());

    sprintf(buffer,SQLViewPlan(Connection),
	    (const char *)planTable.utf8(),ident);

    {
      map <QString,QListViewItem *> parents;
      map <QString,QListViewItem *> last;
      QListViewItem *lastTop=NULL;
      otl_stream query;
      query.set_all_column_types(otl_all_num2str|otl_all_date2str);
      query.open(1,
		 buffer,
		 Connection.connection());
      while(!query.eof()) {
	char id[50];
	char parentid[50];
	char operation[31];
	char options[31];
	char object[31];
	char optimizer[256];
	char cost[50];
	char bytes[50];
	char cardinality[50];
	query>>id;
	query>>parentid;
	query>>operation;
	query>>options;
	query>>object;
	query>>optimizer;
	query>>cost;
	query>>bytes;
	query>>cardinality;

	QListViewItem *item;
	if (parentid&&parents[parentid]) {
	  item=new QListViewItem(parents[parentid],last[parentid],
				 QString::fromUtf8(id),
				 QString::fromUtf8(operation),
				 QString::fromUtf8(options),
				 QString::fromUtf8(object),
				 QString::fromUtf8(optimizer),
				 QString::fromUtf8(cost),
				 QString::fromUtf8(bytes),
				 QString::fromUtf8(cardinality));
	  setOpen(parents[parentid],true);
	  parents[id]=item;
	  last[parentid]=item;
	} else {
	  item=new QListViewItem(this,lastTop,
				 QString::fromUtf8(id),
				 QString::fromUtf8(operation),
				 QString::fromUtf8(options),
				 QString::fromUtf8(object),
				 QString::fromUtf8(optimizer),
				 QString::fromUtf8(cost),
				 QString::fromUtf8(bytes),
				 QString::fromUtf8(cardinality));
	  parents[id]=item;
	  lastTop=item;
	}
      }
    }

    sprintf(buffer,"ROLLBACK TO SAVEPOINT %s",(const char *)chkPoint.utf8());
    otl_cursor::direct_exec(Connection.connection(),buffer);

  } TOCATCH
  updateContents();
}
