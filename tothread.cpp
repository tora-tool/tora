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
 *      software in the executable aside from Oracle client libraries.
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
#include <errno.h>

#include <qstring.h>

#include "otlv32.h"

#include "tothread.h"

#ifndef TO_QTHREAD

#define SEM_ASSERT(x) if((x)!=0) { throw QString(\
"Error in semaphore function \"" #x "\" didn't work"); }

void toSemaphore::init(int ival)
{
  SEM_ASSERT(sem_init(&Semaphore,0,ival));
}

toSemaphore::toSemaphore() 
{
  init(0);
}


toSemaphore::toSemaphore(int i)
{
  init(i);
}

toSemaphore::~toSemaphore()
{
  SEM_ASSERT(sem_destroy(&Semaphore));
}

void toSemaphore::up()
{
  SEM_ASSERT(sem_post(&Semaphore));
}

void toSemaphore::down()
{
  SEM_ASSERT(sem_wait(&Semaphore));
}

int toSemaphore::getValue()
{
  int r;
  SEM_ASSERT(sem_getvalue(&Semaphore, &r));
  return r;
}

#define MUTEX_ASSERT(x) if((x)!=0) throw QString(\
"The mutex function \"" #x "\" failed");

toLock::toLock(void)
{
  MUTEX_ASSERT(pthread_mutex_init(&Mutex,NULL));
}

toLock::~toLock()
{
  MUTEX_ASSERT(pthread_mutex_destroy(&Mutex));
}

void toLock::lock() 
{
  MUTEX_ASSERT(pthread_mutex_lock(&Mutex));
}

void toLock::unlock() 
{
  MUTEX_ASSERT(pthread_mutex_unlock(&Mutex));
}

#define THREAD_ASSERT(x) if((x)!=0) { \
  throw QString("Thread function \"" #x "\" failed."); }

void toThread::initAttr()
{
  //create the thread detached, so everything is cleaned up
  //after it's finished.
  THREAD_ASSERT(pthread_attr_init(&ThreadAttr));
  THREAD_ASSERT(pthread_attr_setdetachstate(&ThreadAttr,
					    PTHREAD_CREATE_DETACHED));
}	

toThread::toThread(toTask *t)
  :Task(t)
{
  initAttr();
}

toThread::~toThread()
{
  THREAD_ASSERT(pthread_attr_destroy(&ThreadAttr));
}

void toThread::start()
{
  THREAD_ASSERT(pthread_create(&(Thread),
			       &(ThreadAttr),
			       toThreadStartWrapper,
			       (void *)this));
  StartSemaphore.down();
}

void toThread::startAsync()
{
  THREAD_ASSERT(pthread_create(&(Thread),
			       &(ThreadAttr),
			       toThreadStartWrapper,
			       (void *)this));
}

void *toThreadStartWrapper(void *t) 
{
  toThread *thread=(toThread*)t;
  //tell whoever called this->start that we're running
  sigset_t Sigs;
  sigfillset(&Sigs);
  sigdelset(&Sigs,SIGHUP);
  // I hope this signal will cancel subqueries
  sigdelset(&Sigs,SIGQUIT);
  sigdelset(&Sigs,SIGALRM);
  sigdelset(&Sigs,SIGTERM);
  sigdelset(&Sigs,SIGCHLD);
  sigdelset(&Sigs,SIGWINCH);
  sigdelset(&Sigs,SIGSEGV);
  sigdelset(&Sigs,SIGPIPE);
  sigdelset(&Sigs,SIGUSR1);
  pthread_sigmask(SIG_BLOCK,&Sigs,NULL);
  sigemptyset(&Sigs);
  sigaddset(&Sigs,SIGINT);
  pthread_sigmask(SIG_UNBLOCK,&Sigs,NULL);
  try {
    thread->StartSemaphore.up();
    thread->Task->run();
  } catch (const otl_exception &exc) {
    printf("Unhandled exception in thread:\n%s\n",
	   (const char *)QString::fromUtf8((const char *)exc.msg));
  } catch(const QString &exc) {
    printf("Unhandled exception in thread:\n%s\n",(const char *)exc);
  } catch(...) {
    printf("Unhandled exception in thread:\nUnknown type\n");
  }
  delete thread->Task;
  delete thread;
  return NULL;
}

void toThread::kill(int signo)
{
  THREAD_ASSERT(pthread_kill(Thread,signo));
}

#else

list<toThread *> *toThread::Threads;
toLock *toThread::Lock;

void toSemaphore::up(void)
{
  Mutex.lock();
  Value++;
  if (Value>0)
    Condition.wakeOne();
  Mutex.unlock();
}

void toSemaphore::down(void)
{
  Mutex.lock();
  while(Value<=0) {
    Mutex.unlock();
    Condition.wait();
    Mutex.lock();
  }
  Value++;
  Mutex.unlock();
}

int toSemaphore::getValue(void)
{
  Mutex.lock();
  int val=Value;
  Mutex.unlock();
  return val;
}

toThread::toThread(toTask *task)
  : Thread(task)
{
  if (!Threads)
    Threads=new list<toThread *>;
  if (!Lock)
    Lock=new toLock;

  // This is a cludge to clean up finnished threads, there won't be many hanging at least

  Lock->lock();
  for (list<toThread *>::iterator i=Threads->begin();i!=Threads->end();) {
    if ((*i)->Thread.finished()&&(*i)!=this) {
      Lock->unlock();
      delete (*i);
      Lock->lock();
      i=Threads->begin();
    } else
      i++;
  }
  Lock->unlock();
}

toThread::~toThread()
{
  Lock->lock();
  for (list<toThread *>::iterator i=Threads->begin();i!=Threads->end();i++) {
    if ((*i)==this) {
      Threads->erase(i);
      break;
    }
  }
  Lock->unlock();
}

void toThread::start(void)
{
  Thread.start();
  Thread.StartSemaphore.down();
  Lock->lock();
  Threads->insert(Threads->end(),this);
  Lock->unlock();
}

void toThread::startAsync(void)
{
  Thread.start();
  Lock->lock();
  Threads->insert(Threads->end(),this);
  Lock->unlock();
}

toThread::taskRunner::taskRunner(toTask *task)
  : Task(task)
{
}

void toThread::taskRunner::run(void)
{
  try {
    StartSemaphore.up();
    Task->run();
  } catch (const otl_exception &exc) {
    printf("Unhandled exception in thread:\n%s\n",
	   (const char *)QString::fromUtf8((const char *)exc.msg));
  } catch(const QString &exc) {
    printf("Unhandled exception in thread:\n%s\n",(const char *)exc);
  } catch(...) {
    printf("Unhandled exception in thread:\nUnknown type\n");
  }
}

#endif
