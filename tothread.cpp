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
#include <errno.h>

#include <qstring.h>

#include "tothread.h"

#ifndef TO_QTHREAD

pthread_t toThread::MainThread=pthread_self();

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

bool toThread::mainThread(void)
{
  return pthread_equal(MainThread,pthread_self());
}

#else

std::list<toThread *> *toThread::Threads;
toLock *toThread::Lock;
HANDLE toThread::MainThread=QThread::currentThread();

bool toThread::mainThread(void)
{
  return MainThread==QThread::currentThread();
}

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
  Value--;
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
    Threads=new std::list<toThread *>;
  if (!Lock)
    Lock=new toLock;

  // This is a cludge to clean up finnished threads, there won't be many hanging at least

  Lock->lock();
  for (std::list<toThread *>::iterator i=Threads->begin();i!=Threads->end();) {
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
  for (std::list<toThread *>::iterator i=Threads->begin();i!=Threads->end();i++) {
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
  } catch(const QString &exc) {
    printf("Unhandled exception in thread:\n%s\n",(const char *)exc);
  } catch(...) {
    printf("Unhandled exception in thread:\nUnknown type\n");
  }
}

#endif
