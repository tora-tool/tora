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
    Parent.Query.execute(Parent.SQL,Parent.Param);

    {
      TO_DEBUGOUT("Locking description\n");
      toLocker lock(Parent.Lock);
      Parent.Description=Parent.Query.describe();
      Length=Parent.Query.columns();
    }
    if (!Parent.Query.eof()) {
      bool signaled=false;
      for (;;) {
	for (int i=0;i<Length;i++) {
	  TO_DEBUGOUT("Reading value\n");
	  toQValue value(Parent.Query.readValue());
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
	if (Parent.Query.eof())
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
    Parent.Processed=Parent.Query.rowsProcessed();
  } catch (const QString &str) {
    TO_DEBUGOUT("Locking exception string\n");
    toLocker lock(Parent.Lock);
    Parent.Error=str;
  } catch (...) {
    TO_DEBUGOUT("Unknown exception\n");
    toLocker lock(Parent.Lock);
    Parent.Error="Unknown exception";
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
  }
  checkError();
  QString ret=*CurrentValue;
  CurrentValue++;
  return ret;
}


toNoBlockQuery::toNoBlockQuery(toConnection &conn,const QString &sql,
			       const toQList &param,toResultStats *stats)
  : SQL(sql),
    Param(param),
    Statistics(stats),
    Query(conn,toQuery::Long)
{
  TO_DEBUGOUT("Created no block query\n");
  CurrentValue=Values.end();
  Quit=EOQ=false;
  Processed=0;

  toLocker lock(Lock);
  if (Statistics)
    Statistics->changeSession(Query);

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

toNoBlockQuery::~toNoBlockQuery()
{
  {
    TO_DEBUGOUT("Locking for delete\n");
    toLocker lock(Lock);
    if (!EOQ) {
      TO_DEBUGOUT("Sending INT\n");
#ifndef TO_QTHREAD
      if (Query.connection().provider()=="Oracle")
	Thread->kill(SIGINT);
#else
      // Can't kill it, so well just leave it alone till it's done
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
  TO_DEBUGOUT("Done deleting\n");
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
  return false;
}

void toNoBlockQuery::checkError()
{
  if (!Error.isNull()) {
    TO_DEBUGOUT("Throwing exception\n");
    throw Error;
  }
}
