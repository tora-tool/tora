
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

#include "core/totaskrunner.h"
#include "core/utils.h"

#include <stdio.h>

toTaskRunner::toTaskRunner(toTask *task)
	: Task(task)
{
	setObjectName(QString::fromLatin1("TaskRunner:") + Task->objectName());
}

void toTaskRunner::run(void)
{
       Utils::toSetThreadName(*this);
	try {
		toThread::Lock->lock();
		toThread::toThreadInfoStorage.setLocalData(new toThreadInfo(++toThread::lastThreadNumber));
		StartSemaphore.up();
		toThread::mainThread();
		toThread::Lock->unlock();

		// Any background thread must not end with any exception raised
		// Note: TODO emit some kind of error into the main window here
		try {
			Task->run();
		} catch (const QString &exc) {
			fprintf(stderr, "Unhandled exception in thread:\n%s\n",	exc.toAscii().constData());
		} catch (...) {
			fprintf(stderr, "Unhandled exception in thread:\nUnknown type\n");
		}

		toThread::Lock->lock();
		delete Task;
		Task = NULL;
		toThread::Lock->unlock();
	} catch (const QString &exc) {
		fprintf(stderr, "Unhandled exception in thread:\n%s\n",	exc.toAscii().constData());
	} catch (...) {
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
