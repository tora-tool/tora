//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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

#include <signal.h>
#include <sys/types.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include "tonoblockquery.h"
#include "totool.h"
#include "toconf.h"
#include "tomain.h"
#include "toresultstats.h"

#undef TO_DEBUG
#ifdef TO_DEBUG
#include <stdio.h>
#define TO_DEBUGOUT(x) printf((const char *)x);
#else
#define TO_DEBUGOUT(x)
#endif

#define PREFETCH_SIZE 1000

void toNoBlockQuery::queryTask::run(void)
{
  TO_DEBUGOUT("Thread started\n");
  int Length;
  try {
    TO_DEBUGOUT("Open query\n");
    Parent.Query->execute(Parent.SQL,Parent.Param);

    {
      TO_DEBUGOUT("Locking description\n");
      toLocker lock(Parent.Lock);
      Parent.Description=Parent.Query->describe();
      Length=Parent.Query->columns();
    }
    if (!Parent.Query->eof()) {
      bool signaled=false;
      for (;;) {
	for (int i=0;i<Length;i++) {
	  TO_DEBUGOUT("Reading value\n");
	  toQValue value(Parent.Query->readValueNull());
	  {
	    TO_DEBUGOUT("Locking parent\n");
	    toLocker lock(Parent.Lock);
	    Parent.ReadingValues.insert(Parent.ReadingValues.end(),value);
	    if (Parent.ReadingValues.size()==1) {
	      if (signaled) {
		Parent.Lock.unlock();
		TO_DEBUGOUT("Pulling down continue\n");
		Parent.Continue.down();
		Parent.Lock.lock();
	      }
	      TO_DEBUGOUT("Running up\n");
	      Parent.Running.up();
	      signaled=true;
	    }
	  }
	}
	TO_DEBUGOUT("Locking to check size\n");
	toLocker lock(Parent.Lock);
	if (Parent.Query->eof())
	  break;
	else {
	  if (Parent.ReadingValues.size()>PREFETCH_SIZE) {
	    Parent.Lock.unlock();
	    TO_DEBUGOUT("Waiting for next\n");
	    Parent.Continue.down();
	    TO_DEBUGOUT("Done waiting\n");
	    signaled=false;
	    Parent.Lock.lock();
	  }
	}
	if (Parent.Quit)
	  break;
      }
    }
    TO_DEBUGOUT("EOQ\n");
    Parent.Processed=Parent.Query->rowsProcessed();
  } catch (const toConnection::exception &str) {
    TO_DEBUGOUT("Locking exception string\n");
    toLocker lock(Parent.Lock);
    Parent.Error=str;
  } catch (const QString &str) {
    TO_DEBUGOUT("Locking exception string\n");
    toLocker lock(Parent.Lock);
    Parent.Error=str;
  } catch (...) {
    TO_DEBUGOUT("Unknown exception\n");
    toLocker lock(Parent.Lock);
    Parent.Error=QString("Unknown exception");
  }

  TO_DEBUGOUT("Locking EOQ\n");
  toLocker lock(Parent.Lock);
  TO_DEBUGOUT("Deleting query\n");
  delete Parent.Query;
  Parent.Query=NULL;
  TO_DEBUGOUT("Running up\n");
  Parent.Running.up();
  Parent.EOQ=true;
  TO_DEBUGOUT("Done\n");
}

QString toNoBlockQuery::readValueNull()
{
  if (CurrentValue==Values.end()) {
    TO_DEBUGOUT("Waiting for running\n");
    Lock.lock();
    bool eoq=EOQ;
    Lock.unlock();
    if (!eoq)
      Running.down();
    TO_DEBUGOUT("Locking reading\n");
    toLocker lock(Lock);
    Values=ReadingValues;
    CurrentValue=Values.begin();
    ReadingValues.clear();
    TO_DEBUGOUT("Continue running\n");
    Continue.up();
    if (CurrentValue==Values.end())
      throw QString("Reading past end of query");
  }
  checkError();
  QString ret=*CurrentValue;
  CurrentValue++;
  return ret;
}

QString toNoBlockQuery::readValue()
{
  QString ret=readValueNull();
  if (ret.isNull())
    ret="{null}";
  return ret;
}

