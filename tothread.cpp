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
#include <errno.h>

#include <qstring.h>

#include "otlv32.h"

#include "tothread.h"

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

bool toSemaphore::tryDown()
{
  if(sem_trywait(&Semaphore)==0)
    return true;
  return false;
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

bool toLock::tryLock() 
{
  int r;
  r=pthread_mutex_trylock(&Mutex);
  if (r==EBUSY)
    return false;
  else if (r==0)
    return true;
  else
    throw QString("The mutex function \"pthread_mutex_trylock\" failed");
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

void toThread::sleep(struct timespec tv)
{
  struct timespec tmp;
  THREAD_ASSERT(nanosleep(&tv,&tmp));
}

void *toThreadStartWrapper(void *t) 
{
  toThread *thread=(toThread*)t;
  //tell whoever called this->start that we're running
  sigset_t Sigs;
  sigfillset(&Sigs);
  sigdelset(&Sigs,SIGHUP);
  sigdelset(&Sigs,SIGINT);
  sigdelset(&Sigs,SIGQUIT);
  sigdelset(&Sigs,SIGALRM);
  sigdelset(&Sigs,SIGTERM);
  sigdelset(&Sigs,SIGCHLD);
  sigdelset(&Sigs,SIGWINCH);
  sigdelset(&Sigs,SIGSEGV);
  sigdelset(&Sigs,SIGPIPE);
  sigdelset(&Sigs,SIGUSR1);
  pthread_sigmask(SIG_BLOCK,&Sigs,NULL);
  try {
    thread->StartSemaphore.up();
    thread->Task->run();
  } catch (const otl_exception &exc) {
    printf("Unhandled exception in thread:\n%s\n",exc.msg);
  } catch(const QString &exc) {
    printf("Unhandled exception in thread:\n%s\n",(const char *)exc);
  } catch(...) {
    printf("Unhandled exception in thread:\nUnknown type\n");
  }
  delete thread->Task;
  delete thread;
  return NULL;
}

