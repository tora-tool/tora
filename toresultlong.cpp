//***************************************************************************
/*
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

TO_NAMESPACE;

#include <stdio.h>

#include <qclipboard.h>
#include <qheader.h>
#include <qtooltip.h>
#include "toresultlong.h"
#include "toresultlong.moc"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"
#include "tonoblockquery.h"

toResultLong::toResultLong(bool readable,bool dispCol,QWidget *parent,const char *name)
  : toResultView(readable,dispCol,parent,name)
{
  Query=NULL;
  Statistics=NULL;
  connect(&Timer,SIGNAL(timeout(void)),this,SLOT(addItem(void)));
}

toResultLong::toResultLong(QWidget *parent,const char *name)
  : toResultView(parent,name)
{
  Query=NULL;
  Statistics=NULL;
  connect(&Timer,SIGNAL(timeout(void)),this,SLOT(addItem(void)));
}

void toResultLong::query(const QString &sql,const toQList &param)
{
  stop();
  SQL=sql;
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
    Query=new toNoBlockQuery(connection(),sql,param,Statistics);

    MaxNumber=toTool::globalConfig(CONF_MAX_NUMBER,DEFAULT_MAX_NUMBER).toInt();
    addItem();
  } catch (const QString &str) {
    emit firstResult(SQL,str);
    emit done();
    throw;
  }
  updateContents();
}

#define TO_POLL_CHECK 500

void toResultLong::readAll(void)
{
  MaxNumber=-1;
  addItem();
}

void toResultLong::addItem(void)
{
  try {
    if (Query) {
      if (Query->poll()) {
	bool em=false;
	char buffer[100];
	if (First) {
	  if (Query->rowsProcessed()>0)
	    sprintf(buffer,"%d rows processed",(int)Query->rowsProcessed());
	  else
	    sprintf(buffer,"Query executed");
	  toStatusMessage(buffer);
	  em=true;
	}
	if (!HasHeaders) {
	  toQDescList Description=Query->describe();
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
	  RowNumber++;
	  int disp=0;
	  LastItem=createItem(LastItem,NULL);
	  if (NumberColumn) {
	    LastItem->setText(0,QString::number(RowNumber));
	    disp=1;
	  } else
	    LastItem->setText(Description.size(),QString::number(RowNumber));
	  for (unsigned int j=0;(j<Description.size()||j==0)&&!Query->eof();j++)
	    LastItem->setText(j+disp,Query->readValue());
	}
	if (em) {
	  First=false;
	  emit firstResult(SQL,buffer);
	}	  
	if (Query->eof()) {
	  cleanup();
	  return;
	}
	if (MaxNumber<0||MaxNumber>RowNumber)
	  Timer.start(1,true); // Must use timer, would mean really long recursion otherwise
      } else {
	if (Query->eof()) {
	  cleanup();
	  return;
	} else if (!Timer.isActive())
	  Timer.start(TO_POLL_CHECK,true);
      }
    }
  } catch (const QString &str) {
    if (First) {
      emit firstResult(SQL,str);
      First=false;
    }
    cleanup();
    toStatusMessage(str);
  }
}

void toResultLong::cleanup(void)
{
  delete Query;
  Query=NULL;
  emit done();  
}

bool toResultLong::eof(void)
{
  return !Query||Query->eof();
}

#ifdef TO_QTHREAD
class toDeleteQuery : public toTask {
  toNoBlockQuery *RIP;
public:
  toDeleteQuery(toNoBlockQuery *rip)
  { RIP=rip; }
  virtual void run(void)
  { delete RIP; }
};

#endif

void toResultLong::stop(void)
{
  if (Query) {
#ifdef TO_QTHREAD
    try {
      if (Query->poll())
	delete Query;
      else {
	toThread *thread=new toThread(new toDeleteQuery(Query));
	thread->startAsync();
      }
    } catch(...) {
      delete Query;
    }
#else
    delete Query;
#endif
    Query=NULL;
    emit done();
  }
}

toResultLong::~toResultLong()
{
  if (Query) {
#ifdef TO_QTHREAD
    toThread *thread=new toThread(new toDeleteQuery(Query));
    thread->startAsync();
#else
    delete Query;
#endif
    Query=NULL;
  }
}

