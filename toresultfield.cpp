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
#include "tonoblockquery.h"
#include "toresultfield.h"
#include "tosql.h"
#include "tosqlparse.h"
#include "totool.h"

#include "toresultfield.moc"

toResultField::toResultField(QWidget *parent,const char *name)
  : toHighlightedText(parent,name)
{
  setReadOnly(true);
  Query=NULL;
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
}

toResultField::~toResultField()
{
  delete Query;
}

void toResultField::query(const QString &sql,const toQList &param)
{
  if (!setSQLParams(sql,param))
    return;

  try {
    clear();
    Unapplied=QString::null;
    if (Query) {
      delete Query;
      Query=NULL;
    }

    Query=new toNoBlockQuery(connection(),toQuery::Background,sql,param);
    Poll.start(100);
  } TOCATCH
}

#define THRESHOLD 10240

void toResultField::poll(void)
{
  try {
    if (Query&&Query->poll()) {
      while(Query->poll()&&!Query->eof()) {
	Unapplied+=Query->readValue();
      }
      if (Unapplied.length()>THRESHOLD) {
	append(Unapplied);
	Unapplied=QString::null;
      }
      if (Query->eof()) {
	delete Query;
	Query=NULL;
	Poll.stop();
	if (!toTool::globalConfig(CONF_AUTO_INDENT_RO,"Yes").isEmpty())
	  setText(toSQLParse::indent(text()+Unapplied));
	else
	  append(Unapplied);
	Unapplied=QString::null;
      }
    }
  } catch(const QString &exc) {
    delete Query;
    Query=NULL;
    Poll.stop();
    toStatusMessage(exc);
  }
}