toNoBlockQuery::toNoBlockQuery(toConnection &conn,const QString &sql,
			       const toQList &param,toResultStats *stats)
  : SQL(sql),
    Error(QString::null),
    Param(param),
    Statistics(stats)
{
  try {
    Query=NULL;
    Query=new toQuery(conn,toQuery::Long);
  } catch(...) {
    delete Query;
    throw;
  }
  TO_DEBUGOUT("Created no block query\n");
  CurrentValue=Values.end();
  Quit=EOQ=false;
  Processed=0;

  if (Statistics)
    Statistics->changeSession(*Query);

  int val=toTool::globalConfig(CONF_AUTO_LONG,"0").toInt();
  if (val!=0)
    Started=time(NULL)+val;
  else
    Started=0;

  toLocker lock(Lock);
  TO_DEBUGOUT("Creating thread\n");
  Thread=new toThread(new queryTask(*this));
  TO_DEBUGOUT("Created thread\n");
  Thread->start();
  TO_DEBUGOUT("Started thread\n");
}

toNoBlockQuery::toNoBlockQuery(toConnection &conn,toQuery::queryMode mode,
			       const QString &sql,const toQList &param,toResultStats *stats)
  : SQL(sql),
    Error(QString::null),
    Param(param),
    Statistics(stats)
{
  try {
    Query=NULL;
    Query=new toQuery(conn,toQuery::Long);
  } catch(...) {
    delete Query;
    throw;
  }
  TO_DEBUGOUT("Created no block query\n");
  CurrentValue=Values.end();
  Quit=EOQ=false;
  Processed=0;

  if (Statistics)
    Statistics->changeSession(*Query);

  int val=toTool::globalConfig(CONF_AUTO_LONG,"0").toInt();
  if (val!=0)
    Started=time(NULL)+val;
  else
    Started=0;

  toLocker lock(Lock);
  TO_DEBUGOUT("Creating thread\n");
  Thread=new toThread(new queryTask(*this));
  TO_DEBUGOUT("Created thread\n");
  Thread->start();
  TO_DEBUGOUT("Started thread\n");
}

toQDescList &toNoBlockQuery::describe(void)
{
  TO_DEBUGOUT("Locking describe\n");
  toLocker lock(Lock);
  checkError();
  return Description;
}

bool toNoBlockQuery::eof(void)
{
  TO_DEBUGOUT("Locking eof\n");
  toLocker lock(Lock);
  checkError();
  return EOQ&&CurrentValue==Values.end()&&!ReadingValues.size();
}

int toNoBlockQuery::rowsProcessed(void)
{
  TO_DEBUGOUT("Locking processed\n");
  toLocker lock(Lock);
  checkError();
  return Processed;
}

void toNoBlockQuery::stop(void)
{
  {
    TO_DEBUGOUT("Locking for delete\n");
    Lock.lock();
    if (!EOQ) {
      TO_DEBUGOUT("Sending INT\n");
      do {
	if (Query)
	  Query->cancel();
	TO_DEBUGOUT("Sleeping and retrying cancel\n");
	Lock.unlock();
	toThread::msleep(100);
	Lock.lock();
      } while(Running.getValue()==0);
    }
    Lock.unlock();
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
  TO_DEBUGOUT("Done deleting\n");
}

toNoBlockQuery::~toNoBlockQuery()
{
  stop();
}

bool toNoBlockQuery::poll(void)
{
  unsigned int count=0;
  for(toQList::iterator i=CurrentValue;i!=Values.end();i++)
    count++;
  if (Running.getValue()>0) {
    toLocker lock(Lock);
    count+=ReadingValues.size();
  }

  if ((count>=Description.size()&&Description.size()>0)||eof())
    return true;

#if 0
  Lock.lock();
  if (Started>0&&Started<time(NULL)&&!Description.size()) {
    if (Query&&Query->mode()==toQuery::Normal) {
      toStatusMessage("Restarting query in own connection");
      toConnection &conn=Query->connection();
      Lock.unlock();
      TO_DEBUGOUT("Stopping normal query\n");
      stop();
      toLocker lock(Lock);
      while(Running.getValue()>0)
	Running.down();
      while(Continue.getValue()>0)
	Continue.down();
      Error=QString::null;
      ReadingValues.clear();
      Values.clear();
      CurrentValue=Values.end();
      Quit=EOQ=false;
      Processed=0;

      TO_DEBUGOUT("Creating new long query\n");
      Query=new toQuery(conn,toQuery::Long);

      if (Statistics)
	Statistics->changeSession(*Query);

      TO_DEBUGOUT("Creating thread\n");
      Thread=new toThread(new queryTask(*this));
      TO_DEBUGOUT("Created thread\n");
      Thread->start();
      TO_DEBUGOUT("Started thread\n");
    } else
      Lock.unlock();
  } else
    Lock.unlock();
#endif

  return false;
}

void toNoBlockQuery::checkError()
{
  if (!Error.isNull()) {
    TO_DEBUGOUT("Throwing exception\n");
    throw Error;
  }
}
