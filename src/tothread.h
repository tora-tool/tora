/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
