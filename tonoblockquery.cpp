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

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "tonoblockquery.h"
#include "totool.h"
#include "toconf.h"
#include "tomain.h"
#include "toresultstats.h"

#undef TO_DEBUG
#ifdef TO_DEBUG
#include <stdio.h>
#define TO_DEBUGOUT(x) printf(x);
#else
#define TO_DEBUGOUT(x)
#endif

#define PREFETCH_SIZE 1000

void toNoBlockQuery::queryTask::run(void)
{
  TO_DEBUGOUT("Thread started\n");
  otl_stream Query;
  int Length;
  try {
    {
      Query.set_all_column_types(otl_all_num2str|otl_all_date2str);
      TO_DEBUGOUT("Open query\n");

      Query.open(1,
		 Parent.SQL.utf8(),
		 *Parent.LongConn);

      TO_DEBUGOUT("Supplying parameters\n");
      otl_null null;
      for (list<QString>::iterator i=((list<QString> &)Parent.Param).begin();
	   i!=((list<QString> &)Parent.Param).end();
	   i++) {
	if ((*i).isNull())
	  Query<<null;
	else
	  Query<<(*i).utf8();
      }
      TO_DEBUGOUT("Locking description\n");
      toLocker lock(Parent.Lock);
      otl_column_desc *desc=Query.describe_select(Parent.DescriptionLength);
      Length=Parent.DescriptionLength;
      Parent.Description=new otl_column_desc[Length];
      memcpy(Parent.Description,desc,sizeof(otl_column_desc)*Length);
    }
    if (!Query.eof()) {
      bool signaled=false;
      for (;;) {
	for (int i=0;i<Length;i++) {
	  TO_DEBUGOUT("Reading value\n");
	  QString value(toReadValue(Parent.Description[i],Query,Parent.MaxColSize));
	  {
	    TO_DEBUGOUT("Locking parent\n");
	    toLocker lock(Parent.Lock);
	    Parent.ReadingValues.insert(Parent.ReadingValues.end(),value);
	    if (Parent.ReadingValues.size()==1) {
	      if (signaled) {
		try { // Writing threadsafe is a bitch!
		  Parent.Lock.unlock();
		} catch(...) {
		  throw;
		}
		TO_DEBUGOUT("Pulling down continue\n");
		Parent.Continue.down();
		try {
		  Parent.Lock.lock();
		} catch(...) {
		  throw;
		}
	      }
	      TO_DEBUGOUT("Running up\n");
	      Parent.Running.up();
	      signaled=true;
	    }
	  }
	}
	TO_DEBUGOUT("Locking to check size\n");
	toLocker lock(Parent.Lock);
	if (Query.eof())
	  break;
	else {
	  if (Parent.ReadingValues.size()>PREFETCH_SIZE) {
	    try {
	      Parent.Lock.unlock();
	    } catch(...) {
	      throw;
	    }
	    TO_DEBUGOUT("Waiting for next\n");
	    Parent.Continue.down();
	    TO_DEBUGOUT("Done waiting\n");
	    signaled=false;
	    try {
	      Parent.Lock.lock();
	    } catch(...) {
	      throw;
	    }
	  }
	}
	if (Parent.Quit)
	  break;
      }
    }
    TO_DEBUGOUT("EOQ\n");
    Parent.Processed=Query.get_rpc();
  } catch (const otl_exception &exc) {
    TO_DEBUGOUT("Locking exception\n");
    toLocker lock(Parent.Lock);
    Parent.Error=QString::fromUtf8((const char *)exc.msg);
  } catch (const QString &str) {
    TO_DEBUGOUT("Locking exception string\n");
    toLocker lock(Parent.Lock);
    Parent.Error=str;
  }

  TO_DEBUGOUT("Locking EOQ\n");
  toLocker lock(Parent.Lock);
  TO_DEBUGOUT("Running up\n");
  Parent.Running.up();
  Parent.EOQ=true;
  TO_DEBUGOUT("Done\n");
}

QString toNoBlockQuery::readValue()
{
  if (CurrentValue==Values.end()) {
    TO_DEBUGOUT("Waiting for running\n");
    Running.down();
    TO_DEBUGOUT("Locking reading\n");
    toLocker lock(Lock);
    Values=ReadingValues;
    CurrentValue=Values.begin();
    ReadingValues.clear();
    TO_DEBUGOUT("Continue running\n");
    Continue.up();
  }
  checkError();
  QString ret=*CurrentValue;
  CurrentValue++;
  return ret;
}


toNoBlockQuery::toNoBlockQuery(toConnection &conn,const QString &sql,
			       const list<QString> &param,toResultStats *stats)
  : Connection(conn),SQL(sql),Param(param),Statistics(stats)
{
  TO_DEBUGOUT("Created no block query\n");
  CurrentValue=Values.end();
  Description=NULL;
  DescriptionLength=0;
  Quit=EOQ=false;
  Processed=0;
  MaxColSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();

  toLocker lock(Lock);
  try {
    LongConn=Connection.longOperation();
    if (Statistics)
      Statistics->changeSession(*LongConn);

    Thread=new toThread(new queryTask(*this));
    TO_DEBUGOUT("Created thread\n");
    Thread->start();
    TO_DEBUGOUT("Started thread\n");
  } catch(...) {
    Connection.longOperationFree(LongConn);
  }
}

otl_column_desc *toNoBlockQuery::describe(int &length)
{
  TO_DEBUGOUT("Locking describe\n");
  toLocker lock(Lock);
  checkError();
  length=DescriptionLength;
  return Description;
}

bool toNoBlockQuery::eof(void)
{
  TO_DEBUGOUT("Locking eof\n");
  toLocker lock(Lock);
  checkError();
  return EOQ&&CurrentValue==Values.end()&&!ReadingValues.size();
}

int toNoBlockQuery::getProcessed(void)
{
  TO_DEBUGOUT("Locking processed\n");
  toLocker lock(Lock);
  checkError();
  return Processed;
}

toNoBlockQuery::~toNoBlockQuery()
{
  {
    TO_DEBUGOUT("Locking for delete\n");
    toLocker lock(Lock);
    if (!EOQ) {
      TO_DEBUGOUT("Sending INT\n");
#if 1
      // This is how I would like it to work
      Thread->kill(SIGINT);
#else
      // This is how it works
      kill(getpid(),SIGINT);
#endif
      Quit=true;
    }
  }
  do {
    TO_DEBUGOUT("Waiting for client\n");
    Running.down();
    {
      TO_DEBUGOUT("Locking clear values\n");
      toLocker lock(Lock);
      ReadingValues.clear();
    }
    Continue.up();
    TO_DEBUGOUT("Locking delete EOQ\n");
    toLocker lock(Lock);
  } while(!EOQ);
  if (Statistics) {
    TO_DEBUGOUT("Get statistics\n");
    Statistics->refreshStats(false);
  }
  Connection.longOperationFree(LongConn);
  TO_DEBUGOUT("Done deleting\n");
}
