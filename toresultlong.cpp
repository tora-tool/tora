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
	  RowNumber++;
	  int disp=0;
	  unsigned int cols=Description.size();
	  LastItem=createItem(LastItem,NULL);
	  if (NumberColumn) {
	    LastItem->setText(0,QString::number(RowNumber));
	    disp=1;
	  } else
	    LastItem->setText(cols,QString::number(RowNumber));
	  for (unsigned int j=0;(j<cols||j==0)&&!Query->eof();j++)
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

