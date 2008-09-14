/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "tomain.h"
#include "tothread.h"
#include "tothread_p.h"

#include <stdio.h>
#include <errno.h>

#include <qapplication.h>
#include <qstring.h>

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

std::list<toThread *> *toThread::Threads;
toLock *toThread::Lock;
QThread *toThread::MainThread = NULL;

void toThread::setMainThread(QThread *main)
{
    toThread::MainThread = main;
}

bool toThread::mainThread(void)
{
    return toThread::MainThread == QThread::currentThread();
}

toThread::toThread(toTask *task)
{
    Thread = new taskRunner(task);
    if (!Threads)
        Threads = new std::list<toThread *>;
    if (!Lock)
        Lock = new toLock;
}

toThread::~toThread()
{
    delete Thread;
}

void toThread::start(void)
{
    Thread->start();
    Thread->StartSemaphore.down();
    Lock->lock ()
    ;
    Threads->insert(Threads->end(), this);
    Lock->unlock();
}

void toThread::startAsync(void)
{
    Thread->start();
    Lock->lock ()
    ;
    Threads->insert(Threads->end(), this);
    Lock->unlock();
}

void toThread::msleep(int msec)
{
    taskRunner::msleep(msec);
}

taskRunner::taskRunner(toTask *task)
        : Task(task)
{}

void taskRunner::run(void)
{
    try
    {
        toThread::Lock->lock ()
        ;
        StartSemaphore.up();
        toThread::mainThread();
        toThread::Lock->unlock();
        Task->run();
        toThread::Lock->lock ()
        ;
        delete Task;
        Task = NULL;
        toThread::Lock->unlock();
    }
    catch (const QString &exc)
    {
        fprintf(stderr, "Unhandled exception in thread:\n%s\n", exc.toAscii().constData());
    }
    catch (...)
    {
        fprintf(stderr, "Unhandled exception in thread:\nUnknown type\n");
    }


    // This is a cludge to clean up finnished threads, there won't be many hanging at least

    toThread::Lock->lock ()
    ;
    for (std::list<toThread *>::iterator i = toThread::Threads->begin();
            i != toThread::Threads->end();)
    {
        if ((*i)->Thread->isFinished())
        {
            delete (*i);
            toThread::Threads->erase(i);
            i = toThread::Threads->begin();
        }
        else
            i++;
    }
    toThread::Lock->unlock();
}
