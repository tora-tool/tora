//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003 Quest Software, Inc
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "toresultlong.h"
#include "toresultlong.moc"
#include "toconf.h"
#include "totool.h"
#include "tonoblockquery.h"

#include <stdio.h>

#include <qregexp.h>
#include <qclipboard.h>
#include <qheader.h>
#include <qtooltip.h>

toResultLong::toResultLong(bool readable,bool dispCol,toQuery::queryMode mode,
			   QWidget *parent,const char *name,WFlags f)
  : toResultView(readable,dispCol,parent,name,f)
{
  Query=NULL;
  Statistics=NULL;
  Mode=mode;
  connect(&Timer,SIGNAL(timeout(void)),this,SLOT(addItem(void)));
}

toResultLong::toResultLong(QWidget *parent,const char *name,WFlags f)
  : toResultView(parent,name,f)
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
		     toConnection::exception(tr("Will not reexecute same query")),false);
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
  setSorting(-1);

  if (NumberColumn) {
    addColumn(QString::fromLatin1("#"));
    setColumnAlignment(0,AlignRight);
  }

  if (Filter)
    Filter->startingQuery();

  try {
    Query=new toNoBlockQuery(connection(),Mode,sql,param,Statistics);

    if (ReadAll)
      MaxNumber=-1;
    else
      MaxNumber=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER).toInt();
    addItem();
  } catch (const toConnection::exception &str) {
    First=false;
    emit firstResult(toResult::sql(),str,true);
    emit done();
    if (Mode!=toQuery::Long)
      toStatusMessage(str);
  } catch (const QString &str) {
    First=false;
    emit firstResult(toResult::sql(),str,true);
    emit done();
    if (Mode!=toQuery::Long)
      toStatusMessage(str);
  }
  updateContents();
}

#define TO_POLL_CHECK 100

void toResultLong::editReadAll(void)
{
  if (Query&&!Query->eof()) {
    MaxNumber=-1;
    Timer.start(TO_POLL_CHECK);
  }
}

void toResultLong::addItem(void)
{
  try {
    if (!toCheckModal(this))
      return;
    if (Query) {
      if (Query->poll()) {
	bool em=false;
	QString buffer;
	if (First) {
	  QString tmp=sql().simplifyWhiteSpace().mid(0,10).lower();
	  if (tmp.startsWith(QString::fromLatin1("update"))||
	      tmp.startsWith(QString::fromLatin1("delete"))||
	      tmp.startsWith(QString::fromLatin1("insert")))
	    buffer=tr("%1 rows processed").arg(Query->rowsProcessed());
	  else if (tmp.startsWith(QString::fromLatin1("select")))
	    buffer=tr("Query executed");
	  else
	    buffer=tr("Statement executed");
	  em=true;
	}
	if (!HasHeaders) {
	  Description=Query->describe();
	  bool hidden=false;

	  for (toQDescList::iterator i=Description.begin();i!=Description.end();i++) {
	    QString name=(*i).Name;
	    if (ReadableColumns)
	      toReadableColumn(name);
	    if (name.length()>0&&name[0].latin1()!=' ') {
	      if (hidden)
		throw tr("Can only hide last column in query");
	      if (name[0].latin1()=='-') {
		addColumn(toTranslateMayby(sqlName(),name.right(name.length()-1)));
		setColumnAlignment(columns()-1,AlignRight);
	      } else {
		addColumn(toTranslateMayby(sqlName(),name));
		if ((*i).AlignRight)
		  setColumnAlignment(columns()-1,AlignRight);
	      }
	    } else
	      hidden=true;
	  }
	  HasHeaders=true;

	  if (resizeMode()!=QListView::NoColumn)
	    setResizeMode(resizeMode());

	  if (sortColumn()<0) {
	    if (NumberColumn)
	      setSorting(0);
	    else
	      setSorting(Description.size());
	  }
	}

	if (!Query->eof()) {
	  int disp=0;
	  unsigned int cols=Description.size();
	  if (NumberColumn) {
	    disp=1;
	  }
	  do {
	    QListViewItem *last=LastItem;
	    LastItem=createItem(LastItem,QString::null);
	    if (NumberColumn)
	      LastItem->setText(0,QString::number(RowNumber+1));
	    else
	      LastItem->setText(cols,QString::number(RowNumber+1));
	    toResultViewItem *ri=dynamic_cast<toResultViewItem *>(LastItem);
	    toResultViewCheck *ci=dynamic_cast<toResultViewCheck *>(LastItem);
	    for (unsigned int j=0;(j<cols||j==0)&&!Query->eof();j++) {
	      if (ri)
		ri->setText(j+disp,Query->readValue());
	      else if (ci)
		ci->setText(j+disp,Query->readValue());
	      else
		LastItem->setText(j+disp,Query->readValue());
	    }
	    if (Filter&&!Filter->check(LastItem)) {
	      delete LastItem;
	      LastItem=last;
	    } else
	      RowNumber++;
	  } while(Query->poll()&&!Query->eof()&&(MaxNumber<0||MaxNumber>RowNumber));
	}
	if (em) {
	  First=false;
	  emit firstResult(sql(),toConnection::exception(buffer),false);
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
      emit firstResult(sql(),str,true);
      if (Mode!=toQuery::Long)
	toStatusMessage(str);
    } else
      toStatusMessage(str);
    cleanup();
  } catch (const QString &str) {
    if (First) {
      First=false;
      emit firstResult(sql(),str,true);
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

