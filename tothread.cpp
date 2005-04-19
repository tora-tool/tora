/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "tomain.h"
#include "tothread.h"

#include <stdio.h>
#include <errno.h>

#include <qapplication.h>
#include <qstring.h>

#ifdef QT_THREAD_SUPPORT

#ifdef WIN32
bool toWaitCondition::wait(QMutex *mutex, unsigned long time)
{
    bool status = FALSE;

    // this implementation relies on existing wait() method
    if (mutex)
    {
        mutex->unlock();
        status = QWaitCondition::wait(time);
        mutex->lock ()
        ;
    }
    else
        status = QWaitCondition::wait(time);

    return status;
}
#endif

void toSemaphore::up(void)
{
    Mutex.lock();
    Value++;
    if (Value > 0)
        Condition.wakeOne();
    Mutex.unlock();
}

void toSemaphore::down(void)
{
    Mutex.lock();
    while (Value <= 0)
    {
        Condition.wait(&Mutex);
    }
    Value--;
    Mutex.unlock();
}

int toSemaphore::getValue(void)
{
    Mutex.lock();
    int val = Value;
    Mutex.unlock();
    return val;
}

#else

#define SEM_ASSERT(x) if((x)!=0) toStatusMessage(\
qApp->translate("toSemaphore","The semaphore function \"%1\" failed").arg(QString::fromLatin1( #x )));

void toSemaphore::init(int ival)
{
    SEM_ASSERT(sem_init(&Semaphore, 0, ival));
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

#endif

#ifndef WIN32

#include <time.h>

pthread_t toThread::MainThread = pthread_self();
toThread *toThread::DeleteThread;
toLock *toThread::Lock;

#define MUTEX_ASSERT(x) if((x)!=0) toStatusMessage(\
qApp->translate("toLock","The mutex function \"%1\" failed").arg(QString::fromLatin1( #x )));

toLock::toLock(void)
{
    MUTEX_ASSERT(pthread_mutex_init(&Mutex, NULL));
}

toLock::~toLock()
{
    MUTEX_ASSERT(pthread_mutex_destroy(&Mutex));
}

void toLock::lock ()
{
    MUTEX_ASSERT(pthread_mutex_lock(&Mutex));
}

void toLock::unlock()
{
    MUTEX_ASSERT(pthread_mutex_unlock(&Mutex));
}

#define THREAD_ASSERT(x) if((x)!=0) { \
  throw (qApp->translate("toThread","Thread function \"%1\" failed.").arg(QString::fromLatin1( #x ))); }

void toThread::initAttr()
{
    //create the thread detached, so everything is cleaned up
    //after it's finished.
    try
    {
        THREAD_ASSERT(pthread_attr_init(&ThreadAttr));
        THREAD_ASSERT(pthread_attr_setdetachstate(&ThreadAttr,
                      PTHREAD_CREATE_DETACHED));
    }
    TOCATCH
}

toThread::toThread(toTask *t)
        : Task(t)
{
    if (!Lock)
        Lock = new toLock;

    initAttr();
}

toThread::~toThread()
{
    THREAD_ASSERT(pthread_attr_destroy(&ThreadAttr));
}

void toThread::start()
{
    if (pthread_create(&(Thread),
                       &(ThreadAttr),
                       toThreadStartWrapper,
                       (void *)this) == 0)
        StartSemaphore.down();
    else
        throw qApp->translate("toThread", "Thread function \"%1\" failed.").arg("toThread::start()");
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
    toThread *thread = (toThread*)t;
    //tell whoever called this->start that we're running
    sigset_t Sigs;
    sigfillset(&Sigs);
#ifndef TO_NO_PTHREADSIGMASK

    sigdelset(&Sigs, SIGHUP);
    // I hope this signal will cancel subqueries
    sigdelset(&Sigs, SIGQUIT);
    sigdelset(&Sigs, SIGALRM);
    sigdelset(&Sigs, SIGTERM);
    sigdelset(&Sigs, SIGCHLD);
    sigdelset(&Sigs, SIGWINCH);
    sigdelset(&Sigs, SIGSEGV);
    sigdelset(&Sigs, SIGPIPE);
    sigdelset(&Sigs, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &Sigs, NULL);
    sigemptyset(&Sigs);
    sigaddset(&Sigs, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &Sigs, NULL);
#endif

    try
    {
        thread->StartSemaphore.up();
        thread->Task->run();
    }
    catch (const QString &exc)
    {
        fprintf(stderr, "Unhandled exception in thread:\n%s\n", (const char *)exc);
    }
    catch (...)
    {
        fprintf(stderr, "Unhandled exception in thread:\nUnknown type\n");
    }
    delete thread->Task;
    toThread::Lock->lock ()
    ;
    if (toThread::DeleteThread)
        delete toThread::DeleteThread;
    toThread::DeleteThread = thread;
    toThread::Lock->unlock();
    return NULL;
}

bool toThread::mainThread(void)
{
    return pthread_equal(MainThread, pthread_self());
}

#ifdef QT_THREAD_SUPPORT
class toThreadWrapper : public QThread
{
public:
    static void msleep(int num)
    {
        QThread::msleep(num);
    }
};

#endif

void toThread::msleep(int msec)
{
#ifdef QT_THREAD_SUPPORT
    toThreadWrapper::msleep(msec);
#else

    struct timespec req;
    req.tv_sec = msec / 1000;
    req.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&req, &req);
#endif
}

#else

std::list<toThread *> *toThread::Threads;
toLock *toThread::Lock;
int toThread::LastID = 0;
__declspec( thread ) int toThread::ThreadID = 0;
int toThread::MainThread = 0;

bool toThread::mainThread(void)
{
    return MainThread == ThreadID;
}

toThread::toThread(toTask *task)
        : Thread(task)
{
    if (!Threads)
        Threads = new std::list<toThread *>;
    if (!Lock)
        Lock = new toLock;
}

toThread::~toThread()
{}

void toThread::start(void)
{
    Thread.start();
    Thread.StartSemaphore.down();
    Lock->lock ()
    ;
    Threads->insert(Threads->end(), this);
    Lock->unlock();
}

void toThread::startAsync(void)
{
    Thread.start();
    Lock->lock ()
    ;
    Threads->insert(Threads->end(), this);
    Lock->unlock();
}

void toThread::msleep(int msec)
{
    taskRunner::msleep(msec);
}

toThread::taskRunner::taskRunner(toTask *task)
        : Task(task)
{}

void toThread::taskRunner::run(void)
{
    try
    {
        Lock->lock ()
        ;
        StartSemaphore.up();
        toThread::LastID++;
        toThread::ThreadID = LastID;
        Lock->unlock();
        Task->run();
        Lock->lock ()
        ;
        delete Task;
        Task = NULL;
        Lock->unlock();
    }
    catch (const QString &exc)
    {
        fprintf(stderr, "Unhandled exception in thread:\n%s\n", (const char *)exc);
    }
    catch (...)
    {
        fprintf(stderr, "Unhandled exception in thread:\nUnknown type\n");
    }


    // This is a cludge to clean up finnished threads, there won't be many hanging at least

    Lock->lock ()
    ;
    for (std::list<toThread *>::iterator i = toThread::Threads->begin();
            i != toThread::Threads->end();)
    {
        if ((*i)->Thread.finished())
        {
            delete (*i);
            Threads->erase(i);
            i = Threads->begin();
        }
        else
            i++;
    }
    Lock->unlock();
}

#endif
