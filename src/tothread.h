
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOTHREAD_H
#define TOTHREAD_H

#include "config.h"

#include <QMutex>
#include <QWaitCondition>
#include <QThread>

/** This is an abstract class that defines something that is to be performed by a
 * thread.
 */
class toTask
{
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

void *toThreadStartWrapper(void*);
class taskRunner;

class toThread
{
private:
    /** Not part of the API.
     */

    taskRunner *Thread;
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
    static bool mainThread(void);

    /**
     * called from tomain.cpp to set the main thread. no more
     * guessing which thread is main!
     *
     */
    static void setMainThread(QThread *main);

    friend class taskRunner;
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


// much thanks to:
// http://www.ddj.com/cpp/184403766
// i added lock and unlock
template <typename T> class LockingPtr {
public:
    // Constructors/destructors
    LockingPtr(volatile T& obj, QMutex& mtx) : pObj_(const_cast<T*>(&obj)),
                                               pMtx_(&mtx) {
        mtx.lock();
        locked = true;
    }

    ~LockingPtr() {
        if(locked)
            pMtx_->unlock();
    }

    void lock() {
        pMtx_->lock();
        locked = true;
    }

    void unlock() {
        locked = false;
        pMtx_->unlock();
    }

    // Pointer behavior
    T& operator*() {
        return *pObj_;
    }

    T* operator->() {
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
