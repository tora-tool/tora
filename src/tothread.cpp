
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

#include "utils.h"
#ifdef DEBUG
#include "tologger.h"
#endif

#include "tomain.h"
#include "tothread.h"
#include "totaskrunner.h"

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

QThreadStorage<toThreadInfo*> toThread::toThreadInfoStorage;
volatile unsigned toThread::lastThreadNumber = 0;
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
    Thread = new toTaskRunner(task);
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
    toTaskRunner::msleep(msec);
}

void toThread::sleep(int sec)
{
    toTaskRunner::sleep(sec);
}

toThreadInfo::toThreadInfo(unsigned number) :threadNumber(number), threadTask("")
{
#ifdef DEBUG
    TLOG(0,toDecorator,__HERE__) << "Thread no: " << threadNumber << " started." << std::endl;
#endif
}
toThreadInfo::~toThreadInfo()
{
#ifdef DEBUG
    TLOG(0,toDecorator,__HERE__) << "Thread no: " << threadNumber << " exited." << std::endl;
#endif
}
