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

toSGAStatement::toSGAStatement(QWidget *parent)
  : QTabWidget(parent)
{
  SQLText=new toResultField(this);
  addTab(SQLText,"SQL");
  Plan=new toResultPlan(this);
  addTab(Plan,"Execution plan");
  Resources=new toResultResources(this);
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
	SQLText->setText(toSQLString(toCurrentConnection(this),Address));
      else if (CurrentTab==Plan)
	Plan->query(toSQLString(toCurrentConnection(this),Address));
      else if (CurrentTab==Resources)
	viewResources();
    }
  } TOCATCH
}

void toSGAStatement::changeAddress(const QString &str)
{
  Address=str;
  changeTab(CurrentTab);
}
