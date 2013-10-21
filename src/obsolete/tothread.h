
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOTHREAD_H
#define TOTHREAD_H

#include "core/tora_export.h"


#include <QtGui/QMutex>
#include <QtGui/QWaitCondition>
#include <QtGui/QThread>
#include <QtGui/QThreadStorage>
#include <QtGui/QtGlobal>

/** This is an abstract class that defines something that is to be performed by a
 * thread.
 */
class toTask : public QObject
{
Q_OBJECT

public:
   virtual ~toTask()
   { }
   /** This member is called when the class is started.
    */
   virtual void run(void) = 0;
};

/** Encapsulation of pthread semaphores. A semaphore can be raise to any value
* but will wait till raised above zero when lowered below 0. Can also be implemented
* without pthreads using Qt multithreaded primitives. Observe that these function
* different than Qt semaphores.
*/

class toSemaphore
{
private:
   QMutex Mutex;
   QWaitCondition Condition;

   int Value;
public:
   /** Create semaphore
   */
   toSemaphore()
       : Condition()
   {
       Value = 0;
   }
   /** Unimplemented copy constructor.
   */
   toSemaphore(const toSemaphore &);
   /** Create semaphore
   * @param val Value of new semaphore.
   */
   toSemaphore(int val)
       : Condition()
   {
       Value = val;
   }

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

#include <list>

class toLock
{
private:
    QMutex Mutex;
public:
    toLock(void)
        : Mutex(QMutex::NonRecursive)
    { }
    toLock(const toLock &);

    void lock (void)
    {
        Mutex.lock();
    }
    void unlock(void)
    {
        Mutex.unlock();
    }
};

class toSharedLock
{
private:
    QMutex ExclusiveMutex;
    QAtomicInt sharedCount;
public:
    toSharedLock(void)
        : ExclusiveMutex(QMutex::NonRecursive)
        , sharedCount(0)
    {};

    void sharedLock()
    {
        if(sharedCount.fetchAndAddAcquire(1) == 0)
        {
            ExclusiveMutex.lock();
        }
        Q_ASSERT((int)sharedCount > 0);
    }

    void sharedUnlock()
    {
        if(!sharedCount.deref())
        {
            ExclusiveMutex.unlock();
        }
    }

    void exclusiveLock()
    {
        ExclusiveMutex.lock();
    }

    void exclusiveUnlock()
    {
        ExclusiveMutex.unlock();
    }
private:
    toSharedLock(const toSharedLock&);
};

void *toThreadStartWrapper(void*);
class toTaskRunner;
class toThreadInfo;

class TORA_EXPORT toThread
{
private:
   /** Not part of the API.
    */
   static QThreadStorage<toThreadInfo*> toThreadInfoStorage;
   volatile static unsigned lastThreadNumber;

   toTaskRunner *Thread;
   static std::list<toThread *> *Threads;
   static toLock *Lock;
   static QThread *MainThread;

   toThread(const toThread &);
public:
   toThread(toTask *);
   ~toThread();

   void start(void);
   void startAsync(void);
   static void msleep(int msec);
   static void sleep(int sec);
   static bool mainThread(void);

   /**
    * called from tomain.cpp to set the main thread. no more
    * guessing which thread is main!
    *
    */
   static void setMainThread(QThread *main);

   friend class toTaskRunner;
   friend class toThreadInfo;
   friend class toConnectionPoolTest;
   friend class toConnectionPoolExec;
};

/** This class is used for debugging purposes only
 * Holds thread specific data
 */
class toThreadInfo
{
public:
   unsigned threadNumber;
   QString threadTask;
   toThreadInfo(unsigned number);
   static unsigned getThreadNumber()
   {
       if(toThread::toThreadInfoStorage.hasLocalData())
       {
           toThreadInfo *i = toThread::toThreadInfoStorage.localData();
           return i->threadNumber;
       }
       else
       {
           throw QString("This thread has no in information associated");
       }
   }
   virtual ~toThreadInfo();
};

/** This is a convenience class that holds a lock for the duration of the scope
 * of the object. It is very convenient to use if exceptions can be thrown, simply
 * declare an auto @ref toLocker to hold the lock. If any exception is thrown the
 * locker will be deallocated and the lock released.
 */
class toLocker
{
private:
    /** Lock held.
     */
    toLock &Lock;
    toLocker(const toLocker &);
public:
    /** Create locker.
     * @param lock Lock to hold.
     */
    toLocker(toLock &lock )
        : Lock(lock )
    {
        Lock.lock();
    }
    ~toLocker()
    {
        Lock.unlock();
    }
};

/** This is a convenience class that holds a lock for the duration of the scope
 * of the object. It is very convenient to use if exceptions can be thrown, simply
 * declare an auto @ref toLocker to hold the lock. If any exception is thrown the
 * locker will be deallocated and the lock released.
 */
class toSharedLocker
{
private:
    /** Lock held.
     */
    toSharedLock &Lock;
public:
    /** Create locker.
     * @param lock Lock to hold.
     */
    toSharedLocker(toSharedLock &lock )
        : Lock(lock )
    {
        Lock.sharedLock();
    }
    ~toSharedLocker()
    {
        Lock.sharedUnlock();
    }
private:
    toSharedLocker(const toSharedLocker &);
};

class toExclusiveLocker
{
private:
    /** Lock held.
     */
    toSharedLock &Lock;
public:
    /** Create locker.
     * @param lock Lock to hold.
     */
    toExclusiveLocker(toSharedLock &lock )
        : Lock(lock )
    {
        Lock.exclusiveLock();
    }
    ~toExclusiveLocker()
    {
        Lock.exclusiveUnlock();
    }
private:
    toExclusiveLocker(const toExclusiveLocker &);
};


// much thanks to:
// http://www.ddj.com/cpp/184403766
// i added lock and unlock
template <typename T> class LockingPtr
{
public:
   // Constructors/destructors
   LockingPtr(volatile T& obj, QMutex& mtx) : pObj_(const_cast<T*>(&obj)),
       pMtx_(&mtx)
   {
       mtx.lock();
       locked = true;
   }

   /**
    * Overloaded contructor that attempts lock rather than blocking.
    *
    * Will throw an instance of QString if class failed to obtain
    * lock and tryLock is true.
    *
    */
   LockingPtr(volatile T& obj,
              QMutex& mtx,
              bool tryLock) : pObj_(const_cast<T*>(&obj)),
       pMtx_(&mtx)
   {
       if(tryLock)
       {
           locked = mtx.tryLock();
           if(!locked)
               throw QString::fromLatin1("Busy");
       }
       else
       {
           mtx.lock();
           locked = true;
       }
   }

   ~LockingPtr()
   {
       if(locked)
           pMtx_->unlock();
   }

   void lock()
   {
       pMtx_->lock();
       locked = true;
   }

   void unlock()
   {
       locked = false;
       pMtx_->unlock();
   }

   // Pointer behavior
   T& operator*()
   {
       return *pObj_;
   }

   T* operator->()
   {
       return pObj_;
   }

private:
   bool locked;
   T* pObj_;
   QMutex* pMtx_;
   LockingPtr(const LockingPtr&);
   LockingPtr& operator=(const LockingPtr&);
};

#endif
