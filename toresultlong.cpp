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
#include <ctype.h>

#include <qclipboard.h>
#include <qheader.h>
#include <qtooltip.h>
#include "toresultlong.h"
#include "toresultlong.moc"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"
#include "tonoblockquery.h"

toResultLong::toResultLong(bool readable,bool dispCol,toConnection &conn,QWidget *parent,const char *name)
  : toResultView(readable,dispCol,conn,parent,name)
{
  Query=NULL;
  Statistics=NULL;
  connect(&Timer,SIGNAL(timeout(void)),this,SLOT(addItem(void)));
}

toResultLong::toResultLong(toConnection &conn,QWidget *parent,const char *name)
  : toResultView(conn,parent,name)
{
  Query=NULL;
  Statistics=NULL;
  connect(&Timer,SIGNAL(timeout(void)),this,SLOT(addItem(void)));
}

void toResultLong::query(const QString &sql,const list<QString> &param)
{
  delete Query;
  SQL=sql;
  Query=NULL;
  LastItem=NULL;
  RowNumber=0;
  First=true;

  clear();
  while(columns()>1) {
    removeColumn(1);
  }
  HasHeaders=false;

  if (!NumberColumn)
    removeColumn(0);

  try {
    Query=new toNoBlockQuery(Connection,sql,param,Statistics);

    addItem();

  } TOCATCH
  updateContents();
}

#define TO_POLL_CHECK 500

void toResultLong::addItem(void)
{
  if (Query) {
    if (Query->poll()) {
      try {
	if (First) {
	  char buffer[100];
	  if (Query->getProcessed()>0)
	    sprintf(buffer,"%d rows processed",(int)Query->getProcessed());
	  else
	    sprintf(buffer,"Query executed");
	  emit firstResult(SQL,buffer);
	  First=false;
	}
	if (!HasHeaders) {
	  Description=Query->describe(DescriptionLen);
	  if (Description) {
	    bool hidden=false;

	    for (int i=0;i<DescriptionLen;i++) {
	      QString name=QString::fromUtf8(Description[i].name);
	      if (ReadableColumns) {
		bool inWord=false;
		for (unsigned int j=0;j<name.length();j++) {
		  if (name[j]=='_')
		    name[j]=' ';
		  if (name[j].isSpace())
		    inWord=false;
		  else if (name[j].isLetter()) {
		    if (inWord)
		      name[j]=name[j].lower();
		    else
		      name[j]=name[j].upper();
		    inWord=true;
		  }
		}
	      }
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
	  }
	  setQueryColumns(DescriptionLen);
	  if (NumberColumn)
	    setSorting(0);
	  else
	    setSorting(DescriptionLen);
	}

	if (!Query->eof()) {
	  RowNumber++;
	  int disp=0;
	  LastItem=createItem(this,LastItem,NULL);
	  if (NumberColumn) {
	    LastItem->setText(0,QString::number(RowNumber));
	    disp=1;
	  } else
	    LastItem->setText(DescriptionLen,QString::number(RowNumber));
	  for (int j=0;(j<DescriptionLen||j==0)&&!Query->eof();j++)
	    LastItem->setText(j+disp,Query->readValue());
	}
	if (Query->eof()) {
	  delete Query;
	  Query=NULL;
	  emit done();
	  return;
	}
      } catch (const otl_exception &exc) {
	if (First) {
	  emit firstResult(SQL,QString::fromUtf8((const char *)exc.msg));
	  First=false;
	}
	emit done();
	toStatusMessage(QString::fromUtf8((const char *)exc.msg));
      } catch (const QString &str) {
	if (First) {
	  emit firstResult(SQL,str);
	  First=false;
	}
	emit done();
	toStatusMessage(str);
      }
    } else if (Query->eof()) {
      delete Query;
      Query=NULL;
      emit done();
      return;
    } else if (!Timer.isActive())
      Timer.start(TO_POLL_CHECK,true);
  }
}

toResultLong::~toResultLong()
{
  delete Query;
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
    emit done();
  }
}
