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

#ifndef __TOTHREAD_H
#define __TOTHREAD_H

#ifdef WIN32
#define TO_QTHREAD
#endif

/** This is an abstract class that defines something that is to be performed by a
 * thread.
 */
class toTask {
public:
  virtual ~toTask() { }
  /** This member is called when the class is started.
   */
  virtual void run(void) = 0;
};

#ifdef TO_QTHREAD
#include <qthread.h>
#include <list>

class toSemaphore {
private:
  QMutex Mutex;
  QWaitCondition Condition;
  int Value;
public:
  toSemaphore()
    : Condition()
  { Value=0; }
  toSemaphore(const toSemaphore &);
  toSemaphore(int val)
    : Condition()
  { Value=val; }

  void up();
  void down();
  int getValue();
};

class toLock {
private:
  QMutex Mutex;
public:
  toLock(void)
    : Mutex(false)
  { }
  toLock(const toLock &);

  void lock(void)
  { Mutex.lock(); }
  void unlock(void)
  { Mutex.unlock(); }
};

class toThread {
private:
  /** Not part of the API.
   */

  class taskRunner : public QThread {
  public:
    toSemaphore		StartSemaphore;
    toTask		*Task;
    taskRunner(toTask *);
    virtual void run(void);
  }			Thread;
  static list<toThread *> *Threads;
  static toLock *Lock;
  static HANDLE MainThread;
  toThread(const toThread &);
public:
  toThread(toTask *);
  ~toThread();
  
  void start(void);
  void startAsync(void);
  static bool mainThread(void);
};

#else
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

/** Encapsulation of pthread semaphores. A semaphore can be raise to any value
 * but will wait till raised above zero when lowered below 0. Can also be implemented
 * without pthreads using Qt multithreaded primitives.
 */

class toSemaphore {
private:
  /** Actual semaphore.
   */
  sem_t			Semaphore;
  /** Initialise semaphore.
   * @param val Value to init semaphore to.
   */
  void			init(int val);

  toSemaphore(const toSemaphore &);
public:
  /** Create semaphore
   */
  toSemaphore();
  /** Create semaphore
   * @param val Value of new semaphore.
   */
  toSemaphore(int val);
  ~toSemaphore();

  /** Increase semaphore value by 1.
   */
  void up();
  /** Decrease semaphore value by 1, wait for it to never go below 0.
   */
  void down();
  /** Get current semaphore value.
   */
  int getValue();
};

/** A wrapper around the pthread mutexfunctions. A lock can only be locked
 *  by one thread at a time and is the basis of most thread synchronisation.
 */

class toLock {
private:
  /** Actual pthread mutex of class.
   */
  pthread_mutex_t Mutex;
  toLock(const toLock &);
public:
  /** Create lock
   */
  toLock(void);
  ~toLock();

  /** Lock this lock.
   */
  void lock(void);
  /** Unlock this lock.
   */
  void unlock(void);
};

/** Used to create new threads of execution. When a thread exits it will delete the
 * thread and task objects so the calling thread must never delete a started thread.
 */

class toThread {
private:
  /** PThread identifier.
   */
  pthread_t		Thread;
  /** Thread attributes.
   */
  pthread_attr_t	ThreadAttr;
  /** Task to run in new thread.
   */
  toTask		*Task;
  /** Semaphore that is raised when new thread has started running.
   */
  toSemaphore		StartSemaphore;
  /** Initialise thread attributes.
   */
  void			initAttr(void);
  /** Called when thread is started to execute task. Pointer to @ref toThread
   * is passed as parameter.
   */
  friend void		*toThreadStartWrapper(void*);
  /** Main thread id
   */
  static pthread_t MainThread;
  
  toThread(const toThread &);
public:
  /** Create thread.
   * @param task Task to run.
   */
  toThread(toTask *task);
  ~toThread();

  /** Start thread and wait for other thread to start running.
   */
  void start(void);
  /** Start thread and continue executing this thread until normal scheduling
   * handles over execution to child thread.
   */
  void startAsync(void);

  /** Send a signal to this thread. This function is not available if using
   * Qt Threads.
   */
  void kill(int signo);
  /** Returns true if this is the main thread.
   */
  static bool mainThread(void);
};

#endif

/** This is a convenience class that holds a lock for the duration of the scope
 * of the object. It is very convenient to use if exceptions can be thrown, simply
 * declare an auto @ref toLocker to hold the lock. If any exception is thrown the
 * locker will be deallocated and the lock released.
 */

class toLocker {
private:
  /** Lock held.
   */
  toLock &Lock;
  toLocker(const toLocker &);
public:
  /** Create locker.
   * @param lock Lock to hold.
   */
  toLocker(toLock &lock)
    : Lock(lock)
  { Lock.lock(); }
  ~toLocker()
  { Lock.unlock(); }
};

#endif
