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


#include "totool.h"
#include "toresultplan.h"
#include "tosgatrace.h"
#include "toresultview.h"
#include "toresultresources.h"
#include "toresultfield.h"
#include "toresultplan.h"
#include "toconf.h"
#include "tosgastatement.h"
#include "tomain.h"

#include "tosgastatement.moc"

void toSGAStatement::viewResources(void)
{
  try {
    Resources->changeParams(Address);
  } catch (...) {
    toStatusMessage("Couldn't find SQL statement in SGA");
  }
}

toSGAStatement::toSGAStatement(QWidget *parent,toConnection &connection)
  : QTabWidget(parent),Connection(connection)
{
  SQLText=new toResultField(Connection,this);
  addTab(SQLText,"SQL");
  Plan=new toResultPlan(connection,this);
  addTab(Plan,"Execution plan");
  Resources=new toResultResources(connection,this);
  addTab(Resources,"Information");
  connect(this,SIGNAL(currentChanged(QWidget *)),
	  this,SLOT(changeTab(QWidget *)));
  CurrentTab=SQLText;
}

void toSGAStatement::changeTab(QWidget *widget)
{
  try {
    CurrentTab=widget;
    if (!Address.isEmpty()) {
      if (CurrentTab==SQLText)
	SQLText->setText(toSQLString(Connection,Address));
      else if (CurrentTab==Plan)
	Plan->query(toSQLString(Connection,Address));
      else if (CurrentTab==Resources)
	viewResources();
    }
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
  } catch (const QString &str) {
    toStatusMessage(str);
  }
}

void toSGAStatement::changeAddress(const QString &str)
{
  Address=str;
  changeTab(CurrentTab);
}
