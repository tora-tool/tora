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


#ifndef __TOTHREAD_H
#define __TOTHREAD_H

#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

class toSemaphore {
private:
  sem_t			Semaphore;
  void			init(int val);

public:
  toSemaphore();
  toSemaphore(const toSemaphore &);
  toSemaphore(int val);
  ~toSemaphore();

  void up();
  void down();
  bool tryDown();
  int getValue();
};

class toTask {
public:
  virtual ~toTask() { }
  virtual void run(void) = 0;
};

class toLock {
private:
  pthread_mutex_t Mutex;
public:
  toLock(void);
  toLock(const toLock &);
  ~toLock();

  void lock(void);
  void unlock(void);
  bool tryLock(void);

  operator pthread_mutex_t *()
  { return &Mutex; }
};

class toLocker {
private:
  toLock &Lock;
public:
  toLocker(toLock &lock)
    : Lock(lock)
  { Lock.lock(); }
  toLocker(const toLocker &);
  ~toLocker()
  { Lock.unlock(); }
};

class toThread {
private:
  pthread_t		Thread;
  pthread_attr_t	ThreadAttr;
  toTask		*Task;
  toSemaphore		StartSemaphore;
  void			initAttr(void);
  friend void		*toThreadStartWrapper(void*);
  
  toThread(const toThread &);
public:
  toThread(toTask *);
  ~toThread();
  
  void start(void);
  void startAsync(void);

  operator pthread_t()
  { return Thread; };

  static pthread_t getID(void)
  { return pthread_self(); }
  static void yield(void)
  { sched_yield(); }
  static void sleep(struct timespec time);
};

#endif
