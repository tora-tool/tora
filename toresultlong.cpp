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

#include <stdio.h>

#include <qregexp.h>
#include <qclipboard.h>
#include <qheader.h>
#include <qtooltip.h>

#include "toresultlong.h"
#include "toresultlong.moc"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"
#include "tonoblockquery.h"

toResultLong::toResultLong(bool readable,bool dispCol,toQuery::queryMode mode,
			   QWidget *parent,const char *name)
  : toResultView(readable,dispCol,parent,name)
{
  Query=NULL;
  Statistics=NULL;
  Mode=mode;
  connect(&Timer,SIGNAL(timeout(void)),this,SLOT(addItem(void)));
}

toResultLong::toResultLong(QWidget *parent,const char *name)
  : toResultView(parent,name)
{
  Query=NULL;
  Statistics=NULL;
  Mode=toQuery::Long;
  connect(&Timer,SIGNAL(timeout(void)),this,SLOT(addItem(void)));
}

void toResultLong::query(const QString &sql,const toQList &param)
{
  if (!setSQLParams(sql,param)) {
    emit firstResult(toResult::sql(),
		     toConnection::exception("Will not reexecute same query"));
    emit done();
    return;
  }
  stop();
  Query=NULL;
  LastItem=NULL;
  RowNumber=0;
  First=true;

  clear();
  while(columns()>0) {
    removeColumn(0);
  }
  HasHeaders=false;

  if (NumberColumn)
    addColumn("#");

  try {
    Query=new toNoBlockQuery(connection(),Mode,sql,param,Statistics);

    if (ReadAll)
      MaxNumber=-1;
    else
      MaxNumber=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER).toInt();
    addItem();
  } catch (const toConnection::exception &str) {
    First=false;
    emit firstResult(toResult::sql(),str);
    emit done();
    if (Mode!=toQuery::Long)
      toStatusMessage(str);
  } catch (const QString &str) {
    First=false;
    emit firstResult(toResult::sql(),str);
    emit done();
    if (Mode!=toQuery::Long)
      toStatusMessage(str);
  }
  updateContents();
}

#define TO_POLL_CHECK 100

void toResultLong::editReadAll(void)
{
  MaxNumber=-1;
  Timer.start(TO_POLL_CHECK);
}

void toResultLong::addItem(void)
{
  try {
    if (Query) {
      if (Query->poll()) {
	bool em=false;
	char buffer[100];
	if (First) {
	  QString tmp=sql().simplifyWhiteSpace().mid(0,10).lower();
	  if (tmp.startsWith("update")||tmp.startsWith("delete")||tmp.startsWith("insert"))
	    sprintf(buffer,"%d rows processed",(int)Query->rowsProcessed());
	  else if (tmp.startsWith("select"))
	    sprintf(buffer,"Query executed");
	  else
	    sprintf(buffer,"Statement executed");
	  em=true;
	}
	if (!HasHeaders) {
	  Description=Query->describe();
	  bool hidden=false;

	  for (toQDescList::iterator i=Description.begin();i!=Description.end();i++) {
	    QString name=(*i).Name;
	    if (ReadableColumns)
	      toReadableColumn(name);
	    if (name.length()>0&&name[0]!=' ') {
	      if (hidden)
		throw QString("Can only hide last column in query");
	      if (name[0]=='-') {
		addColumn(name.right(name.length()-1));
		setColumnAlignment(columns()-1,AlignRight);
	      } else
		addColumn(name);
	    } else
	      hidden=true;
	  }
	  HasHeaders=true;

	  if (NumberColumn)
	    setSorting(0);
	  else
	    setSorting(Description.size());
	}

	if (!Query->eof()) {
	  int disp=0;
	  unsigned int cols=Description.size();
	  if (NumberColumn) {
	    disp=1;
	  }
	  do {
	    QListViewItem *last=LastItem;
	    LastItem=createItem(LastItem,NULL);
	    if (NumberColumn)
	      LastItem->setText(0,QString::number(RowNumber));
	    else
	      LastItem->setText(cols,QString::number(RowNumber));
	    for (unsigned int j=0;(j<cols||j==0)&&!Query->eof();j++)
	      LastItem->setText(j+disp,Query->readValue());
	    if (Filter&&!Filter->check(LastItem)) {
	      delete LastItem;
	      LastItem=last;
	    } else
	      RowNumber++;
	  } while(Query->poll()&&!Query->eof()&&(MaxNumber<0||MaxNumber>RowNumber));
	}
	if (em) {
	  First=false;
	  emit firstResult(sql(),toConnection::exception(QString(buffer)));
	}
	if (Query->eof()) {
	  cleanup();
	  return;
	}
	if (MaxNumber<0||MaxNumber>RowNumber) {
	  if (!Timer.isActive())
	    Timer.start(1); // Must use timer, would mean really long recursion otherwise
	  else
	    Timer.start(TO_POLL_CHECK);
	} else
	  Timer.stop();
      } else {
	if (Query->eof()) {
	  cleanup();
	  return;
	} else if (!Timer.isActive())
	  Timer.start(TO_POLL_CHECK);
      }
    }
  } catch (const toConnection::exception &str) {
    if (First) {
      First=false;
      emit firstResult(sql(),str);
      if (Mode!=toQuery::Long)
	toStatusMessage(str);
    } else
      toStatusMessage(str);
    cleanup();
  } catch (const QString &str) {
    if (First) {
      First=false;
      emit firstResult(sql(),str);
      if (Mode!=toQuery::Long)
	toStatusMessage(str);
    } else
      toStatusMessage(str);
    cleanup();
  }
}

void toResultLong::cleanup(void)
{
  delete Query;
  Query=NULL;
  emit done();  
  Timer.stop();
}

bool toResultLong::eof(void)
{
  return !Query||Query->eof();
}

void toResultLong::stop(void)
{
  if (Query) {
    delete Query;
    Query=NULL;
    Timer.stop();
    emit done();
  }
}

toResultLong::~toResultLong()
{
  if (Query) {
    delete Query;
    Query=NULL;
  }
}

